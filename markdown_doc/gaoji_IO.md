# 高级IO

## 1. 非阻塞IO

阻塞概念：进程获取不到资源，从而进入休眠状态，交出 CPU 控制权

> 阻塞式I/O：对文件的 IO 操作都是阻塞式的
>
> 非阻塞式I/O：对文件的 IO 操作都是非阻塞式的

阻塞式IO和非阻塞式IO的优缺点：

> 阻塞式IO，对文件进行IO时，无数据可读，对将调用者应用程序挂起，进入休眠阻塞状态，直到有数据的时候才会解除阻塞（无法实现并发读取）
>
> 非阻塞式IO，应用程序不会被挂起，立即返回，要么配合轮询一直等待，要么放弃（一直轮询会造成CPU负载过高）

设置文件非阻塞的方式：

```c
/* 1. 文件打开指定非阻塞 */
fd = open("/dev/input/event3", O_RDONLY | O_NONBLOCK);

/* 2. 利用 fcntl 来实现文件的非阻塞设置 */
int flag;

flag = fcntl(0, F_GETFL); 	//先获取原来的 flag
flag |= O_NONBLOCK; 		//将 O_NONBLOCK 标志添加到 flag
fcntl(0, F_SETFL, flag); 	//重新设置 flag
```

## 2. I/O 多路复用

- 解决非阻塞轮询CPU占用率高的问题，进程通过系统调用 select() 和 poll() 来主动查询文件描述符上是否可以执行 IO 操作
- I/O 多路复用通过一种机制，可以监视多个文件描述符，一旦某个文件描述符可以执行 I/O 操作时，能够通知应用程序进行相应的读写操作；常用于并发式非阻塞 I/O
- I/O 多路复用特点：外部阻塞式，内部监视多路 I/O
- 内部实验原理是通过轮询的方式来检查多个文件描述符是否可执行IO操作

### 2.1 select

调用 select() 会阻塞，直到某一个或多个文件描述符称为就绪态（可以读或写）

```c
#include <sys/select.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
- nfds:通常表示最大文件描述符编号值加 1，readfds、writefds 以及 exceptfds 这三个文件描述符集合最大描述符 +1
- readfds：用来检测读是否就绪（是否可读）的文件描述符集合；对该事件不感兴趣可以设置为 NULL
- writefds：是用来检测写是否就绪（是否可写）的文件描述符集合
- exceptfds：是用来检测异常情况是否发生的文件描述符集合
- timeout：设定 select 阻塞的时间上限，控制 select 的阻塞行为；NULL 一直阻塞；0立刻返回

返回值：
- 返回-1 表示有错误发生，并且会设置 errno
- 返回 0 表示在任何文件描述符成为就绪态之前 select()调用已经超时
- 返回一个正整数表示有一个或多个文件描述符已达到就绪态
```

```c
#include <sys/select.h>

void FD_ZERO(fd_set *set);				// 将参数 set 所指向的集合初始化为空
void FD_SET(int fd, fd_set *set);		// 将文件描述符 fd 添加到参数 set 所指向的集合中
void FD_CLR(int fd, fd_set *set);		// 将文件描述符 fd 从参数 set 所指向的集合中移除
int FD_ISSET(int fd, fd_set *set);		// 判断文件描述符 fd 是参数 set 所指向的集合中的成员
```

select()函数将阻塞知道有以下事情发生： 

- readfds、 writefds 或 exceptfds 指定的文件描述符中至少有一个称为就绪态； 
- 该调用被信号处理函数中断； 
- 参数 timeout 中指定的时间上限已经超时 

### 2.2 poll

```c
#include <poll.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout);
fds：结构体，里面包含每个元素都会指定一个文件描述符以及我们对该文件描述符所关心的条件
nfds：参数 nfds 指定了 fds 数组中的元素个数
timeout：阻塞行为
	- -1：poll()会一直阻塞
	- 0： poll()不会阻塞，只是执行一次检查看看哪个文件描述符处于就绪态
	
struct pollfd {
	int fd; 		/* file descriptor */
	short events; 	/* requested events */
	short revents; 	/* returned events */
};
- 调用者初始化 events 来指定需要为文件描述符 fd 做检查的事件
- 当 poll()函数返回时， revents 变量由 poll()函数内部进行设置，用于说明文件描述符 fd 发生了哪些事件

POLLIN/POLLOUT/POLLERR

返回值：
	- -1：表示有错误发生，并且会设置 errno。
    - 0：该调用在任意一个文件描述符成为就绪态之前就超时了
```

