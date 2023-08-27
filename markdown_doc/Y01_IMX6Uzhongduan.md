# Cortex-A7(I.MX6U) 中断系统

## 0. Cortex-A3 中断系统

STM32 的中断系统主要有一下关键点：

1. 中断向量表
2. NVIC（内嵌向量中断控制器）
3. 中断使能
4. 中断服务函数

ARM 处理器都是从地址 0X0000 0000 开始运行，代码是下载到 0X8000 0000 开始的存储区域，中断向量表偏移实现将中断向量表存放到任意地址处，中断向量表偏移配置在函数SystemInit 中完成，通过 SCB_VTOR 寄存器写入新的中断向量表首地址即可

## 1. Cortex-A7 中断系统简介

### 1.1 中断向量表

- 中断向量表中存放的是中断向量；中断服务程序的入口地址或存放中断服务程序的首地址成为中断向量
- 中断向量表是一系列中断服务程序入口地址组成的表
- 当某个中断被触发以后就会自动跳转到中断向量表中对应的中断服务程序入口地址
- 中断向量表位于整个程序的最前面

**Cortex-A7 一共有 8 个中断（一个没用，实际只有 7 个中断）**

![1686819263262](E:\typora\markdownImage\1686819263262.png)

- 复位中断(Rest)：CPU 复位以后就会进入复位中断，在复位中断服务函数里面做一些初始化工作，比如初始化 SP 指针、 DDR 等等 
- 未定义指令中断(Undefined Instruction)，如果指令不能识别的话就会产生此中断
- 软中断(Software Interrupt,SWI)，由 SWI 指令引起的中断， Linux 的系统调用会用 SWI 指令来引起软中断，通过软中断来陷入到内核空间
- 指令预取中止中断(Prefetch Abort)，预取指令的出错的时候会产生此中断 
- 数据访问中止中断(Data Abort)，访问数据出错的时候会产生此中断 
- IRQ 中断(IRQ Interrupt)，外部中断，芯片内部的外设中断都会引起此中断的发生 
- FIQ 中断(FIQ Interrupt)，快速中断，如果需要快速处理中断的话就可以使用此中断 

当任意一个外部中断发生的时候都会触发 IRQ 中断。在 IRQ 中断服务函数里面就可以读取指定的寄存器来判断发生的具体是什么中断，进而根据具体的中断做出相应的处理

------

实际只需要编写 复位中断函数 Reset_Handler 和 IRQ 中断服务函数 IRQ_Handler，其他的中断函数都是死循环

```
/* start.S */
/* 汇编代码 设置中断向量表；初始化 c 语言环境 */

.global _start 	/* 全局标号 */

_start:
	ldr pc, =Reset_Handler 		/* 复位中断 */
	ldr pc, =Undefined_Handler 	/* 未定义指令中断 */
	ldr pc, =SVC_Handler 		/* SVC(Supervisor)中断 */
	ldr pc, =PrefAbort_Handler 	/* 预取终止中断 */
	ldr pc, =DataAbort_Handler 	/* 数据终止中断 */
	ldr pc, =NotUsed_Handler 	/* 未使用中断 */
	ldr pc, =IRQ_Handler 		/* IRQ 中断 */
	ldr pc, =FIQ_Handler 		/* FIQ(快速中断)未定义中断 */
	

/* 复位中断 */
Reset_Handler:
	/* 复位中断具体处理过程 */

/* 未定义中断 */
Undefined_Handler:
	ldr r0, =Undefined_Handler
	bx r0
	
/* SVC 中断 */
SVC_Handler:
	ldr r0, =SVC_Handler
	bx r0

/* 预取终止中断 */
PrefAbort_Handler:
	ldr r0, =PrefAbort_Handler
	bx r0

/* 数据终止中断 */
DataAbort_Handler:
	ldr r0, =DataAbort_Handler
	bx r0

/* 未使用的中断 */
NotUsed_Handler:
	ldr r0, =NotUsed_Handler
	bx r0
	
/* IRQ 中断！重点！！！！！ */
IRQ_Handler:
	/* 复位中断具体处理过程 */
	
/* FIQ 中断 */
FIQ_Handler:
	ldr r0, =FIQ_Handler
	bx r0
```

### 1.2 GIC (general interrupt controller)，中断管理机构

