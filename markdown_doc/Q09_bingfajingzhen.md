# 并发和竞争

## 0. 概念

并发：多个“用户”同时访问同一个共享资源

> 并发产生的原因：
>
> 1. 多线程并发访问
>
>    Linux 是多任务（线程）的系统
>
> 2. 抢占式并发访问
>
>    Linux 内核支持抢占，调度程序可以在任意时刻抢占正在运行的线程
>
> 3. 中断程序并发访问
>
> 4. 多核核间并发访问

临界区（共享数据段）：必须保证一次只有一个线程访问，保证临界区是原子访问

## 1. 原子操作

C 语言里面的一条代码编译成汇编指令不一定只有一条！！！

linux 提供两组原子操作 API 函数

- 原子整形操作 API 函数

  定义了 atomic_t 的结构体来代替整形变量进行原子操作

```
/* 结构体定义在 include/linux/types.h */
typedef struct{
	int counter;
}atomic_t;

// 要使用原子操作 API 必须定义 atomic_t 变量
atomic_t a;
atomic_t b = ATOMIC_INIT(0);	// 定义原子变量b并赋初值为0
```

![1678791350453](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1678791350453.png)

------

- 原子位操作 API 函数

  原子位操作是直接对内存进行操作

  ![1678792610387](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1678792610387.png)

![1678792640322](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1678792640322.png)

## 2.自旋锁

- 当一个线程要访问某个共享资源是，首先要获得相应的锁，锁只能被一个线程持有
- 自旋锁就是若锁被其他线程所占有，线程就会不断查询，处于忙循环-旋转-等待状态，不会进入休眠状态或者说去做其他的处理
- 适用于短时期的轻量级加锁，会浪费处理器事件，降低系统性能

```
/* Linux 内核使用 spinlock_t 表示自旋锁 */
/* 内核关于自旋锁的定义 */
typedef struct spinlock {
	union {
		struct raw_spinlock rlock;
	
#ifdef CONFIG_DEBUG_LOCK_ALLOC
#define LOCK_PADSIZE (offsetof(struct raw_spinlock, dep_map))
		struct {
			u8 __padding[LOCK_PADSIZE];
			struct lockdep_map dep_map;
			};
#endif
		};
} spinlock_t;
```

```
spinlock_t lock; 	// 使用自旋锁之前要定义自旋锁
```

![1678793309752](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1678793309752.png)

使用自旋锁所存在的问题：

1. 被自旋锁保护的临界区内一定不能调用任何能够引起睡眠和阻塞的 API 函数（可能会引起死锁）

2. 自旋锁会自动禁止抢占

3. 中断里面可以使用自旋锁，但在获取锁之前一定要先禁止本地中断

   ![1678793895736](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1678793895736.png)

```
/* 中断中使用自旋锁的一个demo */
DEFINE_SPINLOCK(lock) /* 定义并初始化一个

/* 线程 A */
void functionA()
{
	unsigned long flags; /* 中断状态 */
	spin_lock_irqsave(&lock, flags) /* 获取锁 */
	/* 临界区 */
	spin_unlock_irqrestore(&lock, flags) /* 释放锁 */
}

/* 中断服务函数 */
void irq()
{
	spin_lock(&lock) /* 获取锁 */
	/* 临界区 */
	spin_unlock(&lock) /* 释放锁 */
}
```

### 2. 1其他类型的锁

1. 读写自旋锁

   - 为读和写提供了不同的锁
   - 一次只能允许一个写操作，一个线程持有写锁，且不能进行读操作
   - 没有写操作的时候允许一个或者多个线程持有读锁，可以进行并发的读操作

   ```
   /* Linux 内核 使用 rwlock_t 表示 读写锁 */
   typedef struct {
   	arch_rwlock_t raw_lock;
   }rwlock_t;
   ```

   ![1678795025269](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1678795025269.png)

   ![1678795048533](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1678795048533.png)

2. 顺序锁

   - 在读写锁的基础上衍生，读写锁的读操作和写操作不能同时进行
   - 允许在写的时候进行读操作，实现同时读写，但是不允许同时进行并发的写操作。
   - 循序锁保护的资源不可以是指针，在写的时候会使指针短时失效，读取野指针会发生意外

   ```
   /* Linux 内核使用 seqlock_t 表示顺序锁 */
   typedef struct{
   	struct seqcount seqcount;
   	spinlock_t lock;
   }seqlock_t;
   ```

   ![1678795716183](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1678795716183.png)

