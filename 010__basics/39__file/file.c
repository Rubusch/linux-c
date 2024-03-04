// SPDX-License-Identifier: GPL-2.0+
/*
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

#define FILENAME "/tmp/lothars_happy_file"

static int __init mod_init(void)
{
	struct file *filp;
	char data[128] = "....ooooOOOO0000*\n";
	int ret;

	pr_info("%s(): called\n", __func__);

	// open a file
	filp = filp_open(FILENAME, O_RDWR | O_CREAT, 0644);
	if (!filp) {
		pr_err("%s(): failed to open file\n", __func__);
		return -EFAULT;
	}

	// write
	ret = kernel_write(filp, data, sizeof(data), &filp->f_pos);
	if (0 > ret) {
		pr_err("%s(): failed to write: %d\n", __func__, ret);
		goto err;
	}
	pr_info("%s(): wrote %d bytes to file\n", __func__, ret);

	// read
	memset(data, 0, sizeof(data));
	filp->f_pos = 0;

	ret = kernel_read(filp, data, sizeof(data), &filp->f_pos);
	if (0 > ret) {
		pr_err("%s(): failed to read file: %d\n", __func__, ret);
		goto err;
	}

	pr_info("%s(): read %d bytes: '%s'\n", __func__, ret, data);
	ret = 0;
err:
	filp_close(filp, NULL);
	return ret;
}

static void __exit mod_exit(void)
{
	pr_info("%s(): called\n", __func__);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with files.");
