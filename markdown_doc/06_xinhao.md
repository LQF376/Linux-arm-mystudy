# 信号

## 1. 信号概念

信号是事件发生时对进程的通知机制，也可以把它称为软件中断。 只是软件上对中断机制的模拟，无法预测信号达到的准确时间，提供一种处理异步事件的方法

产生信号的情况：

1. 硬件发生异常，即硬件检测到错误条件并通知内核 
2. 终端下输入了能够产生信号的特殊字符 
3. 进程调用 kill()系统调用可将任意信号发送给另一个进程或进程组 
4. 通过 kill 命令将信号发送给其它进程 
5. 发生了软件事件，即当检测到某种软件条件已经发生 

进程对信号处理方式：

1. 忽略信号。 SIGKILL 和 SIGSTOP  不能被忽略
2. 捕获信号。  当信号到达进程后，执行预先绑定好的信号处理函数 
3. 执行系统默认操作。 进程不对该信号事件作出处理，而是交由系统进行处理， 

信号本质上是 int 类型数字编号 

```c
#include<signum.h> //定义了信号，信号从1开始
```

## 2. 信号的分类

不可靠信号：信号可能丢失（处理信号时，又来了新的信号，则导致信号丢失）34号之前的信号都是不可靠信号

```shell
kill -l  # 查看所有信号（64种）
```

可靠信号：支持排队，不会丢失

实时信号：都支持排队，都是可靠信号 ；实时信号保证了发送的多个信号都能被接收， 

非实时信号：不支持排队，都是不可靠信号 （标准信号）

## 3. 常见信号与默认行为

![1682428849044](E:\typora\markdownImage\1682428849044.png)

## 3. 设置信号处理方式

### 3.1 signal

```c
#include <signal.h>

typedef void (*sig_t)(int);
sig_t signal(int signum, sig_t handler);
signum:信号名（宏）或信号的数字编号
handler:指向信号对应的信号处理函数
	handler可以自定义，也可以设置
		 SIG_IGN：表示此进程需要忽略该信号
		 SIG_DFL：系统默认操作
返回值：sig_t 函数指针 成功：指向信号处理函数 失败SIG_ERR
		 
static void sig_handler(int sig)
{...}
ret = signal(SIGINT, (sig_t)sig_handler);
```

两种不同状态下信号的处理方式 

- 程序启动 ：没有调用 signal()函数，执行系统默认操作
- 进程创建 ：子进程将会继承父进程的信号处理方式 

### 3.2 sigaction 相比 signal 更复杂更富有灵活性

```c
#include <signal.h>

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
act:执行 sigaction 结构体，NULL 表示不改变信号当前处理方式
oldact:信号之前的处理方式等信息通过参数 oldact 返回出来
返回值：成功 0 ； 失败 -1
```

```c
struct sigaction {
	void (*sa_handler)(int);
	void (*sa_sigaction)(int, siginfo_t *, void *);
	sigset_t sa_mask;
	int sa_flags;
	void (*sa_restorer)(void);
};

sa_handler:指定信号处理函数，
sa_sigaction：也用于指定信号处理函数，与sa_handler互斥；该处理函数可以带参数信息 siginfo_t
sa_mask：定义了一组信号，当进程在执行由 sa_handler 所定义的信号处理函数之前，会先将这组信号添加到进程的信号掩码字段中，当进程执行完处理函数之后再恢复信号掩码，将这组信号从信号掩码字段中删除。 可实现（在执行信号处理函数期间不希望被另一个信号所打断）通过信号掩码来实现， 如果进程接收到了信号掩码中的这些信号，那么这个信号将会被阻塞暂时不能得到处理，直到这些信号从进程的信号掩码中移除。 在信号处理函数调用时，进程会自动将当前处理的信号添加到信号掩码字段中，这样保证了在处理一个给定的信号时，如果此信号再次发生，那么它将会被阻塞。
sa_flags：标志；用于控制信号的处理过程
	SA_NOCLDSTOP： 如果signum为 SIGCHLD，则子进程停止时（即当它们接收到 SIGSTOP、SIGTSTP、SIGTTIN 或SIGTTOU中的一种时）或恢复（即它们接收到 SIGCONT）时不会收到 SIGCHLD 信号
	SA_NOCLDWAIT：如果 signum 是 SIGCHLD，则在子进程终止时不要将其转变为僵尸进程。
	SA_NODEFER：不要阻塞从某个信号自身的信号处理函数中接收此信号。 
	SA_RESETHAND：执行完信号处理函数之后，将信号的处理方式设置为系统默认操作。
	SA_RESTART：被信号中断的系统调用，在信号处理完成之后将自动重新发起。
	SA_SIGINFO：表示使用 sa_sigaction 作为信号处理函数、而不是 sa_handler，
```

