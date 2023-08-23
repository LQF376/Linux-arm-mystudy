## 线程

- 参与系统调度的最小单位，包含在进程中，是进程的实际运行单位
- 同一进程中的多个线程共享该进程中的全部系统资源，如虚拟地址空间、文件描述符、信号处理等，但多个线程拥有各自的调用栈，自己的寄存器环境、自己线程的本地存储

__线程 ID__

```c
#include<pthread.h>

pthread_t pthread_self(void);
返回：线程ID
```

```c
/* 检查两个线程ID 是否相等 */
#include <pthread.h>

int pthread_equal(pthread_t t1, pthread_t t2);
返回值：相等：非零值     不相等：0
```

__创建线程__

```c
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
thread： 返回新创建的线程ID
attr：NULL 默认属性
start_routine：函数指针，新创建的线程从 start_routine 函数开始
arg：传入 start_routine 的函数参数
返回值： 成功 0 	失败 错误号
```

注意：创建出线程后，无法保证新创建出来的线程和之前的主线程之间的顺序问题，调度算法

```c
/* 创建线程 demo */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>

static void *new_thread_start(void *arg)
{
	printf("新线程: 进程 ID<%d> 线程 ID<%lu>\n", getpid(), pthread_self());
	return (void *)0;
}

int main(void)
{
	pthread_t tid;
	int ret;

	ret = pthread_create(&tid, NULL, new_thread_start, NULL);
	if (ret) 
	{
		fprintf(stderr, "Error: %s\n", strerror(ret));
		exit(-1);
	}

	printf("主线程: 进程 ID<%d> 线程 ID<%lu>\n", getpid(), pthread_self());
	sleep(1);
	exit(0);
}
```

__终止线程__

线程创建后，从start_routine函数进行开始，终止线程的方式有以下几种：

1. 线程的 start 函数执行 return 语句并返回指定值，返回值就是线程的退出码； 
2. 线程调用 pthread_exit()函数； 
3. 调用 pthread_cancel()取消线程 ；

```c
#include <pthread.h>

void pthread_exit(void *retval);
retval：线程的退出码，该返回值可以由另一个线程通过 pthread_join() 来获取，return 同理
```

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>

static void *new_thread_start(void *arg)
{
	printf("新线程 start\n");
	sleep(1);
	printf("新线程 end\n");
	pthread_exit(NULL);
}

int main(void)
{
	pthread_t tid;
	int ret;
	
	ret = pthread_create(&tid, NULL, new_thread_start, NULL);
	if (ret) 
	{
		fprintf(stderr, "Error: %s\n", strerror(ret));
		exit(-1);
	}

	printf("主线程 end\n");
	pthread_exit(NULL);
	exit(0);
}
```

__回收线程__

调用 pthread_join() 来阻塞等待线程的终止，并获取线程的退出码，回收线程资源

```c
#include <pthread.h>

int pthread_join(pthread_t thread, void **retval);
thread：指定需要等待的线程
retval：不为 NULL，则 pthread_join()将目标线程的退出状态（即目标线程通过 pthread_exit()退出时指定的返回值或者在线程 start 函数中执行 return 语句对应的返回值）复制到*retval 所指向的内存区域；目标线程被pthread_cancel() 取消， 则将 PTHREAD_CANCELED 放在*retval 中。
返回值： 成功：0    失败：返回错误码（非errno）

注：如果该线程已经终止，则 pthread_join() 立刻返回。
   若线程并未分离（detached，将在 11.6.1 小节介绍），则必须使用 pthread_join()来等待线程终止，回收线程资源；如果线程终止后，其它线程没有调用 pthread_join()函数来回收该线程，那么该线程将变成僵尸线程。
```

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>

static void *new_thread_start(void *arg)
{
	printf("新线程 start\n");
	sleep(2);
	printf("新线程 end\n");
	pthread_exit((void *)10);
}

int main(void)
{
	pthread_t tid;
	void *tret;
	int ret;
	
	ret = pthread_create(&tid, NULL, new_thread_start, NULL);
	if(ret)
	{
		fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
		exit(-1);
	}
	
	ret = pthread_join(tid, &tret);
	if(ret)
	{
		fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
		exit(-1);
	}
	printf("新线程终止, code=%ld\n", (long)tret);
	
	exit(0);
}
```

__取消线程__

- 向指定的线程发送一个请求，要求其立刻停止、退出

```c
#include <pthread.h>

int pthread_cancel(pthread_t thread);
注：pthread_cancel()并不会等待线程终止，仅仅只是提出请求，线程可以自己设置不被取消
表现为调用了参数为 PTHREAD_CANCELED（其实就是(void *)-1） 的 pthread_exit()函数，
```

