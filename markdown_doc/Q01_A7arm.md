# Cortex-A7 MPCore 架构

## 1. Cortex-A7 MPCore 简介

Cortex-A7 MPCore 支持在一个处理器上选配 1~4 个内核 

![1686141128179](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1686141128179.png)

## 2. Coretex-A 处理器运行模型

- 以前 ARM 处理器有7种运行模型：User、 FIQ、 IRQ、 Supervisor(SVC)、 Abort、 Undef 和 System 
- Cortex-A 架构加入了 Monitor 、Hyp；共有9种处理模式
- 除了 User 用户模式以外，其他8种运行模式都是特权模式
- 用户模式下不能访问系统所有资源的，有些资源是受限的，想要访问这些资源必须进行模式的切换，用户模式是不能直接进行切换的，用户模式下需要借助异常来完成模式切换，当要切换模式的时候，应用程序可以产生异常，在异常的处理过程中完成处理器模式的切换
- 当中断或者异常发生以后，处理器就会进入到相应的异常模式种，每一种模式都有一组寄存器供异常处理程序使用，保证在进入异常模式后，用户模式下的寄存器不会被破坏

![1686141288144](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1686141288144.png)

## 3. Cortex-A 寄存器组

- ARM 架构提供 16 个 32 位通用寄存器(R0 ~ R15)供软件使用，前 15 个(R0 ~ R14)可以用作通用的数据存储，R15 是程序计数器 PC，用来保存将要执行的指令
- ARM 还提供了一个当前程序状态寄存器 CPSR 和一个备份程序状态寄存器 SPSR， SPSR 寄存器就是 CPSR 寄存器的备份 

![1686141666417](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1686141666417.png)

每一种运行模式都有一组与之对应的寄存器组，有的是所有模式所共用的同一个物理寄存器，有一些是各模式自己独立拥有的；在所有模式中，低寄存器(R0~R7)是共享同一组物理寄存器的

![1686141875224](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1686141875224.png)

Cortex-A 内核寄存器组成如下：

- 34 个通用寄存器，包括 R15 程序计数器(PC)，这些寄存器都是 32 位的 
- 8 个状态寄存器，包括 CPSR 和 SPSR 
- Hyp 模式下独有一个 ELR_Hyp 寄存器 

**通用寄存器（16 个通用寄存器R0~R15可以分为三类）：**

- 未备份寄存器

  未备份寄存器指的是 R0~R7 这 8 个寄存器，因为在所有的处理器模式下这 8 个寄存器都是同一个物理寄存器，在不同的模式下，这 8 个寄存器中的数据就会被破坏

- 备份寄存器

  备份寄存器中的 R8~R12 这 5 个寄存器有两种物理寄存器 

- 程序计数器 R15 

  程序计数器 R15 也叫做 PC，R15 保存着当前执行的指令地址值加 8 个字节，受 ARM 3级流水线机制导致的

## 4. 程序状态寄存器

- 所有的处理器模式都共用一个 CPSR 物理寄存器
- CPSR 是当前程序状态寄存器，该寄存器包含了条件标志位、中断禁止位、当前处理器模式标志等一些状态位以及一些控制位 
- 除了 User 和 Sys 这两个模式以外，其他 7 个模式每个都配备了一个专用的物理状态寄存器，叫做 SPSR(备份程序状态寄存器)，当特定的异常中断发生时， SPSR 寄存器用来保存当前程序状态寄存器(CPSR)的值，当异常退出以后可以用 SPSR 中保存的值来恢复 CPSR 

![1686142370840](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1686142370840.png)

