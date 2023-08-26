## pinctrl 和 gpio 子系统

## pinctrl 系统

根据设备树来设置引脚的复用功能和电气属性（就是用来设置引脚干什么的）

### 1. PIN 配置信息（设备树里面设置 PIN 信息）

```
/* imx6ull.dtsi */
iomuxc: iomuxc@020e0000 {
	compatible = "fsl,imx6ul-iomuxc";	// 用来查找对应的驱动文件
	reg = <0x020e0000 0x4000>;
	}；

&iomuxc {
	pinctrl-names = "default";
	pinctrl-0 = <&pinctrl_hog_1>;	// iomuxc 驱动会自动初始化 pinctrl_hog_1 节点下的所有PIN
	imx6ul-evk {
		pinctrl_hog_1: hoggrp-1 {
			fsl,pins = <
				MX6UL_PAD_UART1_RTS_B__GPIO1_IO19 0x17059	//<复用功能> <电气属性 conf_reg 的值>
				MX6UL_PAD_GPIO1_IO05__USDHC1_VSELECT 0x17059
				MX6UL_PAD_GPIO1_IO09__GPIO1_IO09 0x17059
				MX6UL_PAD_GPIO1_IO00__ANATOP_OTG1_ID 0x13058
				>;
			};
		....
		};
	};
```

```
/* imx6ul-pinfunc.h 宏定义 */
 #define MX6UL_PAD_UART1_RTS_B__GPIO1_IO19 0x0090 0x031C 0x0000 0x5 0x0
 --------------------------------------------------------------
 0x0090 0x031C 0x0000 0x5 0x0
 <mux_reg conf_reg input_reg mux_mode input_val>
 mux_reg：mux_reg 寄存器偏移地址 = 0x020e0000 + 0x0090；即 IOMUXC_SW_MUX_CTL_PAD_UART1_RST_B 的寄存器地址（复用寄存器地址）
 conf_reg：conf_reg 寄存器偏移地址 = 0x020e0000 + 0x031C；即 IOMUXC_SW_PAD_CTL_PAD_UART1_RST_B 的寄存器地址
 input_reg：input_reg 寄存器偏移地址 = 0x020e0000 + 0x0000；有的外设没有 Input 寄存器，所以不需要设置
 mux_mode：mux_reg 寄存器的值为 0x5；即设置引脚复用为 GPIO1_IO19
 input_val：input_reg 寄存器的值
```

## GPIO 子系统

pinctrl 用来设置引脚的复用功能和电气属性，如果 pinctrl 子系统将一个引脚复用为 GPIO 的话，接下来姐可以用到 gpio 子系统；用来初始化 GPIO 并且提供相应的 API 函数，比如设置 GPIO 为输入输出，读取 GPIO 的值

### 1. 设备树中的 gpio 信息

在 pinctrl 设置为 gpio 复用后，需要在相应的设备节点下添加一个属性来描述哪一个 GPIO 

```
&usdhc1 {
	pinctrl-names = "default", "state_100mhz", "state_200mhz";
	pinctrl-0 = <&pinctrl_usdhc1>;
	...
	pinctrl-3 = <&pinctrl_hog_1>; 	// 指明 SD 卡所用的引脚信息，供设备驱动设置引脚的复用功能
	cd-gpios = <&gpio1 19 GPIO_ACTIVE_LOW>;	// 描述 SD 卡的 CD 引脚使用的哪个 IO
	...
};
```

```
/* imx6ull.dtsi */
gpio1: gpio@0209c000 {
	compatible = "fsl,imx6ul-gpio", "fsl,imx35-gpio";	// 用来匹配 GPIO 驱动程序
	reg = <0x0209c000 0x4000>;	// gpio1 控制器的寄存器基地址
	interrupts = <GIC_SPI 66 IRQ_TYPE_LEVEL_HIGH>,
				<GIC_SPI 67 IRQ_TYPE_LEVEL_HIGH>;
	gpio-controller;		// 表示 gpio1 节点是个 GPIO 控制器
	#gpio-cells = <2>;		// 表示有两个 cell， 第一个编号，第二个表示 GPIO 极性
	interrupt-controller;
	#interrupt-cells = <2>;
}；
```

### 2. gpio 子系统 API 函数

