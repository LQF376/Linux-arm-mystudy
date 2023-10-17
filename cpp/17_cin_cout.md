# 17. 输入、输出、文件

## 1. C++ 输入输出

C++ 依赖于C++的I/O解决方案，由 iostream 和 fstream 中定义的一组类（不是正式语言定义的组成部分）

- streambuf 类为缓冲区提供了内存，并提供了相应缓冲区的方法
- ios_base 类表示流的一般特征
- ios 类基于 ios_base，其中包括一个指向 streambuf 对象的指针成员
- ostream 类 从ios类派生而来，提供了输出方法
- istream 类 从 ios 类派生而来，提供了输入方法
- iostream 类 是基于 istream 和 ostream 类的，因此继承了输入方法和输出方法

iostream 文件会自动创建8个流对象：cin；cout；cerr；clog（对应宽字符也有4个）

## 2. cout 输出（ostream）

C++将输出看作字节流，ostream类的任务就是将数值类型转换为文本形式表示的字符流

### 重载<<运算符

对于每种数据类型，ostream都提供了 operator<<() 函数定义

```cpp
ostream & operator<<(int);			// 原型
cout << 88;
```

对于指针类型

| const signed char * | const unsigned char* |
| :-----------------: | :------------------: |
|     const char*     |      **void***      |

````cpp
// 插入运算符还可以输出指针类型(char*、void*)
char *pn = "Violet D'Amore";
cout << pn;		// 输出字符串，遇到空字符停止
cout << (void *)pn		// 输出字符串的地址（其它类型的指针会自动转化为 void*，显示地址） 
````

### 拼接输出

````cpp
ostream & operator<<(type);			//拼接反应，链式
cout << "hello" << "tony" << endl;
````

### put、write 方法

- put 显示字符
- write 显示字符串；按字节数进行输出，并不对结尾做任何的校验

```cpp
ostream & put(char);				// 原型，显示字符
cout.put('W');
cout.put('C').put('W');	
cout.put(65);			// 'A'

basic_ostream<charT, traits>& write(const char_type *s, streamsize n);
long val = 5000000;
cout.write((char *)&val, sizeof(long));
```

### 刷新输出缓冲区

- flush：刷新缓冲区
- endl：刷新缓冲区，并插入一个换行符

```
flush(cout);			// 函数
cout << flush;			// 控制符
cout << endl;
```

### cout 进行格式化

ios_base 类存储了描述格式状态的信息（计数系统和字段宽度）

- 计数系统：dec（十进制）、hex（十六进制）、oct（八进制）

  ```
  hex(cout);			// 函数
  cout << hex;		// 控制符（长久有效，直到下一次设置）
  ```

- 字段宽度/填充字符/浮点数的显示精度

  - width 只影响下一次，然后字段宽度将恢复为默认值
  - C++永远不会截断数据，当打的数据超过字段宽度，C++将增宽字段
  - 默认字段宽度为0，填充为 ‘ ’，右对齐

  ```cpp
  int width();		// 返回当前字段设置
  int width(int i);	// 设置字段宽度，并返回字段宽度值
  ```

  - 填充字符（一直有效，直到被重新设置）

  ```
  cout.fill('*');
  ```

  - 浮点数的显示精度（一直有效，直到重新被设置）

  ```
  cout.precision(2);
  ```

  - setf() 函数（ios_base 类提供，控制多种格式化特性）

  ```
  cout.setf(ios_base::showpoint);	 // 显示末尾小数点
  ```

  - setf() 函数设置标记（位设置）

  ```
  fmtflags setf(fmtflags);	 	// 设置某一位并返回所有标记以前的设置
  fmtflags：bitmask类型的typedef，表示某一个位
  ```

  | 常量                | 含义                                  |
  | ------------------- | ------------------------------------- |
  | ios_base::boolalpha | 输入输出bool值，可以为 true 或 false  |
  | ios_base::showbase  | 对于输出，使用C++前缀                 |
  | ios_base::showpoint | 显示末尾的小数点                      |
  | ios_base::uppercase | 对于16进制输出，使用大写字母，E表示法 |
  | ios_base::showpos   | 在正数前面加 +                        |

  ```cpp
  fmtflags setf(fmtflags, fmtflags);	 	// 设置第一个参数位，清除第二个参数位（清除哪些位）
  ```

  | 第二个参数            | 第一个参数           | 含义                         |
  | --------------------- | -------------------- | ---------------------------- |
  | ios_base::basefield   | ios_base::dec        | 基数10                       |
  |                       | ios_base::oct        | 基数8                        |
  |                       | ios_base::hex        | 基数16                       |
  | ios_base::floatfield  | ios_base::fixed      | 使用定点计数法               |
  |                       | ios_base::scientific | 使用科学计数法               |
  | ios_base::adjustfield | ios_base::left       | 左对齐                       |
  |                       | ios_base::right      | 右对齐                       |
  |                       | ios_base::internal   | 符号或者基数左对齐，值右对齐 |

  ```c++
  /* 设置并恢复原来 */
  ios_base::fmtflags old = cout.setf(ios::left, ios::adjustfield);
  cout.setf(old, ios::adjustfield);
  ```

  unsetf() 消除位

  ```cpp
  void unsetf(fmtflags mask);			// mask中对应位被复位
  ```

