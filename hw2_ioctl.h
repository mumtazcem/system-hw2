#ifndef __HW2_H
#define __HW2_H

#include <linux/ioctl.h> /* needed for the _IOW  */

#define HW2_IOC_MAGIC  'k'

//  ioctl command would be defined here as below.

#define HW2_IOCPOP    _IO(HW2_IOC_MAGIC, 0, char*)

#endif
