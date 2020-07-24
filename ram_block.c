/*
fileName        1_ram_block.c
author  :       
teamLead:       Rajesh Dommaraju
Details :       allocates (with vmalloc) an array of memory; it then makes that array available via block operations.
License :       Spanidea Systems Pvt. Ltd.
*/

//--------------------------------------------------------------------------------------------------------------

/* Disk on RAM Driver */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/blkdev.h> // For at least, struct block_device_operations
#include <linux/errno.h>


#define RB_FIRST_MINOR 0	//First Minor number	
#define RB_MINOR_CNT 1		//Number of minor devices
#define RB_DEVICE_SIZE 1	//Represents disk size in units of sectors
#define RB_SECTOR_SIZE 512	//sector size of the disk 

static u_int rb_major = 0;	//Major number to be alloted by kernel
static u8 *dev_data;		//pointer that will store the starting address of the disk.


/* 
 * The internal structure representation of our Device
 */
static struct rb_device
{
	/* Size is the size of the device (in sectors) */
	unsigned int size;
	/* For exclusive access to our request queue */
	//The spinlock is provided to control the access to the request queue.
	spinlock_t lock;
	/* Our request queue */
	struct request_queue *rb_queue;
	/* This is kernel's representation of an individual disk device */
	struct gendisk *rb_disk;
} rb_dev;

static int rb_open(struct block_device *bdev, fmode_t mode)
{
	printk(KERN_INFO "rb:Device is opened \n");
	return 0;
}

static void rb_close(struct gendisk *disk, fmode_t mode)
{
	printk(KERN_INFO "rb: Device is closed\n");
}


/*
function        :static int rb_transfer(struct request *)
desc            :The function performs the read and write operations

*/
static int rb_transfer(struct request *req)
{
	int dir = rq_data_dir(req);	//Macro determines the direction the Block IO request
					//Returns READ for a read request and WRITE for a write request
	sector_t start_sector = blk_rq_pos(req);	//returns the current sector number
	unsigned int sector_cnt = blk_rq_sectors(req);	//returns number of sectors left in the current segment

	#define BV_PAGE(bv) ((bv).bv_page)
	#define BV_OFFSET(bv) ((bv).bv_offset)
	#define BV_LEN(bv) ((bv).bv_len)
	
	struct bio_vec bv;	
	/*
		struct bio{
			struct bio_vec *bi_io_vec; //points to an array of struct bio_vec 
		}
			struct bio_vec{
				struct page *bv_page;
				unsigned int bv_len;
				unsigned int bv_offset;
			}
			
	*/
	struct req_iterator iter;//it transfer the data from every physical 					//page
	sector_t sector_offset;//it is location block of page are store
	unsigned int sectors;
	u8 *buffer;

	int ret = 0;

	//printk(KERN_DEBUG "rb: Dir:%d; Sec:%lld; Cnt:%d\n", dir, start_sector, sector_cnt);

	sector_offset = 0;

	//This macrodefinition iterates through each segment of each struct bio structure of a struct request 
	//structure and updates a struct req_iterator structure. The struct req_iterator contains the current 
	//struct bio structure and the iterator that traverses its segments.
	rq_for_each_segment(bv, req, iter)
	{	
		//In this loop,the struct bio_vec bv is the current bio_vec entry,
			
		//void* page_address(struct page *page)
		//Macros that convert between kernel logical addresses and their associated memory map entries.
		buffer = page_address(BV_PAGE(bv)) + BV_OFFSET(bv);
		
		//printk("Physical address of page %p\n",BV_PAGE(bv));
		printk("Len of page %d\n",BV_LEN(bv));
		printk("page offset %d\n",BV_OFFSET(bv));
	
		if (BV_LEN(bv) % RB_SECTOR_SIZE != 0)
		{
			printk(KERN_ERR "rb: Should never happen: "
				"bio size (%d) is not a multiple of RB_SECTOR_SIZE (%d).\n"
				"This may lead to data truncation.\n",
				BV_LEN(bv), RB_SECTOR_SIZE);
			ret = -EIO;
		}

	
		sectors = BV_LEN(bv) / RB_SECTOR_SIZE;
		printk(KERN_DEBUG "Start Sector: %llu\nSector Offset: %llu\nBuffer: %p\nLength: %u sectors\n\n",(unsigned long long)(start_sector), (unsigned long long)(sector_offset),buffer, sectors);
		
		if (dir == WRITE) /* Write to the device */
		{
			printk("writing into the disk......!!!!!!!!!!\n");
				printk("Address of dest is %p\n",(dev_data+(start_sector+sector_offset)*RB_SECTOR_SIZE));
			printk("Address of src is %p\n",buffer);
			printk("Number of bytes that will be copied %d\n\n\n",sectors*RB_SECTOR_SIZE);

				//printk("start sector number %d\n",start_sector);
				//printk("sector offset %d\n",sector_offset);
				//printk("writing into the disk......!!!!!!!!!!\n");
			
			memcpy(dev_data + (start_sector+sector_offset) * RB_SECTOR_SIZE, buffer,sectors*RB_SECTOR_SIZE);
		}
		else /* Read from the device */
		{
			printk("reading from the disk.......!!!!!!!!!!!!\n");
			printk("Address of dest is %p\n",buffer);
			printk("Address of src is %p\n",(dev_data+(start_sector+sector_offset)*RB_SECTOR_SIZE));
			printk("Number of bytes that will be copied %d\n\n\n\n",sectors*RB_SECTOR_SIZE);

		
			memcpy(buffer, dev_data+(start_sector+sector_offset)*RB_SECTOR_SIZE,sectors*RB_SECTOR_SIZE);
		}
		sector_offset += sectors;
	}
	if (sector_offset != sector_cnt)
	{
		printk(KERN_ERR "rb: bio info doesn't match with the request info");
		ret = -EIO;
	}

	return ret;
}
	
