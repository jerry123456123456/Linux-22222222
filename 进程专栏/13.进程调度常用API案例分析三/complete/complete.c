#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/sched.h>

//定义完成量
struct completion buffer_ready;
//定义一个简单的缓冲区
char buffer[100];
//用于表示缓冲区是否有数据
int buffer_has_data = 0;

int producer_thread(void *data);
int consumer_thread(void *data);

//生产者线程函数
int producer_thread(void *data){
    printk(KERN_INFO "Producer thread started\n");
    // 模拟向缓冲区写入数据
    snprintf(buffer, sizeof(buffer), "Hello, world!");
    buffer_has_data = 1;

    //标记完成量为完成，通知消费者线程可以读数据了
    complete(&buffer_ready);
    printk(KERN_INFO "Producer thread finished\n");
    return 0;
}

//消费者线程函数
int consumer_thread(void *data){
    printk(KERN_INFO "Consumer thread started, waiting for data...\n");

    //等待完成了
    wait_for_completion(&buffer_ready);
    if (buffer_has_data) {
       printk(KERN_INFO "Consumer thread read data: %s\n", buffer);
    }
    printk(KERN_INFO "Consumer thread finished\n");
    return 0;
}

//模块初始化的函数
static int __init my_module_init(void){
    struct task_struct *producer,*consumer;
    printk(KERN_INFO "Module initialization started\n");

    //初始化完成量
    init_completion(&buffer_ready);
    //创建生产者线程
    producer = kthread_run(producer_thread,NULL,"prodecer_thread");
    //创建消费者线程
    consumer = kthread_run(consumer_thread,NULL,"consumer_thread");
    printk(KERN_INFO "Module initialization finished\n");
    return 0;
}

// 模块退出函数
static void __exit my_module_exit(void)
{
    printk(KERN_INFO "Module exit\n");
}

MODULE_LICENSE("GPL");
module_init(my_module_init);
module_exit(my_module_exit);