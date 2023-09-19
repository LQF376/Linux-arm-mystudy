# 16 string & STL

## 16.1 string 类

#include<string> 头文件

### 16.1.1 构造函数

```cpp
string(const char *s);
string(size_typen n, char c);
string(const string & str);
string();

template<class Iter>
string(Iter begin, Iter end);			// [begin, end)

string(const string &str, string size_type pos = 0, size_type n = npos);
string(string && str)noexcept;			// c++11
string(initializer_list<class>il)		// C++11
```

> = 运算符重载，可以将 string 对象、C风格字符串、char字符赋给 string 对象
>
> += 运算符重载，可以将一个字符串附加到另一个字符串的后面
>
> [] 运算符重载，可以用于访问字符串中的各个字符

### 16.1.2 string 类输入

getline 停止条件：

- 到达文件末尾
- 遇到分界字符，通常是 \n
- 读取的字符数达到最大允许值

```cpp
/* char */
char info[100];
cin >> info;
cin.getline(info, 100);		// 需要指定长度
cin.get(info, 100);

/* sting */
string stuff;
cin >> stuff;
getline(cin, stuff);		// 不需要指定长度
```

### 16.1.3 使用字符串

```cpp
size_type find(const string &str, size_type pos = 0)const;
size_type find(const char* s, size_type pos = 0)const;
size_type find(const char* s, size_type pos = 0, size_type n);	// 查找 s 前 n 个字符是否出现
size_type find(char ch, size_type pos = 0)const;
```

```cpp
rfind()		// 查找子字符串或字符最后一次出现的位置
find_first_of()		// 在字符串中查找参数中任何一个字符首次出现的位置
find_last_of()			// 在字符串中查找参数中任何一个字符最后一次出现的位置
find_first_not_of()		// 字符串中查找第一个不包含在参数中的字符
```

```cpp
capacity()			// 返回当前分配非字符串的内存块大小
reserve()			// 让你能够请求内存块的最小长度
c_str()				// 将 string 转换 字符串
```

## 16.2 智能指针模板类

智能指针是行为类似于指针的类对象；可以将 new 获得的地址赋给智能指针，当指针对象过期时，它会自动调用其析构函数调用 delete 释放指向的内存；好处就是 创建对象之后，不用自己手动的去释放它

- 可以对它执行解引用操作
- 可以用它来访问结构成员
- 将它赋给指向相同类型的常规指针
- 将智能指针对象赋给另一个同类型的智能指针

> - auto_ptr ：C++98的解决方案
> - unique_ptr
> - shared_ptr

注意：使用智能指针的时候应该避免其指向非堆区内存（delete 无法释放）

```cpp
#include<memory>			// 使用智能指针的头文件

auto_ptr<double> pd(new double);
unique_ptr<double> pdu(new double);
shared_ptr<string> ps(new string);
```

### 16.2.2 智能指针的注意事项

- 2 个智能指针指向同一个堆内存开辟对象时，会释放两次问题（auto_ptr 指针在使用过程中容易引起）

  > - 定义赋值运算符，执行深拷贝；两个指针指向不同的对象，其中一个对象是另一个对象的副本
  > - 建立所有权概念；对于特定的对象，只能有一个智能指针可拥有它（auto_ptr 和 unique_ptr 的策略）
  > - 创建智能更高的指针，跟踪引用特定对象的智能指针计数（引用计数）；赋值时，引用计数+1，指针过期时，引用计数-1，仅当最后一个指针过期时，才调用 delete （shared_ptr 的策略）

```cpp
auto_ptr<string>  p1 (new string("Fowl Balls"));
auto_ptr<string> p2;
p2 = p1;		// p2 接管 string的所有权， p1所有权被剥夺
*p1;			// 报段错误
```

```cpp
unique_ptr<string>  p1 (new string("Fowl Balls"));
unique_ptr<string> p2;
p2 = p1;		// 编译器直接报错，unique 拥有更严格的所有权条件，不允许交换所有权

/* 但是 unique_ptr 允许接收右值性质的 unique */
// demo1
unique_ptr<string> demo(const char * s)
{
	unique_ptr<string> temp (new string (s));
	return temp;
}

unique_ptr<string> ps;
ps = demo("Uniquely special");		// 函数返回值是右值

// demo2
unique_ptr<string> p3;
p3 = unique_ptr<string> (new string("hello"));			// 匿名对象，右值

/* 使用移动语义来使用 unique_ptr */
unique_ptr<string> p1(new sring("hello"));
unique_ptr<string> p2;
p2 = std::move(p1);
```

