## 友元

## 1. 类做友元

- 一个类作友元，则所有成员函数都可以访问

```cpp
class Tv
{
	friend class Remote;
	...
};

class Remote
{
	...
};
```

### 成员函数做友元

- 将类中的某个成员函数做友元

```cpp
class Tv
{
	friend void Remote::set_chan(Tv & t, int c);
}
```

成员函数做友元须注意，声明的顺序（前向声明）

```cpp
class Tv;
class Remote {...};
class Tv {...};
```

### 彼此包含友元类

```cpp
class Tv
{
	friend class Remote;
public:
	void buzz(Remote & t);			// 声明写在前面，定义一定要写在后面
	...
};

class Remote
{
	friend class Tv;
public:
	void Bool volup(Tv & t) {t.volup();}
	...
};

inline void Tv::buzz(Remote & r)		// 注意顺序问题，定义要写在后面
{
	...
}
```

### 共同的友元函数

```cpp
class Analyzer;
class Probe
{
	friend void sync(Analyzer &a, const Probe &p);
	...
};

class Analyzer
{
	friend void sync(Analyzer &a, const Probe &p);
	...
};

inline void sync(Analyzer &a, const Probe &p)
{...}
```

## 2. 嵌套类

- 在另一个类中声明的类被称为嵌套类，通过类作用域来避免名称混乱
- 包含是指将类对象作为另一个类的成员，区别于嵌套

```cpp
class Queue
{
	class Node
	{
	public:
		Item item;
		Node* next;
		Node(const Item &i):item(i), next(0) {}
	};
	...
};
```

### 嵌套类的作用域和可见性

- 嵌套类位于另一个类的私有部分声明，只有后者知道他存在，程序其他部分不知道Node类存在，对于Queue派生而来的类，Node也是不可见
- 嵌套类位于另一个类的保护部分声明，对于后者来说是可见的，但是外部世界则是不可见的；派生类将知道嵌套类，并可以直接创建这种类型的对象
- 嵌套类位于另一个类的共有部分声明，则允许后者、后者的派生类以及外部世界使用他

想要访问嵌套类，就必须指明作用域

## 3. 异常

- 程序遇到运行阶段错误，导致程序无法正常的运行下去，即发生异常
- 异常是相对较新的功能，老式编译器可能没有实现，有的编译器可能默认关闭，需要注意这种情况

### abort()

- 头文件 cstdlib(stdlib.h)，向标准错误刘发送消息（程序异常终止），然后终止程序
- abort 是否刷新文件缓冲区取决于实现，也可以使用exit，会刷新文件缓冲区，但不显示消息

### 返回错误码

- 通过 return 来返回相应的错误码，进而来表述异常的发生

### 异常机制

- c++异常是队程序运行过程中发生的异常情况提供一种响应，将控制权从程序的一部分传递到另一部分的途径

异常的处理步骤：

1. 引发异常
2. 使用处理程序捕获异常
3. 使用try块

```cpp
#include<iostream>
double hmean(double a, double b);

int main()
{
	double x, y, z;
	
	while(std::cin >> x >> y)
	{
		try{
			z = hmean(x, y);
		}
		catch(const char *s)
		{
			std::cout << s << std::endl;
			continue;
		}
		std::cout << x << y << z << std::endl;
	}
}

double hmean(double a, double b)
{
	if(a == -b)
		throw "bad() hmean() atguments";
	return 2.0 * a * b / (a + b);
}
```

- try 块表示其中特定的异常可能被激活的代码块，他后面跟一个或多个catch块
- catch关键字表示捕获异常，指出异常处理程序要响应的异常类型，执行异常处理程序块
- throw 关键字表示引发异常，紧随后面的值指出了异常的特征

------

- 执行完try之后，如果没有引起任何异常，则程序跳过try块后面的catch块，直接执行处理程序后面的第一条语句
- 如果函数引发了异常，而没有try块或没有匹配的处理程序时，程序最终将调用abort() 函数

### 将对象作为异常类型

