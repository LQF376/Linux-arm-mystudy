# Linux中断

- linux内核提供了万能的中断框架，只要申请中断，然后注册中断处理函数即可

**裸机的中断处理方法：**

1. 使能中断，初始化相应的寄存器
2. 注册中断服务函数，也就是向 irqTable 数组的指定标号处写入中断服务函数 
3. 中断发生以后进入 IRQ 中断服务函数，在 IRQ 中断服务函数在数组 irqTable 里面查找具体的中断处理函数，找到以后执行相应的中断处理函数 

## 1. Linux 中断 API

- 中断号

  每一个中断都有一个中断号，通过中断号来区分不同的中断

- 向内核申请中断

  ```c
  int request_irq(unsigned int irq,
  				irq_handler_t handler,
  				unsigned long flags,
  				const char *name,
  				void *dev)
  				
  irq：要申请中断的中断号
  handler：中断处理函数，中断发生后就会执行此中断处理函数
  flags：中断标志   include/linux/interrupt.h
  name：中断名字   可以在 /proc/interrupts 文件内看到中断名字
  dev：一般情况下，设置为设备结构体，会传递给中断处理函数 irq_handler_t 的第二个参数
  返回值： 0->申请成功 负值->申请失败  -EBUSY->中断已经被申请
  ```

  flags 标志位： 

  ![1678868445985](E:\typora\markdownImage\1678868445985.png)

- 释放相应的中断

  ```c
  void free_irq(unsigned int irq, void *dev)
  irq：释放的中断号
  dev：如果 flags 为 IRQF_SHARED，此参数用来区分具体的中断
  返回值：无
  ```

- 中断处理函数格式

  ```c
  irqreturn_t (*irq_handler_t)(int, void*)
  int：中断处理函数要相对应的中断号
  void：需要于 request_irq 中 dev 参数保持一致
  ------
  enum irqreturn {
  	IRQ_NONE = (0 << 0),
  	IRQ_HANDLED = (1 << 0),
  	IRQ_WAKE_THREAD = (1 << 1),
  };
  
  typedef enum irqreturn irqreturn_t;
  // 返回值是一个枚举类型，一共有3种返回值
  return IRQ_RETVAL(IRQ_HANDLED)
  ```

- 中断使能和禁止

  ```c
  void enable_irq(unsigned int irq)		// 使能
  
  // disable_irq 函数会等到当前正在执行的中断处理函数执行完才返回
  void disable_irq(unsigned int irq)		// 	禁止
  
  // disable_irq_nosync 会立刻返回，不会等待当前中断处理程序执行完毕
  void disable_irq_nosync(ubsigned int irq)
  --------------------
  /* 全局关闭和全局打开 */
  local_irq_enable()
  local_irq_disable()			// 多进程间会出现问题
  -------------------
  local_irq_save(flags)		// 禁止中断，并将中断状态保存在 flgas 中
  local_irq_restore(flags)	// 恢复中断，将中断恢复到 flags 状态
  ```

## 2. 中断 上半部和下半部

- 希望中断处理函数一定要快点执行完毕，越短越好，但实际中断处理过程相当花费时间，将中断处理过程分成了两部分

  > __上半部：__ 就是中断处理函数，处理过程比较快，不会占用很长时间的处理就可以放在上半部完成
  >
  > __下半部：__ 如果处理过程比较的耗时，那么就可以将比较耗时的代码提出来，交给下半部去执行，这样中断处理函数就可以实现快进快出

  

### 2.1 实现下半部的机制

