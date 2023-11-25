/*
  Streaming DMA Demo (m2m DMA is considered to be async)

  ---
  REFERENCES:
  - Linux Driver Development for Embedded Processors, A. L. Rios, 2018
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

/* private data structure to hold the miscdevice structure */
struct dma_private
{
	struct miscdevice dma_misc_device;
	struct device *dev;
	char *wbuf; // the wbuf and rbuf pointers will hold the..
	char *rbuf; // ...addresses of the allocated buffers
	struct dma_chan *dma_m2m_chan;
	/*
	   completions are a lightweight mechanism with one task:
	   allowing one thread to tell another that the job is
	   done. The advantage of using completions (over e.g. a
	   semaphore/mutex based approach) is to generate more
	   efficient code as both threads can continue until the
	   result is actually needed.
	*/
	struct completion dma_m2m_ok;
};

/* set the buffer size */
#define SDMA_BUF_SIZE 4096

/*
  Callback notification handling

  Create a callback function to inform about the completion of the DMA
  transaction.
 */
static void
dma_m2m_callback(void *data)
{
	struct dma_private *dma_priv = data;
	dev_info(dma_priv->dev, "%s() - called", __func__);

	// signal the completion of the event inside this function
	complete(&dma_priv->dma_m2m_ok);

	if (*(dma_priv->rbuf) != *(dma_priv->wbuf))
		dev_err(dma_priv->dev, "%s() - buffer copy failed!", __func__);

	dev_info(dma_priv->dev, "%s() - buffer copy passed!", __func__);
	dev_info(dma_priv->dev, "%s() - wbuf = '%s'", __func__, dma_priv->wbuf);
	dev_info(dma_priv->dev, "%s() - rbuf = '%s'", __func__, dma_priv->rbuf);
}

/*
  The sdma_write() function to communicate with the userspace

  This function gets the characters written to the char device by
  using copy_from_user() and store them in the wbuf buffer. The
  dma_src and dma_dst DMA addresses are obtained by using the
  dma_map_single() function, which takes the wbuf and rbuf addresses
  as parameters, previously obtained in the probe() and stored in the
  DMA private structure. These virtual addresses are retrieved in
  sdma_write() by using the container_of() function.
 */
static ssize_t
sdma_write(struct file* file, const char __user* buf, size_t count, loff_t* offset)
{
	struct dma_async_tx_descriptor *dma_m2m_desc;
	struct dma_device *dma_dev;
	struct dma_private *dma_priv;
	struct device *dev;
	dma_cookie_t cookie;
	dma_addr_t dma_src;
	dma_addr_t dma_dst;

	pr_info("%s() - called", __func__);

	// obtain the private data structure using container_of()
	dma_priv = container_of(file->private_data,
				struct dma_private,
				dma_misc_device);

	// obtain the dma_device
	dma_dev = dma_priv->dma_m2m_chan->device;

	if (copy_from_user(dma_priv->wbuf, buf, count))
		return -EFAULT;

	dev = dma_priv->dev;
	dev_info(dev, "%s() - the wbuf string is '%s'",
		 __func__, dma_priv->wbuf);

	// mapping
	//
	// obtain source address using dma_map_single()
	// setup a dma_src and a dma_dst as a single mapped dma memory
	dma_src = dma_map_single(dma_priv->dev,
				 dma_priv->wbuf,
				 SDMA_BUF_SIZE,
				 DMA_TO_DEVICE);
	dev_info(dev, "%s() - dma_src map optained", __func__);

	dma_dst = dma_map_single(dma_priv->dev,
				 dma_priv->rbuf,
				 SDMA_BUF_SIZE,
				 DMA_TO_DEVICE);
	dev_info(dev, "%s() - dma_dst map obtained", __func__);

	// operation
	//
	// perform a memcpy, then get a descriptor for the transaction
	dma_m2m_desc = dma_dev->device_prep_dma_memcpy(dma_priv->dma_m2m_chan,
						       dma_dst,
						       dma_src,
						       SDMA_BUF_SIZE,
						       DMA_CTRL_ACK
						       | DMA_PREP_INTERRUPT);

	dev_info(dev, "%s() - successful descriptor obtained", __func__);

	// setup the dma_callback (called when operation done)
	dma_m2m_desc->callback = dma_m2m_callback;
	dma_m2m_desc->callback_param = dma_priv;
	init_completion(&dma_priv->dma_m2m_ok);

	// submit the obtained descriptor with demaengine_submit()
	// then check the returned transaction cookie with dma_submit_error()
	cookie = dmaengine_submit(dma_m2m_desc);
	if (dma_submit_error(cookie)) {
		dev_err(dev, "%s() - failed to submit DMA", __func__);
		return -EINVAL;
	}

	/*
	  dma_async_issue_pending()

	  The transaction in the pending queue can be activated by
	  calling the issue_pending API. If the channel is idle, then
	  the first transaction in the queue is started and subsequent
	  ones queued up. On completion of each DMA operation, the
	  next in queue isstarted and a tasklet triggered. The tasklet
	  will then call the client driver's completion callback
	  routine for notification.
	*/
	dma_async_issue_pending(dma_priv->dma_m2m_chan);

	wait_for_completion(&dma_priv->dma_m2m_ok);
	dma_async_is_tx_complete(dma_priv->dma_m2m_chan, cookie, NULL, NULL);

	dev_info(dev, "%s() - the rbuf string is '%s'", __func__, dma_priv->rbuf);

	dma_unmap_single(dma_priv->dev, dma_src, SDMA_BUF_SIZE, DMA_TO_DEVICE);
	dma_unmap_single(dma_priv->dev, dma_dst, SDMA_BUF_SIZE, DMA_TO_DEVICE);

	return count;
}

