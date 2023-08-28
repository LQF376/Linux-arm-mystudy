# SPI Linux

## 1. SPI 简介

- SPI（Serial Peripheral Interface）串行外设接口的缩写；一种高速的、全双工、同步的串行通信总线；
- SPI 采用主从方式工作，一般有一个主设备和一个或多个从设备；SPI需要至少4根线，分别时MISO（主设备输入从设备输出）、MOSI（主设备输出从设备输入）、SCLK（时钟）、CS（片选）
- SPI 使用引脚较少且布线方便，所以越来越多芯片集成了这种通信协议

![1683187987525](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1683187987525.png)

- CS/SS：片选信号线。与I2C不同的是，SPI不需要主机发送设备地址，通过拉低对应从机的片选信号即可选中
- SCK：串行时钟线，为 SPI 提供时钟
- MOSI/SDO：主出从入信号线。用于主机向从机发送数据
- MISO/SDI：主入从出信号线。从机向主机发送数据

**SPI 寻址方式**

- 当主设备要和某个从设备进行通信时，主设备需要向对应从设备的片选线上发送使能信号（高电平或者低电平，根据从机而定）表示选中该从设备

**通信过程**

- SPI 总线在进行数据传送时，先传送高位，后传送低位；数据线为高电平表示逻辑‘1’，低电平表示逻辑‘0’；
- 一个字节传送完成后无需应答即可开始下一个字节的传送；
- SPI 总线采用同步方式工作，时钟线在上升沿或下降沿时发送器向数据线上发送数据，在紧接着的下降沿或上升沿时接收器从数据线上读取数据，完成一位数据传送，八个时钟周期即可完成一个字节数据的传送

**极性和相位**

SPI 总线有四种不同的工作模式，取决于极性（CPOL）和相位（CPHA）这两个因素

- CPOL 表示 SCLK 空闲时的状态
  - CPOL = 0；空闲时 SCLK 为低电平
  - CPOL = 1；空闲时 SCLK 为高电平
- CPHA 表示采样时刻
  - CPHA = 0；每个周期的第一个时钟沿采样
  - CPHA = 1；每个周期的第二个时钟沿采样

![1685429743003](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685429743003.png)

CS 片选信号先拉低，选中要通信的从设备，然后通过 MOSI 和 MISO 这两根数据线进行收发数据 

![1685429767358](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685429767358.png)

## 2. I.MX6U ECSPI 简介（SOC 自带 SPI 外设）

- 每个 ECSPI 支持四个片选信号
- 发送和接收都有一个 32x64 的 FIFO 

### 2.1 ECSPIx_CONREG(x=1~4)  控制寄存器

![1685429998632](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685429998632.png)

- BURST_LENGTH(bit31:24)：突发长度，设置 SPI 的突发传输数据长度，一般设置突发长度为一个字节，也就是 8bit， BURST_LENGTH=7 
- CHANNEL_SELECT(bit19:18)：SPI 通道选择，一个 ECSPI 有四个硬件片选信号（0~3），每个片选信号是一个硬件通道
- DRCTL(bit17:16)：用于设置 SPI_RDY 信号， 为 0 的话不关心 SPI_RDY 信号；为 1 的话 SPI_RDY 信号为边沿触发；为 2 的话 SPI_DRY 是电平触发 
- PRE_DIVIDER(bit15:12)：SPI 预分频， ECSPI 时钟频率使用两步来完成分频，此位设置的是第一步，可设置 0~15，分别对应 1~16 分频 
- POST_DIVIDER(bit11:8)：SPI 分频值， ECSPI 时钟频率的第二步分频设置，分频值为 2^POST_DIVIDER 
- CHANNEL_MODE(bit7:4)：CHANNEL_MODE[3:0]分别对应 SPI 通道 3~0， 为 0 的话就是设置为从模式，如果为 1 的话就是主模式
- SMC(bit3)：开始模式控制，此位只能在主模式下起作用，为 0 的话通过 XCH 位来开启 SPI 突发访问，为 1 的话只要向 TXFIFO 写入数据就开启 SPI 突发访问 
- XCH(bit2)：此位只在主模式下起作用，当 SMC 为 0 的话此位用来控制 SPI 突发访问的开启
- HT(bit1)：HT 模式使能位， I.MX6ULL 不支持
- EN(bit0)：SPI 使能位，为 0 的话关闭 SPI，为 1 的话使能 SPI 

### 2.2 ECSPIx_CONFIGREG 配置寄存器

![1685430356333](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685430356333.png)