将异常类型定义为一个类，用不同类来表示不同函数在不同情况下引发的异常；用throw发送异常

### 异常规范和c++11

- 异常规范，是c++98新增的一项功能，以后可能会被抛弃，慎用！！！！
- 目的是为了告诉用户可能要使用 try 块

```cpp
double harm(double a) throw(bad_thing);			// 指明函数会引发 bad_thing 异常
double marm(double) throw();					// 不会引发异常
```

- c++11 使用新增的关键字 noexcept 指出函数不会引发异常

```
double marm() noexcept;				// 不会引发异常
```

### 栈解退

- try 块没有直接调用引发异常的函数，而是调用了对引发异常的函数进行调用的函数，则程序流程将从引发异常的函数跳到包含 try 块和处理程序的函数
- 发生异常后，程序也将释放栈中的内存，但不会在释放栈的一个返回地址后停止，而是继续释放栈，直到找到一个位于 try 块中的返回地址，控制权将转到块尾的异常处理程序，而不函数调用后面的第一条语句

```cpp
class problem {...};

void super() throw(problem)
{
	...
	if(oh_no)
	{
		problem oops;
		throw oops;
	}
}

...
try{
	super();
}
catch(problem& p)			// 引发异常时，编译器总是创建一个临时变量
{
	// statements
}
```

### 其他异常特性

- 引发异常时编译器总是创建一个临时拷贝，即使异常规范和catch块中指定的是引用
- 引用还有一个用处可以基类的引用可以调用派生类的对象
- catch只执行一次，需要注意顺序问题，特别在派生类问题处理方面

```cpp
class bad_1 {...};
class bad_2:public bad_1 {...};
class bad_3:public bad_2 {...};

void duper()
{
	if(oh_no)
		throw bad_1();
	if(rats)
		throw bad_2();
	if(drat)
		throw bad_3();
}

...
try{
	duper();
}
catch(bad_3 &be)
{
	// statements
}
catch(bad_2 &be)
{
	// statements
}
catch(bad_1 &be)
{
	// statements
}
```

捕获任何异常

```cpp
catch(...) { //statement}
```

### exception 类

- exception 头文件（exception.h) 中定义了 exception 类，可以作为其他异常类的基类
- 其中有一个 what() 方法，返回一个字符串；虚方法，具体实现得靠派生类来实现

```
try{
	...
}
catch(std::exception &e)
{
	cout << e.what() << endl;
}
```

### stdexcept 异常类

- 头文件 stdexcept 定义了 logic_error 和 runtime_error 类，他们都是以公有方式是从exception派生而来

logic_error：描述典型的逻辑错误

- domain_error：定义域错误
- invalid_argument：意外，非法值
- length_error：超过长度
- out_of_bounds：索引错误

runtime_error：运行期间发生但难以预计和防范的错误

- range_error：上溢或者下溢
- overflow_error：
- underflow_error

### bad_alloc 异常和 new

- c++中 new 可能会引起 bad_alloc 异常，以前当无法分配请求的内存量时，new返回一个空指针

### 异常、类、继承

- 可以从一个异常类派生出另一个
- 在类定义中嵌套异常类声明来组合异常
- 嵌套声明本身可被继承

```cpp
#include<stdexcept>
#include<string>

class Sales
{
public:
	enum { MONTHS = 12 };
	class bad_index :public std::logic_error
	{
	private:
		int bi;
	public:
		explicit bad_index(int ix, const std::string& s = "Index error in Sale object\n");
		int bi_val() const { return bi; }
		virtual ~bad_index() throw() {}
	};
	explicit Sales(int yy = 0);
	Sales(int yy, const double* gr, int n);
	virtual ~Sales() {}
	int Year() const { return year; }
	virtual double operator[](int i)const;
	virtual double& operator[](int i);
private:
	double gross[MONTHS];
	int year;
};


class LabeledSales :public Sales
{
public:
	class nbad_index :public Sales::bad_index
	{
	private:
		std::string lb1;
	public:
		nbad_index(const std::string& lb, int ix, const std::string& s = "Index error in LabeledSales object\n");
		const std::string& Label()const { return lb1; }
		virtual ~nbad_index() throw() {}
	};
	explicit LabeledSales(const std::string& lb = "none", int yy = 0);
	LabeledSales(const std::string& lb, int yy, const double* gr, int n);
	virtual ~LabeledSales() {}
	const std::string& Label() const { return label; }
	virtual double operator[](int i)const;
	virtual double& operator[](int i);
private:
	std::string label;
}; 
```

