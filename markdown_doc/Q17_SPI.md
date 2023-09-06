# Linux SPI 驱动

## 1.Linux 下 SPI 驱动框架

同 I2C 驱动框架，分为 主机控制器驱动 和 设备驱动

### 1.1 SPI 主机驱动

主机控制器就是 SOC 的 SPI 控制器接口

- Linux 内核使用 spi_master 结构体表示 SPI 主机驱动；include/linux/spi/spi.h
- SPI 主机驱动的核心就是申请 spi_master，然后初始化 spi_master，最后向 Linux 内核注册 spi_master

![1685438508928](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685438508928.png)

![1685438522749](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685438522749.png)

SPI 主机最终会通过 transfer 函数与SPI设备进行通信，对于 SPI 驱动编写者来说，tansfer是需要实现的；不同的 SOC 其 SPI 控制器不同，寄存器不一样；一般 SPI 主机驱动由 SOC 厂商来完成的

**申请/释放 spi_master**

```c
/* 申请 spi_master */
struct spi_master *spi_alloc_master(struct device *dev, unsigned size)
dev：设备，一般是 platform_device 中的 dev 成员变量
size：私有数据的大小； 可以通过 spi_master_get_devdata 函数获取到这些私有数据
返回值：申请到的 spi_master 结构体

/* 释放 spi_master */
void spi_master_put(struct spi_master *master)
master：要释放的 spi_master
```

**注册/注销 spi_master**

```c
int spi_register_master(struct spi_master *master)
返回值：0，成功； 负值，失败
/* I.MX6U 的 SPI 主机驱动会采用 spi_bitbang_start 这个 API 函数来完成 spi_master 的注册 */

void spi_unregister_master(struct spi_master *master)
/* 如果使用 spi_bitbang_start 注册 spi_master 的话就要使用 spi_bitbang_stop 来注销掉 spi_master */
```

### 1.2 SPI 设备驱动

- Linux 使用 spi_driver 结构体来表示 spi 设备驱动；我们编写时需要完成相应设备驱动的编写
- include/linux/spi/spi.h

```c
struct spi_driver {
    const struct spi_device_id *id_table;
    int (*probe)(struct spi_device *spi);
    int (*remove)(struct spi_device *spi);
    void (*shutdown)(struct spi_device *spi);
    struct device_driver driver;
};
```

spi_driver 初始化后需要向 Linux 内核注册

```c
/* 注册 */
int spi_register_driver(struct spi_driver *sdrv)
sdrv：要注册的 spi_driver
返回值：0，注册成功；负值，注册失败

/* 销毁 */
void spi_unregister_driver(struct spi_driver *sdrv)
```

------

**spi_driver 注册demo**

```c
/* spi_driver 注册demo */
/* probe 函数 */
static int xxx_probe(struct spi_device *spi)
{
    /* 具体函数内容 */
    return 0;
}


/* remove 函数 */
static int xxx_remove(struct spi_device *spi)
{
    /* 具体函数内容 */
    return 0;
}

/* 传统匹配方式 ID 列表 */
static const struct spi_device_id xxx_id[] = {
    {"xxx", 0},
    {}
};

/* 设备树匹配列表 */
static const struct of_device_id xxx_of_match[] = {
    { .compatible = "xxx" },
    { /* Sentinel */ }
};

/* SPI 驱动结构体 */
static struct spi_driver xxx_driver = {
    .probe = xxx_probe,
    .remove = xxx_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "xxx",
        .of_match_table = xxx_of_match,
    },
    .id_table = xxx_id,
}

/* 驱动入口函数 */
static int __init xxx_init(void)
{
    return spi_register_driver(&xxx_driver);
}

/* 驱动出口函数 */
static void __exit xxx_exit(void)
{
    spi_unregister_driver(&xxx_driver);
}

module_init(xxx_init);
module_exit(xxx_exit);
```

### 1.3 SPI 设备和驱动匹配过程

- 设备和驱动的匹配过程由 SPI 总线来完成的；spi_bus_type（drivers/spi/spi.c）

```c
struct bus_type spi_bus_type = {
    .name = "spi",
    .dev_groups = spi_dev_groups,
    .match = spi_match_device,		// 匹配设备和驱动的函数
    .uevent = spi_uevent,
};
```

