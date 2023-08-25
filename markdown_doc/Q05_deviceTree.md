# 设备树简介

## 1. DTS、DTB、DTC

- 设备树源文件拓展名为 .dts；其中 .dts 描述板级信息（开发板上有哪些 IIC 设备、 SPI 设备等 ），.dtsi 描述SOC 级信息（SOC 有几个 CPU、主频是多少、各个外设控制器信息等 ）
- DTB 是将 DTS 编译以后得到的二进制文件
- 将 .dts 编译成 .dtb 的工具就是 DTC 工具；DTC源码在 Linux 内核 scripts/dtc 目录下

```
/* scripts/dtc/Makefile */
hostprogs-y := dtc
always := $(hostprogs-y)

dtc-objs:= dtc.o flattree.o fstree.o data.o livetree.o treesource.o \
	srcpos.o checks.o util.o
dtc-objs += dtc-lexer.lex.o dtc-parser.tab.o
...
```

**编译设备树**：进入 Linux 源码根目录 执行 make dtbs

基于 ARM 架构的 SOC 有很多种，如何确定编译哪一个 DTS 文件

```
/* arch/arm/boot/dts/Makefile */
dtb-$(CONFIG_SOC_IMX6UL) += \
	imx6ul-14x14-ddr3-arm2.dtb \
	imx6ul-14x14-ddr3-arm2-emmc.dtb \
......
dtb-$(CONFIG_SOC_IMX6ULL) += \
	imx6ull-14x14-ddr3-arm2.dtb \
	imx6ull-14x14-ddr3-arm2-adc.dtb \
	imx6ull-14x14-ddr3-arm2-cs42888.dtb \
	imx6ull-14x14-ddr3-arm2-ecspi.dtb \
	imx6ull-14x14-ddr3-arm2-emmc.dtb \
	imx6ull-14x14-ddr3-arm2-epdc.dtb \
	imx6ull-14x14-ddr3-arm2-flexcan2.dtb \
	imx6ull-14x14-ddr3-arm2-gpmi-weim.dtb \
	imx6ull-14x14-ddr3-arm2-lcdif.dtb \
	imx6ull-14x14-ddr3-arm2-ldo.dtb \
	imx6ull-14x14-ddr3-arm2-qspi.dtb \
	imx6ull-14x14-ddr3-arm2-qspi-all.dtb \
	imx6ull-14x14-ddr3-arm2-tsc.dtb \
	imx6ull-14x14-ddr3-arm2-uart2.dtb \
	imx6ull-14x14-ddr3-arm2-usb.dtb \
	imx6ull-14x14-ddr3-arm2-wm8958.dtb \
	imx6ull-14x14-evk.dtb \
	imx6ull-14x14-evk-btwifi.dtb \
	imx6ull-14x14-evk-emmc.dtb \
	imx6ull-14x14-evk-gpmi-weim.dtb \
	imx6ull-14x14-evk-usb-certi.dtb \
	imx6ull-alientek-emmc.dtb \
	imx6ull-alientek-nand.dtb \
	imx6ull-9x9-evk.dtb \
	imx6ull-9x9-evk-btwifi.dtb \
	imx6ull-9x9-evk-ldo.dtb
...
```

当选中 I.MX6ULL 这个 SOC 以后(CONFIG_SOC_IMX6ULL=y) ，所有使用到 I.MX6ULL 这个 SOC 的板子对应的 .dts 都会被编译为 .dtb；在做开发板移植的时候只需要将设备树文件添加到对应的 SOC 之下即可

## 2. 设备树语法

### 2.1 .dtsi头文件

```
#include <dt-bindings/input/input.h>
#include "imx6ull.dtsi"
#include "imx6ull-14x14-evk.dts"
```

### 2.2 设备节点

设备树采用树形结构来描述板子上的设备信息的文件，每个设备都是一个节点，叫作设备节点

```
label:node-name@unit-address
label：节点标签，为了方便访问节点，可以直接通过 &label 来访问
node-name：节点名字
unit-address：设备的地址或者寄存器的首地址
```

每个节点有不同属性，属性都是键值对，值可以为空或者任意字节流；常见属性形式：

1. 字符串

   ```
   compatible = "arm,cortex-a7";
   ```

