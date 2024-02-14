// SPDX-License-Identifier: GPL-2.0+
/*
  displays a virtual memory area (vma) of a process provided as PID
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/moduleparam.h>

static int pid_mem = 1;
module_param(pid_mem, int, S_IRUGO);
MODULE_PARM_DESC(pid_mem, "PID of the process to list VMAs");

static void print_mem(struct mm_struct *mm)
{
        struct vm_area_struct *vma;
        int count = 0;

	VMA_ITERATOR(vmi, mm, 0);
        pr_info("%s(): this mm_struct has %d vmas\n", __func__, mm->map_count);

	mmap_read_lock(mm);
	for_each_vma(vmi, vma) {
                pr_info("%s(): vma number %d - start 0x%lx, end 0x%lx\n",
			__func__, ++count, vma->vm_start, vma->vm_end);
        }
        pr_info("%s(): CS start = 0x%lx, end = 0x%lx\n" // code segment
		"DS start = 0x%lx, end = 0x%lx\n"       // data segment
		"SS start = 0x%lx\n", __func__,         // stack segment
		mm->start_code, mm->end_code,
		mm->start_data, mm->end_data,
		mm->start_stack);

	mmap_read_unlock(mm);
}

static int __init mod_init(void)
{
        struct task_struct *task;
	pr_info("%s(): called\n", __func__);

        pr_info("%s(): provided process id: %d\n", __func__, pid_mem);
        for_each_process(task) {
                if (task->pid == pid_mem) {
                        pr_info("%s[%d]\n", task->comm, task->pid);
			print_mem(task->mm);
                }
        }
        return 0;
}

static void __exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with vma_list()");
