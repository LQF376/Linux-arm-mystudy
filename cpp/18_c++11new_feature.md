# 18. C++ 新特性

## 1. C++11新特性

### 新增类型

- C++11 新增类型 long long 和 unsigned long long，以支持64位长整型；
- 新增类型 char16_t 和 char32_t 以支持16位和32位字符表示

### 统一的初始化

- 扩大了 用大括号括起的列表（初始化列表）的使用范围，使其可用于所有内置类型和用户定义的类型；
- 初始化列表时，可添加等号（=），也可不添加

```cpp
int x = {5};
short quar[5] {4, 5, 6, 7};

int * ar = new int[4] {2, 4, 6, 7};
/* 用大括号来初始化对象 */
class stump
{
private:
	int roots;
	double weight;
public:
	Stump(int r, double w): roots(r), weight(w) {}
};
stump s1(3, 15.6);		// 旧标准
stump s2{5, 43.4};
stump s3 = {4, 32.1};
```

如果类有将模板 std:initializer_list 作为参数的构造函数，则只有该构造函数可以使用列表初始化形式

1. 缩窄问题

   初始化列表语法可以防止缩窄，禁止将数值赋给无法存储给无法存储它的数值变量；但是值在较窄类型的取值范围内，将其转换为较窄的类型也是允许的；

   允许转换为更宽的类型

2. std:initializer_list

   - C++11 提供模板类 initializer_list，可将其作为构造函数的参数，如果类有接受 initializer_list 作为参数的构造函数，则初始化列表语法只能用于该构造函数

   列表中的元素必须是同一类型或可转换为同一类型的

   ```cpp
   #include<initializer_list>
   double sum(std::initializer_list<double> list);
   int main()
   {
   	double total = sum({2.5, 3.1, 4})
   	return 0;
   }
   double sum(std::initializer_list<double> list)
   {
   	double tot = 0;
   	for(auto p = list.begin(); p != list.end(); p++)
   		tot += *p;
   	return tot;
   }
   ```

### 18.1.3 声明

1. auto

   实现自动类型推断

2. decltype

   将变量的类型声明为表达式指定的类型

   ```cpp
   decltype(x) y;		// y 的类型和 x 相同
   
   template<typename T, typename U>
   void ef(T t, U u)
   {
   	decltype(T*U) tu;
   	...
   }
   ```

3. 返回类型后置

   在函数名和参数列表后面指定返回类型

   ```cpp
   double f1(doubel, int);
   auto f1(double, int) -> double;
   
   template<typename T, typename U>
   auto ef(T t, U u) -> decltype(T*U)		// 主要作用就是使用 decltype 将返回值后置定义（返回值类型可以取决参数类型）
   ```

4. 模板别名 using = 

   对于冗长复杂的标识符，可以为其创建别名

   ```cpp
   typedef std::vector<std::string>::iterator itType;
   
   using itType = std::vector<std::string>::iterator;
   
   template<typename T>			// using 可用于模板部分具体化，typedef 不行
   using arr12 = std::array<T, 12>;
   ```

5. nullptr

   专门用来表示空指针

### 18.1.4 智能指针

c++11 摒弃了 auto_ptr，并新增了三种智能指针：unique_ptr、shared_ptr、weak_ptr

### 18.1.5 异常规范方面

C++ 提供一种语法，可用于指出函数可能引发哪些异常；C++11认为异常规划没啥用，指出函数不会引起异常有一定的价值，添加关键字 noexcept

```cpp
void f501(int) throw(bad_dog);
void f733(int) throw();			// 指出函数可能引起哪些异常

void f875(short, short) noexcept;		// 指出函数不会引发异常
```

### 18.1.6 作用域内枚举

- 新枚举要求进行显示限定，以免发生名称冲突
- 枚举名的作用域为枚举定义所属的作用域，在同一作用域内定义两个枚举，枚举成员不能同名

```cpp
enum Old {yes, no, maybe};			// 旧版本
enum class New1 {never, sometimes, often, always};	// C++11
enum struct New2 {never, lever, sever};			// C++11

New1::never		// 新枚举要求进行显式限定，以免发生名称冲突
```

### 18.1.7 对类的修改

允许构造函数被继承和彼此调用，更佳的方法访问控制方式以及移动构造函数和移动赋值运算

