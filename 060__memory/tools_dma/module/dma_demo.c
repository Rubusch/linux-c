/*
  Consistent DMA Demo with instrumentation

  A variant using dma_alloc_coherent() and dma_free_coherent().

  This fixes actually the original idea represented in the referenced
  book of A. L. Rios (2018) which did not work out for me.

  ---
  REFERENCES:
  - John Linn (Xilinx), Linux DMA in Device Drivers, v3.14, https://www.youtube.com/watch?v=yJg-DkyH5CM
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
#include <linux/types.h> /* uint64_t */

#include <linux/of_device.h>  /* struct of_device_id */

/* private data structure to hold the miscdevice structure */
struct dma_private
{
	struct miscdevice dma_misc_device;
	struct device *dev;
	char *wbuf; // the wbuf and rbuf pointers will hold the..
	char *rbuf; // ...addresses of the allocated buffers
	dma_addr_t dma_src;
	dma_addr_t dma_dst;
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
  debug: instrumentation code

  Debugging dma and dma_addr_t items.
 */
static void
describe_dma(void* pdev_dev, dma_addr_t dma_addr, void* buf)
{
	struct device *dev = pdev_dev;

	dev_info(dev, "%s() - XXX called", __func__);

	/* dma_addr

	   virtual addresses (VA) equals the set value for ARM
	*/
	dev_info(dev, "%s() - XXX init dma_addr VA\t= 0x%llX",
		 __func__, (uint64_t) dma_addr);

	dev_info(dev, "%s() - XXX init dma_addr PA\t= 0x%llX",
		 __func__, (uint64_t) virt_to_phys(&dma_addr));

	/* buf

	   virtual addresses (VA) equals the set value for ARM
	 */
	dev_info(dev, "%s() - XXX init buf VA\t\t= 0x%llX",
		 __func__, (uint64_t) buf);

	dev_info(dev, "%s() - XXX init buf PA\t\t= 0x%llX",
		 __func__, (uint64_t) virt_to_phys(&buf));

	/* context

	   CONFIG_MMU: MMU-based Paged Memory Management Support
	*/
#ifdef CONFIG_MMU
	dev_info(dev, "%s() - XXX CONFIG_MMU is set", __func__);
#else
	dev_info(dev, "%s() - XXX CONFIG_MMU is NOT set", __func__);
#endif

	/* CONFIG_IOMMU: IOMMU support, i.e. we use IOVAs
	 */
#ifdef CONFIG_IOMMU
	dev_info(dev, "%s() - XXX CONFIG_IOMMU is set", __func__);
#else
	dev_info(dev, "%s() - XXX CONFIG_IOMMU is NOT set", __func__);
#endif

	dev_info(dev, "%s() - XXX dma_need_sync() = %s",
		 __func__, (dma_need_sync(dev, dma_addr)?"yes":"no"));

	dev_info(dev, "%s() - XXX dma_max_mapping_size() = %lld",
		 __func__, (uint64_t) dma_max_mapping_size(dev));

	dev_info(dev, "%s() - XXX dma_opt_mapping_size() = %lld",
		 __func__, (uint64_t) dma_opt_mapping_size(dev));

	dev_info(dev, "%s() - XXX dma_get_required_mask() = 0x%llX",
		 __func__, (uint64_t) dma_get_required_mask(dev));

	dev_info(dev, "%s() - XXX dma_get_merge_boundary() = 0x%llX",
		 __func__, (uint64_t) dma_get_merge_boundary(dev));

}


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

	if (*(dma_priv->rbuf) != *(dma_priv->wbuf))
		dev_err(dma_priv->dev, "%s() - buffer copy failed!", __func__);

	dev_info(dma_priv->dev, "%s() - wbuf = '%s'", __func__, dma_priv->wbuf);
	dev_info(dma_priv->dev, "%s() - rbuf = '%s'", __func__, dma_priv->rbuf);

	// signal the completion of the event inside this function
	complete(&dma_priv->dma_m2m_ok);
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
	int status;
	int ret;

	pr_info("%s() - called", __func__);

	// obtain the private data structure using container_of()
	dma_priv = container_of(file->private_data,
				struct dma_private,
				dma_misc_device);