.match 用于匹配设备和驱动

![1685440036152](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685440036152.png)

![1685440051287](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685440051287.png)

## 2. I.MX6U SPI 主机驱动分析

- SPI 主机驱动一般由 SOC 厂商编写好了

```
/* imx6ull.dtsi */
 ecspi3: ecspi@02010000 {
	#address-cells = <1>;
	#size-cells = <0>;
	compatible = "fsl,imx6ul-ecspi", "fsl,imx51-ecspi";		// 匹配spi主机驱动
	reg = <0x02010000 0x4000>;
	interrupts = <GIC_SPI 33 IRQ_TYPE_LEVEL_HIGH>;
	clocks = <&clks IMX6UL_CLK_ECSPI3>,
			<&clks IMX6UL_CLK_ECSPI3>;
	clock-names = "ipg", "per";
	dmas = <&sdma 7 7 1>, <&sdma 8 7 2>;
	dma-names = "rx", "tx";
	status = "disabled";
};
```

对应 I.MX6U 对应的 ECSPI 主机驱动（drivers/spi/spi-imx.c）

![1685440766176](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685440766176.png)

![1685440781909](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685440781909.png)

spi_imx_probe 函数会从设备树中读取相应的节点属性值，申请并初始化 spi_master，最后调用 spi_bitbang_start 函数(spi_bitbang_start 会调用 spi_register_master 函数)向 Linux 内核注册 spi_master

对于 I.MX6U 来讲，SPI 主机的最终数据收发函数为 spi_imx_transfer，调用如下：

```
spi_imx_transfer
	-> spi_imx_pio_transfer
		-> spi_imx_push
			-> spi_imx->tx
```

spi_imx 是一个 spi_imx_data 类型的指针变量，tx 和 rx 分别为 SPI 数据发送和接收函数；主机驱动会维护一个 spi_imx_data 类型的变量 spi_imx，并且使用 spi_imx_setupxfer 函数来设置 spi_imx 的 tx 和 rx 函数 

```
/* 根据要发送的数据数据位宽的不同 */
spi_imx_buf_tx_u8
spi_imx_buf_tx_u16
spi_imx_buf_tx_u32

/* 接收函数 */
spi_imx_buf_rx_u8
spi_imx_buf_rx_u16
spi_imx_buf_rx_u32
```

![1688974380180](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1688974380180.png)

## 3. SPI 设备驱动编写流程

### 3.1 SPI 设备信息描述

- IO 的 pincrtl 子节点创建与修改

  所 SPI 所用到的 IO 口进行端口复用

- SPI 设备设备节点的创建与修改

  ```c
  &ecspi1 {
  	fsl,spi-num-chipselects = <1>;		// 表示只有一个设备
  	cs-gpios = <&gpio4 9 0>;			// 片选信号 是 GIO4_IO09
  	pinctrl-names = "default";			// SPI 设备所使用的 IO 名字
  	pinctrl-0 = <&pinctrl_ecspi1>;		// 所使用的IO对应的 pinctrl
  	status = "okay";
      
  	flash: m25p80@0 {	// 0表示接到了ESPI的0通道上
  		#address-cells = <1>;
  		#size-cells = <1>;
  		compatible = "st,m25p32";	// 匹配驱动
  		spi-max-frequency = <20000000>;	// 设置 SPI 控制器的最高频率
  		reg = <0>;		// 接到了 ESPI 的第0通道
  	};
  };
  ```

### 3.2 SPI 设备数据收发处理流程

向 Linux 内核注册成功 spi_driver 以后就可以使用 SPI 核心层提供的 API 来对设备进行读写操作了

1. 申请并初始化 spi_transfer，设置 tx_buf、rx_buf 、len 成员变量
2. 初始化 spi_message
3. 将设置好的 spi_transfer 添加到 spi_message 队列中
4. 使用同步传输或者异步传输，将 spi_message 发出去

spi_transfer 结构体；用来描述 SPI 传输信息

![1685446992272](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685446992272.png)

spi_transfer 需要组织成 spi_message 结构体

![1685447055442](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685447055442.png)

![1685447068748](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685447068748.png)

- complete 回调函数，完成异步传输后自动执行

