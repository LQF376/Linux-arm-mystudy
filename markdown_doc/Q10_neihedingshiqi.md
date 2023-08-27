# Linux 内核定时器

## 0.  系统节拍

系统频率（节拍率）：硬件定时器提供时钟源，时钟源通过设置的频率周期性的产生定时中断，系统使用定时中断来计时。中断周期性产生的频率就是系统频率

```c
/* .config 文件 */
CONFIG_HZ=100    // 系统节拍率

/* include/asm-generic/param.h */
#define HZ	CONFIG_HZ
// HZ 表示一秒的节拍数，就是频率
```

高节拍率会导致中断的产生更加频繁，加剧系统的负担

jiffies 记录了系统从启动以来的系统节拍数（32位机器是32位的），启动时会初始化为0，定义在 include/linux/jiffies.h 中

- jiffies / HZ 就是系统运行时间，单位秒

32位的 jiffies 要注意绕回问题（溢出风险）常用处理绕回问题的 API：

![1678848413867](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1678848413867.png)

```c
time_after(unkown, known)
unkown 超过 known , time_after 函数返回真，否则返回假
time_before(unkown, known)
unkown 没有超过 known , time_after 函数返回真，否则返回假
time_after_eq()   // 多了判断相等的条件
time_before_eq()  // 同理
```

```c
/* demo */
unsigned long timeout;
timeout = jiffies + (2 * HZ); 	/* 超时的时间点 */

if(time_before(jiffies, timeout))
{
	/* 超时未发生 */
}
else
{
    /* 超时发生 */
}
```

```c
/* 使用 jiffies 判断超时 定时2s */
unsigned long timeout;
timeout = jiffies + (2 * HZ);

/* 判断有没有超时 */
if(time_before(jiffies, timeout))
{
	/* 超时未发生 */
}
else
{
	/* 超时发生 */
}
```

jiffies 和 ms、us、ns 之间转换的 API：

![1678848529194](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1678848529194.png)

## 1. 内核定时器

- 只需要提供超时时间（定时值）和定时处理函数即可，当超时时间到了以后设置的定时处理函数就会执行
- 内核定时器不是周期性运行的，超时以后就会自动关闭，要实现周期性，就要在定时处理函数中再次打开定时
- Linux 用 timer_list 结构体表示内核定时器

```c
/* include/linux/timer.h */

struct timer_list {
	struct list_head entry;
	unsigned long expires; 			/* 定时器超时时间，单位是节拍数  jiffies+(2*HZ)定时2s */
	struct tvec_base *base;
	void (*function)(unsigned long); /* 定时处理函数 */
	unsigned long data; 			/* 要传递给 function 函数的参数 */
	int slack;
};
```

> - 初始化 timer_list 类型变量（定义一个变量后，要初始化一下）
>
> ```c
> void init_timer(struct timer_list *timer)
> timer：要初始化的定时器
> 返回值：无
> ```
>
> - 向内核注册定时器，注册定时器后就会开始运行
>
> ```c
> void add_timer(sturct timer_list *timer)
> timer：要注册的定时器
> 返回值：无
> ```
>
> - 删除一个定时器（不管定时器是否激活）
>
> ```c
> int del_timer(struct timer_list *timer)
> timer：要删除的定时器
> 返回值：0->定时器还没被激活	1->定时器已经激活
> ```
>
> - 等待其他处理器使用完定时器再删除（同步版）
>
> ```c
> int del_timer_sync(struct timer_list *timer)
> timer：要删除的定时器
> 返回值：0->定时器未被激活 1->计时器已经激活
> ```
>
> - 修改定时器的定时值，若定时器没有被激活，mod_timer 会激活定时器
>
> ```c
> int mod_timer(struct timer_list *timer, unsigned long expires)
> timer：定时器
> expires：修改后的超时时间
> 返回值： 0->调用前定时器未被激活 1->调用前定时器已被激活
> ```

```c
/* demo */
struct timer_list timer; 			/* 定义定时器 变量 */

/* 定时器回调函数 */
void function(unsigned long arg)
{
	...
	mod_timer(&timer, jiffies + msecs_to_jiffies(2000));
}

void init(void)
{
	init_timer(&timer);			// 初始化定时器
	
	timer.function = function;	// 设置定时处理函数
	timer.expires = jiffes + msecs_to_jiffies(2000);	// 超时时间2秒
	timer.data = (unsigned long)&dev;					// 将设备结构体作为参数
	
	add_timer(&timer);					// 启动定时器
}

void exit(void)
{
	del_timer(&timer);
	/* 或者使用 */
	del_timer_sync(&timer);
}
```

