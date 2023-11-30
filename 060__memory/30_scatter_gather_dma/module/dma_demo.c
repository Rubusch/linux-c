// SPDX-License-Identifier: GPL-2.0+
/*
  Scatter/Gather DMA Demo


  scatterlist setup approach
  1. struct scatterlist *sg;
  2. sg_init_table(sg, nents);

  3. sg_set_buf(sg, buf, buflen)
     or sg_set_page(...)
     // prepare sg with content

  4. dma_map_sg(dev, sg, nents, dir)
     // map before dma transaction

  5. dma_unmap_sg(dev, sg, nents, dir)
     // in remove


  dma slave setup approach
  NB: "slave" usually means "peripheral DMA capable device"

  1. chan = dma_request_chan(dev, name)
     // in probe()

  2. dmaengine_slave_config(chan, slave_config)
     // in probe(), prepare scatterlist sg

  3. desc = dmaengine_prep_slave_sg(chan, sg, num, dir)
     // also other slave modes are possible here:
     // - dma_cyclic
     // - interleaved_dma
     // - slave_sg

  4. cookie = dmaengine_submit(desc)
     // opt: check cookie

  5. dma_async_issue_pending(chan)

  ---
  REFERENCES:
  - kernel.org / documentation
  - https://forums.raspberrypi.com/viewtopic.php?t=130160
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h> /* DMA mapping functions */

/*
  functions needed to allocate a DMA slave channel, set slave and
  controller specific parameters, get a desriptor for transaction,
  submit the transaction , issue pending requests and wait for
  callback notification
 */
#include <linux/dmaengine.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

#include <linux/of_device.h>  /* struct of_device_id */

// the device simulation
static char *dma_buf_src;
static char *dma_buf_dst;
static struct scatterlist sg_device[1];

struct dma_private
{
	struct miscdevice dma_misc_device;
	struct device *dev;
	struct scatterlist sg_cpu[1];
	struct dma_chan *dma_m2m_chan;
	struct completion dma_m2m_ok;
};

#define SDMA_BUF_SIZE 4096

static void
dma_sg_callback(void* data)
{
	struct dma_private *dma_priv = data;
	dev_info(dma_priv->dev, "%s() - finished SG DMA transaction",
		 __func__);
	complete(&dma_priv->dma_m2m_ok);
}

/*
  This function is a total DMA mock - usually here we would connect
  DMA_DEV_TO_MEM or DMA_MEM_TO_DEV channels, in our case we have
  rather something like DMA_MEM_TO_MEM, and even nor particular
  dmaengine implementation.

  To "mock" the dmaengine, here just copy over as if it were a single
  mapping.
*/
struct dma_async_tx_descriptor*
lothars_prep_slave_sg( struct dma_chan *chan, struct scatterlist *sgl,
		       unsigned int sg_len,
		       enum dma_transfer_direction dir, unsigned long flags)
{
	dma_addr_t dma_src = 0;
	dma_addr_t dma_dst = 0;

	if (!chan || !chan->device || !chan->device->device_prep_slave_sg)
		return NULL;

	dma_src = sg_dma_address(sgl);

	// HACK: device simulation
	dma_dst = sg_dma_address(sg_device);

	return chan->device->device_prep_dma_memcpy(chan, dma_dst,
						    dma_src, SDMA_BUF_SIZE,
						    flags);
}

/*
  Initialize scatterlist arrays with sg_init_table() and set each
  entry in the allocated arrays with sg_set_buf() to point at given
  data. The exact DMA direction should be provided: DMA_MEM_TO_MEM.
  Map the scatterlist to get the DMA addresses by using the
  dma_map_sg().

  Get the channel descriptors, add DMA operations to the pending
  queue, issue pending DMA requests and set a wait for callback
  notifications. After each DMA transaction, compare the values of the
  transmission and reception buffers.
 */