1. 软中断（代替 bottom half 来实现下半部）

   - Linux 使用 softirq_action表示软中断 

   ```c
   /* include/linux/interrupt.h */
   struct softirq_action
   {
   	void (*action)(struct softirq_action *);	// 软中断服务函数
   };
   
   /* kernel/softirq.c 中定义了10个软中断 */
   static struct softirq_action softirq_vec[NR_SOFTIRQS];
   
   enum
   {
   	HI_SOFTIRQ=0, 	/* 高优先级软中断 */
   	TIMER_SOFTIRQ, 	/* 定时器软中断 */
   	NET_TX_SOFTIRQ, /* 网络数据发送软中断 */
   	NET_RX_SOFTIRQ, /* 网络数据接收软中断 */
   	BLOCK_SOFTIRQ,
   	BLOCK_IOPOLL_SOFTIRQ,
   	TASKLET_SOFTIRQ, 	/* tasklet 软中断 */
   	SCHED_SOFTIRQ, 		/* 调度软中断 */
   	HRTIMER_SOFTIRQ, 	/* 高精度定时器软中断 */
   	RCU_SOFTIRQ, 		/* RCU 软中断 */
   	
   	NR_SOFTIRQS
   };
   ```

   - 注册对应的软中断处理函数

     ```c
     void open_softirq(int nr, void (*action)(struct softirq_action *))
     nr：要开启的软中断（10选1）
     action：对应的处理函数
     返回值：没有返回值
     ```

     __软中断必须在编译时候静态注册！！__

     Linux 内核使用 softirq_init 函数来初始化软中断，定义在 kernel/softirq.c 内（softirq_init(void)）

     默认会打开 HI_SOFTIRQ 、TASKLET_SOFTIRQ

   - 触发软中断

     ```c
     void raise_softirq(unsigned int nr)
     nr：要触发的软中断
     返回值：无
     ```

2. tasklet（和软中断两者优先选择）

   利用软中断来实现的另一种下半部机制

   Linux 内核使用 tasklet_struct 结构体来表示 tasklet

   ```c
   struct tasklet_struct
   {
   	struct tasklet_struct *next; 	/* 下一个 tasklet */
   	unsigned long state;		 /* tasklet 状态 */
   	atomic_t count; 				/* 计数器，记录对 tasklet 的引用数 */
   	void (*func)(unsigned long); 	/* tasklet 执行的函数 */
   	unsigned long data;			 /* 函数 func 的参数 */
   };
   ```

   - 定义一个 tasklet，然后使用 tasklet_init 函数初始化 tasklet

     ```c
     void tasklet_init(struct tasklet_struct *t,
     				void (*func)(unsigned long),
     				unsigned long data);
     t：要初始化的 tasklet
     func：tasklet的处理函数
     data：要传递给func函数的参数
     返回值：无
     
     // 利用宏一次性完成 taklet 的定义和初始化 include/linux/interrupt.h
     DECLARE_TASKLET(name, func, data)
     ```

   - 上半部，中断处理函数中调用 tasklet_schedule 函数，就能使 tasklet 在适合的时间运行

     ```c
     void tasklet_schedule(struct tasklet_struct *t)
     t：要调度的 tasklet， DECLARE_TASKLET 宏里面的name
     ```

   ```c
   /* tasklet demo */
   /* 利用 tasklet 实现中断的上半部分和下半部分分离 */
   struct tasklet_struct testtasklet;		// 定义一个 tasklet
   
   /* tasklet 处理函数（下半部） */
   void testtasklet_func(unsigned long data)
   {
   	...
   }
   
   /* 中断处理函数（上半部） */
   irqreturn_t test_handler(int irq, void *dev_id)
   {
   	...
   	tasklet_schedule(&testtasklet);
   	...
   }
   
   /* 驱动入口函数 */
   static int __init xxx_init(void)
   {
   	...
   	tasklet_init(&testtasklet, testtasklet_func, data);	// 初始化 tasklet
   	request_irq(xxx_irq, test_handler, 0, "xxx", &xxx_dev);	// 注册中断处理函数
   	...
   }
   ```

