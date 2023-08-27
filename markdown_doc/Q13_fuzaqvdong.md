# MISC驱动（结合 platfrom 和 蜂鸣器）

## 1. Linux MISC 驱动实验

MISC 杂项驱动；当某些外设无法进行分类的时候就可以使用 MISC 驱动

- 所有的 MISC 设备驱动的主设备号都为 10，不同的设备使用不同的从设备号 
- Linux 内核使用 miscdevice 结构体来表示 miscdevice 设备（include/linux/miscdevice.h）

```c
struct miscdevice {
    int minor; 				/* 子设备号 */
    const char *name; 		/* 设备名字 */
    const struct file_operations *fops; 	/* 设备操作集 */
    struct list_head list;
    struct device *parent;
    struct device *this_device;
    const struct attribute_group **groups;
    const char *nodename;
    umode_t mode;
};
```

定义一个 MISC 设备以后，我们需要设置 minor、name 和 fops 这三个成员变量

- minor：子设备号
- name：此 MISC 设备名字，当此设备注册成功以后就会在/dev 目录下生成一个名为 name 的设备文件 
- fops：字符设备的操作集合（需要用户提供编写 file_operation）

Linux 系统预定义了一些 MISC 设备的子设备号

```c
/* include/linux/miscdevice.h */
#define PSMOUSE_MINOR 1
#define MS_BUSMOUSE_MINOR 2 /* unused */
#define ATIXL_BUSMOUSE_MINOR 3 /* unused */
#define AMIGAMOUSE_MINOR 4     /* FIXME OBSOLETE */
#define ATARIMOUSE_MINOR 5  /* unused */
#define SUN_MOUSE_MINOR 6  /* unused */
......
#define MISC_DYNAMIC_MINOR 255
```

**向系统中注册/注销一个 MISC 设备**

```c
/* 注册 */
int misc_register(struct miscdevice * misc)

/* 注销 */
int misc_deregister(struct miscdevice *misc)
misc：misc 设备结构体
返回值：负值 失败； 0 成功
```

## 2. 蜂鸣器的原理图

 ![1685015318953](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685011916708-1685015318953.png)

## 3. 修改设备树（添加蜂鸣器节点）

1. 添加 pinctrl 节点

   ```
   /* iomuxc 节点的 imx6ul-evk 子节点下 创建一个 pinctrl_beep 子节点 */
   pinctrl_beep:beepgrp{
   	fsl,pins = <
   		MX6UL_PAD_SNVS_TAMPER1_GPIO5_IO01	0X10B0
   	>;
   };
   // 将 GPIO1_IO03 这个 PIN 复用为 GPIO1_IO03, 电气属性值为 0X10B0
   ```