unique_ptr 相对于 auto_ptr 还有一个优点：有使用 new[] 和 delete[] 版本

```cpp
unique_ptr<double[]> p1 (new double(5));		// 不太懂
```

### 16.2.4 选择智能指针

- 如果程序要使用多个指向同一个对象的指针，应该使用 share_ptr
- 如果程序不需要多个指向同一个对象的指针，则可使用 unique_ptr（有注意点），若编译器不支持，则可以使用 auto_ptr

```cpp
unique_ptr<int> make_int(int n)
{
	return unique_ptr<int> (new int(n));
}
void show(unique_ptr<int> & p)		// 此处一定要引用，不可用值传递，值传递报错
{
	cout << *p << " ";
}
int main()
{
	vector<unique_ptr<int>> vp(size);
	for(int i = 0; i < vp.size(); i++)
	{
		vp[i] = make_int(rand() % 1000)；
	}
	for_each(vp.begin(), vp.end(), show);
}
```

## 16.3 标准模板库

### 16.4.2 迭代器类型

STL定义了5种迭代器：

- 输入迭代器
  - 能够读取容器里面的值，但不一定能让程序修改值
  - 单通行，能遍历容器中的所有值，但是不依赖于前一次遍历时的迭代器值，也不依赖于本次遍历中前面的迭代器值
- 输出迭代器
  - 解引用让程序能修改容器的值，而不能读取
  - 单通行，只写算法
- 正向迭代器
  - 每次遍历总按相同的顺序遍历元素，故将正向迭代器递增后，仍然可以对前面的迭代器值座解引用，并得到相同的值
  - 可读可写，也可以只读（const 关键字）
- 双向迭代器
  - 有正向迭代器的所有特性，同时支持递减运算符
- 随机访问迭代器
  - 直接跳到容器的任何一个元素，叫随机访问（支持 a+n 访问方式）

存在多种迭代器的目的：编写算法的时候尽可能使用要求最低的迭代器，让其使用容器范围最大

### 16.4.4 概念、改进、模型

迭代器只是一种概念，双向迭代器是对正向迭代器的概念的一种改进（不是继承），指针满足随机访问迭代器的特征，所以指针也是一个随机访问迭代器模型

```cpp
const int SIZE = 100;
double Receipts[SIZE];

sort(Receipts, Receipts+SIZE);		// 指针作为迭代器用于算法
```

#### STL 预定义的迭代器

- 表示输出流的迭代器

```
#include<iterator>

ostream_iterator<int, char> out_iter(cout, " ");		// 表示输出流的迭代器 <输入类型， 输出类型>
*out_iter++ = 15; 			// 等价于 cout << 15 << " "

copy(dice.begin(), dice.end(), out_iter);		// 将 dice 容器内的元素输出到屏幕上
```

- 表示输入流的迭代器

```cpp
istream_iterator<int, char> in_iter (cin);     // <要读取的数据类型， 输入流使用的字符类型>
```

- reverse_iterator
  - 反转迭代器 ++ 相当于 -- ，而且先减在解引用（rbegin() 和 end() 同位，都位于超尾）

```cpp
vector<int> dice(10);

ostream_iterator<int, char> out_iter(cout, " ");
copy(dice.rbegin(), dice.rend(), out_iter);			// 反向输出

vector<int>:: reverse_iterator ri;
for(ri = dice.rbegin(), ri != dice.rend(); ++ri)
	cout<< *ri<<' ';
```

- 插入迭代器
  - 插入将添加的新元素，而不会覆盖已有的数据，并使用自动内存分配来确保能够容纳新的信息

> - back_insert_iterator 将元素插入到容器尾部
> - front_insert_iterator 将元素插入到容器的前部
> - insert_iterator 将元素插入到 构造函数指定的位置前面

```cpp
back_insert_iterator<vector<int> > back_iter(dice);			// 以容器为模板参数
front_insert_iterator<vector<int> > front_iter(dice);			// 通常是配合算法使用的
insert_iterator<vector<int> > insert_iter(dice, diec.begin());
```

