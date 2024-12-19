#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>

//定义消息结构体
struct msgbuf{
    long mtype;
    char mtext[100];
};  

int main(){
    //创建或者获取消息队列标识符
    //"."：表示当前目录。ftok() 会使用当前目录的 inode 号来生成键，因此路径是必要的
    //'a'：字符 'a' 被用作 proj_id，是一个额外的标识符，用于区分不同的 IPC 键
    key_t key = ftok(".",'a');
    if (key == -1) {
        perror("ftok");
        return 1;
    }
    //创建或者打开消息队列  
    //key：这是之前通过 ftok() 生成的唯一标识符。它用于标识要访问的消息队列
    //IPC_CREAT：这是一个常量，表示如果消息队列不存在，则创建一个新的消息队列
    //0666：这是一个文件权限位，类似于 UNIX 文件系统中的文件权限
    int msgid = msgget(key,IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget");
        return 1;
    }

    //发送消息的进程
    if(fork() == 0){
        struct msgbuf msg;
        msg.mtype = 1;
        sprintf(msg.mtext, "Hello from child process!");
        //发送消息
        if(msgsnd(msgid,&msg,sizeof(msg.mtext),0) == -1){
            perror("msgsnd");
            return 1;
        }
        printf("Child process sent a message.\n");
        exit(0);
    }

    //接收消息的进程
    //wait(NULL) 是一个系统调用，用于父进程等待其子进程的终止。它的作用是使父进程阻塞，直到子进程结束，并且回收子进程的状态
    wait(NULL);
    struct msgbuf msg;
    if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
        perror("msgrcv");
        return 1;
    }
    printf("Parent process received: %s\n", msg.mtext);

    //删除消息队列
    if(msgctl(msgid,IPC_RMID,NULL) == -1){
        perror("msgctl");
        return 1;
    }

    return 0;
}