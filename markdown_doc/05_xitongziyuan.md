# 系统信息与系统资源

## 1. 系统信息

### 1.1 系统标识 uname

uname 用于获取当前操作系统内核的名称和信息

```c
#include<sys/utsname.h>

int uname(struct utsname *buf);
buf：utsname 结构体参数，传出参数；用于记录系统相关信息
```

```c
struct utsname {
	char sysname[]; 	/* 当前操作系统的名称 */
	char nodename[]; 	/* 网络上的名称（主机名） */
	char release[]; 	/* 操作系统内核版本 */
	char version[]; 	/* 操作系统发行版本 */
	char machine[]; 	/* 硬件架构类型 */
	#ifdef _GNU_SOURCE
		char domainname[];	/* 当前域名 */
	#endif
};
```

### 1.2 sysinfo 函数

用于获取一些系统统计信息；系统调用

```c
#include <sys/sysinfo.h>

int sysinfo(struct sysinfo *info);
返回值：成功 0；失败 -1，设置 errno
```

```c
struct sysinfo {
	long uptime; /* 自系统启动之后所经过的时间（以秒为单位） */
	unsigned long loads[3]; /* 1, 5, and 15 minute load averages */
	unsigned long totalram; /* 总的可用内存大小 */
	unsigned long freeram; /* 还未被使用的内存大小 */
	unsigned long sharedram; /* Amount of shared memory */
	unsigned long bufferram; /* Memory used by buffers */
	unsigned long totalswap; /* Total swap space size */
	unsigned long freeswap; /* swap space still available */
	unsigned short procs; /* 系统当前进程数量 */
	unsigned long totalhigh; /* Total high memory size */
	unsigned long freehigh; /* Available high memory size */
	unsigned int mem_unit; /* 内存单元大小（以字节为单位） */
	char _f[20-2*sizeof(long)-sizeof(int)]; /* Padding to 64 bytes */
};
```

### 1.3 gethostname 函数

单独获取 Linux 系统主机名

```c
#include <unistd.h>

int gethostname(char *name, size_t len);
name：指向用于存放主机名字字符串的缓冲区
len：缓冲区长度
返回值：成功 0；失败 -1 errno
```

### 1.4 sysconf 函数

库函数；在运行时获取系统的一些配置信息（页大小、主机名的最大长度、进程可打开的最大文件数、每个用户ID的最大并发进程数）

```
#include <unistd.h>

long sysconf(int name);
name：指定了要获取系统的哪类配置信息
- _SC_ARG_MAX： exec 族函数的参数的最大长度
- _SC_CHILD_MAX： 每个用户的最大并发进程数，也就是同一个用户可以同时运行的最大进程数
- _SC_HOST_NAME_MAX： 主机名的最大长度。
- _SC_LOGIN_NAME_MAX： 登录名的最大长度
- _SC_CLK_TCK： 每秒时钟滴答数，也就是系统节拍率
- _SC_OPEN_MAX： 一个进程可以打开的最大文件数
- _SC_PAGESIZE： 系统页大小（page size）
- _SC_TTY_NAME_MAX： 终端设备名称的最大长度
返回值：
	name 无效，返回-1，errno 为 EINVAL
	有效，返回相应的配置值
```

## 2. 时间、日期

### 2.1 时间标准

**GMT 时间**

格林威治标准时间；GMT 时间就是英国格林威治当地时间， 也就是零时区（中时区） 所在时间 

**UTC 时间**

世界协调时间（又称世界标准时间、世界统一时间）；都是指格林威治标准时间，只不过 UTC 时间是修正后的 GMT 时间（更加精准 ）

```shell
data		# 查看系统当前的本地时间
data -u 	# 查看 UTC 时间
```

系统的本地时间由时区配置文件 /etc/localtime 定义

```shell
sudo rm -rf localtime #删除原有链接文件
sudo ln -s /usr/share/zoneinfo/EST localtime #重新建立链接文件
```

### 2.2 Linux 中的时间

**实时时钟 RTC**

操作系统中一般有两个时钟：系统时钟（system clock）、实时时钟（Real time clock）

