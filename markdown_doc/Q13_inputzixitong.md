# Linux Input 子系统

input 子系统就是管理输入的子系统；input 子系统 分为 input 驱动层、 input 核心层、 input 事件处理层（驱动分层的体现）

![1685019553221](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685019553221.png)

- 驱动层：输入设备的具体驱动程序，向内核层报告输入内容
- 核心层：为驱动层提供输入设备注册和操作接口；通知事件层对输入事件进行处理
- 事件层：主要和用户空间进行交互

## 1. input 驱动编写流程

input 核心层会向 Linux 内核注册一个字符设备；

```c
/* drivers/input/input.c 就是 input 输入子系统的核心层 */
struct class input_class = {
	.name = "input",
	.devnode = input_devnode,
};
...
static int __init input_init(void)
{
    int err;
    
    err = class_register(&input_class);	// 注册一个类 /sys/class/input 子目录
    if(err) 
    {
        pr_err("unable to register input_dev class\n");
        return err;
    }
    
    err = input_proc_init();
    if (err)
        goto fail1;
    
    err = register_chrdev_region(MKDEV(INPUT_MAJOR, 0), 
                                 INPUT_MAX_CHAR_DEVICES, "input");	// 注册一个字符设备，主设备号为 INPUT_MAJOR
    if(err)
    {
        pr_err("unable to register char major %d", INPUT_MAJOR);
        goto fail2;
    }
    
    return 0;
    
fail2: input_proc_exit();
fail1: class_unregister(&input_class);
    return err;
}
```

input 子系统的所有设备主设备号都为13；再使用 input 子系统处理输入设备的时候就不需要去注册字符设备，只需要向系统注册一个 input_device 即可

```c
/* include/uapi/linux/major.h */
#define INPUT_MAJOR 13
```

**1. 注册 input_dev**

input_dev 结构体表示 input 设备

```c
/* include/linux/input.h */
struct input_dev {
    const char *name;
    const char *phys;
    const char *uniq;
    struct input_id id;
    
    unsigned long propbit[BITS_TO_LONGS(INPUT_PROP_CNT)];
    
    unsigned long evbit[BITS_TO_LONGS(EV_CNT)]; 	/* 事件类型的位图 */
    unsigned long keybit[BITS_TO_LONGS(KEY_CNT)]; 	/* 按键值的位图 */
    unsigned long relbit[BITS_TO_LONGS(REL_CNT)]; 	/* 相对坐标的位图 */
    unsigned long absbit[BITS_TO_LONGS(ABS_CNT)]; 	/* 绝对坐标的位图 */
    unsigned long mscbit[BITS_TO_LONGS(MSC_CNT)]; 	/* 杂项事件的位图 */
    unsigned long ledbit[BITS_TO_LONGS(LED_CNT)];	/* LED 相关的位图 */
    unsigned long sndbit[BITS_TO_LONGS(SND_CNT)];	/* sound 有关的位图 */
    unsigned long ffbit[BITS_TO_LONGS(FF_CNT)]; 	/* 压力反馈的位图 */
    unsigned long swbit[BITS_TO_LONGS(SW_CNT)]; 	/* 开关状态的位图 */
......
    bool devres_managed;
};
```

```c
/* evbit 表示输入事件类型 */
/* 输入事件可选类型 include/uapi/linux/input.h */
#define EV_SYN 0x00 		/* 同步事件 */
#define EV_KEY 0x01 		/* 按键事件 */
#define EV_REL 0x02 		/* 相对坐标事件 */
#define EV_ABS 0x03 		/* 绝对坐标事件 */
#define EV_MSC 0x04 		/* 杂项(其他)事件 */
#define EV_SW 0x05 			/* 开关事件 */
#define EV_LED 0x11 		/* LED */
#define EV_SND 0x12 		/* sound(声音) */
#define EV_REP 0x14 		/* 重复事件 */
#define EV_FF 0x15 			/* 压力事件 */
#define EV_PWR 0x16 		/* 电源事件 */
#define EV_FF_STATUS 0x17 	/* 压力状态事件 */
```

evbit、 keybit、 relbit 等等都是存放不同事件对应的值， 类似于 code

```c
/* 以 keybit 按键为例 */
/* include/uapi/linux/input.h */
#define KEY_RESERVED 0
#define KEY_ESC 1
#define KEY_1 2
#define KEY_2 3
#define KEY_3 4
#define KEY_4 5
#define KEY_5 6
#define KEY_6 7
#define KEY_7 8
#define KEY_8 9
#define KEY_9 10
#define KEY_0 11
......
#define BTN_TRIGGER_HAPPY39 0x2e6
#define BTN_TRIGGER_HAPPY40 0x2e7
```

