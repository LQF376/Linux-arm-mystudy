# Linux 块设备驱动框架

字符设备是顺序的数据流设备，字符设备是按照字节进行读写访问；字符设备不需要缓冲区，对于字符设备的访问都是实时的，不需要按固定的块大小进行访问

## 1.  块设备

块设备针对存储设备（SD卡、EMMC、NAND Flash、固态硬盘等）

- 块设备只能以块为单位进行读写访问（字符设备是以字节为单位进行数据传输的，不需要缓存）
- 块设备再结构上是可以进行随机访问的，块设备使用缓冲区来暂时存放数据，等到条件成熟以后再一次性将缓冲区的数据写入块设备中（主要为了提高块设备的使用寿命）
- 块设备的结构不同其I/O算法也会不同；对于 EMMC、SD 卡 等可以随机访问的；机械硬盘存取数据需要移动磁头，Linux 针对不同的存储设备实现了不同的I/O调度算法

## 2. 块设备驱动框架

### 2.1 block_device 结构体（表示块设备）

- Linux 内核 使用 block_device 表示块设备；定义在 include/linux/fs.h

![1685694842358](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685694842358.png)

![1685694855667](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685694855667.png)

```c
/* 向内核注册新的块设备、申请设备号 */
int register_blkdev(unsigned int major, const char *name)
major：主设备号
name：块设备名字
返回值：1. major 在1~255之间，表示自定义主设备号，返回值 0：成功；负值：失败
2. major=0，表示系统自动分配主设备号，返回值 系统分配的主设备号（1~255）；负值：失败

/* 不使用某个块设备了，就要注销掉 */
void unregister_blkdev(unsigned int major, const char *name)
major：要注销的块设备主设备号
name：要注销的块设备名字
```

### 2.2 gendisk 结构体

- Linux 内核使用 gendisk 来描述一个磁盘设备，定义在 include/linux/genhd.h

![1689075103891](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1689075103891.png)

重点：fops：块设备操作集；request_queue：磁盘对应的请求列表

------

编写块的设备驱动的时候需要分配并初始化一个 gendisk

```c
/* 1. 申请一个 gendisk */
struct gendisk *alloc_disk(int minors)
minors：次设备号数量，gendisk 对应的分区数量
返回值：成功：申请到的 gendisk ；失败：NULL

/* 2. 删除 gendisk */
void del_gendisk(struct gendisk *gp)
gp：要删除的 gendisk
返回值：无

/* 3. 将 gendisk 添加到内核 */
/* 初始化完 gendisk 后，还需要将其添加到内核中才能使用 */
void add_disk(struct gendisk *disk)
disk：要添加到内核的 gendisk
返回值：无

/* 4. 设置 gendisk 容量 */
/* 每一个磁盘都有容量，在初始化 gendisk 时候需要设置其容量 */
void set_capacity(struct gendisk *disk, sector_t size)
disk：要设置容量的 gendisk
size：磁盘容量大小，单位：多少扇区；一个扇区一般是512字节

/* 增加/减小 gendisk 引用 */
turck kobject *get_disk(struct gendisk *disk)		// 增加 gendisk 的引用计数
void put_disk(struct gendisk *disk)					// 减少 gendisk 的引用计数
```

### 2.3 block_device_operations 结构体

- 块设备操作集；定义在 include/linux/blkdev.h

![1685705350483](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685705350483.png)

### 2.4 块设备I/O请求过程

- 块设备操作集（block_device_operations 中没有 read/write）；块设备读写数据依靠 request_queue、request、bio 来实现
- 内核将对块设备的读写都发送到请求队列 request_queue 中，request_queue 中是大量的 request （请求结构体），而 request 又包含了 bio，bio 保存了读写相关数据
- 上层应用程序对于块设备的读写会被构成一个或多个 bio 结构，bio 结构描述了要读写的起始扇区、要读写的扇区数量、是读取还是写入、页偏移、数据长度等等信息；上层会将 bio 提交给 I/O 调度器，I/O调度器会将这些 bio 构成 request 结构，而一个物理存储设备对应一个 request_queue，request_queue 里面顺序存放着一系列的 request，然后插入到 request_queue 中合适的位置，这一切都是由 I/O 调度器来完成