- 系统时钟由系统启动之后由内核来维护，系统关机情况下是不存在的
- 实时时钟一般由 RTC 时钟芯片提供；RTC 芯片有相应的电池为其供电；保证关机下正常

Linux 开机启动后会读取 RTC 硬件获取实时时钟作为系统时钟的初始值，之后内核扁开始维护自己的系统时钟；系统关机时，内核会将系统时钟写入到 RTC 硬件，进行同步操作

**jiffies**

jiffies 是内核中定义的一个全局变量，用来记录系统从启动以来的系统节拍数；内核在编译配置时定义了一个节拍时间（一秒多少个节拍；节拍率）

内核通过 jiffies 来维护系统时钟；系统时钟初始化，其实就是对内核的 jiffies 变量进行初始化操作，将实时时钟转化为 jiffies 节拍数

我们获取系统当前时间，可以使用 jiffies 变量去计算

### 2.3 获取时间 time / gettimeofday

time 获取当前时间，以秒为单位；自  1970-01-01 00:00:00 以来的秒数

```c
#include <time.h>

time_t time(time_t *tloc);
tloc：不是NULL，返回值存储在 tloc 中
返回值：自 1970 以来的时间秒数，失败 -1 errno
```

gettimeofday 获取微妙级时间精度

```c
#include <sys/time.h>

int gettimeofday(struct timeval *tv, struct timezone *tz);
tz：历史产物，直接设置 NULL 即可
tv：timeval 结构体 包含  tv_sec（秒） + tv_usec（微秒）
返回值：成功 0；失败 -1，errno
```

### 2.4 时间转化函数

将 1970 以来的秒数，转化到便于查看的形式

ctime：C库函数，转化为可打印输出的字符串形式

```c
#include <time.h>

char *ctime(const time_t *timep);			// 不可重入版本
char *ctime_r(const time_t *timep, char *buf);	// 可重入函数
```

```c
char tm_str[100] = {0};
time_t tm;

tm = time(NULL);
ctime_r(&tm, tm_str);
printf("%s", tm_str);
```

localtime 可以将得到的秒数 变成 一个 struct tm 结构体，对应本地时间

```c
#include <time.h>

struct tm *localtime(const time_t *timep);
struct tm *localtime_r(const time_t *timep, struct tm *result);
timep：要进行转化的秒数，time gettimeofday 得到
```

```c
struct tm {
	int tm_sec; 	/* 秒(0-60) */
	int tm_min; 	/* 分(0-59) */
	int tm_hour; 	/* 时(0-23) */
	int tm_mday; 	/* 日(1-31) */
	int tm_mon; 	/* 月(0-11) */
	int tm_year; 	/* 年(这个值表示的是自 1900 年到现在经过的年数) */
	int tm_wday; 	/* 星期(0-6, 星期日 Sunday = 0、星期一=1…) */
	int tm_yday; 	/* 一年里的第几天(0-365, 1 Jan = 0) */
	int tm_isdst; 	/* 夏令时 */
};
```

```c
struct tm t;
time_t sec;

sec = time(NULL);
localtime_t(&sec, &t);
```

gmtime 可以把 time_t 时间变成一个 struct tm 结构体表示的时间，得到的时 UTC 国际标准时间

```c
#include <time.h>

struct tm *gmtime(const time_t *timep);
struct tm *gmtime_r(const time_t *timep, struct tm *result);
```

mktime 将 struct tm 结构体转换为 time_t 时间

```c
#include <time.h>

time_t mktime(struct tm *tm);
```

asctime 将 struct tm 表示的分解时间转换为固定格式的字符串

```c
#include <time.h>

char *asctime(const struct tm *tm);
char *asctime_r(const struct tm *tm, char *buf);
返回值：失败 NULL；成功 char *类型指针
```

strftime C库函数；可以将一个 struct tm 变量表示的分解时间转换为为格式化字符串 

```c
#include <time.h>

size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);
s：指向一个缓存区的指针
max：字符串的最大字节数
```

![1688456452168](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1688456452168.png)

### 2.5 设置时间 settimeofday

```c
#include <sys/time.h>

int settimeofday(const struct timeval *tv, const struct timezone *tz);
tv：timeval 结构体指针变量
tz：NULL
返回值：成功 0；失败 -1 errno
```

