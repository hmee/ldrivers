/* **************** LDD:2.0 s_02/lab1_chrdrv.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein, 2012
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://www.coopj.com
 *     email:  coop@coopj.com
 *
 * The primary maintainer for this code is Jerry Cooperstein
 * The CONTRIBUTORS file (distributed with this
 * file) lists those known to have contributed to the source.
 *
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 *
 */
/* 
Sample Character Driver 
@*/

#include <linux/module.h>	/* for modules */
#include <linux/fs.h>		/* file_operations */
#include <linux/uaccess.h>	/* copy_(to,from)_user */
#include <linux/init.h>		/* module_init, module_exit */
#include <linux/slab.h>		/* kmalloc */
#include <linux/cdev.h>		/* cdev utilities */
#include <linux/moduleparam.h>  /* for passing parameters */
#include <linux/stat.h>     /* read/write permission */
#include <linux/irq.h>     /* irq */
#include <linux/interrupt.h>     /* irq */
#include <linux/timer.h>     /* timer */
#include <linux/jiffies.h>     /* timer */
#include <linux/sched.h>    /* current */

#define MYDEV_NAME  "pdrive11_1"
#define IRQ_NUM     16

#define TIMER_PERIOD_MS (2000)

static dev_t first;
static unsigned int count = 1;
static int my_major = 700, my_minor = 0;
static struct cdev *my_cdev;
static struct timer_list timer;
int param = 1;
unsigned long last_jiffies;
module_param_named(param_name, param, int, S_IRUGO | S_IWUSR );


static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
};

void timer_function(unsigned long data)
{
    static int count = 0;
    long delta;
    unsigned long lj;

    lj =  last_jiffies;           // for this time
    last_jiffies +=  msecs_to_jiffies( TIMER_PERIOD_MS );  // for next time

    // if firing early
    if( time_after( lj, jiffies) ) {
        delta = lj - jiffies;
    } else {
        delta = jiffies - lj;
        delta = -delta;
    }
    mod_timer( &timer, last_jiffies + delta/2 );
    count++;
	pr_info("Timer (%u) fire, rearm it now!\n", count);
}

static int __init my_init(void)
{
	first = MKDEV(my_major, my_minor);
	register_chrdev_region(first, count, MYDEV_NAME);
	my_cdev = cdev_alloc();
	cdev_init(my_cdev, &mycdrv_fops);
	cdev_add(my_cdev, first, count);
	pr_info("\nSucceeded in registering character device %s with param_name = %d\n", 
            MYDEV_NAME,
            param );
    /* init timer */
    init_timer( &timer );
    timer.function = timer_function;
    timer.expires = jiffies + msecs_to_jiffies( TIMER_PERIOD_MS );
    last_jiffies =  timer.expires;
    add_timer( &timer );

	return 0;
}

static void __exit my_exit(void)
{
    // remove the timer
    del_timer_sync( &timer );

	cdev_del(my_cdev);
	unregister_chrdev_region(first, count);
	pr_info("\ndevice unregistered\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_LICENSE("GPL v2");
