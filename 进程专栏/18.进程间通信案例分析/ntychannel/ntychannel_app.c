#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <errno.h>

#define BUFFER_LENGTH	128

int main(){
    int fd = open("/dev/ntychannel",O_RDWR);
    if (fd < 0) {
		printf("open failed : errno : %d\n", errno);
		return -1;
	}
    char *buffer = (char *)malloc(BUFFER_LENGTH);
    memset(buffer, 0, BUFFER_LENGTH);

    fd_set rds;
    FD_ZERO(&rds);
    //修改rds所代表的位掩码，将与文件描述符fd对应的位设置为1，以此来表示这个文件描述符现在处于被监视的状态
    FD_SET(fd,&rds);

    /*select函数带五个参数：
    max_fd表示要监视的最大的文件描述符，即从0判断到max_fd
    rset表示监听的可读的集合
    wset表示监听的可写的集合
    eset表示出错的集合
    timeout是超时时间
    */
    while(1){
        //这里用户使用的select\poll通过fs关联到特定的file_operations结构体中的poll函数
        int ret = select(fd + 1,&rds,NULL,NULL,NULL);
        if (ret < 0) {
			printf("select error\n");
			exit(1);
		}
        //FD_ISSET是一个宏，用于检查一个特定的文件描述符fd是否在fd_set类型的集合rds中
        if (FD_ISSET(fd, &rds)) {
            read(fd,buffer,BUFFER_LENGTH);
            printf("channel_data : %s\n",buffer);
        }
    }
    free(buffer);
	close(fd);

    return 0;
}