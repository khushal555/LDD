#include <linux/module.h>

static int __init function_name_insert(void)
{
	printk(KERN_INFO"Loaded sucessfully");
	return 0;
}

module_init(function_name_insert);

void __exit function_name_remove(void)
{
	printk(KERN_INFO"Remove sucessfully");
}

module_exit(function_name_remove);

MODULE_LICENSE("GPL");

MODULE_AUTHOR("Khushal");
MODULE_DESCRIPTION("ITS A FIRST MODULE");

