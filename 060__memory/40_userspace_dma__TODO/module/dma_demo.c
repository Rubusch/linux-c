// SPDX-License-Identifier: GPL-2.0+
/*
  DMA from Userspace

  ---
  REFERENCES:
  - Linux Driver Development for Embedded Processors, A. L. Rios, 2018
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h> /* DMA mapping functions */
#include <linux/dma-map-ops.h> /* dma_common_mmap() */
#include <linux/slab.h>
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
	dma_addr_t dma_src;
	dma_addr_t dma_dst;
};

/* set the buffer size */
#define SDMA_BUF_SIZE 4096
//#define SDMA_BUF_SIZE (1024*63)

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

static int
sdma_open(struct inode* inode, struct file* file)
{
	struct dma_private *dma_priv;
	dma_priv = container_of(file->private_data, struct dma_private,
				dma_misc_device);

	dma_priv->wbuf = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if (!dma_priv->wbuf) {
		dev_err(dma_priv->dev, "%s() - error allocating wbuf", __func__);
		return -ENOMEM;
	}

	dma_priv->rbuf = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if (!dma_priv->rbuf) {
		dev_err(dma_priv->dev, "%s() - error allocating rbuf", __func__);
		return -ENOMEM;
	}

	dma_priv->dma_src = dma_map_single(dma_priv->dev, dma_priv->wbuf,
					   SDMA_BUF_SIZE, DMA_TO_DEVICE);

	return 0;
}

/*
  ioctl function

  implements the trigger for importing the mapped memory from userspace
 */
static long
sdma_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
	struct dma_async_tx_descriptor *dma_m2m_desc;
	struct dma_device *dma_dev;
	struct dma_private *dma_priv;
	struct device *dev;
	dma_cookie_t cookie;
	int ret = 0;

	// obtain the private data structure using container_of()
	dma_priv = container_of(file->private_data, struct dma_private,
				dma_misc_device);

	// obtain the dma device
	dma_dev = dma_priv->dma_m2m_chan->device;

	// obtain device
	dev = dma_priv->dev;
	
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
	
	// obtain handle for printers
	dev_info(dev, "%s() - the wbuf string is '%s' (initially)",
		 __func__, dma_priv->wbuf);
	dev_info(dev, "%s() - the rbuf string is '%s' (initially)",
		 __func__, dma_priv->rbuf);

	/* 3. DMA transaction: memcpy
	 */
	dev_info(dev, "%s() - 3. DMA transaction memcpy", __func__);
	dma_m2m_desc = dma_dev->device_prep_dma_memcpy(dma_priv->dma_m2m_chan,
						       dma_priv->dma_dst,
						       dma_priv->dma_src,
						       SDMA_BUF_SIZE,
						       DMA_MEM_TO_MEM |  
						       DMA_CTRL_ACK
						       | DMA_PREP_INTERRUPT);

	if (!dma_m2m_desc) {
		dev_err(dev, "%s() - transaction setup failed", __func__);
		goto err;
	}

	/* 4. setup a DMA callback
	 */
	dev_info(dev, "%s() - 4. setup a DMA callback", __func__);
	dma_m2m_desc->callback = dma_m2m_callback;
	dma_m2m_desc->callback_param = dma_priv;
	init_completion(&dma_priv->dma_m2m_ok);

	/* 5. Submit the DMA transaction
	 */
	dev_info(dev, "%s() - 5. submit the DMA transaction", __func__);
	cookie = dmaengine_submit(dma_m2m_desc);
	if (dma_submit_error(cookie)) {
		dev_err(dev, "%s() - failed to submit DMA", __func__);
		ret = -EINVAL;
		return -EINVAL;
	}

	/* 6. Start DMA transaction
	 */
	dev_info(dev, "%s() - 6. start dma transaction", __func__);
	dma_async_issue_pending(dma_priv->dma_m2m_chan);

	// sync options
