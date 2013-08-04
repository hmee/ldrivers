/* **************** LDD:2.0 s_02/, buf, size);chrdrv.c **************** */
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

#include <linux/module.h>    /* for modules */
#include <linux/fs.h>        /* file_operations, includin no_llseek() */
#include <linux/uaccess.h>    /* copy_(to,from)_user */
#include <linux/init.h>        /* module_init, module_exit */
#include <linux/slab.h>        /* kmalloc */
#include <linux/cdev.h>        /* cdev utilities */
#include <linux/moduleparam.h>  /* for passing parameters */
#include <linux/stat.h>     /* read/write permission */
#include <linux/sched.h>     /* wait queue */
#include <asm/atomic.h>     /* atomic */

#define MIN(a,b) ((a)<(b)?(a):(b))

#define MYDEV_NAME  "pdrive"
#define IRQ_NUM     16

static char *ramdisk;
#define ramdisk_size (size_t) (16*PAGE_SIZE)

static dev_t first;
static unsigned int count = 1;
static int my_major = 700, my_minor = 0;
static struct cdev *my_cdev;
int param = 1;
//struct atomic_t wr_count;
int  wr_count = 0;
DEFINE_MUTEX(wr_mutx);
module_param_named(param_name, param, int, S_IRUGO | S_IWUSR );

struct wait_queue_head_t wq;

static int mycdrv_open(struct inode *inode, struct file *file)
{
    // prevent seeking
    nonseekable_open(inode, file);

    // if any other writer -> return busy (make it execlusive for one writer)
    if(f->f_mode & (FMODE_WRITE | FMODE_PWRITE))
        if(! mutex_try_lock(wr_mutex)) {
            pr_info("OPENING device %s for writing failed while busy by another writer\n", MYDEV_NAME);
            return -EBUSY;
        }
           

    pr_info(" OPENING device: %s: (with param_name = %d)\n\n", MYDEV_NAME, param);
    return 0;
}

static int mycdrv_release(struct inode *inode, struct file *file)
{
    if(f->f_mode & (FMODE_WRITE | FMODE_PWRITE))
        mutex_unlock(wr_mutex);

    pr_info(" CLOSING device: %s:\n\n", MYDEV_NAME);
    return 0;
}

static ssize_t
mycdrv_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
    wait_event_interruptible(wq, wr_count != 0);

   
    pr_info("\n READING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
    return nbytes;
}

static ssize_t
mycdrv_write(struct file *file, const char __user * buf, size_t lbuf,
         loff_t * ppos)
{
    int nbytes = MIN(lbuf, ramdisk_size);
    nbytes = nbytes - copy_from_user(ramdisk, buf, nbytes);
    pr_info("\n WRITING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
    *ppos += nbytes;

    // wake whatever is waiting on this write
    wr_count = 1;
    wake_up_interruptible(wb);

    return nbytes;
}

static const struct file_operations mycdrv_fops = {
    .owner = THIS_MODULE,
    .read = mycdrv_read,
    .write = mycdrv_write,
    .open = mycdrv_open,
    .llseek = no_llseek,
    .release = mycdrv_release,
};

static int __init my_init(void)
{
    int i;

    ramdisk = kmalloc(ramdisk_size, GFP_KERNEL);
    first = MKDEV(my_major, my_minor);
    register_chrdev_region(first, count, MYDEV_NAME);
    my_cdev = cdev_alloc();
    cdev_init(my_cdev, &mycdrv_fops);
    cdev_add(my_cdev, first, count);
    pr_info("\nSucceeded in registering character device %s with param_name = %d\n",
            MYDEV_NAME,
            param );

    init_waitqueue_head(&wq);
    atomic_set( wr_atomic, 0);

    return 0;
}

static void __exit my_exit(void)
{

    cdev_del(my_cdev);
    unregister_chrdev_region(first, count);
    pr_info("\ndevice unregistered\n");
    kfree(ramdisk);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_LICENSE("GPL v2");