2. 32 位无符号整数 

   ```
   reg = <0>;
   reg = <0 0x123456 100>;		// 一组值
   ```

3. 字符串列表 

   ```
   compatible = "fsl,imx6ull-gpmi-nand", "fsl, imx6ul-gpmi-nand";   // 中间用 ， 分隔
   ```

### 2.3 标准属性

1. compatible

   - 值是一个字符串列表
   - 用于将设备和驱动绑定起来

   ```
   compatible = "manufacturer,model"
   manufacturer：厂商
   model：模块对应的驱动名字
   ```

2. model

   - 值是一个字符串，描述设备模块信息

   ```
   model = "wm8960-audio";
   ```

3. status

   - 描述设备状态信息

   ![1686059209517](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1686059209517.png)

4. #address-cells  #size-cells  

   - 值都是 无符号 32位整形；用于描述子节点的地址信息
   - #address-cells 属性值决定了子节点 reg 属性中地址信息所占用的字长（32位）
   - #size-cells 属性值决定了子节点 reg 属性中长度信息所占的字长(32 位) 

   ```
   reg = <address1 length1 address2 length2 address3 length3……>
   address：起始地址
   length：地址长度
   ```

5. reg 

   - 描述地址空间资源信息

6. ranges

   - ranges属性值可以为空或者按照(child-bus-address,parent-bus-address,length)格式编写的数字矩阵 
   - ranges 是一个地址映射/转换表
   - child-bus-address：子总线地址空间的物理地址，由父节点的#address-cells 确定此物理地址所占用的字长
   - parent-bus-address：父总线地址空间的物理地址，由父节点的#address-cells 确定此物理地址所占用的字长
   - length：子地址空间的长度，由父节点的#size-cells 确定此地址长度所占用的字长 
   - 子地址空间和父地址空间完全相同，ranges值可以为空

   ```
   soc {
   	compatible = "simple-bus";
   	#address-cells = <1>;
   	#size-cells = <1>;
   	ranges = <0x0 0xe0000000 0x00100000>;
   	
   	serial{
   		device_type = "serial";
   		compatible = "ns16550";
   		reg = <0x4600 0x100>;
   		clock-frequency = <0>;
   		interrupts = <0xA 0x8>;
   		interrupt-parent = <&ipic>;
   	};
   };
   ```

7. name （弃用）

   - 属性值为字符串，name属性用来记录节点的名字

8. device_type （弃用）

   - 属性值为字符串，用来描述设备的 FCode

### 2.4 根节点 compatible 属性

- 通过根节点的 compatible 属性可以知道我们所使用的设备，第一个值通常描述所使用的硬件设备名字，第二个值描述了设备所使用的 SOC

- Linux 内核会通过根节点的 compatible 属性查看是否支持该设备，如果支持就会启动 Linux 内核

  ```
  /{
  	model = "Freescale i.MX6 ULL 14x14 EVK Board";
  	compatible = "fsl,imx6ull-14x14-evk", "fsl,imx6ull";
  	.....
  };
  ```

  **未引入设备树之前 u-boot 给内核传递 machine_id 参数， linux 内核检查 machine_id， 符合则启动内核，当内核引入设备树后，会根据 machine_desc 结构体内 .dt_compat 表内设备值与根节点的 compatible 属性值进行对比，有一个相等就表示内核支持这个设备**

