//sbull.c Source Code :
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/vmalloc.h>

#define KERNEL_SECTOR_SIZE 512
// define sector size is 512 in kernel
// HARDWARE define (ramdisk size = 20000*512 = 10M bytes)
int nsectors = 20000;
// number of secotrs
int hardsector_size = 512; 
// sector size = 512 bytes
int dev_major_nr = 0;
// system automatically allocate major number
int SBULL_MINORS = 1; 
// MAX minor number is 1 ,ie.only one partition
char dev_name[8] = "sbull";
MODULE_AUTHOR("wisemark");
MODULE_LICENSE("Dual BSD/GPL");

struct sbull_dev
{
	unsigned long
	size;
	unsigned char 
	*data;
	spinlock_t
	lock;
	struct gendisk *gd;
	struct block_device *bdev;
}Device;
static struct sbull_dev *dev = &Device;

// function prototype
static int sbull_open(struct inode *inode, struct file *filp);
static int sbull_release(struct inode *inode, struct file *filp);
static void sbull_transfer(struct sbull_dev *dev, unsigned long sector,
unsigned long nsect, char *buffer, int write); 
static void sbull_request(request_queue_t *q);

static struct block_device_operations sbull_ops =
{
	.owner = THIS_MODULE,
	.open = sbull_open,
	.release = sbull_release
};

static int sbull_open(struct inode *inode, struct file *filp)
{
	printk("sbull_open(): do nothing...\n");
	return 0;
}

static int sbull_release(struct inode *inode, struct file *filp)
{
	printk("sbull_release(): do nothing...\n");
	return 0;
}
 
static int sbull_init(void)
{
	printk("sbull_init(): initial sbull...\n");
	
	// initial struct sbull_dev
	dev_major_nr = register_blkdev(dev_major_nr,dev_name);
	if (dev_major_nr <= 0)
	{
		printk("sbull_init(): unable to get major number\n");
		return 0;
	}
	memset(dev, 0, sizeof(struct sbull_dev));
	dev->size = nsectors * hardsector_size;
	dev->data = vmalloc(dev->size);
	if (dev->data == NULL)
	{
		printk("sbull_init(): vmalloc failure.\n");
		return 0;
	}
	
	// initial spinlock
	spin_lock_init(&dev->lock);
	
	// initial gendisk struct *gd
	dev->gd = alloc_disk(SBULL_MINORS); 
	if (!dev->gd)
	{
		printk("sbull_init(): allocate disk failure.\n");
		return 0;
	}
	dev->gd->major = dev_major_nr;
	dev->gd->first_minor = 0;
	dev->gd->fops = &sbull_ops;
	// file operations pointer
	dev->gd->private_data = dev;
	// for private use
	strcpy(dev->gd->disk_name,"sbulla");
	// device name is sbull
	set_capacity(dev->gd,nsectors);
	// initial queue
	dev->gd->queue = blk_init_queue(sbull_request, &dev->lock);
	add_disk(dev->gd);
	return 0;
}

static void sbull_request(request_queue_t *q)
{
	struct request *req;
	while((req = elv_next_request(q)) != NULL)
	{
		struct sbull_dev *dev = req->rq_disk->private_data;
		if( !blk_fs_request(req))
		{
			end_request(req,0);
		}
		else
		{
			sbull_transfer(dev, req->sector, 
			req->current_nr_sectors,
			req->buffer, rq_data_dir(req));
			end_request(req,1);
		} 
	}
}

static void sbull_transfer(struct sbull_dev *dev, unsigned long sector, 
							unsigned long nsect, char *buffer, int write) 
{
	unsigned long offset = sector*KERNEL_SECTOR_SIZE;
	unsigned long nbytes = nsect*KERNEL_SECTOR_SIZE;
	if( (offset+nbytes) > dev->size )
	{
		printk(KERN_NOTICE "beyond-end write (%ld %ld)\n",
		offset, nbytes); 
		return;
	}
	if(write)
		memcpy(dev->data + offset, buffer, nbytes);
	else
		memcpy(buffer, dev->data + offset, nbytes);
}

static void sbull_exit(void)
{
	printk("sbull_exit(): exit sbull...\n");
	del_gendisk(dev->gd);
	put_disk(dev->gd);
	unregister_blkdev(dev_major_nr, dev_name);
	vfree(dev->data);
}

module_init(sbull_init);
module_exit(sbull_exit);