3. 工作列队

   工作列队在进程的上下文执行，工作列队将要推后的工作交给一个内核线程去执行（允许睡眠和重新调度）

   如果推后的工作支持睡眠，就可以选择工作列队，否则选择 tasklet

   Linux 内核使用 work_struct 结构体表示一个工作

   ```c
   struct work_struct {
   	atomic_long_t data;
   	struct list_head entry;
   	work_func_t func; 	/* 工作队列处理函数 */
   };
   ```

   多个工作组成工作队列，工作队列用 workqueue_struct 结构体表示

   工作者线程（worker thread）来处理工作队列中的各个工作，worker结构体表示工作者线程，每个 worker 都有一个工作队列，工作者线程处理自己工作队列中的工作

   在实际驱动工作中，我们只需要定义工作（work_struct）即可，关于队列和工作者线程基本不用管

   ```c
   /* 定义工作(work_struct)->使用 INIT_WORK 来初始化 定义完后再初始化！！！！*/
   #define INIT_WORK(_work, _func)
   _work：要初始化的工作
   _func：工作对应的处理函数
   ```

   ```c
   #define DECLARE_WORK(n, f)	// 一次性完成创建和初始化的工作
   n：工作（work_struct） f：工作对应的处理函数
   ```

在上半部执行调度才能运行

```c
bool schedule_work(struct work_struct *work)
work：要调度的工作
返回值：0->成功 其他值->失败
```

```c
   /* demo */
   /* 利用工作队列来实现中断的上半部和下半部分离 */
   struct work_struct testwork;		// 创建一个工作
   
   /* work处理函数 */
   void testwork_func_t(struct work_struct *work)
   {
   	...
   }
   
   /* 中断处理函数 */
   irqreturn_t test_handler(int irq, void *dev_id)
   {
   	...
   	schedule_work(&testwork);	// 调度 work
   	...
   }
   
   /* 驱动入口函数 */
   static int __init xxx_init(void)
   {
   	...
   	INIT_WORK(&testwork, testwork_func_t);	// 初始化 work
   	request_irq(xxx_irq, test_handler, 0, "xxx", &xxx_dev);
   	...
   }
```

### 3. 设备树中断信息节点

> #interrupt-cells：指定中断源的信息 cells 个数
>
> interrupt-controller：表示当前节点为中断控制器
>
> interrupts：指定终端号、触发方式等
>
> interrupt-parent：指定父中断，也就是中断控制器

```c
intc: interrupt-controller@00a01000 {
	compatible = "arm,cortex-a7-gic";		// 用来匹配 GIC 中断控制器驱动
    #interrupt-cells = <3>;		// 表示中断控制器下设备的 cells 大小
    interrupt-controller;		// 表示当前节点是中断控制器
    reg = <0x00a01000 0x1000>,
    	<0x00a02000 0x100>;
};
-------------------------------------------------------------------
interrupt-cells 表示对于 ARM 处理的 GIC 来说，一共有3个cells
第一个cells：中断类型，0 表示 SPI 中断， 1 表示 PPI 中断
第二个cells：中断号，对于 SPI 中断来说中断号的范围为 0~987，对于 PPI 中断来说中断号的范围为 0~15
第三个 cells：标志，bit[3:0]表示中断触发类型，为 1 的时候表示上升沿触发，为 2 的时候表示下降沿触发，为 4 的时候表示高电平触发，为 8 的时候表示低电平触发。 bit[15:8]为 PPI 中断的 CPU 掩码
```

gpio 节点也可以作为中断控制器

```c
/* imx6ull.dts */
gpio5: gpio@020ac000 {
	compatible = "fsl,imx6ul-gpio", "fsl,imx35-gpio";
    reg = <0x020ac000 0x4000>;
    interrupts = <GIC_SPI 74 IRQ_TYPE_LEVEL_HIGH>,	// 中断类型SPI，中断源74，触发电平LEVEL_HIGH
    			<GIC_SPI 75 IRQ_TYPE_LEVEL_HIGH>;	
    gpio-controller;
    #gpio-cells = <2>;
    interrupt-controller;		// 表示中断控制器
    #interrupt-cells = <2>;		// 下面的中断控制器 有 2 个 cells
};
-----------------------------------------------------
```

![1685080442438](E:\typora\markdownImage\1685080442438.png)

```c
/* imx6ill-alientek-emmc.dts */
fxls8471@1e {
	compatible = "fsl,fxls8471";
    reg = <0x1e>;
    position = <0>;
    interrupt-parent = <&gpio5>; // 设置中断控制器，这里使用 gpio5 作为中断控制器
    interrupts = <0 8>;	  // 表示中断信息，0表示 GPIO5_IO00，8 表示低电平触发
};
```