- HT_LENGTH(bit28:24)：HT 模式下的消息长度设置， I.MX6ULL 不支持 
- SCLK_CTL(bit23:20)：设置 SCLK 信号线空闲状态电平， SCLK_CTL[3:0]分别对应通道 3~0，为 0 的话 SCLK 空闲状态为低电平，为 1 的话 SCLK 空闲状态为高电平
- DATA_CTL(bit19:16)：设置 DATA 信号线空闲状态电平， DATA_CTL[3:0]分别对应通道 3~0，为 0 的话 DATA 空闲状态为高电平，为 1 的话 DATA 空闲状态为低电平
- SS_POL(bit15:12)：设置 SPI 片选信号极性设置， SS_POL[3:0]分别对应通道 3~0，为 0 的话片选信号低电平有效，为 1 的话片选信号高电平有效 
- SCLK_POL(bit7:4)：SPI 时钟信号极性设置，也就是 CPOL， SCLK_POL[3:0]分别对应通道 3~0，为 0 的话 SCLK 高电平有效(空闲的时候为低电平)，为 1 的话 SCLK 低电平有效(空闲的时候为高电平) 
- SCLK_PHA(bit3:0)：SPI时钟相位设置，也就是CPHA，SCLK_PHA[3:0]分别对应通道3~0，为 0 的话串行时钟的第一个跳变沿(上升沿或下降沿)采集数据，为 1 的话串行时钟的第二个跳变沿(上升沿或下降沿)采集数据

### 2.3 ECSPIx_PERIODREG 采样周期寄存器 

![1685431372697](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685431372697.png)

- CSD_CTL(bit21:16)：片选信号延时控制位，用于设置片选信号和第一个 SPI 时钟信号之间的时间间隔，范围为 0~63 

- CSRC(bit15)：SPI 时钟源选择，为 0 的话选择 SPI CLK 为 SPI 的时钟源，为 1 的话选择 32.768KHz 的晶振为 SPI 时钟源

  ![1685431437877](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685431437877.png)

- SAMPLE_PERIO:采样周期寄存器，可设置为 0~0X7FFF 分别对应 0~32767 个周期 

### 2.4 ECSPIx_STATREG 状态寄存器

![1685431496583](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685431496583.png)

- TC(bit7)：传输完成标志位，为 0 表示正在传输，为 1 表示传输完成 
- RO(bit6)：RXFIFO 溢出标志位，为 0 表示 RXFIFO 无溢出，为 1 表示 RXFIFO 溢出 
- RF(bit5)：RXFIFO 空标志位，为 0 表示 RXFIFO 不为空，为 1 表示 RXFIFO 为空 
- RDR(bit4)：RXFIFO 数据请求标志位，此位为 0 表示 RXFIFO 里面的数据不大于 RX_THRESHOLD，此位为 1 的话表示 RXFIFO 里面的数据大于 RX_THRESHOLD 
- RR(bit3)：RXFIFO 就绪标志位，为 0 的话 RXFIFO 没有数据，为 1 的话表示 RXFIFO 中至少有一个字的数据
-  TF(bit2)：TXFIFO 满标志位，为 0 的话表示 TXFIFO 不为满，为 1 的话表示 TXFIFO 为满 
- TDR(bit1) ：TXFIFO 数据请求标志位，为 0 表示 TXFIFO 中的数据大于 TX_THRESHOLD，为 1 表示 TXFIFO 中的数据不大于 TX_THRESHOLD 
- TE(bit0)：TXFIFO 空标志位，为 0 表示 TXFIFO 中至少有一个字的数据，为 1 表示 TXFIFO 为空 

### 2.5 ECSPIx_TXDATA 和 ECSPIx_RXDATA （32 位）

发送数据就向寄存器 ECSPIx_TXDATA 写入数据；读取及存取 ECSPIx_RXDATA 里面的数据

## 3. ICM-20608 简介 

- 6 轴 MEMS 传感器（包括 3 轴加速度和 3 轴陀螺仪 ）
- 陀螺仪和加速度计都是 16 位的 ADC；并且支持 I2C 和 SPI 两种协议 

![1685433711001](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685433711001.png)

- 如果使用 IIC 接口的话 ICM-20608 的 AD0 引脚决定 I2C 设备从地址的最后一位，如果 AD0 为 0 的话 ICM-20608 从设备地址是 0X68，如果 AD0 为 1 的话 ICM-20608 从设备地址为 0X69 
- ICM-20608 也是通过读写寄存器来配置和读取传感器数据；第一个字节包含要读写的寄存器地址，寄存器地址最高位是读写标志位，如果是读的话寄存器地址最高位要为 1，如果是写的话寄存器地址最高位要为 0，剩下的 7 位才是实际的寄存器地址，寄存器地址后面跟着的就是读写的数据 

![1685433816436](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685433816436.png)

![1685433859142](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685433859142.png)

![1685433874383](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685433874383.png)

## 4. 硬件原理图

![1685433921282](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685433921282.png)

## 5. 裸机程序

```c
/* bsp_spi.h */
#ifndef _BSP_SPI_H
#define _BSP_SPI_H

#include "imx6ul.h"

/* 函数声明 */
void spi_init(ECSPI_Type *base);
unsigned char spich0_readwrite_byte(ECSPI_Type *base, unsigned char txdata);

#endif
```

