# platform 设备驱动

## 1. platform 驱动模型简介

为了适应Linux环境中的总线、驱动、设备模型，Linux提出了 platform 虚拟总线，相应的结构体 platform_driver 和 platforma_device

> Linux 内核 用 bus_type 结构体来表示总线， include/linux/device.h
>
> ```c
> struct bus_type{
>     ...
> 	int (*match)(struct device *dev, struct device_driver *drv);	// 用于实现设备和驱动之间的匹配问题
>     ...
> };
> ```

```c
/* platform 是 bus_type 的一个具体实例 */
struct bus_type platform_bus_type = {
	.name = "platform",
	.dev_groups = platform_dev_groups,
	.match = platform_match,
	.uevent = platform_uevent,
	.pm = &platform_dev_pm_ops,
}；
```

## 2. platform 驱动

驱动编程的重点在于 probe函数实现和实现匹配方法上面

![1680004353254](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1680004353254.png)

![1680004977210](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1680004977210.png)

![1680005002340](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1680005002340.png)

定义并初始化好 platform_driver 结构体之后，需要向内核注册一个 platform 驱动

```c
int platform_driver_register(struct platform_driver *driver)
driver: 要注册的 platform 驱动
返回值： 负数，失败；0，成功
```

驱动卸载函数中调用函数卸载 platform 驱动

```c
void platform_driver_unregister(struct platform_driver *drv);
drv: 要卸载的驱动
```

### 2.1 platform驱动框架

```c
/* 设备结构体 */
struct xxx_dev{
	struct cdev cdev;
	/* 设备结构体其他具体内容 */
};

struct xxx_dev_ xxxdev;	// 定义个设备结构体变量

static int xxx_open(struct inode *inode, struct file *filp)
{
	/* 函数具体内容 */
	return 0;
}

static ssize_t xxx_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	/* 函数具体内容 */
	return 0;
}

/* 字符设备驱动操作集 */
static struct file_operations xxx_fops = {
	.owner = THIS_MODULE,
	.open = xxx_open,
	.write = xxx_write,
};

/* probe 函数 */
static int xxx_probe(struct platform_device *dev)
{
	...
	cdev_init(&xxxdev.cdev, &xxx_fops);	/* 注册字符设备驱动 */
	/* 函数具体操作 */
	return 0;
}

static int xxx_remove(struct platform_device *dev)
{
	....
	cdev_del(&xxxdev.cdev); // 删除 cdev
	/* 具体函数内容 */
	return 0；
}

/* 匹配列表 */
static const struct of_device_id xxx_of_match[] = {
	{.compatible = "xxx-gpio"},
	{/* Sentinel */},
};

/* platform 驱动结构体 */
static struct platform_driver xxx_driver  = {
	.driver = {
		.name = "xxx",
		.of_match_table = xxx_of_match,
	},
	.probe = xxx_probe,
	.remove = xxx_remove,
};

/* 驱动模块加载 */
static int __init xxx_driver_init(void)
{
	return platform_driver_register(&xxx_driver);
}

/* 驱动模块卸载 */
static int __init xxxdriver_init(void)
{
	return platform_driver_register(&xxx_driver);
}

/* 驱动模块卸载 */
static void __exit xxxdriver_exit(void)
{
	platform_driver_unregister(&xxx_driver);
}

module_init(xxxdriver_init);
module_exit(xxxdriver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lvqifeng");
```

## 3. platform_device 结构体（设备）

内核如果支持设备树就可以不使用 platform_device 来描述设备，改用设备树描述

include/linux/platform_device.h

![1680006725038](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1680006725038.png)

- name字段需要和所使用的 platform 驱动的name字段相同，否则设备就无法匹配到对应的驱动
- resource 表示资源，也就是设备信息

![1680006824919](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1680006824919.png)

- flag 表示资源类型 include/linux/ioport.h

![1680006881682](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1680006881682.png)

定义和初始化 platform_device 结构体后，需要使用函数向内核注册 设备信息

```c
int platform_device_register(struct platform_device *pdev)
pdev: 要注册的 platform 设备
返回值： 负值 失败；0 成功
```

注销相应的 platform 设备

```c
void platform_device_unregister(struct platform_device *pdev);
pdev:要注销的 platform 设备
```

### 3.1 platform  设备框架

```c
/* 定义寄存器地址 */
#define PERIPH1_REGISTER_BASE	(0X20000000) /* 外设寄存器1首地址 */
#define PERIPH2_REGISTER_BASE	(0X020E0068) /* 外设寄存器2首地址 */
#define REGISTER_LENGTH			4

/* 资源 */
static struct resource xxx_resources[] = {
	[0] = {
		.start = PERIPH1_REGISTER_BASE,
		.end = (PERIPH1_REGISTER_BASE + REGISTER_LENGTH -1),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = PERIPH2_REGISTER_BASE,
		.end = (PERIPH2_REGISTER_BASE + REGISTER_LENGTH -1),
		.flags = IORESOURCE_MEM,
	},
};

/* platform 设备结构体 */
static struct platform_device xxxdevice = {
	.name = "xxx_gpio",
	.id = -1,
	.num_resources = ARRAY_SIZE(xxx_resources),
	.resource = xxx_resources,
};

/* 模块加载 */
static int __init xxxdevice_init(void)
{
	return platform_device_register(&xxxdevice);
}

/* 模块注销 */
static void __exit xxx_resourcesdevice_exit(void)
{
	platform_device_unregister(&xxxdevice);
}

module_init(xxxdevice_init);
module_exit(xxx_resourcesdevice_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lvqifeng");
```

