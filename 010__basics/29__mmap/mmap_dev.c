// SPDX-License-Identifier: GPL-2.0+
/*
  The demo shows the classic/"by foot" way how to deal with mapping,
  paging and some conversions, which are nowadays covered by kernel
  APIs.
*/
#include <linux/module.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <asm/io.h>

#include <linux/miscdevice.h>

static void *happy_data;

/*
  The johannes4linux way to implement read() and write() - a good
  illustration!
*/
static ssize_t
chardev_read(struct file *file, char __user *ubuf, size_t len, loff_t *offs)
{
	int not_copied, to_copy = (len > PAGE_SIZE) ? PAGE_SIZE : len;

	pr_info("%s(): called\n", __func__);
	not_copied = copy_to_user(ubuf, happy_data, to_copy);
	return to_copy - not_copied;
}

/*
  The johannes4linux way to implement read() and write() - a good
  illustration!
*/
static ssize_t
chardev_write(struct file *file, const char __user *ubuf,
			size_t len, loff_t *offs)
{
	int not_copied, to_copy = (len > PAGE_SIZE) ? PAGE_SIZE : len;

	pr_info("%s(): called\n", __func__);
	not_copied = copy_from_user(happy_data, ubuf, to_copy);
	return to_copy - not_copied;
}

static int
chardev_mmap(struct file *file, struct vm_area_struct *vma)
{
	int ret;

	pr_info("%s(): called\n", __func__);

	// page shift arithmetic (for non dma stuff)
	vma->vm_pgoff = virt_to_phys(happy_data) >> PAGE_SHIFT;

	// then remap page frame number
	ret = remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
				 vma->vm_end - vma->vm_start, vma->vm_page_prot);
	if (ret) {
		pr_err("%s(): failed to remap_pfn_range()\n", __func__);
		return -EAGAIN;
	}
	return 0;
}

static struct file_operations fops = {
	.read = chardev_read,
	.write = chardev_write,
	.mmap = chardev_mmap,
};

static struct miscdevice mmap_dev = {
	.name = MMAP_DEV_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

int __init mod_init(void)
{
	int ret;

	pr_info("%s(): called\n", __func__);

	happy_data = kzalloc(PAGE_SIZE, GFP_DMA);
	if (!happy_data) {
		pr_err("%s(): failed to alloc memory\n", __func__);
		return -ENOMEM;
	}

	pr_info("%s(): allocated a page (%ld bytes)\n", __func__, PAGE_SIZE);
	pr_info("%s(): PAGE_SHIFT: %d\n", __func__, PAGE_SHIFT);

	ret = misc_register(&mmap_dev);
	if (0 != ret) {
		pr_err("%s(): failed to register miscdevice\n", __func__);
		kfree(happy_data);
		return -EFAULT;
	}

	return 0;
}

void __exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);
	if(happy_data)
		kfree(happy_data);
	misc_deregister(&mmap_dev);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing mmap calls");
