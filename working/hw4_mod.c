/*
 * hw4mod.c -- the bare hw4mod char module
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 * Modified by Keith Shomper, 10/27/2017 for use in CS3320
 *
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>      /* kmalloc() */
#include <linux/fs.h>      /* everything... */
#include <linux/errno.h>   /* error codes */
#include <linux/types.h>   /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>   /* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>

#include <linux/uaccess.h> /* needed for some reason*/
#include <asm/uaccess.h>   /* copy_*_user */
#include <linux/sched.h>

#include "hw4_mod.h"         /* local definitions */

/*
 * Our parameters which can be set at load time.
 */
int hw4mod_major   = HW4MOD_MAJOR;
int hw4mod_minor   = 0;
int hw4mod_nr_devs = HW4MOD_NR_DEVS;

module_param(hw4mod_major,   int, S_IRUGO);
module_param(hw4mod_minor,   int, S_IRUGO);
module_param(hw4mod_nr_devs, int, S_IRUGO);

MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet modified K. Shomper");
MODULE_LICENSE("Dual BSD/GPL");

/* the set of devices allocated in hw4mod_init_module */
struct hw4mod_dev *hw4mod_devices = NULL;

/*
 * Open: to open the device is to initialize it for the remaining methods.
 */
int hw4mod_open(struct inode *inode, struct file *filp) {

   int uid = get_current_user()->uid.val;
   uid -= 1000;

   /* the device this function is handling (one of the hw4mod_devices) */
   struct hw4mod_dev *dev;

   /* we need the hw4mod_dev object (dev), but the required prototpye
      for the open method is that it receives a pointer to an inode.
      now an inode contains a struct cdev (the field is called
      i_cdev) and we can use this field with the container_of macro
      to obtain the hw4mod_dev object (since hw4mod_dev also contains
      a cdev object.
    */
   dev = container_of(inode->i_cdev, struct hw4mod_dev, cdev);

   /* so that we don't need to use the container_of() macro repeatedly,
      we save the handle to dev in the file's private_data for other methods.
    */
   filp->private_data = dev;

   /* grab the semaphore, so the call to trim() is atomic */
   if (down_interruptible(&dev->sem)) return -ERESTARTSYS;

   dev->pwd_vault.uhpw_data[uid].data;
   
   if(dev->pwd_vault.uhpw_data[uid].num_hints > 0){
     dev->pwd_vault.uhpw_data[uid].fp = dev->pwd_vault.uhpw_data[uid].data[0];
   } else {
     dev->pwd_vault.uhpw_data[uid].fp = NULL;
   }

   /* release the semaphore */
   up(&dev->sem);

   return 0;
}

/*
 * Release: release is the opposite of open, so it deallocates any
 *          memory allocated by hw4mod_open and shuts down the device.
 *          since open didn't allocate anything and our device exists
 *          only in memory, there are no actions to take here.
 */
int hw4mod_release(struct inode *inode, struct file *filp) {
   return 0;
}


/*
 * Read: implements the read action on the device by reading count
 *       bytes into buf beginning at file position f_pos from the file 
 *       referenced by filp.  The attribute "__user" is a gcc extension
 *       that indicates that buf originates from user space memory and
 *       should therefore not be trusted.
 */
ssize_t hw4mod_read(struct file *filp, char __user *buf, size_t count,
                    loff_t *f_pos) {

   struct hw4mod_dev  *dev  = filp->private_data; 
   ssize_t retval   = 0;

   /* acquire the semaphore */
   if (down_interruptible(&dev->sem)) return -ERESTARTSYS;

   /* if the read position is beyond the end of the file, then goto exit
    * note that we can't simply return, because we are holding the
    * semaphore, "goto out" provides a single exit point that allows for
    * releasing the semaphore.
    */
   struct hpw_list *filePtr = dev->pwd_vault.uhpw_data->fp;

   if(filePtr != NULL){
     
   }

   up(&dev->sem);
   return retval;
}