### 16.4.5 容器

- 存储其他对象的对象，被存储的对象必须是同一类型的；被存储的对象必须是可复制构造的和可赋值的（定义为私有则不行）

```cpp
/*
* T 存储在容器中的数据类型
* a、b 表示容器
* u 表示X容器的标识符
* rv：非常量右值
*/
X::iterator				// 迭代器
X::value_type			// T 的类型
X u;					// 空容器
X();					// 匿名的空容器
X u(a);					// 用一个已有容器初始化另一个容器
X u = a;				// 赋值初始化
X& = a;					// 引用赋值运算符
(&a)-> ~X();			// 调用析构函数
a.begin();				// 第一个元素的迭代器
a.end();				// 超尾的迭代器
a.size();				// 返回元素个数
a.swap(b);				// 交换 a 和 b
a == b;					// ab长度相同，且每个元素相同
a != b;

/* 11新特性 */
X u(rv);				// 移动构造
X u = rv;				// 移动赋值
a.cbegin();				// const_iterator
a.cend();				// const_iterator
```

#### 序列容器

要求每个元素按严格的线性排序排列（至少是正向迭代器）

```cpp
/*
* 序列容器要求
* p,q,i,j 迭代器
*/
X a(n, t);					// n 个 t
X(n, t);
X a(i, j);
x(i, j);
a.insert(p, t);
a.insert(p, n, t);
a.insert(p, i, j);			// [i, j) 插入到p前面
a.erase(p);
a.erase(p, q);				// [p, q) 区间
a.char()；
```

```cpp
a.front();				// vector、list、deque
a.back();				// vector、list、deque
a.push_front(t);		// list、deque
a.push_back(t);			// vector、list、deque
a.pop_front();			// list、deque
a.pop_back();			// vector、list、deque
a[n];					// vector、deque
a.at(n);				// 区别于[]， 若n若在容器的有效区域外，at将做边界检查
```

> - vector
>   - 数组的一种类表示，提供了自动内存管理，可以动态改变vector的长度，随着元素的添加和删除而增大和缩小
>   - 对元素随机访问

> - deque
>   - 双端队列，支持随机访问
>   - 可以从 deque 对象的开始位置插入和删除元素（时间固定），但是访问中间元素的速度不及vector

> - list
>
>   - 双向链表（可以双向遍历链表）
>   - 在任一位置进行插入和删除元素时间固定
>   - list 不支持数组表示法和随机访问
>   - 从容器中插入或删除元素时候，链表迭代器指向元素将不变
>
>   ```
>   void merge(list<T, Alloc>& x);				// 合并两个链表，前提两个链表有序
>   void remove(const T& val);					// 链表 中删除 val 所有实例
>   void sort();							// 排序
>   void spice(iterator pos, list<T Alloc> x)	// 将链表x的内容插入到pos前面，x将为空（移动）
>   void unique();							// 将连续的相同的元素压缩为单个元素（先sort）
>   ```

> - forward_list(c++11)
>   - 单链表（正向迭代器）

> - queue
>
>   - 队列，底层为 deque 的适配器
>
>   ```
>   bool empty()const; 				// 判空
>   size_type size()const;			// 元素个数
>   T& front();					// 访问队首
>   T& back();					// 访问队尾
>   void push(const T& x);		// 队尾插入x
>   void pop();			// 删除队首元素
>   ```

> - priority_queue
>
>   - 最大的元素被移到队首（可以自己指定）
>
>   ```
>   priority_queue<int> pq1;
>   priority_queue<int> pq2(greater<int>);
>   ```

> - stack
>
>   - 栈，底层为 vector 的适配器
>
>   ```
>   bool empty()const;
>   size_type size()const;
>   T& top();
>   void push(const T& x);
>   void pop();
>   ```

> - array(c++11)
>   - 长度是固定的，原始数组概念；目的为了适用 STL 算法库

#### 关联容器

- 将值和键关联在一起，使用键来查找值
- 关联容器允许插入新元素，但不能指定元素的插入位置（底层是用树实现的）
- set(键和值相同)、mutiset、map、mutilmap