- I.MX6U(Cortex-A) 的中断控制器叫 GIC
- GIC 是 ARM 公司 为 Cortex-A/R 内核提供的一个中断控制器，目前 GIC 有 4 个版本：V1~V4（Cotex-A7用的是v2）
- 当 GIC 接收到外部中断信号以后就会报告给 ARM 内核，ARM 内核只提供了 四个信号给 GIC 来汇报中断情况

![1686821124221](E:\typora\markdownImage\1686821124221.png)

> - VFIQ：虚拟快速 FIQ 
> - VIRQ：虚拟外部 IRQ 
> - FIQ：快速中断 FIQ
> - IRQ：外部中断 IRQ 

GIC 将众多的中断源分为三类：

1. SPI(Shared Peripheral Interrupt),共享中断（支持多核工作）

   所有 Core 共享的中断，这个是最常见的，那些外部中断都属于 SPI 中断（这些中断所有 Core 都可以处理，不限定特定 Core）

2. PPI(Private Peripheral Interrupt)，私有中断

   由于 GIC 是多核的，独有的中断要指定的核心处理，这些中断叫做私有中断

3. SGI(Software-generated Interrupt)，软件中断 

   由软件触发引起的中断，通过向寄存器 GICD_SGIR 写入数据来触发，系统会使用 SGI 中断来完成多核之间的通信 

------

**中断 ID**

（用来区分不同的中断源）每个 CPU 最多支持 1020 个中断 ID，ID0~ID1019；包含 PPI、SPI、SGI；分配如下：

- ID0~ID15：分配给 SGI
- ID16~ID31：分配给 PPI
- ID32~ID1019：分配给 SPI

I.MX6U 的中断源共有 128 + 32 = 160 个（128 个 SPI + 32 个 SGI和PPI）

- I.MX6ULL 参考手册 “3.2 Cortex A7 interrupts” 

![1686827265272](E:\typora\markdownImage\1686827265272.png)

```c
/* MCIMX6Y2C.h(移植文件) */
 #define NUMBER_OF_INT_VECTORS 160 /* 中断源 160 个， SGI+PPI+SPI*/
 
typedef enum IRQn {
	/* Auxiliary constants */
	NotAvail_IRQn = -128,

	/* Core interrupts */
	Software0_IRQn = 0,
	Software1_IRQn = 1,
	Software2_IRQn = 2,
	...
	Reserved158_IRQn = 158,
	PMU_IRQ2_IRQn = 159
} IRQn_Type;
```

------

**GIC 架构**

GIC 架构分为了两个逻辑块：Distributor 和 CPU Interface，也就是分发器端和 CPU 接口端 

- Distributor（分发器端）：负责处理各个中断事件的分发问题；分发器收集所有的中断源，可以控制每个中断的优先级，它总是将优先级最高的中断事件发送到 CPU Interface

> 1. 全局中断使能控制 
> 2. 控制每一个中断的使能或者关闭 
> 3. 设置每个中断的优先级 
> 4. 设置每个中断的目标处理器列表 
> 5. 设置每个外部中断的触发模式：电平触发或边沿触发 
> 6. 设置每个中断属于组 0 还是组 1 

- CPU Interface（CPU 接口端）： CPU 接口端就是分发器和 CPU Core 之间的桥梁

> 1. 使能或者关闭发送到 CPU Core 的中断请求信号 
> 2. 应答中断 
> 3. 通知中断处理完成 
> 4. 设置优先级掩码，通过掩码来设置哪些中断不需要上报给 CPU Core 
> 5. 定义抢占策略 
> 6. 当多个中断到来的时候，选择优先级最高的中断通知给 CPU Core 

```c
/* core_ca7.h */
/* 定义了 GIC 结构体；分为了 发器端 和 CPU 接口端 */
typedef struct
{
	/* 分发器端寄存器 */
	uint32_t RESERVED0[1024];
	__IOM uint32_t D_CTLR; 	/* Offset: 0x1000 (R/W) */
	__IM uint32_t D_TYPER; 	/* Offset: 0x1004 (R/ ) */
	__IM uint32_t D_IIDR; 	/* Offset: 0x1008 (R/ ) */
	...
	/* CPU 接口端寄存器 */
	__IOM uint32_t C_CTLR; 	/* Offset: 0x2000 (R/W) */
	__IOM uint32_t C_PMR; 	/* Offset: 0x2004 (R/W) */
	__IOM uint32_t C_BPR; 	/* Offset: 0x2008 (R/W) */
	__IM uint32_t C_IAR; 	/* Offset: 0x200C (R/ ) */
} GIC_Type;
```