## 3. 信号量

信号量有一个信号量值，通过信号量值来控制访问共享资源的访问数量；如果申请访问，则信号量值 -1，当信号量值为0时，就不允许访问，线程将进入休眠状态

- 相对于自旋锁，信号量可以使线程进入休眠状态（提高处理器的使用效率）
- 信号量的开销要比自旋锁大，因为进入休眠后会切换线程
- 适用于占用资源较久的场合
- 不能用于中断，因为信号量会引起休眠
- 初始化信号量大于1，计数型信号量（不可用于互斥访问）；初始化不大于1，二值信号量

```
/* Linux内核使用 semaphore 表示信号量 */
struct semaphore{
	raw_spinlock_t	lock;
	unsigned int 	count;
	struct list_head wait_list;
};
```

![1678797446090](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1678797446090.png)

```
/* demo */
struct semaphore sem;	// 定义信号量
sema_init(&sem, 1);		// 初始化信号量
down(&sem);				// 申请信号量
	/* 临界区 */
up(&sem);				// 释放信号量
```

## 4. 互斥体

互斥体(互斥锁)表示一次只有一个线程可以访问共享资源

- mutex 可能导致休眠，因此中断中不能使用 mutex，中断中只能使用自旋锁
- 和信号量一样，mutex 保护的临界区内可以调用引起阻塞的 API 函数

```
struct mutex{
	/* 1:未锁 0：上锁 */
	atomic_t count;
	spinlock_t wait_lock;
};
```

![1678798483962](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1678798483962.png)

```
struct mutex lock;		// 定义互斥锁
mutex_init(&lock);		// 初始化互斥锁
mutex_lock(&lock);		// 上锁
...
mutex_unlock(&lock);	// 解锁
```

# 实验代码