```c
siginfo_t {
	int si_signo; /* Signal number */
	int si_errno; /* An errno value */
	int si_code; /* Signal code */
	int si_trapno; /* Trap number that caused hardware-generated signal(unused on most architectures) */
	pid_t si_pid; /* Sending process ID */
	uid_t si_uid; /* Real user ID of sending process */
	int si_status; /* Exit value or signal */
	clock_t si_utime; /* User time consumed */
	clock_t si_stime; /* System time consumed */
	sigval_t si_value; /* Signal value */
	int si_int; /* POSIX.1b signal */
	void *si_ptr; /* POSIX.1b signal */
	int si_overrun; /* Timer overrun count; POSIX.1b timers */
	int si_timerid; /* Timer ID; POSIX.1b timers */
	void *si_addr; /* Memory location which caused fault */
	long si_band; /* Band event (was int in glibc 2.3.2 and earlier) */
	int si_fd; /* File descriptor */
	short si_addr_lsb; /* Least significant bit of address(since Linux 2.6.32) */
	void *si_call_addr; /* Address of system call instruction(since Linux 3.5) */
	int si_syscall; /* Number of attempted system call(since Linux 3.5) */
	unsigned int si_arch; /* Architecture of attempted system call(since Linux 3.5) */
}
```

```c
static void sig_handler(int sig)
{....}

struct sigaction sig = {0};
sig.sa_handler = sig_handler;
sig.sa_flags = 0;
ret = sigaction(SIGINT, &sig, NULL);
```

## 4. 向进程发信号

```c
#include <sys/types.h>
#include <signal.h>

int kill(pid_t pid, int sig);
pid：接收信号进程pid
	整数：发送到 pid 指定的进程
	= 0：sig 发送到当前进程的进程组中的每个进程。
	-1: 将 sig 发送到当前进程有权发送信号的每个进程
	< -1：将 sig 发送到 ID 为-pid 的进程组中的每个进程。
```

------

向自身发送信号

```c
#include <signal.h>

int raise(int sig);
返回值：成功0 失败非零值
```

## 5. 闹钟和设置进程进入睡眠

设置一个定时器（闹钟） ，当定时器定时时间到时，内核会向进程发送 SIGALRM 信号；SIGALRM 信号的系统默认操作是终止进程

```c
#include <unistd.h>

unsigned int alarm(unsigned int seconds);
seconds：设置时间（单位为秒）；0 表示取消之前设置的 alarm 闹钟
返回值：如果有设置上一次闹钟，返回上一次设置闹钟的剩余值；否则返回0
```

使得进程暂停运行、进入休眠状态，直到进程捕获到一个信号为止，只有执行了信号处理函数并从其返回时， pause()才返回， 在这种情况下， pause()返回-1，并且将 errno 设置为 EINTR 

```c
#include <unistd.h>

int pause(void);
```

## 6. 信号集

表示多个信号（一组信号）的数据类型---信号集（signal set），其实就是 sigset_t 结构体；Linux提供信号集的API

### 6.1 初始化信号集

```c
#include <signal.h>
/* 两种都是初始化信号集 */
int sigemptyset(sigset_t *set);	// 清空所有信号
int sigfillset(sigset_t *set);	// 包含所有信号
set：信号集变量
返回值： 成功0 失败-1
```

### 6.2 向信号集中添加/删除信号

```c
#include <signal.h>

int sigaddset(sigset_t *set, int signum);	// 添加信号
int sigdelset(sigset_t *set, int signum);	// 删除信号
set：信号集
signum：添加/删除的信号
```

### 6.3 测试信号是否在信号集中

```c
#include <signal.h>

int sigismember(const sigset_t *set, int signum);
返回值： 在信号集中，返回1； 不在信号集中，返回0；失败 -1
```

### 6.4 获取信号的描述信息

```c
sys_siglist[signum]	// 字符串数组，存放信息

#include <string.h>
char *strsignal(int sig);	// 或许字符串描述信息
```

### 6.5 psignal

在标准错误（stderr）上输出信号描述信息

```c
#include <signal.h>

void psignal(int sig, const char *s);
```

## 7. 信号掩码(阻塞信号传递) 

内核为每一个进程维护了一个信号掩码（其实就是一个信号集） ，即一组信号。 当进程接收到一个属于信号掩码中定义的信号时，该信号将会被阻塞、无法传递给进程进行处理， 那么内核会将其阻塞，直到该信号从信号掩码中移除，内核才会把该信号传递给进程从而得到处理。 

