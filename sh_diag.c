/************************************************************************************************
  @fileName  : camera_diag.c
  @author    : Shital Shinde
  @teamLead  : Rajesh Dommaraju 
  @details   : Software diagnostic of ov5693_camera.
  @license   : SpanIdea Systems Pvt. Ltd. All rights reserved.
 ******************************************************************************************************/

/*****************************************************************************************************
                                            INCLUDES
 ******************************************************************************************************/

#include<linux/module.h>                                                                       //module function related operations 
#include<linux/fs.h>                                                                          //file operations
#include<linux/cdev.h>                                                                       //to create a char device with major and minor number
#include<linux/device.h>                                                                    //to perform device related operations
#include<linux/uaccess.h>                                                                  //copy_to/from_user()
#include<linux/utsname.h>                                                                 //for kernel information related fun
#include <linux/sh_diag.h>

/*******************************************************************************************************
  Creating ioctl command in driver

#define CAM_DIAG _IOR('a',2,struct st *)

 ********************************************************************************************************/

//struct st cam;

#define CAM_DIAG _IOR('a',2,struct st *)

char data[50],from[20];
int i,num[2];
dev_t dev;
static struct class *dev_class;
static struct cdev my_cdev; //if u want to read write and open operations need to register some structre to driver.

/********************************************************************************************************

                                         function initialization

 ***********************************************************************************************************/

static int my_driver_init(void);
static void my_driver_exit(void);
static int my_open(struct inode *inode, struct file *file);
static int my_release(struct inode *inode, struct file *file);
static long camera_ioctl(struct file *file, unsigned int cmd, unsigned long arg);


static struct file_operations fops =
{
	.owner          = THIS_MODULE,
	.open           = my_open,
	.unlocked_ioctl = camera_ioctl,
	.release        = my_release,
};

static int my_open(struct inode *inode, struct file *file)                                      //inode-> having information about file //file-> opening file
{
	printk(KERN_INFO "\nyour Device File Opened\n");
	return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "\nyour Device File Closed\n");
	return 0;
}

/*********************************************************************************************************

                                writting IOCTL fun in driver

 **********************************************************************************************************/

static long camera_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct st *cam1;

	switch(cmd) 
	{
		case CAM_DIAG:

			cam1=camera_parameters();

			copy_to_user((struct st *)arg,cam1,sizeof(cam1));                           
			printk(KERN_ALERT "camera mode     :  %d\n",cam1->default_mode);
			printk(KERN_ALERT "width           :  %d\n",cam1->default_width);
			printk(KERN_ALERT "height          :  %d\n",cam1->default_height);
			printk(KERN_ALERT "fmt_width       :  %d\n",cam1->fmt_width);
			printk(KERN_ALERT "fmt_height      :  %d\n",cam1->fmt_height);
			printk(KERN_ALERT "numctrls        :  %d\n",cam1->numctrls);
			printk(KERN_ALERT "numfmts         :  %d\n",cam1->numfmts);
			printk(KERN_ALERT "clock frequency :  %d\n",cam1->def_clk_freq);
			printk(KERN_ALERT "frame length    :  %d\n",cam1->frame_length);
			printk(KERN_ALERT "gain            :  %d\n",cam1->gain);
			printk(KERN_ALERT "coarse time     :  %d\n",cam1->coarse_time);

			break;

		default:printk("invalid argument....????\n");
			break;

	}

	return 0;
}


static int my_driver_init(void)
{
	/*Allocating Major number*/


	if((alloc_chrdev_region(&dev, 0, 1, "my_Dev")) <0)
	{
		printk(KERN_INFO "Cannot allocate major number\n");
		return -1;
	}
	printk(KERN_INFO "Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

	/*Creating cdev structure*/
	cdev_init(&my_cdev,&fops);

	/*Adding character device to the system*/
	if((cdev_add(&my_cdev,dev,1)) < 0)
	{
		printk(KERN_INFO "Cannot add the device to the system\n");
		unregister_chrdev_region(dev,1);
		return -1;

	}

	/*Creating struct class*/
	if((dev_class = class_create(THIS_MODULE,"my_class")) == NULL)
	{
		printk(KERN_INFO "Cannot create the struct class\n");
		unregister_chrdev_region(dev,1);
		return -1;

	}

	/*Creating device*/

	if((device_create(dev_class,NULL,dev,NULL,"camera_diagnostic")) == NULL)
	{
		printk(KERN_INFO "Cannot create the Device 1\n");
		class_destroy(dev_class);

	}
	printk(KERN_INFO "your char Device Driver Inserted successfully\n");
	return 0;

}

void my_driver_exit(void)
{
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&my_cdev);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "your char Device Driver Removed successfully\n");
}

module_init(my_driver_init);
module_exit(my_driver_exit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("SHITAL SHINDE <shitalshinde@spanidea.com>");
MODULE_DESCRIPTION("ov5693 camera driver");