input_dev 注册过程大致如下：

1. 使用 input_allocate_device 函数申请一个 input_dev 
2. 初始化 input_dev 的事件类型以及事件值 
3. 使用 input_register_device 函数向 Linux 系统注册前面初始化好的 input_dev 
4. 卸载 input驱动的时候需要先使用 input_unregister_device 函数注销掉注册的 input_dev 

```c
/* input_dev 注册过程相关 API */
struct input_dev *input_allocate_device(void)	// 申请一个 input_dev
返回值：申请到的 input_dev 结构体

void input_free_device(struct input_dev *dev)	// 释放掉前面申请的 input_dev
dev：待释放的结构体

int input_register_device(struct input_dev *dev) // 待 input_dev 结构体初始化完成后，向内核注册 Input_dev
返回值：0 注册成功；1 注册失败

void input_unregister_device(struct input_dev *dev) // 注销 input_dev 结构体
```

```c
/* 初始化并注册 input_dev 结构体 母版 */
struct input_dev *inputdev; 		/* input 结构体变量 */

/* 驱动入口函数 */
static int __init xxx_init(void)
{
    ...
    inputdev = input_allocate_device(); 	/* 申请 input_dev */
    inputdev->name = "test_inputdev"; 		/* 设置 input_dev 名字 */
    
    /*********第一种设置事件和事件值的方法***********/
    __set_bit(EV_KEY, inputdev->evbit); 	/* 设置产生按键事件 */
    __set_bit(EV_REP, inputdev->evbit); 	/* 重复事件 */
    __set_bit(KEY_0, inputdev->keybit); 	/*设置产生哪些按键值 */
    /************************************************/
    
    /*********第二种设置事件和事件值的方法***********/
    keyinputdev.inputdev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
    keyinputdev.inputdev->keybit[BIT_WORD(KEY_0)] |= BIT_MASK(KEY_0);
    /************************************************/
    
    /*********第三种设置事件和事件值的方法***********/
    keyinputdev.inputdev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
    input_set_capability(keyinputdev.inputdev, EV_KEY, KEY_0);
    /************************************************/
    
    /* 注册 input_dev */
    input_register_device(inputdev);
    ...
    return 0;
}

/* 驱动出口函数 */
static void __exit xxx_exit(void)
{
    input_unregister_device(inputdev); 		/* 注销 input_dev */
    input_free_device(inputdev); 			/* 删除 input_dev */
}
```

**2. 上报输入事件**

注册 input_dev 结构体后，还需要将输入事件上报给 Linux 内核

```c
/* 上报输入事件的 API */
// input_event 函数可以上报所有的事件类型和事件值
void input_event(struct input_dev *dev,
				unsigned int type,
				unsigned int code,
				int value)
dev：需要上报的 input_dev
type：上报的事件类型
code：事件码，也就是我们注册的按键值
value：事件值；1表示按下，0表示松开
--------------------------------------------------------------
// Linux 内核也提供了其他的针对具体事件的上报函数，这些函数其实都用到了 input_event 函数
void input_report_key(struct input_dev *dev, unsigned int code, int value);
void input_report_rel(struct input_dev *dev, unsigned int code, int value);
void input_report_abs(struct input_dev *dev, unsigned int code, int value);
void input_report_ff_status(struct input_dev *dev, unsigned int code, int value);
void input_report_switch(struct input_dev *dev, unsigned int code, int value);
void input_mt_sync(struct input_dev *dev);
/* 上报一个同步事件，告诉 Linux 内核 input 子系统上报结束 */
void input_sync(struct input_dev *dev)
```

```c
/* 按键类型事件 上报 demo */
/* 用于按键消抖的定时器服务函数 */
void timer_function(unsigned long arg)
{
    unsigned char value;
    
	value = gpio_get_value(keydesc->gpio); 	/* 读取 IO 值 */
    if(value == 0){ 	/* 按下按键 */
        /* 上报按键值 */
        input_report_key(inputdev, KEY_0, 1); /* 最后一个参数 1， 按下 */
        input_sync(inputdev); 				/* 同步事件 */
    } else { 			/* 按键松开 */
        input_report_key(inputdev, KEY_0, 0); /* 最后一个参数 0， 松开 */
        input_sync(inputdev); 				/* 同步事件 */
    }
}
```

