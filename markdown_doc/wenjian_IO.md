# 文件IO

## 1. 文件描述符

用 open 打开文件的时候会返回一个非负整数，该返回值就是文件描述符，对 Linux 内核而言，所有打开的文件都会通过 文件描述符 进行索引

一个进程可以打开的文件是有上限的（默认是 1024 个）：

```shell
ulimit -n		// 查看进程打开多少个文件数
```

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);
flag: 文件访问模式
O_RDONLY | O_WRONLY | O_RDWR | O_CREAT | O_EXCL
```

分配的文件描述符一般从 3 开始，0、1、2 这三个文件描述符已经默认被系统占用了，分别分配给系统 标准输入（0）、标准输出（1）以及标准错误（2）

------

### 1.1 对文件操作函数

```c
#include <unistd.h>

ssize_t write(int fd, const void *buf, size_t count);
```

```c
#include <unistd.h>

ssize_t read(int fd, void *buf, size_t count);
```

```c
#include <unistd.h>

int close(int fd);
```

读写操作都是从文件的当前位置偏移量处开始，当前位置偏移量可以通过 lseek 系统调用进行设置，默认情况下，当前位置偏移量为0

```c
#include <sys/types.h>
#include <unistd.h>

off_t lseek(int fd, off_t offset, int whence);
offset：偏移量，字节为单位
whence：
	- SEEK_SET 读写偏移量指向 offset 处
	- SEEK_CUR 读写偏移量指向 当前位置偏移量 + offset 字节处
	- SEEK_END 读写位置偏移量指向 文件末尾 + offset 字节位置处
返回值： 错误 -1 从文件头部开始算的位置偏移量
```

## 2. Linux 系统管理文件

### 2.1 静态文件与 inode

![1682436486911](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1682436486911.png)

文件在没有被打开的情况下一般都是存放在硬盘中，一般称为静态文件；硬盘的最小存储单位叫做“扇区”；操作系统在读取硬盘的时候，为了效率，一次性读取多个“扇区”，叫做“块”；“块”是文件存取的最小单位

扇区：硬盘的最小存储单位，每个扇区512字节（0.5KB）

块：文件存取的最小单位，通常 4 KB（8个扇区）

**inode**

磁盘分区分为 数据区 和 inode 区；inode区本质是一个 inode表（inode table），里面存放的是一个个 inode 结构体，每一个文件对应一个 inode，inode 结构体记录了文件相关信息（文件字节大小、文件所有着、文件时间戳）

每一个文件都有唯一的一个 inode，通过数字编号就可以找到 inode table 中对应的 inode

```shell
ls -i // 查看文件 inode 编号，通过inode编号可以在inode表中找到inode
```

系统找文件过程

1. 系统找到文件名对应的 inode 编号；
2. 通过 inode 编号从 inode table 中找到对应的 inode 结构体
3. 根据 inode 结构体中信息，确定数据文件所在的 block，并读出数据

对于操作系统而言，打开文件时，内核会申请一段内存（缓冲区），并将磁盘中的静态文件读取到内存进行管理、缓存（内存里的这份文件叫 动态文件/内核缓冲区），读写操作都是针对这份动态文件，而不是直接操作磁盘中的静态文件（因为磁盘块设备读写都是一块块进行的）；对动态文件进行读写操作后，动态文件和静态文件的同步工作是由操作系统来完成的

原因：硬盘等存储设备基本都是 Flash 块设备，硬件本身有读写限制，设备是以块为单位进行读写的；而内存是可以按字节为单位操作的

**PCB 进程控制块**

PCB进程控制块：内核会为每个进程设置一个专门的数据结构用于管理该进程，包含进程状态信息、运动特征

PCB进程控制块内有一指针指向__文件描述符表__，表内每个元素索引到对应文件表，__文件表__是一个记录文件相关信息的数据结构，内部有 i-node 指针（指向该文件的 inode）

![1682436540032](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1682436540032.png)

### 2.2 返回错误处理与 errno

Linux 系统下对常见的错误做了一个编号，当函数执行发生错误时，操作系统会将这个错误编号赋值给 errno（每个进程都维护自己的 errno 全局变量）

```c
/* errno 是在 errno.h 中声明的 */
#include<stdio.h>
#include<errno.h>

int main(void)
{
	printf("%d\n", errno);
	return 0;
}
```

------

```c
#include<string.h>
// 打印错误编号的字符串信息
char *strerrno(int errnum);
errnum：错误编号，errno
```

```c
#include<stdio.h>
// 自动获取 errno，并打印错误信息
void perror(const char *s)
s:在打印错误信息之前，可加入自己的打印信息
```

### 2.3 程序退出（进程退出）

Linux ，进程退出可以分为正常退出和异常退出（异常表示出现不可意料的系统异常，比如执行程序的时候收到了信号）；进程正常退出 return 、 exit、_exit、_Exit

main函数中：

- return 执行后会把控制权交给调用函数，结束该进程
- _exit 会清除其使用的内存空间，并销毁其在内核中的各种数据结构，关闭进程的所有文件描述符，并结束进程，将控制权交给操作系统
- _Exit 等价于 _exit
- exit 标准C库函数（<stdlib.h>），\_exit、_Exit 系统调用；exit 会在执行一些清理工作，再调用 _exit() 函数

```c
#include<unistd.h>