## 2. Linux 内核短延时函数

- Linux 提供了 毫秒、微妙、纳秒 级别的延时函数

![1678850720093](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1678850720093.png)

## 3. 定时器实现 LED 灯闪烁驱动程序

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
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define TIMER_CNT 1 					/* 设备号个数 */
#define TIMER_NAME "timer" 				/* 名字 */
#define CLOSE_CMD (_IO(0XEF, 0x1)) 		/* 关闭定时器 */
#define OPEN_CMD (_IO(0XEF, 0x2))	 	/* 打开定时器 */
#define SETPERIOD_CMD (_IO(0XEF, 0x3)) /* 设置定时器周期命令 */
#define LEDON 1 						/* 开灯 */
#define LEDOFF 0 						/* 关灯 */

/* timer 设备结构体 */
struct timer_dev{
	dev_t devid;		 /* 设备号 */
	struct cdev cdev; 	/* cdev */
	struct class *class; 	/* 类 */
	struct device *device; 	/* 设备 */
	int major; 		/* 主设备号 */
	int minor;		 /* 次设备号 */
	struct device_node *nd; /* 设备节点 */
	int led_gpio; /* key 所使用的 GPIO 编号 */
	int timeperiod; /* 定时周期,单位为 ms */
	struct timer_list timer; /* 定义一个定时器 */
	spinlock_t lock; /* 定义自旋锁 */
};

struct timer_dev timerdev; /* timer 设备 */

/* 初始化LED灯 */
static int led_init(void)
{
	int ret = 0;

	timerdev.nd = of_find_node_by_path("/gpioled");
	if (timerdev.nd== NULL) 
	{
		return -EINVAL;
	}
	
	timerdev.led_gpio = of_get_named_gpio(timerdev.nd ,"led-gpio", 0);
	if (timerdev.led_gpio < 0) 
	{
		printk("can't get led\r\n");
		return -EINVAL;
	}
	
	/* 初始化 led 所使用的 IO */
	gpio_request(timerdev.led_gpio, "led"); /* 请求 IO */
	ret = gpio_direction_output(timerdev.led_gpio, 1);
	if(ret < 0) 
	{
		printk("can't set gpio!\r\n");
	}
	return 0;
}

/* 打开设备 */
static int timer_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	filp->private_data = &timerdev; /* 设置私有数据 */

	timerdev.timeperiod = 1000; /* 默认周期为 1s */
	ret = led_init(); /* 初始化 LED IO */
	if (ret < 0) 
	{
		return ret;
	}
	return 0;
}

/* ioctl 函数 */
static long timer_unlocked_ioctl(struct file *filp,
								unsigned int cmd, unsigned long arg)
{
	struct timer_dev *dev = (struct timer_dev *)filp->private_data;
	int timerperiod;
	unsigned long flags;

	switch (cmd) {
		case CLOSE_CMD: /* 关闭定时器 */
			del_timer_sync(&dev->timer);
			break;
		case OPEN_CMD: /* 打开定时器 */
			spin_lock_irqsave(&dev->lock, flags);
			timerperiod = dev->timeperiod;
			spin_unlock_irqrestore(&dev->lock, flags);
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(timerperiod));
			break;
		case SETPERIOD_CMD: /* 设置定时器周期 */
			spin_lock_irqsave(&dev->lock, flags);
			dev->timeperiod = arg;
			spin_unlock_irqrestore(&dev->lock, flags);
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(arg));
			break;
		default:
			break;
		}
	return 0;
}

/* 设备操作函数 */
static struct file_operations timer_fops = {
	.owner = THIS_MODULE,
	.open = timer_open,
	.unlocked_ioctl = timer_unlocked_ioctl,
};

/* 定时器回调函数 */
void timer_function(unsigned long arg)
{
	struct timer_dev *dev = (struct timer_dev *)arg;
	static int sta = 1;
	int timerperiod;
	unsigned long flags;

	sta = !sta; /* 每次都取反，实现 LED 灯反转 */
	gpio_set_value(dev->led_gpio, sta);

	/* 重启定时器 */
	spin_lock_irqsave(&dev->lock, flags);
	timerperiod = dev->timeperiod;
	spin_unlock_irqrestore(&dev->lock, flags);
	mod_timer(&dev->timer, jiffies + msecs_to_jiffies(dev->timeperiod));
}

