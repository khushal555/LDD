#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>

#define I2C_BUS_AVAILABLE  (1)            
#define SLAVE_DEVICE_NAME  ("BMP280")      
#define BMP280_SLAVE_ADDR  (0x40)           

static struct i2c_adapter *my_i2c_adapter= NULL;  
static struct i2c_client  *bmp280_client = NULL;  

static int      __init bmp280_driver_init(void);
static void     __exit bmp280_driver_exit(void);
static int bmp280_probe(struct i2c_client *client,const struct i2c_device_id *device_id);