struct file_operations dma_fops = {
	write: sdma_write,
};

/*
  Once the channel is obtained, you have to configure it by filling
  the dma_slave_config structure with the proper values to do a DMA
  transaction. Most of the generic information that a slave DMA can
  use, is included in this dma_slave_config structure. It allows the
  clients to specify DMA direction, DMA addresses, bus widths, DMA
  burst lengths, etc. If some DMA controllers have more parameters to
  be sent, then they should try to embed dma_slave_config in their
  controller specific structure, providing flexibility to pass more
  parameters if required.
 */
static int
lothars_probe(struct platform_device* pdev)
{
	int ret;
	struct dma_private *dma_priv;
	dma_cap_mask_t dma_m2m_mask;
	struct device *dev = &pdev->dev;

	dev_info(dev, "%s() - called", __func__);

	dma_priv = devm_kzalloc(&pdev->dev, sizeof(*dma_priv), GFP_KERNEL);
	if (!dma_priv) {
		dev_err(dev, "%s() - error allocating dma_priv structure", __func__);
		return -ENOMEM;
	}

	dma_priv->dma_misc_device.minor = MISC_DYNAMIC_MINOR;
	dma_priv->dma_misc_device.name = "sdma_test";
	dma_priv->dma_misc_device.fops = &dma_fops;

	dma_priv->dev = &pdev->dev;

	dma_priv->wbuf = devm_kzalloc(&pdev->dev, SDMA_BUF_SIZE, GFP_KERNEL);
	if (!dma_priv->wbuf) {
		dev_err(dev, "%s() - error allocating wbuf", __func__);
		return -ENOMEM;
	}

	dma_priv->rbuf = devm_kzalloc(&pdev->dev, SDMA_BUF_SIZE, GFP_KERNEL);
	if (!dma_priv->rbuf) {
		dev_err(dev, "%s() - error allocating rbuf", __func__);
		return -ENOMEM;
	}

	dma_cap_zero(dma_m2m_mask);
	dma_cap_set(DMA_MEMCPY, dma_m2m_mask);

	/*
	  the dma_request_channel() function takes three parameters
	  - the dma_m2m_mask, that holds the channel capabilities
	  - the m2m_dma_data, custom data structure
	  - the dma_m2m_filter that helps to select a more specific
            channel between multiple channel possibilities
	 */
	dma_priv->dma_m2m_chan = dma_request_channel(dma_m2m_mask, 0, NULL);
	if (!dma_priv->dma_m2m_chan) {
		dev_err(dev, "%s() - error opening the sDMA memory to memory channel", __func__);
		return -EINVAL;
	}

	// register miscdevice structure and init private data
	ret = misc_register(&dma_priv->dma_misc_device);
	platform_set_drvdata(pdev, dma_priv);
	dev_info(dev, "%s() - done", __func__);

	return ret;
}

static int
lothars_remove(struct platform_device* pdev)
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

static struct platform_driver lothars_platform_driver = {
	.probe = lothars_probe,
	.remove = lothars_remove,
	.driver = {
		.name = "sdma_m2m",
		.of_match_table = lothars_of_ids,
	},
};
module_platform_driver(lothars_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("a streaming DMA demo");