```c
/* 线程取消 demo */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>

static void *new_thread_start(void *arg)
{
	printf("新线程--running\n");
	for(;;)
		sleep(1);
	return (void *)0;
}

int main(void)
{
	pthread_t tid;
	void *tret;
	int ret;
	
	/* 创建新线程 */
	ret = pthread_create(&tid, NULL, new_thread_start, NULL);
	if(ret)
	{
		fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
		exit(-1);
	}
	
	sleep(1);
	
	/* 向新线程发送取消请求 */
	ret = pthread_cancel(tid);
	if (ret) 
	{
		fprintf(stderr, "pthread_cancel error: %s\n", strerror(ret));
		exit(-1);
	}
	
	/* 等待新线程终止 */
	ret = pthread_join(tid, &tret);
	if (ret) 
	{
		fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
		exit(-1);
	}
	printf("新线程终止, code=%ld\n", (long)tret);
	
	exit(0);
}
```

__修改取消类型和状态__

```c
#include <pthread.h>

/* 设置取消性状态和获取旧状态操作（原子操作）*/
int pthread_setcancelstate(int state, int *oldstate);
state：取消状态设置为 state
	PTHREAD_CANCEL_ENABLE：线程可以取消（新创建线程的默认值）
	PTHREAD_CANCEL_DISABLE：线程不可被取消
oldstate：旧状态返回，允许设置 NULL
返回值： 成功0  失败 非零值的错误码

/* 设置线程的取消性类型 */
/* 程的取消性状态为 PTHREAD_CANCEL_ENABLE，那么对取消请求的处理则取决于线程的取消性类型 */
int pthread_setcanceltype(int type, int *oldtype);
type：要设置的取消性类型
	PTHREAD_CANCEL_DEFERRED：线程还是继续运行，取消请求被挂起，直到线程到达某个取消点（cancellation point，将在 11.6.3 小节介绍） 为止（默认取消性类型）
	PTHREAD_CANCEL_ASYNCHRONOUS：可能会在任何时间点（也许是立即取消，但不一定）取消线程


```

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>

static void *new_thread_start(void *arg)
{
	/* 设置为不可被取消 */
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	
	for ( ; ; )
	{
		printf("新线程--running\n");
		sleep(2);
	}
	return (void *)0;
}

int main(void)
{
	pthread_t tid;
	void *tret;
	int ret;
	
	/* 创建新线程 */
	ret = pthread_create(&tid, NULL, new_thread_start, NULL);
	if(ret)
	{
		fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
		exit(-1);
	}
	
	sleep(1);
	
	/* 向新线程发送取消请求 */
	ret = pthread_cancel(tid);
	if(ret)
	{
		fprintf(stderr, "pthread_cancel error: %s\n", strerror(ret));
		exit(-1);
	}
	
	/* 等待新线程终止 */
	ret = pthread_join(tid, &tret);
	if(ret)
	{
		fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
		exit(-1);
	}
	printf("新线程终止, code=%ld\n", (long)tret);
	
	exit(0);
}
```

__取消点__

- 一系列函数，当执行到这些函数的时候，才会真正的相应取消请求
- man 7 pthreads

__添加一个取消点__

```c
#include <pthread.h>

/* 当前位置添加一个取消点，如果已经被挂起，则随之终止 */
void pthread_testcancel(void);
```

```c
/* 测试取消点 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>

static void *new_thread_start(void *arg)
{
	printf("新线程--start run\n");
	for ( ; ; ) 
	{
		// pthread_testcancel();
	}
	return (void *)0;
}

int main(void)
{
	pthread_t tid;
	void *tret;
	int ret;
	
	/* 创建新线程 */
	ret = pthread_create(&tid, NULL, new_thread_start, NULL);
	if(ret)
	{
		fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
		exit(-1);
	}
	
	sleep(1);
	
	/* 向新线程发送取消请求 */
	ret = pthread_cancel(tid);
	if(ret)
	{
		fprintf(stderr, "pthread_cancel error: %s\n", strerror(ret));
		exit(-1);
	}
	
	/* 等待新线程终止 */
	ret = pthread_join(tid, &tret);
	if(ret)
	{
		fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
		exit(-1);
	}
	printf("新线程终止, code=%ld\n", (long)tret);
	
	exit(0);
}
```

__分离线程__

- 若并不关心线程的返回状态，希望系统在线程终止时能够自动回收线程资源并将其移除，即可以将线程分离

```c
#include <pthread.h>

