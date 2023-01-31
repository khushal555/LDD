#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fcntl.h> /*Helps fix O_ACCMODE*/
#include <linux/sched.h> /*Helps fix TASK_UNINTERRUPTIBLE */
#include <linux/fs.h> /*Helps fix the struct intializer */

#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>

#include <linux/proc_fs.h>    // proc file system
#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/proc_ns.h>

#include <linux/kobject.h>

volatile int sysfs_value = 0;
struct kobject *kobj_ref;


#define MEM_SIZE        1024            //Memory Size
#define WR_VALUE _IOW('a','a',int32_t*) //(copy_from_user)
#define RD_VALUE _IOR('a','b',int32_t*) //(copy_to_user)

dev_t dev =0;
static struct class *dev_class;
static struct cdev my_cdev;
uint8_t *kernel_buffer =NULL;
int32_t value = 0;



static int      __init my_driver_init(void);
static void     __exit my_driver_exit(void);
static int      my_open(struct inode *inode, struct file *file);
static int      my_release(struct inode *inode, struct file *file);
static ssize_t  my_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  my_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     my_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/***************** sysfs Functions *******************/
static ssize_t  sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf); 
static ssize_t  sysfs_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count);
struct kobj_attribute sys_attr = __ATTR(sysfs_value, 0660, sysfs_show, sysfs_store);

/*
** This fuction will be called when we read the sysfs file
*/ 
 static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
 { 
        printk(KERN_INFO "Sysfs - Read!!!\n"); 
         return sprintf(buf, "%d", sysfs_value ); 
 } 
/* 
** This fuction will be called when we write the sysfsfs file
*/ 
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{ 
         printk(KERN_INFO "Sysfs - Write!!!\n"); 
         sscanf(buf,"%d",&sysfs_value ); 
         return count; 
}
/***************** sysfs Functions end *******************/
static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = my_read,
    .write      = my_write,
    .open       = my_open,
	.unlocked_ioctl = my_ioctl,
    .release    = my_release,
};



static int my_open(struct inode *inode, struct file *file)
{
	
	if((kernel_buffer = kmalloc(MEM_SIZE,GFP_KERNEL))==0)
	{
		printk(KERN_INFO "Cannot allocate memory in kernel\n");
		return -1;
	}
	printk(KERN_INFO"Driver open  function called .....\n");
	return 0;	
}
static int my_release(struct inode *inode, struct file *file)
{
	kfree(kernel_buffer);
	printk(KERN_INFO"Driver release function called ....\n");
	return 0;
}
static ssize_t my_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	int ret =0;
	//Copy the data from the kernel space to the user-space
    ret = copy_to_user(buf, kernel_buffer, MEM_SIZE);
	if(ret > 0)
	{
		printk(KERN_INFO"writing data to user-space failed\n");
	}
	printk(KERN_INFO"Driver read function called .....\n");
	return 0;
}
static ssize_t my_write(struct file *filp, const char __user *buf,size_t len,loff_t *off)
{
	int ret = 0;
	//Copy the data to kernel space from the user-space
	copy_from_user(kernel_buffer, buf, len);
	if(ret > 0)
	{
		printk(KERN_INFO"Copy the data to kernel space from the user-space\n");
	}
	printk(KERN_INFO"Driver write function called .... \n");
	return len;
} 
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
                case WR_VALUE:
                        copy_from_user(&value ,(int32_t*) arg, sizeof(value));
                        printk(KERN_INFO "Value = %d\n", value);
                        break;
                case RD_VALUE:
                        copy_to_user((int32_t*) arg, &value, sizeof(value));
                         break;
         }
         return 0;
 }

static int __init my_driver_init(void)
{
	if((alloc_chrdev_region(&dev,0,1,"my_Char"))<0)
	{
		printk(KERN_INFO"cannot allocate major and minor number");
		return -1;
	}
	printk(KERN_INFO"Major =%d Minor = %d \n",MAJOR(dev),MINOR(dev));
	
	//creating cdev structure
	cdev_init(&my_cdev,&fops);
	
	//Adding charriver to system
	if((cdev_add(&my_cdev,dev,1)) <0)
	{
		printk(KERN_INFO"cannot add the device to the system \n");
		goto r_class;
	}

	if((dev_class = class_create(THIS_MODULE,"my_Char_class")) == NULL){
		printk(KERN_INFO"Cannot create the struct class for device");
		goto r_class;
	}
	if((device_create(dev_class,NULL,dev,NULL,"my_Char_driver"))==NULL) {
		printk(KERN_INFO" cannot create the device\n");
		goto r_device;
	}
	
	/*Creating a directory in /sys/kernel/ */
 	kobj_ref = kobject_create_and_add("my_sysfs",kernel_kobj);

	/*Creating sysfs file for etx_value*/
	if(sysfs_create_file(kobj_ref,&sys_attr.attr)){
 		printk(KERN_INFO"Cannot create sysfs file......\n");
		goto r_sysfs;
 	}



	printk(KERN_INFO"character drivre init sucess\n");

	return 0;
r_sysfs:
	kobject_put(kobj_ref);  // free the kobject 
	sysfs_remove_file(kernel_kobj, &sys_attr.attr);// removing the file entry

r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev,1);
	return -1;
}

static void __exit my_driver_exit(void)
{
	kobject_put(kobj_ref);  // free the kobject 
	sysfs_remove_file(kernel_kobj, &sys_attr.attr);// removing the file entry
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&my_cdev);
	unregister_chrdev_region(dev,1);
	printk(KERN_INFO "kernel driver removed  ... done \n");

}
module_init(my_driver_init);
module_exit(my_driver_exit);

MODULE_LICENSE("GPL");

