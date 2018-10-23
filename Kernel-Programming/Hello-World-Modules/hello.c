/*                                                     
 * hello.c - The Hello, World! Kernel Module
 *           from Linux Kernel Development, 3rd Ed, Robert Love
 */                                                    

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

/*                                                        
 * hello_init - The init function, called when the module is loaded.
 *              It returns 0 when successfully loaded; non-zero otherwise.
 */                                                       
                                                          
static int hello_init(void) { 
	printk(KERN_ALERT "Hello, World\n"); 
	return 0; 
}

/*                                                        
 * hello_exit - The exit function, called when the module is removed.
 */                                                       
                                                          
static void hello_exit(void) { 
	printk(KERN_ALERT "Goodbye cruel world\n"); 
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Love");
MODULE_DESCRIPTION("Hello World Module");

