# 字符设备驱动开发

字符设备驱动开发重点是使用 register_chrdev 函数注册字符设备，当不再使用设备的时候就用 unregister_chrdev 函数注销字符设备，驱动模块加载成功以后需要手动使用 mknod 命令创建设备节点

## 1. 字符设备驱动简介

- 字符设备就是一个字节一个字节，按字节进行读写操作的设备

Linux 驱动有两种运行方式：

1. 将驱动编译进Linux内核中，内核启动时候就会自动运行驱动程序
2. 将驱动编译成模块（Linux下模块拓展名.ko），内核启动后使用"insmod"命令加载驱动模块

- 模块的加载和卸载

```
module_init(xxx_init);	// 注册模块加载
module_exit(xxx_exit);	// 注册模块卸载
/* 驱动入口函数 */
static int __init xxx_init(void)
{
	/* ... */
	return 0;
}
/* 驱动出口函数 */
static int __exit xxx_init(void)
{
	
}
```

- 驱动模块的加载和卸载

```
insmod drv.ko		// 不判断依赖关系
modprobe drv.ko		//  自动判断依赖关系，会到 /lib/modules/4.1.15目录内查找相应的驱动模块

rmmod drv.ko		// 卸载驱动
modprobe -r drv.ko
```

- 字符设备注册与注销

驱动模块加载成功以后需要注册字符设备；卸载驱动模块时候需要注销字符设备

```c
static inline int register_chrdev(unsigned int major, const char *name,
							const struct file_operations *fops)
static inline void unregister_chrdev(unsigned int major, const char *name)

major：主设备号
name：设备名字
fops：结构体 file_operations 类型指针，指向设备的操作函数集合变量
```

```c
static struct file_operations test_fops;

/* 驱动入口函数 */
static int __init xxx_init(void)
{
	/* 入口函数具体内容 */
	int retvalue = 0;
	
	/* 注册字符设备驱动 */
	retvalue = register_chrdev(200, "chrtest", &test_fops);
	if(retvalue < 0)
	{
		/* 字符设备注册失败，自行处理 */
	}
	return 0;
}

/* 驱动出口函数 */
static void __exit xxx_exit(void)
{
	unregister_chrdev(200, "chrtest");
}

module_init(xxx_init);
module_exit(xxx_exit);
```

```
cat /proc/devices		// 查看当前已经被用掉的设备号
```

- 根据需求，初始化 file_operations结构体类型变量 test_fops

```c
/* 打开设备 */
static int chrtest_open(struct inode *inode, struct file *filp)
{
	/* 用户实现具体功能 */
	return 0;
}

/* 从设备读取 */
static ssize_t chrtest_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	/* 用户实现具体功能 */
	return 0;
}

/* 向设备写数据 */
static ssize_t chrtest_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	/* 用户实现具体功能 */
	return 0;
}

/* 关闭释放设备 */
static int chrtest_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations test_fops = {
	.owner = THIS_MODULE,
	.open = chrtest_open,
	.read = chrtest_read,
	.write = chrtest_write,
	.release = chrtest_release,
};

static int __init xxx_init(void)
{
	...
	int retvalue = 0;
	
	retvalue = register_chrdev(200, "chrtest", &test_fops);
	if(retvalue < 0)
	{
		...
	}
	return 0;
}

static void __exit xxx_exit(void)
{
	...
	unregister_chrdev(200, "chrtest");
}

module_init(xxx_init);
module_exit(xxx_exit);
```

- 添加 LICENSE 和 作者信息

```
MODULE_LICENSE();		// 添加 LICENSE 信息 （必加）
MODULE_AUTHOR();		// 添加作者信息
```

## 2. 设备号

设备号分为了主设备号和次设备号（32位）；

- 主设备号：某一个具体的驱动（12位； 0~4095）
- 次设备号：使用这个驱动的各个设备（低 20位）

```
include/linux/types.h
/* Linux 用 dev_t 表示设备号 */
typedef __u32 __kernel_dev_t;
...
typedef __kernel_dev_t dev_t;

include/uapi/asm-generic/int-ll64.h
typedef unsigned int __u32;
```

关于设备号的一些宏定义：

```c
#define MINORBITS 20
#define MINORMASK ((1U << MINORBITS) - 1)

#define MAJOR(dev) ((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev) ((unsigned int) ((dev) & MINORMASK))
#define MKDEV(ma,mi) (((ma) << MINORBITS) | (mi))
```

### 2.1设备号的分配

- 静态分配设备号（手动指定未使用的设备号进行分配）
- 动态分配设备号（系统帮你分配）

```c
/* 设备号申请函数 */
int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)
dev：保存申请到的设备号
baseminor：次设备号起始地址，函数可申请一段连续的多个设备号，其中主设备号相同， 次设备号从baseminor 开始，一般从0开始
count：要申请的设备号数量
name：设备名字

/* 注销字符设备之后要释放设备号 */
void unregister_chrdev_region(dev_t from, unsigned count)
from：要释放的设备号
count：表示从from开始，要释放的设备号数量
```

## 3. 字符设备驱动开发 demo

