#include <linux/module.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/wait.h>

// 定义一个指向task_struct结构体的指针，用于保存当前初始化进程的数据信息
struct task_struct *pthreads;
// 定义一个completion结构体，用于保存completion的所有状态
static struct completion cp;

// 自定义的内核线程函数声明
int myfunc_threadtest(void *argc);

// 自定义的内核线程函数
int myfunc_threadtest(void *argc)
{
    // 打印调用内核线程函数的信息
    printk("调用内核线程函数: myfunc_threadtest(...).\n");

    // 打印当前进程的PID值
    printk("打印输出当前进程的PID值: %d\n", current->pid);
    // 打印当前completion结构体中done字段的值
    printk("打印输出当前字段done的值为: %d\n", cp.done);
    // 这里原代码有误，task_struct结构体没有state成员，此处假设是想要获取进程状态相关信息
    // printk("打印输出父进程的状态: %ld\n",pthreads->state);

    // 调用函数唤醒进程并且更改done字段的值
    complete_all(&cp);

    // 再次打印当前completion结构体中done字段的值
    printk("打印输出当前字段done的值为: %d\n", cp.done);
    // 这里原代码有误，task_struct结构体没有state成员，此处假设是想要获取进程状态相关信息
    // printk("打印输出父进程状态state的值为: %ld\n",pthreads->state);

    // 打印退出内核线程函数的信息
    printk("退出内核线程函数: myfunc_threadtest(...)\n");

    return 0;
}

// 模块初始化函数
static int __init completionall_initfunc(void)
{
    // 定义一个指向task_struct结构体的指针，用于后续创建新线程
    struct task_struct *pts;
    // 定义一个等待队列元素
    wait_queue_entry_t data;
    // 定义一个长整型变量，用于存储等待超时时间
    long lefttime;

    // 打印调用模块初始化函数的信息
    printk("调用模块初始化函数: completionall_initfunc(...).\n");

    // 将当前进程赋值给pthreads
    pthreads = current;

    // 创建一个新的线程，线程函数为myfunc_threadtest，线程名是"complete_all_test"
    pts = kthread_create_on_node(myfunc_threadtest, NULL, -1, "complete_all_test");
    // 唤醒新创建的线程
    wake_up_process(pts);

    // 初始化completion结构体
    init_completion(&cp);
    init_waitqueue_entry(&data, current);
    add_wait_queue(&(cp.wait),&data)

    // 使等待队列进程进入不可中断的等待状态，等待时间为10000
    lefttime = schedule_timeout_uninterruptible(10000);

    // 打印调用函数schedule_timeout_uninterruptible的timeout的值
    printk("打印输出调用函数 sched_timeout_uninterruptible(...)的timeout的值为: %ld\n", lefttime);
    // 打印当前进程的PID值
    printk("打印输出当前进程的PID值为: %d\n", current->pid);
    // 打印新创建线程的PID值
    printk("打印输出task_struct的pid的值为: %d\n", pts->pid);

    // 打印退出模块初始化函数的信息
    printk("退出模块初始化函数\n");
    return 0;
}

// 模块退出函数
static void __exit completionall_exitfunc(void)
{
    // 打印内核正常退出信息
    printk("内核正常退出: complete_all(...)函数.\n");
}

// 声明模块遵循GPL许可证
MODULE_LICENSE("GPL");
// 指定模块初始化函数
module_init(completionall_initfunc);
// 指定模块退出函数
module_exit(completionall_exitfunc);