	// obtain the dma_device
	dma_dev = dma_priv->dma_m2m_chan->device;

	// obtain data from userspace
	if (copy_from_user(dma_priv->wbuf, buf, count))
		return -EFAULT;
	ret = count;

	// obtain handle for dev_info()
	dev = dma_priv->dev;
	dev_info(dev, "%s() - the wbuf string is '%s' (initially)",
		 __func__, dma_priv->wbuf);
	dev_info(dev, "%s() - the rbuf string is '%s' (initially)",
		 __func__, dma_priv->rbuf);

	describe_dma(dma_priv->dev, dma_priv->dma_src, dma_priv->wbuf);   

	/* 3. DMA transaction: memcpy()

	   prepare a DMA memcpy transaction, then get a descriptor for
	   the transaction
	*/
	dev_info(dev, "%s() - 3. DMA transaction memcpy()", __func__);
	dma_m2m_desc = dma_dev->device_prep_dma_memcpy(dma_priv->dma_m2m_chan,
						       dma_priv->dma_dst,
						       dma_priv->dma_src,
						       SDMA_BUF_SIZE,
						       DMA_MEM_TO_MEM
						       | DMA_CTRL_ACK
						       | DMA_PREP_INTERRUPT);

	if (!dma_m2m_desc) {
		dev_err(dev, "%s() - transaction setup failed", __func__);
		ret = -EINVAL;
		goto err;
	}

	/* 4. setup a DMA callback

	   - called when operation done
	   - an alternative design might be to setup a regular
             completion, where the dmaengine API comes with the more
             specified callback appraoch
	*/
	dev_info(dev, "%s() - 4. setup a DMA completion", __func__);
	dma_m2m_desc->callback = dma_m2m_callback;
	dma_m2m_desc->callback_param = dma_priv;
	init_completion(&dma_priv->dma_m2m_ok);

	/* 5. Submit the DMA transaction

	   Submit the obtained descriptor with demaengine_submit()
	   then check the returned transaction cookie with
	   dma_submit_error()
	*/
	dev_info(dev, "%s() - 5. submit the DMA transaction", __func__);
	cookie = dmaengine_submit(dma_m2m_desc);
	if (dma_submit_error(cookie)) {
		dev_err(dev, "%s() - failed to submit DMA", __func__);
		ret = -EINVAL;
		goto err;
	}

	/*
	  6. Start DMA transaction

	  The transaction in the pending queue can be activated by
	  calling the issue_pending API. If the channel is idle, then
	  the first transaction in the queue is started and subsequent
	  ones queued up. On completion of each DMA operation, the
	  next in queue isstarted and a tasklet triggered. The tasklet
	  will then call the client driver's completion callback
	  routine for notification.

	  NB: This is the older dma_async... API
	*/
	dev_info(dev, "%s() - 6. start DMA transaction", __func__);
	dma_async_issue_pending(dma_priv->dma_m2m_chan);

	/* miscellaneous

	   - wait for the transaction: wait_for_completion(), or
	   - use alternative a timeout as here: wait_for_completion_timeout()
	   - check if the DMA status: dma_async_is_tx_complete()
	*/
	status = wait_for_completion_timeout(&dma_priv->dma_m2m_ok,
					     msecs_to_jiffies(2000));
	if (0 >= status) {
		dev_err(dev, "%s() - wait_for_completion() failed, or timeout",
			__func__);
		dmaengine_terminate_sync(dma_priv->dma_m2m_chan);
		ret = -ETIMEDOUT;
		goto err;
	}

	// check status...
	status = dma_async_is_tx_complete(dma_priv->dma_m2m_chan,
					  cookie, NULL, NULL);
	if (DMA_COMPLETE == status) {
		dev_info(dev, "%s() - dma transaction has completed: DMA_COMPLETE",
			 __func__);
	} else {
		dev_err(dev, "%s() - dma transaction did not complete: %d",
			__func__, status);
	}

	dev_info(dev, "%s() - wbuf = '%s'", __func__, dma_priv->wbuf);
	dev_info(dev, "%s() - rbuf = '%s'", __func__, dma_priv->rbuf);