/* 驱动入口函数 */
static int __init timer_init(void)
{
	/* 初始化自旋锁 */
	spin_lock_init(&timerdev.lock);

	/* 注册字符设备驱动 */
		/* 1、创建设备号 */
	if (timerdev.major) 
	{ 	/* 定义了设备号 */
		timerdev.devid = MKDEV(timerdev.major, 0);
		register_chrdev_region(timerdev.devid, TIMER_CNT, TIMER_NAME);
	} 
	else 
	{ /* 没有定义设备号 */
		alloc_chrdev_region(&timerdev.devid, 0, TIMER_CNT, TIMER_NAME);
		timerdev.major = MAJOR(timerdev.devid); 	/* 获取主设备号 */
		timerdev.minor = MINOR(timerdev.devid); 	/* 获取次设备号 */
	}

	/* 2、初始化 cdev */
	timerdev.cdev.owner = THIS_MODULE;
	cdev_init(&timerdev.cdev, &timer_fops);

	/* 3、添加一个 cdev */
	cdev_add(&timerdev.cdev, timerdev.devid, TIMER_CNT);

	/* 4、创建类 */
	timerdev.class = class_create(THIS_MODULE, TIMER_NAME);
	if (IS_ERR(timerdev.class)) 
	{
		return PTR_ERR(timerdev.class);
	}
	
	/* 5、创建设备 */
	timerdev.device = device_create(timerdev.class, NULL, 
								timerdev.devid, NULL, TIMER_NAME);
	if (IS_ERR(timerdev.device)) 
	{
		return PTR_ERR(timerdev.device);
	}

	/* 6、初始化 timer，设置定时器处理函数,还未设置周期，所有不会激活定时器 */
	init_timer(&timerdev.timer);
	timerdev.timer.function = timer_function;
	timerdev.timer.data = (unsigned long)&timerdev;
	return 0;
}

/* 驱动出口函数 */
static void __exit timer_exit(void)
{
	gpio_set_value(timerdev.led_gpio, 1); /* 卸载驱动的时候关闭 LED */
	del_timer_sync(&timerdev.timer); /* 删除 timer */
#if 0
	del_timer(&timerdev.tiemr);
#endif

	/* 注销字符设备驱动 */
	cdev_del(&timerdev.cdev); /* 删除 cdev */
	unregister_chrdev_region(timerdev.devid, TIMER_CNT);

	device_destroy(timerdev.class, timerdev.devid);
	class_destroy(timerdev.class);
}

module_init(timer_init);
module_exit(timer_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lvqifeng");
```

### 4. 测试 APP

```c
/*
1：关闭定时器	2：打开定时器	3：设置定时周期
*/
#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "linux/ioctl.h"

/* 命令值 */
#define CLOSE_CMD (_IO(0XEF, 0x1)) 	/* 关闭定时器 */
#define OPEN_CMD (_IO(0XEF, 0x2)) 	/* 打开定时器 */
#define SETPERIOD_CMD (_IO(0XEF, 0x3)) 	/* 设置定时器周期命令 */

int main(int argc, char *argv[])
{
	int fd, ret;
	char *filename;
	unsigned int cmd;
	unsigned int arg;
	unsigned char str[100];

	if (argc != 2) 
	{
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = argv[1];

	fd = open(filename, O_RDWR);
	if (fd < 0) 
	{
		printf("Can't open file %s\r\n", filename);
		return -1;
	}
	
	while (1) 
	{
		printf("Input CMD:");
		ret = scanf("%d", &cmd);
		if (ret != 1) 
		{ /* 参数输入错误 */
			gets(str); /* 防止卡死 */
		}
		
		if(cmd == 1) /* 关闭 LED 灯 */
			cmd = CLOSE_CMD;
		else if(cmd == 2) /* 打开 LED 灯 */
			cmd = OPEN_CMD;
		else if(cmd == 3) 
		{
			cmd = SETPERIOD_CMD; /* 设置周期值 */
			printf("Input Timer Period:");
			ret = scanf("%d", &arg);
			if (ret != 1) 
			{ /* 参数输入错误 */
				gets(str); /* 防止卡死 */
			}
		}
		ioctl(fd, cmd, arg); /* 控制定时器的打开和关闭 */
	}
	close(fd);
}
```

### 5. 编译和运行驱动和测试程序

```shell
./timerApp /dev/timer
```