在使用 spi_message 之前需要对其进行初始化；初始化完后在将 spi_transfer 添加到 spi_message 队列中；spi_message 准备完全后，即可进行数据传输，数据传输分为 同步传输 和 异步传输

```c
void spi_message_init(struct spi_message *m)		// 初始化 spi_transfer

/* 将 spi_transfer 添加到 spi_message 队列中 */
void spi_message_add_tail(struct spi_transfer *t, struct spi_message *m)

/* 同步传输 会阻塞的等待SPI数据传输完成 */
int spi_sync(struct spi_device *spi, struct spi_message *message)
/* 异步传输 需要设置 spi_message 中的 complete 回调函数 spi 异步传输完成函数自动被调用 */
int spi_async(struct spi_device *spi, struct spi_message *message)
```

**同步传输方式实现 SPI 传输**

1. 申请并初始化 spi_transfer，设置 spi_transfer 的 tx_buf 成员变量， tx_buf 为要发送的数据 
2. 使用 spi_message_init 函数初始化 spi_message 
3. 使用 spi_message_add_tail函数将前面设置好的 spi_transfer添加到 spi_message 队列中 
4. 使用 spi_sync 函数完成 SPI 数据同步传输 

```c
/* SPI 多字节发送 */
static int spi_send(struct spi_device *spi, u8 *buf, int len)
{
    int ret;
    struct spi_message m;
    struct spi_transfer t = {
        .tx_buf = buf,
        .len = len,
    };
    spi_message_init(&m); /* 初始化 spi_message */
    spi_message_add_tail(t, &m);/* 将 spi_transfer 添加到 spi_message 队列 */
    ret = spi_sync(spi, &m); /* 同步传输 */
    return ret;
}

/* SPI 多字节接收 */
static int spi_receive(struct spi_device *spi, u8 *buf, int len)
{
    int ret;
    struct spi_message m;
    struct spi_transfer t = {
        .rx_buf = buf,
        .len = len,
    };
    spi_message_init(&m); /* 初始化 spi_message */
    spi_message_add_tail(t, &m);/* 将 spi_transfer 添加到 spi_message 队列 */
    ret = spi_sync(spi, &m); /* 同步传输 */
    return ret;
}
```

## 4. SPI 驱动实验

### 4.1 原理图

![1685433921282](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685433921282.png)

### 4.2 修改设备树

#### 1. 添加ICM20608所用IO

没有将 UART2_TX_DATA这个 IO 复用为 ECSPI3 的 SS0 信号，而复用为普通 IO，因为我们自己控制片选信号

```c
pinctrl_ecspi3: icm20608 {
	fsl,pins = <
		MX6UL_PAD_UART2_TX_DATA__GPIO1_IO20 0x10b0 		/* CS */
		MX6UL_PAD_UART2_RX_DATA__ECSPI3_SCLK 0x10b1 	/* SCLK */
		MX6UL_PAD_UART2_RTS_B__ECSPI3_MISO 0x10b1 		/* MISO */
		MX6UL_PAD_UART2_CTS_B__ECSPI3_MOSI 0x10b1 		/* MOSI */
	>;
};
```

#### 2. 在 ecspi3 节点追加 icm20608 子节点

```c
/* imx6ull-alientek-emmc.dts */
&ecspi3 {
	fsl,spi-num-chipselects = <1>;
	cs-gpios = <&gpio1 20 GPIO_ACTIVE_LOW>;
	pinctrl-names = "default";
	pinctrl-0 = <&pinctrl_ecspi3>;
	status = "okay";

	spidev: icm20608@0 {
		compatible = "alientek,icm20608";
		spi-max-frequency = <8000000>;
		reg = <0>;
	};
};
```

### 4.3  ICM20608 驱动（设备驱动）

