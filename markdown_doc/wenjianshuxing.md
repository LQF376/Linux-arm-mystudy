# 文件属性与目录

## 1. Linux 下一切皆文件

```
stat a.out  // 查看文件类型
	- ：普通文件
	d ：目录文件
	c ：字符设备文件
	b ：块设备文件
	l ：符号链接文件
	s ：套接字文件
	p ：管道文件
```

普通文件分为 文本文件 和 二进制文件

## 2. stat 查看文件属性

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int stat(const char *pathname, struct stat *buf);
buf：传出参数，存储获得的文件属性信息
返回值：成功 0   失败 -1
```

```c
struct stat
{
	dev_t st_dev; 		/* 文件所在设备的 ID */
	ino_t st_ino; 		/* 文件对应 inode 节点编号 */
	mode_t st_mode; 		/* 文件对应的模式 */
	nlink_t st_nlink; 		/* 文件的链接数 */
	uid_t st_uid; 		/* 文件所有者的用户 ID */
	gid_t st_gid; 		/* 文件所有者的组 ID */
	dev_t st_rdev; 		/* 设备号（指针对设备文件） */
	off_t st_size; 		/* 文件大小（以字节为单位） */
	blksize_t st_blksize; 		/* 文件内容存储的块大小 */
	blkcnt_t st_blocks; 		/* 文件内容所占块数 */
	struct timespec st_atim; 		/* 文件最后被访问的时间 */
	struct timespec st_mtim; 		/* 文件内容最后被修改的时间 */
	struct timespec st_ctim; 		/* 文件状态最后被改变的时间 */
}

st_mode:文件权限 32位无符号整数类型
```

**st_mode:**

![1682079183822](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1682079183822.png)

```c
/* 权限（数字均为8进制，0开头）*/
/* st_mode 32 位无符号整形数据 */
S_IRWXU 00700 owner has read, write, and execute permission
S_IRUSR 00400 owner has read permission
S_IWUSR 00200 owner has write permission
S_IXUSR 00100 owner has execute permission

S_IRWXG 00070 group has read, write, and execute permission
S_IRGRP 00040 group has read permission
S_IWGRP 00020 group has write permission
S_IXGRP 00010 group has execute permission

S_IRWXO 00007 others (not in group) have read, write, and execute permission
S_IROTH 00004 others have read permission
S_IWOTH 00002 others have write permission
S_IXOTH 00001 others have execute permission

/* 前4位表示文件类型 */
S_IFSOCK 0140000 socket（套接字文件）
S_IFLNK 0120000 symbolic link（链接文件）
S_IFREG 0100000 regular file（普通文件）
S_IFBLK 0060000 block device（块设备文件）
S_IFDIR 0040000 directory（目录）
S_IFCHR 0020000 character device（字符设备文件）
S_IFIFO 0010000 FIFO（管道文件）
```

### struct timespec 结构体

```c
#include <time.h> 
struct timespec
{
	time_t tv_sec; 				/* 秒 */
	syscall_slong_t tv_nsec; 	/* 纳秒 */
};
```

## 3. fstat 和 lstat 函数

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int fstat(int fd, struct stat *buf);	// 针对文件描述符
int lstat(const char *pathname, struct stat *buf);	// 针对符号链接文件本身，非所指向文件
```

## 4. 改变文件所属所有者和所属组

系统调用，改变文件的所有者和所属组（用户ID 和 组ID）

```c
#include <unistd.h>
int chown(const char *pathname, uid_t owner, gid_t group)；
返回值：成功0 失败-1
```

```c
#include <unistd.h>
#include <sys/types.h>

uid_t getuid(void);
gid_t getgid(void)
```

## 5. 文件访问权限

文件的权限分为普通权限和特殊权限

### 5.1 普通权限

![1688439316564](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1688439316564.png)

### 5.2 特殊权限

![1688439378998](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1688439378998.png)

