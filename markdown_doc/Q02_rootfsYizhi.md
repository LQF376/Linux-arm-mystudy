# Linux 内核移植

## 1. Linux 内核编译

### 1.1 修改顶层的 Makefile

```
/* 设置交叉编译 */
ARCH ?= arm
CROSS_COMPILE ?= arm-linux-gnueabihf-
```

### 1.2 配置并编译内核

默认配置文件保存在 arch/arm/configs （imx_v7_mfg_defconfig）

```
make clean
make imx_v7_mfg_defconfig	// 配置 Linux 内核
make -j16	// 编译Linux内核
```

编译完成后：

- arch/arm/boot 目录下生成 zImage 镜像文件 
- arch/arm/boot/dts 目录下开发板对应的.dtb(设备树)文件 （imx6ull-14x14-evk.dtb）

### 1.3 Linux 内核启动测试

- 将编译后的 zImage 镜像文件 和 设备树文件 拷贝到 tftp 文件夹内

  ```
  cp arch/arm/boot/zImage /home/zuozhongkai/linux/tftpboot/ -f
  cp arch/arm/boot/dts/imx6ull-14x14-evk.dtb /home/zuozhongkai/linux/tftpboot/ -f
  ```

- 进入 uboot 模式，挂载网络系统进行下载

  ```
  tftp 80800000 zImage
  tftp 83000000 imx6ull-14x14-evk.dtb
  bootz 80800000 - 83000000
  ```

### 1.4 根文件系统缺失

Linux 内核启动以后需要根文件系统，根文件系统由 uboot 的 bootargs 环境变量指定；若没有指定根文件系统，就会提示内核崩溃

![1684930659187](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1684930659187.png)

## 2. 修改自己的 Linux 内核

### 2.1 修改设备树

设备树源码文件存放位置：arch/arm/boot/dts

.dts 是设备树源码，编译 Linux 的时候会将其编译为 .dtb 文件

```
/* 修改顶层的 Makefile 文件 */
/* 添加后才能将 .dts 文件 编译成 .dtb 文件 */
dtb-$(CONFIG_SOC_IMX6ULL) += \
	...
	imx6ull-alientek-emmc.dtb
```

## 3. 查看 CPU 主频信息

BogoMIPS 是 Linux 系统中衡量处理器运行速度的一个尺子，处理器性能越强大，主频越高，BogoMIPS 值就越大

```bash
cat /proc/cpuinfo     # 其中一种方法
```

查看当前 CPU 的工作频率

```
/sys/bus/cpu/devices/cpu0/cpufreq     # 第二种方法
```

![1684933205483](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1684933205483.png)

scaling_governor：调频策略

![1684933268970](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1684933268970.png)

```
cat /sys/bus/cpu/devices/cpu0/cpufreq/stat/time_in_state	// 查看 CPU 在各个频率下的工作时间
```

修改 CPU 的调度策略：

- 可以通过修改 imx_xxx_defconfig 文件，重新编译内核来实现调度策略的更改
- 编译前，make menuconfig 进入图形化界面配置 Linux 内核的调度策略

# 根文件系统的构建

根文件系统是内核启动时所 mount（挂载）的第一个文件系统，系统引导启动程序会在根文件系统挂载之后从中把一些基本的初始化脚本和服务等加载到内存中去运行

根文件系统和Linux 内核是分开的，单独的 Linux 内核时没法正常工作的，必须要搭配根文件系统；如果不提供根文件系统，Linux 内核在启动的时候就会提示内核崩溃（Kernel panic）

## 1. 常见跟文件系统的一些子目录

- /bin：bin 文件就是可执行文件。所以此目录下存放着系统需要的可执行文件，一般都是一些命令 
- /dev：此目录下的文件都是和设备有关的，此目录下的文件都是设备文件
- /etc：此目录下存放着各种配置文件 
- /lib：此目录下存放着 Linux 所必须的库文件 
- /mnt：临时挂载目录 
- /proc：当 Linux 系统启动以后会将此目录作为 proc 文件系统的挂载点，一般用来存储系统运行信息文件 
- /usr ：Unix 操作系统软件资源目录 
- /var：存放一些可以改变的数据
- /sbin：存放一些可执行文件，但是此目录下的文件或者说命令只有管理员才能使用
- /sys： 作为 sysfs 文件系统的挂载点，此目录是系统设备管理的重要目录 
- /opt：可选的文件、软件存放区 

## 2. BusyBox 构建根文件系统

BusyBox 是一个集成了大量的 Linux 命令和工具的软件

一般在 Linux 驱动开发的时候都是通过 nfs 挂载根文件系统的，当产品最终上市开卖的时候才会将根文件系统烧写到 EMMC 或者 NAND 中

下载完 busybox.tar.bz2 后，在 Ubuntu 内解压，修改编译

```
mkdir rootfs       # 在 nfs 目录中创建一个 rootfs 的子目录，方便找到解压文件
tar -vxjf busybox-1.29.0.tar.bz2
```

**2.1 修改顶层Makefile**

```
CROSS_COMPILE ?= /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf
ARCH ?= arm
```

**2.2 busybox 中文支持**

需要修改 busybox 源码，取消 busybox 对中文显示的限制

**2.3 配置 busybox**

```
make defconfig	// 默认缺省配置    allyesconfig 全选配置      allnoconfig 最小配置
make menuconfig  // 调出图形配置窗口
```

**2.4 编译 busybox**

