#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/kthread.h>

// 定义等待队列头
wait_queue_head_t my_wait_queue_head;

// 模拟共享资源
int shared_resource = 0;

// 线程1等待条件
int thread1_condition = 5;
// 线程2等待条件
int thread2_condition = 10;

//线程函数1
static int thread_fun_1(void *data){
    printk(KERN_INFO "线程1: 开始等待，等待共享资源达到 %d\n",thread1_condition);
    wait_event_interruptible(my_wait_queue_head,shared_resource >= thread1_condition);
    printk(KERN_INFO "线程1：满足条件，被唤醒，共享资源当前值为 %d\n", shared_resource);
    return 0;
}

// 线程函数2
static int thread_func_2(void *data)
{
    printk(KERN_INFO "线程2：开始等待，等待共享资源达到 %d\n", thread2_condition);
    wait_event_interruptible(my_wait_queue_head, shared_resource >= thread2_condition);
    printk(KERN_INFO "线程2：满足条件，被唤醒，共享资源当前值为 %d\n", shared_resource);
    return 0;
}

//模拟更新共享资源的函数类似一个生产者的角色
static void update_shared_resource(void){
    while(shared_resource < 15){
        shared_resource++;
        printk(KERN_INFO "更新共享资源，当前值为 %d\n", shared_resource);
        if (shared_resource >= thread1_condition) {
            wake_up_interruptible(&my_wait_queue_head);
        }
        if (shared_resource >= thread2_condition) {
            wake_up_interruptible(&my_wait_queue_head);
        }

        //简单模拟一些延迟，让过程更符合情况，schedule_timeout单位是jiffies，即几*HZ就是休眠几秒
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(1 * HZ);
    }
}

//模块初始化的函数
static int __init my_module_init(void){
    //初始化等待队列头
    init_waitqueue_head(&my_wait_queue_head);

    printk(KERN_INFO "模块初始化\n");

    //创建线程1
    struct task_struct *thread1;
    //这个函数相当于kthread_create和wake_up_process的组合
    thread1 = kthread_run(thread_fun_1,NULL,"thread_1");
    if (IS_ERR(thread1)) {
        printk(KERN_ERR "创建线程1失败\n");
        return PTR_ERR(thread1);
    }

    //手动将线程1添加到等待队列中
    wait_queue_entry_t wait_entry_1;
    init_waitqueue_entry(&wait_entry_1,thread1);
    add_wait_queue(&my_wait_queue_head,&wait_entry_1);

    //创建线程2
    struct task_struct *thread2;
    thread2 = kthread_run(thread_func_2, NULL, "thread_2");
    if (IS_ERR(thread2)) {
        printk(KERN_ERR "创建线程2失败\n");
        return PTR_ERR(thread2);
    }
    // 手动将线程2添加到等待队列的相关初始化及添加操作
    wait_queue_entry_t wait_entry_2;
    init_waitqueue_entry(&wait_entry_2, thread2);
    add_wait_queue(&my_wait_queue_head, &wait_entry_2);

    //启动更新共享资源函数，模拟资源更新的过程
    update_shared_resource();

    return 0;
}

// 模块退出函数
static void __exit my_module_exit(void)
{
    printk(KERN_INFO "模块退出\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
MODULE_LICENSE("GPL");