1. 显示转换运算符

   引入关键字 explicit，以禁止单参数构造函数导致的自动转换，只有在单参数的情况下会做隐式转换

   ```cpp
   class String
   {
   	int _size;
   	explict String(int size)		// 禁止隐式转换
   	{
   		_size = size;
   	}
   };
   
   /* 没有禁止隐式转换前 */
   String string1 = 10;	// 是可以的，相当于 Sring string1 = String (10);
   /* explict 禁止隐式转换后 */
   String string1 = 10;	// 报错
   String string1(10); 	// 可以
   ```

   ```cpp
   class Plebe
   {
   	Plebe(int);
   	explicit Plebe(double);
   	...
   }；
   Plebe a, b;
   a = 5;		// 可以
   b = 0.5;	// 报错
   b = Plebe(0.5);			// 显示调用，也可以
   ```

   C++11 拓展了用法，使其可对转换函数做类似的处理

   ```cpp
   class Plebe
   {
   	operator int() const;
   	explicit operator double() const;
   	...
   };
   Plebe a, b;
   int n = a;		// allowed
   double x = b;	// error
   x = double(b);	// allowed, 显示转化
   ```

2. 类内成员初始化

   支持在类定义中初始化成员；可使用等号或大括号版本的初始化（不可使用小括号）

   ```cpp
   class Session
   {
   	int mem1 = 10;
   	double mem2 {1966.54};
   	short mem3;
   	...
   };
   ```

### 18.1.8 模板和STL的改进

1. 基于范围的 for 循环

   ```cpp
   std::vector<int> vi(6);
   for(auto &x: vi)
   	x = std::rand();
   ```

2. 新的STL容器

   C++11 新增容器 forward_list、unordered_map、unordered_multimap、unordered_set、unordered_multiset

   新增了模板 array

3. 新的 STL 方法

   新增了方法 cbegin() 和 cend()；返回 const 迭代器

4. valarry 升级

   C++11为 valarry 添加了两个函数（begin(), end()），都接受 valarry 为参数，并返回迭代器；使基于范围的 STL 算法适用于 valarray

5. 摒弃 export

6. 尖括号

   ```cpp
   vector<list<int>>;			// 允许将两个 >> 写在一块，而不需要加一个空格
   ```

### 18.1.9 右值引用

引用只能关联左值，已经分配空间的内存地址；右值引用可以对右值做引用

C++11 新增了右值引用，右值引用可以关联到右值；右值包括字面常量、算术表达式、函数的返回值（函数返回不是引用）

就是获取一个右值，给他分配空间；将右值关联到右值引用导致该右值被存储到特定的位置，且可以获取该位置的地址

```cpp
double && rd1 = 21.5;
cout << &rd1 << ":" << rd1 << endl;
```

右值引用引入的还有一部分原因是为了移动语义的实现

## 18.2 移动语义和右值引用

### 18.2.1 移动语义

通常用一个对象给另一个对象初始化的过程中，如果数据量较大，则调用拷贝会大大影响性能

移动语义：避免了移动原始数据，数据其实位置没动，只是修改了记录，将所有权转移给新对象；

通常需要定义两个构造函数：

- 一个常规的复制构造函数，它使用 const 左值引用作为参数，引用关联到左值实参
- 另一个是移动构造函数，它使用右值引用作为参数，引用关联到右值实参，（在转移过程中，构造函数可以修改值，不加const）

```cpp
#include<iostream>

using namespace std;

class Useless
{
private:
	int n;			// number of elements
	char *pc;		// point of data
	static int ct;	// number of object
	void ShowObject() const;
public:
	Useless();
	explict Useless(int k);
	Useless(int k, char ch);
	Useless(const Useless & f);		// 复制构造函数
	Useless(Useless && f);		// 移动构造
	~Useless();
	Useless & operator=(const Useless & f);
    Useless & operator=(Useless && f);
	
	void ShowObject() const;
};

int Useless::ct = 0;

Useless::Useless()
{
	++ct;
	n = 0;
	pc = nullptr;
	ShowObject();
}

Useless::Useless(int k):n(k)
{
	++ct;
	pc = new char[n];
	ShowObject();
}

Useless::Useless(int k, char ch): n(k)
{
	++ct;
	pc = new char[n];
	for(int i = 0; i < n; i++)
		pc[i] = ch;
	ShowObject();
}

Useless::Useless(Useless & f):n(f.n)
{
	++ct;
	pc = new char[n];
	for(int i = 0; i < n; i++)
		pc[i] = f.pc[i];
	ShowObject();
}

Useless::Useless(Useless && f):n(f.n)			// 移动构造
{
	++ct;
	pc = f.pc;
	f.pc = nullptr;
	f.n = 0;
	ShowObject();
}

Useless::~Useless()
{
	ShowObject();
	delete [] pc;
}

Useless Useless::operator+(const Useless & f)const
{
	Useless temp =  Useless(n + f.n);
	for(int i = 0; i < n; i++)
		temp.pc[i] = pc[i];
	for(int i = n; i < n+f.n; i++)
		temp.pc[i] = f.pc[i-n];
	return temp;	// 返回临时变量
}

void Useless::ShowObject()const
{
	cout << "number of elements:" << n;
	cout << "Data address: " << (void *)pc << endl;
}

void Useless::ShowData() const
{
	if(n == 0)
		cout << "Object empty";
	else
		for(int i = 0; i < n; i++)
			cout << pc[i];
	cout << endl;
}
```

