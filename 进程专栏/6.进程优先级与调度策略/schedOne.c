#include<stdio.h>
#include<pthread.h>
#include<sched.h>
#include<assert.h>

//获取线程策略
static int GetThreadPolicyFunc(pthread_attr_t *pAttr){
    //这个变量将用于存储获取到的线程调度策略相关的值
    int iPlicy;
    //它的主要作用是从给定的线程属性对象（这里通过指针 pAttr 来指定）中获取线程的调度策略信息，并将获取到的调度策略值存储到由 &iPlicy 指向的变量
    int igp = pthread_attr_getschedpolicy(pAttr,&iPlicy);
    assert(igp == 0);

    //Linux内核中有三种调度策略
    switch (iPlicy)
    {
    case SCHED_FIFO:   //实时进程  先进先出实时调度策略
        printf("Policy is --> SCHED_FIFO.\n");
        break;

    case SCHED_RR:    //实时进程   时间片轮转实时调度策略
        printf("Policy is --> SCHED_RR.\n");
        break;

    case SCHED_OTHER:  //普通进程  普通分时调度策略
        printf("Policy is --> SCHED_OTHER.\n");
        break;

    default:
        printf("Policy is --> Unknown.\n");
        break;
    }

    return iPlicy;
}

//打印线程优先级
static void PrintThreadPriorityFunc(pthread_attr_t *pAttr,int iPolicy){
    //获取实时优先级的最大值
    //获取指定调度策略下的最大优先级的函数,传入的参数是指定策略的最大优先级
    int iPriority = sched_get_priority_max(iPolicy);
    assert(iPriority != -1);
    printf("Max_priority is : %d\n",iPriority);
    
    //获取实时优先级的最大值
    iPriority = sched_get_priority_min(iPolicy);
    assert(iPriority != -1);
    printf("Min_priority is : %d\n",iPriority);
}

//获取线程优先级的函数
static int GetThreadPriorityFunc(pthread_attr_t *pAttr){
    struct sched_param sParam;
    //这个函数用于从给定的线程属性对象（由attr参数指定）中获取调度参数（存储在param参数指向的结构体中）
    int irs = pthread_attr_getschedparam(pAttr,&sParam);
    assert(irs == 0);
    printf("Priority = %d\n",sParam.__sched_priority);

    return sParam.__sched_priority;
}

//设置线程策略函数
static void SetThreadPolicyFunc(pthread_attr_t *pAttr,int iPolicy){
    int irs = pthread_attr_setschedpolicy(pAttr,iPolicy);
    assert(irs == 0);

    GetThreadPolicyFunc(pAttr);
}

int main(int argc,char *argv[]){
    pthread_attr_t pAttr;
    struct sched_param sched;

    //为对象分配动态内存空间
    int irs = pthread_attr_init(&pAttr);
    assert(irs == 0);

    //获取线程策略
    int iPlicy = GetThreadPolicyFunc(&pAttr);

    printf("\nExport current Configuration of priority.\n");
    //普通进程不依赖于优先级，所以这里打印SCHED_OTHER调度策略下的优先级最大最小全是0
    PrintThreadPriorityFunc(&pAttr,iPlicy);

    printf("\nExport SCHED_FIFO of prioirty.\n");
    PrintThreadPriorityFunc(&pAttr,SCHED_FIFO);

    printf("\nExport SCHED_RR of prioirty.\n");
    PrintThreadPriorityFunc(&pAttr,SCHED_RR);

    printf("\nExport priority of current thread.\n");
    int iPriority = GetThreadPriorityFunc(&pAttr);

    printf("\nSet SCHED_FIFO policy.\n");
    SetThreadPolicyFunc(&pAttr,SCHED_FIFO);

    printf("\nSet SCHED_RR policy.\n");
    SetThreadPolicyFunc(&pAttr,SCHED_RR);

    printf("\nRestore current policy.\n");
    SetThreadPolicyFunc(&pAttr,iPlicy);

    irs=pthread_attr_destroy(&pAttr);
    assert(irs==0);

    return 0;
}