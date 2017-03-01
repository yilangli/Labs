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

#define FIFO_START 0X1020
#define FIFO_END 0X1024
#define FIFO_HEAD 0X4010
#define FIFO_TAIL 0X4014

#define GRAPHICS_ON 1
#define GRAPHICS_OFF 0

#define FRAME_START 0X8000
#define ENCODER_START 0X9000
#define ACCELERATION 0X1010
#define MODE_SET 0X1008
#define RASTER_EMIT 0X3004
#define RASTER_CLEAR 0X3008
#define RASTER_FLUSH 0X3FFC

#define FIFO_ENTRIES 1024
#define FIFO_SIZE 8192u

#define VMODE _IOW(0XCC, 0, unsigned long)
#define BIND_DMA _IOWR(0XCC, 1, unsigned long)
#define START_DMA _IOWR(0XCC, 2, unsigned long)
#define FIFO_QUEUE _IOWR(0XCC,3, unsigned long)
#define FIFO_FLUSH _IOW(0XCC, 4, unsigned long)
#define UNBIND_DMA _IOW(0XCC, 5, unsigned long)

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


struct FIFO_ENTRY{
	u32 command;
	u32 value;
}

struct fifo{
	u64 p_base;
	struct FIFO_ENTRY* k_base;
	u32 head;
	u32 tail_cache;
}

struct video_card {
	struct pci_dev *pci_dev;
	int graphics;
	struct fifo fifo;
	unsigned int* p_control_base;
	unsigned int* p_ram_card_base;
	unsigned int* k_control_base;
	unsigned int* k_ram_card_base;
} kyouko3;



int kyouko3_probe(struct pci_dev *pci_dev, const struct pci_device_id *pci_id){
	printk(KERN_ALERT "kyouko3_probe\n");
	kyouko3.pci_dev = pci_dev;
	pci_enable_device(pci_dev);
	kyouko3.p_control_base = pci_resource_start(pci_dev, 1);
	kyouko3.p_ram_card_base = pci_resource_start(pci_dev, 2);
	pci_set_master(pci_dev);
	printk(KERN_ALERT "P_CONTROL_BASE: %X\n ", kyouko3.p_control_base);
	printk(KERN_ALERT "P_RAM_CARD_BASE: %X\n", kyouko3.p_ram_card_base);
	return 0;
}

void kyouko3_remove(struct pci_dev *pci_dev){
	printk(KERN_ALERT "kyouko3_remove\n");
	pci_disable_device(pci_dev);
}

unsigned int K_READ_REG(unsigned int reg){
	unsigned int value;
	rmb();
	value = *(kyouko3.k_control_base+(reg>>2));
	return value;
}

void K_WRITE_REG(unsigned int reg, unsigned int value){
	rmb();
	*(kyouko3.k_control_base+(reg>>2)) = value;
}

void FIFO_WRITE(unsigned int reg, unsigned int value){

	kyouko3.fifo.k_base[kyouko3.fifo.head].command = reg;
	kyouko3.fifo.k_base[kyouko3.fifo.head].value = value;
	kyouko3.fifo.head++;
	if(kyouko3.fifo.head=FIFO_ENTRIES)
		kyouko3.fifo.head=0;

}

int kyouko3_mmap(struct file *fp, struct vm_area_struct *vma){
	int ret;
	ret = io_remap_pfn_range(vma, vma->vm_start, (unsigned int)(kyouko3.p_control_base)>>PAGE_SHIFT, vma->vm_end-vma->vm_start, vma->vm_page_prot);
	return ret;
}

void flush_fifo(void){
	K_WRITE_REG(FIFO_HEAD, kyouko3.fifo.head);
	while(kyouko3.fifo.tail_cache!=kyouko3.fifo.head){
		kyouko3.fifo.tail_cache = K_READ_REG(FIFO_TAIL);
		schedule(); 
	}

}

int kyouko3_ioctl(struct inode *inode, struct file *fp, unsigned int cmd, unsigned long arg){
	
	
	printk(KERN_ALERT "kyouko3_ioctl\n");
	struct FIFO_ENTRY entry;
	float color[4] = {0.5,0.5,0.5,0.5};	
	int i;
	
	switch(cmd){
		case FIFO_QUEUE：
			copy_from_user(&entry,(struct fifo_entry *)arg, sizeof(struct fifo_entry));
			FIFO_WRITE(entry.command, entry.value);
			break;
		case FIFO_FLUSH:
			flush_fifo();
			break;
		case VMODE:
			if(arg==GRAPHICS_ON){

				printk(KERN_ALERT "GRAPHICS ON\n");
				
				K_WRITE_REG(ACCELERATION, 0X40000000);	

				K_WRITE_REG(FRAME_START + 0X0000, 1024);
				K_WRITE_REG(FRAME_START + 0X0004, 768);	
				K_WRITE_REG(FRAME_START + 0X0008, 1024*4);
				K_WRITE_REG(FRAME_START + 0X000C, 0XF888);
				K_WRITE_REG(FRAME_START + 0X0010, 0);
				
				K_WRITE_REG(ENCODER_START + 0X0000, 1024);
				K_WRITE_REG(ENCODER_START + 0X0004, 768);
				K_WRITE_REG(ENCODER_START + 0X0008, 0);
				K_WRITE_REG(ENCODER_START + 0X000C, 0);
				K_WRITE_REG(ENCODER_START + 0X0010, 0);
			
				K_WRITE_REG(MODE_SET, 0);
				msleep(10);
				
				FIFO_WRITE(RASTER_CLEAR, 0X03);
				FIFO_WRITE(RASTER_FLUSH, 0);
		
				flush_fifo();
				kyouko3.graphics = 1;
				
			}else{
				printk(KERN_ALERT "GRAPHICS OFF \n");
				kyouko3.graphics = 0;
			}
			break;
	}

}


int kyouko3_open(struct inode *inode, struct file *fp){
	printk(KERN_ALERT "kyouko3_open\n");
	kyouko3.k_control_base = ioremap(kyouko3.p_control_base, KYOUKO3_CONTROL_SIZE);
	unsigned int ram_size;
	ram_size = K_READ_REG(Device_RAM);
	printk(KERN_ALERT "kernel: ram_size in MB is %d\n", ram_size);
	ram_size*=(1024*2014);
	kyouko3.k_ram_card_base = ioremap(kyouko3.p_ram_card_base, ram_size);

	kyouko3.fifo.k_base = pci_alloc_consistent(kyouko3.pci_dev, FIFO_SIZE, &kyouko3.fifo.p_base);
	K_WRITE_REG(FIFO_START, &kyouko3.fifo.p_base);
	K_WRITE_REG(FIFO_END, &kyouko3.fifo.p_base+FIFO_SIZE);
	kyouko3.fifo.head = 0;
	kyouko3.fifo.tail_cache = 0;

	return 0;
}

int kyouko3_release(struct inode *inode, struct file *fp){
	printk(KERN_ALERT "kyouko3_release\n");
	pci_free_consistent(kyouko3.pci_dev, FIFO_SIZE, kyouko3.fifo.k_base, kyouko3.fifo.p_base);
	return 0;
}
struct file_operations kyouko3_fops = {
	.open= kyouko3_open,
	.release= kyouko3_release,
	.unlocked_ioctl= kyouko3_ioctl,
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