GIC 分发器端的相关寄存器相对于 GIC 基地址偏移为 0X1000；GIC 接收端相关寄存器相对于GIC基地址的偏移为 0X2000

------

**CP15 协处理器**

GIC 控制器的基地址由 Cortex-A 的 CP15 协处理器决定

- CP15 协处理器一般用于存储系统管理；一共有 16 个 32 位寄存器
- CP15 协处理器的访问通过特殊指令完成

> c0 寄存器可以获取到处理器内核信息
>
> c1 寄存器可以使能或禁止 MMU、 I/D Cache 等
>
> c12 寄存器可以设置中断向量偏移
>
> c15 寄存器可以获取 GIC 基地址 

```visual basic
MRC: 将 CP15 协处理器中的寄存器数据读到 ARM 寄存器；P15->ARM
MCR: 将 ARM 寄存器的数据写入 CP15 协处理器寄存器; ARM->CP15

MCR{cond} p15, <opc1>, <Rt>, <CRn>, <CRm>, <opc2>
cond:指令执行的条件码，如果忽略的话就表示无条件执行
opc1:协处理器要执行的操作码
Rt:ARM 源寄存器，要写入到 CP15 寄存器的数据就保存在此寄存器中
CRn:CP15 协处理器的目标寄存器
CRm:CP15 协处理器中附加的目标寄存器或者源操作数寄存器,如果不需要附加信息就将 CRm 设置为 C0
opc2:可选的协处理器特定操作码，当不需要的时候要设置为 0
```

在使用 MRC 或者 MCR 指令访问这 16 个寄存器(c0~c15)的时候，指令中的 CRn、 opc1、 CRm 和 opc2 通过不同的搭配，其得到的寄存器含义是不同的 

**c0 寄存器**

![1686830207109](E:\typora\markdownImage\1686830207109.png)

CRn=c0， opc1=0， CRm=c0， opc2=0 的时候就表示此时的 c0 就是 MIDR 寄存器

- MIDR 寄存器，主 ID 寄存器

  ![1686830303064](E:\typora\markdownImage\1686830303064.png)

  > bit31:24：厂商编号， 0X41， ARM 
  >
  > bit23:20：内核架构的主版本号， ARM 内核版本一般使用 rnpn 来表示，比如 r0p1，其中 r0 后面的 0 就是内核架构主版本号 
  >
  > bit19:16：架构代码， 0XF， ARMv7 架构 
  >
  > bit15:4：内核版本号， 0XC07， Cortex-A7 MPCore 内核 
  >
  > bit3:0：内核架构的次版本号， rnpn 中的 pn，比如 r0p1 中 p1 后面的 1 就是次版本号 

**c1 寄存器**

![1686830418545](E:\typora\markdownImage\1686830418545.png)

CRn=c1， opc1=0， CRm=c0， opc2=0 的时候就表示此时的 c1 就是 SCTLR 寄存器

- SCTLR 寄存器（系统控制寄存器），主要完成控制功能，比如使能或者禁止 MMU、I/D Cache 等

  ![1686832331820](E:\typora\markdownImage\1686832331820.png)

  > bit13： V , 中断向量表基地址选择位，为 0 的话中断向量表基地址为 0X00000000，软件可以使用 VBAR 来重映射此基地址，也就是中断向量表重定位。为 1 的话中断向量表基地址为0XFFFF0000，此基地址不能被重映射 
  >
  > bit12： I， I Cache 使能位，为 0 的话关闭 I Cache，为 1 的话使能 I Cache 
  >
  > bit11： Z，分支预测使能位，如果开启 MMU 的话，此位也会使能 
  >
  > bit10： SW， SWP 和 SWPB 使能位，当为 0 的话关闭 SWP 和 SWPB 指令，当为 1 的时候就使能 SWP 和 SWPB 指令 
  >
  > bit9:3：未使用，保留 
  >
  > bit2： C， D Cache 和缓存一致性使能位，为 0 的时候禁止 D Cache 和缓存一致性，为 1 时使能 
  >
  > bit1： A，内存对齐检查使能位，为 0 的时候关闭内存对齐检查，为 1 的时候使能内存对齐检查 
  >
  > bit0： M， MMU 使能位，为 0 的时候禁止 MMU，为 1 的时候使能 MMU 