```c
/* bsp_spi.c */
#include "bsp_spi.h"
#include "bsp_gpio.h"
#include "stdio.h"

void spi_init(ECSPI_Type *base)
{
    /* 配置 CONREG 寄存器
	* bit0 : 1 使能 ECSPI
	* bit3 : 1 当向 TXFIFO 写入数据以后立即开启 SPI 突发。
	* bit[7:4]： 0001 SPI 通道 0 主模式，根据实际情况选择，开发板上的
	* ICM-20608 接在 SS0 上，所以设置通道 0 为主模式
	* bit[19:18]:00 选中通道 0(其实不需要，因为片选信号我们我们自己控制)
	* bit[31:20]:0x7 突发长度为 8 个 bit。
	*/
    base->CONREG = 0; 	/* 先清除控制寄存器 */
    base->CONREG |= (1 << 0) | (1 << 3) | (1 << 4) | (7 << 20);
    
    /*
	* ECSPI 通道 0 设置,即设置 CONFIGREG 寄存器
	* bit0: 0 通道 0 PHA 为 0
	* bit4: 0 通道 0 SCLK 高电平有效
	* bit8: 0 通道 0 片选信号 当 SMC 为 1 的时候此位无效
	* bit12: 0 通道 0 POL 为 0
	* bit16: 0 通道 0 数据线空闲时高电平
	* bit20: 0 通道 0 时钟线空闲时低电平
	*/
    base->CONFIGREG = 0; 	/* 设置通道寄存器 */
    
    /*
	* ECSPI 通道 0 设置，设置采样周期
	* bit[14:0] : 0X2000 采样等待周期，比如当 SPI 时钟为 10MHz 的时候
	* 0X2000 就等于 1/10000 * 0X2000 = 0.8192ms，也就是
	* 连续读取数据的时候每次之间间隔 0.8ms
	* bit15 : 0 采样时钟源为 SPI CLK
	* bit[21:16]: 0 片选延时，可设置为 0~63
	*/
    base->PERIODREG = 0X2000; 	/* 设置采样周期寄存器 */
    
    /*
	* ECSPI 的 SPI 时钟配置， SPI 的时钟源来源于 pll3_sw_clk/8=480/8=60MHz
	* SPI CLK = (SourceCLK / PER_DIVIDER) / (2^POST_DIVEDER)
	* 比如我们现在要设置 SPI 时钟为 6MHz，那么设置如下：
	* PER_DIVIDER = 0X9。
	* POST_DIVIDER = 0X0。
	* SPI CLK = 60000000/(0X9 + 1) = 60000000=6MHz
	*/
    base->CONREG &= ~((0XF << 12) | (0XF << 8)); 	/* 清除以前的设置 */
    base->CONREG |= (0X9 << 12); 	/* 设置 SPI CLK = 6MHz */
}

/* SPI 通道 0 发送/接收一个字节的数据 */
unsigned char spich0_readwrite_byte(ECSPI_Type *base, unsigned char txdata)
{
    uint32_t spirxdata = 0;
    uint32_t spitxdata = txdata;
    
    /* 选择通道 0 */
    base->CONREG &= ~(3 << 18);
    base->CONREG |= (0 << 18);
    
    while((base->STATREG & (1 << 0)) == 0){} 	/* 等待发送 FIFO 为空 */
    base->TXDATA = spitxdata;
    
    while((base->STATREG & (1 << 3)) == 0){} /* 等待接收 FIFO 有数据 */
    spirxdata = base->RXDATA;
    return spirxdata;
}
```

```c
/* bsp_icm20608.h */
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
#define ICM20_ZA_OFFSET_H 0x7D
#define ICM20_ZA_OFFSET_L 0x7E
                            
/*
* ICM20608 结构体
*/
struct icm20608_dev_struc
{
    signed int gyro_x_adc; 		/* 陀螺仪 X 轴原始值 */
    signed int gyro_y_adc; 		/* 陀螺仪 Y 轴原始值 */
    signed int gyro_z_adc; 		/* 陀螺仪 Z 轴原始值 */
    signed int accel_x_adc; 	/* 加速度计 X 轴原始值 */
    signed int accel_y_adc; 	/* 加速度计 Y 轴原始值 */
    signed int accel_z_adc; 	/* 加速度计 Z 轴原始值 */
    signed int temp_adc; 		/* 温度原始值 */
    
    /* 下面是计算得到的实际值，扩大 100 倍 */
    signed int gyro_x_act; 		/* 陀螺仪 X 轴实际值 */
    signed int gyro_y_act; 		/* 陀螺仪 Y 轴实际值 */
    signed int gyro_z_act; 		/* 陀螺仪 Z 轴实际值 */
    signed int accel_x_act; 	/* 加速度计 X 轴实际值 */
    signed int accel_y_act; 	/* 加速度计 Y 轴实际值 */
    signed int accel_z_act; 	/* 加速度计 Z 轴实际值 */
    signed int temp_act; 		/* 温度实际值 */
}；

struct icm20608_dev_struc icm20608_dev; 	/* icm20608 设备 */

/* 函数声明 */
unsigned char icm20608_init(void);
void icm20608_write_reg(unsigned char reg, unsigned char value);
unsigned char icm20608_read_reg(unsigned char reg);
void icm20608_read_len(unsigned char reg, unsigned char *buf,
					unsigned char len);
void icm20608_getdata(void);

#endif
```