```c
/* bsp_icm20608.h */
/* ICM20608 寄存器头文件 */
#ifndef _BSP_ICM20608_H
#define _BSP_ICM20608_H

#include "imx6ul.h"
#include "bsp_gpio.h"

/* SPI 片选信号 */
#define ICM20608_CSN(n) (n ? gpio_pinwrite(GPIO1, 20, 1) :
						gpio_pinwrite(GPIO1, 20, 0))

#define ICM20608G_ID 0XAF 	/* ID 值 */
#define ICM20608D_ID 0XAE 	/* ID 值 */

/* ICM20608 寄存器
*复位后所有寄存器地址都为 0，除了
*Register 107(0X6B) Power Management 1 = 0x40
*Register 117(0X75) WHO_AM_I = 0xAF 或者 0xAE
*/
/* 陀螺仪和加速度自测(出产时设置，用于与用户的自检输出值比较） */
#define ICM20_SELF_TEST_X_GYRO 0x00
#define ICM20_SELF_TEST_Y_GYRO 0x01
#define ICM20_SELF_TEST_Z_GYRO 0x02
#define ICM20_SELF_TEST_X_ACCEL 0x0D
#define ICM20_SELF_TEST_Y_ACCEL 0x0E
#define ICM20_SELF_TEST_Z_ACCEL 0x0F
/***********省略掉其他宏定义*************/
/* 加速度静态偏移 */
#define ICM20_XA_OFFSET_H 0x77
#define ICM20_XA_OFFSET_L 0x78
#define ICM20_YA_OFFSET_H 0x7A
#define ICM20_YA_OFFSET_L 0x7B
#define ICM20_ZA_OFFSET_H 0x7D
#define ICM20_ZA_OFFSET_L 0x7E
                            
#endif
```

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
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include "icm20608reg.h"
/***************************************************************
描述	   	: ICM20608 SPI驱动程序
***************************************************************/
#define ICM20608_CNT	1
#define ICM20608_NAME	"icm20608"

struct icm20608_dev {
	dev_t devid;				/* 设备号 	 */
	struct cdev cdev;			/* cdev 	*/
	struct class *class;		/* 类 		*/
	struct device *device;		/* 设备 	 */
	struct device_node	*nd; 	/* 设备节点 */
	int major;					/* 主设备号 */
	void *private_data;			/* 私有数据 		*/
	signed int gyro_x_adc;		/* 陀螺仪X轴原始值 	 */
	signed int gyro_y_adc;		/* 陀螺仪Y轴原始值		*/
	signed int gyro_z_adc;		/* 陀螺仪Z轴原始值 		*/
	signed int accel_x_adc;		/* 加速度计X轴原始值 	*/
	signed int accel_y_adc;		/* 加速度计Y轴原始值	*/
	signed int accel_z_adc;		/* 加速度计Z轴原始值 	*/
	signed int temp_adc;		/* 温度原始值 			*/
};

static struct icm20608_dev icm20608dev;

/*
 * @description	: 从icm20608读取多个寄存器数据
 * @param - dev:  icm20608设备
 * @param - reg:  要读取的寄存器首地址
 * @param - val:  读取到的数据
 * @param - len:  要读取的数据长度
 * @return 		: 操作结果
 */
static int icm20608_read_regs(struct icm20608_dev *dev, u8 reg, void *buf, int len)
{

	int ret = -1;
	unsigned char txdata[1];
	unsigned char * rxdata;
	struct spi_message m;
	struct spi_transfer *t;
	struct spi_device *spi = (struct spi_device *)dev->private_data;
    
	t = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);	/* 申请内存 */
	if(!t) {
		return -ENOMEM;
	}

	rxdata = kzalloc(sizeof(char) * len, GFP_KERNEL);	/* 申请内存 */
	if(!rxdata) {
		goto out1;
	}

	/* 一共发送len+1个字节的数据，第一个字节为
	寄存器首地址，一共要读取len个字节长度的数据，*/
	txdata[0] = reg | 0x80;		/* 写数据的时候首寄存器地址bit8要置1 */			
	t->tx_buf = txdata;			/* 要发送的数据 */
    t->rx_buf = rxdata;			/* 要读取的数据 */
	t->len = len+1;				/* t->len=发送的长度+读取的长度 */
	spi_message_init(&m);		/* 初始化spi_message */
	spi_message_add_tail(t, &m);/* 将spi_transfer添加到spi_message队列 */
	ret = spi_sync(spi, &m);	/* 同步发送 */
	if(ret) {
		goto out2;
	}
	
    memcpy(buf , rxdata+1, len);  /* 只需要读取的数据 */

out2:
	kfree(rxdata);					/* 释放内存 */