//	dma_sync_single_for_cpu(dma_priv->dev, dma_priv->dma_dst,
//				SDMA_BUF_SIZE, DMA_BIDIRECTIONAL);
	dma_sync_single_for_cpu(dma_priv->dev, dma_priv->dma_dst,
				SDMA_BUF_SIZE, DMA_FROM_DEVICE);


	/* miscellaneous
	 */

	// completion waiting
	wait_for_completion(&dma_priv->dma_m2m_ok);

	// check transaction cookie
	dma_async_is_tx_complete(dma_priv->dma_m2m_chan, cookie, NULL, NULL);

	// evaluate
	if (*(dma_priv->rbuf) != *(dma_priv->wbuf)) {
		dev_err(dev, "%s() - buffer copy failed", __func__);
		ret = -EINVAL;
		goto err;
	}

	dev_info(dev, "%s() - wbuf = '%s'", __func__, dma_priv->wbuf);
	dev_info(dev, "%s() - rbuf = '%s'", __func__, dma_priv->rbuf);

err:
	/* 7. Unmap DMA chunks
	 */
/*	 
	dev_info(dev, "%s() - 7. unmap DMA chunks", __func__);
	dma_free_coherent(dma_priv->dev, SDMA_BUF_SIZE,
			  dma_priv->wbuf, dma_priv->dma_src);
	dma_free_coherent(dma_priv->dev, SDMA_BUF_SIZE,
			  dma_priv->rbuf, dma_priv->dma_dst);
/*/
	dma_unmap_single(dma_priv->dev, dma_priv->dma_src,
			 SDMA_BUF_SIZE, DMA_TO_DEVICE);
	dma_unmap_single(dma_priv->dev, dma_priv->dma_dst,
			 SDMA_BUF_SIZE, DMA_TO_DEVICE);
// */
	return ret;
}

/*            

static struct page*
dma_common_vaddr_to_page(void *cpu_addr)
{
	if (is_vmalloc_addr(cpu_addr))
		return vmalloc_to_page(cpu_addr);
	return virt_to_page(cpu_addr);
}
// */

/*
  mmap implementation

  The mmap() implementation is the crucial part of the dma from
  userspace approach.

  There are mainly two ways of mapping in memory from userspace
  - remap_pfn_range(): the more generic approach, not specific to dma
  - dma_common_mmap(): the dma approach, but likely to not work out,
                       easily


  NB: This mapping function is not working anymore! Probably due to
  similar issues:

     The reason dma_addr_t is not same as virt_to_phys(dmabuffp) is
     because IOMMU is enabled.

     Only the device uses the dma_addr_t for DMA access. The processor
     accesses need to use the returned void*, or some sort of
     virt_to_phys() version of that to allow userspace to mmap it
     through devmem.

     Without an IOMMU, the dma_addr_t is simply a
     virt_to_phy()/virt_to_bus() translation of the void* buffer, so
     it can be used for processor access directly but it is incorrect
     usage of the DMA API.

  ref: https://askubuntu.com/questions/1140000/dma-alloc-coherent-and-physical-addresses
 */
//EXPORT_SYMBOL_GPL(dma_mmap_from_dev_coherent);   

//extern int dma_mmap_from_dev_coherent(struct device *dev, struct vm_area_struct *vma, void *cpu_addr, size_t size, int *ret);    

