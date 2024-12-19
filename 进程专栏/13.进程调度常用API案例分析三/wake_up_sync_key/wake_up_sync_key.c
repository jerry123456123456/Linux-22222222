#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/kthread.h>

// 定义等待队列头
wait_queue_head_t my_wait_queue;

//用于模拟共享资源的变量
static int shared_resource = 0;
//用于表示资源是否准备好的标志
static bool resource_ready = false;

static int producer_function(void *data);
static int consumer_function(void *data);

static int producer_function(void *data){
    printk(KERN_INFO "Producer started.\n");
    // 模拟生产资源的过程
    shared_resource = 42;
    resource_ready = true;

    // 是一个同步唤醒函数，其作用是唤醒一个或多个等待队列中的进程
    //第一个参数是指向等待队列的头部；第二个参数指定唤醒进程的方式，为 0 意味着没有指定唤醒模式
    __wake_up_sync(&my_wait_queue,0);

    printk(KERN_INFO "Producer finished.\n");
    return 0;
}

// 消费者函数，模拟消费资源
static int consumer_function(void *data){
    printk(KERN_INFO "Consumer started and waiting for resource...\n");
    // 等待资源准备好
    wait_event(my_wait_queue, resource_ready);
    printk(KERN_INFO "Consumer got resource: %d\n", shared_resource);
    printk(KERN_INFO "Consumer finished.\n");
    return 0;
}

static int __init my_module_init(void){
    struct task_struct *producer,*consumer;

    printk(KERN_INFO "Module initialization started.\n");

    // 初始化等待队列头
    init_waitqueue_head(&my_wait_queue);

    // 创建生产者线程
    producer = kthread_run(producer_function, NULL, "producer_thread");
    if (IS_ERR(producer)) {
        printk(KERN_ERR "Failed to create producer thread.\n");
        return PTR_ERR(producer);
    }

    // 创建消费者线程
    consumer = kthread_run(consumer_function, NULL, "consumer_thread");
    if (IS_ERR(consumer)) {
        printk(KERN_ERR "Failed to create consumer thread.\n");
        return PTR_ERR(consumer);
    }

    printk(KERN_INFO "Module initialization completed.\n");
    return 0;
}

// 模块退出函数
static void __exit my_module_exit(void)
{
    printk(KERN_INFO "Module exiting.\n");
}

// 声明模块遵循GPL许可证
MODULE_LICENSE("GPL");
// 指定模块初始化函数
module_init(my_module_init);
// 指定模块退出函数
module_exit(my_module_exit);