out1:	
	kfree(t);						/* 释放内存 */
	
	return ret;
}

/*
 * @description	: 向icm20608多个寄存器写入数据
 * @param - dev:  icm20608设备
 * @param - reg:  要写入的寄存器首地址
 * @param - val:  要写入的数据缓冲区
 * @param - len:  要写入的数据长度
 * @return 	  :   操作结果
 */
static s32 icm20608_write_regs(struct icm20608_dev *dev, u8 reg, u8 *buf, u8 len)
{
	int ret = -1;
	unsigned char *txdata;
	struct spi_message m;
	struct spi_transfer *t;
	struct spi_device *spi = (struct spi_device *)dev->private_data;
	
	t = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);	/* 申请内存 */
	if(!t) {
		return -ENOMEM;
	}
	
	txdata = kzalloc(sizeof(char)+len, GFP_KERNEL);
	if(!txdata) {
		goto out1;
	}
	
	/* 一共发送len+1个字节的数据，第一个字节为
	寄存器首地址，len为要写入的寄存器的集合，*/
	*txdata = reg & ~0x80;	/* 写数据的时候首寄存器地址bit8要清零 */
    memcpy(txdata+1, buf, len);	/* 把len个寄存器拷贝到txdata里，等待发送 */
	t->tx_buf = txdata;			/* 要发送的数据 */
	t->len = len+1;				/* t->len=发送的长度+读取的长度 */
	spi_message_init(&m);		/* 初始化spi_message */
	spi_message_add_tail(t, &m);/* 将spi_transfer添加到spi_message队列 */
	ret = spi_sync(spi, &m);	/* 同步发送 */
    if(ret) {
        goto out2;
    }
	
out2:
	kfree(txdata);				/* 释放内存 */
out1:
	kfree(t);					/* 释放内存 */
	return ret;
}

/*
 * @description	: 读取icm20608指定寄存器值，读取一个寄存器
 * @param - dev:  icm20608设备
 * @param - reg:  要读取的寄存器
 * @return 	  :   读取到的寄存器值
 */
static unsigned char icm20608_read_onereg(struct icm20608_dev *dev, u8 reg)
{
	u8 data = 0;
	icm20608_read_regs(dev, reg, &data, 1);
	return data;
}

/*
 * @description	: 向icm20608指定寄存器写入指定的值，写一个寄存器
 * @param - dev:  icm20608设备
 * @param - reg:  要写的寄存器
 * @param - data: 要写入的值
 * @return   :    无
 */	

static void icm20608_write_onereg(struct icm20608_dev *dev, u8 reg, u8 value)
{
	u8 buf = value;
	icm20608_write_regs(dev, reg, &buf, 1);
}

/*
 * @description	: 读取ICM20608的数据，读取原始数据，包括三轴陀螺仪、
 * 				: 三轴加速度计和内部温度。
 * @param - dev	: ICM20608设备
 * @return 		: 无。
 */
void icm20608_readdata(struct icm20608_dev *dev)
{
	unsigned char data[14] = { 0 };
	icm20608_read_regs(dev, ICM20_ACCEL_XOUT_H, data, 14);

	dev->accel_x_adc = (signed short)((data[0] << 8) | data[1]); 
	dev->accel_y_adc = (signed short)((data[2] << 8) | data[3]); 
	dev->accel_z_adc = (signed short)((data[4] << 8) | data[5]); 
	dev->temp_adc    = (signed short)((data[6] << 8) | data[7]); 
	dev->gyro_x_adc  = (signed short)((data[8] << 8) | data[9]); 
	dev->gyro_y_adc  = (signed short)((data[10] << 8) | data[11]);
	dev->gyro_z_adc  = (signed short)((data[12] << 8) | data[13]);
}

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做pr似有ate_data的成员变量
 * 					  一般在open的时候将private_data似有向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int icm20608_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &icm20608dev; /* 设置私有数据 */
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
static ssize_t icm20608_read(struct file *filp, char __user *buf, size_t cnt, loff_t *off)
{
	signed int data[7];
	long err = 0;
	struct icm20608_dev *dev = (struct icm20608_dev *)filp->private_data;

	icm20608_readdata(dev);
	data[0] = dev->gyro_x_adc;
	data[1] = dev->gyro_y_adc;
	data[2] = dev->gyro_z_adc;
	data[3] = dev->accel_x_adc;
	data[4] = dev->accel_y_adc;
	data[5] = dev->accel_z_adc;
	data[6] = dev->temp_adc;
	err = copy_to_user(buf, data, sizeof(data));
	return 0;
}