/*
 * Represents a block I/O request for us to execute
 */

/*
Function: 	void request(struct request_queue *)
Desc:		Represent a Block IO request to execute
		The 		

*/
static void rb_request(struct request_queue *q)
{
	struct request *req;
	int ret;

	/* Gets the current request from the  queue */
/*
Function:struct request * blk_fetch_request(struct request_queue *)
Desc:	 The function returns pointer to the next request to process which is DETERMINED BY THE IO SCHEDULER
	 or NULL if no request are present to be processed.	
*/
	while ((req = blk_fetch_request(q)) != NULL)
	{
#if 0
		/*
		 * This function tells us whether we are looking at a filesystem request
		 * - one that moves block of data
		 */
		if (!blk_fs_request(req))
		{
			printk(KERN_NOTICE "rb: Skip non-fs request\n");
			/* We pass 0 to indicate that we successfully completed the request */
			__blk_end_request_all(req, 0);
			//__blk_end_request(req, 0, blk_rq_bytes(req));
			continue;
		}
#endif
		ret = rb_transfer(req);
		__blk_end_request_all(req, ret);//it useful for when no call recives to transfer data block
                                                //that time it return error,where it second argument -EIO.
		//__blk_end_request(req, ret, blk_rq_bytes(req));//if all sector request processed then use.
	}
}

/* 
 * These are the file operations that performed on the ram block device
 */
static struct block_device_operations rb_fops =
{
	.owner = THIS_MODULE,
	.open = rb_open,
	.release = rb_close,
};
	
/* 
 * This is the registration and initialization section of the ram block device
 * driver
 */
