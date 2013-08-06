#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */
#include <linux/fs.h>		/* Needed for register chrdev */
#include <linux/cdev.h>		/* cdev machros */
//#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>

#define DEV_NAME    "lab1_pdrv"
#define MAJOR_NUM   700
#define MINOR_NUM   0
#define COUNT       1

struct cdev *mycdev;

//#define TRY_DELAYED_WORK

#define BUF_SIZE 20

struct data_t {
    int num[BUF_SIZE];
    int r_ptr;  // index of the next read
    int w_ptr;  // index of the next write
#ifdef TRY_DELAYED_WORK
    struct delayed_work mywork;
#else
    struct work_struct mywork;
#endif
} irq_data;

//#ifdef TRY_DELAYED_WORK
//static void work_func(struct delayed_work *);
//#else
static void work_func(struct work_struct *);
//#endif

//DEFINE_MUTEX(wq_mutex);

//DECLARE_WORK(mywork, work_func);

irqreturn_t irq_handler(int irq, void *dev_id)
{
    static int count = 0;
    int tmp;

    irq_data.num[irq_data.w_ptr] = count;
    // update w_ptr
    tmp = irq_data.w_ptr + 1;
    irq_data.w_ptr = tmp % BUF_SIZE;

	printk(KERN_INFO "Device %s IRQ fired (%d)", DEV_NAME, count++);
#ifdef TRY_DELAYED_WORK
    schedule_delayed_work(&irq_data.mywork, HZ >> 2);   // 1/4
#else
    schedule_work( &irq_data.mywork );
#endif
    return IRQ_HANDLED;
}

//#ifdef TRY_DELAYED_WORK
//static void work_func(struct work_struct *w)
//#else
static void work_func(struct work_struct *w)
//#endif
{
    static int count = 0;
    // get pointer to data_t from ptr to work in the same struct
#ifdef TRY_DELAYED_WORK
    struct delayed_work *dw = container_of(w, struct delayed_work, work);
    struct data_t *data = container_of(dw, struct data_t, mywork);
#else
    struct data_t *data = container_of(w, struct data_t, mywork);
#endif
    printk(KERN_INFO "Queued work:\n");
    while ( data->r_ptr !=  data->w_ptr ) {
        printk(KERN_INFO "Queued work fired (%d) got num = %d\n", count, data->num[data->r_ptr]);
        data->r_ptr++;
        data->r_ptr %= BUF_SIZE;
        msleep(30);
    }
    count++;
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

#ifdef TRY_DELAYED_WORK
    INIT_DELAYED_WORK(&irq_data.mywork, work_func);
#else
    INIT_WORK(&irq_data.mywork, work_func);
#endif

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

