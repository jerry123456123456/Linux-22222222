#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
    //pipefd 是一个数组，用来存储管道的两个文件描述符。pipefd[0] 是管道的读取端（读文件描述符），pipefd[1] 是管道的写入端（写文件描述符）
    int pipefd[2];
    //pipe(pipefd) 创建一个管道，将返回的文件描述符保存在 pipefd 数组中
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }
    //创建子进程
    if(fork() == 0){
        //子进程关闭读段，向管道写数据
        close(pipefd[0]);
        char message[] = "Hello from child!";
        write(pipefd[1], message, sizeof(message));
        close(pipefd[1]);
        exit(0);
    }else{
        //父进程关闭写端，从管道读数据
        wait(NULL);
        close(pipefd[1]);
        char buffer[100];
        read(pipefd[0],buffer,sizeof(buffer));
        close(pipefd[0]);
        printf("Parent received: %s\n", buffer);
    }

    return 0;
}