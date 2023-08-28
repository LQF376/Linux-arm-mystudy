# Linux 下 UART 驱动框架

## 1. Linux UART 驱动框架

串口驱动没有主机端和设备端之分，只有一个串口驱动

- 驱动框架通常由半导体厂商进行写好，我们只需要在设备树中添加所要使用的串口节点信息
- 驱动和设备匹配成功后，相应的串口就会被驱动起来，生成 /dev/ttymxcX(X=0...n)

### 1.1 uart_driver 注册与销毁

- 每个串口驱动都需要定义一个 uart_driver 结构体，用来表示 UART 驱动（include/linux/serial_core.h）

![1685538285443](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685538285443.png)

```c
int uart_register_driver(struct uart_driver *drv)	// 向内核注册 uart_driver
返回值： 0，成功；负值，失败

void uart_unregister_driver(struct uart_driver *drv)	// 注销 uart_driver
```

### 1.2 uart_port 添加与移除

- uart_port 结构体表示一个具体的 port；每个 UART 都有一个 uart_port（include/linux/serial_core.h ）

![1685538698074](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685538698074.png)

uart_port 和 uart_driver 相结合起来

```c
/* 将 uart_port 和 uart_driver 相结合起来 */
int uart_add_one_port(struct uart_driver *drv, struct uart_port *uport)
drv：此 port 对应的 uart_driver
uport：要添加到 uart_driver 中的 port
返回值：0，成功；负值，失败

/* 将 uart_port 从相应的 uart_driver 中移除 */
int uart_remove_one_port(struct uart_driver *drv, struct uart_port *uport)
返回值：0，成功；负值，失败
```

### 1.3 uart_ops 实现

- uart_ops 包含了针对 UART 具体的驱动函数，Linux 系统收发数据最终调用的都是 ops 中的函数
- UART驱动编写人员需要实现 uart_ops，其是最底层的 UART 驱动接口，和 UART 寄存器打交道
- 文档 Documentation/serial/driver 
- include/linux/serial_core.h

![1685539247177](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685539247177.png)

![1685539270715](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685539270715.png)

![1685539324559](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685539324559.png)

## 2. I.MX6U UART 驱动分析

### 2.1 UART 的 platform 框架

```c
/* imx6ull.dtsi */
uart3: serial@021ec000 {
	compatible = "fsl,imx6ul-uart",
				"fsl,imx6q-uart", "fsl,imx21-uart";
	reg = <0x021ec000 0x4000>;
	interrupts = <GIC_SPI 28 IRQ_TYPE_LEVEL_HIGH>;
	clocks = <&clks IMX6UL_CLK_UART3_IPG>,
	<&clks IMX6UL_CLK_UART3_SERIAL>;
	clock-names = "ipg", "per";
	dmas = <&sdma 29 4 0>, <&sdma 30 4 0>;
	dma-names = "rx", "tx";
	status = "disabled";
};
```

找到对应驱动文件 drivers/tty/serial/imx.c

![1685540638837](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685540638837.png)

![1685540658241](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685540658241.png)

![1685540692687](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685540692687.png)

- init 入口处会先注册一个 uart_driver；imx_reg 是一个 uart_driver 类型的结构变量

  ![1685540858359](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685540858359.png)

### 2.2 uart_port 初始化与添加

- 当 UART 设备和驱动匹配成功后，serial_imx_probe 函数会执行
- 会初始化 uart_port，将其添加到对应的 uart_driver 中
- imx_port 是 I.MX 系列 SOC 定义的一个结构体，包含了 uart_port 成员变量

![1685541036363](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685541036363.png)

serial_imx_probe 函数会初始化 uart_port，将其添加到对应的 uart_driver 中

![1688978710380](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1688978710380.png)

![1688978727850](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1688978727850.png)

![1688978739588](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1688978739588.png)

**imx_pops 结构体变量**

