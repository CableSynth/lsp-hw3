/*
 * hw4_mod.h -- definitions for the hw4mod char module
 *                    for CS3320 HW4
 *
 * Based on the scull example from LDD3.
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 * Copyright (C) 2017 Modifications: Keith Shomper for CS3320, Cedarville Univ.
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 * $Id: scull.h,v 1.15 2004/11/04 17:51:18 rubini Exp $
 *
 * Modified by Keith Shomper, 10/27/2017 for use in CS3320
 *
 */

#ifndef _HW4_MOD_H_
#define _HW4_MOD_H_

#include <linux/ioctl.h>      /* needed for the _IOW etc stuff used later */

#ifndef HW4MOD_MAJOR
#define HW4MOD_MAJOR 0        /* dynamic major by default    */
#endif

#ifndef HW4MOD_NR_DEVS
#define HW4MOD_NR_DEVS 1      /* need hw4mod0 only           */
#endif

#ifndef HW4MOD_MAX_USERS_IN_VAULT
#define HW4MOD_MAX_USERS_IN_VAULT 20  /* sufficient for HW exercise */
#endif

#define HW4MOD_DATA_SIZE MAX_HINT_PWD_SIZE+2 /* [MAX_HINT_PWD_SIZE] */

/*
 * The bare device is a pwd_vault data structure in memory.
 *
 * hw4mod_dev.pwd_vault               is the password vault
 * hw4mod_dev.pwd_vault->uhpw_data    is the array of hints and pwds for ea user
 * hw4mod_dev.pwd_vault->uhpw_data[i] is the hints and pwds for user i
 *   uhpw_data[i]->total_hpw_pairs    is the num of [hint pwd] pairs for user i
 *   uhpw_data[i]->num_hints          is the num of hints for user i
 *   uhpw_data[i]->seek_hint          is the user's seek paramater set in ioctl
 *   uhpw_data[i]->data               is an array of lists of [hint pwd] pairs
 *   uhpw_data[i]->fp                 is the "file position", a pointer to the
 *                                    current [hint pwd] pair
 *   uhpw_data[i]->data[j]            is the first [hint pwd] in the list of 
 *                                    [hint pwd] pairs for hint j
 *   struct hpw_list *l=next_hint(l); is how to walk to the next pair in list
 *
 */
#include "pwd_vault.h"

struct hw4mod_dev {
	struct pwd_vault    pwd_vault;  /* the password vault               */
	struct semaphore    sem;        /* mutual exclusion semaphore       */
	struct cdev         cdev;	     /* Char device structure	   	     */
};

/*
 * The different configurable parameters
 */
extern int   hw4mod_major;
extern int   hw4mod_nr_devs;
extern int   hw4mod_num_users;

/*
 * Prototypes for shared functions
 */
ssize_t hw4mod_read  (struct file *filp, char __user *buf, size_t count,
                      loff_t *f_pos);
ssize_t hw4mod_write (struct file *filp, const char __user *buf, size_t count,
                      loff_t *f_pos);
loff_t  hw4mod_llseek(struct file *filp, loff_t off, int whence);
long    hw4mod_ioctl (struct file *filp, unsigned int cmd, unsigned long arg);

/*
 * Ioctl definitions
 */

/* Use 'k' as magic number - please use a different 8-bit number in your code */
#define HW4MOD_IOC_MAGIC   'k'
#define HW4MOD_IOCRESET    _IO(HW4MOD_IOC_MAGIC,     0)

/*
 * S means "Set"       through a ptr,
 * G means "Get":      reply by setting through a pointer
 */
#define HW4MOD_IOCSKEY     _IOW (HW4MOD_IOC_MAGIC,   1, char)
#define HW4MOD_IOCGKEY     _IOR (HW4MOD_IOC_MAGIC,   2, char)
#define HW4MOD_IOC_MAXNR                             2

#endif /* _HW4_MOD_H_ */