## 4. 基于 platform 设备驱动的 LED 电灯实验

- leddevice.c 设备文件
- leddriver.c 驱动文件

```c
/* leddevice.c */
#include<linux/types.h>
#include<linux/kernel.h>
#include<linux/delay.h>
#include<linux/ide.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/errno.h>
#include<linux/gpio.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/of_gpio.h>
#include<linux/semaphore.h>
#include<linux/timer.h>
#include<linux/irq.h>
#include<linux/wait.h>
#include<linux/poll.h>
#include<linux/fs.h>
#include<linux/fcntl.h>
#include<linux/platform_device.h>
#include<asm/mach/map.h>
#include<asm/uaccess.h>
#include<asm/io.h>

/* 寄存器地址定义 */
#define CCM_CCGR1_BASE (0X020C406C)
#define SW_MUX_GPIO1_IO03_BASE (0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE (0X020E02F4)
#define GPIO1_DR_BASE (0X0209C000)
#define GPIO1_GDIR_BASE (0X0209C004)
#define REGISTER_LENGTH 4

/* 释放 platform_device 模块，释放函数 */
static void led_release(struct device *dev)
{
	printk("led device released!\r\n");
}

/* 设备资源信息， LED0所用到的所有寄存器 */
static struct resourse led_resources[] = {
	[0] = {
		.start = CCM_CCGR1_BASE,
		.end = (CCM_CCGR1_BASE + REGISTER_LENGTH -1),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = SW_MUX_GPIO1_IO03_BASE,
		.end = (SW_MUX_GPIO1_IO03_BASE + REGISTER_LENGTH -1),
		.flags = IORESOURCE_MEM,
	},
	[2] = {
		.start = SW_PAD_GPIO1_IO03_BASE,
		.end = (SW_PAD_GPIO1_IO03_BASE + REGISTER_LENGTH -1),
		.flags = IORESOURCE_MEM,
	},
	[3] = {
		.start = GPIO1_DR_BASE,
		.end = (GPIO1_DR_BASE + REGISTER_LENGTH -1),
		.flags = IORESOURCE_MEM,
	},
	[4] = {
		.start = GPIO1_GDIR_BASE,
		.end = (GPIO1_GDIR_BASE + REGISTER_LENGTH -1),
		.flags = IORESOURCE_MEM,
	},
};

/* platform 设备结构体 */
static struct platform_device leddevice = {
	.name = "imx6ul-led",
	.id = -1,
	.dev = {
		.release = &led_release,
	},
	.num_resources = ARRAY_SIZE(led_resources),
	.resource = led_resources,
};

/* 设备模块加载 */
static int __init leddevice_init(void)
{
	return platform_device_register(&leddevice);
}

/* 设备模块注销 */
static void __exit leddevice_exit(void)
{
	platform_device_unregister(&leddevice);
}

module_init(leddevice_init);
module_exit(leddevice_exit);
MODULE_LINENSE("GPL");
MODULE_AUTHOR("lvqifeng");
```

------

