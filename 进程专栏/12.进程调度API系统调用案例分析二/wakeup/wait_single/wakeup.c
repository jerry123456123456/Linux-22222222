#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/wait.h>
#include<linux/sched.h>
#include <linux/kthread.h>

//定义一个全局变量，作为子线程任务完成的标志，初始化0表示未完成
static int task_complete = 0;

//等待队列头，用于协调主线程和子线程之间的同步
static wait_queue_head_t my_wait_queue_head;

//子线程函数，模拟执行一些任务，这里简单的让线程睡眠一些时间代表执行任务的过程
static int my_thread_func(void *data){
    //KERN_INFO表示这是一个提供一般信息（informational）的消息
    printk(KERN_INFO "子线程开始执行任务\n");

    //模拟子线程执行任务，这里简单的让线程睡眠一段时间代表执行任务的过程
    //set_current_state函数用于设置当前进程的状态,可中断睡眠状态：TASK_INTERRUPTIBLE是进程状态的一种，表示进程处于睡眠（等待）状态，并且可以被信号（signal）中断
    set_current_state(TASK_INTERRUPTIBLE);
    //schedule_timeout函数用于使当前进程（或线程）进入睡眠状态一段时间，主动让出 CPU，之后再被唤醒继续执行。这是一种基于时间的调度机制
    schedule_timeout(3 * HZ);

    //任务完成，修改标志变量
    task_complete = 1;
    printk(KERN_INFO "子线程执行完毕\n");

    //唤醒在等待队列中等待的主线程任务
    wake_up_interruptible(&my_wait_queue_head);

    printk(KERN_INFO "子线程执行完毕\n");
    return 0;
}

//模拟初始化函数，在这里创建子线程，将主线程添加到等待队列中等待子线程完成任务
static int __init my_module_init(void){
    printk(KERN_INFO "模块初始化开始\n");

    //初始化等待队列头
    init_waitqueue_head(&my_wait_queue_head);

    //创建子线程
    struct task_struct *thread = kthread_create(my_thread_func,NULL,"my_subthread");
    if(thread){
        wake_up_process(thread);
        printk(KERN_INFO "子线程创建成功并已唤醒\n");
    }else{
        printk(KERN_INFO "子线程创建失败\n");
        return -1;
    }

    //将主线程添加到等待队列中等待条件满足（即子线程完成）
    wait_queue_entry_t wait_entry;
    init_waitqueue_entry(&wait_entry,current);
    add_wait_queue(&my_wait_queue_head,&wait_entry);

    //设置等到条件，等待子线程任务完成
    printk(KERN_INFO "主线程进入等待状态，等待子线程完成任务\n");
    wait_event_interruptible(my_wait_queue_head,task_complete == 1);

    printk(KERN_INFO "主线程被唤醒，子线程任务已完成，继续执行\n");

    return 0;
}

// 模块退出函数，简单打印退出信息
static void __exit my_module_exit(void)
{
    printk(KERN_INFO "模块退出\n");
}

MODULE_LICENSE("GPL");
module_init(my_module_init);
module_exit(my_module_exit);