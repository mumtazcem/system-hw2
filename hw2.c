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
#define HW2_NR_DEVS 2

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
};

struct queue_dev {
    struct node *front, *back;
    size_t size_of_data;
    struct semaphore sem;
    struct cdev cdev;
    /*struct cdev is the kernel’s internal structure
     *  that represents char devices; this field 
     * contains a pointer to that structure when 
     * the inode refers to a char device file.*/
};

struct queue_dev *queue_devices;

int delete_queue(struct queue_dev *dev)
{
	struct node *temp;
    if (dev->front) {
        temp = dev->front;
        while (temp){
			kfree(temp->data);
			kfree(temp);
			if (temp->next)
				temp = temp->next;
			else
				temp = NULL;
		}
    }
    kfree(temp);
    dev->size_of_data = 0;
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
	// When reading from the queue, entries in the queue will behave as concatenated strings.
	struct node *temp;
	struct queue_dev *dev = filp->private_data;
	size_t destination_size;
	char *concatenated;
	ssize_t retval = 0;
	if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    // if called device is queue0
    // then no read operation.
    if (dev == &queue_devices[0]){
		printk(KERN_ALERT "queue0 is called!..\n");
		retval = -EINVAL;
        goto out;
	}
	// prevent read operation if front
	// is null.
	if (dev->front == NULL){
		retval = -ESPIPE;
        goto out;
	}
	printk(KERN_ALERT "In queue_read\n"); 
    temp = kmalloc(sizeof(struct node), GFP_KERNEL); 
    concatenated = kmalloc(dev->size_of_data * sizeof(char), GFP_KERNEL);
	// copy data
	destination_size = strlen(dev->front->data);
	if (*f_pos >= dev->size_of_data) {
        retval = 0;
        goto out;
    }
	printk(KERN_ALERT "destination_size is: %d \n", destination_size);
	printk(KERN_ALERT "dev->front->data data is %s\n", dev->front->data); 
	temp->data = kmalloc((destination_size+1) * sizeof(char), GFP_KERNEL);
	strncpy(temp->data, dev->front->data, destination_size+1);
	temp->next = dev->front->next;
	
	// concatenate strings
	if (temp){
		// front is copied
		strncpy(concatenated, temp->data, destination_size+1);
		printk(KERN_ALERT "Read data is %s\n", temp->data); 
	}
	else{
		printk(KERN_ALERT "QUEUE IS EMPTY, CAN NOT READ..\n");
		retval = -EFAULT;
        goto out;
	}
	temp = temp->next;
	while(temp){
		destination_size = strlen(temp->data);
		printk(KERN_ALERT "tempSize is: %d \n", destination_size);
		strncat(concatenated, temp->data, destination_size);
		printk(KERN_ALERT "Concatenated data is %s\n", temp->data); 
		temp = temp->next;
	}
	dev->size_of_data = strlen(concatenated)+1;
	*(concatenated+dev->size_of_data-1) = '\0';
	printk(KERN_ALERT "Result data is %s\n", concatenated);
	printk(KERN_ALERT "dev->size_of_data is: %d \n", dev->size_of_data);
	if (copy_to_user(buf, concatenated, strlen(concatenated)+1)) {
        retval = -EFAULT;
        goto out;
    }
    *f_pos += dev->size_of_data;
	retval = dev->size_of_data;
	kfree(temp); 
	kfree(concatenated);
	out:
    up(&dev->sem);
    return retval;
}