- 当应用程序调用 signal()或 sigaction()函数为某一个信号设置处理方式时，进程会自动将该信号添加到信号掩码中
- 使用 sigaction()函数为信号设置处理方式时，可以额外指定一组信号，当调用信号处理函数时将该组信号自动添加到信号掩码中， 当信号处理函数结束返回后，再将这组信号从信号掩码中移除；
- 可以使用 sigprocmask()系统调用，随时可以显式地向信号掩码中添加/移除信号。 

```c
#include <signal.h>

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
how:指定了调用函数时的一些行为。
	SIG_BLOCK：set 所指向的信号集内的所有信号添加到进程的信号掩码中
	SIG_UNBLOCK：将参数 set 指向的信号集内的所有信号从进程信号掩码中移除。
	SIG_SETMASK：进程信号掩码直接设置为参数 set 指向的信号集。
set: set 指向的信号集内的所有信号添加到信号掩码中或者从信号掩码中移除;NULL 表示不对当前信号掩码做操作
oldset：添加新的信号之前，获取到进程当前的信号掩码，
返回值：成功0   失败-1
```

## 8. 阻塞等待信号 sigsuspend 

将恢复信号掩码和 pause()挂起进程这两个动作封装成一个原子操作

将参数 mask 所指向的信号集来替换进程的信号掩码，也就是将进程的信号掩码设置为参数 mask 所指向的信号集，然后挂起进程，直到捕获到信号被唤醒（如果捕获的信号是 mask 信号集中的成员，将不会唤醒、继续挂起） 、并从信号处理函数返回，一旦从信号处理函数返回， sigsuspend()会将进程的信号掩码恢复成调用前的值。 

```c
#include <signal.h>
int sigsuspend(const sigset_t *mask);
mask:恢复的信号掩码
返回值：始终返回-1  EINTR，表示被信号所中断； EFAULT，调用失败
```

## 9. 实时信号

### 9.1 确定进程中处于等待状态的是哪些信号

```c
#include <signal.h>

int sigpending(sigset_t *set);
set：处于等待状态的信号 放在信号集里面
返回值：成功0 失败 -1
```

### 9.2 发送实时信号 

实时信号的优点

- 实时信号的信号范围有所扩大，可应用于应用程序自定义的目的；标准信号仅提供了两个信号可用于应用程序自定义使用： SIGUSR1 和 SIGUSR2。 
- 内核对于实时信号所采取的是队列化管理。 如果将某一实时信号多次发送给另一个进程，那么将会多次传递此信号。
- 当发送一个实时信号时，可为信号指定伴随数据（一整形数据或者指针值），供接收信号的进程在它的信号处理函数中获取。 
- 不同实时信号的传递顺序得到保障。如果有多个不同的实时信号处于等待状态，那么将率先传递具有最小编号的信号。 

Linux 内核定义了 31 个不同的实时信号，信号编号范围为 34~64； SIGRTMIN 编号最小的实时信号， SIGRTMAX 编号最大

应用程序当中使用实时信号，需要有以下的两点要求： 

- 发送进程使用 sigqueue()系统调用向另一个进程发送实时信号以及伴随数据。 
- 接收实时信号的进程要为该信号建立一个信号处理函数，使用 sigaction函数为信号建立处理函数，并加入 SA_SIGINFO，这样信号处理函数才能够接收到实时信号以及伴随数据，也就是要使用 sa_sigaction 指针指向的处理函数， 

```c
#include <signal.h>

int sigqueue(pid_t pid, int sig, const union sigval value);	// 发送实时信号
sig: 0 用于检查参数 pid 所指定的进程是否存在
value:信号的伴随数据
返回值：成功 0 失败 -1
```

```c
typedef union sigval
{
	int sival_int;		// 整数
	void *sival_ptr;	// 指针
} sigval_t;
```

```c
sigqueue(pid, sig, sig_val)
--------------
static void sig_handler(int sig, siginfo_t *info, void *context)
{
	sigval_t sig_val = info->si_value;
}

struct sigaction sig = {0};
sig.sa_sigaction = sig_handler;
sig.sa_flags = SA_SIGINFO;
sigaction(num, &sig, NULL);
```

## 10. 异常退出 abort()函数 

abort()终止进程运行，会生成核心转储文件 

abort()通常产生 SIGABRT 信号来终止调用该函数的进程， SIGABRT 信号的系统默认操作是终止进程运行、并生成核心转储文件；当调用 abort()函数之后，内核会向进程发送 SIGABRT 信号 

```c
#include <stdlib.h>

void abort(void);
```

