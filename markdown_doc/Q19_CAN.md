# CAN Linux

## 1. CAN 协议简介

- CAN（Controller Area Network），控制局域网络
- 目前是汽车网络的标准协议
- 各个单元通过 CAN 总线连接在一起，同一个 CAN 网络中的所有单元的通信速度必须一致，不同网络之间通信速度可以不同

### 1.0 CAN 特点

- 多主控制（总线空闲时，所有单元都可以发送消息）

  两个以上的单元同时发送消息，根据标识符（Identifier，ID）决定优先级（表示访问总线的消息优先级）

- 系统柔软性

  总线没有地址的概念，因此在总线上增加单元时，其他单元软硬件不用发生改变

- 通信速度快，距离远

  1Mbps/S

- 具有错误检测、错误通知和错误恢复功能

  所有单元都可以检测错误；检测出错误的单元会通知其他所有单元；正在发消息的单元一旦检测出错误，会强制结束；强制结束的单元会反复发送此消息直到成功

- 故障封闭功能

  当总线上发生持续数据错误时，可将引起此故障的单元从总线上隔离出去

- 连接节点多

  可连接的单元总数理论上无限制；降低通信速度，可连接的单元数增加

### 1.1 电气属性

- 显性电平表示逻辑“0”，此时 CAN_H 电平比 CAN_L 高，分别为 3.5V 和 1.5V，电位差为 2V
- 隐形电平表示逻辑“1”，此时 CAN_H 和 CAN_L 电压都为 2.5V 左右，电位差为 0V 

![1685602940423](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685602940423.png)

- 总线空闲状态的时候一直处于隐性
- 连接时候 CAN_H 接 CAN_H，CAN_L 接 CAN_L；两端各接120欧的端接电阻，匹配总线阻抗

### 1.2 CAN 协议

CAN 协议提供了 5 种帧格式来传输数据：数据帧、遥控帧、错误帧、过载帧和帧间隔 

- 其中 数据帧 和 遥控帧 有标准格式和扩展格式：标准格式11位标识符（ID）；拓展格式29位标识符（ID）

![1685603540300](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685603540300.png)

#### 1. 数据帧

帧起始->仲裁断->控制段->数据段->CRC段->ACK段->帧结束

**帧起始：表示数据帧开始的段**

- 由一个位的显性电平 0 来表示帧起始

**仲裁段：表示帧的优先级**

![1685603973683](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685603973683.png)

- 标准格式的 ID 为 11 位，发送顺序是从 ID10 到 ID0，最高 7 位 ID10~ID4 不能全为隐性(1) 
- 扩展格式的 ID 为 29 位，基本 ID 从 ID28 到 ID18，扩展 ID 由 ID17 到 ID0 （先高位后低位）

**控制段：表示数据段的字节数及保留位**

![1685604059375](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685604059375.png)

- r1 和 r0 为保留位，保留位必须以显性电平发送 
- DLC 为数据长度，高位在前， DLC 段有效值范围为 0~8 字节

**数据段：帧的有效数据（包含0~8字节的数据）**

![1685604204723](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685604204723.png)

**CRC段：检查帧传输错误（做校验）**

![1685604257907](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685604257907.png)

- CRC 段由 15 位的 CRC 值与 1 位的 CRC 界定符组成
- CRC 值的计算范围包括：帧起始、仲裁段、控制段、数据段；接收发送双方使用相同算法进行计算，对比

**ACK 段：用来确认接收是否正常**

![1685604426817](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685604426817.png)

- ACK 段由 ACK 槽(ACK Slot)和 ACK 界定符两部分组成
- 发送单元的 ACK，发送 2 个隐性位，而接收到正确消息的单元在 ACK 槽（ACK Slot）发送显性位 

**帧结束：表是一帧结束**

![1685604576126](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685604576126.png)

- 帧结束段由 7 位隐性位构成 

#### 2. 遥控帧

- 接收单元发送单元请求数据的时候用到遥控帧
- 帧起始；仲裁段；控制段；CRC段；ACK段；帧结束（相较数据帧没有数据段）
- 遥控帧的 RTR 位为隐性的，数据帧的 RTR 位为显性，因此可以通过 RTR 位来区分遥控帧和没有数据的数据帧 

![1685604700812](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685604700812.png)

#### 3. 错误帧