int pthread_detach(pthread_t thread);
thread：指定要分离的线程
返回值： 成功 0；失败 错误码
```

- 一个线程可以将另一个线程分离，也可以将自己分离

```c
pthread_detach(pthread_self());
```

- 一旦线程处于分离状态，就不能再使用 pthread_join()来获取其终止状态， 此过程是不可逆的 

```c
/* 线程分离 demo */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>

static void *new_thread_start(void *arg)
{
	int ret;
	
	/* 自行分离 */
	ret = pthread_detach(pthread_self());
	if(ret)
	{
		fprintf(stderr, "pthread_detach error: %s\n", strerror(ret));
		return NULL;
	}
	
	printf("新线程 start\n");
	sleep(2); //休眠 2 秒钟
	printf("新线程 end\n");
	pthread_exit(NULL);
}

int main(void)
{
	pthread_t tid;
	int ret;
	
	/* 创建新线程 */
	ret = pthread_create(&tid, NULL, new_thread_start, NULL);
	if (ret) 
	{
		fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
		exit(-1);
	}
	
	sleep(1); //休眠 1 秒钟
	
	/* 等待新线程终止 */
	ret = pthread_join(tid, NULL);
	if(ret)
		fprintf(stderr, "pthread_join error: %s\n", strerror(ret));	// 出错
	
	pthread_exit(NULL);
}
```

__注册线程清理处理函数__

- 当线程终止退出时，去执行线程清理函数
- 一个线程可以注册多个清理函数，清理函数记录在栈中，每个线程都可以拥有一个清理函数栈，栈的执行顺序与注册（添加）顺序相反，执行完所有清理函数后，线程终止

清理函数栈中的清理函数被调用的3种情况：

1. 线程调用 pthread_exit()退出时； 
2. 线程响应取消请求时； 
3. 用非 0 参数调用 pthread_cleanup_pop() 

```c
#include <pthread.h>

/* 向调用线程的清理函数栈中添加 清理函数 */
void pthread_cleanup_push(void (*routine)(void *), void *arg);
routine：函数指针
arg：向清理函数传递的参数

/* 从栈中移除 清理函数 */
void pthread_cleanup_pop(int execute);
execute：
	0：只是从栈顶部中清除，并不调用
	非0：除了将清理函数栈中顶层的函数移除，还会清理寒素
```

```c
/* 线程清理函数 demo */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>

static void cleanup(void *arg)
{
	printf("cleanup: %s\n", (char *)arg);
}

static void *new_thread_start(void *arg)
{
	printf("新线程--start run\n");
	pthread_cleanup_push(cleanup, "第 1 次调用");
	pthread_cleanup_push(cleanup, "第 2 次调用");
	pthread_cleanup_push(cleanup, "第 3 次调用");
	
	sleep(2);
	pthread_exit((void *)0); //线程终止
	
	/* 为了与 pthread_cleanup_push 配对,不添加程序编译会通不过 */
	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);
}

int main(void)
{
	pthread_t tid;
	void *tret;
	int ret;
	
	/* 创建新线程 */
	ret = pthread_create(&tid, NULL, new_thread_start, NULL);
	if(ret)
	{
		fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
		exit(-1);
	}
	
	/* 等待新线程终止 */
	ret = pthread_join(tid, &tret);
	if (ret) 
	{
		fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
		exit(-1);
	}
	printf("新线程终止, code=%ld\n", (long)tret);
	
	exit(0);
}
```

__线程属性__

- 调用 pthread_create() 创建线程， 可对新建线程的各种属性进行设置。 
- Linux下，使用 pthread_attr_t 数据类型定义线程的所有属性
- 线程创建时，若需要对线程的属性进行更改，需初始化 pthread_attr_t 结构体，不使用时并销毁

```c
#include <pthread.h>

