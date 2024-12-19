#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>   // 处理用户空间和内核空间之间的数据访问
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/wait.h>

// 定义主次设备号
#ifndef NTYCHANNEL_MAJOR
#define NTYCHANNEL_MAJOR 96
#endif

// 注册的设备数量
#ifndef NTYCHANNEL_NR_DEVS
#define NTYCHANNEL_NR_DEVS 2
#endif

// 每个设备私有空间的大小
#ifndef NTYCHANNEL_SIZE
#define NTYCHANNEL_SIZE 4096
#endif

// 设备私有结构体
struct ntychannel {
    char *data;              // 私有空间
    unsigned long size;      // 空间大小
    wait_queue_head_t inq;   // 等待队列
};

static int channel_major = NTYCHANNEL_MAJOR;
module_param(channel_major, int, S_IRUGO);

struct ntychannel *channel_devp;
struct cdev cdev;

char have_data = 0;

int ntychannel_open(struct inode *inode, struct file *filp);
int ntychannel_release(struct inode *inode, struct file *filp);
ssize_t ntychannel_read(struct file *filp, char __user *buffer, size_t size, loff_t *ppos);
ssize_t ntychannel_write(struct file *filp, const char __user *buffer, size_t size, loff_t *ppos);
unsigned int ntychannel_poll(struct file *filp, struct poll_table_struct *wait);
loff_t ntychannel_llseek(struct file *filp, loff_t offset, int whence);
int ntychannel_mmap(struct file *filp, struct vm_area_struct *vma);

// 打开设备
//inode文件系统层面的索引节点指针：在 Linux 文件系统中，inode（索引节点）是一个关键的数据结构。每个文件和目录都有对应的inode，它存储了文件的元数据信息，如文件大小、所有者、权限、时间戳（包括创建时间、修改时间和访问时间），以及文件数据块的指针等
//file用于代表打开的文件信息和存储设备相关的私有数据和操作上下文
int ntychannel_open(struct inode *inode, struct file *filp) {
    struct ntychannel *channel;
    int num = MINOR(inode->i_rdev);
    if (num >= NTYCHANNEL_NR_DEVS)
        return -ENODEV;

    channel = &channel_devp[num];
    filp->private_data = channel;

    return 0;
}

// 释放设备
int ntychannel_release(struct inode *inode, struct file *filp) {
    return 0;
}

// 读取操作
ssize_t ntychannel_read(struct file *filp, char __user *buffer, size_t size, loff_t *ppos) {
    unsigned long p = *ppos;
    unsigned int count = size;
    int ret = 0;

    struct ntychannel *channel = filp->private_data;
    if (p >= NTYCHANNEL_SIZE) {
        return 0;
    }

    if (count > NTYCHANNEL_SIZE - p) {
        count = NTYCHANNEL_SIZE - p;
    }

    while (!have_data) {
        if (filp->f_flags & O_NONBLOCK) {
            return -EAGAIN;
        }

        ret = wait_event_interruptible(channel->inq, have_data);
        if (ret < 0) {
            return ret;
        }
    }

    if (copy_to_user(buffer, (void *)(channel->data + p), count)) {
        ret = -EFAULT;
    } else {
        ret = count;
        channel->size -= ret;
        printk(KERN_INFO "read %d byte(s) from %ld\n", ret, p);
    }

    have_data = 0;
    return ret;
}

// 写入操作
ssize_t ntychannel_write(struct file *filp, const char __user *buffer, size_t size, loff_t *ppos) {
    int ret = 0;
    unsigned long p = *ppos;
    unsigned int count = size;
    
    struct ntychannel *channel = filp->private_data;
    if (p >= NTYCHANNEL_SIZE) {
        return 0;
    }

    if (count > NTYCHANNEL_SIZE - p) {
        count = NTYCHANNEL_SIZE - p;
    }

    if (copy_from_user(channel->data + p, buffer, count)) {
        return -EFAULT;
    } else {
        *ppos += count;
        ret = count;
        channel->size += count;
        *(channel->data + p + count) = '\0'; 
        printk(KERN_INFO "written %d byte(s) from %ld\n", count, p);
    }

    have_data = 1;
    wake_up(&channel->inq);

    return ret;
}

