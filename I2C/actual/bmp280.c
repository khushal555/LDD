
#include "bmp280.h"

#define MAJORNO 240
#define MINORNO  1

static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = bmp280_read,
    .open       = bmp280_open,
    .release    = bmp280_release,
};

static const struct i2c_device_id bmp280_id[] = {
        { SLAVE_DEVICE_NAME, BMP280_SLAVE_ADDR },
        { }
};
MODULE_DEVICE_TABLE(i2c,bmp280_id);


static struct i2c_driver bmp280_driver = {
	.class          = I2C_CLASS_DEPRECATED,
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
	s8 id = i2c_smbus_read_byte_data(bmp280_client, 0xD0);
	printk(" Chip ID: 0x%x\n", id);

	calib_T1 = i2c_smbus_read_word_data(bmp280_client, 0x88);
	calib_T2 = i2c_smbus_read_word_data(bmp280_client, 0x8a);
	calib_T3 = i2c_smbus_read_word_data(bmp280_client, 0x8c);

	if(calib_T2 > 32767)
		calib_T2 -= 65536;

	if(calib_T3 > 32767)
		calib_T3 -= 65536;

	i2c_smbus_write_byte_data(bmp280_client, 0xf5, 5<<5);  //1010 0000   
	i2c_smbus_write_byte_data(bmp280_client, 0xf4, ((5<<5) | (5<<2) | (3<<0)));  // 1011 0111     
	
	pr_info("Probe Exit....\n");
	return 0;
}

static int bmp280_open(struct inode *inode, struct file *file)
{
        pr_info("Driver Open Function Called...!!!\n");
        return 0;
}


static int bmp280_release(struct inode *inode, struct file *file)
{
        pr_info("Driver Release Function Called...!!!\n");
        return 0;
}


static ssize_t bmp280_read(struct file *filp, char *user_buffer, size_t count, loff_t *off)
{
        int ret,t_len=0;
	char temp_str[30],pres_str[15],*t_ptr,*p_ptr;
	int temperature,pressure;
	
	pr_info("Driver Read Function Called...!!!\n");

	temperature = read_temperature();
	sprintf(temp_str, "%d.%d",temperature/100,temperature%100);
	t_ptr=temp_str;
	
	pressure=read_pressure();
	sprintf(pres_str, "%d.%d",pressure/100,pressure%100);
	p_ptr=pres_str;
// TS   // HYD  // TS HYD
	while(*t_ptr!='\0')
	{
		t_ptr++;
	}
	*t_ptr=' ';
	t_ptr++;

	while(*p_ptr!='\0')
	{
		*t_ptr=*p_ptr;
		t_ptr++;
		p_ptr++;
	}
		*t_ptr='\0';

	t_ptr=temp_str;	
	while(*t_ptr!='\0')
	{
		t_len++;
		t_ptr++;
	}
	t_len++;

	ret = copy_to_user(user_buffer, temp_str, t_len);

	return count;
}


s32 read_temperature(void) {
	int var1, var2;
	s32 raw_temp;
	s32 d1, d2, d3;
	pr_info("Read temperature working....\n");
	
	d1 = i2c_smbus_read_byte_data(bmp280_client, 0xFA);// 8
	d2 = i2c_smbus_read_byte_data(bmp280_client, 0xFB);//8
	d3 = i2c_smbus_read_byte_data(bmp280_client, 0xFC);//4
	raw_temp = ((d1<<16) | (d2<<8) | d3) >> 4;

	var1 = ((((raw_temp >> 3) - (calib_T1 << 1))) * (calib_T2)) >> 11;

	var2 = (((((raw_temp >> 4) - (calib_T1)) * ((raw_temp >> 4) - (calib_T1))) >> 12) * (calib_T3)) >> 14;
	t_fine=var1+var2;
	return ((var1 + var2) *5 +128) >> 8;
}

s32 read_pressure(void){
	int var1, var2;
	int pressure;
	s32 r1, r2, r3; 
	s32 raw_pressure;
	
	calib_P1=i2c_smbus_read_byte_data(bmp280_client, 0x8F);
	calib_P2=i2c_smbus_read_byte_data(bmp280_client, 0x91);
	calib_P3=i2c_smbus_read_byte_data(bmp280_client, 0x93);
	calib_P4=i2c_smbus_read_byte_data(bmp280_client, 0x95);
	calib_P5=i2c_smbus_read_byte_data(bmp280_client, 0x97);
	calib_P6=i2c_smbus_read_byte_data(bmp280_client, 0x99);
	calib_P7=i2c_smbus_read_byte_data(bmp280_client, 0x9B);
	calib_P8=i2c_smbus_read_byte_data(bmp280_client, 0x9D);
	calib_P9=i2c_smbus_read_byte_data(bmp280_client, 0x9F);
	
	r1=i2c_smbus_read_byte_data(bmp280_client, 0xF7);
	r2=i2c_smbus_read_byte_data(bmp280_client, 0xF8);
	r3=i2c_smbus_read_byte_data(bmp280_client, 0xF9);

	raw_pressure = ((r1<<16) | (r2<<8) | r3) >> 4;
	
	var1 = (t_fine-128000);
	var2 = var1 * var1 * calib_P6;
	var2 = var2 + ((var1*calib_P5)<<17);
	var2 = var2 + ((s64)(calib_P4)<<35);
	var1 = ((var1 * var1 * calib_P3)>>8) + ((var1 * calib_P2)<<12);
	var1 = ((((1UL<<47)+var1))*((s64)(calib_P1)>>33));

	if (var1 == 0)
	{
		return 0; 	
	}
	
	raw_pressure=((r1<<16) | (r2<<8) | r3) >> 4;
	pressure = 1048576-raw_pressure;
	pressure = (((pressure<<31)-var2)*3125)/var1;
	var1 = (calib_P9 * (pressure>>13) * (pressure>>13)) >> 25;
	var2 = (calib_P8 * pressure) >> 19;
	pressure = ((pressure + var1 + var2) >> 8) + (calib_P7<<4);

	return pressure;
}

static int __init bmp280_driver_init(void)
{	
	int ret=0;
        dev = MKDEV(MAJORNO,MINORNO);
	register_chrdev_region(dev,1,"Static_dev"); 

        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

        cdev_init(&i2c_cdev,&fops);

        if((cdev_add(&i2c_cdev,dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_class;
        }

        if((i2c_class = class_create(THIS_MODULE,"bmp280_class")) == NULL){
            pr_err("Cannot create the struct class\n");
            goto r_class;
        }

        if((device_create(i2c_class,NULL,dev,NULL,"bmp280_device")) == NULL){
            pr_err("Cannot create the Device file\n");
            goto r_device;
        }
        pr_info("Device Driver Insert...Done!!!\n");
	
        bmp280_client = i2c_new_device(my_i2c_adapter, &bmp280_i2c_board_info);
        
       	if( bmp280_client != NULL )
        {
         	ret = i2c_add_driver(&bmp280_driver);
            	if(ret) {
			pr_info("Error: i2c_add_driver failed");				
		}
       	}
	
    	return 0;
r_device:
        class_destroy(i2c_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}

static void __exit bmp280_driver_exit(void)
{
        device_destroy(i2c_class,dev);
        class_destroy(i2c_class);
        cdev_del(&i2c_cdev);
        unregister_chrdev_region(dev, 1);
        i2c_unregister_device(bmp280_client);
	i2c_del_driver(&bmp280_driver);
	pr_info("BMP280 Driver Remove..\n");
}

module_init(bmp280_driver_init);
module_exit(bmp280_driver_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("I2C driver using SMBus");
MODULE_VERSION("1.1");

