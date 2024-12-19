#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/rcupdate.h>

// 引入MySQL客户端相关头文件
#include <mysql.h>

// 定义一个阈值，当value达到这个值后，关闭线程
#define STOP_THRESHOLD 100

struct RCUStruct{
    int a;
    struct rcu_head rcu;
};

static struct RCUStruct *Global_pointer;
// 创建2个读内核线程和1个写内核线程
static struct task_struct *RCURD_Thr1, *RCURD_Thr2, *RCUWriter_Thread;

// 读者线程1
static int MyRCU_ReaderThreadFunc1(void *data){
    struct RCUStruct *pointer = NULL;
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    while(1){
        // msleep相当于互斥锁，mdelay相当于自旋锁
        // 这里的休眠5秒，相当于模拟线程在每次读取操作之间的一个短暂的间隔
        msleep(5);
        // 读多写少的并发场景
        rcu_read_lock();  // 加锁
        // 模拟获取锁之后的一些处理操作
        mdelay(10);
        // 这是通过 rcu_dereference 函数获取共享数据指针，rcu_dereference 函数在 RCU 机制中有特殊作用，
        // 它可以在保证并发安全的前提下返回共享指针的值，允许读者线程获取到当前有效的共享数据指针
        pointer = rcu_dereference(Global_pointer);
        if(pointer){
            printk("%s:read a=%d\n", __func__);
        }

        // 连接MySQL数据库并查询数据（这里简化处理，实际要做好错误处理等更多逻辑）
        conn = mysql_init(NULL);
        if (mysql_real_connect(conn, "localhost", "root", "123456", "test_rcu", 4000, NULL, 0) == NULL) {
            printk("Error connecting to database: %s\n", mysql_error(conn));
            continue;
        }

        if (mysql_query(conn, "SELECT * FROM Users")) {
            printk("Query execution failed: %s\n", mysql_error(conn));
            mysql_close(conn);
            continue;
        }

        res = mysql_store_result(conn);
        while ((row = mysql_fetch_row(res))!= NULL) {
            // 这里可以根据实际Users表结构来处理获取到的每一行数据，简单打印示例
            printk("Data from Users table: %s %s\n", row[0], row[1]);
        }

        mysql_free_result(res);
        mysql_close(conn);

        rcu_read_unlock();

        // 检查是否收到停止线程的信号，如果收到则退出循环，结束线程
        if (kthread_should_stop()) {
            break;
        }
    }
    return 0;
}

static int MyRCU_ReaderThreadFunc2(void *data) // 读者线程2
{
    struct RCUStruct *pointer2 = NULL;
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    while(1)
    {
        msleep(5);
        // 读多写少并发场景：高效、低开销
        rcu_read_lock();

        mdelay(10);
        pointer2 = rcu_dereference(Global_pointer);
        if(pointer2)
            printk("%s:read a=%d\n", __func__);

        // 连接MySQL数据库并查询数据（这里简化处理，实际要做好错误处理等更多逻辑）
        conn = mysql_init(NULL);
        if (mysql_real_connect(conn, "localhost", "root", "123456", "test_rcu", 4000, NULL, 0) == NULL) {
            printk("Error connecting to database: %s\n", mysql_error(conn));
            continue;
        }

        if (mysql_query(conn, "SELECT * FROM Users")) {
            printk("Query execution failed: %s\n", mysql_error(conn));
            mysql_close(conn);
            continue;
        }

        res = mysql_store_result(conn);
        while ((row = mysql_fetch_row(res))!= NULL) {
            // 这里可以根据实际Users表结构来处理获取到的每一行数据，简单打印示例
            printk("Data from Users table: %s %s\n", row[0], row[1]);
        }

        mysql_free_result(res);
        mysql_close(conn);

        rcu_read_unlock();

        // 检查是否收到停止线程的信号，如果收到则退出循环，结束线程
        if (kthread_should_stop()) {
            break;
        }
    }

    return 0;
}