```cpp
#include"sales.h"

using std::string;

Sales::bad_index::bad_index(int ix, const string &s):std::logic_error(s), bi(ix)
{}

Sales::Sales(int yy)
{
	year = yy;
	for (int i = 0; i < MONTHS; ++i)
		gross[i] = 0;
}

Sales::Sales(int yy, const double* gr, int n)
{
	year = yy;
	int lim = (n < MONTHS) ? n : MONTHS;
	int i;
	for (i = 0; i < lim; ++i)
		gross[i] = gr[i];
	for ( ; i < MONTHS; ++i)
		gross[i] = 0;
}

double Sales::operator[](int i) const
{
	if (i < 0 || i >= MONTHS)
		throw bad_index(i);
	return gross[i];
}

double& Sales::operator[](int i)
{
	if (i < 0 || i >= MONTHS)
		throw bad_index(i);
	return gross[i];
}

LabeledSales::nbad_index::nbad_index(const string& lb, int ix, const string& s) :Sales::bad_index(ix, s)
{
	lb1 = lb;
}

LabeledSales::LabeledSales(const string& lb, int yy) :Sales(yy)
{
	label = lb;
}

LabeledSales::LabeledSales(const string& lb, int yy, const double* gr, int n) :Sales(yy, gr, n)
{
	label = lb;
}

double LabeledSales::operator[](int i)const
{
	if (i < 0 || i >= MONTHS)
		throw nbad_index(Label(), i);
	return Sales::operator[](i);
}

double& LabeledSales::operator[](int i)
{
	if (i < 0 || i >= MONTHS)
		throw nbad_index(Label(), i);
	return Sales::operator[](i);
}
```

### 未捕获异常和意外异常

- 未捕获异常

  - 异常不是在函数中引发的或函数没有异常规范
  - 不会导致程序立刻异常终止，会先调用 terminate()，在默认情况下，terminate() 调用 abort() 函数；可以指定terminate 回调函数

  ```cpp
  typedef void (*terminate_handler) ();
  terminate_handler set_terminate(terminate_handler f) throw();		// c++98
  terminate_handler set_terminate(terminate_handler f) noexcept;		// c++11
  void terminate();							// c++98
  void terminate() noexcept;					// c++11
  ```

  ```cpp
  #include<exception>
  using namespace std;
  
  void myQuit()
  {
  	cout << "Quit" << endl;
      exit(5);
  }
  
  set_terminate(myQuit);
  ```

- 意外异常

  - 异常规范列表中没有匹配的值
  - 程序将调用 unexcepted() 函数，这个函数再调用 terminate()，后者在默认情况下会调用abort()，可以修改unexcepted() 的回调函数

  ```cpp
  typedef void (*unexpected_handler) ();
  unexpected_handler set_unexpected(unexpected_handler f) throw();		// c++98
  unexpected_handler set_unexpected(unexpected_handler f) noexcept;		// c++11
  void unexpected();							// c++98
  void unexpected() noexcept;					// c++11
  ```

  提供给 unexcepted() 的因为收到严格限制，具体说 unexpected() 的函数行为收到严格的限制：

  - 通过调用 terminate() （默认行为）、abort()或者exit() 来终止程序
  - 引发异常（意外异常将由这个异常作为代替）

  ```cpp
  #include<exception>
  using namespace std;
  
  void myUnexpected()
  {
  	throw std::bad_exception();
  }
  
  set_unexpected(myUnexpected);
  ```

