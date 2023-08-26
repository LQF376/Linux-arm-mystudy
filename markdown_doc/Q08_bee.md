## 蜂鸣器（基于 pinctrl 和 gpio 子系统）
![1685011916708](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685011916708.png)

### 1. 修改设备树

1. 添加 pinctrl 节点

```c
/* iomuxc 节点的 imx6ul-evk 子节点下 创建一个 pinctrl_beep 子节点 */
pinctrl_beep:beepgrp{
	fsl,pins = <
		MX6UL_PAD_SNVS_TAMPER1_GPIO5_IO01	0X10B0
	>;
};
// 将 GPIO1_IO03 这个 PIN 复用为 GPIO1_IO03, 电气属性值为 0X10B0
```

2. 添加LED设备节点

```c
/* 根目录下，创建 beep 节点 */
beep{
	#address-cells = <1>;
	#size-cells = <1>;
	compatible = "atkalpha-beep";
	pinctrl-names = "default";
	pinctrl-0 = <&pinctrl_beep>;		// 添加 pinctrl 信息
	led-gpio = <&gpio5 1 GPIO_ACTIVE_HIGH>;	// 添加 GPIO 信息
	status = "okay";
};
```

3. 检查 PIN 是否被其他外设所使用

> 先检查 PIN 为 SNVS_TAMPER1 这个 PIN 有没有被其他 pinctrl 节点使用，如果有就屏蔽掉
>
> 再检查GPIO5_IO01 这个 GPIO 有没有被其他外设使用，有也屏蔽掉

重新编译设备树文件，启动Linux系统，进入 /proc/device-tree 中查看 “beep” 节点是否存在

### 2. LED驱动程序

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

#define BEEP_CNT 1 			/* 设备号个数 */
#define BEEP_NAME "gpioled" 	/* 名字 */
#define BEEPOFF 0 				/* 关灯 */
#define BEEPON 1 				/* 开灯 */

/* gpioled 设备结构体 */
struct beep_dev{
	dev_t devid; 					/* 设备号 */
	struct cdev cdev; 				/* cdev */
	struct class *class; 			/* 类 */
	struct device *device; 			/* 设备 */
	int major; 					/* 主设备号 */
	int minor; 					/* 次设备号 */
	struct device_node *nd; 	/* 设备节点 */
	int beep_gpio; 			/* led 所使用的 GPIO 编号 */
};

struct beep_dev beep; 		/* led 设备 */

/* 打开设备 */
static int led_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &beep; 	/* 设置私有数据 */
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
	unsigned char beepstat;
	struct beep_dev *dev = filp->private_data;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0) 
	{
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}
	
	beepstat = databuf[0]; /* 获取状态值 */

	if(beepstat == BEEPON) 
	{
		gpio_set_value(dev->beep_gpio, 0); /* 打开 BEEP */
	} 
	else if(beepstat == BEEPOFF) 
	{
		gpio_set_value(dev->beep_gpio, 1); /* 关闭 BEEP */
	}
	return 0;
}

/* 关闭和释放设备 */
static int beep_release(struct inode *inode, struct file *filp)
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
static int __init beep_init(void)
{
	int ret = 0;

	/* 设置 LED 所使用的 GPIO */
	/* 1、获取设备节点： gpioled */
	beep.nd = of_find_node_by_path("/beep");
	if(beep.nd == NULL) 
	{
		printk("beep node cant not found!\r\n");
		return -EINVAL;
	} 
	else 
	{
		printk("beep node has been found!\r\n");
	}
	
	/* 2、 获取设备树中的 gpio 属性，得到 BEEP 所使用的 GPIO 编号 */
	beep.beep_gpio = of_get_named_gpio(beep.nd, "beep-gpio", 0);
	if(beep.beep_gpio < 0) 
	{
		printk("can't get beep-gpio");
		return -EINVAL;
	}
	printk("beep-gpio num = %d\r\n", beep.beep_gpio);
	
	/* 3、设置 GPIO5_IO01 为输出，并且输出高电平，默认关闭 BEEP 灯 */
	ret = gpio_direction_output(beep.beep_gpio, 1);
	if(ret < 0) 
	{
		printk("can't set gpio!\r\n");
	}
	
	/* 注册字符设备驱动 */
	/* 1、创建设备号 */
	if (beep.major) 
	{ /* 定义了设备号 */
		beep.devid = MKDEV(beep.major, 0);
		register_chrdev_region(beep.devid, BEEP_CNT, BEEP_NAME);
	}
    else 
    { /* 没有定义设备号 */
		alloc_chrdev_region(&beep.devid, 0, BEEP_CNT, BEEP_NAME); /* 申请设备号 */
		beep.major = MAJOR(beep.devid); /* 获取分配号的主设备号 */
		beep.minor = MINOR(beep.devid); /* 获取分配号的次设备号 */
	}
	printk("gpioled major=%d,minor=%d\r\n",beep.major, beep.minor);

	/* 2、初始化 cdev */
	beep.cdev.owner = THIS_MODULE;
	cdev_init(&beep.cdev, &beep_fops);
	
	/* 3、添加一个 cdev */
	cdev_add(&beep.cdev, beep.devid, BEEP_CNT);

	 /* 4、创建类 */
	beep.class = class_create(THIS_MODULE, BEEP_NAME);
	if (IS_ERR(beep.class)) 
	{
		return PTR_ERR(beep.class);
	}

	/* 5、创建设备 */
	beep.device = device_create(beep.class, NULL, beep.devid, NULL, BEEP_NAME);
	
	if (IS_ERR(beep.device)) 
	{
		return PTR_ERR(beep.device);
	}
	return 0;
}

/* 驱动出口函数 */
static void __exit beep_exit(void)
{
	/* 注销字符设备驱动 */
	cdev_del(&beep.cdev); /* 删除 cdev */
	unregister_chrdev_region(beep.devid, BEEP_CNT); /* 注销 */

	device_destroy(beep.class, beep.devid);
	class_destroy(beep.class);
}

module_init(beep_init);
module_exit(beep_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("zuozhongkai");
```

### 3. 测试APP

```c
#include"stdio.h"
#include"unistd.h"
#include"sys/types.h"
#include"sys/stat.h"
#include"fcntl.h"
#include"stdlib.h"
#include"string.h"

#define BEEPOFF 0		// 蜂鸣器关闭
#define BEEPON 1		// 蜂鸣器打开

int main(int argc, char *argv[])
{
	int fd, retvalue;
	char *filename;
	unsigned char databuf[1];
	
	if(argc != 3)
	{
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = argv[1];
	
	/* 打开 beep 驱动 */
	fd = open(filename, O_RDWR);
	if(fd < 0)
	{
		printf("file %s open failed!\r\n", argv[1]);
		return -1;
	}
	
	databuf[0] = atoi(argv[2]); /* 要执行的操作：打开或关闭 */

	/* 向/dev/beep 文件写入数据 */
	retvalue = write(fd, databuf, sizeof(databuf));
	if(retvalue < 0)
	{
		printf("BEEP Control Failed!\r\n");
		close(fd);
		return -1;
	}

	retvalue = close(fd); /* 关闭文件 */
	if(retvalue < 0)
	{
		printf("file %s close failed!\r\n", argv[1]);
		return -1;
	}
	return 0;
}
```

### 4.编译运行 驱动程序 和 测试 APP

```
depmod
modprobe beep.ko		// 加载驱动

./ledApp /dev/beep 1 // 打开LED灯
./ledApp /dev/beep 0 // 关闭LED灯

rmmod beep.ko
```