```c
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define GPIOLED_CNT			1		  	/* 设备号个数 */
#define GPIOLED_NAME		"gpioled"	/* 名字 */
#define LEDOFF 				0			/* 关灯 */
#define LEDON 				1			/* 开灯 */


/* gpioled设备结构体 */
struct gpioled_dev{
	dev_t devid;			/* 设备号 	 */
	struct cdev cdev;		/* cdev 	*/
	struct class *class;	/* 类 		*/
	struct device *device;	/* 设备 	 */
	int major;				/* 主设备号	  */
	int minor;				/* 次设备号   */
	struct device_node	*nd; /* 设备节点 */
	int led_gpio;			/* led所使用的GPIO编号		*/
	atomic_t lock;			/* 原子变量 */
};

struct gpioled_dev gpioled;	/* led设备 */

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int led_open(struct inode *inode, struct file *filp)
{
	/* 通过判断原子变量的值来检查LED有没有被别的应用使用 */
	if (!atomic_dec_and_test(&gpioled.lock)) {
		atomic_inc(&gpioled.lock);	/* 小于0的话就加1,使其原子变量等于0 */
		return -EBUSY;				/* LED被使用，返回忙 */
	}

	filp->private_data = &gpioled; /* 设置私有数据 */
	return 0;
}

/*
 * @description		: 从设备读取数据 
 * @param - filp 	: 要打开的设备文件(文件描述符)
 * @param - buf 	: 返回给用户空间的数据缓冲区
 * @param - cnt 	: 要读取的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 读取的字节数，如果为负值，表示读取失败
 */
static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	return 0;
}

/*
 * @description		: 向设备写数据 
 * @param - filp 	: 设备文件，表示打开的文件描述符
 * @param - buf 	: 要写给设备写入的数据
 * @param - cnt 	: 要写入的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[1];
	unsigned char ledstat;
	struct gpioled_dev *dev = filp->private_data;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

	ledstat = databuf[0];		/* 获取状态值 */

	if(ledstat == LEDON) {	
		gpio_set_value(dev->led_gpio, 0);	/* 打开LED灯 */
	} else if(ledstat == LEDOFF) {
		gpio_set_value(dev->led_gpio, 1);	/* 关闭LED灯 */
	}
	return 0;
}

/*
 * @description		: 关闭/释放设备
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
 */
static int led_release(struct inode *inode, struct file *filp)
{
	struct gpioled_dev *dev = filp->private_data;

	/* 关闭驱动文件的时候释放原子变量 */
	atomic_inc(&dev->lock);
	return 0;
}

/* 设备操作函数 */
static struct file_operations gpioled_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
	.release = 	led_release,
};

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init led_init(void)
{
	int ret = 0;

	/* 初始化原子变量 */
	atomic_set(&gpioled.lock, 1);	/* 原子变量初始值为1 */

	/* 设置LED所使用的GPIO */
	/* 1、获取设备节点：gpioled */
	gpioled.nd = of_find_node_by_path("/gpioled");
	if(gpioled.nd == NULL) {
		printk("gpioled node not find!\r\n");
		return -EINVAL;
	} else {
		printk("gpioled node find!\r\n");
	}

	/* 2、 获取设备树中的gpio属性，得到LED所使用的LED编号 */
	gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "led-gpio", 0);
	if(gpioled.led_gpio < 0) {
		printk("can't get led-gpio");
		return -EINVAL;
	}
	printk("led-gpio num = %d\r\n", gpioled.led_gpio);

	/* 3、设置GPIO1_IO03为输出，并且输出高电平，默认关闭LED灯 */
	ret = gpio_direction_output(gpioled.led_gpio, 1);
	if(ret < 0) {
		printk("can't set gpio!\r\n");
	}

	/* 注册字符设备驱动 */
	/* 1、创建设备号 */
	if (gpioled.major) {		/*  定义了设备号 */
		gpioled.devid = MKDEV(gpioled.major, 0);
		register_chrdev_region(gpioled.devid, GPIOLED_CNT, GPIOLED_NAME);
	} else {						/* 没有定义设备号 */
		alloc_chrdev_region(&gpioled.devid, 0, GPIOLED_CNT, GPIOLED_NAME);	/* 申请设备号 */
		gpioled.major = MAJOR(gpioled.devid);	/* 获取分配号的主设备号 */
		gpioled.minor = MINOR(gpioled.devid);	/* 获取分配号的次设备号 */
	}
	printk("gpioled major=%d,minor=%d\r\n",gpioled.major, gpioled.minor);	
	
	/* 2、初始化cdev */
	gpioled.cdev.owner = THIS_MODULE;
	cdev_init(&gpioled.cdev, &gpioled_fops);
	
	/* 3、添加一个cdev */
	cdev_add(&gpioled.cdev, gpioled.devid, GPIOLED_CNT);

	/* 4、创建类 */
	gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);
	if (IS_ERR(gpioled.class)) {
		return PTR_ERR(gpioled.class);
	}

	/* 5、创建设备 */
	gpioled.device = device_create(gpioled.class, NULL, gpioled.devid, NULL, GPIOLED_NAME);
	if (IS_ERR(gpioled.device)) {
		return PTR_ERR(gpioled.device);
	}
	
	return 0;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit led_exit(void)
{
	/* 注销字符设备驱动 */
	cdev_del(&gpioled.cdev);/*  删除cdev */
	unregister_chrdev_region(gpioled.devid, GPIOLED_CNT); /* 注销设备号 */

	device_destroy(gpioled.class, gpioled.devid);
	class_destroy(gpioled.class);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("zuozhongkai");
```

```c
/* 测试 APP */
#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"

#define LEDOFF 	0
#define LEDON 	1

/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
	int fd, retvalue;
	char *filename;
	unsigned char cnt = 0;
	unsigned char databuf[1];
	
	if(argc != 3){
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = argv[1];

	/* 打开beep驱动 */
	fd = open(filename, O_RDWR);
	if(fd < 0){
		printf("file %s open failed!\r\n", argv[1]);
		return -1;
	}

	databuf[0] = atoi(argv[2]);	/* 要执行的操作：打开或关闭 */

	/* 向/dev/gpioled文件写入数据 */
	retvalue = write(fd, databuf, sizeof(databuf));
	if(retvalue < 0){
		printf("LED Control Failed!\r\n");
		close(fd);
		return -1;
	}

	/* 模拟占用25S LED */
	while(1) {
		sleep(5);
		cnt++;
		printf("App running times:%d\r\n", cnt);
		if(cnt >= 5) break;
	}

	printf("App running finished!");
	retvalue = close(fd); /* 关闭文件 */
	if(retvalue < 0){
		printf("file %s close failed!\r\n", argv[1]);
		return -1;
	}
	return 0;
}
```

