
# 异步通知（信号）

驱动程序主动向应用程序发出通知，报告自己可以访问，然后应用程序从驱动程序中读取/写入数据

## 1. 信号处理函数

驱动程序通过向应用程序发送不同的信号来实现异步通知

应用程序需要注册信号处理函数

```c
sighandler_t signal(int signum, sighandler_t handler)
signum：要设置处理函数的信号
handler：信号的处理函数
返回值：设置成功的话返回信号的前一个处理函数，设置失败返回 SIG_ERR

信号处理函数
typedef void (*sighandler_t)(int)
```

```c
/* 注册信号处理函数 demo */
#include<stdlib.h>
#include<stdio.h>
#include<signal.h>

void sigint_handler(int num)
{
	printf("\r\n SIGINT signal\r\n");
	exit(0);
}

int main(void)
{
	signal(SIGINT,sigint_handler);
	while(1);
	return 0;
}
```

## 2. 驱动中的信号处理

- fasync_struct 结构体

首先要在驱动中定义一个 fasync_struct 结构体，最好定义在设备结构体里面

```c
struct fasync_struct {
	spinlock_t fa_lock;
	int magic;
	int fa_fd;
	struct fasync_struct *fa_next;
	struct file *fa_file;
	struct rcu_head fa_rcu;
};
```

- fansync 函数

要使用异步通知，需要在设备驱动中实现 file_operations 操作集中的 fasync 函数

```c
int (*fasync)(int fd, struct file *filp, int on)

/* fasync 函数中 一般通过调用 fasync_helper 函数来初始化前面定义的 fasync_struct 结构体指针 */
int fasync_helper(int fd, struct file *filp, int on, struct fasync_struct **fapp)
fapp：要初始化的 fasync_struct 结构体
```

```c
/* 驱动demo */
struct xxx_dev{
	...
	struct fasync_struct *async_queue;
};

static int xxx_fasync(int fd, struct file *filp, int on)
{
	struct xxx_dev *dev = (xxx_dev)filp->private_data;
	
	if(fasync_helper(fd, filp, on, &dev->async_queue) < 0)
		return -EIO;
	return 0;
}

/* 关闭需要释放 fasync_struct */
static in xxx_release(struct inode *inode, struct file *filp)
{
	return xxx_fasync(-1, filp, 0);	// 删除异步通知
}

static struct file_operations xxx_ops = {
	...
        .fasync = xxx_fasync,
	.release = xxx_release,
	...
};
```

- 驱动向应用程序发出信号（kill_fasync 函数）

```c
void kill_fasync(struct fasync_struct **fp, int sig, int band)
fp：需要操作的 fasync_struct
sig：要发送的信号
band:可读时设置为 POLL_IN ，可写时设置为 POLL_OUT
```

## 3. 应用程序对异步通知的处理

1. 注册信号处理函数

   应用程序根据驱动所使用的信号，使用 signal 函数来注册信号处理函数

2. 将本应用程序的进程号告诉内核

   ```c
   fcntl(fd, F_SETOWN, getpid())
   ```

3. 开启异步通知

   ```c
   flags = fcntl(fd, F_GETFL);		// 当前的进程状态 
   fcntl(fd, F_SETFL, flag | FASYNC)	// 开启当前进程异步通知功能
   
   ！！！ 当应用程序通过 fcntl(fd, F_SETFL, flag | FASYNC) 改变标记的时候，驱动程序 file-operarions 操作集中的 fansync 函数就会执行
   ```

## 4. 异步通知实现

### 4.1 驱动代码

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
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <linux/fcntl.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
/***************************************************************
描述	   	: 非阻塞IO访问
***************************************************************/
#define IMX6UIRQ_CNT		1			/* 设备号个数 	*/
#define IMX6UIRQ_NAME		"asyncnoti"	/* 名字 		*/
#define KEY0VALUE			0X01		/* KEY0按键值 	*/
#define INVAKEY				0XFF		/* 无效的按键值 */
#define KEY_NUM				1			/* 按键数量 	*/

/* 中断IO描述结构体 */
struct irq_keydesc {
	int gpio;								/* gpio */
	int irqnum;								/* 中断号     */
	unsigned char value;					/* 按键对应的键值 */
	char name[10];							/* 名字 */
	irqreturn_t (*handler)(int, void *);	/* 中断服务函数 */
};