1. 使用设备树之前的设备匹配法

   - 没使用设备树以前，uboot 会向 Linux 内核传递一个 machine id 值（设备 ID），Linux 内核根据这个 machine id 来判断是否支持该设备
   - Linux 内核 使用 MACHINE_START 和 MACHINE_END 来定义一个 machine_desc 结构体来描述设备

   ```
   /* arch/arm/mach-imx/mach-mx35_3ds.c */
   MACHINE_START(MX35_3DS, "Freescale MX35PDK")
   	/* Maintainer: Freescale Semiconductor, Inc */
   	.atag_offset = 0x100,
   	.map_io = mx35_map_io,
   	.init_early = imx35_init_early,
   	.init_irq = mx35_init_irq,
   	.init_time = mx35pdk_timer_init,
   	.init_machine = mx35_3ds_init,
   	.reserve = mx35_3ds_reserve,
   	.restart = mxc_restart,
   MACHINE_END
   ```

   ```c
   /* 宏定义在 arch/arm/include/asm/mach/arch.h */
   #define MACHINE_START(_type,_name) \
   static const struct machine_desc __mach_desc_##_type \
   	__used \
   	__attribute__((__section__(".arch.info.init"))) = { \
   	.nr = MACH_TYPE_##_type, \
   	.name = _name,
   #define MACHINE_END \
   }
   ```

   ```c
   /* 展开 */
   static const struct machine_desc __mach_desc_MX35_3DS \
   	__used \
   	__attribute__((__section__(".arch.info.init"))) = {
   	.nr = MACH_TYPE_MX35_3DS,		/* machine id */
   	.name = "Freescale MX35PDK",
   	/* Maintainer: Freescale Semiconductor, Inc */
   	.atag_offset = 0x100,
   	.map_io = mx35_map_io,
   	.init_early = imx35_init_early,
   	.init_irq = mx35_init_irq,
   	.init_time = mx35pdk_timer_init,
   	.init_machine = mx35_3ds_init,
   	.reserve = mx35_3ds_reserve,
   	.restart = mxc_restart,
   };
   ```

   include/generated/mach-types.h 定义了大量的 machine id；uboot 会给 Linux 内核传递 machine id 这个参数，Linux 内核会检查这个 machine id，其实就是与 这些宏定义进行对比，若相等则表明 Linux 内核支持该设备

   ```
   /* include/generated/mach-types.h */
   #define MACH_TYPE_EBSA110 0
   #define MACH_TYPE_RISCPC 1
   #define MACH_TYPE_EBSA285 4
   #define MACH_TYPE_NETWINDER 5
   #define MACH_TYPE_CATS 6
   ....
   #define MACH_TYPE_MX35_3DS 1645
   ...
   #define MACH_TYPE_PFLA03 4575
   ```

2. 使用设备树以后的设备匹配方法

   Linux 内核引入设备树之后，就不再使用 MACHINE_START，而是 DT_MACHINE_START

   ```
   /* 定义 arch/arm/include/asm/mach/arch.h */
   #define DT_MACHINE_START(_name, _namestr) \
   	static const struct machine_desc __mach_desc_##_name \
   	__used \
   	__attribute__((__section__(".arch.info.init"))) = { \
   		.nr = ~0,
   		.name = _namestr,
   ```

   DT_MACHINE_START 直接将 .nr 设置为 ~0，表明不会根据 machine id 来检查 Linux 内核是否支持

   ```c
   /* demo arch/arm/mach-imx/mach-imx6ul.c */
   static const char *imx6ul_dt_compat[] __initconst = {
   	"fsl,imx6ul",
   	"fsl,imx6ull",
   	NULL,
   };
   
   DT_MACHINE_START(IMX6UL, "Freescale i.MX6 Ultralite (Device Tree)")
   	.map_io = imx6ul_map_io,
   	.init_irq = imx6ul_init_irq,
   	.init_machine = imx6ul_init_machine,
   	.init_late = imx6ul_init_late,
   	.dt_compat = imx6ul_dt_compat,  /* 根据 .dt_compat 表来实现匹配 */
   MACHINE_END
   ```

   machine_desc 结构体中有个 .dt_compat 成员变量，内部保存这本设备兼容属性，只要某个设备（板子）的根根节点的 compatible 属性值与 该设备.dt_compat 表中任意一个值相等，就表示 Linux 支持该设备，就能启动内核

   **LInux内核匹配过程：**

   Linux 内核会调用 start_kernel 来启动内核，start_kernel 会调用 setup_arch 来匹配 machine_desc

   - arch/arm/kernel/setup.c

   ![1686108778973](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1686108778973.png)

   setup_machine_fdt 来获取匹配的 machine_desc，参数就是 atags 的首地址，也就是 Uboot 传递给 Linux 内核的 dtb 文件首地址，返回值就是找到的最匹配的 machine_desc

   - arch/arm/kernel/devtree.c

   ![1686108950803](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1686108950803.png)

   of_flat_dt_match_machine 来获取匹配的 machine_desc，参数 mdesc_best 是默认的 macine_desc，参数 arch_get_next_match 是个函数（arch/arm/kernel/devtree.c），作用就是获取 Linux 内核中下一个 machine_desc

   匹配过程就是：用设备树根节点的 compatible 属性值和 Linux 内核中 machine_desc 下 .dt_compat 的值比较，相等表示找到匹配的 machine_desc

   - drivers/of/fdt.c

   ![1686109468355](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1686109468355.png)