- 当进程对文件进行操作的时候、将进行权限检查，如果文件的 set-user-ID 位权限被设置，内核会将进程的有效 ID 设置为该文件的用户 ID（文件所有者 ID） ，意味着该进程直接获取了文件所有者的权限、以文件所有者的身份操作该文件
- 当进程对文件进行操作的时候、将进行权限检查，如果文件的 set-group-ID 位权限被设置，内核会将进程的有效用户组 ID 设置为该文件的用户组 ID（文件所属组 ID） ，意味着该进程直接获取了文件所属组成员的权限、以文件所属组成员的身份操作该文件 

### 5.3 目录的权限

- 目录的读权限： 可列出（譬如：通过 ls 命令） 目录之下的内容（即目录下有哪些文件） 
- 目录的写权限： 可以在目录下创建文件、删除文件 
- 目录的执行权限： 可访问目录下的文件，譬如对目录下的文件进行读、写、执行等操作 

### 5.4 检查文件权限

检查执行进程的用户是否对该文件拥有相应的权限；可检查文件是否存在

```c
#include <unistd.h>

int access(const char *pathname, int mode);
mode: 
	- F_OK：检查文件是否存在
	- R_OK：检查是否拥有读权限
	- W_OK：检查是否拥有写权限
	- X_OK：检查是否拥有执行权限(均可 | 复合使用)
返回值：检查通过 0  不通过：-1
```

### 5.5 修改文件权限

```c
#include <sys/stat.h>

int chmod(const char *pathname, mode_t mode);
mode: 文件权限 同open mode相同

int fchmod(int fd, mode_t mode);	// 用文件描述符 fd
```

### 5.6 文件掩码

对新建文件进行屏蔽

```c
#include <sys/types.h>
#include <sys/stat.h>
mode_t umask(mode_t mask);
mask: 文件掩码
```

## 6. 修改文件时间属性

![1688440326836](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1688440326836.png)

系统调用显示的修改文件的时间属性

```c
#include <sys/types.h>
#include <utime.h>

int utime(const char *filename, const struct utimbuf *times);		// 精确到秒级别
times：将时间属性设置为所指定结构体，NULL表示当前系统时间

struct utimbuf {
	time_t actime; 		/* 访问时间 */
	time_t modtime; 	/* 内容修改时间 */
};

系统调用只能显式修改文件最后依次访问时间和文件内容最后被修改时间
```

```c
#include <sys/time.h>
// 可以精确到微妙级别
int utimes(const char *filename, const struct timeval times[2]);
times: 数组；第一个元素为访问时间 第二个元素为内容修改时间
返回值：成功0 失败-1
struct timeval {
	long tv_sec; 	/* 秒 */
	long tv_usec; 	/* 微秒 */
}
```

------

```c
#include <fcntl.h>
#include <sys/stat.h>
// 精确到纳秒
int futimens(int fd, const struct timespec times[2]);
times:	访问时间  和   内容修改时间
	- NULL:都设置为系统当前时间
	- 其中一个为 UTIME_NOW ，将其设置为系统当前时间
	- 其中一个为 UTIME_OMIT， 相应的时间保持不变
```

```c
int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags);
dirfd: 目录的文件描述符，如果是绝对路径此项忽略
flags:一般为0；如果是 AT_SYMLINK_NOFOLLOW ,x修改的是符号链接的时间蹉
返回值：成功 0   失败-1
```

## 7. 符号链接

硬链接文件和源文件有相同的 inode 指向物理磁盘的同一区块，链接文件和源文件处于平等地位；文件的链接数指的是硬链接的数量，删除一个文件，就会使得链接数减一，直到0，被文件系统回收；

软链接，链接文件和源文件有着不同的 inode 号，软链接文件的数据块中存储的是源文件的路径名，源文件和链接文件是主从关系，源文件被删除后，软链接将无效，称为悬空链接

