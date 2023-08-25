# 字符串处理

## 1. 字符串输出

输出字符串并换行，"\0"换成了"\n"

```c
#include <stdio.h>
int puts(const char *s);
放回值： 非负数  失败 EOF（-1）
```

------

把参数 c 指定的字符（一个无符号字符）输出到标准输出设备， 其输出可以是一个字符，可以是介于 0~127 之间的一个十进制整型数（包含 0 和 127，输出其对应的 ASCII 码字符） 也可以是用 char 类型定义好的一个字符型变量。 

```c
#include <stdio.h>
int putchar(int c);
返回值： 错误 EOF
putchar("C");
```

------

同 putchar ，能指定输出的文件

```c
#include <stdio.h>
int fputc(int c, FILE *stream);
返回值： 失败 EOF
```

------

把字符串输出到指定文件中

```c
#include <stdio.h>
int fputs(const char *s, FILE *stream);
```

## 2. 字符串输入

从标准输入设备（譬如键盘）中获取用户输入的字符串 ；空格也可以输入，输入完成回车即可；当从输入缓存区中读走字符后，相应的字符就不存在于缓冲区中

```c
#include <stdio.h>
char *gets(char *s);
返回值： 成功 返回指向 s 的指针   失败 NULL
```

- get 允许制表符和回车的存在，scanf 则通常将其作为分隔符
- get 会将 回车换行符号从输入缓存中取出来，然后将其丢弃，gets 读走缓冲区的字符串数据后，缓冲区并不会留下回车换行符；scanf 读取缓冲区中的字符串时，并不会将分隔符（空格、制表符、回车换行）读走，缓冲区仍留有用户残留的分割符

```c
#include <stdio.h>
//从标准输入设备中读取一个字符（一个无符号字符）
int getchar(void);
返回值：以无符号 char 强制转换为 int 的形式返回读取的字符    失败：EOF
```

------

指定获取字符串文件，区别于gets，fgets 也会将缓冲区中的换行符读取出来，但并不丢弃，而是作为字符串组成字符存在，读取完成之后自动在最后添加字符串结束字符'\0' 

```c#
#include <stdio.h>
char *fgets(char *s, int size, FILE *stream);
```

------

```c
#include <stdio.h>
int fgetc(FILE *stream);
返回值：无符号 char 强制转换为 int 的形式返回读取的字符
```

## 3. 字符串长度

```c
#include <string.h>
size_t strlen(const char *s);
s：带计算长度的字符串，字符串必须包含结束字符' \0 '
返回值：字符串的长度  "\0"不算在内
```

## 4. 字符串拼接

```c
#include <string.h>
char *strcat(char *dest, const char *src);、
返回值：指向 dest
把 src 所指向的字符串 追加到 dest 所指向的字符串末尾，dest末尾的"\0"会被删除，src的"\0"会被保留
```

可以指定追加数量

```c
#include <string.h>
char *strncat(char *dest, const char *src, size_t n);
n：追加的最大字符数
源字符串 src 包含 n 个或更多个字符， 则 strncat()将 n+1 个字节追加到 dest 目标字符串（src 中的 n 个字符加上结束字符' \0 '）。
```

## 5. 字符串拷贝

```c
#include <string.h>
char *strcpy(char *dest, const char *src);
返回值：指向dest的指针
strcpy()会把 src（必须包含结束字符' \0 '） 指向的字符串复制（包括字符串结束字符' \0 '） 到 dest，
```

------

```c
#include <string.h>
char *strncpy(char *dest, const char *src, size_t n);
n：从 src 中复制的最大字符数
把 src 所指向的字符串复制到 dest，最多复制 n 个字符。当 n 小于或等于 src 字符串长度（不包括结束
字符的长度）时， 则复制过去的字符串中没有包含结束字符' \0 '；当 n 大于 src 字符串长度时，则会将 src 字符串的结束字符' \0 '也一并拷贝过去， 
```

## 6. 内存 填充

以字节为单位进行数据填充

```c
#include <string.h>
void *memset(void *s, int c, size_t n);
s：填充的内存空间起始地址
c：填充的值
n：填充的字节数
返回值：指向 s 的指针
```

------

将一段内存空间中的数据全部设置为 0 

```c
#include <strings.h>
void bzero(void *s, size_t n);
n：字节数
```

## 7. 字符串比较

比较的是第一个不同字符的 ASCII 码

```c
#include <string.h>
int strcmp(const char *s1, const char *s2);
返回值：
	如果返回值小于 0，则表示 str1 小于 str2
	如果返回值大于 0，则表示 str1 大于 str2
	如果返回值等于 0，则表示字符串 str1 等于字符串 str2
```

------

最多比较前 n 个字符 

```c
#include <string.h>
int strncmp(const char *s1, const char *s2, size_t n);
```

## 8. 字符串查找

查找到给定字符串当中的某一个字符 

```c
#include <string.h>
char *strchr(const char *s, int c);
s：被查字符串
c：需要查找字符
返回值：c 在 s 中第一次出现的位置，未找到 NULL
```

------

在给定的字符串 haystack 中查找第一次出现子字符串 needle 的位置 ,不包含结束字符' \0 ' 

```c
#include <string.h>
char *strstr(const char *haystack, const char *needle);
返回值：返回该字符串首次出现的位置；如果未能找到子字符串 needle，则返回 NULL。
```

## 9. 字符串与数字互转

字符串转 int 、long int 、long long int

跳过开头空格，直到遇上数字字符或正负符号才开始做转换，而再遇到非数字或字符串结束时(' /0 ')才结束转换 

```c
#include <stdlib.h>

int atoi(const char *nptr);
long atol(const char *nptr);
long long atoll(const char *nptr);
返回值：转化后的 int 数据
```

------

字符串转 long int 、long long int 数据类型，可以实现不同进制数

可以以任意数量的空格或者 0 开头， 转换时跳过前面的空格字符，直到遇
上数字字符或正负符号（' + '或' - '） 才开始做转换，而再遇到非数字或字符串结束时(' /0 ')才结束转换， 

```c#
#include <stdlib.h>

long int strtol(const char *nptr, char **endptr, int base);
long long int strtoll(const char *nptr, char **endptr, int base);
nptr：需要转化的目标字符串
endptr： strtol()或 strtoll()会将字符串中第一个无效字符的地址存储在*endptr 中，NULL表示不接收
base：数字基数 2  8 16
返回值：转化后的数据类型
```

------

字符串 转 unsigned long int 、 unsigned long long int

```c
#include <stdlib.h>

unsigned long int strtoul(const char *nptr, char **endptr, int base);
unsigned long long int strtoull(const char *nptr, char **endptr, int base);
```

------

字符串转浮点型数据 double

```c
#include <stdlib.h>

double atof(const char *nptr);
```

------

```c
#include <stdlib.h>

double strtod(const char *nptr, char **endptr);		// float
float strtof(const char *nptr, char **endptr);		// double
long double strtold(const char *nptr, char **endptr);	// long double
endptr：意义同上
```

## 10. 数字转字符串

printf 、sprintf 、snprintf 

```c
char str[50] = {0};

sprintf(str, "%d", 500);
sprintf(str, "%f", 500.111);
sprintf(str, "%u", 500)
```

