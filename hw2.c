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

struct queue_dev {
    char **data;
    int quantum;
    int qset;
    unsigned long size;
    struct semaphore sem;
    struct cdev cdev;
    /*struct cdev is the kernel’s internal structure that represents char devices; this
	field contains a pointer to that structure when the inode refers to a char device
	file.*/
};

struct queue_dev *queue_devices;

int queue_trim(struct queue_dev *dev)
{
}

int queue_open(struct inode *inode, struct file *filp)
{}

int queue_release(struct inode *inode, struct file *filp)
{
	/*This operation is invoked when the file structure is being released. Like open,
release can be NULL.*/
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
{}

int queue_init_module(void)
{}

module_init(queue_init_module);
module_exit(queue_cleanup_module);