```
make
make install CONFIG_PREFIX=/home/book/nfs/rootfs	// 安装路径
```

![1684986381872](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1684986381872.png)

Linux 内核 init 进程最后会查找用户空间的 init 程序，找到后会运行用户空间的 init 程序，从而切换到用户态

**2.5 完善根文件系统**

- "/lib" 目录添加库文件（将交叉编译器内部的库文件）

  ```
  /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/libc/lib
  cp *so* *.a /home/book/linux/nfs/rootfs/lib -d
  
  /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/lib
  cp *so* *.a /home/book/linux/nfs/rootfs/lib -d
  ```

- "/usr/lib" 目录添加库文件

  ```
  /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/libc/usr/lib
  cp *so* *.a /home/book/linux/nfs/rootfs/lib -d
  ```

- 创建其他文件夹，dev、proc、mnt、sys、tmp、root

- 创建 /etc/init.d/rcS 文件

  Linux 内核启动后需要启动一些服务，rcS 就是规定启动哪些文件的脚本文件

  ```
  #!/bin/sh
  
  PATH=/sbin:/bin:/usr/sbin:/usr/bin:$PATH
  LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lib:/usr/lib
  export PATH LD_LIBRARY_PATH			// 环境变量
  
  mount -a	// 挂载所有的文件系统，文件系统由 /etc/fstab 指定
  mkdir /dev/pts
  mount -t devpts devpts /dev/pts
  
  echo /sbin/mdev > /proc/sys/kernel/hotplug   // 使用 mdev 来管理热插拔设备
  mdev -s
  ```

  ```
  chmod 777 rcS	// 创建好文件后一定要给权限！！！
  ```

- 创建 /etc/fstab 文件（fstab 在 Linux 开机以后自动配置哪些需要自动挂载的分区）

  ```
  /* 格式 */
  <file system> <mount point> <type> <options> <dump> <pass>
  file system：要挂载的特殊设备
  mount point：挂载点
  type：文件系统类型 ext2、ext3、proc、romfs等
  options：挂载选项 "man mount" 查看具体选项
  dump：1 允许备份；0 不备份
  pass：磁盘检查设置，为0表示不检查
  -----------------------------------------------------------------
  #<file system> <mount point> <type> <options> <dump> <pass>
  proc 			/proc 		proc 	defaults 0 		0
  tmpfs			/tmp 		tmpfs 	defaults 0 		0
  sysfs 			/sys 		sysfs 	defaults 0 		0
  ```

- 创建 /etc/inittab 文件

  init 程序会读取 /etc/inittab 文件；由若干条指令组成；每条指令用 ：分隔 4 个段

  ```
  <id>:<runlevels>:<action>:<process>
  id：每条指令的标识符，不能重复，busybox 用来指定启动进程的控制 tty
  runlevels：对 busybox 来说这项没用
  action：动作，busybox 支持的动作如图所示
  process：具体的动作
  ---------------------------------------------
  #etc/inittab
  ::sysinit:/etc/init.d/rcS
  console::askfirst:-/bin/sh
  ::restart:/sbin/init
  ::ctrlaltdel:/sbin/reboot
  ::shutdown:/bin/umount -a -r
  ::shutdown:/sbin/swapoff -a
  ```

  ![1684995167395](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1684995167395.png)

## 3.0 根文件系统测试

一般 Linux 驱动开发的时候都是通过 nfs 挂载跟文件系统的，当产品最终上市开卖的时候才会将根文件系统烧写到 EMMC 或者 NAND 中

我们一般通过 NFS 挂载的方式来测试建好的 根文件系统 rootfs，uboot 中 bootargs 环境变量中会设置 ”root“ 的值，我们只需将 root 的值改为 NFS 挂载即可

```
root=/dev/nfs nfsroot=[<server-ip>:]<root-dir>[,<nfs-options>] ip=<client-ip>:<server-ip>:<gwip>:<netmask>:<hostname>:<device>:<autoconf>:<dns0-ip>:<dns1-ip>

server-ip：服务器 IP 地址，Ubuntu 的 IP 地址
root-dir：根文件系统的存放路径
nfs-options：NFS 的其他可选选项，一般不设置
client-ip：客户端 IP 地址，开发板的 IP 地址
gw-ip：网关地址，192.168.1.1
netmask：子网掩码 255.255.255.0
hostname：客户端名字，一般不设置
device：设备名，网卡名，一般是 eth0、eth1
autoconf：自动配置，一般不使用，off
dns0-ip：DNS0 服务器 IP 地址，不使用
dns1-ip：DNS1 服务器 IP 地址，不使用
```

```
root=/dev/nfs nfsroot=192.168.1.250:/home/zuozhongkai/linux/nfs/rootfs,proto=tcp rw ip=192.168.1.251:192.168.1.250:192.168.1.1:255.255.255.0::eth0:off

proto=tcp # 表示使用 tcp 协议
rw # 表示 nfs 挂载的根文件系统为可读可写
```

```
setenv bootargs 'console=ttymxc0,115200 root=/dev/nfs nfsroot=192.168.1.250:
/home/zuozhongkai/linux/nfs/rootfs,proto=tcp rw ip=192.168.1.251:192.168.1.250:192.168.1.1:
255.255.255.0::eth0:off' //设置 bootargs
saveenv //保存环境变量
boot	// 启动 Linux 内核
```

![1688126209142](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1688126209142.png)