### 异常存在问题

- 异常应该在程序设计时考虑，而不是后加；会增加程序代码，影响程序的运行速度
- 异常规范不适用于模板；异常和动态分配内存并非总能协同工作

```cpp
void test2(int n)
{
	double *ar = new double[n];
	...
	if(oh_no)
		throw exception();			// 栈解退会导致内存泄露
	...
	delete [] ar;
	return;
}
```

解决方案：

- 在异常捕获中进行释放
- 使用智能指针

```cpp
void test2(int n)
{
	double *ar = new double[n];
	...
	try{
		if(oh_no)
			throw exception();
	}
	catch(exception & ex)
	{
		delete []ar;
		throw;
	}
	...
	delete [] ar;
	return;
}
```

## 4 RTTI（运行阶段类型识别）

**dynamic_cast、typeid、type_info**

RTTI 解决的问题：

- 在虚函数中，基类的指针可以指向派生类的对象，但是无法知道基类的指针指向的派生类类型；
- 若想要追踪生成的对象类型，RTTI 提供解决方案
- **RTTI只适用于包含虚函数的类**

### RTTI 的工作原理

- dynamic_cast：用来判断类型转换是否安全，即能否将基类指针转化为派生类指针

- 成功则返回转化后的指针类型，失败则返回空指针

  ```
  class Grand {// };
  class Super:public Grand{...};
  class Magnificent:public Superb {...};
  
  Grand *pg = new Grand;
  Grand *ps = new Superb;
  Grand *pm = new Magnificent;
  
  Superb *pm = dynamic_cast<Superb *>(pg);
  ```

- dynamic_cast 用于引用，由于没有空指针，故用异常来表示失败，引发 bad_cast 的异常，该异常是由 exception 类派生而来的，定义在头文件 typeinfo 中定义的

  ```
  #include<typeinfo>
  
  try{
  	Superb & rs = dynamic_cast<Superb &>(rg);
  	...
  }
  catch(bad_cast &)
  {
  	...;
  };
  ```

- typeid 用来确定两个对象是否是同种类型；可以接收两种参数：类名、结果为对象的表达式

- 返回一个对 type_info 对象的引用，定义在头文件 typeinfo 中

- type_info 重载了 == 和 ！= 运算符

- typeid(*p) ；若p是一个空指针，程序将引发 bad_typeid 异常，从exception类派生而来

```cpp
typeid(Magnificent) == typeid(*pg);
```

type_info 类的实现通常随厂家而异，但包含一个 name() 成员，该函数返回一个随实现而异的字符串，通常是类的名称

```cpp
cout << typeid(*pg).name() << endl;
```

### RTTI 使用的注意事项

- 能够使用 dynamic_cast 的时候尽量不要用 typeid 做枚举对比

## 5. 类型转换运算符

创始人认为C语言的强制类型转换较为松散且没有意义，c++提供4种类型转换

- dynamic_cast

  - 在类层次结构中提供向上类型转换

  ```cpp
  dynamic_cast<type-name>(expression)
  ```

- const_cast

  - 只能修改 const、volatile，无法对类的类型进行转换，主要用于 去const类型

  ```cpp
  const_cast<type-name>(expression);
  
  High bar;
  const High * pbar = &bar;
  
  High *pb = const_cast<High *> (pbar);
  const High *pt = &bar;
  ```

- static_cast

  - 仅当 type-name 可被隐式转化为 expression 所属的类型或expression可被隐式转换为type-name所属的类型时，转换才合法
  - 禁止毫无相关类之间的转换

  ```cpp
  High bar;
  Low blow;
  
  High * pb = static_cast<High *> (&blow);
  Low * pl = static_cast<Low *> (&bar);
  ```

- reinterpret_cast

  - 高危操作，不允许删除 const

  ```
  reinterpret_cast<type-name> (expression)
      
  struct dat {short a; short b;};
  long value = 0xA224B118;
  dat *pd = reinterpret<dat *> (&value);
  cout << pd->a << endl;
  ```

  