### 3.1 获取中断号（从设备树中）

将中断信息写入设备树里面，通过相应的函数从 interupts 属性中提取到对应的设备号

```c
unsigned int irq_of_parse_and_map(struct device_node *dev, int index)
dev:设备节点
index：interupts 属性中可能包含多条中断信息，通过 index 指定获取信息

如果使用GPIO，通过 gpio_to_irq 函数来获取gpio对应的中断号
int gpio_to_irq(unsigned int gpio)
gpio：要获取的 gpio 编号
返回值：GPIO 对应的中断号
```

## 4.硬件原理图

采用中断的方式，并且采用定时器来实现按键消抖，应用程序读取按键值并通过终端打印出来

![1685080933217](E:\typora\markdownImage\1685080933217.png)

## 5. 修改设备树

按键 KEY0 使用中断模式，需要在 key 节点下添加中断相关属性

```c
key {
	#address-cells = <1>;
    #size-cells = <1>;
    compatible = "atkalpha-key";
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_key>;
    key-gpio = <&gpio1 18 GPIO_ACTIVE_LOW>; /* KEY0 */
    interrupt-parent = <&gpio1>;	// 指定中断控制器
    interrupts = <18 IRQ_TYPE_EDGE_BOTH>; 	/* FALLING RISING */ // 所用 GPIO1_IO18
    status = "okay";
};
```

```c
/* include/linux/irq.h */
enum {
	IRQ_TYPE_NONE = 0x00000000,
    IRQ_TYPE_EDGE_RISING = 0x00000001,
    IRQ_TYPE_EDGE_FALLING = 0x00000002,
    IRQ_TYPE_EDGE_BOTH = (IRQ_TYPE_EDGE_FALLING |
						IRQ_TYPE_EDGE_RISING),
    IRQ_TYPE_LEVEL_HIGH = 0x00000004,
    IRQ_TYPE_LEVEL_LOW = 0x00000008,
    IRQ_TYPE_LEVEL_MASK = (IRQ_TYPE_LEVEL_LOW |
						IRQ_TYPE_LEVEL_HIGH),
    ...
};
```

## 6. 驱动程序编写