- 当接收或发送消息出错的时候使用错误帧来通知
- 错误帧由错误标志和错误界定符两部分组成
- 错误标志有主动错误标志和被动错误标志两种；主动错误标志是 6 个显性位，被动错误标志是 6 个隐性位，错误界定符由 8 个隐性位组成 

![1685606050671](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685606050671.png)

#### 4. 过载帧

- 接收单元尚未完成接收准备的话就会发送过载帧
- 过载帧由过载标志和过载界定符构成
- 过载标志由 6 个显性位组成；过载界定符由 8 个隐性位组成 

![1685606179754](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685606179754.png)

#### 5. 帧间隔

- 帧间隔用于分隔数据帧和遥控帧；过载帧和错误帧前不能插入帧间隔
- 间隔由 3 个隐性位构成，总线空闲为隐性电平，长度没有限制；延迟发送由 8 个隐性位构成，处于被动错误状态的单元发送一个消息后的帧间隔中才会有延迟发送 

### 1.3 CAN 速率（一个位分为几段）

- 一个位分为4段：同步段（SS）、传播时间段（PTS）、相位缓冲段1（PBS1）、相位缓冲段2（PBS2）
- 帧由位构成，一个位由4个段构成，每个段由若干个Tq（Time Quantum）组成，最小的时间单位
- 可以任意设定位时序，来实现 多个单元同时采样，或者 任意设定采样点

![1685606683226](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685606683226.png)

- 采样点是指读取总线电平，并将读到的电平作为位值的点。位置在 PBS1 结束处 

![1685606736816](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685606736816.png)

**仲裁原理**

- 当多个单元同时开始发送时，各发送单元从仲裁段的第一位开始进行仲裁。连续输出显性电平最多的单元可继续发送 

![1685607003643](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685607003643.png)

## 2. I.MX6ULL FlexCAN 简介

- I.MX6ULL 带有 CAN 控制器外设：FlexCAN，完全符合CAN协议

- FlexCAN 支持四种模式：正常模式、冻结模式、仅监听模式、回环模式；禁止模式和停止模式（两种低功耗模式）

  - 正常模式(Normal)：

    在正常模式下， FlexCAN 正常接收或发送消息帧，所有的 CAN 协议功能都使能

  - 冻结模式(Freeze)：

    MCR 寄存器的 FRZ 位置 1 ；此模式下无法进行帧的发送或接收，CAN 总线同步丢失 

  - 仅监听模式(Listen-Onley) 

    CTRL 寄存器的 LOM 位置 1 ；在此模式下帧发送被禁止；CAN 控制器工作在被动错误模式，此时只会接收其他 CAN 单元发出的 ACK 消息 

  - 回环模式(Loop-Back)

    CTRL 寄存器的 LPB 位置 1；从模式下发送出来的数据流直接反馈给内部接收单元

**设置位时序（CAN通信波特率的计算）**

CTRL 寄存器中的 PRESDIV、 PROPSEG、 PSEG1、PSEG2 和 RJW 这 5 个位域用于设置 CAN 位时序 

- PRESDIV 为 CAN 分频值，设置CAN协议中的Tq值；fCANCLK为 FlexCAN 模块时钟 

  ![1685608010404](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685608010404.png)

SS：此段固定为 1 个 Tq 长度，因此不需要我们去设置

PTS： CTRL 寄存器中的 PROPSEG 位域设置此段，可以设置为 0 ~ 7，对应 1 ~ 8 个 Tq 

PBS1：CRTL 寄存器中的 PSEG1 位域设置此段，可以设置为 0 ~ 7，对应 1 ~ 8 个 Tq 

PBS2：CRTL 寄存器中的 PSEG2 位域设置此段，可以设置为 1 ~ 7，对应 2 ~ 8 个 Tq

SJW：CRTL 寄存器中的 RJW 位域设置此段，可以设置 0 ~ 3，对应 1 ~ 4 个 Tq 

- 总Tq = SYNC_SEG+(PROP_SEG+PSEG1+2)+(PSEG2+1) 

![1685608657987](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685608657987.png)

## 3. 程序编写

### 3.1 原理图

![1685608730574](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685608730574.png)

TJA1050 是 CAN 收发器

### 3.2 修改设备树

**FlexCAN1 引脚节点信息 **

```c
 pinctrl_flexcan1: flexcan1grp{
 	fsl,pins = <
 		MX6UL_PAD_UART3_RTS_B__FLEXCAN1_RX 0x1b020
 		MX6UL_PAD_UART3_CTS_B__FLEXCAN1_TX 0x1b020
 	>;
 };
```