```c
/* bsp_icm20608.c */
#include "bsp_icm20608.h"
#include "bsp_delay.h"
#include "bsp_spi.h"
#include "stdio.h"

struct icm20608_dev_struc icm20608_dev; 	/* icm20608 设备 */

/* 初始化 ICM20608 */
unsigned char icm20608_init(void)
{
    unsigned char regvalue;
    gpio_pin_config_t cs_config;
    
    /* 1、 ESPI3 IO 初始化
    * ECSPI3_SCLK -> UART2_RXD
	* ECSPI3_MISO -> UART2_RTS
	* ECSPI3_MOSI -> UART2_CTS
	*/
    IOMUXC_SetPinMux(IOMUXC_UART2_RX_DATA_ECSPI3_SCLK, 0);
    IOMUXC_SetPinMux(IOMUXC_UART2_CTS_B_ECSPI3_MOSI, 0);
    IOMUXC_SetPinMux(IOMUXC_UART2_RTS_B_ECSPI3_MISO, 0);
    IOMUXC_SetPinConfig(IOMUXC_UART2_RX_DATA_ECSPI3_SCLK, 0x10B1);
    IOMUXC_SetPinConfig(IOMUXC_UART2_CTS_B_ECSPI3_MOSI, 0x10B1);
    IOMUXC_SetPinConfig(IOMUXC_UART2_RTS_B_ECSPI3_MISO, 0x10B1);
    
    /* 初始化片选引脚 */
    IOMUXC_SetPinMux(IOMUXC_UART2_TX_DATA_GPIO1_IO20, 0);
    IOMUXC_SetPinConfig(IOMUXC_UART2_TX_DATA_GPIO1_IO20, 0X10B0);
    cs_config.direction = kGPIO_DigitalOutput;
    cs_config.outputLogic = 0;
    gpio_init(GPIO1, 20, &cs_config);
    
    /* 2、初始化 SPI */
    spi_init(ECSPI3);
    
    icm20608_write_reg(ICM20_PWR_MGMT_1, 0x80); 	/* 复位 */
    delayms(50);
    icm20608_write_reg(ICM20_PWR_MGMT_1, 0x01); 	/* 关闭睡眠 */
    delayms(50);
    
    regvalue = icm20608_read_reg(ICM20_WHO_AM_I);
    printf("icm20608 id = %#X\r\n", regvalue);
    if(regvalue != ICM20608G_ID && regvalue != ICM20608D_ID)
        return 1;
    
    icm20608_write_reg(ICM20_SMPLRT_DIV, 0x00); 	/* 输出速率设置 */
    icm20608_write_reg(ICM20_GYRO_CONFIG, 0x18); 	/* 陀螺仪±2000dps */
    icm20608_write_reg(ICM20_ACCEL_CONFIG, 0x18); /* 加速度计±16G */
    icm20608_write_reg(ICM20_CONFIG, 0x04); /* 陀螺 BW=20Hz */
    icm20608_write_reg(ICM20_ACCEL_CONFIG2, 0x04);
    icm20608_write_reg(ICM20_PWR_MGMT_2, 0x00); /* 打开所有轴 */
    icm20608_write_reg(ICM20_LP_MODE_CFG, 0x00); /* 关闭低功耗 */
    icm20608_write_reg(ICM20_FIFO_EN, 0x00); /* 关闭 FIFO */
    return 0;
}

/* 写 ICM20608 指定寄存器 */
void icm20608_write_reg(unsigned char reg, unsigned char value)
{
    /* ICM20608 在使用 SPI 接口的时候寄存器地址只有低 7 位有效,
	* 寄存器地址最高位是读/写标志位，读的时候要为 1，写的时候要为 0。
	*/
    reg &= ~0X80;
    
    ICM20608_CSN(0); 						/* 使能 SPI 传输 */
    spich0_readwrite_byte(ECSPI3, reg); 	/* 发送寄存器地址 */
    spich0_readwrite_byte(ECSPI3, value); 	/* 发送要写入的值 */
    ICM20608_CSN(1); 						/* 禁止 SPI 传输 */   
}

/*  读取 ICM20608 寄存器值 */
unsigned char icm20608_read_reg(unsigned char reg)
{
    unsigned char reg_val;
    
    /* ICM20608 在使用 SPI 接口的时候寄存器地址只有低 7 位有效,
	* 寄存器地址最高位是读/写标志位，读的时候要为 1，写的时候要为 0。
	*/
    reg |= 0x80;
    
    ICM20608_CSN(0); 								/* 使能 SPI 传输 */
    spich0_readwrite_byte(ECSPI3, reg); 			/* 发送寄存器地址 */
    reg_val = spich0_readwrite_byte(ECSPI3, 0XFF);	/* 读取寄存器的值*/
    ICM20608_CSN(1); 								/* 禁止 SPI 传输 */
    return(reg_val); 								/* 返回读取到的寄存器值 */
}

/* 读取 ICM20608 连续多个寄存器 */
void icm20608_read_len(unsigned char reg, unsigned char *buf, unsigned char len)
{
    unsigned char i;
    
    /* ICM20608 在使用 SPI 接口的时候寄存器地址，只有低 7 位有效,
	* 寄存器地址最高位是读/写标志位读的时候要为 1，写的时候要为 0。
	*/
    reg |= 0x80;
    
    ICM20608_CSN(0); 				/* 使能 SPI 传输 */
    spich0_readwrite_byte(ECSPI3, reg);	/* 发送寄存器地址 */
    for(i = 0; i < len; i++)		/* 顺序读取寄存器的值 */
    {
        buf[i] = spich0_readwrite_byte(ECSPI3, 0XFF);
    }
    ICM20608_CSN(1); 		/* 禁止 SPI 传输 */
}

/* 获取陀螺仪的分辨率(分辨率根据选择的量程不同而不同) */
float icm20608_gyro_scaleget(void)
{
    unsigned char data;
    float gyroscale;
    
    data = (icm20608_read_reg(ICM20_GYRO_CONFIG) >> 3) & 0X3;
    switch(data)
    {
        case 0:
            gyroscale = 131;
            break;
        case 1:
            gyroscale = 65.5;
            break;
        case 2:
            gyroscale = 32.8;
            break;
        case 3:
            gyroscale = 16.4;
            break;
    }
    return gyroscale;
}

/* 获取加速度计的分辨率 */
unsigned short icm20608_accel_scaleget(void)
{
    unsigned char data;
    unsigned short accelscale;
    
    data = (icm20608_read_reg(ICM20_ACCEL_CONFIG) >> 3) & 0X3;
    switch(data)
    {
        case 0:
            accelscale = 16384;
            break;
        case 1:
            accelscale = 8192;
            break;
        case 2:
            accelscale = 4096;
            break;
        case 3:
            accelscale = 2048;
            break;
    }
    return accelscale;
}

/* 读取 ICM20608 的加速度、陀螺仪和温度原始值 */
void icm20608_getdata(void)
{
    float gyroscale;
    unsigned short accescale;
    unsigned char data[14];
    
    icm20608_read_len(ICM20_ACCEL_XOUT_H, data, 14);
    
    gyroscale = icm20608_gyro_scaleget();
    accescale = icm20608_accel_scaleget();
    
    icm20608_dev.accel_x_adc = (signed short)((data[0] << 8) | data[1]);
    icm20608_dev.accel_y_adc = (signed short)((data[2] << 8) | data[3]);
    icm20608_dev.accel_z_adc = (signed short)((data[4] << 8) | data[5]);
    icm20608_dev.temp_adc = (signed short)((data[6] << 8) | data[7]);
    icm20608_dev.gyro_x_adc = (signed short)((data[8] << 8) | data[9]);
    icm20608_dev.gyro_y_adc = (signed short)((data[10] << 8) | data[11]);
    icm20608_dev.gyro_z_adc = (signed short)((data[12] << 8) | data[13]);
    
    /* 计算实际值 */
    icm20608_dev.gyro_x_act = ((float)(icm20608_dev.gyro_x_adc) / gyroscale) * 100;
    icm20608_dev.gyro_y_act = ((float)(icm20608_dev.gyro_y_adc) / gyroscale) * 100;
    icm20608_dev.gyro_z_act = ((float)(icm20608_dev.gyro_z_adc) / gyroscale) * 100;
    icm20608_dev.accel_x_act = ((float)(icm20608_dev.accel_x_adc) / accescale) * 100;
    icm20608_dev.accel_y_act = ((float)(icm20608_dev.accel_y_adc) / accescale) * 100;
    icm20608_dev.accel_z_act = ((float)(icm20608_dev.accel_z_adc) / accescale) * 100;
    icm20608_dev.temp_act = (((float)(icm20608_dev.temp_adc) - 25) / 326.8 + 25) * 100;
}
```