/* 初始化属性结构体 */
int pthread_attr_init(pthread_attr_t *attr);
返回值： 成功0； 失败 非0错误码
/* 销毁属性结构体 */
int pthread_attr_destroy(pthread_attr_t *attr);
```

1. 线程栈属性

   - 每个线程都有自己栈空间，pthread_attr_t 数据结构中定义了栈的起始地址以及栈大小， 

   ```c
   #include <pthread.h>
   
   /* 设置栈空间 */
   int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize);
   attr：线程属性对象
   stackaddr：栈的起始地址
   stacksize：栈的大小
   返回值： 成功 0 失败：非0 错误码
   /* 获取栈空间信息 */
   int pthread_attr_getstack(const pthread_attr_t *attr, void **stackaddr, size_t *stacksize);
   attr：线程属性对象
   stackaddr：返回栈的起始地址
   stacksize：返回栈的大小
   返回值： 成功 0 失败：非0 错误码
   ```

   ```c
   /* 单独获取或者设置 */
   #include <pthread.h>
   
   int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
   int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);
   int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr);
   int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr);
   ```

   ```c
   /* 设置线程栈大小为 4 Kb demo*/
   #include <stdio.h>
   #include <stdlib.h>
   #include <pthread.h>
   #include <string.h>
   
   static void *new_thread_start(void *arg)
   {
   	puts("Hello World!");
   	return (void *)0;
   }
   
   int main(int argc, char *argv[])
   {
   	pthread_attr_t attr;
   	size_t stacksize;
   	pthread_t tid;
   	int ret;
   	
   	/* 对 attr 对象进行初始化 */
   	pthread_attr_init(&attr);
   	
   	/* 设置栈大小为 4K */
   	pthread_attr_setstacksize(&attr, 4096);
   	
   	/* 创建新线程 */
   	ret = pthread_create(&tid, &attr, new_thread_start, NULL);
   	if(ret)
   	{
   		fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
   		exit(-1);
   	}
   	
   	/* 等待新线程终止 */
   	ret = pthread_join(tid, NULL);
   	if(ret)
   	{
   		fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
   		exit(-1);
   	}
   	
   	/* 销毁 attr 对象 */
   	pthread_attr_destroy(&attr);
   	exit(0);
   }
   ```

2. 分离状态属性

   - 创建线程时，可以通过设置 detachstate 线程属性，让线程一开始就处于分离状态

   ```c
   #include <pthread.h>
   
   int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
   int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
   detachstate：分离状态属性
   	PTHREAD_CREATE_DETACHED：新建线程一开始运行便处于分离状态， 以分离状态启动线程
   	PTHREAD_CREATE_JOINABLE：默认属性，正常启动线程
   ```

   ```c
   /* 设置线程分离属性 demo */
   #include <stdio.h>
   #include <stdlib.h>
   #include <pthread.h>
   #include <string.h>
   #include <unistd.h>
   
   static void *new_thread_start(void *arg)
   {
   	puts("Hello World!");
   	return (void *)0;
   }
   
   int main(int argc, char *argv[])
   {
   	pthread_attr_t attr;
   	pthread_t tid;
   	int ret;
   	
   	/* 对 attr 对象进行初始化 */
   	pthread_attr_init(&attr);
   	
   	/* 设置以分离状态启动线程 */
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
   	
   	/* 创建新线程 */
   	ret = pthread_create(&tid, &attr, new_thread_start, NULL);
   	if (ret) 
   	{
   		fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
   		exit(-1);
   	}
   	
   	sleep(1);
   	
   	/* 销毁 attr 对象 */
   	pthread_attr_destroy(&attr);
   	exit(0);
   }
   ```

__线程安全__

- 每个线程都有自己独立的栈地址空间，每个线程在运行过程中所定义的局部变量都是分配在自己的线程栈中的，互不干扰

```c
/* demo */
/* 线程安全 互不干扰 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static void *new_thread(void *arg)
{
	int number = *((int *)arg);
	unsigned long int tid = pthread_self();
	printf("当前为<%d>号线程, 线程 ID<%lu>\n", number, tid);
	return (void *)0;
}

static int nums[5] = {0, 1, 2, 3, 4};

int main(int argc, char *argv[])
{
	pthread_t tid[5];
	int j;

	/* 创建 5 个线程 */
	for (j = 0; j < 5; j++)
		pthread_create(&tid[j], NULL, new_thread, &nums[j]);
	
	/* 等待线程结束 */
	for (j = 0; j < 5; j++)
		pthread_join(tid[j], NULL);//回收线程
	
	exit(0);
}
```

__可重入函数__

- 一个函数被同一进程的多个不同的执行流同时调用，每次函数调用都能产生正确的结果（预期结果）

多个执行流同时调用的两种情况：

1. 含有信号处理的程序中，主程序调用函数，此时接收信号，主程序打断，信号处理函数同时也调用此函数
2. 多线程并发调用同一个函数

可重入函数分为绝对的可重入函数和带条件的可重入函数（调全局变量还是局部变量问题）

线程安全函数：一个函数被多个线程（其实也是多个执行流，但是不包括由信号处理函数所产生的执行流） 同时调用时，它总会一直产生正确的结果。__可重入函数一定是线程安全函数，但线程安全函数不一定是可重入函数__

 __一次性初始化__

```c
#include <pthread.h>