**FlexCAN1 控制器节点信息 **

```c
flexcan1: can@02090000 {
	compatible = "fsl,imx6ul-flexcan", "fsl,imx6q-flexcan";	 //  drivers/net/can/flexcan.c
	reg = <0x02090000 0x4000>;
	interrupts = <GIC_SPI 110 IRQ_TYPE_LEVEL_HIGH>;
	clocks = <&clks IMX6UL_CLK_CAN1_IPG>,
		<&clks IMX6UL_CLK_CAN1_SERIAL>;
	clock-names = "ipg", "per";
	stop-mode = <&gpr 0x10 1 0x10 17>;
	status = "disabled";
};
```

```c
 &flexcan1 {
 	pinctrl-names = "default";
 	pinctrl-0 = <&pinctrl_flexcan1>;
 	xceiver-supply = <&reg_can_3v3>;    // 指定 CAN 收发器的电压为 3.3V
 	status = "okay";		// 使能 FlexCAN1
 };
```

NXP 官方提供了 Linux 内核默认已经集成了 I.MX6ULL 的 FlexCAN 驱动，但没有使能，我们需要配置 Linux 内核，使能 FlexCAN 驱动

**使能 Freescale 系 CPU 的 FlexCAN 外设驱动**

```
-> Networking support
	-> CAN bus subsystem support
		-> CAN Device Drivers
			-> Platform CAN drivers with Netlink support
				-> <*> Support for Freescale FLEXCAN based chips //选中
```

重新编译内核，使用新的内核和设备树启动开发板

## 4. 测试 CAN

```shell
ifconfig -a    // 查看所有网卡 CAN总线接口设备作为网络设备进行统一管理
```

### 4.1 移植 iproute 2

busybox 自带的 ip 不支持 can 操作，重新移植 ip 命令

```
tar -vxzf iproute2-4.4.0.tar.gz

CC:=arm-linux-gnueabihf-gcc		// 配置

make	// 编译结束，会得到一个 ip 的命令
```

将交叉编译的 ip 拷贝到 开发板中（先不要替换原根文件系统中的 ip 命令）

```
sudo cp ip /home/zuozhongkai/linux/nfs/test_rootfs/lib/modules/4.1.15/ -f

cd lib/modules/4.1.15/
./ip -V //执行新的 ip 命令，查看版本号
```

新的 ip 替换原来的

```
cd lib/modules/4.1.15/
cp ip /sbin/ip -f

ip -V //查看 ip 命令版本号
```

### 4.2  移植 can-utils 工具

can-utils 这个工具来对 can0 网卡进行测试 

```
tar -vxzf can-utils-2020.02.04.tar.gz //解压

cd can-utils-2020.02.04 		//进入 can-utils 源码目录
./autogen.sh //先执行 autogen.sh，生成配置文件 configure
./configure --target=arm-linux-gnueabihf --host=arm-linux-gnueabihf --prefix=/home/zuozhongkai/linux/IMX6ULL/tool/can-utils --disable-static --enable-shared       //配置

make 					//编译
make install
```

can-utils 小工具全部拷贝到开发板根文件系统下的/usr/bin  

```
sudo cp bin/* /home/zuozhongkai/linux/nfs/rootfs/usr/bin/ -f
```

## 5. CAN 通信测试

### 5.1 两块 ALPHA 开发板连接

```shell
ip link set can0 type can bitrate 500000		# can0 速度为 500Kbit/S

ifconfig can0 up # 打开 can0

candump can0 # 接收数据

cansend can0 5A1#11.22.33.44.55.66.77.88	# “5A1”是帧 ID，“#”号后面的“11.22.33.44.55.66.77.88” 就是要发送的数据，十六进制

ifconfig can0 down	# 关闭 can0
```

回环测试

```c
ifconfig can0 down //如果 can0 已经打开了，先关闭
ip link set can0 type can bitrate 500000 loopback on //开启回环测试
ifconfig can0 up //重新打开 can0
candump can0 & //candump 后台接收数据
cansend can0 5A1#11.22.33.44.55.66.77.88 //cansend 发送数据
```

### 5.2 开发板与电脑

需要用到 USB 转 CAN 卡；编写 CAN 总线应用直接使用 Linux 提供的 SocketCAN 接口