void _exit(int status);    // 需要传入状态参数
status: 0 正常退出； 其他：有错误发生
------------------
#include<stdlib.h>
// 等价 _exit()
void _Exit(int status);
----------------------
#include<stdlib.h>
void exit(int status);
```

**空洞文件**
文件空洞部分实际上不会占用任何物理空间，直到对空洞部分进行写入才会分配对应的空间

空洞文件对多线程共同操作文件是及其有用的，每个线程负责其中一段数据的写入

**open 中的 O_TRUNC 和 O_APPEND 标志**

O_TRUNC：将文件原本的内容全部丢弃，文件大小变为0

O_APPEND：使用 write 写操作时候，会自动把文件当前位置偏移量移动到文件末尾，从文件末尾开始写数据；不会改变读的位置；lseek 也无法影响写操作的位置；实现定位到文件末尾和写的一个原子操作

### 2.4 多次打开同一文件

![1688375293479](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1688375293479.png)

- 一个进程内多次 open 打开同一个文件，会得到多个不同的文件描述符 fd，同理在关闭文件的时候也需要 close 依次关闭各个文件描述符
- 一个进程内多次 open 打开同一文件，在内存中并不会存在多份动态文件
- 一个进程内多次 open 打开同一文件，不同文件描述符所对应的读写位置偏移量是相互独立的

### 2.5 复制文件描述符

Linux 系统中，可以对 open 打开文件返回的 文件描述符 fd 进行复制；复制得到的文件描述符都指向了同一个文件表

![1688375555798](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1688375555798.png)

```c
#include<unistd.h>

int dup(int oldfd);
返回值：成功：返回复制新的文件描述符 失败：-1
```

```c
#include<unistd.h>

int dup2(int oldfd, int newfd);
返回值：成功：newfd  失败:-1
```

文件共享实现方式：

- 同一个进程中多次调用 open 函数打开同一个文件 
- 不同进程中分别使用 open 函数打开同一个文件
- 同一个进程中通过 dup（dup2）函数对文件描述符进行复制

### 2.6 原子操作和竞争冒险

**O_APPEND 实现原子操作**

O_APPEND 可以实现移动位置偏移量和写的原子操作，每次执行 write 写操作时都会将文件当前读写位置偏移量移动到文件末尾，然后再写入数据

**pread() pwrite() 系统调用**

pread() 和 pwrite() 系统调用相当于调用 lseek 之后再 read、write

```c
/* 将读写位置移动和读写操作合并为一个原子操作(系统调用) */
#include<unistd.h>

ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
- pread 期间无法中断其定位和读操作
- 不更新文件表中的当前位置偏移量
```

### 2.7 fcntl 对一个打开的文件描述符执行一系列的控制操作

比如 复制一个文件描述符、获取/设置文件描述符标志、获取/设置文件状态标志 等

```c
#include<unistd.h>
#include<fcntl.h>

int fcntl(int fd, int cmd, ...)
- 复制文件描述符（cmd=F_DUPFD 或 cmd=F_DUPFD_CLOEXEC）；
- 获取/设置文件描述符标志（cmd=F_GETFD 或 cmd=F_SETFD）；
- 获取/设置文件状态标志（cmd=F_GETFL 或 cmd=F_SETFL）；
- 获取/设置异步 IO 所有权（cmd=F_GETOWN 或 cmd=F_SETOWN）；
- 获取/设置记录锁（cmd=F_GETLK 或 cmd=F_SETLK）；
返回值： 失败：-1  成功：具体与cmd有关
```

```c
/* 复制文件描述符 */
fd2 = fcntl(fd1, F_DUPFD, 0);

/* 获取/设置文件状态标志 */
flag = fcntl(fd, F_GETFL);
ret = fcntl(fd, F_SETFL, flag | O_APPEND);
```

### 2.8 ioctl 文件 io 操作的杂物箱

```c
#include <sys/ioctl.h>

int ioctl(int fd, unsigned long request, ...);
返回值： 成功0 失败-1
```

### 2.8 文件截断

系统调用 truncate 和 ftruncate 可将普通文件截断为指定字节长度

文件目前的大小大于参数 length 所指定的大小，则多余的数据将被丢失；文件目前的大小小于参数 length 所指定的大小，则将其进行扩展， 对扩展部分进行读取将得到空字节"\0"。 

```c
#include<unistd.h>
#include<sys/types.h>

int truncate(const char *path, off_t length);
int ftruncate(int fd, off_t length);
```

文件截断不会导致文件读写位置偏移量发生改变，所以截断之后一般需要重新设置文件当前的读写位置偏移量



