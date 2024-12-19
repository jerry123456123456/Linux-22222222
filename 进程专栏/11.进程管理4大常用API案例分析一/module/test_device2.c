// test_device2.c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE "/dev/my_char_device"

int main() {
    int fd;
    char buffer[256];

    // 打开设备文件
    fd = open(DEVICE, O_RDONLY);
    if (fd == -1) {
        perror("Open failed");
        return 1;
    }

    // 读取数据
    ssize_t read_ret = read(fd, buffer, sizeof(buffer) - 1);
    if (read_ret == -1) {
        perror("Read failed");
        close(fd);
        return 1;
    }

    buffer[read_ret] = '\0';  // 确保字符串终止
    printf("Read %ld bytes from the device: %s\n", read_ret, buffer);

    // 关闭设备文件
    close(fd);
    return 0;
}
