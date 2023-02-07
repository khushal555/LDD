
#include "bmp280.h"

static const struct i2c_device_id bmp280_id[] = {
        { SLAVE_DEVICE_NAME, BMP280_SLAVE_ADDR },
        { }
};
MODULE_DEVICE_TABLE(i2c,bmp280_id);


static struct i2c_driver bmp280_driver = {
        .driver = {
            .name   = SLAVE_DEVICE_NAME,
            .owner  = THIS_MODULE,
        },
        .probe          = bmp280_probe,
        .id_table       = bmp280_id,
};

static struct i2c_board_info bmp280_i2c_board_info = {
        I2C_BOARD_INFO(SLAVE_DEVICE_NAME, BMP280_SLAVE_ADDR)
    };


static int bmp280_probe(struct i2c_client *client,const struct i2c_device_id *device_id)
{		
	pr_info("Probe Exit....\n");
	return 0;
}

static int __init bmp280_driver_init(void)
{	
	my_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    
        bmp280_client = i2c_new_device(my_i2c_adapter, &bmp280_i2c_board_info);
        
        i2c_add_driver(&bmp280_driver);

        i2c_put_adapter(my_i2c_adapter);
	
        return 0;
}

static void __exit bmp280_driver_exit(void)
{
        i2c_unregister_device(bmp280_client);
	i2c_del_driver(&bmp280_driver);

	pr_info("BMP280 Driver Remove..\n");
}

module_init(bmp280_driver_init);
module_exit(bmp280_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Khushal");
MODULE_DESCRIPTION("I2C driver using SMBus");
MODULE_VERSION("1.0");

