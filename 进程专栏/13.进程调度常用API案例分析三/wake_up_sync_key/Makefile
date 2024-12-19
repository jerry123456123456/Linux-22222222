# 指定要构建的内核模块对应的目标文件
obj-m := wake_up_sync_key.o

# 构建所有模块的目标规则
all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 清理模块的目标规则
clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