## 6. Linux 下 SPI 驱动框架

分为 主机控制器驱动（SOC的 SPI 控制器接口） 和 设备驱动

### 6.1 主机控制器驱动

- Linux 内核使用 spi_master 结构体表示 SPI 主机驱动；include/linux/spi/spi.h
- SPI 主机驱动的核心就是申请 spi_master，然后初始化 spi_master，最后向 Linux 内核注册 spi_master

![1685438508928](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685438508928.png)

![1685438522749](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685438522749.png)

```
/* 申请 spi_master */
struct spi_master *spi_alloc_master(struct device *dev, unsigned size)
dev：设备，一般是 platform_device 中的 dev 成员变量
size：私有数据的大小； 可以通过 spi_master_get_devdata 函数获取到这些私有数据
返回值：申请到的 spi_master 结构体

/* 释放 spi_master */
void spi_master_put(struct spi_master *master)
master：要释放的 spi_master
```

spi_master 初始化后需要将其注册到 Linux 内核

```c
int spi_register_master(struct spi_master *master)
返回值：0，成功； 负值，失败
/* I.MX6U 的 SPI 主机驱动会采用 spi_bitbang_start 这个 API 函数来完成 spi_master 的注册 */

void spi_unregister_master(struct spi_master *master)
/* 如果使用 spi_bitbang_start 注册 spi_master 的话就要使用 spi_bitbang_stop 来注销掉 spi_master */
```

### 6.2 SPI 设备驱动

- Linux 使用 spi_driver 结构体来表示 spi 设备驱动；include/linux/spi/spi.h

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

### 6.3 设备和驱动匹配过程

- 设备和驱动的匹配过程由 SPI 总线来完成的；spi_bus_type（drivers/spi/spi.c）