## 2. input_event 结构体

Linux 内核使用 input_event 这个结构体来表示所有的输入事件 

```c
/* include/uapi/linux/input.h */
struct input_event {
    struct timeval time;	// 事件发生的时间
    __u16 type;
    __u16 code;
    __s32 value;
};
```

```c
/* timeval 结构体定义 */
typedef long __kernel_long_t;
typedef __kernel_long_t __kernel_time_t;
typedef __kernel_long_t __kernel_suseconds_t;

struct timeval {
    __kernel_time_t tv_sec; 		/* 秒 */
    __kernel_suseconds_t tv_usec; 	/* 微秒 */
};
```

## 2. 按键输入实验

### 2.1 实验原理图

![1685022680000](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685022680000.png)

### 2.2 修改设备树文件

1. 添加 pinctrl 节点

   ```c
   /* imx6ull-alientek-emmc.dts */
   /* 在 iomuxc 节点的 imx6ul-evk 子节点下创建一个名为 pinctrl_key 子节点 */
   pinctrl_key: keygrp {
   	fsl,pins = <
   	MX6UL_PAD_UART1_CTS_B__GPIO1_IO18 0xF080 	/* KEY0 复用为 GPIO1_IO18*/	
   	>;
   };
   ```

2. 添加 KEY 设备节点

   ```c
   /* 在根节点 / 下创建 KEY 设备节点 */
   key {
       #address-cells = <1>;
       #size-cells = <1>;
       compatible = "atkalpha-key";
       pinctrl-names = "default";
       pinctrl-0 = <&pinctrl_key>;
       key-gpio = <&gpio1 18 GPIO_ACTIVE_LOW>; /* KEY0 */
       status = "okay";
   };
   ```

### 2.3 按键驱动程序编写

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
#include <linux/input.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
/***************************************************************
描述	   	: Linux按键input子系统实验
***************************************************************/
#define KEYINPUT_CNT		1			/* 设备号个数 	*/
#define KEYINPUT_NAME		"keyinput"	/* 名字 		*/
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

/* keyinput设备结构体 */
struct keyinput_dev{
	dev_t devid;			/* 设备号 	 */
	struct cdev cdev;		/* cdev 	*/
	struct class *class;	/* 类 		*/
	struct device *device;	/* 设备 	 */
	struct device_node	*nd; /* 设备节点 */
	struct timer_list timer;/* 定义一个定时器*/
	struct irq_keydesc irqkeydesc[KEY_NUM];	/* 按键描述数组 */
	unsigned char curkeynum;				/* 当前的按键号 */
	struct input_dev *inputdev;		/* input结构体 */
};

struct keyinput_dev keyinputdev;	/* key input设备 */

/* @description		: 中断服务函数，开启定时器，延时10ms，
 *				  	  定时器用于按键消抖。
 * @param - irq 	: 中断号 
 * @param - dev_id	: 设备结构。
 * @return 			: 中断执行结果
 */