> - set
>   - 递增排序，且键只能出现一次（去重）
>
> ```cpp
> set<string> A;
> set<string, less<string> > A;		// 指定对键进行排序的比较函数和对象
> ```
>
> ```cpp
> /* 并集 */
> set_union(A.begin(), A.end(), B.begin(), B.end(), ostream_iterator<string, char>(cout, " "));
> // 新的容器是空的，所以一定要用插入迭代器
> set_union(A.begin(), A.end(), B.begin(), B.end(), insert_iterator<set<string> > (C, C.begin()));
> 
> /* 交集 */
> set_intersection(A.begin(), A.end(), B.begin(), B.end(), ...);
> 
> /* 两个集合的差 */
> set_difference(A.begin(), A.end(), B.begin(), B.end(), ...);
> 
> /* 将键作为参数返回迭代器 */
> set<string> a;
> a.lower_bound("a");				// 第一个大于等于键参数的成员
> a.upper_bound("z");				// 第一个大于键参数的成员
> ```

> - multimap
>   - 键和值类型不同，但同一个键可以与多个值进行关联
>
> ```cpp
> multimap<int, string> codes;
> 
> pair<const int, string> item(213, "Los Angeles");
> codes.insert(item);
> 
> codes.insert(pair<int, string> (213， "Los Angeles"));			// 匿名对象
> ```
>
> ```cpp
> codes.count(key);		// 返回 key 的个数
> 
> /* equal_range() 根据键值来获取一个返回 */
> pair<multimap<KeyType, string>::iterator, multimap<KeyType, string>::iterator> range = codes.equal_range(718);
> 
> for(auto it = range.fist; it != range.second; it++)
> {
> 	cout << (*it).second << endl;
> }
> ```

## 16.5 函数对象（函数符）

- 可以以函数方式与（）结合使用的任意对象

> - 生成器：是不用参数就可以调用的函数符
> - 一元函数：用一个参数可以调用的函数符
> - 二元函数：用两个参数可以调用的函数符
>
> - 谓词：返回值为 bool 的一元函数
> - 二元谓词：返回值为 bool 的二元函数

### 16.5.2 预定义的函数符

- 所有内置的算数运算符、关系运算符和逻辑运算符，STL都提供了等价的函数符

```cpp
#include<functional>

plus<double> add;
double y = add(2.2, 3.4);
```

### 16.5.3 自适应函数符和函数适配器

- 携带了标识参数类型和返回类型的 typedef 成员：result_type、first_argument_type、second_argument_type

```cpp
plus<int>::result_type
```

- 函数适配器，将接收两个参数的函数符转化为接收1个参数的函数符

```cpp
binder1st(f2, val) f1;			// f1(x) == f2(val, x);
binder2nd(f2, val) f1;			// f1(x) == f2(x, val);

binder1st(multiplies<double>(), 2.5);
```

## 16.6 算法

头文件 #include <algorithm>

> STL 将算法库分成 4 组：
>
> - 非修改式序列操作j：不修改容器的内容
> - 修改式序列操作：可以修改容器的内容
> - 排序和相关操作：排序函数和集合操作
> - 通用数字运算：区间的内容累计、计算两个容器的内部乘积

- 就地算法：结果存放在原始数据的位置上
- 复制算法：结果发送到另一个位置

> 有的算法有两个版本：就地算法和复制算法；复制算法版本通常以 _copy 结尾
>
> _if 结尾版本：将函数用于容器内的每个元素，若返回值为true，就进行操作

```cpp
string letters;

cin >> letters;
sort(letters.begin(), letters.end());
while(next_permutation(letters.begin(), letters.end()))
	cout << letters << endl;
```

### 16.6.4 函数和容器的方法区别

- 容器内的方法可以使用容器模板类中的内存管理工具，从而在需要时调整容器的长度
- 而 STL 函数无法操作容器大小

```cpp
list<int> la;
list<int> lb;
la.remove(4);		// 会删除链表中所有 4 的项
last = remove(lb.begin(), lb.end(), 4);		// 将没被删除的值放在链表的开始位置，并返回指向新的超尾值的迭代器，即不改变容器原来的大小，后面的数值不确定
lb.erase(last, lb.end());
```

### 16.6.5 使用 STL

输入单词，不区分大小写，统计单词出现的个数