/* imx6uirq设备结构体 */
struct imx6uirq_dev{
	dev_t devid;			/* 设备号 	 */	
	struct cdev cdev;		/* cdev 	*/                 
	struct class *class;	/* 类 		*/
	struct device *device;	/* 设备 	 */
	int major;				/* 主设备号	  */
	int minor;				/* 次设备号   */
	struct device_node	*nd; /* 设备节点 */	
	atomic_t keyvalue;		/* 有效的按键键值 */
	atomic_t releasekey;	/* 标记是否完成一次完成的按键，包括按下和释放 */
	struct timer_list timer;/* 定义一个定时器*/
	struct irq_keydesc irqkeydesc[KEY_NUM];	/* 按键init述数组 */
	unsigned char curkeynum;				/* 当前init按键号 */
	wait_queue_head_t r_wait;				/* 读等待队列头 */
	struct fasync_struct *async_queue;		/* 异步相关结构体 */
};

struct imx6uirq_dev imx6uirq;	/* irq设备 */

/* @description		: 中断服务函数，开启定时器		
 *				  	  定时器用于按键消抖。
 * @param - irq 	: 中断号 
 * @param - dev_id	: 设备结构。
 * @return 			: 中断执行结果
 */
static irqreturn_t key0_handler(int irq, void *dev_id)
{
	struct imx6uirq_dev *dev = (struct imx6uirq_dev*)dev_id;

	dev->curkeynum = 0;
	dev->timer.data = (volatile long)dev_id;
	mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));	/* 10ms定时 */
	return IRQ_RETVAL(IRQ_HANDLED);
}

/* @description	: 定时器服务函数，用于按键消抖，定时器到了以后
 *				  再次读取按键值，如果按键还是处于按下状态就表示按键有效。
 * @param - arg	: 设备结构变量
 * @return 		: 无
 */
void timer_function(unsigned long arg)
{
	unsigned char value;
	unsigned char num;
	struct irq_keydesc *keydesc;
	struct imx6uirq_dev *dev = (struct imx6uirq_dev *)arg;

	num = dev->curkeynum;
	keydesc = &dev->irqkeydesc[num];

	value = gpio_get_value(keydesc->gpio); 	/* 读取IO值 */
	if(value == 0){ 						/* 按下按键 */
		atomic_set(&dev->keyvalue, keydesc->value);
	}
	else{ 									/* 按键松开 */
		atomic_set(&dev->keyvalue, 0x80 | keydesc->value);
		atomic_set(&dev->releasekey, 1);	/* 标记松开按键，即完成一次完整的按键过程 */
	}               

	if(atomic_read(&dev->releasekey)) {		/* 一次完整的按键过程 */
		if(dev->async_queue)
			kill_fasync(&dev->async_queue, SIGIO, POLL_IN);	/* 释放SIGIO信号 */
	}

#if 0
	/* 唤醒进程 */
	if(atomic_read(&dev->releasekey)) {	/* 完成一次按键过程 */
		/* wake_up(&dev->r_wait); */
		wake_up_interruptible(&dev->r_wait);
	}
#endif
}

/*
 * @description	: 按键IO初始化
 * @param 		: 无
 * @return 		: 无
 */