## 3. 异步 I/O（信号驱动 I/O）

当文件描述符上可执行 I/O 操作时，内核会向进程发送一个信号，当进程接收到信号时，会执行预先注册好的信号处理函数，可以在信号处理函数中进行 I/O 操作

> 使用异步I/O，需要进行步骤：
>
> 1. 通过指定 O_NONBLOCK 标志使能非阻塞 I/O
> 2. 通过指定 O_ASYNC 标志使能异步 I/O 
> 3. 设置异步 I/O 事件的接收进程 
> 4. 为内核发送的通知信号注册一个信号处理函数 

当 I/O 操作就绪时，内核会向进程发送一个 SIGIO 信号，当进程接收到信号时，会执行预先注册好的信号处理函数，我们就可以在信号处理函数中进行 I/O 操作 

```c
/* 2. O_ASYNC 标志使能异步 I/O (只能通过 fcntl 方式进行) */
int flag;

flag = fcntl(0, F_GETFL); 		//先获取原来的 flag
flag |= O_ASYNC; 			//将 O_ASYNC 标志添加到 flag
fcntl(fd, F_SETFL, flag); 	//重新设置 flag
```

```c
/* 3. 设置异步 I/O 事件的接收进程 */
fcntl(fd, F_SETOWN, getpid());
```

```c
/* 4. 注册 SIGIO 信号的处理函数 */
static void sigio_handler(int sig)
{
	/* 数据接收工作（read等） */
}

int main()
{
	signal(SIGIO, sigio_handler);
}
```

异步 I/O 存在问题：

1. 默认的异步 I/O 通知信号 SIGIO 是非排队信号（非实时信号、不可靠信号）
2.  无法得知文件描述符发生了什么事件

### 3.1 优化异步 I/O

针对异步 I/O 的两大问题提出改进：实时信号+sigaction

```c
/* 将异步IO通知信号换成实时信号 */
// 需要定义_GNU_SOURCE宏之后才能使用 F_SETSIG
fcntl(fd, F_SETSIG, SIGRTMIN);				// 换成了 SIGRTMIN
fcntl(fd, F_SETSIG, 0);					// 设置回默认信号 SIGIO 信号
```

使用 sigaction 函数进行注册，并为 sa_flags 参数指定 SA_SIGINFO， 表示使用 sa_sigaction 指向的函数作为信号处理函数

函数参数中包括一个 siginfo_t 指针，当触发信号时该对象由内核构建 

```c
#define _GNU_SOURCE         // 定义了_GNU_SOURCE宏之后才能使用 F_SETSIG

static void io_handler(int sig, siginfo_t *info, void *context)
{
	if(SIGRTMIN != sig)
        return;
    /* 判断是否可读 */
    if (POLL_IN == info->si_code) 
    {
        read(fd, buf, sizeof(buf));
    }
}

int main(void)
{
    struct sigaction act;
    int flag;
    
    fd = open(MOUSE, O_RDONLY | O_NONBLOCK);
    
    /* 使能异步 I/O */
    flag = fcntl(fd, F_GETFL);
    flag |= O_ASYNC;
    fcntl(fd, F_SETFL, flag);
    
    /* 设置异步 I/O 的所有者 */
    fcntl(fd, F_SETOWN, getpid());
    
    /* 指定实时信号 SIGRTMIN 作为异步 I/O 通知信号 */
    fcntl(fd, F_SETSIG, SIGRTMIN);
    
    /* 为实时信号 SIGRTMIN 注册信号处理函数 */
    act.sa_sigaction = io_handler;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaction(SIGRTMIN, &act, NULL);
    
    while(1){}
}
```

## 4. 存储映射 I/O

- 将一个文件映射到进程地址空间中的一块内存区域中，对内存区域的读写相当于对文件的读写；
- 本质是共享，减少了数据的复制操作，在效率上会比普通I/O要高
- 文件映射的内存区域的大小必须是系统页大小的整数倍，适合处理大量数据

