#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include "TestMasterSlave.h"
#include "console/console.h"

#define minor 0

MODULE_LICENSE("GPL");

static dev_t canftest_dev;
static struct cdev *canftest_cdev;
static struct task_struct *thread_start_p, *thread_stop_p;
static DECLARE_MUTEX (canftest_mutex);
static int canftest_stopped = 1;

int thread_start (void* data);
int thread_stop (void* data);

// handler processing write() requests from user-space
ssize_t canftest_write(struct file *filp, const char __user *buf, size_t count,
		       loff_t *f_pos)
{
	int cmd;

	// get integer from user-space
	if (get_user (cmd, buf))
		return -EFAULT;

	// process integer as command
	switch (cmd) {
		case CMD_START:
			if (!canftest_stopped) break;
			thread_start_p = kthread_create (thread_start, NULL, "canftest_start");

			if (PTR_ERR(thread_start_p) == -ENOMEM) {
				printk(KERN_WARNING "canftest: error creating start thread\n");
				return -ENOMEM;
			}

			wake_up_process (thread_start_p);
			break;

		case CMD_STOP:
			if (canftest_stopped) break;
			thread_stop_p = kthread_create (thread_stop, NULL, "canftest_stop");

			if (PTR_ERR(thread_stop_p) == -ENOMEM) {
				printk(KERN_WARNING "canftest: error creating stop thread\n");
				return -ENOMEM;
			}

			wake_up_process (thread_stop_p);
			break;

		// ignore new line character
		case 10:
			break;

		default:
			printk("canftest: bad command %d\n", cmd);
			break;
	}
	
	return count;
}

// register write() handler
static struct file_operations canftest_fops = {
	.owner =    THIS_MODULE,
	.write =    canftest_write,
};

// start TestMasterSlave example
int thread_start (void* data)
{
	int ret=0;
	
	down_interruptible (&canftest_mutex);
	
	ret=TestMasterSlave_start();

	// if started
	if (ret == 0) {
		canftest_stopped = 0;

		// increment module usage count
		try_module_get(THIS_MODULE);
	}
	
	up (&canftest_mutex);

	return ret;
}

// finish TestMasterSlave example
int thread_stop (void* data)
{
	down_interruptible (&canftest_mutex);

	TestMasterSlave_stop();
	canftest_stopped = 1;

	// decrement usage count
	module_put(THIS_MODULE);
	
	up (&canftest_mutex);

	return 0;
}

int init_module(void)
{
	int ret, major;

	// get major device number dynamically
	ret = alloc_chrdev_region(&canftest_dev, minor, 1, DEVICE_NAME);
	major = MAJOR(canftest_dev);
	if (ret < 0) {
		printk(KERN_WARNING "canftest: can't get major %d\n", major);
		return ret;
	}
	
	canftest_cdev = cdev_alloc( );
	canftest_cdev->owner = THIS_MODULE;
	canftest_cdev->ops = &canftest_fops;
	
	// register new character device
	ret = cdev_add (canftest_cdev, canftest_dev, 1);
	if (ret) {
		printk(KERN_WARNING "canftest: error %d adding char device\n", ret);
		return ret;
	}

	return 0;
}

void cleanup_module(void)
{
	// unregister major device number and character device
	unregister_chrdev_region(canftest_dev, 1);
	cdev_del(canftest_cdev);
}
