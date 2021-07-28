/*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

/*
  forwards
*/

static int __init mod_init(void);
static void __exit mod_exit(void);

int init_hello_arguments(void);
void cleanup_hello_arguments(void);

int notify_param(const char *, const struct kernel_param *);


/*
  globals
*/

/**
 * module_param() - typesafe helper for a module/cmdline parameter
 * @name: the variable to alter, and exposed parameter name.
 * @type: the type of the parameter
 * @perm: visibility in sysfs.
 *
 * @name becomes the module parameter, or (prefixed by KBUILD_MODNAME
 * and a ".") the kernel commandline parameter.  Note that - is
 * changed to _, so the user can use "foo-bar=1" even for variable
 * "foo_bar".
 *
 * @perm is 0 if the variable is not to appear in sysfs, or 0444
 * for world-readable, 0644 for root-writable, etc.  Note that if it
 * is writable, you may need to use kernel_param_lock() around
 * accesses (esp. charp, which can be kfreed when it changes).
 *
 * The @type is simply pasted to refer to a param_ops_##type and a
 * param_check_##type: for convenience many standard types are
 * provided but you can create your own by defining those variables.
 *
 * Standard types are:
 *    byte, hexint, short, ushort, int, uint, long, ulong
 *    charp: a character pointer
 *    bool: a bool, values 0/1, y/n, Y/N.
 *    invbool: the above, only sense-reversed (N = true).
 */
int hello_int_arg;
module_param(hello_int_arg, int, S_IRUSR|S_IWUSR);

int hello_int_array[3];
module_param_array(hello_int_array, int, NULL, S_IRUSR|S_IWUSR);

char *hello_string_arg;
module_param(hello_string_arg, charp, S_IRUSR|S_IWUSR);

// the callback value for the customized setter
int hello_int_arg_cb = 0;

const struct kernel_param_ops hello_param_ops =
{
	// the customized setter
	.set = &notify_param,
	.get = &param_get_int, // standard getter from linunx/moduleparam.h
};

/**
 * module_param_cb() - general callback for a module/cmdline parameter
 * @name: a valid C identifier which is the parameter name.
 * @ops: the set & get operations for this parameter.
 * @arg: args for @ops
 * @perm: visibility in sysfs.
 *
 * The ops can have NULL set or get functions.
 */
module_param_cb(hello_int_arg_cb, &hello_param_ops, &hello_int_arg_cb, S_IRUGO|S_IWUSR);

/*
  implementation
*/

/*
  When setting values via /sys this callback function will be
  triggered.
*/
int notify_param(const char* val, const struct kernel_param *kp)
{
	/* The macro to do compile-time type checking */
	int res = param_set_int(val, kp);
	if (0 != res) {
		printk(KERN_ERR "%s() invalid argument\n", __func__);
		return -EINVAL;
	}
	printk(KERN_INFO "new value: %d\n", hello_int_arg_cb);
	return 0;
}


/*
  start / stop module
*/

int init_hello_arguments(void)
{
	int idx;

	printk(KERN_INFO "%s() initializing...\n", __func__);
	printk(KERN_INFO "%s() hello_int_arg = %d\n", __func__, hello_int_arg);
	printk(KERN_INFO "%s() hello_int_arg_cb = %d\n", __func__, hello_int_arg_cb);
	for (idx=0; idx<sizeof(hello_int_array) / sizeof(*hello_int_array); ++idx) {
		printk(KERN_INFO "%s() hello_int_array[%d] = %d\n", __func__, idx, hello_int_array[idx] );
	}
	printk(KERN_INFO "%s() hello_string_arg = '%s'\n", __func__, hello_string_arg);
	return 0;
}

void cleanup_hello_arguments(void)
{
	printk(KERN_INFO "%s() READY.\n", __func__);
}


/*
  init / exit
*/

static int __init mod_init(void)
{
	return init_hello_arguments();
}

static void __exit mod_exit(void)
{
	cleanup_hello_arguments();
}

module_init(mod_init);
module_exit(mod_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Demonstrates module load arguments.");