```c
#include <sys/mman.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
- addr：参数 addr 用于指定映射到内存区域的起始地址。通常 NULL 表示由系统选择该映射区的起始地址
- length：参数 length 指定映射长度，字节为单位
- offset：文件映射的偏移量，通常将其设置为 0，表示从文件头部开始映射；
- fd：文件描述符，指定要映射到内存区域中的文件
- prot： 参数 prot 指定了映射区的保护要求
	- PROT_EXEC： 映射区可执行；
	- PROT_READ： 映射区可读；
	- PROT_WRITE： 映射区可写；
	- PROT_NONE： 映射区不可访问
- flags：参数 flags 可影响映射区的多种属性
	- MAP_SHARED：此标志指定当对映射区写入数据时，数据会写入到文件中，并且允许其它进程共享
	- MAP_PRIVATE： 此标志指定当对映射区写入数据时，会创建映射文件的一个私人副本（copy-onwrite）
	
返回值：
	函数的返回值便是映射区的起始地址；发生错误时，返回(void *)-1
```

![1692778224700](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1692778224700.png)

- addr 和 offset 的值通常被要求是系统页大小的整数倍；但对于参数 length 长度来说，却没有这种要求，系统映射时候会自动拓展到系统页的整数倍，之后面部分为0，不会影响到源文件

```c
#include <sys/mman.h>

int munmap(void *addr, size_t length);			// 解除映射
```

mprotect() 可以更改一个现有映射区的保护要求

```c
#include <sys/mman.h>

int mprotect(void *addr, size_t len, int prot);
```

写入到文件映射区中的数据也不会立马刷新至磁盘设备中，而是会在我们将数据写入到映射区之后的某个时刻将映射区中的数据写入磁盘中。 所以会导致映射区中的内容与磁盘文件中的内容不同步 

调用 msync()函数将映射区中的数据刷写、更新至磁盘文件中（同步操作） 

```c
#include <sys/mman.h>

int msync(void *addr, size_t length, int flags);			// 同步映射
- flag：应指定为 MS_ASYNC 和 MS_SYNC 两个标志之一
	- MS_ASYNC：以异步方式进行同步操作
	- MS_SYNC： 以同步方式进行同步操作
	- MS_INVALIDATE：（可选） 请求使同一文件的其它映射无效（以便可以用刚写入的新值更新它们）
```

## 5. 文件锁

确保只有本进程能够对某一文件进行 I/O 操作，在这段时间内不允许其它进程对该文件进行 I/O 操作 

linux 通常采用的方法是对文件上锁， 来避免多个进程同时操作同一文件时产生竞争状态。譬如进程对文件进行 I/O 操作时，首先对文件进行上锁，将其锁住，然后再进行读写操作；只要进程没有对文件进行解锁，那么其它的进程将无法对其进行操作 

> - 建议性锁：建议上锁后在对文件进行访问，若在没有对文件上锁的情况下直接访问文件，也是可以访问的，并非无法访问文件
> - 强制性锁：其它的进程在没有获取到文件锁的情况下是无法对文件进行访问的；强制性锁会让内核检查每一个 I/O 操作，验证调用进程是否是该文件锁的拥有者，如果不是将无法访问文件 

### 5.1 flock() 加锁（建议锁）

```c
#include <sys/file.h>

int flock(int fd, int operation);
fd：参数 fd 为文件描述符，指定需要加锁的文件
operation：指定了操作方式
	- LOCK_SH：文件上放置一把共享锁，指的便是多个进程可以拥有对同一个文件的共享锁，该共享锁可被多个进程同时拥有
	- LOCK_EX：排它锁（或叫互斥锁） 。 所谓互斥，指的便是互斥锁只能同时被一个进程所拥有
	- LOCK_UN：解除文件锁定状态，解锁、释放锁
	- LOCK_NB：表示以非阻塞方式获取锁。默认情况下，调用 flock()无法获取到文件锁时会阻塞、 直到其它进程释放锁为止， 如果不想让程序被阻塞，可以指定 LOCK_NB 标志， 如果无法获取到锁应立刻返回

返回值：
	成功将返回 0；失败返回-1、并会设置 errno，
```

