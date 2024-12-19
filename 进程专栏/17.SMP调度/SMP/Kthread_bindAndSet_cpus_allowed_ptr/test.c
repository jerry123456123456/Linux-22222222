#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/cpu.h>  // 引入此头文件来使用 CPU 亲和性函数

static struct task_struct *my_thread;

int my_thread_fn(void *data);

// 内核线程的主函数
int my_thread_fn(void *data) {
    pr_info("Kernel thread is running on CPU: %d\n", smp_processor_id());

    while (!kthread_should_stop()) {
        pr_info("Kernel thread is running on CPU: %d\n", smp_processor_id());
        ssleep(1);  // 线程休眠1秒
    }

    pr_info("Kernel thread is stopping\n");
    return 0;
}

static int __init my_module_init(void) {
    // 创建内核线程
    my_thread = kthread_run(my_thread_fn, NULL, "my_kernel_thread");

    if (IS_ERR(my_thread)) {
        pr_err("Failed to create kernel thread\n");
        return PTR_ERR(my_thread);
    }

    pr_info("Kernel thread started\n");

    // 使用 kthread_bind 将线程绑定到 CPU 1
    kthread_bind(my_thread, 1); // 将线程绑定到 CPU 1

    // 使用 cpumask_t 强制将线程绑定到 CPU 1
    cpumask_t mask;
    cpumask_clear(&mask);  // 清空 cpumask
    cpumask_set_cpu(1, &mask);  // 将 CPU 1 添加到 mask 中
    set_cpus_allowed_ptr(my_thread, &mask);  // 设置线程的 CPU 亲和性

    pr_info("Thread is bound to CPU 1\n");

    return 0;
}

static void __exit my_module_exit(void) {
    // 停止内核线程
    if (my_thread) {
        kthread_stop(my_thread);  // 发出停止信号
        pr_info("Kernel thread stopped\n");
    }
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Example using kthread_bind and set_cpus_allowed_ptr");