static ssize_t
sdma_write(struct file* file, const char __user* buf, size_t count, loff_t* offset)
{
	unsigned int sg_len = 1, n_sg_cpu = 0, n_sg_device = 0;
	struct dma_async_tx_descriptor *dma_m2m_desc;
	struct dma_device *dma_dev;
	struct dma_private *dma_priv;
	struct device *dev;
	dma_cookie_t cookie;

	pr_info("%s() - called", __func__);

	// obtain the private data structure using container_of()
	dma_priv = container_of(file->private_data,
			       struct dma_private,
			       dma_misc_device);

	// obtain the dma_device
	dma_dev = dma_priv->dma_m2m_chan->device;

	if (copy_from_user(dma_buf_src, buf, count))
		return -EFAULT;

	dev = dma_priv->dev;
	dev_info(dev, "%s() - the dma_buf_src string is '%s'",
		 __func__, dma_buf_src);


	// scatterlist
	dev_info(dev, "%s() - scatterlist setup", __func__);
        // NB: dma_buf_src should be coherent (TODO verify)
	sg_init_table(dma_priv->sg_cpu, sg_len);
	sg_set_buf(dma_priv->sg_cpu, dma_buf_src, SDMA_BUF_SIZE);

	sg_init_table(sg_device, sg_len);
	sg_set_buf(sg_device, dma_buf_dst, SDMA_BUF_SIZE);


	// mapping
	dev_info(dev, "%s() - mapping", __func__);
	// NB: our issue here: "slave" is usually a piece of dma hardware!
	// in M2M dma, this won't be a device, and later down neither a "slave"
	n_sg_cpu = dma_map_sg(dev, dma_priv->sg_cpu, sg_len, 0);// DMA_TO_DEVICE);
	if (0 == n_sg_cpu) {
		dev_err(dev, "%s() - mapping sg_cpu to dma failed", __func__);
		return -EINVAL;
	}

	n_sg_device = dma_map_sg(dev, sg_device, sg_len, 0); //DMA_FROM_DEVICE);
	if (0 == n_sg_device) {
		dev_err(dev, "%s() - mapping sg_device to dma failed", __func__);
		return -EINVAL;
	}

	// operation

        // NB: mock implementation of dmaengine_prep_slave_sg()
	// "slave" refers to dma hardware, since this is just
	// a small scatterlist / dma demo, this function here will
	// be mocked
	dma_m2m_desc = lothars_prep_slave_sg(
		dma_priv->dma_m2m_chan, dma_priv->sg_cpu, sg_len,
		DMA_MEM_TO_MEM, DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!dma_m2m_desc) {
		dev_err(dev, "%s() - m2m operation failed", __func__);
		return -ENOMEM;
	}

	dev_info(dev, "%s() - init completion", __func__);
	dma_m2m_desc->callback = dma_sg_callback;
	dma_m2m_desc->callback_param = dma_priv;
	init_completion(&dma_priv->dma_m2m_ok);

	// dmaengine_submit()
	dev_info(dev, "%s() - call dmaengine_submit()", __func__);
	cookie = dmaengine_submit(dma_m2m_desc);
	if (dma_submit_error(cookie)) {
		dev_err(dev, "%s() - failed to submit DMA", __func__);
		return -EINVAL;
	}

	// dma_async_issue_pending() - finish up
	dev_info(dev, "%s() - call dma_async_issue_pending()", __func__);
	dma_async_issue_pending(dma_priv->dma_m2m_chan);
	wait_for_completion(&dma_priv->dma_m2m_ok);
	dma_async_is_tx_complete(dma_priv->dma_m2m_chan, cookie, NULL, NULL);

	dev_info(dev, "%s() - the device can read '%s'",
		 __func__, dma_buf_dst);

	// unmap scatterlist
	dma_unmap_sg(dev, dma_priv->sg_cpu, sg_len, DMA_TO_DEVICE);
	dma_unmap_sg(dev, sg_device, sg_len, DMA_TO_DEVICE);


	if (*(dma_buf_src) != *(dma_buf_dst)) {
		pr_info("%s() - buffer copy failed", __func__);
		return -EINVAL;
	}
	pr_info("%s() - buffer coherent sg copy successful", __func__);
	pr_info("%s() - dma_buf_src is '%s'", __func__, dma_buf_src);

	return count;
}