/* 多线程编程中，只执行一次 */
pthread_once_t once_control = PTHREAD_ONCE_INIT;
int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));
init_routine：函数仅执行一次，究竟在哪个线程中执行不定
返回值：成功0； 失败 非零错误码
```

如果在一个线程调用 pthread_once()时，另外一个线程也调用了 pthread_once，则该线程将会被阻塞等待， 直到第一个完成初始化后返回。 

```c
/* demo 初始化一次 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void initialize_once(void)
{
	printf("initialize_once 被执行: 线程 ID<%lu>\n", pthread_self());
}

static void func(void)
{
	pthread_once(&once, initialize_once);//执行一次性初始化函数
	printf("函数 func 执行完毕.\n");
}

static void *thread_start(void *arg)
{
	printf("线程%d 被创建: 线程 ID<%lu>\n", *((int *)arg), pthread_self());
	func(); //调用函数 func
	pthread_exit(NULL); //线程终止
}

static int nums[5] = {0, 1, 2, 3, 4};

int main(void)
{
	pthread_t tid[5];
	int j;
	
	/* 创建 5 个线程 */
	for (j = 0; j < 5; j++)
		pthread_create(&tid[j], NULL, thread_start, &nums[j]);
	
	/* 等待线程结束 */
	for (j = 0; j < 5; j++)
		pthread_join(tid[j], NULL);//回收线程
		
	exit(0);
}
```

__线程特有数据（线程私有数据）__

- 为每一个调用线程分配属于该线程的私有数据区，为每个线程分别维护一份变量的副本

```c
/* 创建一个特有数据键 key （只要在首个调用的线程中创建一次即可） */
#include <pthread.h>

int pthread_key_create(pthread_key_t *key, void (*destructor)(void*));
key：返回 创建的一个特有数据键 key，调用函数前需要先创建一个 struct pthread_key_t *key
destructor：函数指针，指向自定义的析构函数，通常用于释放与特有数据键关联的线程私有数据区占用的内存空间，当使用线程特有数据的线程终止时， destructor()函数会被自动调用。
返回值：成功0；失败：错误编号
```

```c
#include <pthread.h>

int pthread_setspecific(pthread_key_t key, const void *value);
key：特有数据键
value：指向由调用着分配的一块内存，作为线程的私有数据缓冲去
返回值：成功0； 失败：错误码
```

```c
#include <pthread.h>

/* 获取调用线程的私有数据区 */
void *pthread_getspecific(pthread_key_t key);
返回值：当前调用线程关联到特有数据键的私有数据缓冲区；如果当前调用线程并没有设置线程私有数据缓冲区与特有数据键进行关联，则返回值应为NULL，利用这一点来判断当前调用线程是否为初次调用该函数
```

```c
#include <pthread.h>

/* 删除先前由 pthread_key_create()创建的键。 */
int pthread_key_delete(pthread_key_t key);
返回值：成功0 ；失败 错误码
```

调用删除特有数据键时应保证：

1. 所有线程已经释放了私有数据区（显式调用解构函数或线程终止）。 
2. 参数 key 指定的特有数据键将不再使用。 

__线程局部存储__

- 每个线程都会拥有一份对该变量的备份

```c
/* __thread 修饰 */
static __thread char buf[512];
```

```c
/* demo */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static __thread char buf[100];

static void *thread_start(void *arg)
{
	strcpy(buf, "Child Thread\n");
	printf("子线程: buf (%p) = %s", buf, buf);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	pthread_t tid;
	int ret;
	
	strcpy(buf, "Main Thread\n");
	
	/* 创建子线程 */
	if (ret = pthread_create(&tid, NULL, thread_start, NULL)) 
	{
		fprintf(stderr, "pthread_create error: %d\n", ret);
		exit(-1);
	}
	
	/* 等待回收子线程 */
	if (ret = pthread_join(tid, NULL))
	{
		fprintf(stderr, "pthread_join error: %d\n", ret);
		exit(-1);
	}
	
	printf("主线程: buf (%p) = %s", buf, buf);
	exit(0);
}
```

__信号与线程__

1. 设置个线程的信号掩码（每个刚创建的线程，会从其创建者处继承信号掩码 ）

   ```c
   #include <signal.h>
   
   int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);
   ```

2. 向线程发送信号

   ```c
   #include <signal.h>
   
   int pthread_kill(pthread_t thread, int sig);
   thread：用于指定哪个线程
   sig：哪个信号
   返回值：成功0；失败 错误码
   ```

   ```c
   #include <signal.h>
   #include <pthread.h>
   
   int pthread_sigqueue(pthread_t thread, int sig, const union sigval value);
   ```

   __异步信号安全函数__：可以在信号处理函数中可以被安全调用的线程安全函数 