static int keyio_init(void)
{
	unsigned char i = 0;
	char name[10];
	int ret = 0;
	
	imx6uirq.nd = of_find_node_by_path("/key");
	if (imx6uirq.nd== NULL){
		printk("key node not find!\r\n");
		return -EINVAL;
	} 

	/* 提取GPIO */
	for (i = 0; i < KEY_NUM; i++) {
		imx6uirq.irqkeydesc[i].gpio = of_get_named_gpio(imx6uirq.nd ,"key-gpio", i);
		if (imx6uirq.irqkeydesc[i].gpio < 0) {
			printk("can't get key%d\r\n", i);
		}
	}
	
	/* 初始化key所使用的IO，并且设置成中断模式 */
	for (i = 0; i < KEY_NUM; i++) {
		memset(imx6uirq.irqkeydesc[i].name, 0, sizeof(name));	/* 缓冲区清零 */
		sprintf(imx6uirq.irqkeydesc[i].name, "KEY%d", i);		/* 组合名字 */
		gpio_request(imx6uirq.irqkeydesc[i].gpio, name);
		gpio_direction_input(imx6uirq.irqkeydesc[i].gpio);	
		imx6uirq.irqkeydesc[i].irqnum = irq_of_parse_and_map(imx6uirq.nd, i);
#if 0
		imx6uirq.irqkeydesc[i].irqnum = gpio_to_irq(imx6uirq.irqkeydesc[i].gpio);
#endif
	}

	/* 申请中断 */
	imx6uirq.irqkeydesc[0].handler = key0_handler;
	imx6uirq.irqkeydesc[0].value = KEY0VALUE;
	
	for (i = 0; i < KEY_NUM; i++) {
		ret = request_irq(imx6uirq.irqkeydesc[i].irqnum, imx6uirq.irqkeydesc[i].handler, 
		                 IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, imx6uirq.irqkeydesc[i].name, &imx6uirq);
		if(ret < 0){
			printk("irq %d request failed!\r\n", imx6uirq.irqkeydesc[i].irqnum);
			return -EFAULT;
		}
	}

	/* 创建定时器 */
    init_timer(&imx6uirq.timer);
    imx6uirq.timer.function = timer_function;

	/* 初始化等待队列头 */
	init_waitqueue_head(&imx6uirq.r_wait);
	return 0;
}

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int imx6uirq_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &imx6uirq;	/* 设置私有数据 */
	return 0;
}

 /*
  * @description     : 从设备读取数据 
  * @param - filp    : 要打开的设备文件(文件描述符)
  * @param - buf     : 返回给用户空间的数据缓冲区
  * @param - cnt     : 要读取的数据长度
  * @param - offt    : 相对于文件首地址的偏移
  * @return          : 读取的字节数，如果为负值，表示读取失败
  */
static ssize_t imx6uirq_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	int ret = 0;
	unsigned char keyvalue = 0;
	unsigned char releasekey = 0;
	struct imx6uirq_dev *dev = (struct imx6uirq_dev *)filp->private_data;

	if (filp->f_flags & O_NONBLOCK)	{ /* 非阻塞访问 */
		if(atomic_read(&dev->releasekey) == 0)	/* 没有按键按下，返回-EAGAIN */
			return -EAGAIN;
	} else {							/* 阻塞访问 */
		/* 加入等待队列，等待被唤醒,也就是有按键按下 */
 		ret = wait_event_interruptible(dev->r_wait, atomic_read(&dev->releasekey)); 
		if (ret) {
			goto wait_error;
		}
	}

	keyvalue = atomic_read(&dev->keyvalue);
	releasekey = atomic_read(&dev->releasekey);

	if (releasekey) { /* 有按键按下 */	
		if (keyvalue & 0x80) {
			keyvalue &= ~0x80;
			ret = copy_to_user(buf, &keyvalue, sizeof(keyvalue));
		} else {
			goto data_error;
		}
		atomic_set(&dev->releasekey, 0);/* 按下标志清零 */
	} else {
		goto data_error;
	}
	return 0;

wait_error:
	return ret;
data_error:
	return -EINVAL;
}

 /*
  * @description     : poll函数，用于处理非阻塞访问
  * @param - filp    : 要打开的设备文件(文件描述符)
  * @param - wait    : 等待列表(poll_table)
  * @return          : 设备或者资源状态，
  */
unsigned int imx6uirq_poll(struct file *filp, struct poll_table_struct *wait)
{
	unsigned int mask = 0;
	struct imx6uirq_dev *dev = (struct imx6uirq_dev *)filp->private_data;

	poll_wait(filp, &dev->r_wait, wait);	/* 将等待队列头添加到poll_table中 */
	
	if(atomic_read(&dev->releasekey)) {		/* 按键按下 */
		mask = POLLIN | POLLRDNORM;			/* 返回PLLIN */
	}
	return mask;
}

/*
 * @description     : fasync函数，用于处理异步通知
 * @param - fd		: 文件描述符
 * @param - filp    : 要打开的设备文件(文件描述符)
 * @param - on      : 模式
 * @return          : 负数表示函数执行失败
 */
static int imx6uirq_fasync(int fd, struct file *filp, int on)
{
	struct imx6uirq_dev *dev = (struct imx6uirq_dev *)filp->private_data;
	return fasync_helper(fd, filp, on, &dev->async_queue);
}

/*
 * @description     : release函数，应用程序调用close关闭驱动文件的时候会执行
 * @param - inode	: inode节点
 * @param - filp    : 要打开的设备文件(文件描述符)
 * @return          : 负数表示函数执行失败
 */