struct file_operations dma_fops = {
	write: sdma_write,
};

static int
lothars_probe(struct platform_device *pdev)
{
	int ret;
	struct dma_private *dma_priv;
	dma_cap_mask_t dma_m2m_mask;
	struct dma_slave_config dma_m2m_config = {0};
	struct device *dev = &pdev->dev;

	dev_info(dev, "%s() - called", __func__);

	dma_priv = devm_kzalloc(&pdev->dev, sizeof(*dma_priv), GFP_KERNEL);
	if (!dma_priv) {
		dev_err(dev, "%s() - allocating dma_riv failed", __func__);
		return -ENOMEM;
	}

	dma_priv->dma_misc_device.minor = MISC_DYNAMIC_MINOR;
	dma_priv->dma_misc_device.name = "sdma_test";
	dma_priv->dma_misc_device.fops = &dma_fops;

	dma_priv->dev = &pdev->dev;

	// NB: the kzalloc() is sufficient for dma_buf_src, if
	// dma was already mapped and we had some dma_addr_t values
	// here, we could use dma_alloc_coherent() and
	// dma_free_coherent() in the remove() function
	dma_buf_src = devm_kzalloc(&pdev->dev, SDMA_BUF_SIZE, GFP_ATOMIC);
	if (!dma_buf_src) {
		dev_err(dev, "%s() - failed allocating wbuf", __func__);
		return -ENOMEM;
	}

	dma_buf_dst = devm_kzalloc(&pdev->dev, SDMA_BUF_SIZE, GFP_ATOMIC);
	if (!dma_buf_dst) {
		dev_err(dev, "%s() - failed allocating dma_buf_dst", __func__);
		return -ENOMEM;
	}

	dma_cap_zero(dma_m2m_mask);
	dma_cap_set(DMA_MEMCPY, dma_m2m_mask);

	// NB: instead use here dma_request_slave_channel(<dev>, <name>);
	dma_priv->dma_m2m_chan = dma_request_channel(dma_m2m_mask, 0, NULL);
	if (!dma_priv->dma_m2m_chan) {
		dev_err(dev, "%s() - opening the DMA channel failed", __func__);
		return -EINVAL;
	}

	// register miscdevice
	ret = misc_register(&dma_priv->dma_misc_device);
	platform_set_drvdata(pdev, dma_priv);
	dev_info(dev, "%s() - done", __func__);

	// setup config and init dma slaves
	// (in case of a real dma device involved, this slave config
	// should be prepared)
	dma_m2m_config.direction = DMA_MEM_TO_MEM;
	dma_m2m_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	ret = dmaengine_slave_config(dma_priv->dma_m2m_chan, &dma_m2m_config);
	if (ret) {
		dev_err(dev, "%s() - failed to set slave configuration: %d\n",
			__func__, ret);
		return -EINVAL;
	}

	return ret;
}

static int
lothars_remove(struct platform_device *pdev)
{
	struct dma_private *dma_priv = platform_get_drvdata(pdev);
	dev_info(dma_priv->dev, "%s() - called", __func__);
	misc_deregister(&dma_priv->dma_misc_device);
	dma_release_channel(dma_priv->dma_m2m_chan);
	dev_info(dma_priv->dev, "%s() - done", __func__);
	return 0;
}

static const struct of_device_id lothars_of_ids[] = {
	{ .compatible = "lothars,sdma_m2m" },
	{},
};
MODULE_DEVICE_TABLE(of, lothars_of_ids);

static struct platform_driver lothars_driver = {
	.probe = lothars_probe,
	.remove = lothars_remove,
	.driver = {
		.name = "smda_m2m",
		.of_match_table = lothars_of_ids,
	}
};
module_platform_driver(lothars_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("a scatter/gather DMA demo");