uart_port 中最重要的就是 uart_ops 类型结构体；保存 I.MX6ULL 串口最底层的操作函数

![1685541292436](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685541292436.png)

![1685541302757](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685541302757.png)

## 3. RS232 驱动实验

I.MX6U-ALPHA 开发板上 RS232、RS485 和 GPS 这三个接口都连接到了 UART3 上 

### 3.1 电路原理图

![1688979130005](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1688979130005.png)

### 3.2 RS232 驱动编写

I.MX6U 的UART 驱动 NXP 已经编写好了；只需在设备树中添加UART3对应的设备节点信息即可

- UART IO 节点创建

  用到了哪些 IO 端口

  ```c
  pinctrl_uart3: uart3grp {
  	fsl, pins = <
  		MX6UL_PAD_UART3_TX_DATA__UART3_DCE_TX 0X1b0b1
  		MX6UL_PAD_UART3_RX_DATA__UART3_DCE_RX 0X1b0b1
  	>;
  };
  ```

- 添加 uart3 节点

  ```c
  &uart3 {
  	pinctrl-names = "default";
  	pinctrl-0 = <&pinctrl_uart3>;
  	status = "okay";
  };
  ```

完成设备树后重新编译，启动后会自动生成一个”/dev/ttymxc2“ 的设备文件

## 4. 移植 minicom

- Linux环境下的串口调试助手

### 4.1 移植 ncurses

需要先移植 ncurses 才能移植 minicom

```shell
tar -vxzf ncurses-6.0.tar.gz	# 解压 ncurses
./configure --prefix=/home/zuozhongkai/linux/IMX6ULL/tool/ncurses --host=arm-linux-gnueabihf --target=arm-linux-gnueabihf --with-shared --without-profile --disable-stripping --without-progs --with-manpages --without-tests		# 配置

--prefix 用于指定编译结果的保存目录
--host 用于指定编译器前缀
--target 用于指定目标，这里也设置为“arm-linux-gnueabihf”

make	# 编译
make install #安装
```

include、 lib 和 share 这三个目录中存放的文件分别拷贝到开发板根文件系统中的/usr/include、 /usr/lib 和/usr/share 这三个目录中 

```
sudo cp lib/* /home/zuozhongkai/linux/nfs/rootfs/usr/lib/ -rfa
sudo cp share/* /home/zuozhongkai/linux/nfs/rootfs/usr/share/ -rfa
sudo cp include/* /home/zuozhongkai/linux/nfs/rootfs/usr/include/ -rfa
```

在开发板根目录的/etc/profile(没有的话自己创建一个)文件中添加:

```shell
#!/bin/sh
LD_LIBRARY_PATH=/lib:/usr/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH

export TERM=vt100
export TERMINFO=/usr/share/terminfo
```

### 4.2 移植 minicom

```shell
tar -vxzf minicom-2.7.1.tar.gz
cd minicom-2.7.1/      //进入 minicom 源码目录

./configure CC=arm-linux-gnueabihf-gcc --prefix=/home/zuozhongkai/linux/IMX6ULL/tool/minicom --host=arm-linux-gnueabihf CPPFLAGS=-I/home/zuozhongkai/linux/IMX6ULL/tool/ncurses/include LDFLAGS=-L/home/zuozhongkai/linux/IMX6ULL/tool/ncurses/lib -enable-cfg-dir=/etc/minicom   //配置
------------------------------------
CPPFLAGS 指定 ncurses 的头文件路径
LDFLAGS 指定 ncurses 的库路径

make		// 编译
make install	// 安装
```

```shell
sudo cp bin/* /home/zuozhongkai/linux/nfs/rootfs/usr/bin/
```

新建 /etc/passwd 文件

```
root:x:0:0:root:/root:/bin/sh
```

重启开发板即可

```
minicom -v		// 查看版本
minicom -s		// 进入配置界面
```

![1688980194368](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1688980194368.png)

