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

#define MYDEV_NAME  "pdrive"
#define IRQ_NUM     16

static char *ramdisk;
#define ramdisk_size (size_t) (16*PAGE_SIZE)

static dev_t first;
static unsigned int count = 1;
static int my_major = 700, my_minor = 0;
static struct cdev *my_cdev;
int param = 1;
int irq_count = 0;
module_param_named(param_name, param, int, S_IRUGO | S_IWUSR );
module_param_named(irq_count, irq_count, int, S_IRUGO );

static int mycdrv_open(struct inode *inode, struct file *file)
{
	pr_info(" OPENING device: %s: (with param_name = %d)\n\n", MYDEV_NAME, param);
	return 0;
}

static int mycdrv_release(struct inode *inode, struct file *file)
{
	pr_info(" CLOSING device: %s:\n\n", MYDEV_NAME);
	return 0;
}

static ssize_t
mycdrv_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
	int nbytes;
	if ((lbuf + *ppos) > ramdisk_size) {
		pr_info("trying to read past end of device,"
			"aborting because this is just a stub!\n");
		return 0;
	}
	nbytes = lbuf - copy_to_user(buf, ramdisk + *ppos, lbuf);
	*ppos += nbytes;
	pr_info("\n READING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	return nbytes;
}

static ssize_t
mycdrv_write(struct file *file, const char __user * buf, size_t lbuf,
	     loff_t * ppos)
{
	int nbytes;
	if ((lbuf + *ppos) > ramdisk_size) {
		pr_info("trying to read past end of device,"
			"aborting because this is just a stub!\n");
		return 0;
	}
	nbytes = lbuf - copy_from_user(ramdisk + *ppos, buf, lbuf);
	*ppos += nbytes;
	pr_info("\n WRITING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	return nbytes;
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_read,
	.write = mycdrv_write,
	.open = mycdrv_open,
	.release = mycdrv_release,
};

static irqreturn_t irq_routine(int irq, void *dev_id)
{
    if(irq != 16) {
        pr_info("\nReceiving wrong irq num = %d\n", irq);
        return IRQ_NONE;
    } else {
        irq_count++;
        pr_info("\nReceiving irq 16 with irq_count = %d\n", irq_count);
        return IRQ_HANDLED;
    }
}

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
    /* request/register an interrupt routine, shared with audio '16' */
    i = request_irq( IRQ_NUM, irq_routine, IRQF_SHARED | IRQF_PROBE_SHARED,
                    "pdrv", &irq_count); 
    if( i != 0) {
        pr_info("\nFailed registering nterrupt with return = %d\n", i);
    }

	return 0;
}

static void __exit my_exit(void)
{
    /* unregister the irq handle */
    synchronize_irq( IRQ_NUM );
    free_irq( IRQ_NUM, &irq_count );

	cdev_del(my_cdev);
	unregister_chrdev_region(first, count);
	pr_info("\ndevice unregistered\n");
	kfree(ramdisk);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_LICENSE("GPL v2");