### 标准控制符

c++ 提供一些标准的控制符

````cpp
cout << left << fixed;
````

### 头文件iomanip

C++在头文件 iomapip 中提供了其它的一些控制符

- setprecision() 设置精度
- setfill() 设置填充字符
- setw() 设置字段宽度

```cpp
#include<iomapip>

setw(6);
setprecision(3);
setfill('*');
```

## 3. cin 进行输入(istream)

```cpp
cin >> value_holder;

istream& operator>>(int &);			// 重载抽取运算符

cin >> hex;				// 可以配合 hex、oct、dec 控制符进行输入
```

针对 char*   ; signed char*  ; unsigned char*；抽取运算符将会读取输入中的下一个单词，把他放置指定地址

- 抽取运算符 >> 他读取从非空白字符开始，到与目标类型不匹配的第一个字符之间的全部内容

### 流状态

- 流状态被定义为 iostate 类型（一种bitmask）：eofbit、badbit、failbit 组成
- eofbit：cin操作到达文件末尾时
- badbit：在一些无法诊断的失败破坏流时
- failbit：cin操作未能读到预期的字符时或者IO失败（没有读写权限）

| 成员                    | 描述 |
| ----------------------- | ---- |
| eofbit                  |      |
| badbit                  |      |
| failbit                 |      |
| goodbit                 |      |
| good()                  |      |
| eof()                   |      |
| bad()                   |      |
| fail()                  |      |
| rdstate()               |      |
| exceptions()            |      |
| exceptions(isostate ex) |      |
| clear(iostate s)        |      |
| setstate(iostate s)     |      |

```
clear();
clear(eofbit);
setstate(eofbit);
```

### I/O和异常

- exceptions() 方法来控制异常如何被处理，默认设置为 goodbit，即不引发异常
- clear() （setstate() 底层也是clear）将当前的流状态与 expections() 返回的值进行比较。如果在返回值中某一位被设置，而当前状态流中的对应位也被设置，则 clear() 将引发 ios_base::failure 异常
- ios_base::failure 异常类从 std::exception 类派生而来的，包含一个 what() 方法

```cpp
#include<iostream>
#include<exception>

int main()
{
	using namespace std;
	cin.exceptions(ios_base::failbit);
	int sum = 0;
	int input;
	try{
		while(cin >> input)
		{
			sum += input;
		}
	}catch(ios_base::failure &bf)
	{
		cout << bf.what() << endl;
		cout << "error!" << endl;
	}
	
	return 0;
}
```

只有在流状态良好（所有位都被清除）的情况下，下面测试才返回 true

```cpp
while(cin >> input)
```

跳过一行（两种方法）

```cpp
/* 1. */
while(!isspace(cin.get()))
	continue;
	
/* 2. */
while(cin.get() != '\n')
	continue;
```

### 非格式化输入函数

- get(char&) 和 get(void) 提供不跳过空白的单字符输入功能，读取下一个输入字符，即使是空格、制表符、换行符
- get(char* , int, char) 和 getline(char *, int , char)在默认情况下读取整行而不是一个单词

注意！！！ cin 输入会跳过空格

- cin.get(char &) 到达文件尾，不会给其参数赋值，还会调用 setstate (failbit)，导致cin的测试结果为false

```cpp
char ch;
while(cin.get(ch))
{
	// process
}
```

- cin.get() 返回是 int 类型，不能链式！！！，到达文件尾后，将返回EOF（文件 iostream 提供的常量）

```cpp
int ch;
while((ch = cin.get()) != EOF)
{
    // process
}
```

