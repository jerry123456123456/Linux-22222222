#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  //提供了多种数据类型的定义,如pid_t size_t key_t
#include <sys/ipc.h>    //定义了用于进程间通信的 IPC 键
#include <sys/shm.h>    //提供了与共享内存相关的函数原型和常量
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
    //创建或获取共享内存端标识符
    key_t key = ftok(".",'b');
    if(key == -1){
        perror("ftok");
        return 1;
    }
    //创建共享内存内存段标识符
    int shmid = shmget(key,1024,IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }
    //将共享内存段映射到进程地址空间
    void *shmaddr = shmat(shmid,NULL,0);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }
    //子进程写入数据到共享内存
    if(fork()){
        strcpy((char *)shmaddr, "Data written by child process");
        printf("Child process wrote data to shared memory.\n");
        //解除对共享内存的映射
        shmdt(shmaddr);
        exit(0);
    }
    //父进程从共享内存中读取数据
    wait(NULL);
    printf("Parent process read from shared memory: %s\n", (char *)shmaddr);

    // 解除共享内存映射并删除共享内存段
    if (shmdt(shmaddr) == -1) {
        perror("shmdt");
        return 1;
    }

    //删除共享内存段
    if(shmctl(shmid,IPC_RMID,NULL) == -1){
        perror("shmctl");
        return 1;
    }

    return 0;
}