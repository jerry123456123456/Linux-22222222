#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>

void TestThread1Func(){
    //sleep(1);
    int i,j;
    int iPolicy;
    struct sched_param sParam;
    //这个函数主要的作用是获取指定线程（由thread参数指定）的调度策略（存储在policy指向的变量中）和调度参数（存储在param指向的结构体中
    pthread_getschedparam(pthread_self(),&iPolicy,&sParam);

    if(iPolicy==SCHED_OTHER)
        printf("SCHED_OTHER.\n");

    if(iPolicy==SCHED_FIFO)
        printf("SCHED_FIFO.\n");
    
    if(iPolicy==SCHED_RR)
        printf("SCHED_RR TEST001.\n");

    for(i=1;i<=5;i++)
    {
        for(j=1;j<=5000000;j++){}
        printf("Execute thread function 1.\n");
    }
    printf("Pthread 1 Exit.\n\n");
}

void TestThread2Func()
{
    //sleep(2);
    int i,j;
    int iPolicy;
    struct sched_param sParam;

    pthread_getschedparam(pthread_self(),&iPolicy,&sParam);

    if(iPolicy==SCHED_OTHER)
        printf("SCHED_OTHER.\n");

    if(iPolicy==SCHED_FIFO)
        printf("SCHED_FIFO.\n");
    
    if(iPolicy==SCHED_RR)
        printf("SCHED_RR TEST002.\n");

    for(i=1;i<=6;i++)
    {
        for(j=1;j<=6000000;j++){}
        printf("Execute thread function 2.\n");
    }
    printf("Pthread 2 Exit.\n\n");
}

void TestThread3Func()
{
    sleep(3);
    int i,j;
    int iPolicy;
    struct sched_param sParam;

    pthread_getschedparam(pthread_self(),&iPolicy,&sParam);

    if(iPolicy==SCHED_OTHER)
        printf("SCHED_OTHER.\n");

    if(iPolicy==SCHED_FIFO)
        printf("SCHED_FIFO.\n");
    
    if(iPolicy==SCHED_RR)
        printf("SCHED_RR TEST003.\n");

    for(i=1;i<=7;i++)
    {
        for(j=1;j<=7000000;j++){}
        printf("Execute thread function 3.\n");
    }
    printf("Pthread 3 Exit.\n\n");
}

int main(int argc,char *argv[]){
    int i = 0;
    i = getuid();

    if(i == 0){
        printf("The current user is root.\n\n");
    }else{
        printf("The current user is not root.\n\n");
    }

    pthread_t ppid1,ppid2,ppid3;
    struct sched_param sParam;
    pthread_attr_t pAttr1,pAttr2,pAttr3;

    pthread_attr_init(&pAttr1);
    pthread_attr_init(&pAttr2);
    pthread_attr_init(&pAttr3);

    //SCHED_RR和SCHED_FIFO必须有root权限才可以设置成功

    //在这里虽然优先级更高，但是SCHED_FIFO有很强的独立性，不被更高优先级的FIFO抢占，不会下CPU
    sParam.sched_priority=31;
    pthread_attr_setschedpolicy(&pAttr2,SCHED_FIFO);
    pthread_attr_setschedparam(&pAttr2,&sParam);
    //pthread_attr_setinheritsched函数的作用是设置线程属性对象中关于调度继承方面的属性
    //PTHREAD_EXPLICIT_SCHED作为第二个参数，表示线程的调度属性将明确按照之前设置好的（通过pthread_attr_setschedpolicy和pthread_attr_setschedparam等函数设置的调度策略和调度参数）来执行，而不会继承创建它的线程的调度属性
    pthread_attr_setinheritsched(&pAttr2,PTHREAD_EXPLICIT_SCHED);

    sParam.sched_priority=11;
    pthread_attr_setschedpolicy(&pAttr1,SCHED_FIFO);
    pthread_attr_setschedparam(&pAttr1,&sParam);
    pthread_attr_setinheritsched(&pAttr1,PTHREAD_EXPLICIT_SCHED);

    pthread_create(&ppid3,&pAttr3,(void*)TestThread3Func,NULL);
    pthread_create(&ppid2,&pAttr2,(void*)TestThread2Func,NULL);
    pthread_create(&ppid1,&pAttr1,(void*)TestThread1Func,NULL);

    pthread_join(ppid3,NULL);
    pthread_join(ppid2,NULL);
    pthread_join(ppid1,NULL);

    pthread_attr_destroy(&pAttr3);
    pthread_attr_destroy(&pAttr2);
    pthread_attr_destroy(&pAttr1);

    return 0;
}

/*输出结果
The current user is root.

SCHED_FIFO.
SCHED_FIFO.
Execute thread function 2.
Execute thread function 1.
Execute thread function 2.
Execute thread function 1.
Execute thread function 1.
Execute thread function 2.
Execute thread function 1.
Execute thread function 2.
Execute thread function 1.
Pthread 1 Exit.

Execute thread function 2.
Execute thread function 2.
Pthread 2 Exit.

SCHED_OTHER.
Execute thread function 3.
Execute thread function 3.
Execute thread function 3.
Execute thread function 3.
Execute thread function 3.
Execute thread function 3.
Execute thread function 3.
Pthread 3 Exit.
*/

/*
线程 1 和 2 的执行交替：

TestThread2Func 先执行，理论上它的优先级高，不应该被 TestThread1Func 中的线程抢占。然而，你看到它们交替执行，可能的原因是：
线程切换时的 CPU 调度：在 Linux 系统中，如果系统负载较轻，调度器可能会频繁地对线程进行切换，即使它们没有显式的抢占行为。
pthread_create 后的线程启动时机：线程 1、2 和 3 是并行创建的，它们并不是严格按顺序启动的。这可能导致线程 1 和 2 并没有按照严格的优先级顺序执行，而是被操作系统的调度器插入。
CPU 时间片分配：尽管 SCHED_FIFO 是基于优先级的调度策略，但 Linux 的调度器仍可能会根据一些系统设置或硬件负载情况，偶尔对同一优先级的线程进行调度。
*/