/*
 * Write: implements the write action on the device by writing count
 *        bytes from buf into the "file" referenced by filp beginning at the 
 *        file position f_pos.  The attribute "__user" indicates that buf 
 *        originates from user space memory and should therefore not be trusted.
 */
ssize_t hw4mod_write(struct file *filp, const char __user *buf, size_t count,
                     loff_t *f_pos) {

   struct hw4mod_dev  *dev  = filp->private_data;
   ssize_t retval   = -ENOMEM;         /* value used in "goto out" statements */

   if (down_interruptible(&dev->sem)) return -ERESTARTSYS;
   
   int uid = get_current_user()->uid.val;
   uid -= 999;
   struct hpw_list *filePtr = dev->pwd_vault.uhpw_data[uid-1].fp;
   
   char *hint, *password, *temp;
   
   if(strcmp(buf, "") == 0){
     printk(KERN_WARNING "<3> Empty string\n");
     dump_vault (&dev->pwd_vault, FORWARD);
     //delete_from_list(&dev->pwd_vault.uhpw_data->fp);
     if(filePtr != NULL){
       struct hpw_list *next_Ptr = filePtr->next;
       hint = filePtr->hpw.hint;
       password = filePtr->hpw.pwd;
       delete_pair(&dev->pwd_vault, uid, hint, password);
     }
     struct hpw_list *next_Ptr = filePtr->next;
     
     dump_vault (&dev->pwd_vault, FORWARD);

     //here we need to delete a pair and move fp
   }else {
     printk(KERN_WARNING "<3> Not Empty Buf\n");
     hint = "hint";
     password = "password";
     printk(KERN_WARNING "%s\n",buf);
     printk(KERN_WARNING "hint: %s\n", hint);
     printk(KERN_WARNING "password: %s\n", password);

     insert_pair (&dev->pwd_vault, uid, hint, password);
     dump_vault (&dev->pwd_vault, FORWARD);
     //sscanf(buf, "%s %s", hint, password);
     password = strchr(buf, ' ');
     password++;
     printk(KERN_WARNING "password: %s\n", password);
     hint = buf;
     char * tmp = password;
     tmp[-1] = '\0';
     insert_pair (&dev->pwd_vault, uid, hint, password);
     dump_vault (&dev->pwd_vault, FORWARD);
     insert_pair (&dev->pwd_vault, uid, "hint", "new");
     dump_vault (&dev->pwd_vault, FORWARD);

   }

   up(&dev->sem);
   return retval;
}

/*
 * Ioctl:  the ioctl() call is the "catchall" device function; its purpose
 *         is to provide device control through a single standard function
 *         call.  It accomplishes this via a command value and an arg
 *         parameter which indicates which action to take.
 */
long hw4mod_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {

   int err    = 0, tmp;
   int retval = 0;

   /*
    * the direction is a bitmask, and VERIFY_WRITE catches R/W
    * transfers. `Type' is user-oriented, while
    * access_ok is kernel-oriented, so the concept of "read" and
    * "write" is reversed
    */

    if(cmd == SCULL_IOCSQUANTUM){
      if (! capable (CAP_SYS_ADMIN))
          return -EPERM;
      retval = __get_user(scull_quantum, (int __user *)arg);
    }

      /* Tell: arg is the value */


   return retval;
}



/*
 * Seek:  the only one of the "extended" operations which hw4mod implements.
 */
loff_t hw4mod_llseek(struct file *filp, loff_t off, int whence) {

   struct hw4mod_dev *dev    = filp->private_data;
   loff_t            newpos;

   /* reset the file position as is standard */
   switch(whence) {
     case 0: /* SEEK_SET */
      newpos = off;
      break;

     case 1: /* SEEK_CUR */
      newpos = filp->f_pos + off;
      break;

     default: /* can't happen */
      return -EINVAL;
   }

   /* set the postion and return */
   filp->f_pos = newpos;
   return newpos;
}

/* this assignment is what "binds" the template file operations with those that
 * are implemented herein.
 */