左值引用指向右值的方法，实参为右值，const 引用形参将指向一个临时变量

### 18.2.4 赋值

复制赋值运算符和移动赋值运算符

```cpp
/* 赋值运算符 */
Useless & Useless::operator=(const Useless & f)
{
	if(this == &f)
		return this;
	delete [] pc;
	n = f.n;
	pc = new char[n];
	for(int i = 0; i < n; i++)
		pc[i] = f.pc[i];
	return *this;
}

/* 移动赋值运算符 */
Useless & Useless::operator=(const Useless && f)
{
	if(this = &f)
		return this;
	delete [] pc;
	n = f.n;
	pc = f.pc;
	f.pc = nullptr;
	f.n = 0;
	return *this;
}
```

### 18.2.5 强制移动

将左值转化为右值，进而调用移动构造或者移动赋值运算符；C++11 使用头文件 utility 中声明的 std::move() 函数实现

```cpp
Useless one(10, 'x');
Useless two;
two = std::move(one);
```

但是类如果没有定义移动赋值运算符，编译器将使用复制赋值运算符

## 18.3 新的类功能

### 18.3.1 特殊的成员函数

默认构造函数执行过程：

- 对于内置类型成员，默认的默认构造函数不会对其进行初始化；对于属于类对象的成员，则调用其默认构造函数

编译器默认提供6个成员函数：默认构造函数、复制构造函数、复制赋值运算符、析构函数、移动构造函数、移动赋值运算符

- 如果你提供了析构函数、复制构造函数或复制赋值运算符，编译器将不会自动提供移动构造函数和移动赋值运算符
- 如果你提供了移动构造函数或移动赋值运算符，编译器将不会自动提供复制构造函数和复制赋值运算符

```cpp
class Someclass
{
public:
	Someclass();				// 默认构造函数
	Someclass(const Someclass &);		// 复制构造函数
	Someclass & operator=(const Someclass &);		// 复制赋值运算符
	Someclass(Someclass &&);					// 移动构造
	Someclass & operator=(Someclass &&);		// 移动赋值运算符
	~Someclass();			// 析构函数
};
```

### 18.3.2 默认的方法和禁用方法

- 关键字 default 显示声明这些方法的默认版本（由于某些原因有些默认函数编译器不会自动创建）；只能用于6个特殊的成员函数
- 关键字 delete 可用于禁止编译器使用特定的方法；禁用函数只用于查找匹配函数，使用它们将导致编译错误；可用于任何成员函数
- 要禁止复制，可以将复制构造函数和复制赋值运算符放在类定义的 private 中

```cpp
class Someclass
{
public:
	Someclass() = default;
	Someclass(const Someclass&) = delete;
	Someclass(const Someclass&&) = default;
};
```

### 18.3.3 委托构造函数

委托：C++11 允许在一个构造函数的定义中使用另一个构造函数；是彼此调用

```cpp
class Notes
{
	int k;
	double x;
	std::string st;
public:
	Notes();
	Notes(int);
	Notes(int, double);
	Notes(int, double, std::string);
};

Notes::Notes(int kk, double xx, std::string stt): k(kk), x(xx), st(stt)
{}
Notes::Notes():Notes(0, 0.01, "Oh")
{}
Notes::Notes(int kk):Notes(kk, 0.01, "Ah")
{}
Notes::Notes(int kk, double xx):Notes(kk, xx, "Uh")
{}
```

### 18.3.4 继承构造函数

C++11 提供一种让派生类能够继承基类构造函数的机制（派生类去调用基类的构造函数）

```cpp
class C1
{
public:
	int fn(int j) {...}
	double fn(double w) {...}
	void fn (const char *s) {...}
};

class C2:public C1
{
	using C1:fn;			// 可以使用 C1 类中的fn成员函数，但是会覆盖
	double fn(double) {...};		// 会覆盖 C1 里面的成员函数
};
```