![1685709928106](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685709928106.png)

### 2.5 request_queue 请求队列

- gendisk 结构体中有一个 request_queue 结构体类型成员变量，即在编写块设备驱动的时候，每个磁盘（gendisk）都要分配一个 request_queue

request_queue 根据设备是否需要调度器优化可分为两种初始化方式：

1. 申请并初始化请求队列（针对机械硬盘，需要调度器优化的初始化方式）

   - blk_init_queue 函数完成了请求队列的申请以及请求处理函数的绑定，会给请求队列分配一个 I/O 调度器
   - 一般适用于像机械硬盘这样的存储设备，需要 I/O 调度器来优化数据读写过程

   ```c
   /* 申请并初始化一个 request_queue */
   request_queue *blk_init_queue(request_fn_proc *rfn, spinlock_t *lock)
   rfn：请求处理函数指针，每个 request_queue 都要有一个请求处理函数
   lock：自旋锁指针，需要驱动编写人员定义一个自旋锁，传进来；请求队列会使用这个自旋锁
   返回值：失败：NULL；成功：申请到的 request_queue 地址
   
   /* 请求处理函数原型 */
   void (request_fn_proc) (struct request_queue *q)
   ```

2. 删除请求队列

   ```c
   /* 卸载块设备驱动的时候，我们要删除前面申请的request_queue */
   void blk_cleanup_queue(struct request_queue *q)
   q：需要删除的请求队列
   ```

3. 分配请求队列并绑定制造请求函数（对于 非机械存储设备，无需 I/O 调度）

   - 对于 EMMC、SD卡这样的非机械设备，可以进行完全随机访问，所以不需要复杂的 I/O 调度器

   - 只需先申请 request_queue，然后将申请到 request_queue 与 “制造请求” 函数绑定在一起

     ```c
     /* 申请 request_queue */
     struct request_queue *blk_alloc_queue(gfp_t gfp_mask)
     gfp_mask：内存分配掩码 include/linux/gfp.h  一般为GFP_KERNEL
     返回值：申请到的无 I/O 调度的 request_queue
     
     /* 将申请到的 request_queue 与 "制造请求"函数绑定在一起 */
     void blk_queue_make_request(struct request_queue *q, make_request_fn *mfn)
     q：需要绑定的请求队列， blk_alloc_queue 申请到的请求队列
     mfn：需要绑定的“制造”请求函数
     void (make_request_fn) (struct request_queue *q, struct bio *bio)
     ```

### 2.6 request 请求

- 请求队列（request_queue）里面包含的就是一系列的请求（request）
- request 结构体定义在 include/linux/blkdev.h
- request 里面有一个 bio 成员变量，真正的数据就保存在 bio 里面
- 我们需要从 request_queue 中取出一个个的 request，然后再从每个 request 里面取出 bio，最后根据 bio 的描述将数据写入块设备，或者从块设备中读取数据

```c
/* 1. 获取请求 */
/* 从 request_queue 中依次获取每个 request */
request *blk_peek_request(struct request_queue *q)
q：request_queue
返回值：request_queue 中下一个要处理的请求(request)；如果没有要处理的请求就返回 NULL

/* 2. 开启请求 */
/* 获取到下一个要处理的请求以后就要开始处理这个请求 */
void blk_start_request(struct request *req)
req：要开始处理的请求
返回值：无

/* 3. 一步到位处理请求，即 获取请求+开启请求 */
struct request *blk_fetch_request(struct request_queue *q)
{
    struct request *rq;
    
    rq = blk_peek_request(q);
    if (rq)
        blk_start_request(rq);
    return rq;
}
```

![1685708793925](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685708793925.png)

### 2.7 bio 结构

