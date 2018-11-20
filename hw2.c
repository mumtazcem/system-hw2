#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>

#include <asm/switch_to.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */

#include "hw2_ioctl.h"

// default values
#define HW2_MAJOR 0
#define HW2_NR_DEVS 0

int hw2_major = HW2_MAJOR;
int hw2_minor = 0;
// The number of queue devices will be a module parameter. 
int hw2_nr_devs = HW2_NR_DEVS;

module_param(hw2_major, int, S_IRUGO);
module_param(hw2_minor, int, S_IRUGO);
// number of devices as a module parameter
module_param(hw2_nr_devs, int, S_IRUGO);

MODULE_AUTHOR("Ata, Cem, Furkan");
MODULE_LICENSE("Dual BSD/GPL");

struct node{
	char *data;
	struct node *next;
}

struct queue_dev {
    struct node *front, *back;
    unsigned long size;
    struct semaphore sem;
    struct cdev cdev;
    /*struct cdev is the kernel’s internal structure that represents char devices; this
	field contains a pointer to that structure when the inode refers to a char device
	file.*/
};

struct queue_dev *queue_devices;

int delete_queue(struct queue_dev *dev)
{
	struct node *temp;
    if (dev->front) {
        temp = dev->front;
        while (temp){
			kfree(temp);
			if (temp->next)
				temp = temp->next;
			else
				temp = NULL;
		}
    }
    return 0;
}


int queue_open(struct inode *inode, struct file *filp)
{
	struct queue_dev *dev;
	printk(KERN_ALERT "queue_open: Opening device..\n");
    dev = container_of(inode->i_cdev, struct queue_dev, cdev);
    filp->private_data = dev;
    return 0;
}

int queue_release(struct inode *inode, struct file *filp)
{
	/*This operation is invoked when the file structure is being released. Like open,
	release can be NULL.*/
	// no hardware to shut down
    return 0;
}

ssize_t queue_read(struct file *filp, char __user *buf, size_t count,
                   loff_t *f_pos)
{
	// When reading from the queue, entries in the queue will behave as concatenated strings.}

ssize_t queue_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t *f_pos)
{
	// Writing to a queue device will insert the written text to the end of the queue.
	 struct queue_dev *dev = filp->private_data;
    ssize_t retval = -ENOMEM;

    if (down_interruptible(&dev->sem))
       
}

long queue_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	// There will be an ioctl command named "pop" which will return the entry at the front of the queue and remove it from the queue.
	
}

loff_t queue_llseek(struct file *filp, loff_t off, int whence)
{
	/* i think it is not necessary:
	The llseek method is used to change the current read/write position in a file, and
the new position is returned as a (positive) return value*/
}

struct file_operations queue_fops = {
    .owner =    THIS_MODULE,
    .llseek =   queue_llseek,
    .read =     queue_read,
    .write =    queue_write,
    .unlocked_ioctl =  queue_ioctl,
    .open =     queue_open,
    .release =  queue_release,
};


void queue_cleanup_module(void)
{
	int i;
    dev_t devno = MKDEV(hw2_major, hw2_minor);
	printk(KERN_ALERT "queue_cleanup_module: number of devices:  %d\n", hw2_nr_devs);
    if (queue_devices) {
        for (i = 0; i < hw2_nr_devs; i++) {
            delete_queue(queue_devices + i);
            cdev_del(&queue_devices[i].cdev);
        }
    kfree(queue_devices);
    }

    unregister_chrdev_region(devno, hw2_nr_devs);
}

int queue_init_module(void)
{
	int result, i;
    int err;
    dev_t devno = 0;
    struct queue_dev *dev;

    if (hw2_major) {
        devno = MKDEV(hw2_major, hw2_minor);
        result = register_chrdev_region(devno, hw2_nr_devs, "queue");
    } else {
        result = alloc_chrdev_region(&devno, hw2_minor, hw2_nr_devs,
                                     "queue");
        hw2_major = MAJOR(devno);
    }
    if (result < 0) {
        printk(KERN_WARNING "queue: can't get major %d\n", hw2_major);
        return result;
    }
    
    if (hw2_nr_devs == 0) {
        printk(KERN_WARNING "queue: number of devices is not set: %d\n", hw2_nr_devs);
        return result;
    }
	printk(KERN_ALERT "queue: number of devices:  %d\n", hw2_nr_devs);
    queue_devices = kmalloc(hw2_nr_devs * sizeof(struct queue_dev),
                            GFP_KERNEL);
    if (!queue_devices) {
        result = -ENOMEM;
        goto fail;
    }
    memset(queue_devices, 0, hw2_nr_devs * sizeof(struct queue_dev));

    /* Initialize each device. */
    for (i = 0; i < hw2_nr_devs; i++) {
        dev = &queue_devices[i];
        dev->front = NULL;       
        dev->back = NULL;
        sema_init(&dev->sem,1);
        devno = MKDEV(hw2_major, hw2_minor + i);
        cdev_init(&dev->cdev, &hw2_fops);
        dev->cdev.owner = THIS_MODULE;
        dev->cdev.ops = &scull_fops;
        err = cdev_add(&dev->cdev, devno, 1);
        if (err)
            printk(KERN_NOTICE "Error %d adding queue%d", err, i);
    }

    return 0; /* succeed */

  fail:
    queue_cleanup_module();
    return result;
}

module_init(queue_init_module);
module_exit(queue_cleanup_module);