```c
/* leddriver.c */
#include<linux/types.h>
#include<linux/kernel.h>
#include<linux/delay.h>
#include<linux/ide.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/errno.h>
#include<linux/gpio.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/of_gpio.h>
#include<linux/semaphore.h>
#include<linux/timer.h>
#include<linux/irq.h>
#include<linux/wait.h>
#include<linux/poll.h>
#include<linux/fs.h>
#include<linux/fcntl.h>
#include<linux/platform_device.h>
#include<asm/mach/map.h>
#include<asm/uaccess.h>
#include<asm/io.h>

#define LEDDEV_CNT	1			/* 设备号长度 */
#define LEDDEV_NAME	"platled"	/* 设备名字 */
#define LEDOFF	0
#define LEDON	1

/* 寄存器名 */
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

/* leddev 设备结构体 */
static leddev_dev{
	dev_t devid;	// 设备号
	struct cdev cdev;	// cdev
	struct class *class;	// 类
	struct device *device;	// 设备
	int major;				// 主设备号
};

struct leddev_dev leddev;	// led 设备

void led0_switch(u8 sta)
{
	u32 val = 0;
	if(sta == LEDON)
	{
		val = readl(GPIO1_DR);
		val &= ~(1 << 3);
		writel(val, GPIO1_DR);
	}
	else if(sta == LEDOFF)
	{
		val = readl(GPIO1_DR);
		val |= (1 << 3);
		writel(val, GPIO1_DR);
	}
}

/* 打开设备 */
static int led_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &leddev; /* 设置私有数据 */
	return 0;
}

/* 向设备写数据 */
static ssize_t led_write(struct file *filp, const char __user *buf,
						size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[1];
	unsigned char ledstat;
	revalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0)
	{
		return -EFAULT;
	}
	
	ledstat = databuf[0];
	if(ledstat == LEDON)
	{
		led0_switch(LEDON);
	}
	else if(ledstat == LEDOFF)
	{
		led0_switch(LEDOFF);
	}
	return 0;
}

/* 设备操作函数 */
static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.write = led_write,
};

/* platform 驱动程序的 probe 函数 */
static int led_probe(struct platform_device *dev)
{
	int i = 0;
	int ressize[5];
	u32 val = 0;
	struct resource *ledsource[5];
	
	printk("led driver and device has matched!\r\n");
	/* 1. 获取资源 */
	for(i = 0; i < 5; i++)
	{
		ledsource[i] = platform_get_resource(dev, IORESOURCE_MEM, i);
		if(!ledsource[i])
		{
			dev_err(&dev->dev, "No MEM resource for always on\n");
			return -ENXIO;
		}
		ressize[i] = resource_size(ledsource[i]);
	}
	
	/* 2. 初始化 LED */
	/* 寄存器地址映射 */
	IMX6U_CCM_CCGR1 = ioremap(ledsource[0]->start, ressize[0]);
	SW_MUX_GPIO1_IO03 = ioremap(ledsource[1]->start, ressize[1]);
	SW_PAD_GPIO1_IO03 = ioremap(ledsource[2]->start, ressize[2]);
	GPIO1_DR = ioremap(ledsource[3]->start, ressize[3]);
	GPIO1_GDIR = ioremap(ledsource[4]->start, ressize[4]);
	
	val = readl(IMX6U_CCM_CCGR1);
	val &= ~(3 << 26);		// 清除以前的值
	val |= (3 << 26);		// 设置新值
	writel(val, IMX6U_CCM_CCGR1);
	
	/* 设置 GPIO1_IO03 复用功能，将其复用为 GPIO1_IO03 */
	writel(5, SW_MUX_GPIO1_IO03);
	writel(0x10B0, SW_PAD_GPIO1_IO03);
	
	/* 设置 GPIO1_IO03 为输出功能 */
	val = readl(GPIO1_GDIR);
	val &= ~(1 << 3);	// 清除以前的值
	val |= (1 << 3);	// 设置新值
	writel(val, GPIO1_GDIR);
	
	/* 默认关闭 LED1 */
	val = readl(GPIO1_DR);
	val |= (1 << 3) ;
	writel(val, GPIO1_DR);
	
	/* 注册字符设备驱动 */
	/*1、创建设备号 */
	if(leddev.major)
	{
		/* 定义了设备号 */
		leddev.devid = MKDEV(leddev.major, 0);
		register_chrdev_region(leddev.devid, LEDDEV_CNT, LEDDEV_NAME);
	}
	else
	{
		alloc_chrdev_region(&leddev.devid, 0, LEDDEV_CNT, LEDDEV_NAME);
		leddev.major = MAJOR(leddev.devid);
	}
	/* 2、初始化 cdev */
	leddev.cdev.owner = THIS_MODULE;
	cdev_init(&leddev.cdev, &led_fops);
	
	/* 3、添加一个 cdev */
	cdev_add(&leddev.cdev, leddev.devid, LEDDEV_CNT);
	
	/* 4、创建类 */
	leddev.class = class_create(THIS_MODULE, LEDDEV_NAME);
	if (IS_ERR(leddev.class))
	{
		return PTR_ERR(leddev.device);
	}
	
	/* 5. 创建设备 */
	leddev.device = device_create(leddev.class, NULL, leddev.devid, NULL, LEDDEV_NAME);
	if(IS_ERR(leddev.device))
	{
		return PTR_ERR(leddev.device);
	}
	
	return 0;
}

/* platform 驱动程序的 remove 函数 */
static int led_remove(struct platform_device *dev)
{
	iounmap(IMX6U_CCM_CCGR1);
	iounmap(SW_MUX_GPIO1_IO03);
	iounmap(SW_PAD_GPIO1_IO03);
	iounmap(GPIO1_DR);
	iounmap(GPIO1_GDIR);
	
	cdev_del(&leddev.cdev); /* 删除 cdev */
	unregister_chrdev_region(leddev.devid, LEDDEV_CNT);
	device_destroy(leddev.class, leddev.devid);
	class_destroy(leddev.class);
	
	return 0;
}

/* platform_driver */
static struct platform_driver led_driver = {
	.driver = {
		.name = "imx6ul-led",	// 驱动名字，用于和设备匹配
	},
	.probe = led_probe,
	.remove = led_remove,
};

/* 模块加载函数 */
static int __init leddriver_init(void)
{
	return platform_driver_register(&led_driver);
}

/* 驱动模块卸载函数 */
static void __exit leddriver_exit(void)
{
	platform_driver_unregister(&led_driver);
}

module_init(leddriver_init);
module_exit(leddriver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("zuozhongkai");
```



