#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>

int main() {
    // 获取当前进程的 CPU 亲和性
    cpu_set_t current_mask;
    CPU_ZERO(&current_mask);

    // 获取当前进程的 CPU 亲和性掩码
    if (sched_getaffinity(0, sizeof(cpu_set_t), &current_mask) == -1) {
        perror("sched_getaffinity failed");
        return 1;
    }

    // 打印当前进程的 CPU 亲和性
    printf("Current CPU affinity mask: ");
    for (int i = 0; i < CPU_SETSIZE; i++) {
        //这个宏是检查是否可以在i处理器上运行
        if (CPU_ISSET(i, &current_mask)) {
            printf("%d ", i);
        }
    }
    printf("\n");

    // 设置当前进程的 CPU 亲和性，只允许在 CPU 0 上运行
    cpu_set_t new_mask;
    CPU_ZERO(&new_mask);
    CPU_SET(0, &new_mask);

    if (sched_setaffinity(0, sizeof(cpu_set_t), &new_mask) == -1) {
        perror("sched_setaffinity failed");
        return 1;
    }

    // 打印设置后的 CPU 亲和性
    printf("New CPU affinity mask: ");
    for (int i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &new_mask)) {
            printf("%d ", i);
        }
    }
    printf("\n");

    // 获取并打印更新后的 CPU 亲和性
    CPU_ZERO(&current_mask);
    if (sched_getaffinity(0, sizeof(cpu_set_t), &current_mask) == -1) {
        perror("sched_getaffinity failed");
        return 1;
    }

    printf("Updated CPU affinity mask: ");
    for (int i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &current_mask)) {
            printf("%d ", i);
        }
    }
    printf("\n");

    return 0;
}