static irqreturn_t key0_handler(int irq, void *dev_id)
{
	struct keyinput_dev *dev = (struct keyinput_dev *)dev_id;

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
	struct keyinput_dev *dev = (struct keyinput_dev *)arg;

	num = dev->curkeynum;
	keydesc = &dev->irqkeydesc[num];
	value = gpio_get_value(keydesc->gpio); 	/* 读取IO值 */
	if(value == 0){ 						/* 按下按键 */
		/* 上报按键值 */
		//input_event(dev->inputdev, EV_KEY, keydesc->value, 1);
		input_report_key(dev->inputdev, keydesc->value, 1);/* 最后一个参数表示按下还是松开，1为按下，0为松开 */
		input_sync(dev->inputdev);
	} else { 									/* 按键松开 */
		//input_event(dev->inputdev, EV_KEY, keydesc->value, 0);
		input_report_key(dev->inputdev, keydesc->value, 0);
		input_sync(dev->inputdev);
	}	
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
	
	keyinputdev.nd = of_find_node_by_path("/key");
	if (keyinputdev.nd== NULL){
		printk("key node not find!\r\n");
		return -EINVAL;
	} 

	/* 提取GPIO */
	for (i = 0; i < KEY_NUM; i++) {
		keyinputdev.irqkeydesc[i].gpio = of_get_named_gpio(keyinputdev.nd ,"key-gpio", i);
		if (keyinputdev.irqkeydesc[i].gpio < 0) {
			printk("can't get key%d\r\n", i);
		}
	}
	
	/* 初始化key所使用的IO，并且设置成中断模式 */
	for (i = 0; i < KEY_NUM; i++) {
		memset(keyinputdev.irqkeydesc[i].name, 0, sizeof(name));	/* 缓冲区清零 */
		sprintf(keyinputdev.irqkeydesc[i].name, "KEY%d", i);		/* 组合名字 */
		gpio_request(keyinputdev.irqkeydesc[i].gpio, name);
		gpio_direction_input(keyinputdev.irqkeydesc[i].gpio);	
		keyinputdev.irqkeydesc[i].irqnum = irq_of_parse_and_map(keyinputdev.nd, i);
	}
	/* 申请中断 */
	keyinputdev.irqkeydesc[0].handler = key0_handler;
	keyinputdev.irqkeydesc[0].value = KEY_0;
	
	for (i = 0; i < KEY_NUM; i++) {
		ret = request_irq(keyinputdev.irqkeydesc[i].irqnum, keyinputdev.irqkeydesc[i].handler, 
		                 IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, keyinputdev.irqkeydesc[i].name, &keyinputdev);
		if(ret < 0){
			printk("irq %d request failed!\r\n", keyinputdev.irqkeydesc[i].irqnum);
			return -EFAULT;
		}
	}

	/* 创建定时器 */
	init_timer(&keyinputdev.timer);
	keyinputdev.timer.function = timer_function;

	/* 申请input_dev */
	keyinputdev.inputdev = input_allocate_device();
	keyinputdev.inputdev->name = KEYINPUT_NAME;
#if 0
	/* 初始化input_dev，设置产生哪些事件 */
	__set_bit(EV_KEY, keyinputdev.inputdev->evbit);	/* 设置产生按键事件          */
	__set_bit(EV_REP, keyinputdev.inputdev->evbit);	/* 重复事件，比如按下去不放开，就会一直输出信息 		 */

	/* 初始化input_dev，设置产生哪些按键 */
	__set_bit(KEY_0, keyinputdev.inputdev->keybit);	
#endif

#if 0
	keyinputdev.inputdev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
	keyinputdev.inputdev->keybit[BIT_WORD(KEY_0)] |= BIT_MASK(KEY_0);
#endif

	keyinputdev.inputdev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
	input_set_capability(keyinputdev.inputdev, EV_KEY, KEY_0);

	/* 注册输入设备 */
	ret = input_register_device(keyinputdev.inputdev);
	if (ret) {
		printk("register input device failed!\r\n");
		return ret;
	}
	return 0;
}

/*
 * @description	: 驱动入口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init keyinput_init(void)
{
	keyio_init();
	return 0;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit keyinput_exit(void)
{
	unsigned int i = 0;
	/* 删除定时器 */
	del_timer_sync(&keyinputdev.timer);	/* 删除定时器 */
		
	/* 释放中断 */
	for (i = 0; i < KEY_NUM; i++) {
		free_irq(keyinputdev.irqkeydesc[i].irqnum, &keyinputdev);
	}
	/* 释放input_dev */
	input_unregister_device(keyinputdev.inputdev);
	input_free_device(keyinputdev.inputdev);
}

module_init(keyinput_init);
module_exit(keyinput_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("zuozhongkai");

```

### 2.4 测试APP

```c
/* keyApp.c */
#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <linux/input.h>

/* 定义一个input_event变量，存放输入事件信息 */
static struct input_event inputevent;

/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
	int fd;
	int err = 0;
	char *filename;

	filename = argv[1];

	if(argc != 2) {
		printf("Error Usage!\r\n");
		return -1;
	}

	fd = open(filename, O_RDWR);
	if (fd < 0) {
		printf("Can't open file %s\r\n", filename);
		return -1;
	}

	while (1) {
		err = read(fd, &inputevent, sizeof(inputevent));
		if (err > 0) { /* 读取数据成功 */
			switch (inputevent.type) {


				case EV_KEY:
					if (inputevent.code < BTN_MISC) { /* 键盘键值 */
						printf("key %d %s\r\n", inputevent.code, inputevent.value ? "press" : "release");
					} else {
						printf("button %d %s\r\n", inputevent.code, inputevent.value ? "press" : "release");
					}
					break;

				/* 其他类型的事件，自行处理 */
				case EV_REL:
					break;
				case EV_ABS:
					break;
				case EV_MSC:
					break;
				case EV_SW:
					break;
			}
		} else {
			printf("读取数据失败\r\n");
		}
	}
	return 0;
}
```

