#include<linux/module.h>
#include<linux/pid.h>
#include<linux/sched.h>
#include<linux/wait.h>
#include<linux/kthread.h>

int MyFunc_ThreadFunc(void *argc);

//新进程用来打印新进程的pid
int MyFunc_ThreadFunc(void *argc){
    printk("调用线程函数MyFunc_ThreadFunc(...)\n");
    printk("打印输出当前进程的PID值为: %d\n",current->pid);
    printk("退出线程函数MyFunc_ThreadFunc(...)\n");
    return 0;
}

//配置
static int __init kthreadCreateOnNode_Func(void){
    struct task_struct *pts = NULL;
    printk("调用数MyFunc_ThreadFunc(...)\n");

    pts = kthread_create_on_node(MyFunc_ThreadFunc,NULL,-1,"ktconode.c");

    printk("打印新线程的值: %d\n",pts->pid);

    //唤醒刚才新创建的内核线程
    /*
    这个线程只是被创建并等待执行其任务函数（在这个例子中是MyFunc_ThreadFunc），
    如果没有调用wake_up_process来手动唤醒它，内核没有理由自动将其唤醒进行调度，
    因为它没有等待任何内核已知的、会自动触发唤醒的事件。
    */
    wake_up_process(pts);

    printk("打印当前进程的PID值: %d\n",current->pid);

    printk("退出数MyFunc_ThreadFunc(...)\n");
    return 0;
}

//退出
static void __exit kthreadCreateOnNode_Exit(void){
    printk("正常退出函数kthread_create_on_node...\n");   
}

MODULE_LICENSE("GPL");
module_init(kthreadCreateOnNode_Func);
module_exit(kthreadCreateOnNode_Exit);