/*      
struct dma_coherent_mem {
	void		*virt_base;
	dma_addr_t	device_base;
	unsigned long	pfn_base;
	int		size;
	unsigned long	*bitmap;
	spinlock_t	spinlock;
	bool		use_dev_dma_pfn_offset;
};

static int __dma_mmap_from_coherent(struct dma_coherent_mem *mem,
		struct vm_area_struct *vma, void *vaddr, size_t size, int *ret)
{
	if (mem && vaddr >= mem->virt_base && vaddr + size <=
		   (mem->virt_base + ((dma_addr_t)mem->size << PAGE_SHIFT))) {
		unsigned long off = vma->vm_pgoff;
		int start = (vaddr - mem->virt_base) >> PAGE_SHIFT;
		unsigned long user_count = vma_pages(vma);
		int count = PAGE_ALIGN(size) >> PAGE_SHIFT;

		*ret = -ENXIO;
		if (off < count && user_count <= count - off) {
			unsigned long pfn = mem->pfn_base + start + off;
			*ret = remap_pfn_range(vma, vma->vm_start, pfn,
					       user_count << PAGE_SHIFT,
					       vma->vm_page_prot);
		}
		return 1;
	}
	return 0;
}

static inline struct dma_coherent_mem *dev_get_coherent_memory(struct device *dev)
{
	if (dev && dev->dma_mem)
		return dev->dma_mem;
	return NULL;
}

int dma_mmap_from_dev_coherent(struct device *dev, struct vm_area_struct *vma,
			   void *vaddr, size_t size, int *ret)
{
	struct dma_coherent_mem *mem = dev_get_coherent_memory(dev);

	return __dma_mmap_from_coherent(mem, vma, vaddr, size, ret);
}
// */       

static int
sdma_mmap(struct file* file, struct vm_area_struct *vma)
/*
{
	pgprot_t prot = vm_get_page_prot(vma->vm_flags);
//	struct dmabuf_file *priv = buf->priv;
	struct dma_private *dma_priv = container_of(file->private_data,
						    struct dma_private,
						    dma_misc_device);


//	vma->vm_flags |= VM_IO | VM_PFNMAP | VM_DONTEXPAND | VM_DONTDUMP;
//	vma->vm_ops = &dmabuf_vm_ops;
	vma->vm_private_data = dma_priv;
	vma->vm_page_prot = pgprot_writecombine(prot);

	return remap_pfn_range(vma, vma->vm_start,
//			       priv->phys >> PAGE_SHIFT,
			       dma_priv->dma_src >> PAGE_SHIFT,
			       vma->vm_end - vma->vm_start,
			       vma->vm_page_prot);

}
/*/
{
	struct dma_private *dma_priv = container_of(file->private_data,
						    struct dma_private,
						    dma_misc_device);

	unsigned long user_count = vma_pages(vma);
	unsigned long size =  vma->vm_end - vma->vm_start;
	unsigned long count = PAGE_ALIGN(size) >> PAGE_SHIFT;
	unsigned long off = vma->vm_pgoff;
//	struct page *page = dma_common_vaddr_to_page(&dma_priv->dma_src);
//	int ret = -ENXIO;

#ifdef CONFIG_MMU
	dev_info(dma_priv->dev, "%s() - CONFIG_MMU is set", __func__);
#else
	dev_info(dma_priv->dev, "%s() - CONFIG_MMU is NOT set", __func__);
	return ret;
#endif

#ifdef CONFIG_DMA_DECLARE_COHERENT
	dev_info(dma_priv->dev, "%s() - CONFIG_DMA_DECLARE_COHERENT is set",
		 __func__);
#else
	dev_info(dma_priv->dev, "%s() - CONFIG_DMA_DECLARE_COHERENT is NOT set",
		 __func__);
#endif

	dev_info(dma_priv->dev, "%s() - XXX user_count '%ld'", __func__, user_count);
	dev_info(dma_priv->dev, "%s() - XXX size '%ld'", __func__, size);
	dev_info(dma_priv->dev, "%s() - XXX count '%ld'", __func__, count);
	dev_info(dma_priv->dev, "%s() - XXX off '%ld'", __func__, off);

// TODO attrs    
// TODO linker failed    
//	vma->vm_page_prot = dma_pgprot(dma_priv->dev, vma->vm_page_prot, attrs);
//	vma->vm_page_prot = dma_pgprot(dma_priv->dev, vma->vm_page_prot, 0);

// TODO linker failed    
//	if (dma_mmap_from_dev_coherent(dma_priv->dev, vma,
//				       &dma_priv->dma_src, size, &ret))
//		return ret;

//	if (off >= count || user_count > count - off)
//		return ret;

// // NB: not allowed anymore
//	if (dma_common_mmap(dma_priv->dev, vma,
//			    &vma->vm_start,
//			    dma_priv->dma_src,
//			    vma->vm_end - vma->vm_start,
//			    0)
//		) {
//		return -EAGAIN;
//	}

	// current dma approach would be with dma_map_coherent()
	// FIXME ensure alignment
	return dma_mmap_coherent(dma_priv->dev,
				 vma,
				 &vma->vm_start,
//				 virt_to_phys(&dma_priv->dma_src) >> PAGE_SHIFT,
				 dma_priv->dma_src,
				 vma->vm_end - vma->vm_start);

	// general mapping pfn approach, not sufficient for dma mappings
//	return remap_pfn_range(vma, vma->vm_start,
////			    virt_to_phys(&dma_priv->dma_src) >> PAGE_SHIFT,  
//			    page_to_pfn(page) + vma->vm_pgoff,
//			    user_count << PAGE_SHIFT,
////			    vma->vm_end - vma->vm_start,  
//			    vma->vm_page_prot);
}
// */