// 回调函数的定义部分
static void myrcu_del(struct rcu_head *rcuh){
    // container_of 是一个常用的宏，通常用于从 嵌套结构体 中获取外部结构体的指针。
    // 它的基本作用是通过结构体中的某个字段的地址，反推得到整个结构体的地址。
    struct RCUStruct *p = container_of(rcuh, struct RCUStruct, rcu);
    printk("%s\n", __func__);
    kfree(p);
}

// 写者线程
static int MyRCU_WriterThread_Func(void *pointer){
    struct RCUStruct *old;
    struct RCUStruct *new_ptr;
    int value = (unsigned long)pointer;
    MYSQL *conn;
    while(1){
        msleep(10);
        // GFP_KERNEL为内核级别的内存分配，支持阻塞操作。适用于大多数普通的内存分配场景
        new_ptr = kmalloc(sizeof(struct RCUStruct), GFP_KERNEL);

        old = Global_pointer;
        *new_ptr = *old;
        new_ptr->a = value;

        // rcu_assign_pointer 函数将新的共享数据结构体指针赋值给全局的共享指针 Global_pointer，
        // 这个函数在 RCU 机制下可以确保在合适的时候让读者线程能看到新的数据，保证数据更新的并发安全性和一致性
        rcu_assign_pointer(Global_pointer, new_ptr);
        // 调用 call_rcu 函数，注册一个回调函数（这里就是前面定义的 myrcu_del 函数），
        // 当所有正在访问旧数据的读者线程完成访问后（在 RCU 机制的协调下），
        // 会自动调用这个回调函数来释放旧数据结构体占用的内存空间，实现内存的安全回收
        call_rcu(&old->rcu, myrcu_del);

        printk("%s:write to new %d\n", __func__, value);

        // 这里可以添加向数据库写入相关数据等操作（示例暂未详细实现写操作）
        // 连接数据库进行写操作示例（此处简单示意，要完善错误处理等）
        conn = mysql_init(NULL);
        if (mysql_real_connect(conn, "localhost", "root", "你的密码", "survey_system", 0, NULL, 0) == NULL) {
            printk("Error connecting to database for write: %s\n", mysql_error(conn));
            continue;
        }

        // 假设执行一个简单的插入语句示例（实际根据需求替换合适语句）
        if (mysql_query(conn, "INSERT INTO SomeTable (column1) VALUES ('test_value')")) {
            printk("Write query execution failed: %s\n", mysql_error(conn));
        }

        mysql_close(conn);

        value++;

        // 判断value是否达到阈值，如果达到则停止所有相关线程
        if (value >= STOP_THRESHOLD) {
            kthread_stop(RCURD_Thr1);
            kthread_stop(RCURD_Thr2);
            kthread_stop(RCUWriter_Thread);
            break;
        }
    }
    return 0;
}

// 模块加载
static int __init MyRCU_TestFunc_Init(void){
    int value = 2;
    // 提示：初始化内核模块成功
    printk("Prompt:Successfully initialized the kernel module.\n");

    Global_pointer = kzalloc(sizeof(struct RCUStruct), GFP_KERNEL);

    RCURD_Thr1 = kthread_run(MyRCU_ReaderThreadFunc1, NULL, "RCU_RDer1");
    RCURD_Thr2 = kthread_run(MyRCU_ReaderThreadFunc2, NULL, "RCU_RDer2");
    RCUWriter_Thread = kthread_run(MyRCU_WriterThread_Func, (void*)(unsigned long)value, "RCU_Writer");

    return 0;
}

// 模块卸载
static void __exit MyRCU_TestFunc_Exit(void){
    // 卸载内核模块
    printk("Prompt:Successfully uninstalled kernel module!\n");
    if (RCURD_Thr1) {
        kthread_stop(RCURD_Thr1);
    }
    if (RCURD_Thr2) {
        kthread_stop(RCURD_Thr2);
    }
    if (RCUWriter_Thread) {
        kthread_stop(RCUWriter_Thread);
    }

    if(Global_pointer)
        kfree(Global_pointer);
}

module_init(MyRCU_TestFunc_Init); // 内核模块入口函数
module_exit(MyRCU_TestFunc_Exit); // 内核模块退出函数
MODULE_LICENSE("GPL"); // 模块的许可证声明