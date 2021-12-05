#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/percpu-defs.h>
#include <linux/delay.h>

#include <linux/filter.h>
#include <linux/ioctl.h>
#include <linux/pid.h>
#include <linux/sched.h>

#include <linux/signal.h>

#include <linux/netdevice.h>
#include <linux/device.h>

#define WR_VALUE _IOW('a','a',struct message*)
#define RD_VALUE _IOR('a','b',struct message*)


MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Stab linux module for operating system's lab");
MODULE_VERSION("1.0");



dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

struct signal_info {
    int nr_threads;
    int prio;
    unsigned int flags;
    unsigned int sigHandlersAddr[64];
};

struct n_dev_info {
    unsigned int size;
    char name[10][16];
};

struct message {
    struct signal_info si;
    struct n_dev_info ndi;
};

struct task_struct *ts1;

struct signal_info* si;
struct n_dev_info* ndi;
struct message* msg;

struct net_device *n_dev;


static int      __init kmod_init(void);
static void     __exit kmod_exit(void);
static int      etx_open(struct inode *inode, struct file *file);
static int      etx_release(struct inode *inode, struct file *file);
static ssize_t  etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

void fill_structs(void);
/*
** File operation sturcture
*/
static struct file_operations fops =
        {
                .owner          = THIS_MODULE,
                .read           = etx_read,
                .write          = etx_write,
                .open           = etx_open,
                .unlocked_ioctl = etx_ioctl,
                .release        = etx_release,
        };
/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}
/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{

    pr_info("Device File Closed...!!!\n");
    return 0;
}
/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
pr_info("Read Function\n");
return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
pr_info("Write function\n");
return len;
}

int pid = 0;


static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{



    switch(cmd) {
        case WR_VALUE:
            if( copy_from_user(&pid ,(int*) arg, sizeof(pid)) )
            {
                pr_err("Data Write : Err!\n");
            }
            pr_info("Pid = %d\n", pid);
            break;
        case RD_VALUE:
            fill_structs();
            if( copy_to_user((struct message*) arg, msg, sizeof(struct message)) )
            {
                pr_err("Data Read : Err!\n");
            }

            break;
        default:
            pr_info("Default\n");
            break;
    }
    return 0;
}


static int __init kmod_init(void) {
    printk(KERN_INFO "kmod: module loaded.\n");

    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
        pr_err("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

    /*Creating cdev structure*/
    cdev_init(&etx_cdev,&fops);

    /*Adding character device to the system*/
    if((cdev_add(&etx_cdev,dev,1)) < 0){
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }

    /*Creating struct class*/
    if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    /*Creating device*/
    if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL){
        pr_err("Cannot create the Device 1\n");
        goto r_device;
    }
    pr_info("Device Driver Insert...Done!!!\n");



    return 0;

    r_device:
    class_destroy(dev_class);
    r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}

void fill_structs() {

    ts1 = get_pid_task(find_get_pid(pid), PIDTYPE_PID);

    si = vmalloc(sizeof(struct signal_info));
    si->nr_threads = ts1->signal->nr_threads;
    si->prio = ts1->prio;
    si->flags = ts1->signal->flags;

    int i;

    for (i = 0; i<64; i++)
        si->sigHandlersAddr[i] = (unsigned int) ts1->sighand->action[i].sa.sa_handler;


    ndi = vmalloc(sizeof(struct n_dev_info));

    read_lock(&dev_base_lock);
    n_dev = first_net_device(&init_net);
    int c = 0;
    while (n_dev) {
        strcpy((ndi->name)[c], n_dev->name);
        n_dev = next_net_device(n_dev);
        c++;
    }

    read_unlock(&dev_base_lock);

    ndi->size = c;

    msg = vmalloc(sizeof(struct message));
    msg->si = *si;
    msg->ndi = *ndi;

}

static void __exit kmod_exit(void) {
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "kmod: module unloaded\n");
}

module_init(kmod_init);
module_exit(kmod_exit);