```c
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define IMX6UIRQ_CNT 1 				/* 设备号个数 */
#define IMX6UIRQ_NAME "imx6uirq" 		/* 名字 */
#define KEY0VALUE 0X01 				/* KEY0 按键值 */
#define INVAKEY 0XFF 				/* 无效的按键值 */
#define KEY_NUM 1 					/* 按键数量 */

/* 中断 IO 描述结构体 */
struct irq_keydesc {
    int gpio; /* gpio */
    int irqnum; /* 中断号 */
    unsigned char value; /* 按键对应的键值 */
    char name[10]; /* 名字 */
    irqreturn_t (*handler)(int, void *); /* 中断服务函数 */
};

/* imx6uirq 设备结构体 */
struct imx6uirq_dev{
    dev_t devid; 	/* 设备号 */
    struct cdev cdev; 	/* cdev */
    struct class *class; 	/* 类 */
    struct device *device; 	/* 设备 */
    int major; 				/* 主设备号 */
    int minor; 				/* 次设备号 */
    struct device_node *nd; 	/* 设备节点 */
    atomic_t keyvalue; 			/* 有效的按键键值 */
    atomic_t releasekey; 		/* 标记是否完成一次完成的按键*/
    struct timer_list timer; 	/* 定义一个定时器*/
    struct irq_keydesc irqkeydesc[KEY_NUM]; 	/* 按键描述数组 */
    unsigned char curkeynum; 	/* 当前的按键号 */
};

struct imx6uirq_dev imx6uirq; 	/* irq 设备 */

/* 中断服务函数，开启定时器，延时 10ms */
static irqreturn_t key0_handler(int irq, void *dev_id)
{
    struct imx6uirq_dev *dev = (struct imx6uirq_dev *)dev_id;
    dev->curkeynum = 0;
    dev->timer.data = (volatile long)dev_id;
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));
    return IRQ_RETVAL(IRQ_HANDLED);
}

/* 定时器服务函数，用于按键消抖，定时器到了以后
* 再次读取按键值，如果按键还是处于按下状态就表示按键有效 */
void timer_function(unsigned long arg)
{
    unsigned char value;
    unsigned char num;
    struct irq_keydesc *keydesc;
    struct imx6uirq_dev *dev = (struct imx6uirq_dev *)arg;
    num = dev->curkeynum;
    keydesc = &dev->irqkeydesc[num];
    value = gpio_get_value(keydesc->gpio); /* 读取 IO 值 */
    if(value == 0){ /* 按下按键 */
        atomic_set(&dev->keyvalue, keydesc->value);
    }
    else{ /* 按键松开 */
        atomic_set(&dev->keyvalue, 0x80 | keydesc->value);
        atomic_set(&dev->releasekey, 1); /* 标记松开按键 */
    }
}

/* 按键 IO 初始化 */
static int keyio_init(void)
{
	unsigned char i = 0;
    
    int ret = 0;
    
    imx6uirq.nd = of_find_node_by_path("/key");
    if (imx6uirq.nd== NULL)
    {
        printk("key node not find!\r\n");
        return -EINVAL;
    }
    
    /* 提取 GPIO */
    for (i = 0; i < KEY_NUM; i++)
    {
        imx6uirq.irqkeydesc[i].gpio = of_get_named_gpio(imx6uirq.nd, "key-gpio", i);
        if (imx6uirq.irqkeydesc[i].gpio < 0)
        {
            printk("can't get key%d\r\n", i);
        }
    }
    
    /* 初始化 key 所使用的 IO，并且设置成中断模式 */
    for (i = 0; i < KEY_NUM; i++)
    {
        memset(imx6uirq.irqkeydesc[i].name, 0,
				sizeof(imx6uirq.irqkeydesc[i].name));
        sprintf(imx6uirq.irqkeydesc[i].name, "KEY%d", i);
        gpio_request(imx6uirq.irqkeydesc[i].gpio, imx6uirq.irqkeydesc[i].name);
        gpio_direction_input(imx6uirq.irqkeydesc[i].gpio);
        imx6uirq.irqkeydesc[i].irqnum = irq_of_parse_and_map(imx6uirq.nd, i);
#if 0
    imx6uirq.irqkeydesc[i].irqnum = gpio_to_irq(imx6uirq.irqkeydesc[i].gpio);
#endif
    printk("key%d:gpio=%d, irqnum=%d\r\n",i,
									imx6uirq.irqkeydesc[i].gpio,
									imx6uirq.irqkeydesc[i].irqnum);    
    }
    /* 申请中断 */
    imx6uirq.irqkeydesc[0].handler = key0_handler;
    imx6uirq.irqkeydesc[0].value = KEY0VALUE;
    
    for (i = 0; i < KEY_NUM; i++)
    {
        ret = request_irq(imx6uirq.irqkeydesc[i].irqnum,
						imx6uirq.irqkeydesc[i].handler,
						IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
						imx6uirq.irqkeydesc[i].name, &imx6uirq);
        if(ret < 0)
        {
            printk("irq %d request failed!\r\n", imx6uirq.irqkeydesc[i].irqnum);
            return -EFAULT;
        }
    }
    
    /* 创建定时器 */
    init_timer(&imx6uirq.timer);
    imx6uirq.timer.function = timer_function;
    return 0;  
}

/* 打开设备 */
static int imx6uirq_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &imx6uirq; /* 设置私有数据 */
    return 0;
}
/* 从设备读取数据 */
static ssize_t imx6uirq_read(struct file *filp, char __user *buf,
							size_t cnt, loff_t *offt)
{
    int ret = 0;
    unsigned char keyvalue = 0;
    unsigned char releasekey = 0;
    struct imx6uirq_dev *dev = (struct imx6uirq_dev *)filp->private_data;
    
    keyvalue = atomic_read(&dev->keyvalue);
    releasekey = atomic_read(&dev->releasekey);
    
    if (releasekey)
    {
        if (keyvalue & 0x80)
        {
            keyvalue &= ~0x80;
            ret = copy_to_user(buf, &keyvalue, sizeof(keyvalue));
        }
        else
        {
            goto data_error;
        }
        atomic_set(&dev->releasekey, 0); 	/* 按下标志清零 */
    }
    else
    {
        goto data_error;
    }
    return 0;
data_error:
    return -EINVAL;
}    

 /* 设备操作函数 */
static struct file_operations imx6uirq_fops = {
    .owner = THIS_MODULE,
    .open = imx6uirq_open,
    .read = imx6uirq_read,
};

/* 驱动入口函数 */
static int __init imx6uirq_init(void)
{
    /* 1、构建设备号 */
    if (imx6uirq.major)
    {
        imx6uirq.devid = MKDEV(imx6uirq.major, 0);
        register_chrdev_region(imx6uirq.devid, IMX6UIRQ_CNT, IMX6UIRQ_NAME);
    }
    else
    {
        alloc_chrdev_region(&imx6uirq.devid, 0, IMX6UIRQ_CNT, IMX6UIRQ_NAME);
        imx6uirq.major = MAJOR(imx6uirq.devid);
        imx6uirq.minor = MINOR(imx6uirq.devid);
    }
    
    /* 2、注册字符设备 */
    cdev_init(&imx6uirq.cdev, &imx6uirq_fops);
    cdev_add(&imx6uirq.cdev, imx6uirq.devid, IMX6UIRQ_CNT);
    
    /* 3、创建类 */
    imx6uirq.class = class_create(THIS_MODULE, IMX6UIRQ_NAME);
    if (IS_ERR(imx6uirq.class))
    {
        return PTR_ERR(imx6uirq.class);
    }
    
    /* 4、创建设备 */
    imx6uirq.device = device_create(imx6uirq.class, NULL,
									imx6uirq.devid, NULL, IMX6UIRQ_NAME);
    if (IS_ERR(imx6uirq.device)) 
    {
        return PTR_ERR(imx6uirq.device);
    }
    
    /* 5、 初始化按键 */
    atomic_set(&imx6uirq.keyvalue, INVAKEY);
    atomic_set(&imx6uirq.releasekey, 0);
    keyio_init();
    return 0;
}

/* 驱动出口函数 */
static void __exit imx6uirq_exit(void)
{
    unsigned int i = 0;
    /* 删除定时器 */
    del_timer_sync(&imx6uirq.timer);
    
    /* 释放中断 */
    for (i = 0; i < KEY_NUM; i++)
    {
        free_irq(imx6uirq.irqkeydesc[i].irqnum, &imx6uirq);
        gpio_free(imx6uirq.irqkeydesc[i].gpio);
    }
    cdev_del(&imx6uirq.cdev);
    unregister_chrdev_region(imx6uirq.devid, IMX6UIRQ_CNT);
    device_destroy(imx6uirq.class, imx6uirq.devid);
    class_destroy(imx6uirq.class);
}

module_init(imx6uirq_init);
module_exit(imx6uirq_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lvqifeng");
```

### 7. 测试APP

```c
/* imx6uirqAPP.c */
#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "linux/ioctl.h"

int main(int argc, char *argv[])
{
    int fd;
    int ret = 0;
    char *filename;
    unsigned char data;
    if (argc != 2) 
    {
        printf("Error Usage!\r\n");
        return -1;
    }
    filename = argv[1];
    fd = open(filename, O_RDWR);
    if (fd < 0) 
    {
        printf("Can't open file %s\r\n", filename);
        return -1;
    }
    while (1) 
    {
        ret = read(fd, &data, sizeof(data));
        if (ret < 0) 
        { /* 数据读取错误或者无效 */     
        }
        else 
        { /* 数据读取正确 */
            if (data) /* 读取到数据 */
                printf("key value = %#X\r\n", data);
		}
    }
    close(fd);
    return ret;
}

```

```shell
depmod
modprobe imx6uirq.ko //加载驱动
cat /proc/interrupts	// 检查对应中断有没有被注册
./imx6uirqApp /dev/imx6uirq
rmmod imx6uirq.ko
```