**c12 寄存器**

![1686832531212](E:\typora\markdownImage\1686832531212.png)

CRn=c12， opc1=0， CRm=c0， opc2=0 的时候就表示此时 c12 为 VBAR 寄存器 

- VBAR 寄存器，向量表基地址寄存器；设置中断向量表偏移的时候就需要将新的中断向量表基地址写入 VBAR 中

**c15 寄存器**

![1686832712238](E:\typora\markdownImage\1686832712238.png)

- CBAR 寄存器：GIC 的基地址就保存在 CBAR 中

```
/* 获取 GIC 基地址 */
MRC p15, 4, r1, c15, c0, 0 ;  //获取 GIC 基础地址，基地址保存在 r1 中
```

获取 GIC 寄存器的基地址之后就可以设置 GIC 相关寄存器

### 1.3 中断使能

中断使能分为：

- IRQ 和 FIQ 总中断使能
- ID0~ID1019 这 1020 个中断源的使能

#### 1.3.1 IRQ 和 FIQ 总中断使能

- 外部中断和快速中断的总开关
- 寄存器 CPSR 的 I=1 禁止 IRQ，I=0使能 IRQ；F=1 禁止 FIQ，F=0 使能 FIQ
- 也可以用指令来完成 IRQ 和 FIQ 的使能和禁止

![1686833436117](E:\typora\markdownImage\1686833436117.png)

#### 1.3.2 ID0 ~ ID1019 中断使能和禁止

- GIC 寄存器 GICD_ISENABLERn（0~15） 和 GICD_ ICENABLERn 用来完成外部中断的使能和禁止
- Cortex-A7 内核来说中断 ID 只使用了 512 个；一个 bit 控制一个中断 ID 的使能，需要 16 个 GICD_ISENABLER 寄存器来完成中断的使能
- GICD_ISENABLER0 的 bit[15:0]对应 ID15~0 的 SGI 中断 
- GICD_ISENABLER0 的 bit[31:16]对应 ID31~16 的 PPI 中断
- 剩下的GICD_ISENABLER1~GICD_ISENABLER15 就是控制 SPI 中断的 

### 1.4 中断优先级设置

优先级设置主要有三部分：

- 配置优先级个数
- 设置抢占优先级和子优先级位数
- 设置指定中断 ID 的优先级，即设置外设优先级

#### 1.4.1 优先级 数 配置

- GIC 控制器最多可以支持 256 个优先级，数字越小，优先级越高 
- Cortex-A7 选择了 32 个优先级
- 使用中断的时候需要初始化 GICC_PMR 寄存器，此寄存器用来决定用几级优先级

**GICC_PMR 寄存器**

- 低八位有效，用来决定用几级优先级（总共有多少级优先级）

![1686834603257](E:\typora\markdownImage\1686834603257.png)

![1686834617251](E:\typora\markdownImage\1686834617251.png)

I.MX6U 是 Cortex-A7内核，所以支持 32 个优先级，因此 GICC_PMR 要设置为 0b11111000 

#### 1.4.2 抢占优先级和子优先级位数 设置

**GICC_BPR 寄存器**

- 用来决定 抢占优先级和子优先级各占多少位

![1686834798581](E:\typora\markdownImage\1686834798581.png)

低三位有效，用来决定抢占优先级和子优先级的位数

![1686834967982](E:\typora\markdownImage\1686834967982.png)

I.MX6U 的优先级位数为 5(32 个优先级)，所以可以设置 Binary point 为 2，表示 5 个优先级位全部为抢占优先级 

#### 1.4.3 优先级设置

- 某个中断 ID 的中断优先级设置由寄存器 D_IPRIORITYR 来完成
- Cortex-A7 使用了 512 个中断 ID，每个中断 ID 配有一个优先级寄存器，所以一共有 512 个 D_IPRIORITYR 寄存器 
- 如果优先级个数为 32 的话，使用寄存器 D_IPRIORITYR 的 bit7:4 来设置优先级，也就是说实际的优先级要左移 3 位

```
GICD_IPRIORITYR[40] = 5 << 3;
```

## 2. 实验程序

### 2.1 移植 SDK 包 中断相关文件

core_ca7.h 10个 API：

![1686835424123](E:\typora\markdownImage\1686835424123.png)

### 2.2 实验代码

见  03_interrupt