- 每个 request 里面有多个 bio，bio 结构体描述了要读写的起始扇区、要读写的扇区数量、是读取还是写入、页偏移、数据长度等等信息

![1685710054386](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685710054386.png)

![1685710084101](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685710084101.png)

对于物理存储设备的操作不外乎是将 RAM 中的数据写入到物理存储设备中，或者将物理设备中的数据读取到 RAM 中去处理；数据传输三个要求：数据源、数据长度以及数据目的地，即要从物理存储设备的哪个地址开始读取、读取到 RAM 中的哪个地址处、读取的数据长度是多少

- bi_iter 结构体用于描述物理存储设备地址信息；比如要操作的扇区地址

  ![1685710417686](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1685710417686.png)

- bi_io_vec 指向 bio_vec 数组首地址，bio_vec 数组就是 RAM 信息，比如页地址、页偏移以及长度（页地址是Linux内核里面内存管理概念）

  ![1689075596309](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1689075596309.png)

![1689075704969](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1689075704969.png)

------

```c
/* 1. 遍历请求中的 bio */
/* 请求中包含大量的 bio，因此涉及遍历请求中的所有bio并进行处理 */
#define __rq_for_each_bio(_bio, rq) \
	if ((rq->bio)) \
		for (_bio = (rq)->bio; _bio; _bio = _bio->bi_next)
_bio：遍历出来的每个 bio
rq：进行遍历操作的请求


/* 2. 遍历 bio 中所有段 */
/* bio 包含最终要操作的数据，因此要需要遍历 bio 中所有段 */
#define bio_for_each_segment(bvl, bio, iter) \
	__bio_for_each_segment(bvl, bio, iter, (bio)->bi_iter)
bv1：遍历出来的每个 bio_vec
bio：要遍历的 bio，类型为 bio 结构体指针
iter：保存要遍历的 bio 中 bi_iter 成员变量


/* 3. 通知 bio 处理结束 */
/* 如果使用“制造请求”，也就是抛开I/O调度器直接处理I/O，在 I/O 处理完成后要通过内核 bio 处理完成 */
bvoid bio_endio(struct bio *bio, int error)
bio：要结束的 bio
error：如果 bio 处理成功的话直接填0；如果失败的话就填个负值，-EIO
返回值：无
```

## 3. 使用请求队列代码

开辟一块内存，用来模拟块设备