static int imx6uirq_release(struct inode *inode, struct file *filp)
{
	return imx6uirq_fasync(-1, filp, 0);
}

/* 设备操作函数 */
static struct file_operations imx6uirq_fops = {
	.owner = THIS_MODULE,
	.open = imx6uirq_open,
	.read = imx6uirq_read,
	.poll = imx6uirq_poll,
	.fasync = imx6uirq_fasync,
	.release = imx6uirq_release,
};

/*
 * @description	: 驱动入口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init imx6uirq_init(void)
{
	/* 1、构建设备号 */
	if (imx6uirq.major) {
		imx6uirq.devid = MKDEV(imx6uirq.major, 0);
		register_chrdev_region(imx6uirq.devid, IMX6UIRQ_CNT, IMX6UIRQ_NAME);
	} else {
		alloc_chrdev_region(&imx6uirq.devid, 0, IMX6UIRQ_CNT, IMX6UIRQ_NAME);
		imx6uirq.major = MAJOR(imx6uirq.devid);
		imx6uirq.minor = MINOR(imx6uirq.devid);
	}

	/* 2、注册字符设备 */
	cdev_init(&imx6uirq.cdev, &imx6uirq_fops);
	cdev_add(&imx6uirq.cdev, imx6uirq.devid, IMX6UIRQ_CNT);

	/* 3、创建类 */
	imx6uirq.class = class_create(THIS_MODULE, IMX6UIRQ_NAME);
	if (IS_ERR(imx6uirq.class)) {	
		return PTR_ERR(imx6uirq.class);
	}

	/* 4、创建设备 */
	imx6uirq.device = device_create(imx6uirq.class, NULL, imx6uirq.devid, NULL, IMX6UIRQ_NAME);
	if (IS_ERR(imx6uirq.device)) {
		return PTR_ERR(imx6uirq.device);
	}
		
	/* 5、始化按键 */
	atomic_set(&imx6uirq.keyvalue, INVAKEY);
	atomic_set(&imx6uirq.releasekey, 0);
	keyio_init();
	return 0;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit imx6uirq_exit(void)
{
	unsigned i = 0;
	/* 删除定时器 */
	del_timer_sync(&imx6uirq.timer);	/* 删除定时器 */
		
	/* 释放中断 */	
	for (i = 0; i < KEY_NUM; i++) {
		free_irq(imx6uirq.irqkeydesc[i].irqnum, &imx6uirq);
		gpio_free(imx6uirq.irqkeydesc[i].gpio);
	}
	cdev_del(&imx6uirq.cdev);
	unregister_chrdev_region(imx6uirq.devid, IMX6UIRQ_CNT);
	device_destroy(imx6uirq.class, imx6uirq.devid);
	class_destroy(imx6uirq.class);
}	
	
module_init(imx6uirq_init);
module_exit(imx6uirq_exit);
MODULE_LICENSE("GPL");
```

### 4.2 测试 APP

```c
#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "poll.h"
#include "sys/select.h"
#include "sys/time.h"
#include "linux/ioctl.h"
#include "signal.h"
/***************************************************************
描述	   	: 异步通知测试APP
***************************************************************/

static int fd = 0;	/* 文件描述符 */

/*
 * SIGIO信号处理函数
 * @param - signum 	: 信号值
 * @return 			: 无
 */
static void sigio_signal_func(int signum)
{
	int err = 0;
	unsigned int keyvalue = 0;

	err = read(fd, &keyvalue, sizeof(keyvalue));
	if(err < 0) {
		/* 读取错误 */
	} else {
		printf("sigio signal! key value=%d\r\n", keyvalue);
	}
}

/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
	int flags = 0;
	char *filename;

	if (argc != 2) {
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = argv[1];
	fd = open(filename, O_RDWR);
	if (fd < 0) {
		printf("Can't open file %s\r\n", filename);
		return -1;
	}

	/* 设置信号SIGIO的处理函数 */
	signal(SIGIO, sigio_signal_func);
	
	fcntl(fd, F_SETOWN, getpid());		/* 设置当前进程接收SIGIO信号 	*/
	flags = fcntl(fd, F_GETFL);			/* 获取当前的进程状态 			*/
	fcntl(fd, F_SETFL, flags | FASYNC);	/* 设置进程启用异步通知功能 	*/	

	while(1) {
		sleep(2);
	}

	close(fd);
	return 0;
}
```