/*
 * @description		: 关闭/释放设备
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
 */
static int icm20608_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* icm20608操作函数 */
static const struct file_operations icm20608_ops = {
	.owner = THIS_MODULE,
	.open = icm20608_open,
	.read = icm20608_read,
	.release = icm20608_release,
};

/*
 * ICM20608内部寄存器初始化函数 
 * @param  	: 无
 * @return 	: 无
 */
void icm20608_reginit(void)
{
	u8 value = 0;
	
	icm20608_write_onereg(&icm20608dev, ICM20_PWR_MGMT_1, 0x80);
	mdelay(50);
	icm20608_write_onereg(&icm20608dev, ICM20_PWR_MGMT_1, 0x01);
	mdelay(50);

	value = icm20608_read_onereg(&icm20608dev, ICM20_WHO_AM_I);
	printk("ICM20608 ID = %#X\r\n", value);	

	icm20608_write_onereg(&icm20608dev, ICM20_SMPLRT_DIV, 0x00); 	/* 输出速率是内部采样率					*/
	icm20608_write_onereg(&icm20608dev, ICM20_GYRO_CONFIG, 0x18); 	/* 陀螺仪±2000dps量程 				*/
	icm20608_write_onereg(&icm20608dev, ICM20_ACCEL_CONFIG, 0x18); 	/* 加速度计±16G量程 					*/
	icm20608_write_onereg(&icm20608dev, ICM20_CONFIG, 0x04); 		/* 陀螺仪低通滤波BW=20Hz 				*/
	icm20608_write_onereg(&icm20608dev, ICM20_ACCEL_CONFIG2, 0x04); /* 加速度计低通滤波BW=21.2Hz 			*/
	icm20608_write_onereg(&icm20608dev, ICM20_PWR_MGMT_2, 0x00); 	/* 打开加速度计和陀螺仪所有轴 				*/
	icm20608_write_onereg(&icm20608dev, ICM20_LP_MODE_CFG, 0x00); 	/* 关闭低功耗 						*/
	icm20608_write_onereg(&icm20608dev, ICM20_FIFO_EN, 0x00);		/* 关闭FIFO						*/
}

 /*
  * @description     : spi驱动的probe函数，当驱动与
  *                    设备匹配以后此函数就会执行
  * @param - client  : i2c设备
  * @param - id      : i2c设备ID
  * 
  */	
static int icm20608_probe(struct spi_device *spi)
{
	/* 1、构建设备号 */
	if (icm20608dev.major) {
		icm20608dev.devid = MKDEV(icm20608dev.major, 0);
		register_chrdev_region(icm20608dev.devid, ICM20608_CNT, ICM20608_NAME);
	} else {
		alloc_chrdev_region(&icm20608dev.devid, 0, ICM20608_CNT, ICM20608_NAME);
		icm20608dev.major = MAJOR(icm20608dev.devid);
	}

	/* 2、注册设备 */
	cdev_init(&icm20608dev.cdev, &icm20608_ops);
	cdev_add(&icm20608dev.cdev, icm20608dev.devid, ICM20608_CNT);

	/* 3、创建类 */
	icm20608dev.class = class_create(THIS_MODULE, ICM20608_NAME);
	if (IS_ERR(icm20608dev.class)) {
		return PTR_ERR(icm20608dev.class);
	}

	/* 4、创建设备 */
	icm20608dev.device = device_create(icm20608dev.class, NULL, icm20608dev.devid, NULL, ICM20608_NAME);
	if (IS_ERR(icm20608dev.device)) {
		return PTR_ERR(icm20608dev.device);
	}

	/*初始化spi_device */
	spi->mode = SPI_MODE_0;	/*MODE0，CPOL=0，CPHA=0*/
	spi_setup(spi);
	icm20608dev.private_data = spi; /* 设置私有数据 */

	/* 初始化ICM20608内部寄存器 */
	icm20608_reginit();		
	return 0;
}