ssize_t queue_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t *f_pos)
{
	// Writing to a queue device will insert the written text to the end of the queue.
	struct node *temp;
	struct queue_dev *dev = filp->private_data;
    ssize_t retval = -ENOMEM;
    /* error handling */
    if (down_interruptible(&dev->sem))
       return -ERESTARTSYS;
    // if called device is queue0
    // then no write operation.
    if (dev == &queue_devices[0]){
		printk(KERN_ALERT "queue0 is called!..\n");
		retval = -EINVAL;
        goto out;
	}
	printk(KERN_ALERT "In queue_write\n");
    // allocation
    temp = kmalloc(sizeof(struct node), GFP_KERNEL);
    temp->data = kmalloc(count * sizeof(char), GFP_KERNEL);
    temp->next = NULL;
    // check if it is successful.
    if (!temp->data){
		printk(KERN_ALERT "temp->data kmalloc error..\n");
        goto out;
	}
	printk(KERN_ALERT "The buffer is %s\n", buf);
	printk(KERN_ALERT "Its count is %d\n", count);
    // copy buf into temp->data
    if (copy_from_user(temp->data, buf, count)) {
		printk(KERN_ALERT "copy_from_user error..\n");
        retval = -EFAULT;
        goto out;
    }
    *(temp->data+count-1) = '\0';
    retval = count;
    printk(KERN_ALERT "Copied data is %s\n", temp->data);        
    printk(KERN_ALERT "Copied data strlen is %d\n", strlen(temp->data));
    if (dev->front == NULL){
		// first element
		printk(KERN_ALERT "First element in the queue..\n");
		dev->front = kmalloc(sizeof(struct node), GFP_KERNEL);
		dev->back = kmalloc(sizeof(struct node), GFP_KERNEL);
		dev->front = temp;
		dev->back = temp;
	}
	else {
		// write it to the end of the queue
		printk(KERN_ALERT "writing to the end of the queue..\n");
		dev->back->next = kmalloc(sizeof(struct node), GFP_KERNEL);
		dev->back->next = temp;
		dev->back = dev->back->next;
	}
    dev->size_of_data += count;
    printk(KERN_ALERT "size of data is now : %d \n", dev->size_of_data);
    out:
    //kfree(temp);
    up(&dev->sem);
    return retval;
}

long queue_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	// There will be an ioctl command named "pop" which will return the entry at the front of the queue and remove it from the queue.
	int i;
	printk(KERN_ALERT "in queue_ioctl..\n");
	int retval = 0;
	struct queue_dev *dev = filp->private_data;
	// error handling
	if (_IOC_NR(cmd) != 0) return -ENOTTY;
	/* if queue0 is called set dev 
	 * as the first existing device
	 * */ 
	if (dev == &queue_devices[0]){
		printk(KERN_ALERT "queue0 is called!..\n");
		for (i = 1; i < hw2_nr_devs; i++) {
			dev = &queue_devices[i];
			if(dev->front != NULL){
				printk(KERN_ALERT "queue%d is set!..\n", i);
				break;
			}
		}
	}
	if (dev->front == NULL) return -ESPIPE;  //  illegal seek
	printk(KERN_ALERT "The data is about to be popped is %s\n", dev->front->data);
	printk(KERN_ALERT "Its size is %d\n", strlen(dev->front->data)+1);
	// copying the data to the user
	if (copy_to_user((char __user * ) arg, dev->front->data, strlen(dev->front->data)+1)) {
        retval = -EFAULT;
        return retval;
    }
    retval = strlen(dev->front->data)+1;
    dev->size_of_data = dev->size_of_data - retval;
    // pop operation
    dev->front = dev->front->next;
    if(dev->front == NULL)
		dev->back = NULL;
	return retval;
}

loff_t queue_llseek(struct file *filp, loff_t off, int whence)
{
	/* not necessary:
	The llseek method is used to change the current read/write position in a file, and
the new position is returned as a (positive) return value*/
	return 0;
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
    printk(KERN_WARNING "queue: major number is %d\n", hw2_major);
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
        dev->size_of_data = 0;
        sema_init(&dev->sem,1);
        devno = MKDEV(hw2_major, hw2_minor + i);
        cdev_init(&dev->cdev, &queue_fops);
        dev->cdev.owner = THIS_MODULE;
        dev->cdev.ops = &queue_fops;
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
