#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("YILANGLI");



struct cdev whatever;

int kyouko3_open(struct inode *inode, struct file *fp){
	printk(KERN_ALERT "kyouko3_open\n");
	return 0;
}

int kyouko3_release(struct inode *inode, struct file *fp){
	printk(KERN_ALERT "kyouko3_release\n");
	return 0;
}
struct file_operations kyouko3_fops = {
	.open= kyouko3_open,
	.release= kyouko3_release,
	.owner= THIS_MODULE 
	
};

int my_init_function(void){
	printk(KERN_ALERT "init function\n");
	cdev_init(&whatever, &kyouko3_fops);
	cdev_add(&whatever, MKDEV(500,127),1);
	return 0;
}

void my_exit_function(void){
	printk(KERN_ALERT "exit function\n");
	cdev_del(&whatever);
}


module_init(my_init_function);
module_exit(my_exit_function);

