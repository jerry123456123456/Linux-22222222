#include<linux/sched.h>
#include<linux/pid.h>
#include<linux/module.h>
#include<linux/kthread.h>

int MyFuncTest(void *args);

//定义内核线程函数
int MyFuncTest(void *args)
{
    printk("Prompt:kernel thread function.\n");

    // 打印输出当前进程的静态优先级
    printk("Prompt:Current static_prio : %d\n",current->static_prio);

    // 打印输出当前进程的PID值
    printk("Prompt:Current Process PID : %d\n",current->pid);

    // 打印输出当前进程的nice值
    printk("Prompt:Current Process nice : %d\n",task_nice(current));

    return 0;
}

static int __init Tasknice_InitFunc(void){
    struct task_struct *pointer_task;
    int priority;

    printk("Prompt:Tasknice_InitFunc function.\n");

    //创建一个新的线程
    pointer_task = kthread_create_on_node(MyFuncTest,NULL,-1,"mytasknicedemo");
    wake_up_process(pointer_task);

    //打印输出新进程的nice值
    priority = task_nice(pointer_task);

    // 打印输出新进程的静态优先级
    printk("Prompt:static_prio of the child thread : %d\n",pointer_task->static_prio);

    // 打印输出新进程的nice值
    printk("Prompt:New process nice : %d\n",priority);

    // 打印输出当前进程的PID值
    printk("Prompt:Current PID : %d\n",current->pid);

    printk("Prompt:Current nice : %d\n",task_nice(current));

    return 0;
}

static void __exit Tasknice_ExitFunc(void){
    printk("Prompt:Normal exit of kernel module.\n");
}


module_init(Tasknice_InitFunc); // 内核模块入口函数
module_exit(Tasknice_ExitFunc); // 内核模块退出函数
MODULE_LICENSE("GPL"); // 模块的许可证声明