![1688456951178](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1688456951178.png)

## 3. 进程时间

进程时间：指进程从创建到目前为止这段时间内使用 CPU 资源的时间总数

分为：用户 CPU 时间 / 系统 CPU 时间

- 用户 CPU 时间：进程在用户空间（用户态）下运行所花费的 CPU 时间
- 系统 CPU 时间：进程在内核空间（内核态）下运行所花费的 CPU 时间（系统调用）

### 3.1 times 获取当前进程时间

```c
#include <sys/times.h>

clock_t times(struct tms *buf);
buf：当前进程时间信息存在一个 tms 结构体中
返回值：成功，返回从过去任意的一个时间点（譬如系统启动时间）所经过的时钟节拍数
```

```c
struct tms {
	clock_t tms_utime; /* user time, 进程的用户 CPU 时间, tms_utime 个系统节拍数 */
	clock_t tms_stime; /* system time, 进程的系统 CPU 时间, tms_stime 个系统节拍数 */
	clock_t tms_cutime; /* user time of children, 已死掉子进程的 tms_utime + tms_cutime 时间总和 */
	clock_t tms_cstime; /* system time of children, 已死掉子进程的 tms_stime + tms_cstime 时间总和 */
};
```

### 3.2 clock 函数

返回值描述了 进程使用的 总的 CPU 时间

```c
#include <time.h>

clock_t clock(void);
```

## 4. 产生随机数

随机数与伪随机数

```c
#include <stdlib.h>

int rand(void);
返回值：[0, RAND_MAX] 的随机数
```

srand 为 rand 设置随机数种子

```c
#include <stdlib.h>

void srand(unsigned int seed);

srand(time(NULL))
```

## 5. 休眠

### 5.1 秒级休眠 sleep

C 库函数

```c
#include <unistd.h>

unsigned int sleep(unsigned int seconds);
返回值：若被信号中断则返回剩余的秒数
```

### 5.2 微秒级休眠 usleep

C 库函数

```c
#include <unistd.h>

int usleep(useconds_t usec);
```

### 5.3 高精度休眠 nanosleep

系统调用；支持纳秒级时长设置

```c
#include <time.h>

int nanosleep(const struct timespec *req, struct timespec *rem);
req：设置休眠时间长度
rem：NULLL
返回值：成功休眠时长 0；信号中断 -1，剩余时间返回 rem 中
```

## 6. 申请堆内存

内存资源由操作系统管理、分配；当应用需要内存时，可以向操作系统申请内存，用完再释放内存、归还操作系统

### 6.1 在堆上分配内存 malloc / free

```c
#include <stdlib.h>

void *malloc(size_t size);
size：要分配的内存大小，字节为单位
返回值：失败 NULL
```

```c
#include <stdlib.h>

void free(void *ptr);
```

- 当进程终止时，内核会将其占用的所有内存都返还给操作系统，包括堆内存中 malloc 申请的内存

### 7.2 其他堆上分配内存的方法

C 库还提供了一系列在堆上分配内存的其他函数

```c
/* 动态地分配内存空间并初始化为 0 */
/* 动态地分配 nmemb 个长度为 size 的连续空间 */
#include <stdlib.h>

void *calloc(size_t nmemb, size_t size);
```

### 7.3 分配对齐内存

```c
#include <stdlib.h>

int posix_memalign(void **memptr, size_t alignment, size_t size);
size：分配的空间大小字节数
alignment：分配的内存地址将是 alignment 的整数倍 2^n 数值
*memptr：指向要分配的空间
----------------
int *base = NULL;
posix_memalign((void **)&base, 256, 1024);
-------------------
void *aligned_alloc(size_t alignment, size_t size);
size 必须是参数 alignment 整数倍

void *valloc(size_t size);
使用页大小作为对齐长度

#include <malloc.h>

void *memalign(size_t alignment, size_t size);
size 不必是参数 alignment 整数倍
void *pvalloc(size_t size);
```

## 8. proc 文件系统

虚拟文件系统（本身不存在于磁盘中），以文件系统的方式为应用层访问系统内核提供数据接口；提供系统中进程相关信息；内核运行的状态

### 8.1 proc 文件系统的使用

cat 查看 或者 open read