- 同一个文件不会同时具有共享锁和互斥锁 
- 同一进程对文件多次加锁不会导致死锁；新加的锁会覆盖旧的锁
- 文件关闭的时候，会自动解锁 
- 一个进程不可以对另一个进程持有的文件锁进行解锁 
- 由 fork()创建的子进程不会继承父进程所创建的锁 
- 当一个文件描述符被复制时（dup 操作） ，这些通过复制得到的文件描述符和源文件描述符都会引用同一个文件 

### 5.2 fcntl() 加锁（支持建议性的锁和强制性的锁，分共享性质读锁和独占性的写锁）

- fcntl()可以对文件的某个区域（某部分内容）进行加锁/解锁，可以精确到某一个字节数据 
- fcntl() 可支持建议性锁和强制性锁两种类型 

```
#include <unistd.h>
#include <fcntl.h>

int fcntl(int fd, int cmd, ... /* struct flock *flockptr */ );
cmd：F_SETLK、 F_SETLKW、 F_GETLK
- F_SETLK：对文件添加由 flockptr 指向的 struct flock 对象所描述的锁；如果加锁失败，将立即出错返回，此时将 errno 设置为 EACCES 或 EAGAIN。也可用解锁（l_type 等于 F_UNLCK）
- F_SETLKW：F_SETLK 的阻塞版本；如果所请求的读锁或写锁因另一个进程当前已经对所请求区域的某部分进行了加锁，而导致请求失败，那么调用进程将会进入阻塞状态；只有当请求的锁可用时，进程才会被唤醒
- F_GETLK：测试调用进程对文件加一把由参数 flockptr 所描述的锁是否会加锁成功；不成功，现有锁的信息将会重写参数 flockptr


struct flock {
	...
	short l_type; 	/* Type of lock: F_RDLCK,F_WRLCK, F_UNLCK */
	short l_whence; 	/* How to interpret l_start: SEEK_SET, SEEK_CUR, SEEK_END */
	off_t l_start; 	/* Starting offset for lock */
	off_t l_len; 	/* Number of bytes to lock */
	pid_t l_pid; 	/* PID of process blocking our lock(set by F_GETLK and F_OFD_GETLK) */
	...
};
- l_type：所希望的锁类型
	- F_RDLCK：共享性质的读锁
	- F_WRLCK：独占性质的写锁
	- F_UNLCK：解锁一个区域	
- l_whence 和 l_start： 这两个变量用于指定要加锁或解锁区域的起始字节偏移量，同 lseek()函数中的 offset 和whence 参数
- l_len： 需要加锁或解锁区域的字节长度
- l_pid： 一个 pid，指向一个进程，表示该进程持有的锁能阻塞当前进程，当 cmd=F_GETLK 时有效
```

注意事项：

- 锁区域可以在当前文件末尾处开始或者越过末尾处开始，但是不能在文件起始位置之前开始 
- 若参数 l_len 设置为 0，表示将锁区域扩大到最大范围，也就是说从锁区域的起始位置开始， 到文件的最大偏移量处（也就是文件末尾）都处于锁区域范围内。而且是动态的
- 如果我们需要对整个文件加锁，可以将 l_whence 和 l_start 设置为指向文件的起始位置， 并且指定
  参数 l_len 等于 0 

读锁和写锁相关：

- 任意多个进程在一个给定的字节上可以有一把共享的读锁，但是在一个给定的字节上只能有一个进程有一把独占写锁 
- 加读锁时，调用进程必须对该文件有读权限 
- 对文件的某一区域加写锁时，调用进程必须对该文件有写权限 

![1692779978855](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1692779978855.png)

注意事项：

- 文件关闭的时候，会自动解锁 
- 一个进程不可以对另一个进程持有的文件锁进行解锁。 
- 由 fork()创建的子进程不会继承父进程所创建的锁 
- 当一个文件描述符被复制时（ dup 操作）， 这些通过复制得到的文件描述符和源文件描述符都会引用同一个文件锁 

强制性锁设置

- 对于一个特定的文件，开启它的强制性锁机制其实非常简单，主要跟文件的权限位有关系，如果要开启强制性锁机制，需要设置文件的 Set-GroupID（S_ISGID）位为 1，并且禁止文件的组用户执行权限（S_IXGRP），也就是将其设置为 0 

```c
fchmod(fd, (sbuf.st_mode & ~S_IXGRP)| S_ISGID)
```

### 5.3 lockf() 加锁

lockf() 是一个库函数，内部是基于 fcntl() 来实现的