```c
#include <linux/types.h>
...
#include <asm/io.h>

#define RAMDISK_SIZE (2 * 1024 * 1024) 		/* 容量大小为 2MB */
#define RAMDISK_NAME "ramdisk" 				/* 名字 */
#define RADMISK_MINOR 3 					/* 表示三个磁盘分区！不是次设备号为 3！ */

/* ramdisk 设备结构体 */
struct ramdisk_dev{
    int major; 					/* 主设备号 */
    unsigned char *ramdiskbuf; 	/* ramdisk 内存空间,用于模拟块设备 */
    spinlock_t lock; 			/* 自旋锁 */
    struct gendisk *gendisk;	 /* gendisk */
    struct request_queue *queue;	/* 请求队列 */
};

struct ramdisk_dev ramdisk; 	/* ramdisk 设备 */

/* 磁盘信息结构体 */
struct hd_geometry {
    unsigned char heads; /* 磁头 */
    unsigned char sectors; /*一个磁道上的扇区数量 */
    unsigned short cylinders; /* 柱面 */
    unsigned long start;
};

/* 打开块设备 */
int ramdisk_open(struct block_device *dev, fmode_t mode)
{
    printk("ramdisk open\r\n");
    return 0;
}

/*  释放块设备 */
void ramdisk_release(struct gendisk *disk, fmode_t mode)
{
    printk("ramdisk release\r\n");
}

/* 获取磁盘信息 */
int ramdisk_getgeo(struct block_device *dev, struct hd_geometry *geo)
{
    /* 这是相对于机械硬盘的概念 */
    geo->heads = 2; 		/* 磁头 */
    geo->cylinders = 32; 	/* 柱面 */
    geo->sectors = RAMDISK_SIZE / (2 * 32 *512); /* 磁道上的扇区数量 */
    return 0;
}

/* 块设备操作函数 */
static struct block_device_operations ramdisk_fops =
{
    .owner = THIS_MODULE,
    .open = ramdisk_open,
    .release = ramdisk_release,
    .getgeo = ramdisk_getgeo,
};

/* 处理传输过程 */
static void ramdisk_transfer(struct request *req)
{
    /* blk_rq_pos 获取到的是扇区地址，左移 9 位转为字节地址 */
    unsigned long start = blk_rq_pos(req) << 9; 
    unsigned long len = blk_rq_cur_bytes(req); 	/* 大小 */
    
    /* bio 中的数据缓冲区
    * 读：从磁盘读取到的数据存放到 buffer 中
    * 写： buffer 保存这要写入磁盘的数据
    */
    void *buffer = bio_data(req->bio);
    
    if(rq_data_dir(req) == READ) 	/* 读数据 */
        memcpy(buffer, ramdisk.ramdiskbuf + start, len);
    else if(rq_data_dir(req) == WRITE) /* 写数据 */
        memcpy(ramdisk.ramdiskbuf + start, buffer, len);
}

/* 请求处理函数 */
void ramdisk_request_fn(struct request_queue *q)
{
    int err = 0;
    struct request *req;
    
    /* 循环处理请求队列中的每个请求 */
    req = blk_fetch_request(q);
    while(req != NULL) {
        
    	/* 针对请求做具体的传输处理 */
    	ramdisk_transfer(req);
        
    	/* 判断是否为最后一个请求，如果不是的话就获取下一个请求
   		* 循环处理完请求队列中的所有请求。
    	*/
    	if (!__blk_end_request_cur(req, err))
        	req = blk_fetch_request(q);
    }
}

/* 驱动入口函数 */
static int __init ramdisk_init(void)
{
    int ret = 0;
    
    /* 1、申请用于 ramdisk 内存 */
    ramdisk.ramdiskbuf = kzalloc(RAMDISK_SIZE, GFP_KERNEL);
    if(ramdisk.ramdiskbuf == NULL) {
        ret = -EINVAL;
        goto ram_fail;
    }
    
    /* 2、初始化自旋锁 */
    spin_lock_init(&ramdisk.lock);
    
    /* 3、注册块设备 */
    ramdisk.major = register_blkdev(0, RAMDISK_NAME); 	/* 自动分配 */
    if(ramdisk.major < 0) {
        goto register_blkdev_fail;
    }
    printk("ramdisk major = %d\r\n", ramdisk.major);
    
    /* 4、分配并初始化 gendisk */
    ramdisk.gendisk = alloc_disk(RADMISK_MINOR);
    if(!ramdisk.gendisk) {
        ret = -EINVAL;
        goto gendisk_alloc_fail;
    }
    
    /* 5、分配并初始化请求队列 */
    ramdisk.queue = blk_init_queue(ramdisk_request_fn, &ramdisk.lock);
    if(!ramdisk.queue) {
        ret = -EINVAL;
        goto blk_init_fail;
    }
    
    /* 6、添加(注册)disk */
    ramdisk.gendisk->major = ramdisk.major; 	/* 主设备号 */
    ramdisk.gendisk->first_minor = 0; 			/*起始次设备号) */
    ramdisk.gendisk->fops = &ramdisk_fops; 		/* 操作函数 */
    ramdisk.gendisk->private_data = &ramdisk; 	/* 私有数据 */
    ramdisk.gendisk->queue = ramdisk.queue; 	/* 请求队列 */
    sprintf(ramdisk.gendisk->disk_name, RAMDISK_NAME);	/* 名字 */
    set_capacity(ramdisk.gendisk, RAMDISK_SIZE/512); 	/* 设备容量(单位为扇区)*/
    
    add_disk(ramdisk.gendisk);
    
    return 0;
    
blk_init_fail:
    put_disk(ramdisk.gendisk);
gendisk_alloc_fail:
    unregister_blkdev(ramdisk.major, RAMDISK_NAME);
register_blkdev_fail:
    kfree(ramdisk.ramdiskbuf); 		/* 释放内存 */
ram_fail:
    return ret;
}

/* 驱动出口函数 */
static void __exit ramdisk_exit(void)
{
    /* 释放 gendisk */
    del_gendisk(ramdisk.gendisk);
    put_disk(ramdisk.gendisk);
    
    /* 清除请求队列 */
    blk_cleanup_queue(ramdisk.queue);
    
    /* 注销块设备 */
    unregister_blkdev(ramdisk.major, RAMDISK_NAME);
    
    /* 释放内存 */
    kfree(ramdisk.ramdiskbuf);
}

module_init(ramdisk_init);
module_exit(ramdisk_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lvqifeng");
```

