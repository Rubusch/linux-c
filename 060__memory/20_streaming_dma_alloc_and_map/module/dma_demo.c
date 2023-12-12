/*
  Streaming DMA Demo

  Using allocated memory, then mapped into DMA.

  ---
  REFERENCES:
  - John Linn (Xilinx), Linux DMA in Device Drivers, v3.14, https://www.youtube.com/watch?v=yJg-DkyH5CM
  - Linux Driver Development for Embedded Processors, A. L. Rios, 2018
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h> /* DMA mapping functions */

/*
  Functions needed to allocate a DMA slave channel, set slave and
  controller specific parameters, get a desriptor for transaction,
  submit the transaction , issue pending requests and wait for
  callback notification
 */
#include <linux/dmaengine.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

#include <linux/of_device.h>  /* struct of_device_id */

#include <linux/delay.h>

/* Private data structure to hold the miscdevice structure */
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
	   Completions are a lightweight mechanism with one task:
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
	struct dma_private *dma_priv = (struct dma_private *) data;
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

	// obtain a handle to the device for dev_info() and CO
	dev = dma_priv->dev;
	dev_info(dev, "%s() - the wbuf string is '%s' (initially)",
		 __func__, dma_priv->wbuf);
	dev_info(dev, "%s() - the rbuf string is '%s' (initially)",
		 __func__, dma_priv->rbuf);

	/* 2. DMA mapping

	   obtain source address using dma_map_single() setup a
	   dma_src and a dma_dst as a single mapped dma memory
	*/
	dev_info(dev, "%s() - 2. DMA mapping", __func__);

	// map DMA src
	dma_priv->dma_src = dma_map_single(dma_priv->dev,
				 dma_priv->wbuf,
				 SDMA_BUF_SIZE,
				 DMA_TO_DEVICE);
	if (dma_mapping_error(dma_priv->dev, dma_priv->dma_src)) {
		dev_err(dev, "%s() - mapping wbuf failed", __func__);
		goto err;
	}
	dev_info(dev, "%s() - dma_src map optained: 0x%08llX",
		 __func__, dma_priv->dma_src);

	// map DMA dst
	dma_priv->dma_dst = dma_map_single(dma_priv->dev,
				 dma_priv->rbuf,
				 SDMA_BUF_SIZE,
				 DMA_TO_DEVICE);
	if (dma_mapping_error(dma_priv->dev, dma_priv->dma_dst)) {
		dev_err(dev, "%s() - mapping rbuf failed", __func__);
		goto err;
	}
	dev_info(dev, "%s() - dma_dst map obtained: 0x%08llX",
		 __func__, dma_priv->dma_dst);

	/* 3. DMA transaction: memcpy()

	   prepare a DMA memcpy transaction, then get a descriptor for
	   the transaction
	*/
	dev_info(dev, "%s() - 3. DMA transaction memcpy()", __func__);
	dma_m2m_desc = dmaengine_prep_dma_memcpy(dma_priv->dma_m2m_chan,
						       dma_priv->dma_dst,
						       dma_priv->dma_src,
						       SDMA_BUF_SIZE,
						       DMA_MEM_TO_MEM | DMA_CTRL_ACK
						       | DMA_PREP_INTERRUPT);

	/* 4. setup a DMA completion

	   - called when operation done
	   - an alternative design might be to setup a regular
             completion, where the dmaengine API comes with the more
             specified callback appraoch
	*/
	dev_info(dev, "%s() - 4. setup a DMA completion", __func__);
	init_completion(&dma_priv->dma_m2m_ok);
	dma_m2m_desc->callback = dma_m2m_callback;
	dma_m2m_desc->callback_param = dma_priv;

	/* 5. submit the DMA transaction

	   Submit the obtained descriptor with demaengine_submit()
	   then check the returned transaction cookie with
	   dma_submit_error()
	*/
	dev_info(dev, "%s() - 5. submit the DMA transaction", __func__);
	cookie = dmaengine_submit(dma_m2m_desc);
	if (dma_submit_error(cookie)) {
		dev_err(dev, "%s() - failed to submit DMA", __func__);
		return -EINVAL;
	}

	/*
	  6. start DMA transaction

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


	/*
	  !!! CAUTION !!!

	  Issue: By observation of the author there are now situations
	  where most of the time the memcpy will work, but from time
	  to time it will fail. The completion works, and tells the
	  operation succeeded, but the dma_dst will be empty. What
	  happened?

	  Debug: The issue can be put away when placing a msleep()
	  before evaluation. Also, when injecting another time, the
	  old value is now written to the dma_dst. This smells like
	  buffering.
	  Thinking about how this is implemented and the debug
	  behavior, chances are that this has to do with the fact that
	  we are now buffered.

	  Since we allocated memory in the _probe(), and then mapped
	  the allocated wbuf/rbuf dma_map_single(), this seems not to
	  be the same as using dma_alloc_coherent() for allocating
	  unbuffered memory directly and mapping it.

	  Fix1: So, one fix might be to re-write the demo using
	  dma_alloc_coherent() instead.

	  Fix2: Another solution here, your author would recommend
	  using a sync to pass ownership to the device.
	*/
	if (true == dma_need_sync(dma_priv->dev, dma_priv->dma_dst)) {
		dev_info(dev, "%s() - !!! dma_priv->dma_dst device needs sync", __func__);
	} else {
		dev_info(dev, "%s() - dma_priv->dma_dst device is sync'd", __func__);
	}

	if (true == dma_need_sync(dma_priv->dev, dma_priv->dma_src)) {
		dev_info(dev, "%s() - !!! dma_priv->dma_src device needs sync", __func__);
	} else {
		dev_info(dev, "%s() - dma_priv->dma_src device is sync'd", __func__);
	}

	// sync options
