#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/types.h>

#define I2C_BUS_AVAILABLE  (1)             
#define SLAVE_DEVICE_NAME  ("BMP280")      
#define BMP280_SLAVE_ADDR  (0x40)           

dev_t dev = 0;

static struct i2c_adapter *my_i2c_adapter= NULL;  
static struct i2c_client  *bmp280_client = NULL;  
static struct class *i2c_class;
static struct cdev i2c_cdev;

s32 calib_T1, calib_T2, calib_T3;
s32 calib_P1, calib_P2, calib_P3, calib_P4, calib_P5, calib_P6, calib_P7, calib_P8, calib_P9;
int t_fine;	

static int      __init bmp280_driver_init(void);
static void     __exit bmp280_driver_exit(void);
static int      bmp280_open(struct inode *inode, struct file *file);
static int      bmp280_release(struct inode *inode, struct file *file);
static ssize_t  bmp280_read(struct file *filp, char *user_buffer, size_t len,loff_t * off);
static int bmp280_probe(struct i2c_client *client,const struct i2c_device_id *device_id);
s32 read_temperature(void);
s32 read_pressure(void);