```c
depmod 				//第一次加载驱动的时候需要运行此命令
modprobe ramdisk.ko 	//加载驱动模块
```

驱动加载成功后会在 /dev/ 下生成一个 ramdisk 的设备

```c
fdisk -l 		//查看磁盘信息
```

格式化 /dev/ramdisk，将其格式化为 vfat 格式

```c
mkfs.vfat /dev/ramdisk
```

格式完后就可以挂载访问

```
mount /dev/ramdisk /tmp
```

## 4. 不使用请求队列代码

在使用请求队列的代码基础上进行修改

```c
static int __init ramdisk_init(void)
{
    ...
    /* 5、分配请求队列 */
    ramdisk.queue = blk_alloc_queue(GFP_KERNEL);
    if(!ramdisk.queue){
        ret = -EINVAL;
        goto blk_allo_fail;
    }
    
    /* 6、设置“制造请求”函数 */
    blk_queue_make_request(ramdisk.queue, ramdisk_make_request_fn);
    
    /* 7、添加(注册)disk */
    ramdisk.gendisk->major = ramdisk.major; 	/* 主设备号 */
    ramdisk.gendisk->first_minor = 0; 		/* 起始次设备号 */
    ramdisk.gendisk->fops = &ramdisk_fops; 	/* 操作函数 */
    ramdisk.gendisk->private_data = &ramdisk; 	/* 私有数据 */
    ramdisk.gendisk->queue = ramdisk.queue; 	/* 请求队列 */
    sprintf(ramdisk.gendisk->disk_name, RAMDISK_NAME); 	/* 名字 */
    set_capacity(ramdisk.gendisk, RAMDISK_SIZE/512); 	/* 设备容量*/
    add_disk(ramdisk.gendisk);
    ...
    return ret;
}

/*  “制造请求”函数 */
void ramdisk_make_request_fn(struct request_queue *q, struct bio *bio)
{
    int offset;
    struct bio_vec bvec;
    struct bvec_iter iter;
    unsigned long len = 0;
    
    offset = (bio->bi_iter.bi_sector) << 9; 	/* 获取设备的偏移地址（要操作的设备地址 扇区） */
    
    /* 处理 bio 中的每个段 */
    bio_for_each_segment(bvec, bio, iter){		// 遍历 bio 中的每个段，对每个段进行处理
        char *ptr = page_address(bvec.bv_page) + bvec.bv_offset;	// 数据真正起始地址
        len = bvec.bv_len;		// 要处理的数据长度
        
        if(bio_data_dir(bio) == READ) 	/* 读数据 */
            memcpy(ptr, ramdisk.ramdiskbuf + offset, len);
        else if(bio_data_dir(bio) == WRITE) 	/* 写数据 */
            memcpy(ramdisk.ramdiskbuf + offset, ptr, len);
        offset += len;
    }
    set_bit(BIO_UPTODATE, &bio->bi_flags);
    bio_endio(bio, 0);		// 结束 bio
}
```

