#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/i2c.h>
#include <linux/types.h>

#define I2C_BUS_AVAILABLE 1
dev_t dev =0;

static struct i2c_adapter *my_i2c_adapter= NULL;  

static int __init bmp280_driver_init(void)
{	
	pr_info("I2C Bus Driver Insert...Done!!!\n");
	
	my_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    
        if( my_i2c_adapter != NULL )
        {
             i2c_put_adapter(my_i2c_adapter);
	     pr_info("Adapter added...\n");
        }
	else
	{
		pr_info("Error: adapter not created");
	}
	
	if((alloc_chrdev_region(&dev,0,2,"my_Char"))<0)
	{
		printk(KERN_INFO"cannot allocate major and minor number");
		return -1;
	}
	printk(KERN_INFO"Major =%d Minor = %d \n",MAJOR(dev),MINOR(dev));

       return 0;
}

static void __exit bmp280_driver_exit(void)
{
        i2c_del_adapter(&my_i2c_adapter);
	unregister_chrdev_region(dev,1);
	pr_info("BMP280 Bus Driver Remove..\n");
}

module_init(bmp280_driver_init);
module_exit(bmp280_driver_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("I2C driver using SMBus");
MODULE_VERSION("1.1");

