#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */
#include <linux/fs.h>		/* Needed for register chrdev */
#include <linux/cdev.h>		/* cdev machros */
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/smp.h>      /* smp_processor_id() */

#define DEV_NAME    "lab1_pdrv"
#define MAJOR_NUM   700
#define MINOR_NUM   0
#define COUNT       1

struct cdev *mycdev;
struct task_struct *task;
struct semaphore sem;

#define BUF_SIZE 4

struct data_t {
    int num[BUF_SIZE];
    int r_ptr;  // index of the next read
    int w_ptr;  // index of the next write
} irq_data;


irqreturn_t irq_handler(int irq, void *dev_id)
{
    static int count = 0;
    int tmp;

    irq_data.num[irq_data.w_ptr] = count;
    // update w_ptr
    tmp = irq_data.w_ptr + 1;
    irq_data.w_ptr = tmp % BUF_SIZE;

	printk(KERN_INFO "Device %s IRQ fired (%d)", DEV_NAME, count++);
    // put the sempahore
    up(&sem);

    return IRQ_HANDLED;
}

static int kthread_func(void * d)
{
    static int count = 0;
    struct data_t *data = (struct data_t *) d;
    printk(KERN_INFO "Start the kernel thread [cpu %d]\n", smp_processor_id());


    while(1) {
        // wait for up that happens in ISR
        down(&sem);

        if(kthread_should_stop()) {
            break;
        }

        printk(KERN_INFO "Tasklet fired (%d) got num = %d\n", count, data->num[data->r_ptr]);
        data->r_ptr++;
        data->r_ptr %= BUF_SIZE;
        mdelay(30);     // try delay
        count++;
    };

    printk(KERN_INFO "End of the kernel thread\n");

    return 0;
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

    irq_data.r_ptr = irq_data.w_ptr = 0;

    // init the semaphore in locked state
    sema_init(&sem, 0);

    // TODO: creat and start kernel_thread here
    task = kthread_create(kthread_func, (void *) &irq_data, "pdrv_thread");
    wake_up_process(task);  // let it run
    
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
    // TODO: end kernel_thread here
    up(&sem);   // to make the thread proceed
    kthread_stop(task);

	printk(KERN_INFO "Device is unregisterd\n");
}

module_init(lab1_init);
module_exit(lab1_exit);

MODULE_LICENSE("GPL v2");

