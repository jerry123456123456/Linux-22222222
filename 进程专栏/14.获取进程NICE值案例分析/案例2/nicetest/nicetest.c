#include <linux/module.h>
#include <linux/pid.h>
#include <linux/kthread.h>
#include <linux/sched.h>

int MyFunc_Test(void *arg);

int MyFunc_Test(void *arg)
{
    printk("Prompt:********* MyFunc_Test Start *********");

    // 打印输出当前进程的PID
    printk("Prompt:Current PID : %d\n",current->pid);

    // 打印输出当前进程的静态优先级
    printk("Prompt:Current static_prio : %d\n",current->static_prio);

    // 打印输出当前进程的nice值
    printk("Prompt:Current nice :%d\n",task_nice(current));

    printk("Prompt:********* MyFunc_Test End *********");

    return 0;
}

static int __init SetUserNice_InitFunc(void){
    struct task_struct *pointer_task = NULL;

    printk("Prompt:********* SetUserNice_InitFunc Start *********");

    // 创建新进程
    pointer_task=kthread_create_on_node(MyFunc_Test,NULL,-1,"SetUserNice_TestDemo");
    wake_up_process(pointer_task);

    // 打印输出新进程的静态优先级
    printk("Prompt:New static_prio child thread : %d\n",pointer_task->static_prio);
    // 打印输出新进程的nice的值
    printk("Prompt:New nice child thread : %d\n",task_nice(pointer_task));
    // 打印输出新进程的动态优先级
    printk("Prompt:New prio child thread : %d\n",pointer_task->prio);

    ////下面降低了这个新进程的优先级，让cpu更多处理其他进程

    //设置新进程的nice值
    set_user_nice(pointer_task,16);
    printk("Prompt:New value static_prio child thread : %d\n",pointer_task->static_prio);
    printk("Prompt:New value nice child thread ： %d\n",task_nice(pointer_task));
    printk("Prompt:New thread PID : %d\n",pointer_task->pid);

    printk("Prompt:********* SetUserNice_InitFunc End *********");

    return 0;
}


static void __exit SetUserNice_ExitFunc(void)
{
    printk("Prompt:Normal exit of kernel module.\n");

}

module_init(SetUserNice_InitFunc); // 内核模块入口函数
module_exit(SetUserNice_ExitFunc); // 内核模块退出函数
MODULE_LICENSE("GPL"); // 模块的许可证声明