```cpp
#include<iostream>
#include<string>
#include<vector>
#include<set>
#include<map>
#include<iterator>
#include<algorithm>
#include<cctype>

using namespace std;

char toLower(char ch){ return tolower(ch); }
string& ToLower(string& st);
void display(const string & s);

int main()
{
	vector<string> words;
    string input;
    while(cin >> input && input != "quit")
    {
    	words.push_back(input);
    }
    
    for_each(words.begin(), words.end(), display);
    cout << endl;
    
    set<string> wordset;
    transform(words.begin(), words.end(), insert_iterator<set<string>> (wordset, wordset.begin()), ToLower);
    
    for_each(wordset.begin(), wordset.end(), display);
    cout << endl;
    
    map<string, int> wordmap;
    set<string>::iterator si;
    for(si = wordset.begin(), si != wordset.end(), si++)
    {
    	wordmap[*si] = count(words.begin(), words.end(), *si);
    }
    
    for(si = wordset.begin(), si != wordset.end(); si++)
    {
    	cout << *si << ": " << wordmap[*si] << endl;
    }
    
    return 0;
}

void display(const string& s)
{
	cout << s << " ";
}
```

## 16.7 其他库

> complex 库：为复数提供类模板 complex
>
> random 库：提供更多的随机数功能
>
> valarray 库：用于表示数值数组，支持各种数值数组的操作，两个数组内容相加

- valarray
  - 面向数值计算，不是 STL 的一部分
- array
  - 表示长度固定的数组，为替代内置数组而设计

```cpp
vector<double> ved1(10), ved2(10), ved3(10);
array<double, 10> vod1, vod2, vod3;
valarray<double> vad1(10), vad2(10), vad3(10);
/* 实现前两个容器元素相加给第三个容器 */
transform(ved1.begin(), ved1.end(), ved2.begin(), ved3.begin, plus<double> ());
transform(vod1.begin(), vod1.end(), vod2.begin(), vod3.begin, plus<double> ());
vad3 = vad1 + vad2;		// valarray 优势，转为数值计算而生
vad3 = vad1 * vad2;
vad3 = 2.5 * vad1;
vad3 = log(vad1);		// 重载了科学函数
vad3 = vad1.apply(log)	// 针对非重载函数，不修改调用对象，返回一个包含结果的新对象
```

valarray 类还提供了一下方法：

- sum() 计算valarray 对象中所有元素的和
- size() 返回元素数
- max() 返回最大值
- min() 返回最小值

注意！！！！ valarray 不能动态调整大小，不能插入、排序、搜索，没有 .begin() 和 .end()

```cpp
valarray<int> vad;
sort(begin(vad), end(vad));			// c++11 提出
```

slice 类对象可作为数组索引，表示访问一组值；格式：起始索引、索引数、跨距

```cpp
valarray<int> vad;
vad[slice(1, 4, 3)] = 10;		// 1 4 7 10 元素设置为 10
```

通常只有 valarray 和 单个 int 元素支持++操作，而利用 slice 取的索引不支持++操作；可以将 slice 取的索引重新赋一个 valarray

```cpp
valarray<int> subvad(vad[slice(1, 4, 3)]);
subvad++;
```

### 16.7.2 模板 initalizer_list（c++11）

- 可以使用初始化列表语法将 STL 容器初始化为一些列值

```cpp
/* 容器类包含将 initalizer_list<T> 作为参数的构造函数 */
std::vector<double> payments {45.99, 39.23, 19.95, 89.01};
std::vector<double> payments ({45.99, 39.23, 19.95, 89.01});		// 等价于
```

使用 initalizer_list 对象

- 包含头文件 initalizer_list
- 这个模板类包含成员函数 begin() 和 end()，来访问列表元素；size() 返回元素个数
- initalizer_list 的迭代器类型为 const，不能修改其中的值；但是可以将一个 initalizer_list 赋值给另一个 initalizer_list

```cpp
#include<initalizer_list>

std::initalizer_list<double> d1 = {1.1, 2.2, 3.3, 4.4, 5.5};

double tot = 0;
int n = d1.size();
for(auto p = d1.begin(), p != d1.end(); p++)
{
	tot += *p;
}

d1 = {16.0, 25.0, 36.0, 40.0, 64.0};
```