- 不能对目录创建硬链接
- 硬链接要求链接文件和源文件在同一个文件系统中
- 对目录可以创建软连接
- 可以跨文件系统
- 可以对不存在的文件创建软链接

```shell
ln 源文件 链接文件  // 硬链接
ln s 源文件 链接文件 // 软链接
```

### 7.1 创建硬链接

```c
#include <unistd.h>

int link(const char *oldpath, const char *newpath);
返回值：成功0   失败-1
```

### 7.2 创建软链接

```c
#include <unistd.h>

int symlink(const char *target, const char *linkpath);
```

### 7.3 打开读取软链接中的数据

软连接中存储的是链接文件的路径信息，open 打开一个链接文件打开的是该链接指向的文件，读取链接文件的路径信息需要用 readlink

```c
#include <unistd.h>
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
buf：存放读取信息的缓冲区
bufsiz：缓冲区的大小
```

## 8. 目录

普通文件由 inode 节点 和 数据块 组成

目录的存储形式由 inode 节点 和 目录块 所构成；目录块中记录了有哪些文件组织在这个目录下，记录他们的文件名和 Inode 编号

![1682085500210](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1682085500210.png)

### 8.1 创建和删除目录

```c
#include <sys/stat.h>
#include <sys/types.h>

int mkdir(const char *pathname, mode_t mode);
mode: mode &~ umask
返回值： 成功 0  失败 -1
```

```c
#include <unistd.h>

int rmdir(const char *pathname);
```

### 8.2 打开、读取以及关闭目录

```c
#include <sys/types.h>
#include <dirent.h>

DIR *opendir(const char *name);
返回值：指向目录的句柄， 失败 NULL
name: 绝对路径/相对路径
```

获取目录下所有文件的名称以及对应的 inode 号

```c
#include <dirent.h>

struct dirent *readdir(DIR *dirp);
返回值：目录条目结构体，每调用一次，就会从指向的目录流中读取下一条的目录项，并返回结构体（静态分配的会覆盖上一次结果）到目录尾或者出错返回NULL，可通过 error 判断

struct dirent {
	ino_t d_ino; 	/* inode 编号 */
	off_t d_off; 	/* not an offset; see NOTES */
	unsigned short d_reclen; 	/* length of this record */
	unsigned char d_type; 	/* type of file; not supported by all filesystem types */
	char d_name[256]; /* 文件名 */
};
```

重新定位目录流

```c
#include <sys/types.h>
#include <dirent.h>
// 将目录流重置到目录起点，下次从头开始读 readdir是依次读取一个文件的
void rewinddir(DIR *dirp);
```

```c
#include <sys/types.h>
#include <dirent.h>

int closedir(DIR *dirp);
```

### 8.3 进程当前的工作目录

Linux 下每一个进程都有自己的当前工作目录

```c
#include <unistd.h>

char *getcwd(char *buf, size_t size);
buf：当前工作目录绝对路径存放缓存
返回值: 指向buf的指针    失败 NULL
```

### 8.4 改变当前的工作目录

```c
#include <unistd.h>

int chdir(const char *path);	// 以路径方式
int fchdir(int fd);			// 以文件描述符的方式（fd 所指定的 目录）
```

## 9. 删除文件

本质是删除硬链接的数量（当文件被其他进程打开的时候是无法删除的）unlink 不会对软链接进行解引用，删的是软链接本身

```c
/* 系统调用 */
#include <unistd.h>
int unlink(const char *pathname);
```

```c
/* C库函数 移除一个文件或者空目录 不会对软链接进行解引用 */
#include <stdio.h>
int remove(const char *pathname);
```

## 10. 文件重命名

仅操作目录条目，不移动文件数据（不改变 inode 和 数据块），不影响硬链接，也不影响已经打开文件的进程；不会对软链接进行解引用

```c
#include <stdio.h>

int rename(const char *oldpath, const char *newpath);
```

