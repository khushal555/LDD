#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h> 
#include<linux/kobject.h> 
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/err.h>

#define SHARED_IRQ 1

static int irq = SHARED_IRQ, my_dev_id, irq_counter = 0;
module_param (irq, int, S_IRUGO);

static void my_tasklet_handler(unsigned long flag);

DECLARE_TASKLET(my_tasklet,my_tasklet_handler,0);

static void my_tasklet_handler(unsigned long flag)
{
	irq_counter++;
        printk (KERN_INFO "In the ISR: counter = %d\n", irq_counter);
	//tasklet_disable(&my_tasklet);
	printk(KERN_INFO"My tasklet handler function");
	//tasklet_enable(&my_tasklet);
}

  
static irqreturn_t my_interrupt (int irq, void *dev_id)
{
	printk("\n Start ISR...");
    tasklet_schedule(&my_tasklet);
	printk("\n END ISR...");
    return IRQ_NONE;            /* we return IRQ_NONE because we are just observing */
}

static int __init my_init (void)
{
	//IRQF_ONESHOT :: Permission error while loading
	//IRQF_DISABLED:: undeclared 
    if (request_irq(irq, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id))
        return -1;
	
    printk (KERN_INFO "Successfully loading ISR handler\n");
    return 0;
}

static void __exit my_exit (void)
{
	tasklet_kill(&my_tasklet);
	printk(KERN_INFO"My tasklet function exit");
    synchronize_irq (irq);
    free_irq (irq, &my_dev_id);

    printk (KERN_INFO "Successfully unloading \n");
}

module_init (my_init);
module_exit (my_exit);

MODULE_LICENSE ("GPL");
