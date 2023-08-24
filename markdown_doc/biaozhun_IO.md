# 标准IO

## 1. 标准IO 和 文件IO 的区别

- 标准 I/O 是标准 C 库函数，而文件 I/O 则是 Linux系统调用；
- 标准 I/O 是由文件 I/O 封装而来 
- 标准 I/O 相比于文件 I/O 具有更好的可移植性 （不同操作系统）
- 标准 I/O 是带有缓存的（在用户空间），而文件 I/O 在用户空间是不带有缓存的， 

## 2. FILE 指针

标准 IO 来说，所有的操作都是围绕 FILE * 指针来操作的（类同于文件 IO 里面的 文件描述符 fd）

## 3. 标准输入、标准输出、标准错误

```c
/* Standard file descriptors.(fd 版本) */
#define STDIN_FILENO 0 /* Standard input. */
#define STDOUT_FILENO 1 /* Standard output. */
#define STDERR_FILENO 2 /* Standard error output. */

/* Standard streams.  (FILE 指针版本)*/
extern struct _IO_FILE *stdin; /* Standard input stream. */
extern struct _IO_FILE *stdout; /* Standard output stream. */
extern struct _IO_FILE *stderr; /* Standard error output stream. */
/* C89/C99 say they're macros. Make them happy. */
#define stdin stdin
#define stdout stdout
#define stderr stderr
```

## 4. 打开文件

```c
#include <stdio.h>
FILE *fopen(const char *path, const char *mode);
返回值：成功:FILE指针  失败:NULL
mode:
	r: 只读
	r+: 可读可写
	w: 只写，文件存在并截断文件，不存在创建
	w+：可读可写，存在截断文件，不存在创建
	a: 只写，存在追加内容，不存在创建
	a+: 可读、可写，存在追加，不存在创建
	
int fclose(FILE *stream);
返回值：成功0 失败EOF(-1)
```

## 5. 读文件和写文件

```c
#include <stdio.h>

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
ptr:读写缓冲区
size:数据项的大小（字节）
nmemb:读取数据项的个数
stream:FILE指针
返回值：成功时返回读到的数据项个数
```

## 6. fseek 定位 / fetll 获取位置偏移量

```c
#include<stdio.h>

int fseek(FILE *stream, long offset, int whence);
stream：FILE 指针
offset：类同 lseek
whence：类同 lseek
返回值：成功0，发生错误返回 -1，并设置 errno

long ftell(FILE *stream);
返回值：成功：返回当前读写位置偏移量；失败：-1，并设置 errno
```

## 7. 检查或复位状态

当 fread() 读取数据时，返回值小于参数 nmemb 所指的值，表示发生了错误或者已经到了文件末尾（文件结束 end-of-file），可通过判断错误标志 或者 end-of-file 标志 来确定

### 7.1 feof 测试 stream 所指文件的 end-of-file 标志

如果 end-of-file 标志被设置返回非零值，如果 end-of-file 未被设置，返回 0

```c
#include<stdio.h>

int feof(FILE *stream);
返回值：文件末尾时，返回非零值    end-of-file 未设置：0
```

### 7.2 ferror 测试 stream 所指文件的错误标志

```c
#include <stdio.h>

int ferror(FILE *stream);
返回值：发生IO操作错误时返回非零 没有设置，返回0
```

### 7.3 清除 end-of-file 和 错误标志

```c
#include <stdio.h>

void clearerr(FILE *stream);
// end-of-file 标志，使用fseek重新定位时，也会清除文件的标志
```

## 8. 格式化I/O

### 8.1 格式化输出

```c
#include <stdio.h>

int printf(const char *format, ...);	// 字符串
printf("Hello World\n");
printf("%d\n", 5);

int fprintf(FILE *stream, const char *format, ...);		// FILE 指针
fprintf(stderr, "Hello World\n");

int dprintf(int fd, const char *format, ...);			// 文件描述符fd
dprintf(STDERR_FILENO, "Hello World\n");

int sprintf(char *buf, const char *format, ...);		// 写入缓存，末尾自动加终止字符"\0"
char buff[100];
sprintf(buff, "Hello World\n");

int snprintf(char *buf, size_t size, const char *format, ...);	// 显示指定缓冲区多大 末尾加"\0"
// 如果写入到缓冲区的字节数大于参数 size 指定的大小，超出部分将会被丢弃

format:
	%[flags][width][.precision][length]type
	flag:标志
	width:输出最小宽度，表示转换后输出字符串的最小宽度；
	precision：精度
	length: 长度修饰符
	type： 转换类型
	
type:
	d int 				十进制整数
	o unsigned int		八进制整数
	u unsigned int 		十进制整数
	x unsigned int 		十六进制整数
	f double 			浮点数
	e double			科学计数法表示浮点数
	s char*				输出字符串至"\0"
	p void*				十六进制指针
	c char				字符型，可把数字转 ascii 码
```

### 8.2 格式化输出

