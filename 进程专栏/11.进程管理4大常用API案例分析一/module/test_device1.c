// test_device1.c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE "/dev/my_char_device"

int main() {
    int fd;
    printf("请输入要写入的数据: \n");
    char msg[256];  // 给 msg 分配足够的空间
    scanf("%s",msg);

    // 打开设备文件
    fd = open(DEVICE, O_WRONLY);
    if (fd == -1) {
        perror("Open failed");
        return 1;
    }

    // 写入数据到设备
    ssize_t write_ret = write(fd, msg, strlen(msg));
    if (write_ret == -1) {
        perror("Write failed");
        close(fd);
        return 1;
    }
    printf("Written %ld bytes to the device: %s\n", write_ret, msg);

    // 关闭设备文件
    close(fd);
    return 0;
}