```c
struct bus_type spi_bus_type = {
    .name = "spi",
    .dev_groups = spi_dev_groups,
    .match = spi_match_device,
    .uevent = spi_uevent,
};
```

.match 用于匹配设备和驱动

![1685440036152](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685440036152.png)

![1685440051287](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685440051287.png)

## 7. I.MX6U SPI 驱动

### 7.1 I.MX6U SPI 主机驱动分析

SPI 主机驱动一般由 SOC 厂商编写，SPI 控制器 设备树如下

```
ecspi3: ecspi@02010000 {
	#address-cells = <1>;
	#size-cells = <0>;
	compatible = "fsl,imx6ul-ecspi", "fsl,imx51-ecspi";		// 用于找到驱动
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

![1685446323768](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685446323768.png)

### 7.2 SPI 设备驱动

**1. IO的pinctrl子节点创建与修改**

要检查有没有被其他口所占用

**2. SPI 设备节点的创建与修改**

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

### 7.3 数据收发过程

1. 申请并初始化 spi_transfer，设置 tx_buf、rx_buf 、len 成员变量
2. 初始化 spi_message
3. 将设置好的 spi_transfer 添加到 spi_message 队列中
4. 使用同步传输或者异步传输，将 spi_message 发出去

spi_transfer 结构体；用来描述 SPI 传输信息

![1685446992272](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685446992272.png)

spi_transfer 需要组织成 spi_message 结构体

![1685447055442](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685447055442.png)

![1685447068748](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685447068748.png)

在使用 spi_message 之前需要对其进行初始化；初始化完后在将 spi_transfer 添加到 spi_message 队列中；spi_message 准备完全后，即可进行数据传输，数据传输分为 同步传输 和 异步传输

```
void spi_message_init(struct spi_message *m)		// 初始化 spi_transfer

/* 将 spi_transfer 添加到 spi_message 队列中 */
void spi_message_add_tail(struct spi_transfer *t, struct spi_message *m)

/* 同步传输 会阻塞的等待SPI数据传输完成 */
int spi_sync(struct spi_device *spi, struct spi_message *message)
/* 异步传输 需要设置 spi_message 中的 complete 回调函数 spi 异步传输完成函数自动被调用 */
int spi_async(struct spi_device *spi, struct spi_message *message)
```

## 8. SPI 实验

### 8.1 硬件原理图

![1685433921282](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685433921282.png)

### 8.2 修改设备树

**1. 添加ICM20608所用IO**

没有将 UART2_TX_DATA这个 IO 复用为 ECSPI3 的 SS0 信号，而复用为普通 IO，因为我们自己控制片选信号

```CQL
pinctrl_ecspi3: icm20608 {
	fsl,pins = <
		MX6UL_PAD_UART2_TX_DATA__GPIO1_IO20 0x10b0 		/* CS */
		MX6UL_PAD_UART2_RX_DATA__ECSPI3_SCLK 0x10b1 	/* SCLK */
		MX6UL_PAD_UART2_RTS_B__ECSPI3_MISO 0x10b1 		/* MISO */
		MX6UL_PAD_UART2_CTS_B__ECSPI3_MOSI 0x10b1 		/* MOSI */
	>;
};
```

**2. 在 ecspi3 节点追加 icm20608 子节点**

```
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

### 8.3 ICM20608 驱动

```c
/* bsp_icm20608.h */
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
/* icm20608.c */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
......
#include <asm/io.h>
#include "icm20608reg.h"

#define ICM20608_CNT 1
#define ICM20608_NAME "icm20608"

struct icm20608_dev {
    dev_t devid; 			/* 设备号 */
    struct cdev cdev; 		/* cdev */
    struct class *class; 	/* 类 */
    struct device *device; 	/* 设备 */
    struct device_node *nd; 	/* 设备节点 */
    int major; 				/* 主设备号 */
    void *private_data; 	/* 私有数据 */
    int cs_gpio;			/* 片选所使用的 GPIO 编号*/
    signed int gyro_x_adc; 	/* 陀螺仪 X 轴原始值 */
    signed int gyro_y_adc; 	/* 陀螺仪 Y 轴原始值 */
    signed int gyro_z_adc; 	/* 陀螺仪 Z 轴原始值 */
    signed int accel_x_adc; 	/* 加速度计 X 轴原始值 */
    signed int accel_y_adc; 	/* 加速度计 Y 轴原始值 */
    signed int accel_z_adc; 	/* 加速度计 Z 轴原始值 */
    signed int temp_adc; 		/* 温度原始值 */
};

static struct icm20608_dev icm20608dev;

/* probe 函数 */
static int icm20608_probe(struct spi_device *spi)
{
    /* 1、构建设备号 */
    if (icm20608dev.major) 
    {
        icm20608dev.devid = MKDEV(icm20608dev.major, 0);
        register_chrdev_region(icm20608dev.devid, ICM20608_CNT, ICM20608_NAME);
    }
    else
    {
        alloc_chrdev_region(&icm20608dev.devid, 0, ICM20608_CNT, ICM20608_NAME);
        icm20608dev.major = MAJOR(icm20608dev.devid);
    }
    
    /* 2、注册设备 */
    cdev_init(&icm20608dev.cdev, &icm20608_ops);
    cdev_add(&icm20608dev.cdev, icm20608dev.devid, ICM20608_CNT);
    
    /* 3、创建类 */
    icm20608dev.class = class_create(THIS_MODULE, ICM20608_NAME);
    if (IS_ERR(icm20608dev.class))
    {
        return PTR_ERR(icm20608dev.class);
    }
    
    /* 4、创建设备 */
    icm20608dev.device = device_create(icm20608dev.class, NULL, icm20608dev.devid, NULL, ICM20608_NAME);
    if (IS_ERR(icm20608dev.device)) 
    {
        return PTR_ERR(icm20608dev.device);
    }
    
    /*初始化 spi_device */
    spi->mode = SPI_MODE_0; /*MODE0， CPOL=0， CPHA=0*/
    spi_setup(spi);
    icm20608dev.private_data = spi; 	/* 设置私有数据 */
    
    /* 初始化 ICM20608 内部寄存器 */
    icm20608_reginit();
    return 0;
}

/* remove 函数 */
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

/* 传统匹配方式 ID 列表 */
static const struct spi_device_id icm20608_id[] = {
	{"alientek,icm20608", 0},
	{}
};

/* 设备树匹配列表 */
static const struct of_device_id icm20608_of_match[] = {
	{ .compatible = "alientek,icm20608" },
	{ /* Sentinel */ }
};

/* SPI 驱动结构体 */
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

/* 驱动入口函数 */
static int __init icm20608_init(void)
{
    return spi_register_driver(&icm20608_driver);
}

/* 驱动出口函数 */
static void __exit icm20608_exit(void)
{
    spi_unregister_driver(&icm20608_driver);
}

module_init(icm20608_init);
module_exit(icm20608_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("zuozhongkai");
```