- 申请一个 GPIO 管脚

```
int gpio_request(unsigned gpio, const char *label);
gpio：要申请的 GPIO 标号
label：给 GPIO 设置个名字
返回值： 0->申请成功  其他值->申请失败
```

- 释放某个 GPIO

```
void gpio_free(unsigned gpio);
gpio：要释放的 GPIO 标号
返回值：无
```

- 设置某个 GPIO 为输入

```
int gpio_direction_input(unsigned gpio)
gpio：要设置为输入的gpio标号
返回值：0-> 成功 负值->失败
```

- 设置某个 GPIO 为输出，并设置默认输出值

```
int gpio_direction_output(unsigned gpio, int value)
gpio： 要设置为输出的 gpio 标号
value：GPIO默认输出值
返回值： 0->成功 负值->失败
```

- 获取某个GPIO的值（0或者1）

```
#define gpio_get_value	__gpio_get_value
int __gpio_get_value(unsigned gpio)
gpio: gpio 标号
放回值：非负值：得到的GPIO值； 负值，获取失败
```

- 设置某个 GPIO 的值

```
#define gpio_set_value	__gpio_set_value
void __gpio_set_value(unsigned gpio, int value)
gpio：要设置的 gpio 标号
value：要设置的值
返回值： 无
```

------

### 0. 与 GPIO 相关的 OF 函数

- 获取设备树某个属性里面定义了几个 GPIO 信息（空的 GPIO 信息也会被统计）

```
int of_gpio_named_count(struct device_node *np, const char *propname)
np：设备节点
propname：要统计的 GPIO 属性
返回值： 正值，统计到的 GPIO 数量
```

- 统计 "gpios" 这个属性的 GPIO 数量

```
int of_gpio_count(struct device_node *np)
np：设备节点
返回值： 统计到的 GPIO 数量
```

- 获取 GPIO 编号；将<&gpio5 7 GPIO_ACTIVE_LOW>的属性信息转换为对应的GPIO编号

```
int of_get_named_gpio(struct device_node *np, const char *propname, int index)
np：设备节点
propname：获取 GPIO 信息的属性名
index：GPIO索引（一个属性里面可能包含多个 GPIO）
返回值：正值->获得的GPIO编号； 负值，失败
```

### 1. 修改设备树

1. 添加 pinctrl 节点

```
/* iomuxc 节点的 imx6ul-evk 子节点下 创建一个 pinctrl_led 子节点 */
pinctrl_led:ledgrp{
	fsl,pins = <
		MX6UL_PAD_GPIO1_IO03_GPIO1_IO03	0X10B0
	>;
};
// 将 GPIO1_IO03 这个 PIN 复用为 GPIO1_IO03, 电气属性值为 0X10B0
```

2. 添加LED设备节点

```c
/* 根目录下，创建 gpioled 节点 */
gpioled{
	#address-cells = <1>;
	#size-cells = <1>;
	compatible = "atkalpha-gpioled";
	pinctrl-names = "default";
	pinctrl-0 = <&pinctrl_led>;		// 添加 pinctrl 信息
	led-gpio = <&gpio1 3 GPIO_ACTIVE_LOW>;	// 添加 GPIO 信息
	status = "okay";
};
```

3. 检查 PIN 是否被其他外设所使用

### 2. LED驱动程序

