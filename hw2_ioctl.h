#ifndef __HW2_H
#define __HW2_H

#include <linux/ioctl.h> /* needed for the _IOW  */

#define HW2_IOC_MAGIC  'k'

//  ioctl command would be defined here as below.

#define HW2_IO    _IO(HW2_IOC_MAGIC, 0)
#define HW2_IOR    _IOR(HW2_IOC_MAGIC, 1)
#define HW2_IOW    _IOW(HW2_IOC_MAGIC, 2)
#define HW2_IORW    _IORW(HW2_IOC_MAGIC, 3)

/*#define SCULL_IOCRESET    _IO(SCULL_IOC_MAGIC, 0)
#define SCULL_IOCSQUANTUM _IOW(SCULL_IOC_MAGIC,  1, int)
#define SCULL_IOCSQSET    _IOW(SCULL_IOC_MAGIC,  2, int)
#define SCULL_IOCTQUANTUM _IO(SCULL_IOC_MAGIC,   3)
#define SCULL_IOCTQSET    _IO(SCULL_IOC_MAGIC,   4)
#define SCULL_IOCGQUANTUM _IOR(SCULL_IOC_MAGIC,  5, int)
#define SCULL_IOCGQSET    _IOR(SCULL_IOC_MAGIC,  6, int)
#define SCULL_IOCQQUANTUM _IO(SCULL_IOC_MAGIC,   7)
#define SCULL_IOCQQSET    _IO(SCULL_IOC_MAGIC,   8)
#define SCULL_IOCXQUANTUM _IOWR(SCULL_IOC_MAGIC, 9, int)
#define SCULL_IOCXQSET    _IOWR(SCULL_IOC_MAGIC,10, int)
#define SCULL_IOCHQUANTUM _IO(SCULL_IOC_MAGIC,  11)
#define SCULL_IOCHQSET    _IO(SCULL_IOC_MAGIC,  12)
#define SCULL_IOC_MAXNR 12*/

#endif
