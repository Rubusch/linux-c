/*
 * drivers/gcov/gcov-core.c
 *
 * Core functionality for GCOV kernel profiling.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (c) International Business Machines Corp., 2002-2003
 *
 * Author: Hubertus Franke <frankeh@us.ibm.com>
 *         Rajan Ravindran <rajancr@us.ibm.com>
 *         Peter Oberparleiter <Peter.Oberparleiter@de.ibm.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/gcov.h>
#include <asm/semaphore.h>


#define GCOV_CORE_HEADER	"gcov-core: "

/* This structure is used to keep track of all struct bbs associated with a
 * module. */
struct gcov_context
{
	struct list_head list;
	struct module *module;
	unsigned long count;
	struct bb **bb;
};

/* Start of global constructor list. Has to be declared in arch/kernel/head.S */
extern char __CTOR_LIST__;

/* End of global constructor list. Has to be declared in arch/kernel/head.S */
extern char __DTOR_LIST__;

/* Linked list for registered struct bbs. */
struct bb *bb_head;

/* Callback informed of struct bb addition and removal. */
void (*gcov_callback)(enum gcov_cmd, struct bb *bbptr) = NULL;

/* Path to kernel files. */
const char *gcov_sourcepath = GCOV_SRC_PATH;
const char *gcov_objectpath = GCOV_OBJ_PATH;

/* List of contexts for registered bb entries. */
static LIST_HEAD(context_list);

/* Context into which blocks are inserted during initialization. */
static struct gcov_context *current_context;

/* Protect global variables from concurrent access. */
DECLARE_MUTEX(gcov_core_lock);

#if GCC_VERSION_LOWER(3, 4)
/* Register supplied struct BB. Called by each object code constructor. */
void
__bb_init_func(struct bb *bb)
{
	if (bb->zero_word)
		return;
	/* Set up linked list */
	bb->zero_word = 1;
	bb->next = bb_head;
	bb_head = bb;
	/* Associate with module context */
	if (current_context)
		current_context->bb[current_context->count++] = bb;
	/* Notify callback */
	if (gcov_callback != NULL)
		(*gcov_callback)(gcov_add, bb);
}


/* Unused functions needed to prevent linker errors. */
void __bb_fork_func(void) {}

EXPORT_SYMBOL_NOVERS(__bb_init_func);
EXPORT_SYMBOL_NOVERS(__bb_fork_func);
#else
gcov_unsigned_t gcov_version = 0;

/* Register supplied struct BB. Called by each object code constructor. */
void
__gcov_init(struct bb *bb)
{
	if (!bb->version)
		return;
	/* Check for compatible gcc version */
	if (gcov_version == 0)
		gcov_version = bb->version;
	else if (bb->version != gcov_version) {
		printk(KERN_WARNING GCOV_CORE_HEADER "gcc version mismatch in "
		       " file '%s'!\n", bb->filename);
		return;
	}
	/* Set up linked list */
	bb->version = 0;
	bb->next = bb_head;
	bb_head = bb;
	/* Associate with module context */
	if (current_context)
		current_context->bb[current_context->count++] = bb;
	/* Notify callback */
	if (gcov_callback != NULL)
		(*gcov_callback)(gcov_add, bb);
}


/* Unused functions needed to prevent linker errors. */
void __gcov_flush(void) {}
void __gcov_merge_add(gcov_type *counters, unsigned int n_counters) {}
void __gcov_merge_single(gcov_type *counters, unsigned int n_counters) {}
void __gcov_merge_delta(gcov_type *counters, unsigned int n_counters) {}

EXPORT_SYMBOL_NOVERS(gcov_version);
EXPORT_SYMBOL_NOVERS(__gcov_init);
EXPORT_SYMBOL_NOVERS(__gcov_flush);
EXPORT_SYMBOL_NOVERS(__gcov_merge_add);
EXPORT_SYMBOL_NOVERS(__gcov_merge_single);
EXPORT_SYMBOL_NOVERS(__gcov_merge_delta);
#endif /* GCC_VERSION_LOWER */


/* Call all constructor function pointers stored between CTORS_START and
 * CTORS_END. If specified, associate resulting bb data with MODULE. */
void
do_global_ctors(const char *ctors_start, const char *ctors_end,
		struct module *module)
{
	typedef void (*func_ptr)(void);
	func_ptr *func;
	unsigned long count;

	down(&gcov_core_lock);
	if (module) {
		/* Create a context to associate struct bbs with this MODULE */
		count = ((unsigned long) (ctors_end - ctors_start)) /
			sizeof(func_ptr);
		current_context = (struct gcov_context*) kmalloc(
					sizeof(struct gcov_context) +
					count * sizeof(struct bb *),
					GFP_KERNEL);
		if (!current_context) {
			printk(KERN_WARNING GCOV_CORE_HEADER "not enough memory"
			       " for coverage data!\n");
			up(&gcov_core_lock);
			return;
		}
		current_context->module = module;
		current_context->count = 0;
		current_context->bb = (struct bb **) (current_context + 1);
		list_add(&current_context->list, &context_list);
	}
	/* Call constructors */
	for (func = (func_ptr *) ctors_start;
	     *func && (func != (func_ptr *) ctors_end); func++)
		(*func)();
	current_context = NULL;
	up(&gcov_core_lock);
}


/* Remove data associated with MODULE. */
void
remove_bb_link(struct module *module)
{
	struct gcov_context* context;
	struct gcov_context* tmp;
	struct bb *bb;
	struct bb *prev;
	unsigned long i;

	down(&gcov_core_lock);
	/* Get associated context */
	context = NULL;
	list_for_each_entry(tmp, &context_list, list) {
		if (tmp->module == module) {
			context = tmp;
			break;
		}
	}
	if (!context) {
		up(&gcov_core_lock);
		return;
	}
	/* Remove all bb entries belonging to this module */
	prev = NULL;
	for (bb = bb_head; bb ; bb = bb->next) {
		for (i = 0; i < context->count; i++) {
			if (context->bb[i] == bb) {
				/* Detach bb from list. */
				if (prev)
					prev->next = bb->next;
				else
					bb_head = bb->next;
				/* Notify callback */
				if (gcov_callback)
					(*gcov_callback)(gcov_remove, bb);
				break;
			}
		}
		if (i == context->count)
			prev = bb;
	}
	list_del(&context->list);
	kfree(context);
	up(&gcov_core_lock);
}


static int __init
gcov_core_init(void)
{
	printk(KERN_INFO GCOV_CORE_HEADER "initializing core module\n");
	do_global_ctors(&__CTOR_LIST__, &__DTOR_LIST__, NULL);
	return 0;
}

module_init(gcov_core_init);


EXPORT_SYMBOL_NOVERS(bb_head);
EXPORT_SYMBOL_NOVERS(gcov_sourcepath);
EXPORT_SYMBOL_NOVERS(gcov_objectpath);
EXPORT_SYMBOL_NOVERS(gcov_callback);
EXPORT_SYMBOL_NOVERS(gcov_core_lock);
EXPORT_SYMBOL_NOVERS(do_global_ctors);