```c
/* chrdevbase.c */
#include<linux/types.h>
#include<linux/kernel.h>
#include<linux/delay.h>
#include<linux/ide.h>
#include<linux/init.h>
#include<linux/module.h>

#define CHRDEVBASE_MAJOR	200
#define CHRDEVBASE_NAME		"chrdevbase"

static char readbuf[100];
static char writebuf[100];
static char kerneldata[] = {"kernel data!"};

static int chrdevbase_open(struct inode *node, struct file *filp)
{
	printk("chrdevbase open!\r\n");
	return 0;
}

static ssize_t chrdevbase_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue = 0;
	
	memcpy(readbuf, kerneldata, sizeof(kerneldata));
	retvalue = copy_to_user(buf, readbuf, cnt);
	if(retvalue == 0)
	{
		printk("kernel senddata ok! \r\n");
	}
	else
	{
		printk("kernel senddata failed\r\n");
	}
	
	printk("chrdevbase read!\r\n");
	return 0;
}

static ssize_t chrdevbase_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue = 0;
	
	retvalue = copy_from_user(writebuf, buf, cnt);
	if(retvalue == 0)
		printk("kernel recevdata:%s\r\n", writebuf);
	else
		printk("kernel recevdata failed!\r\n");
	
	printk("chrdevbase write! \r\n");
	return 0;
}

static int chrdevbase_release(struct inode *inode, struct file *filp)
{
	printk("chrdevbase release!\r\n");
	return 0;
}

static struct file_operations chrdevbase_fops = {
	.owner = THIS_MODULE,
	.open = chrdevbase_open,
	.read = chrdevbase_read,
	.write = chrdevbase_write,
	.release = chrdevbase_release,
};

static int __init chrdevbase_init(void)
{
	int retvalue = 0;
	
	retvalue = register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME, &chrdevbase_fops);
	
	if(retvalue < 0)
	{
		printk("chrdevbase driver register failed\r\n");
	}
	printk("chrdevbase_init()\r\n");
	return 0;
}

static void __exit chrdevbase_exit(void)
{
	unregister_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME);
	printk("chrdevbase_exit()\r\n");
}

module_init(chrdevbase_init);
module_exit(chrdevbase_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lvqifeng");
```

```
printk // 类printf，但运行在内核态；有8个消息级别；默认等级级别为4
include/linux/kern_levels.h				// 8个级别定义
include/linux/printk.h 中 CONSOLE_LOGLEVEL_DEFAULT = 7 // 高于7就向控制台打印

printk(KERN_EMERG "gsmi:Log Shutdown Reason\n");
```

![1678195324951](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1678195324951.png)

### 3.1 测试APP程序（用于测试写好的驱动）

```c
/* 测试APP chrdevbaseApp.c */
#include"stdio.h"
#include"unistd.h"
#include"sys/types.h"
#include"sys/stat.h"
#include"fcntl.h"
#include"stdlib.h"
#include"string.h"

static char usrdata[] = {"usr data!"};

int main(int argc, char *argv[])
{
	int fd, retvalue;
	char *filename;
	char readbuf[100], writebuf[100];
	
	if(argc != 3)
	{
		printf("Error Usage!\r\n");
		return -1;
	}
	
	filename = argv[1];
	
	fd = open(filename, 0_RDWR);
	if(fd < 0)
	{
		printf("Can't open file %s\r\n", filename);
		return -1;
	}
	
	// 读文件
	if(atoi(argv[2] == 1))
	{
		retvalue = read(fd, readbuf, 50);
		if(retvalue < 0)
		{
			printf("read file %s failed!\r\n", filename);
		}
		else
		{
			printf("read data:%s\r\n", readbuf);
		}
	}
	
	// 写文件
	if(atoi(argv[2]) == 2)
	{
		memcpy(weitebuf, usrdata, sizeof(usrdata));
		retvalue = write(fd, writebuf, 50);
		if(retvalue < 0)
		{
			printf("Write file %s failed!\r\n",filename);
		}
	}
	
	retvalue = close(fd);
	if(retvalue < 0)
	{
		printf("Cant't close file %s\r\n",filename);
		return -1;
	}
	return 0;
}
```

### 3.2编译驱动程序和测试APP

- 将 chrdevbase.c 这个文件编译为 .ko 模块

```c
/* Makefile */
KERNELDIR := /home/zuozhongkai/linux/IMX6ULL/linux/temp/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek			// linux 内核源码目录
CURRENT_PATH := $(shell pwd)
obi-m := chrdevbase.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
	
clean:
	$(MAKE) -C$(KERNELDIR) M=$(CURRENT_PATH) clean
```

- 编译测试APP

### 3.3 运行测试

- 加载驱动模块

```c
insmod chrdevbase.ko	// 加载驱动文件
modprobe chrdevbase.ko	// 注意，一定要检查" /lib/modules/4.1.15" 文件是否存在
/* modprobe中如 缺少modeules.dep文件问题 */
depmod	// 创建 modules.dep 文件（没有需要重新配置busybox）

/* 查看当前系统中存在的模块 */
lsmod
cat /proc/devices
```

- 创建设备节点文件

驱动加载后需要在 /dev 目录下创建一个与之对应的设备节点文件

```
mknod /dev/chrdevbase c 200 0
c：字符设备
200：设备的主设备号
0：次设备号
```

- chrdevbase 设备操作测试

```
./chrdevbaseApp /dev/chrdevbase 1
./chrdevbaseApp /dev/chrdevbase 2
```

- 卸载驱动模块

如果不使用某个设备的话，可以将其驱动卸载掉

```
rmmod chrdevbase.ko
```