//	dma_sync_single_for_cpu(dma_priv->dev, dma_priv->dma_dst,
//				SDMA_BUF_SIZE, DMA_BIDIRECTIONAL);
	dma_sync_single_for_cpu(dma_priv->dev, dma_priv->dma_dst,
				SDMA_BUF_SIZE, DMA_FROM_DEVICE);

	/* miscellaneous approachs here possible

	   - wait for the transaction: wait_for_completion(), or
	   - use alternative a timeout as here: wait_for_completion_timeout()
	   - check if the DMA status: dma_async_is_tx_complete()
	*/

	// wait on transaction...
	status = wait_for_completion_timeout(&dma_priv->dma_m2m_ok, msecs_to_jiffies(5000));
	if (0 >= status) {
		dev_err(dev, "%s() - wait_for_completion() failed, or timeout", __func__);

		dma_unmap_single(dma_priv->dev, dma_priv->dma_src, SDMA_BUF_SIZE, DMA_TO_DEVICE);
		dma_unmap_single(dma_priv->dev, dma_priv->dma_dst, SDMA_BUF_SIZE, DMA_TO_DEVICE);

		dmaengine_terminate_sync(dma_priv->dma_m2m_chan);
		return -ETIMEDOUT;
	}

	// check status...
	status = dma_async_is_tx_complete(dma_priv->dma_m2m_chan, cookie, NULL, NULL);
	if (DMA_COMPLETE == status) {
		dev_info(dev, "%s() - dma transaction has completed: DMA_COMPLETE", __func__);
	} else {
		dev_err(dev, "%s() - dma transaction did not complete: %d", __func__, status);
	}

	dmaengine_terminate_all(dma_priv->dma_m2m_chan);

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
	dev_info(dev, "%s() - 7. unmap DMA chunks", __func__);
	dma_unmap_single(dma_priv->dev, dma_priv->dma_src, SDMA_BUF_SIZE, DMA_TO_DEVICE);
	dma_unmap_single(dma_priv->dev, dma_priv->dma_dst, SDMA_BUF_SIZE, DMA_TO_DEVICE);

	return count;
err:
	// mapping error occured
	return -EINVAL;
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

	/* 0. Preparation: allocation

	   note, an alternative (typically) for dma here - if the
	   memory is bigger, use dma_alloc_coherent() which allocates
	   unbuffered. Unbuffered allocation makes sense if the memory
	   is huge and involves DMA. Be aware that unbuffering usually
	   and in most other cases would not be a good idea, since it
	   usually is rather slowing things down. Not so in the
	   described case above!
	*/
	dev_info(dev, "%s() - 0. preparation: allocation for DMA buffers", __func__);
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

	/* 0. Preparation: specify DMA channel caps

	- specify cap: DMA_MEMCPY
	- alternatively specify a private channel:
	  DMA_SLAVE | DMA_PRIVATE
	*/
	dev_info(dev, "%s() - 0. preparation: specify DMA channel caps", __func__);
	dma_cap_zero(dma_m2m_mask);
	dma_cap_set(DMA_SLAVE | DMA_PRIVATE, dma_m2m_mask);
	// alternative: be more specific for the capability
//	dma_cap_set(DMA_MEMCPY, dma_m2m_mask);

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
		dev_err(dev, "%s() - opening sDMA channel failed", __func__);
		return -EINVAL;
	}

	// char dev: register miscdevice structure and init private data
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
MODULE_DESCRIPTION("allocating memory and mapping it buffered into DMA");