icm20608 寄存器读写与初始化

```c
/* 从 icm20608 读取多个寄存器数据 */
static int icm20608_read_regs(struct icm20608_dev *dev, u8 reg, void *buf, int len)
{
    int ret = -1;
    unsigned char txdata[1];
    unsigned char * rxdata;
    struct spi_message m;
    struct spi_transfer *t;
    struct spi_device *spi = (struct spi_device *)dev->private_data;
    
    t = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
    if(!t) 
    {
        return -ENOMEM;
    }
    
    rxdata = kzalloc(sizeof(char) * len, GFP_KERNEL); /* 申请内存 */
    if(!rxdata) 
    {
        goto out1;
    }
    
    /* 一共发送 len+1 个字节的数据，第一个字节为
	寄存器首地址，一共要读取 len 个字节长度的数据， */
    txdata[0] = reg | 0x80; 	/* 写数据的时候首寄存器地址 bit8 要置 1 */
    t->tx_buf = txdata; 		/* 要发送的数据 */
    t->rx_buf = rxdata; 		/* 要读取的数据 */
    t->len = len+1; 			/* t->len=发送的长度+读取的长度 */
    spi_message_init(&m); 		/* 初始化 spi_message */
    spi_message_add_tail(t, &m);
    ret = spi_sync(spi, &m); 	/* 同步发送 */
    if(ret) 
    {
		goto out2;
    }
    
    memcpy(buf , rxdata+1, len); /* 只需要读取的数据 */
    
out2:
	kfree(rxdata); 	/* 释放内存 */
out1:
	kfree(t); 		/* 释放内存 */
    
    return ret;
}

/* 向 icm20608 多个寄存器写入数据 */
static s32 icm20608_write_regs(struct icm20608_dev *dev, u8 reg, u8 *buf, u8 len)
{
    int ret = -1;
    unsigned char *txdata;
    struct spi_message m;
    struct spi_transfer *t;
    struct spi_device *spi = (struct spi_device *)dev->private_data;
    
    t = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
    if(!t)
    {
        return -ENOMEM;
    }
    
    txdata = kzalloc(sizeof(char)+len, GFP_KERNEL);
    if(!txdata)
    {
        goto out1;
    }
    
    /* 一共发送 len+1 个字节的数据，第一个字节为
	寄存器首地址， len 为要写入的寄存器的集合， */
    *txdata = reg & ~0x80; 			/* 写数据的时候首寄存器地址 bit8 要清零 */
    memcpy(txdata+1, buf, len); 	/* 把 len 个寄存器拷贝到 txdata 里 */
    t->tx_buf = txdata;				 /* 要发送的数据 */
    t->len = len+1; 				/* t->len=发送的长度+读取的长度 */
    spi_message_init(&m); 			/* 初始化 spi_message */
    spi_message_add_tail(t, &m);
    ret = spi_sync(spi, &m);		 /* 同步发送 */
    if(ret) 
    {
        goto out2;
    }
    
out2:
	kfree(txdata); 		/* 释放内存 */
out1:
	kfree(t); 		/* 释放内存 */
	
    return ret;
}

/* 读取 icm20608 指定寄存器值，读取一个寄存器 */
static unsigned char icm20608_read_onereg(struct icm20608_dev *dev, u8 reg)
{
    u8 data = 0;
    icm20608_read_regs(dev, reg, &data, 1);
    return data;
}

/* 向 icm20608 指定寄存器写入指定的值，写一个寄存器 */
static void icm20608_write_onereg(struct icm20608_dev *dev, u8 reg, u8 value)
{
    u8 buf = value;
    icm20608_write_regs(dev, reg, &buf, 1);
}

/* 读取 ICM20608 的数据，读取原始数据，包括三轴陀螺仪 */
void icm20608_readdata(struct icm20608_dev *dev)
{
    unsigned char data[14] = { 0 };
    icm20608_read_regs(dev, ICM20_ACCEL_XOUT_H, data, 14);
    
    dev->accel_x_adc = (signed short)((data[0] << 8) | data[1]);
    dev->accel_y_adc = (signed short)((data[2] << 8) | data[3]);
    dev->accel_z_adc = (signed short)((data[4] << 8) | data[5]);
    dev->temp_adc = (signed short)((data[6] << 8) | data[7]);
    dev->gyro_x_adc = (signed short)((data[8] << 8) | data[9]);
    dev->gyro_y_adc = (signed short)((data[10] << 8) | data[11]);
    dev->gyro_z_adc = (signed short)((data[12] << 8) | data[13]);
}

/*  ICM20608 内部寄存器初始化函数 */
void icm20608_reginit(void)
{
    u8 value = 0;
    
    icm20608_write_onereg(&icm20608dev, ICM20_PWR_MGMT_1, 0x80);
    mdelay(50);
    icm20608_write_onereg(&icm20608dev, ICM20_PWR_MGMT_1, 0x01);
    mdelay(50);
    
    value = icm20608_read_onereg(&icm20608dev, ICM20_WHO_AM_I);
    printk("ICM20608 ID = %#X\r\n", value);
    
    icm20608_write_onereg(&icm20608dev, ICM20_SMPLRT_DIV, 0x00);
    icm20608_write_onereg(&icm20608dev, ICM20_GYRO_CONFIG, 0x18);
    icm20608_write_onereg(&icm20608dev, ICM20_ACCEL_CONFIG, 0x18);
    icm20608_write_onereg(&icm20608dev, ICM20_CONFIG, 0x04);
    icm20608_write_onereg(&icm20608dev, ICM20_ACCEL_CONFIG2, 0x04);
    icm20608_write_onereg(&icm20608dev, ICM20_PWR_MGMT_2, 0x00);
    icm20608_write_onereg(&icm20608dev, ICM20_LP_MODE_CFG, 0x00);
    icm20608_write_onereg(&icm20608dev, ICM20_FIFO_EN, 0x00);
}
```