### 单字符输入形式

- 选择 >> ，希望跳过空白，例如会自动跳过 ‘ \n’
- 选择 get()，程序将检查每个字符，get(char &)也可以

### 字符串处输入形式选择:getline()、get() 和 ignore()

```cpp
// 在读取最大数目的字符或遇到换行符后为止
istream& get(char *, int, char);
istream& get(char *, int);
istream& getline(char *, int, char);
istream& getline(char *, int);

istream& ignore(int = 1, int = EOF);		// 读取并丢弃后续的字符(EOF读到指定数目或者末尾)
ignore(255, '\n');
```

get() 和 getline() 区别：

- get() 将换行符留在输入流中，接下来首先看到的将是换行符
- getline() 抽取并丢弃输入流中的换行符

````cpp
#include<iostream>

const int Limit = 255;

cin.getline(input, Limit, '#');		// 会抽取 '#'
cin.get(input, Limit, '#');			// 会留下 '#'
````

### 意外情况下字符串输入（无法抽取和输入超过指定的最大字符数）

无法抽取：

- 无法抽取情况，将把空值字符放置到输入字符串中，并使用 setstate() 设置 failbit
- 输入方法立刻到达了文件尾，对于 get(char*, int) 来说，可能输入了一个空行
- 空行不会导致 getline() 设置 failbit，getline会抽取空行并能丢弃

```cpp
char temp[80];
while(cin.get(temp, 80));				// 输入空行停止，会设置标志位

char temp[80];
while(cin.getline(temp, 80) && temp[0] != '\0')		// 输入空行停止
```

输入超过指定的最大字符数：

- getline() 遇到文件尾，会设置 eofbit；若读取了最大字符数-1，并且下一个不是换行符，设置failbit
- get() 读取了最大数目的字符，并不会设置 failbit，判断是遇到了换行符停止还是读取了最大数目，可以使用peek() 查看缓冲区的下一个字符是不是换行符，此方法不适用于getline()

### 其它istream方法

- read() 与 getline() 和 get() 区别：read() 不会在输入后加入空值字符，因此不能将输入转为字符串

- peak() 返回输入中的下一个字符，但不抽取输入流中的字符

  ```cpp
  char ch = cin.peak();
  ```

- gcout() 返回最后一个非格式化读取的字符数（由 get()、getline()、ignore()、read()）

- putback() 将一个字符插入到输入字符串中，被插入的字符将是下一条输入语句读取的一个字符

## 4. 文件输入和输出

- C++在头文件 fstream中定义了多个新类，其中包括用于文件输入的 ifstream 和 用于文件输出的 ofstream

### 写入文件：

1. 创建一个 ofstream 对象来管理输出流
2. 将该对象与特定的文件关联起来
3. 以使用cout的方式使用该对象

如此打开，创建一个新文件，或者将对原文件做截断清空处理

```cpp
ofstream fout;
fout.open("jar.txt");

ofstream fout("jar.txt");			// 直接用构造函数打开
```

### 读取文件：

1. 创建一个ifstream 对象来管理输入流
2. 将该对象与特定的文件关联起来
3. 使用cin的方式使用该对象

```cpp
ifstream fin;
fin.open("jar.txt");

ifstream fin("jar.txt")
```

当输入和输出流对象过期，和文件的连接将自动关闭，或者也可以利用close() 显式关闭

关闭连接并不会删除流，而只是断开流到文件的连接

```cpp
fout.close();
fin.close();
```

### 文件模式

- 将流和文件关联的时，可以提供文件模式的第二个参数

```cpp
ifstream fin("jar.txt", mode1);

ofstream fout();
fout.open("jar.txt", mode2);
```

| 常量             | 含义                     |
| ---------------- | ------------------------ |
| ios_base::in     | 打开文件，以便读取       |
| ios_base::out    | 打开文件，以便写入       |
| ios_base::ate    | 打开文件，并移到文件尾   |
| ios_base::app    | 追加到文件尾             |
| ios_base::trunc  | 如果文件存在，则截断文件 |
| ios_base::binary | 二进制文件               |

- ifstream open() 和 构造函数用 ios_base::in 作为默认值
- ofstream open() 和 构造函数用 ios_base::out | ios_base::trunc 作为默认值
- fstream 类不提供默认的模式值，在创建这种类的对象时，必须显式地提供模式

### 二进制文件

