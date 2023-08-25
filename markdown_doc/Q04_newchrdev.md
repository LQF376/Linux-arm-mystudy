# 新字符设备驱动

- register_chrdev 注册字符设备、unregister_chrdev 来注销字符设备（老版本）
- 老版本需要事先确定好哪些主设备没有使用；会将一个主设备号下面的所有次设备号都使用掉
- 所以 Linux 推荐新字符设备驱动 API 函数

## 1. 分配和释放设备号

```c
/* 没有指定设备号 */
int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)
/* 给定了设备号(主设备号和次设备号) */
int register_chrdev_region(dev_t from, unsigned count, const char *name)
from：要申请的起始设备号

/* 释放设备号 */
void unregister_chrdev_region(dev_t from, unsigned count)
```

```c
/* 创建设备号 */
int major;
int minor;
dev_t devid;

if(major)
{
	devid = MKDEV(major, 0);
	register_chrdev_region(devid, 1, "test");
}
else
{
	alloc_chrdev_region(&devid, 0, 1, "test");
	major = MAJOR(devid);
	minor = MINOR(devid);
}

/* 释放设备号 */
unregister_chrdev_region(devid, 1);
```

## 2. 新的字符设备注册方法

- 字符设备结构

Linux中使用 cdev 结构体来表示一个字符设备， 编写字符设备驱动之前要定义一个 cdev 结构体

```c
include/linux/cdev.h

 struct cdev {
	struct kobject kobj;
	struct module *owner;
	const struct file_operations *ops;			// 字符设备文件操作函数集合
	struct list_head list;
	dev_t dev;								// 设备号
	unsigned int count;
};
```

- 初始化 cdev 结构体（定义好 cdev 变量后要使用 cdev_init 对其进行初始化）

```c
// 初始化 cdev结构体，添加字符设备的文件操作函数集合
void cdev_init(struct cdev *cdev, const struct file_operations *fops)
```

- 添加字符设备

```c
// 向Linux系统添加这个字符设备
int cdev_add(struct cdev *p, dev_t dev, unsigned count)
```

- 卸载驱动

```c
// 从Linux内核中删除相应的字符设备
void cdev_del(struct cdev *p)
```

------

```
struct cdev testcdev;

static struct file_operations test_fops = {
	.owner = THIS_MODULE,
};

testcdev.owner = THIS_MODULE;
cdev_init(&testcdev, &test_fops);
cdev_add(&testcdev, devid, 1);

cdev_del(&testcdev);
```

## 3. 自动创建设备节点

解决 加载驱动程序以后还需要使用命令 mknod 手动创建设备节点 问题

- udev 是一个用户程序
- Linux 下通过 udev 来实现设备文件的创建和删除，udev 可以检测系统中硬件设备状态，可以根据系统中硬件设备状态来创建或者删除设备文件；modprobe 成功加载后就自动在 /dev 目录下创建对应的设备文件节点，使用 rmmod 命令卸载驱动模块以后就删除 /dev 目录下的设备节点文件
- busybox 构建根文件系统时，会创建一个 udev 的简化版本--mdev
- 嵌入式 Linux中使用mdev来实现设备节点文件的自动创建与删除

```
/* 写入 /etc/init.d/rcS 中 */
// 设置热插拔事件由 mdev来管理
echo /sbin/mdev > /proc/sys/kernel/hotplug
```

- 创建和删除类

```c
include/linux/device.h
#define class_create(owner, name)	\
({
static struct lock_class_key __key;	\
__class_create(owner, name, &__key);	\
})

struct class *__class_create(struct module *owner, const char *name,
				struct lock_class_key *key)
---------------
struct class *class_create(struct module *owner, const char *name)
owner: 一般是 THIS_MODULE
name：类名字
返回值：指向 class 的指针
---------------
void class_destory(struct class *cls);
```

- 创建设备

在类下创建一个设备

```c
struct device *device_create(
	struct class *class,	// 设备要创建哪个类下面
	struct device *parent,	// 父设备，一般 NULL
	dev__t devt,			// 设备号
	void *drvdata,			// 设备可能用到的一些数据
	const char *fmt,...		// 设备名字 会生成 /dev/xxx 设备文件
)

----
/* 删除创建的设备 */
void device_destroy(struct class *class, dev_t devt)
class：删除设备所处的类
devt：删除的设备号
```

------

在驱动入口函数里卖弄创建类和设备， 在驱动出口函数里面删除类和设备

```c
struct class *class;		// 类
struct device *device;		// 设备
dev_t devid;				// 设备号	

/* 驱动入口 */
static int __init led_init(void)
{
	/* 创建类 */
	class = class_create(THIS_MODULE, "xxx");
	/* 创建设备 */
	device = device_create(class, NULL, devid, NULL, "xxx");
	return 0;
}

/* 驱动出口 */
static void __exit led_exit(void)
{
	/* 删除设备 */
	device_destroy(newchrled.class, newchrled.devid);
	/* 删除类 */
	class_destroy(newchrled.class);
}

module_init(led_init);
module_exit(led_exit);
```

------

- 设置文件私有数据

做成一个结构体，驱动open函数时，将结构体作为私有数据添加到设备文件中

write、read、close 时可以直接读取私有数据

```c
/* 设备结构体 */
struct test_dev{
	dev_t devid;
	struct cdev cdev;
	struct class *class;
	struct device *device;
	int major;
	int minor;
};

struct test_dev testdev;

/* open 函数 */
static int test_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &testdev;		// 设置私有数据
	return 0;
}
```