```c
/* 字符设备驱动框架 */
/* 打开设备 */
static int icm20608_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &icm20608dev; 	/* 设置私有数据 */
    return 0;
}

/* 从设备读取数据 */
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

/* 关闭/释放设备 */
static int icm20608_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/* icm20608 操作函数 */
static const struct file_operations icm20608_ops = {
    .owner = THIS_MODULE,
    .open = icm20608_open,
    .read = icm20608_read,
    .release = icm20608_release,
};
```

**测试APP**

```c
/* icm20608App.c */
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
    
    if (argc != 2) 
    {
        printf("Error Usage!\r\n");
        return -1;
    }
    
    filename = argv[1];
    fd = open(filename, O_RDWR);
    if(fd < 0)
    {
        printf("can't open file %s\r\n", filename);
        return -1;
    }
    
    while (1)
    {
        ret = read(fd, databuf, sizeof(databuf));
        if(ret == 0)
        {
            /* 数据读取成功 */
            gyro_x_adc = databuf[0];
            gyro_y_adc = databuf[1];
            gyro_z_adc = databuf[2];
            accel_x_adc = databuf[3];
            accel_y_adc = databuf[4];
            accel_z_adc = databuf[5];
            temp_adc = databuf[6];
            
            /* 计算实际值 */
            gyro_x_act = (float)(gyro_x_adc) / 16.4;
            gyro_y_act = (float)(gyro_y_adc) / 16.4;
            gyro_z_act = (float)(gyro_z_adc) / 16.4;
            accel_x_act = (float)(accel_x_adc) / 2048;
            accel_y_act = (float)(accel_y_adc) / 2048;
            accel_z_act = (float)(accel_z_adc) / 2048;
            temp_act = ((float)(temp_adc) - 25 ) / 326.8 + 25;
            
            printf("\r\n 原始值:\r\n");
            printf("gx = %d, gy = %d, gz = %d\r\n", gyro_x_adc,
					gyro_y_adc, gyro_z_adc);
            printf("ax = %d, ay = %d, az = %d\r\n", accel_x_adc,
					accel_y_adc, accel_z_adc);
            printf("temp = %d\r\n", temp_adc);
            printf("实际值:");
            printf("act gx = %.2f°/S, act gy = %.2f°/S, act gz = %.2f°/S\r\n", gyro_x_act, 						gyro_y_act, gyro_z_act);
            printf("act ax = %.2fg, act ay = %.2fg, act az = %.2fg\r\n", accel_x_act, 							accel_y_act, accel_z_act);
            printf("act temp = %.2f°C\r\n", temp_act);
        }
        usleep(100000); 	/*100ms */
    }
    close(fd); 		/* 关闭文件 */
    return 0;
}
```

