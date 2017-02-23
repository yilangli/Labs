#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("YILANGLI");

#define PCI_VENDOR_ID_CCORSI  (0X1234)
#define PCI_DEVICE_ID_CCORSI_KYOUKO3  (0x1113)
#define KYOUKO3_CONTROL_SIZE  (65536)
#define Device_RAM  (0X0020)

struct cdev whatever;

struct pci_device_id kyouko3_dev_id[]{
	{PCI_DEVICE(PCI_VENDOR_ID_CCORSI, PCI_DEVICE_ID_CCORSI_KYOUKO3)},
	{0}
}

struct pci_driver kyouko3_pci_dri = {
	.name= "whatever",
	.id_table= kyouko3_dev_id,
	.probe= kyouko3_probe,
	.remove= kyouko3_remove;
}

struct video_card {
	unsigned int* p_control_base;
	unsigned int* p_ram_card_base;
	unsigned int* k_control_base;
	unsigned int* k_ram_card_base;
} kyouko3

int kyouko3_probe(struct pci_dev *pci_dev, const struct pci_device_id *pci_id){
	kyouko3.p_control_base = pci_resource_start(pci_dev, 1);
	kyouko3.p_ram_card_base = pci_resource_start(pci_dev, 2);
	pci_enable_device(pci_dev)
	pci_set_master(pci_dev);
	printk(KERNEL_ALERT "P_CONTROL_BASE: %X\n ", kyouko3.p_control_base);
	printk(KERNEL_ALERT "P_RAM_CARD_BASE: %X\n", kyouko3.p_ram_card_base);
	return 0;
}

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
	.owner= THIS_MODULE, 
	.mmap= kyouko3_mmap
	
};

unsigned int K_READ_REG(unsigned int reg){
	unsigned int value;
	delay();
	value = *(kyouko3.k_ram_card_base);
}

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