static int __init rb_init(void)
{
/*-------------------Modified-----------------------------------------*/
	dev_data = vmalloc(RB_DEVICE_SIZE * RB_SECTOR_SIZE);
	if (dev_data == NULL)
		return -ENOMEM;
	printk(KERN_ERR "Starting address allocated by kernel in memory is  %p\n\n",dev_data);
	//Initializing the allocated memory with 0x00
	memset(dev_data,0x00,RB_SECTOR_SIZE);
	rb_dev.size = RB_DEVICE_SIZE;
/*--------------------------Modified------------------------------*/
	
/*
function   : 	int register_blkdev(int,"")
Description:	register a block driver and returns a major number on success or -1 on error
*/

	/* Get Registered */
	rb_major = register_blkdev(rb_major, "myBlockDriver");
	if (rb_major <= 0)
	{
		printk(KERN_ERR "rb: Unable to get Major Number\n");
		vfree(dev_data);
		return -EBUSY;
	}
	printk("Major Number for the device is %d\n",rb_major);

	/* Get a request queue (here queue is created) */
	spin_lock_init(&rb_dev.lock);
	/*
	function:	struct request queue* blk_init_queue(function pointer,address of spinlock)
	Desc:		Allocates a request queue
			Returns address to the request queue structure
	*/
	rb_dev.rb_queue = blk_init_queue(rb_request, &rb_dev.lock);
	if (rb_dev.rb_queue == NULL)
	{
		printk(KERN_ERR "rb: blk_init_queue failure\n");
		unregister_blkdev(rb_major,"myBlockDriver");
		vfree(dev_data);

		return -ENOMEM;
	}
	
	/*
	 * Add the gendisk structure
	 * By using this memory allocation is involved, 
	 * the minor number we need to pass bcz the device 
	 * will support this much partitions 
	 */
	rb_dev.rb_disk = alloc_disk(RB_MINOR_CNT);
	if (!rb_dev.rb_disk)
	{
		printk(KERN_ERR "rb: alloc_disk failure\n");
		blk_cleanup_queue(rb_dev.rb_queue);
		unregister_blkdev(rb_major, "myBlockDriver");
		vfree(dev_data);

		return -ENOMEM;
	}

 	/* Setting the major number */
	rb_dev.rb_disk->major = rb_major;
  	/* Setting the first mior number */
	rb_dev.rb_disk->first_minor = RB_FIRST_MINOR;
 	/* Initializing the device operations */
	rb_dev.rb_disk->fops = &rb_fops;
 	/* Driver-specific own internal data */
	rb_dev.rb_disk->private_data = &rb_dev;
	rb_dev.rb_disk->queue = rb_dev.rb_queue;
	/*
	 * You do not want partition information to show up in 
	 * cat /proc/partitions set this flags
	 */
	//rb_dev.rb_disk->flags = GENHD_FL_SUPPRESS_PARTITION_INFO;
	sprintf(rb_dev.rb_disk->disk_name, "myBlockDriver");
	/* Setting the capacity of the device in its gendisk structure */
	
	//The driver sets the capacity of the device in the gendisk structure
	set_capacity(rb_dev.rb_disk, rb_dev.size);

	/* Adding the disk to the system */
        //it activeds the disk
	add_disk(rb_dev.rb_disk);
	/* Now the disk is "live" */
	printk(KERN_INFO "rb: Ram Block driver initialised (%d sectors; %d bytes)\n",
		rb_dev.size, rb_dev.size * RB_SECTOR_SIZE);

	return 0;
}
/*
 * This is the unregistration and uninitialization section of the ram block
 * device driver
 */
static void __exit rb_cleanup(void)
{
	printk("Exit...............!\n");
	del_gendisk(rb_dev.rb_disk);
	put_disk(rb_dev.rb_disk);
	blk_cleanup_queue(rb_dev.rb_queue);
	unregister_blkdev(rb_major, "rb");
	vfree(dev_data);
}

module_init(rb_init);
module_exit(rb_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mayur Gajanan & Mustansir @ Spanidea Systems");
MODULE_DESCRIPTION("Block driver emulation on memory area");
MODULE_ALIAS_BLOCKDEV_MAJOR(rb_major);
