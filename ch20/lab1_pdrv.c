#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */
#include <linux/fs.h>		/* Needed for register chrdev */
#include <linux/cdev.h>		/* cdev machros */
#include <linux/semaphore.h>
#include <linux/interrupt.h>

#define DEV_NAME    "lab1_pdrv"
#define MAJOR_NUM   700
#define MINOR_NUM   0
#define COUNT       1

DEFINE_SEMAPHORE(my_sem);
EXPORT_SYMBOL(my_sem);

struct cdev *mycdev;

struct data_t {
    int num;
} irq_data;

static void tasklet_func(unsigned long);

DECLARE_TASKLET(mytasklet, tasklet_func, (unsigned int) &irq_data);


irqreturn_t irq_handler(int irq, void *dev_id)
{
    static int count = 0;
    irq_data.num = count;
	printk(KERN_INFO "Device %s IRQ fired (%d)", DEV_NAME, count++);
    tasklet_schedule( &mytasklet );
    return IRQ_HANDLED;
}

static void tasklet_func(unsigned long d)
{
    static int count = 0;
    struct data_t *data = (struct data_t *) d;
	printk(KERN_INFO "Tasklet fired (%d) got num = %d\n", count++, data->num);

}

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

    rt = request_irq(19, 
                irq_handler,
                IRQF_SHARED,
                "pdrv",
                (void *) &irq_data
                );

    if(rt != 0)
        printk(KERN_INFO "Device %s failed to request IRQ(rt = %d)", DEV_NAME, rt);

	printk(KERN_INFO "Device %s is registered [rt = %u]\n", DEV_NAME, rt);
	return 0;
}

static void __exit lab1_exit(void)
{
    unregister_chrdev_region(MKDEV(MAJOR_NUM, MINOR_NUM), COUNT);
    cdev_del(mycdev);

    free_irq(19, (void *) &irq_data);

	printk(KERN_INFO "Device is unregisterd\n");
}

module_init(lab1_init);
module_exit(lab1_exit);

MODULE_LICENSE("GPL v2");

