//sudo insmod /root/linux-5.6.18/test_myself/module/my_char_device.ko   加载模块
//sudo rmmod my_char_device   卸载模块
//modinfo my_char_device.ko  这个命令用于输出作者以及模块信息

#include<linux/module.h>  //用于构建可加载内核模块的基本定义和宏
#include<linux/kernel.h>  //这个头文件包含了一些内核中常用的基本函数和定义。它提供了一些用于打印内核消息的函数，如printk
#include<linux/fs.h>      //主要用于文件系统相关的操作
#include<linux/cdev.h>    //这个头文件是用于字符设备（Character Device）开发的重要头文件,如终端设备、字符型鼠标等
#include<linux/device.h>  //主要用于设备模型相关的操作。在 Linux 内核中，设备模型是一个复杂而重要的概念，它用于管理和组织系统中的各种设备，包括设备的注册、创建设备文件等操作
#include<asm/uaccess.h>   //这个头文件用于用户空间和内核空间之间的数据访问操作

#define DEVICE_NAME "my_char_device"
#define BUFFER_SIZE 1024

static int major_num;   //存储设备的主设备号
static char buffer[BUFFER_SIZE];  //用于存储设备文件中的数据
static struct cdev my_cdev;  //结构体指针表示字符设备的描述符
static struct class *my_class;  //设备类指针，用于创建设备并管理相关属性
static struct device *my_device;  //设备文件指针，表示设备文件的实例

static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);


//文件操作的结构体
static const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write,
};

//四个参数：文件，缓冲区，字节个数，文件中偏移
//每调用这个函数只读取一次
////这里的file还必须得写，因为与内核期望的(*read)兼容
static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    //是从内核缓冲区读到用户空间buf中，所以这个判断就是读到末尾
    if(*f_pos >= BUFFER_SIZE){
        return 0;
    }

    //限制读取的字节
    if(*f_pos + count > BUFFER_SIZE){
        count = BUFFER_SIZE - *f_pos;
    }
    
    //将数据从内核读取到用户空间
    //内核缓冲区的*f_pos位置开始读取数据
    if(copy_to_user(buf,buffer + *f_pos,count)){
        return -EFAULT;
    }

    *f_pos +=count;   //更新文件指针的位置
    return count;
}

//写入操作的实现
static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    //从用户空间buf写入到内核缓冲区中
    //这一点很有意思，其实这个代码就是系统调用的，所以并不是用户直接操作内核，于这个清除内核缓冲区的操作不冲突
    memset(buffer,0,BUFFER_SIZE);  //每次写入之前先清空设备（即缓冲区里的内容）

    // 如果写入超过缓冲区大小，限制写入的字节数
    if (*f_pos >= BUFFER_SIZE) {
        return -ENOSPC;  // 如果设备空间已满，返回空间不足的错误
    }

    if (*f_pos + count > BUFFER_SIZE) {
        count = BUFFER_SIZE - *f_pos;  // 限制写入的字节数
    }

    // 将用户空间的内容复制到设备缓冲区
    if (copy_from_user(buffer + *f_pos, buf, count)) {
        return -EFAULT;  // 如果从用户空间到内核空间的复制失败，返回错误
    }

    *f_pos += count;  // 更新文件指针位置
    return count;  // 返回实际写入的字节数
}

//设备初始化
static int __init my_device_init(void){
    //注册主设备号
    //当传入0时，表示让内核动态分配一个主设备号  DEVICE_NAME这是一个字符串，用于给设备命名
    major_num = register_chrdev(0,DEVICE_NAME,&my_fops);

    if (major_num < 0) {
        printk(KERN_ALERT "Failed to register character device\n");
        return major_num;
    }

    //创建设备类，使用DEVICE_NAME 作为设备的名字
    //class_create函数用于创建一个设备类（device class）。
    //设备类是一种用于对设备进行逻辑分组的机制，它有助于更好地管理和组织设备，
    //特别是在用户空间与内核空间交互的过程中，设备类发挥了重要作用
    my_class = class_create(DEVICE_NAME);
    if(IS_ERR(my_class)){
        unregister_chrdev(major_num,DEVICE_NAME);
        printk(KERN_ALERT "Failed to create class\n");
        return PTR_ERR(my_class);
    }

    //device_create函数用于在内核空间创建设备文件相关的设备对象，这个设备对象最终会在用户空间通过/dev目录下的设备文件来访问
    //第一个参数：设备类为设备提供了一个逻辑分组，通过将设备关联到这个设备类，设备可以继承设备类的一些属性和规则
    //第二个参数NULL：这个参数通常用于指定设备的父设备
    //第三个参数：MKDEV是一个宏，用于根据主设备号（major_num）和次设备号（这里次设备号为0）创建一个设备号
    //这个参数可以用于传递一些额外的设备数据结构或者设置。在简单的设备创建场景下，如果没有特殊的需求，可以传入NULL
    //这是一个字符串，用于给要创建的设备命名
    my_device = device_create(my_class,NULL,MKDEV(major_num,0),NULL,DEVICE_NAME);
    if (IS_ERR(my_device)) {
        class_destroy(my_class);
        unregister_chrdev(major_num, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create device\n");
        return PTR_ERR(my_device);
    }

    //初始化字符设备
    cdev_init(&my_cdev,&my_fops);
    //cdev_add函数用于将一个已经初始化的字符设备添加到内核的设备管理系统中。只有将字符设备成功添加后，内核才能正确地处理对这个设备的各种操作请求
    if(cdev_add(&my_cdev,MKDEV(major_num,0),1) < 0){
        device_destroy(my_class, MKDEV(major_num, 0));
        class_destroy(my_class);
        unregister_chrdev(major_num, DEVICE_NAME);
        printk(KERN_ALERT "Failed to add character device\n");
        return -1;
    }

    printk(KERN_INFO "Device %s registered successfully with major number %d\n", DEVICE_NAME, major_num);
    return 0;
}

//卸载设备
static void __exit my_device_exit(void){
    cdev_del(&my_cdev);  //删除字符串设备
    device_destroy(my_class, MKDEV(major_num, 0));
    class_destroy(my_class);
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "Device %s unregistered\n", DEVICE_NAME);
}

module_init(my_device_init);
module_exit(my_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jerry");
MODULE_DESCRIPTION("device");