## 3. 手写设备树

1. 创建根节点及根节点的 compatible
2. 添加 cpu 节点
3. 添加 soc 节点
   - 像 uart，iic 控制器等这些都是属于 SOC 内部外设，一般会创建一个 soc 父节点来管理 soc 内部外设子节点
4. 添加 ocram 节点
5. 添加 aips1、aips2、aips3 三个域 子节点
6. ...

```
 / {
	compatible = "fsl,imx6ull-alientek-evk", "fsl,imx6ull";

	cpus {
		#address-cells = <1>;
		#size-cells = <0>;

		//CPU0 节点
		cpu0: cpu@0 {
			compatible = "arm,cortex-a7";
			device_type = "cpu";
			reg = <0>;
			};
	};

	//soc 
	soc {
		#address-cells = <1>;
		#size-cells = <1>;
		compatible = "simple-bus";
		ranges;
		
		//ocram 节点
		ocram: sram@00900000 {
			compatible = "fsl,lpm-sram";
			reg = <0x00900000 0x20000>;
		};
		
		//aips1 节点
		aips1: aips-bus@02000000 {
			compatible = "fsl,aips-bus", "simple-bus";
			#address-cells = <1>;
			#size-cells = <1>;
			reg = <0x02000000 0x100000>;
			ranges;
			
			//ecspi1 节点
			ecspi1: ecspi@02008000 {
				#address-cells = <1>;
				#size-cells = <0>;
				compatible = "fsl,imx6ul-ecspi", "fsl,imx51-ecspi";
				reg = <0x02008000 0x4000>;
				status = "disabled";
			};	
		}
		
		//aips2 节点
		aips2: aips-bus@02100000 {
			compatible = "fsl,aips-bus", "simple-bus";
			#address-cells = <1>;
			#size-cells = <1>;
			reg = <0x02100000 0x100000>;
			ranges;
			
			//usbotg1 节点
			usbotg1: usb@02184000 {
				compatible = "fsl,imx6ul-usb", "fsl,imx27-usb";
				reg = <0x02184000 0x4000>;
				status = "disabled";
			};
		}
		
		//aips3 节点
		aips3: aips-bus@02200000 {
			compatible = "fsl,aips-bus", "simple-bus";
			#address-cells = <1>;
			#size-cells = <1>;
			reg = <0x02200000 0x100000>;
			ranges;
			
			//rngb 节点
			rngb: rngb@02284000 {
				compatible = "fsl,imx6sl-rng", "fsl,imx-rng", "imxrng";
				reg = <0x02284000 0x4000>;
			};
		}	
	}
}
```

## 4. 设备树在系统中的体现

Linux 内核启动的时候会解析设备树中各个节点的信息，并且在根文件系统的/proc/device-tree  

### 4.1 特殊节点

根节点 / 下有两个特殊的子节点 aliases 和 chosen

**aliases 子节点**

- 主要功能是为了定义别名，方便访问节点

**chosen 子节点**

- 不是真实的设备，主要是为了 uboot 向 Linux 内核传递数据，重点是里面的 bootargs 参数
- uboot 在启动 Linux 内核的时候会将 bootargs 的值传递给 Linux 内核，bootargs 会作为 Linux 内核的命令行参数
- uboot 在 chosen 节点里面添加了 bootargs 属性，并设置 bootargs 属性值为 bootargs 环境变量的值
- common/fdt_support.c

![1686129241411](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1686129241411.png)

![1686129252771](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1686129252771.png)

![1686129269619](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1686129269619.png)

## 5. Linux 内核解析 DTB 文件

Linux 内核在启动的时候会解析 DTB 文件，然后在/proc/device-tree 目录下生成相应的设备树节点

![1686129394742](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1686129394742.png)

## 6. 绑定信息文档

- 设备树是用来描述板子上的设备信息的，不同设备其信息不同
- 添加一个硬件对应的节点，可从 /Documentaion/devicetree/bindings 查看相关帮助文档