struct file_operations dma_fops = {
	.open = sdma_open,
	.unlocked_ioctl = sdma_ioctl,
	.mmap = sdma_mmap,
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

	// prepare char device
	dma_priv = devm_kzalloc(&pdev->dev, sizeof(*dma_priv), GFP_KERNEL);
	if (!dma_priv) {
		dev_err(dev, "%s() - error allocating dma_priv structure",
			__func__);
		return -ENOMEM;
	}

	dma_priv->dma_misc_device.minor = MISC_DYNAMIC_MINOR;
	dma_priv->dma_misc_device.name = "sdma_test";
	dma_priv->dma_misc_device.fops = &dma_fops;

	dma_priv->dev = &pdev->dev;

	/* 0. Preparation: allocate buffers coherent

	   - allocates and maps DMA
	   - unbuffered access (better for huge pieces of memory)
	   NB: using the pdev->dev for the *device here!
	 */
/*                  
	dev_info(dev, "%s() - 0. preparation: allocate coherent", __func__);
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
/*/
	// streaming mapping
	dev_info(dev, "%s() - 0. preparation: allocation for DMA buffers",
		 __func__);
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
// */


	/* 0. Preparation: specify DMA channel caps
	 */
	dev_info(dev, "%s() - 0. preparation: specify DMA channel caps",
		 __func__);
	dma_cap_zero(dma_m2m_mask);
	dma_cap_set(DMA_SLAVE | DMA_PRIVATE, dma_m2m_mask);
	// alternatively only define DMA_MEMCPY capability
//	dma_cap_set(DMA_MEMCPY, dma_m2m_mask);

	/* 1. Request DMA channel

	  The dma_request_channel() function takes three parameters
	  - the dma_m2m_mask, that holds the channel capabilities
	  - the m2m_dma_data, custom data structure
	  - the dma_m2m_filter that helps to select a more specific
            channel between multiple channel possibilities
	 */
	dev_info(dev, "%s() - 1. request DMA channel", __func__);
	dma_priv->dma_m2m_chan = dma_request_channel(dma_m2m_mask, 0, NULL);
	if (!dma_priv->dma_m2m_chan) {
		dev_err(dev, "%s() - opening DMA channel failed", __func__);
		return -EINVAL;
	}

	// register miscdevice structure (char dev) and init private data
	ret = misc_register(&dma_priv->dma_misc_device);
	platform_set_drvdata(pdev, dma_priv);

	dev_info(dev, "%s() - PAGE_SHIFT '%d'", __func__, PAGE_SHIFT);

	return ret;
}

static int
lothars_remove(struct platform_device* pdev)
{
	struct dma_private *dma_priv = platform_get_drvdata(pdev);
	dev_info(dma_priv->dev, "%s() - platform_remove_enter", __func__);
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

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("a streaming DMA demo");