```cpp
classs BS
{
	int q;
	double w;
public:
	BS():q(0), w(0) {}
	BS(int k):q(k), w(100) 	{ }
	BS(double x):q(-1), w(x) { }
	BS(int k, double x):q(k), w(x) {}
	void Show() const	{ std::cout << q << "," << w << '\n';}
};

class DR:public BS
{
	short j;
public:
	using BS::BS;			// 重点
	DR(): j(100) { }
	DR(double x): BS(2*x), j(int(x)) { }
	DR(int i): j(-2), BS(i, 0.5*i) { }
	void Show() const {std::cout << j << "," << BS::Show();}
};
```

### 18.3.5 虚函数管理方法 override 和 final

假设基类声明了一个虚方法，而您决定在派生类中提供不同的版本，这将覆盖旧版本；如果特征标不匹配，将隐藏而不是覆盖旧版本

- 虚说明符 override 指出要覆盖一个虚函数，将其放在参数列表后面，如果声明与基类方法不匹配，编译器将视为错误
- 在参数列表后面加上 finial，可禁止派生类覆盖特定的虚方法

```cpp
virtual void f(char * ch) const override {...}
virtual void f(char * ch) const final {...}
```

## 18.4 Lambda 函数（匿名函数）

### 18.4.1 函数指针、函数符、Lambda函数

函数指针、函数符（仿函数）、Lambda函数统称函数对象

```cpp
/* 判断能否被3整除 */
bool f3(int x)
{
	return x % 3 == 0;
}

class f_mod
{
private:
	int dv;
public:
	f_mod(int d = 1): dv(d) {}
	bool operator() (int x) { return x % dv == 0;}
};
f_mod obj(3);
bool is_div_by_3 = obj(7);

[](int x) {return x % 3 == 0;}
```

### 18.4.2 Lambda函数

仅当lambda表达式完全由一条返回语句组成时，自动类型推断才管用；否则，需要使用新增的返回类型后置语法

```cpp
[](double)->double(int y = x; return x-y;)
```

```cpp
/* 给 Lambda 一个名字 */
auto mod3 = [](int x) {return x % 3 == 0};
bool result = mod3(z);			// 可以像函数一样调用
```

lambda 可以访问作用域内的任何动态变量；捕获要使用的变量，可以将其名车放在中括号内

- [z]：按值访问变量
- [&count]：按引用访问变量
- [&]：按引用访问所有动态变量
- [=]：按值访问所有动态变量

## 18.5 包装器（相当于函数指针）

### 18.5.1 function包装器和模板低效性

函数名、函数指针、函数对象或者lambda属于不同数据类型，在函数调用过程中会产生多份函数模板代码！！！

为了解决多份函数模板的产生，引入 function

调用特征标：有返回值类型以及用括号括起并用逗号分隔设为参数类型列表定义；而 function 就是用来接受相同调用特征标的不同函数对象，从而在调用模板函数的时候，只产生一份实例

function 在头文件 functional 中声明，它通过调用特征标的角度定义了一个对象，可用于包装调用特征标相同的函数指针、函数对象、lambda 表达式

```cpp
#include<functional>

template <typename T, typename F>
T use_f(T v, F f)
{
	return f(v);
}

function<double(double)> ef1 = dub;			// 去接收函数对象
use_f(y, ef1);				// 只产生一份模板函数实例
```

也可以将 function 直接写进模板函数里面

```cpp
#include<functional>
T use_f(T v, std::function<T(T)> f)
{
	return f(v);
}
use_f<double>(y, dub);
```

### 18.6 可变参数模板

### 18.6.1 模板和函数参数包

C++ 提供一个用省略号表示的元运算符，能够表示模板参数包的标识符，模板参数包基本上是一个类型列表；

同样，还能够用来声明表示函数参数包的标识符，而函数参数包基本上是一个值列表

```cpp
/* 会出现一个问题，一直循环调用 */
template<typename... Args>			// Args 是模板参数包，可以与多个类型进行匹配
void show_list1(Args... args)		// args 函数参数包
{
	show_list1(args...);		// 将省略号放在函数参数的右边，将参数包展开；这样会导致循环调用
}
```

### 18.6.3 在可变参数模板函数中使用递归

核心思想：将函数参数包展开，对列表中的第一项进行处理，再将剩余的内容传递给递归调用

```cpp
void show_list1() {}			// 递归停止项

template<typename T>
void show_list1(const T& value)
{
    cout << value << endl;
}

template<typename T, typename... Args>
void show_list1(const T& value, const Args&... args)
{
	cout << value << ", ";
	show_list1(args...);
}
```

## C++11新增的其它功能

### 18.7.1 并行编程

定义了一个支持线程化执行的内存模型

关键字 thread_local 将变量声明为静态存储，其持续性与特定线程相关

### 18.7.2 新增的库

....