struct file_operations hw4mod_fops = {
   .owner =    THIS_MODULE,
   .llseek =   hw4mod_llseek,
   .read =     hw4mod_read,
   .write =    hw4mod_write,
   .unlocked_ioctl = hw4mod_ioctl,
   .open =     hw4mod_open,
   .release =  hw4mod_release,
};

/*
 * Finally, the module stuff
 */

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
void hw4mod_cleanup_module(void) {

   dev_t devno = MKDEV(hw4mod_major, hw4mod_minor);

   /* if the devices were succesfully allocated, then the referencing pointer
    * will be non-NULL.
    */
   if (hw4mod_devices != NULL) {

      /* Get rid of our char dev entries by first deallocating memory and then
       * deleting them from the kernel */
      int i;
      for (i = 0; i < hw4mod_nr_devs; i++) {
	 finalize_vault(&(hw4mod_devices+i)->pwd_vault);
         cdev_del(&hw4mod_devices[i].cdev);
      }

      /* free the referencing structures */
      kfree(hw4mod_devices);
   }

   /* cleanup_module is never called if registering failed */
   unregister_chrdev_region(devno, hw4mod_nr_devs);

   printk(KERN_NOTICE "hw4mod unloaded\n");
}


/*
 * Set up the char_dev structure for this device.
 */
static void hw4mod_setup_cdev(struct hw4mod_dev *dev, int index) {
   int err, devno = MKDEV(hw4mod_major, hw4mod_minor + index);
    
   /* cdev_init() and cdev_add() are kernel-required initialization */
   cdev_init(&dev->cdev, &hw4mod_fops);
   dev->cdev.owner = THIS_MODULE;
   dev->cdev.ops   = &hw4mod_fops;
   err             = cdev_add (&dev->cdev, devno, 1);

   /* Fail gracefully if need be */
   if (err) printk(KERN_NOTICE "Error %d adding hw4mod%d", err, index);

   printk(KERN_NOTICE "hw4mod device setup\n");
}


int hw4mod_init_module(void) {
   int result, i;
   dev_t dev = 0;

   /*
    * Compile-time default for major is zero (dynamically assigned) unless 
    * directed otherwise at load time.  Also get range of minors to work with.
    */
   if (hw4mod_major == 0) {
      result      = alloc_chrdev_region(&dev,hw4mod_minor,hw4mod_nr_devs,"hw4mod");
      hw4mod_major = MAJOR(dev);
   } else {
      dev    = MKDEV(hw4mod_major, hw4mod_minor);
      result = register_chrdev_region(dev, hw4mod_nr_devs, "hw4mod");
   }

   /* report failue to aquire major number */
   if (result < 0) {
      printk(KERN_WARNING "hw4mod: can't get major %d\n", hw4mod_major);
      return result;
   }

   /* 
    * allocate the devices -- we can't have them static, as the number
    * can be specified at load time
    */
   hw4mod_devices = kmalloc(hw4mod_nr_devs*sizeof(struct hw4mod_dev), GFP_KERNEL);

   /* exit if memory allocation fails */
   if (!hw4mod_devices) {
      result = -ENOMEM;
      goto fail;
   }

   /* otherwise, zero the memory */
   memset(hw4mod_devices, 0, hw4mod_nr_devs * sizeof(struct hw4mod_dev));

   /* Initialize each device. */
   for (i = 0; i < hw4mod_nr_devs; i++) {
      initialize_vault(&hw4mod_devices[i].pwd_vault, HW4MOD_MAX_USERS_IN_VAULT);
      sema_init(&hw4mod_devices[i].sem, 1);
      hw4mod_setup_cdev(&hw4mod_devices[i], i);
   }

   printk(KERN_NOTICE "hw4mod loaded\n");

   /* succeed */
   return 0;

   /* failure, so cleanup is necessary */
  fail:
   hw4mod_cleanup_module();
   return result;
}


/* identify to the kernel the entry points for initialization and release, these
 * functions are called on insmod and rmmod, respectively
 */
module_init(hw4mod_init_module);
module_exit(hw4mod_cleanup_module);