```c
#include <stdio.h>

int scanf(const char *format, ...);			// 字符串
int a, b, c
scanf("%d %d %d", &a, &b, &c);

int fscanf(FILE *stream, const char *format, ...);	// 从FILE 格式指定文件内读 stdin
int a, b, c
fscanf(stdin, "%d %d %d", &a, &b, &c);

int sscanf(const char *str, const char *format, ...);	// 从字符串内读
char *str = '5454 hello';
char buf[10];
int a;
sscanf(str, "%d %s", &a, buf);

%[*][width][length]type
%[m][width][length]type
如果添加了 m，调用者无需分配相应的缓冲区来保存转换后的数据，格式化输入函数内部会自动分配足够大小的缓冲区
char *buf;
scanf("%ms", &buf);
- width：最大字符宽度
- length：长度修饰符
- type：指定输入数据的类型
```

## 9. 文件IO的内核缓冲

read、write 系统调用对文件进行读写的时候并不会直接访问磁盘，而是访问内核分配的缓冲区进行修改，后续由操作系统完成缓冲区与磁盘的同步；同步时间由内核相应的存储算法自动判断

这个内核缓冲区就称为 文件 IO 的内核缓冲；目的为了提高文件 I/O的速度和效率

### 9.1 刷新文件 I/O 的内核缓冲区

强制将文件 I/O 内核缓冲区中缓存的数据刷新到磁盘设备中

**控制文件 I/O 内核缓冲的系统调用**

- sync()、syncfs()、fsync()、fdatasync()
- 对性能的影响较大，尽量能不用就不用

元数据：不是文件内容本身的数据，用于记录文件属性相关的数据信息（譬如文件大小、时间戳、权限）

```c
#include<unistd.h>

int fsync(int fd);		// 将 fd 所指文件内容数据和元数据写入磁盘
返回值：成功 0  失败 -1

元数据：记录文件属性相关信息
```

```c
#include<unistd.h>
// 仅将 fd 所指文件的内容数据写入磁盘，不包含元数据
int fdatasync(int fd);
```

```c
#include <unistd.h>
// 将所有文件的内容数据和元数据都写入磁盘设备中
void sync(void);
```

**控制文件 I/O 内核缓冲的标志**

open 时指定一些标志可以影响到文件 I/O 内核缓存

**O_DSYNC 标志**

相当于write 之后自动调用 fdatasync 

```c
fd = open(filepath, O_WRONLY | O_DSYNC)
```

**O_SYNC 标志**

write 之后会自动将文件内容数据和元数据刷到磁盘当中去

### 9.2 直接I/O 绕过内核缓冲

允许应用程序在执行 I/O 操作时绕过内核缓冲区，从用户空间将数据传递到文件或者磁盘设备，称为 直接 I/O（会降低性能）

```c
// O_DIRECT 标志，直接 I/O
fd = open(filepath, O_WRONLY | O_DIRECT);
```

直接IO三对齐要求：

- 存放数据的缓冲区，其内存起始地址必须以块大小的整数倍进行对齐
- 写文件是，文件的位置偏移量必须是块大小的整数倍
- 写入到文件的数据大小必须是块大小的整数倍

```
tune2fs -l /dev/sda1 | grep "Block size"	// 查看块大小
```

## 10. stdio 缓冲

标准IO 是在 文件IO 基础上进行封装而实现，性能上要优于 文件 I/O，原因是标准 I/O 实现维护了自己的缓冲区，目的为了减少系统调用的次数

标准 I/O 维护的缓冲区叫 stdio 缓冲，是用户空间的缓冲区

### 10.1 对 stdio 缓冲进行设置

```c
#include<stdio.h>

int setvbuf(FILE *stream, char *buf, int mode, size_t size);
FILE: 每个文件都可以设置它对应的 stdio 缓冲区
buf: buf 指向的size大小缓冲区将作为该文件的stdio缓冲区
mode:缓冲类型
	_IONBF：不对I/O 进行缓冲（无缓冲）标准错误一般是
	_IOLBF：行缓冲I/O，碰到"\n"（终端设备 标准输入 标准输出 一般是）
	_IOFBF：全缓冲I/O，填满stdio缓冲区后才进行文件IO操作（普通磁盘默认）
返回值： 成功0 失败非0
```

```c
#include <stdio.h>

void setbuf(FILE *stream, char *buf);
相当于 setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
```

```c
#include <stdio.h>

void setbuffer(FILE *stream, char *buf, size_t size);
相当于 setvbuf(stream, buf, buf ? _IOFBF : _IONBF, size);
```

### 10.2 强制刷新 stdio 缓冲区

无论采用何种缓冲模式，都可以利用库函数 fflush 来强制刷新 stdio 缓冲区的数据写入到 内核缓冲区

```c
#include <stdio.h>
int fflush(FILE *stream);
```

- fclose 关闭文件时，会自动刷新该文件的 stdio 缓冲区
- 程序退出时刷新 stdio 缓冲区

![1682078395030](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1682078395030.png)

## 11. 文件描述符和 FILE 指针互相转化

注意：文件I/O 会直接将数据写入内核缓冲区进行高速缓存；标准 I/O 会将数据写入到 stdio 缓冲区，之后再调用 write() 将 stdio 缓冲区的数据写到内核缓冲区

```c
#include <stdio.h>

int fileno(FILE *stream);
FILE *fdopen(int fd, const char *mode);
```

