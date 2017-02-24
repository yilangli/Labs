#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/pci.h>

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("YILANGLI");

#define PCI_VENDOR_ID_CCORSI  (0X1234)
#define PCI_DEVICE_ID_CCORSI_KYOUKO3  (0x1113)
#define KYOUKO3_CONTROL_SIZE  (65536)
#define Device_RAM  (0X0020)

struct cdev whatever;

struct pci_device_id kyouko3_dev_id[]={
	{PCI_DEVICE(PCI_VENDOR_ID_CCORSI, PCI_DEVICE_ID_CCORSI_KYOUKO3)},
	{0},
};


int kyouko3_probe(struct pci_dev *pci_dev, const struct pci_device_id *pci_id);
void kyouko3_remove(struct pci_dev *pci_dev);

struct pci_driver kyouko3_pci_dri = {
	.name= "whatever",
	.id_table= kyouko3_dev_id,
	.probe= kyouko3_probe,
	.remove= kyouko3_remove
};

struct video_card {
	unsigned int* p_control_base;
	unsigned int* p_ram_card_base;
	unsigned int* k_control_base;
	unsigned int* k_ram_card_base;
} kyouko3;


int kyouko3_probe(struct pci_dev *pci_dev, const struct pci_device_id *pci_id){
	printk(KERN_ALERT "kyouko3_probe\n");
	pci_enable_device(pci_dev);
	kyouko3.p_control_base = pci_resource_start(pci_dev, 1);
	kyouko3.p_ram_card_base = pci_resource_start(pci_dev, 2);
	pci_set_master(pci_dev);
	printk(KERN_ALERT "P_CONTROL_BASE: %X\n ", kyouko3.p_control_base);
	printk(KERN_ALERT "P_RAM_CARD_BASE: %X\n", kyouko3.p_ram_card_base);
	return 0;
}

void kyouko3_remove(struct pci_dev *pci_dev){
	printk(KERN_ALERT "kyouko3_remove");
	pci_disable_device(pci_dev);
}

unsigned int K_READ_REG(unsigned int reg){
	unsigned int value;
	delay();
	value = *(kyouko3.k_control_base+(reg>>2));
	return value;
}

void K_WRITE_REG(unsigned int reg, unsigned int value){
	delay();
	*(kyouko3.k_control_base+(reg>>2)) = value;
}

int kyouko3_mmap(struct file *fp, struct vm_area_struct *vma){
	int ret;
	ret = io_remap_pfn_range(vma, vma->vm_start, (unsigned int)(kyouko3.p_control_base)>>PAGE_SHIFT, vma->vm_end-vma->vm_start, vma->vm_page_prot);
	return ret;
}

int kyouko3_open(struct inode *inode, struct file *fp){
	printk(KERN_ALERT "kyouko3_open\n");
	kyouko3.k_control_base = ioremap(kyouko3.p_control_base, KYOUKO3_CONTROL_SIZE);
	int ram_size;
	ram_size = K_READ_REG(Device_RAM);
	printk(KERN_ALERT "kernel: ram_size in MB is %d\n", ram_size);
	ram_size*=(1024*2014);
	kyouko3.k_ram_card_base = ioremap(kyouko3.p_ram_card_base, ram_size);
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



int my_init_function(void){
	printk(KERN_ALERT "init function\n");
	cdev_init(&whatever, &kyouko3_fops);
	cdev_add(&whatever, MKDEV(500,127),1);
	pci_register_driver(&kyouko3_pci_dri);
	return 0;
}

void my_exit_function(void){
	printk(KERN_ALERT "exit function\n");
	pci_unregister_driver(&kyouko3_pci_dri);
	cdev_del(&whatever);
}


module_init(my_init_function);
module_exit(my_exit_function);