2. 添加LED设备节点

   ```
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

## 4. 驱动程序编写

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
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define MISCBEEP_NAME "miscbeep" 	/* 名字 */
#define MISCBEEP_MINOR 144 			/* 子设备号 */
#define BEEPOFF 0 					/* 关蜂鸣器 */
#define BEEPON 1 					/* 开蜂鸣器 */

/* miscbeep 设备结构体 */
struct miscbeep_dev{
    dev_t devid; 			/* 设备号 */
    struct cdev cdev; 		/* cdev */
    struct class *class; 	/* 类 */
    struct device *device; 	/* 设备 */
    struct device_node *nd; /* 设备节点 */
    int beep_gpio; 			/* beep 所使用的 GPIO 编号 */
};

struct miscbeep_dev miscbeep; 		/* beep 设备 */


static int miscbeep_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &miscbeep; /* 设置私有数据 */
    return 0;
}

static ssize_t miscbeep_write(struct file *filp,
						const char __user *buf, size_t cnt, loff_t *offt)
{
    int retvalue;
    unsigned char databuf[1];
    unsigned char beepstat;
    struct miscbeep_dev *dev = filp->private_data;
    retvalue = copy_from_user(databuf, buf, cnt);
    if(retvalue < 0) 
    {
        printk("kernel write failed!\r\n");
        return -EFAULT;
    }
    beepstat = databuf[0]; /* 获取状态值 */
    if(beepstat == BEEPON) 
    {
        gpio_set_value(dev->beep_gpio, 0); /* 打开蜂鸣器 */
    } 
    else if(beepstat == BEEPOFF) 
    {
        gpio_set_value(dev->beep_gpio, 1); /* 关闭蜂鸣器 */
    }
    return 0;
}

/* 设备操作函数 */
static struct file_operations miscbeep_fops = {
    .owner = THIS_MODULE,
    .open = miscbeep_open,
    .write = miscbeep_write,
};

/* MISC 设备结构体 */
static struct miscdevice beep_miscdev = {
    .minor = MISCBEEP_MINOR,
    .name = MISCBEEP_NAME,
    .fops = &miscbeep_fops,
};

/* flatform 驱动的 probe 函数，当驱动与设备匹配以后此函数就会执行 */
static int miscbeep_probe(struct platform_device *dev)
{
    int ret = 0;
    printk("beep driver and device was matched!\r\n");
    /* 设置 BEEP 所使用的 GPIO */
    /* 1、获取设备节点： beep */
    miscbeep.nd = of_find_node_by_path("/beep");
    if(miscbeep.nd == NULL) 
    {
        printk("beep node not find!\r\n");
        return -EINVAL;
    }
    /* 2、 获取设备树中的 gpio 属性，得到 BEEP 所使用的 BEEP 编号 */
    miscbeep.beep_gpio = of_get_named_gpio(miscbeep.nd, "beep-gpio", 0);
    if(miscbeep.beep_gpio < 0) 
    {
        printk("can't get beep-gpio");
        return -EINVAL;
    }
    gpio_request(miscbeep.beep_gpio, "beep");
    ret = gpio_direction_output(miscbeep.beep_gpio, 1);
    if(ret < 0) 
    {
        printk("can't set gpio!\r\n");
    }
    
    /* 不需要自己注册字符设备驱动，只需要注册 misc 设备驱动即可 */
    ret = misc_register(&beep_miscdev);
    if(ret < 0)
    {
        printk("misc device register failed!\r\n");
        return -EFAULT;
    }
    
    return 0;
}

/* remove 函数，移除 platform 驱动的时候此函数会执行 */
static int miscbeep_remove(struct platform_device *dev)
{
    /* 注销设备的时候关闭蜂鸣器 */
    gpio_set_value(miscbeep.beep_gpio, 1);
    gpio_free(miscbeep.beep_gpio);
    /* 注销 misc 设备驱动 */
    misc_deregister(&beep_miscdev);
    return 0;
}

/* 匹配列表 */
static const struct of_device_id beep_of_match[] = {
    { .compatible = "atkalpha-beep" },
    { /* Sentinel */ }
}；

/* platform 驱动结构体 */
static struct platform_driver beep_driver = {
	.driver = {
		.name = "imx6ul-beep",				/* 驱动名字 */
		.of_match_table = beep_of_match, 	/* 设备树匹配表 */
	}；
	.probe = miscbeep_probe,
	.remove = miscbeep_remove,
}；

/* 驱动入口函数 */
static int __init miscbeep_init(void)
{
    return platform_driver_register(&beep_driver);
}

/* 驱动出口函数 */
static void __exit miscbeep_exit(void)
{
    platform_driver_unregister(&beep_driver);
}

module_init(miscbeep_init);
module_exit(miscbeep_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lvqifeng");
```

### 5. 编写测试 APP 

```c
/* miscbeepApp.c */
#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"

#define BEEPOFF 0
#define BEEPON 1

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
    fd = open(filename, O_RDWR); 	/* 打开 beep 驱动 */
    if(fd < 0)
    {
        printf("file %s open failed!\r\n", argv[1]);
        return -1;
    }
    
    databuf[0] = atoi(argv[2]);
    retvalue = write(fd, databuf, sizeof(databuf));
    if(retvalue < 0)
    {
        printf("BEEP Control Failed!\r\n");
        close(fd);
        return -1;
    }
    
    retvalue = close(fd); 	/* 关闭文件 */
    if(retvalue < 0)
    {
        printf("file %s close failed!\r\n", argv[1]);
        return -1;
    }
    return 0;
}
```



