#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/percpu-defs.h>
#include <linux/delay.h>
#include <linux/filter.h>
#include <linux/ioctl.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/netdevice.h>
#include <linux/device.h>
#include <linux/fd.h>
#include <linux/path.h>
#include <linux/mount.h>
#include <linux/net_namespace.h>
#include <linux/net.h>

#define WR_VALUE _IOW('a','a',struct message_to_user*)
#define RD_VALUE _IOR('a','b',struct message_to_user*)

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Stab linux module for operating system's lab");
MODULE_VERSION("1.0");

dev_t dev = 0;

static struct class *dev_class;
static struct cdev etx_cdev;

struct vfsmount_cut {
    int mnt_flags;
    unsigned char s_blocksize_bits;
    unsigned long s_blocksize;
    int s_count;
    loff_t s_maxbytes;
    dev_t s_dev;
};

struct socket_cut {
    socket_state state;
    short type;
    unsigned long flags;
};

// kernel --message--> user space
struct message_to_user { 
    struct vfsmount_cut vfs_cut;
    struct socket_cut socket_cut;
};

struct message_to_kernel {
    int fd;
    int socketfd;
};

static int      __init kmod_init(void);
static void     __exit kmod_exit(void);
static int      etx_open(struct inode *inode, struct file *file);
static int      etx_release(struct inode *inode, struct file *file);
static ssize_t  etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct message_to_user* read_structs(void);

static struct file_operations fops = {
    .owner          = THIS_MODULE,
    .read           = etx_read,
    .write          = etx_write,
    .open           = etx_open,
    .unlocked_ioctl = etx_ioctl,
    .release        = etx_release,
};

static int etx_open(struct inode *inode, struct file *file) {
    pr_info("Device file opened\n");
    return 0;
}

static int etx_release(struct inode *inode, struct file *file) {
    pr_info("Device file closed\n");
    return 0;
}

static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    pr_info("Reading from device file\n");
    return 0;
}

static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    pr_info("Writing to device file\n");
    return len;
}

static int fd = 0;
static int socketfd = 0;

static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct message_to_kernel msg_to_kern; 
    struct message_to_user* msg;

    switch(cmd) {
        case WR_VALUE:
            if (copy_from_user(&msg_to_kern ,(int *) arg, sizeof(struct message_to_kernel))) {
                pr_err("Failed to write data to user space\n");
            }
            fd = msg_to_kern.fd;
            socketfd = msg_to_kern.socketfd;
            return 0;
        case RD_VALUE:
            msg = read_structs();
            if (copy_to_user((struct message_to_user*) arg, msg, sizeof(struct message_to_user))) {
                pr_err("Failed to read data from user space\n");
            }
            return 0;
        default:
            pr_err("Unknown command\n"); 
            return 0;
    }
}

static int __init kmod_init(void) {
    printk(KERN_INFO "kmod: module loaded\n");

    // Allocating Major number
    if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) < 0){
        pr_err("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

    // Creating cdev structure
    cdev_init(&etx_cdev, &fops);

    // Adding character device to the system
    if ((cdev_add(&etx_cdev,dev,1)) < 0) {
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }

    // Creating struct class
    if ((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL) {
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    // Creating device
    if ((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL) {
        pr_err("Cannot create the Device 1\n");
        goto r_device;
    }
    pr_info("Device driver insert done\n");

    return 0;

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    return -1;
}

static struct message_to_user* read_structs(void) {
    struct message_to_user* msg = vmalloc(sizeof(struct message_to_user));

    // vfsmount
    struct fd f = fdget(fd);
    if(!f.file) {
        printk(KERN_INFO "kmod: error opening file by descriptor\n");
    }
    
    struct vfsmount *vfs = f.file->f_path.mnt;
    struct super_block *sb = vfs->mnt_sb;
    msg->vfs_cut = (struct vfsmount_cut) {
        .mnt_flags = vfs->mnt_flags,
        .s_blocksize_bits = sb->s_blocksize_bits,
        .s_blocksize = sb->s_blocksize,
        .s_count = sb->s_count,
        .s_maxbytes = sb->s_maxbytes,
        .s_dev = sb->s_dev
    };
    fdput(f);

    // socket
    int err = 0;
    struct socket *s = sockfd_lookup(socketfd, &err);
    if (err < 0) {
        printk(KERN_INFO "kmod: error on socket lookup\n");
    }
    msg->socket_cut = (struct socket_cut) {
        .state = s->state,
        .type = s->type,
        .flags = s->flags
    };

    return msg;
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