/*
 * @description     : i2c驱动的remove函数，移除i2c驱动的时候此函数会执行
 * @param - client 	: i2c设备
 * @return          : 0，成功;其他负值,失败
 */
static int icm20608_remove(struct spi_device *spi)
{
	/* 删除设备 */
	cdev_del(&icm20608dev.cdev);
	unregister_chrdev_region(icm20608dev.devid, ICM20608_CNT);

	/* 注销掉类和设备 */
	device_destroy(icm20608dev.class, icm20608dev.devid);
	class_destroy(icm20608dev.class);
	return 0;
}

/* 传统匹配方式ID列表 */
static const struct spi_device_id icm20608_id[] = {
	{"alientek,icm20608", 0},  
	{}
};

/* 设备树匹配列表 */
static const struct of_device_id icm20608_of_match[] = {
	{ .compatible = "alientek,icm20608" },
	{ /* Sentinel */ }
};

/* SPI驱动结构体 */	
static struct spi_driver icm20608_driver = {
	.probe = icm20608_probe,
	.remove = icm20608_remove,
	.driver = {
			.owner = THIS_MODULE,
		   	.name = "icm20608",
		   	.of_match_table = icm20608_of_match, 
		   },
	.id_table = icm20608_id,
};
		   
/*
 * @description	: 驱动入口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init icm20608_init(void)
{
	return spi_register_driver(&icm20608_driver);
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit icm20608_exit(void)
{
	spi_unregister_driver(&icm20608_driver);
}

module_init(icm20608_init);
module_exit(icm20608_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("zuozhongkai");
```

### 4.4 测试app

```c
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
/***************************************************************
描述	   	: icm20608设备测试APP。
***************************************************************/

/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
	int fd;
	char *filename;
	signed int databuf[7];
	unsigned char data[14];
	signed int gyro_x_adc, gyro_y_adc, gyro_z_adc;
	signed int accel_x_adc, accel_y_adc, accel_z_adc;
	signed int temp_adc;

	float gyro_x_act, gyro_y_act, gyro_z_act;
	float accel_x_act, accel_y_act, accel_z_act;
	float temp_act;

	int ret = 0;

	if (argc != 2) {
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = argv[1];
	fd = open(filename, O_RDWR);
	if(fd < 0) {
		printf("can't open file %s\r\n", filename);
		return -1;
	}

	while (1) {
		ret = read(fd, databuf, sizeof(databuf));
		if(ret == 0) { 			/* 数据读取成功 */
			gyro_x_adc = databuf[0];
			gyro_y_adc = databuf[1];
			gyro_z_adc = databuf[2];
			accel_x_adc = databuf[3];
			accel_y_adc = databuf[4];
			accel_z_adc = databuf[5];
			temp_adc = databuf[6];

			/* 计算实际值 */
			gyro_x_act = (float)(gyro_x_adc)  / 16.4;
			gyro_y_act = (float)(gyro_y_adc)  / 16.4;
			gyro_z_act = (float)(gyro_z_adc)  / 16.4;
			accel_x_act = (float)(accel_x_adc) / 2048;
			accel_y_act = (float)(accel_y_adc) / 2048;
			accel_z_act = (float)(accel_z_adc) / 2048;
			temp_act = ((float)(temp_adc) - 25 ) / 326.8 + 25;


			printf("\r\n原始值:\r\n");
			printf("gx = %d, gy = %d, gz = %d\r\n", gyro_x_adc, gyro_y_adc, gyro_z_adc);
			printf("ax = %d, ay = %d, az = %d\r\n", accel_x_adc, accel_y_adc, accel_z_adc);
			printf("temp = %d\r\n", temp_adc);
			printf("实际值:");
			printf("act gx = %.2f°/S, act gy = %.2f°/S, act gz = %.2f°/S\r\n", gyro_x_act, gyro_y_act, gyro_z_act);
			printf("act ax = %.2fg, act ay = %.2fg, act az = %.2fg\r\n", accel_x_act, accel_y_act, accel_z_act);
			printf("act temp = %.2f°C\r\n", temp_act);
		}
		usleep(100000); /*100ms */
	}
	close(fd);	/* 关闭文件 */	
	return 0;
}
```