// 多路复用
unsigned int ntychannel_poll(struct file *filp, struct poll_table_struct *wait) {
    struct ntychannel *channel = filp->private_data;
    unsigned int mask = 0;

    poll_wait(filp, &channel->inq, wait);

    if (have_data) {
        mask |= (POLLIN | POLLRDNORM);
    }

    return mask;
}

// 文件偏移操作
loff_t ntychannel_llseek(struct file *filp, loff_t offset, int whence) {
    loff_t newpos;

    switch (whence) {
        case SEEK_SET:
            newpos = offset;
            break;
        case SEEK_CUR:
            newpos = filp->f_pos + offset;
            break;
        case SEEK_END:
            newpos = NTYCHANNEL_SIZE - 1 + offset;
            break;
        default:
            return -EINVAL;
    }

    if (newpos < 0 || newpos > NTYCHANNEL_SIZE) {
        return -EINVAL;
    }

    filp->f_pos = newpos;

    return newpos;
}

static const struct file_operations ntychannel_fops = {
    .owner = THIS_MODULE,
    .llseek = ntychannel_llseek,
    .read = ntychannel_read,
    .write = ntychannel_write,
    .open = ntychannel_open,
    .release = ntychannel_release,
    .poll = ntychannel_poll,
};

// 初始化函数
static int ntychannel_init(void) {
    int result;
    int i;
    //由于手动创建在/dev下的ntychannel设备，使用主、次设备号关联
    dev_t devno = MKDEV(channel_major, 0);
    if (channel_major) {
        result = register_chrdev_region(devno, NTYCHANNEL_NR_DEVS, "ntychannel");
    } else {
        result = alloc_chrdev_region(&devno, 0, NTYCHANNEL_NR_DEVS, "ntychannel");
        channel_major = MAJOR(devno);
    }

    if (result < 0)
        return result;

    cdev_init(&cdev, &ntychannel_fops);
    cdev.owner = THIS_MODULE;
    cdev_add(&cdev, devno, NTYCHANNEL_NR_DEVS);

    channel_devp = kmalloc(NTYCHANNEL_NR_DEVS * sizeof(struct ntychannel), GFP_KERNEL);
    if (!channel_devp) {
        result = -ENOMEM;
        goto fail_malloc;
    }

    memset(channel_devp, 0, sizeof(struct ntychannel));

    for (i = 0; i < NTYCHANNEL_NR_DEVS; i++) {
        channel_devp[i].size = NTYCHANNEL_SIZE;
        channel_devp[i].data = kmalloc(NTYCHANNEL_SIZE, GFP_KERNEL);
        memset(channel_devp[i].data, 0, NTYCHANNEL_SIZE);
        init_waitqueue_head(&(channel_devp[i].inq));
    }

    printk(KERN_INFO "ntychannel_init\n");

    return 0;

fail_malloc:
    unregister_chrdev_region(devno, NTYCHANNEL_NR_DEVS);
    return result;
}

// 卸载函数
static void ntychannel_exit(void) {
    int i;
    printk(KERN_INFO "ntychannel_exit\n");

    for (i = 0; i < NTYCHANNEL_NR_DEVS; i++) {
        kfree(channel_devp[i].data);
    }

    cdev_del(&cdev);
    kfree(channel_devp);

    unregister_chrdev_region(MKDEV(channel_major, 0), NTYCHANNEL_NR_DEVS);
}

MODULE_AUTHOR("SongQingYuan");
MODULE_LICENSE("GPL");

module_init(ntychannel_init);
module_exit(ntychannel_exit);
