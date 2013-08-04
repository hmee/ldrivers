#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */
#include <linux/fs.h>		/* Needed for register chrdev */
#include <linux/cdev.h>		/* cdev machros */
#include <linux/semaphore.h>

#define DEV_NAME    "lab1_pdrv"
#define MAJOR_NUM   700
#define MINOR_NUM   0
#define COUNT       1

// DEFINE_SEMAPHORE(my_sem);   // newer kernel
DECLARE_MUTEX(my_sem);   // old kernel
EXPORT_SYMBOL(my_sem);

struct cdev *mycdev;

struct file_operations fops = {
    .owner = THIS_MODULE,
};

static int __init lab1_init(void)
{
    int rt;

    register_chrdev_region(MKDEV(MAJOR_NUM, MINOR_NUM), COUNT, DEV_NAME);
    mycdev = cdev_alloc();
    cdev_init(mycdev, &fops);
    rt = cdev_add(mycdev, MKDEV(MAJOR_NUM, MINOR_NUM), COUNT);

	printk(KERN_INFO "Device %s is registered [rt = %u]\n", DEV_NAME, rt);
	return 0;
}

static void __exit lab1_exit(void)
{
    unregister_chrdev_region(MKDEV(MAJOR_NUM, MINOR_NUM), COUNT);
    cdev_del(mycdev);
	printk(KERN_INFO "Device is unregisterd\n");
}

module_init(lab1_init);
module_exit(lab1_exit);

MODULE_LICENSE("GPL v2");