	/* 7. Unmap DMA chunks

	   unmap the single chunks used for the dma transaction

	   NB: if we used dma_alloc_coherent() instead of kzalloc() in
	   the probe(), we probably would have needed an evaluation at
	   preparation of transaction and in case of failure
	   dma_free_coherent() the wbuf and rbuf memories,
	   respectively.
	 */
err:
	dev_info(dev, "%s() - 7. unmap DMA chunks", __func__);
	dma_free_coherent(dma_priv->dev, SDMA_BUF_SIZE,
			  dma_priv->wbuf, dma_priv->dma_src);
	dma_free_coherent(dma_priv->dev, SDMA_BUF_SIZE,
			  dma_priv->rbuf, dma_priv->dma_dst);

	return ret;
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

	// prepare a char device
	dma_priv = devm_kzalloc(&pdev->dev, sizeof(*dma_priv), GFP_KERNEL);
	if (!dma_priv) {
		dev_err(dev, "%s() - error allocating dma_priv structure", __func__);
		return -ENOMEM;
	}

	dma_priv->dma_misc_device.minor = MISC_DYNAMIC_MINOR;
	dma_priv->dma_misc_device.name = "sdma_test";
	dma_priv->dma_misc_device.fops = &dma_fops;

	dma_priv->dev = &pdev->dev;

	/* 0. Preparation: allocation and mapping: dma_alloc_coherent()

	   For huge memories use dma_alloc_coherent() which allocates
	   and maps DMA memory for unbuffered transactions
	*/
	dev_info(dev, "%s() - 0. preparation: dma_alloc_coherent()",  __func__);
	dma_priv->wbuf = dma_alloc_coherent(&pdev->dev, SDMA_BUF_SIZE,
					    &(dma_priv->dma_src), GFP_KERNEL);
	if (!dma_priv->wbuf) {
		dev_err(dev, "%s() - error allocating wbuf", __func__);
		return -ENOMEM;
	}
	dma_priv->rbuf = dma_alloc_coherent(&pdev->dev, SDMA_BUF_SIZE,
					    &(dma_priv->dma_dst), GFP_KERNEL);
	if (!dma_priv->rbuf) {
		dev_err(dev, "%s() - error allocating rbuf", __func__);
		return -ENOMEM;
	}

	/* 0. Preparation: specify DMA channel caps

	- specify cap: DMA_MEMCPY
	- alternatively specify a private channel:
            DMA_SLAVE | DMA_PRIVATE
	*/
	dev_info(dev, "%s() - 0. preparation: specify DMA channel caps",
		 __func__);
	dma_cap_zero(dma_m2m_mask);
	dma_cap_set(DMA_MEMCPY, dma_m2m_mask);
	// as alternative use the following for a private channel
//	dma_cap_set(DMA_SLAVE | DMA_PRIVATE, dma_m2m_mask);

	/* 1. Request DMA channel

	  the dma_request_channel() function takes three parameters
	  - the dma_m2m_mask, that holds the channel capabilities
	  - the m2m_dma_data, custom data structure
	  - the dma_m2m_filter that helps to select a more specific
            channel between multiple channel possibilities
	 */
	dev_info(dev, "%s() - 1. request DMA channel", __func__);
	dma_priv->dma_m2m_chan = dma_request_channel(dma_m2m_mask, 0, NULL);
	if (!dma_priv->dma_m2m_chan) {
		dev_err(dev, "%s() - error opening the memory to memory channel",
			__func__);
		return -EINVAL;
	}

	// register miscdevice structure (char dev) and init private data
	ret = misc_register(&dma_priv->dma_misc_device);
	platform_set_drvdata(pdev, dma_priv);

	return ret;
}

static int
lothars_remove(struct platform_device* pdev)
{
	struct dma_private *dma_priv = platform_get_drvdata(pdev);
	dev_info(dma_priv->dev, "%s() - called", __func__);
	misc_deregister(&dma_priv->dma_misc_device);
	dma_release_channel(dma_priv->dma_m2m_chan);

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
MODULE_DESCRIPTION("a consistent DMA demo");