```cpp
const int LIM = 20;
struct planet
{
	char name[LIM];
	double population;
	double g;
};
planet p1;

// 二进制写入
ofstream fout("planet.dat", ios_base::out | ios_base::app | ios_base::binary);
fout.write((char *)&p1, sizeof p1);

// 二进制读取
ifstream fin("planet.dat", ios_base::in | ios_base::binary);
fin.read((char *) &p1, sizeof p1);
```

### 随机存取

- 可直接移动到文件的任意位置

fstream类继承了两个方法：

- seekg()：输入指针移到指定为文件位置，用于 ifstream 对象
- seekp()：输出指针移到指定的文件位置，用于 ofstream 对象

```cpp
//原型
basic_istream<charT, traits>& seekg(off_type, ios_base::seekdir);
basic_istream<charT, traits>& seekg(pos_type);

istream& seekg(streamoff, ios_base::seekdir);
istream& seekg(streampos);

ios_base::seekdir:
 - ios_base::beg: 文件开始
 - ios_base::cur：当前位置
 - ios_base::end：相对于文件尾的偏移量
 
 streamoff：据 seekdir 处的偏移量（字节）
 
 streampos：文件中的绝对位置（从文件开始处算起）
```

```cpp
fstream finout;
finout.open(file, ios::in | ios::out | ios::binary);

finout.seekg(0);
```

检查文件指针的当前位置：（都返回一个 streampos），输入指针和输出指针彼此独立移动

- 输入流，tellg() 方法
- 输出流，tellp() 方法

```cpp
#include<iostream>
#include<fstream>
#include<iomanip>
#include<cstdlib>

const int LIM = 20;
struct planet
{
	char name[LIM];
	double population;
	double g;
};

const char *file = "planets.dat";
inline void eatline() {while(std::cin.get() != "\n") continue;}

int main()
{
	using namespace std;
	planet p1;
	
	fstream finout;
	finout.open(file, ios_base::in | ios::base::out | ios_base::binary);
	
	int ct = 0;
	if(finout.is_open())
	{
		finout.seekg(0);
		while(finout.read((char *) &p1, sizeof p1) )
		{
			ct++;
			cout << p1.name << p1.population << endl;
		}
		if(finout.eof())
			finout.clear();
		else
		{
			cerr << "err..." << endl;
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		cerr << "err..." << endl;
		exit(EXIT_FAILURE);
	}
	
	long rec;
	cin >> rec;
	eatline();
	if(rec < 0 || rec >= ct)
	{
		cerr << "err..." << endl;
		exit(EXIT_FAILURE);
	}
	streampos place = rec * sizeof(p1);
	finout.seekg(place);
	
	finout.read((char *)&p1, sizeof(p1));
	cin.get(p1.name, LIM);
	eatline();
	cin >> p1.population;
	cin >> p1.g;
	finout.seekp(place);
	finout.write((char *)&p1, sizeof(p1)) << flush;
	
	ct = 0;
	finout.seekg(0);
	while(finout.read((char *)&p1, sizeof(p1)))
	{
		ct++;
		cout << p1.name << p1.population << endl;
	}
	finout.close();
	return 0;
}
```

### 临时文件

使用临时文件，为每个临时文件都指定一个独一无二的文件名

```cpp
#include<cstdio>
// 创建一个临时文件名，将它放在pszName指向的C-风格字符串中
char* tmpnam(char * pszName);
L_tmpnam：文件名包括的字符数
TMP_MAX：确保当前目录中不生成重复文件名的情况下，tmpnam() 可被调用的最多次数
```

```cpp
/* 创建10个临时文件 */
#include<cstdio>
#include<iostream>

int main()
{
	char pszName[L_tmpnam] = {'\0'};
	
	for(int i = 0; i < 10; i++)
	{
		tmpnam(pszName);
		cout << pszName << endl;
	}
	return 0;
}
```

## 5. 内核格式化

C++ 还提供了 sstream 族，使用相同的接口提供程序和string对象之间的I/O

- ostringstream 类（ostream类派生的），可使用 ostream方法将格式化信息写入 string 对象中
- istringstream 类（istream 类派生），可使用 istream 方法来读取 string 对象中的信息

```cpp
ostringstream outstr;
string name;
outstr << "Hello" << name << endl;
string result = outstr.str();
cout << result;
```

```cpp
string list = "hello lv qi feng";
istringstream instr(list);
string word;
while(instr >> word)
{
	cout << word << endl;
}
```