```
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

#define GPIOLED_CNT 1 			/* 设备号个数 */
#define GPIOLED_NAME "gpioled" 	/* 名字 */
#define LEDOFF 0 				/* 关灯 */
#define LEDON 1 				/* 开灯 */

/* gpioled 设备结构体 */
struct gpioled_dev{
	dev_t devid; 					/* 设备号 */
	struct cdev cdev; 				/* cdev */
	struct class *class; 			/* 类 */
	struct device *device; 			/* 设备 */
	int major; 					/* 主设备号 */
	int minor; 					/* 次设备号 */
	struct device_node *nd; 	/* 设备节点 */
	int led_gpio; 			/* led 所使用的 GPIO 编号 */
};

struct gpioled_dev gpioled; 		/* led 设备 */

/* 打开设备 */
static int led_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &gpioled; 	/* 设置私有数据 */
	return 0;
}

/* 读取数据 */
static ssize_t led_read(struct file *filp, char __user *buf,
						size_t cnt, loff_t *offt)
{
	return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf,
						size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[1];
	unsigned char ledstat;
	struct gpioled_dev *dev = filp->private_data;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0) 
	{
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}
	
	ledstat = databuf[0]; /* 获取状态值 */

	if(ledstat == LEDON) 
	{
		gpio_set_value(dev->led_gpio, 0); /* 打开 LED 灯 */
	} 
	else if(ledstat == LEDOFF) 
	{
		gpio_set_value(dev->led_gpio, 1); /* 关闭 LED 灯 */
	}
	return 0;
}

/* 关闭和释放设备 */
static int led_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作函数 */
static struct file_operations gpioled_fops = {
		.owner = THIS_MODULE,
		.open = led_open,
		.read = led_read,
		.write = led_write,
		.release = led_release,
};

/* 驱动入口函数 */
static int __init led_init(void)
{
	int ret = 0;

	/* 设置 LED 所使用的 GPIO */
	/* 1、获取设备节点： gpioled */
	gpioled.nd = of_find_node_by_path("/gpioled");
	if(gpioled.nd == NULL) 
	{
		printk("gpioled node cant not found!\r\n");
		return -EINVAL;
	} 
	else 
	{
		printk("gpioled node has been found!\r\n");
	}
	
	/* 2、 获取设备树中的 gpio 属性，得到 LED 所使用的 LED 编号 */
	gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "led-gpio", 0);
	if(gpioled.led_gpio < 0) 
	{
		printk("can't get led-gpio");
		return -EINVAL;
	}
	printk("led-gpio num = %d\r\n", gpioled.led_gpio);
	
	/* 3、设置 GPIO1_IO03 为输出，并且输出高电平，默认关闭 LED 灯 */
	ret = gpio_direction_output(gpioled.led_gpio, 1);
	if(ret < 0) 
	{
		printk("can't set gpio!\r\n");
	}
	
	/* 注册字符设备驱动 */
	/* 1、创建设备号 */
	if (gpioled.major) 
	{ /* 定义了设备号 */
		gpioled.devid = MKDEV(gpioled.major, 0);
		register_chrdev_region(gpioled.devid, GPIOLED_CNT, GPIOLED_NAME);
	}
    else 
    { /* 没有定义设备号 */
		alloc_chrdev_region(&gpioled.devid, 0, GPIOLED_CNT, GPIOLED_NAME); /* 申请设备号 */
		gpioled.major = MAJOR(gpioled.devid); /* 获取分配号的主设备号 */
		gpioled.minor = MINOR(gpioled.devid); /* 获取分配号的次设备号 */
	}
	printk("gpioled major=%d,minor=%d\r\n",gpioled.major, gpioled.minor);

	/* 2、初始化 cdev */
	gpioled.cdev.owner = THIS_MODULE;
	cdev_init(&gpioled.cdev, &gpioled_fops);
	
	/* 3、添加一个 cdev */
	cdev_add(&gpioled.cdev, gpioled.devid, GPIOLED_CNT);

	 /* 4、创建类 */
	gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);
	if (IS_ERR(gpioled.class)) 
	{
		return PTR_ERR(gpioled.class);
	}

	/* 5、创建设备 */
	gpioled.device = device_create(gpioled.class, NULL,
							gpioled.devid, NULL, GPIOLED_NAME);
	if (IS_ERR(gpioled.device)) 
	{
		return PTR_ERR(gpioled.device);
	}
	return 0;
}

/* 驱动出口函数 */
static void __exit led_exit(void)
{
	/* 注销字符设备驱动 */
	cdev_del(&gpioled.cdev); /* 删除 cdev */
	unregister_chrdev_region(gpioled.devid, GPIOLED_CNT); /* 注销 */

	device_destroy(gpioled.class, gpioled.devid);
	class_destroy(gpioled.class);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("zuozhongkai");
```

### 3. LED测试APP

同之前

### 4. 编译运行 驱动程序和测试APP

```
depmod
modprobe gpioled.ko		// 加载驱动

./ledApp /dev/gpioled 1 // 打开LED灯
./ledApp /dev/gpioled 0 // 关闭LED灯

rmmod gpioled.ko
```





