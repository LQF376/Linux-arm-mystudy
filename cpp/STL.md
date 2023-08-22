# vector

```cpp
#include<vector>

/* 定义一个 vector */
vector<typename> name;
vector<typename> Arrayname[size]; 		// 二维 vector 数组

/* 1. 初始化 vector */
vector<int> v1{1, 2, 3, 4, 5};		// 花括号 初始化

vector<int> v2(v1.begin(), v1.end());		// 用一个 vector 给 另一个 vector 初始化
vector<int> v2(10);			// 最大容量初始化
vector<int> v2(10, 100);	// 10 个 100    vector(n, elem) 方式

/* 2. 访问/遍历方式 */
	// 2.1 通过下标来访问		注意下标
v1[0];  // 等价于 *( v.begin() )

	// 2.2 通过迭代器来访问
vector<typename>::iterator it;
for(vector<int>::iterator it = v1.begin(); it != end(); it++)
{
    cout << *it << endl;
}

	// 2.3 一些特殊的访问方法
for(auto i : v1)	// 遍历容器
	cout << i << endl;

v1.front();		// 访问第一个元素（头部）
v1.back();		// 访问最后一个元素（尾部）
v1.at(index);	// index 下标（int 类型）

/* 3. 插入操作 */
v1.push_back(1);		// 尾部插入
v1.pop_back();			// 尾部弹出
v1.insert(pos, ele);	// 在 pos 处插入 ele
v1.insert(pos, count, ele); 	// 在 pos 处插入 count 个 ele
v1.erase(pos);		// 清除 pos 处元素
v1.erase(pos_start, pos_end);	// 清除 [pos_start, pos_end) 区间的元素

v1.size();			// vector 中元素的个数
v1.clear(); 		// 清空 vector 中所有元素
v1.resize(int num);		// 重新指定容器的长度
v1.resize(int num, elem); // 重新指定容器长度，拓展部分用 elem 填充
v1.empty();			// 判容器是否为空

v1.capacity();		// 返回分配存储的大小
v1.reserve(int len);	// 预留 len 个元素长度预留位置（容量），元素不可访问

v1.swap(v2);		// 容器交换
// resize 直接修改了数据量的大熊啊，让我们不能访问后面的数据，而并没有释放空间
vector<int> v1;
for(int i = 0; i < 10000; i++)
{
    v1.push_back(i);
}
v1.resize(3);
vector<int>(v1).swap(v1);	// 匿名对象做替换
```

## string 容器

```cpp
#include<string>

/* 1. sting 初始化 */
	// 1. 无参默认构造
string s1;
	// 2. sting(const char *s)
string s1("hello world");
	// 3. string(const string& str)
string s2(s1);
	// 4. string(int n, char c)
string s1(10, 'x');
	// 5. string(const string& str, size_t pos, size_t len = npos) 字符串字串初始化
string s2(s1, 5, 5);
	// 6. string(const char* s, size_t n) 字符串的前n个字符初始化
string s1("hello world", 6);


/* 2. 字符串拼接 */
s1 += "hello world";	// string& operator += (const char* str)；
s1 += 'a';				// string& operator += (const char c)；
s2 += s1;				//  string& operator += (const string& str)；
s1.append("hello world");	// string& append(const char* s);
s1.append("hello world", 6);	// string& append(const char* s, int n);
s2.append(s1);			// string& append(const string& s);
s2.append(s1, 0, 6);	// string& append(const string& s, int pos, int n);

/* 3. 查找和替换 （查找返回的是下标） */
s1.find("abc", 0);	// int find(const char* s, int pos=0)const;
s1.find("abcd", 0, 3);	// int find(const char* s, int pos, int n)const;  从 pos 查前 s 前 n 个
s2.find(s1, 0);		// int find(const sting& str, int pos=0)const;

// sting& replace(int pos, int n, const char *s)
s1.replace(1, 3, "aaa");	// s1 的 pos 开始 n个字符替换成 aaa
// sting& replace(int pos, int n, const string str)
s2.replace(1, 3, s2);		// s2 的 pos 开始 n个字符替换成 s1

/* 4. 字符串比较 */
s1.compare(s2);
s1.compare("hello word");

/* 5.字符串存取 */
// 1. char& operator[](int n);
s1[2];
// 2. char& at(int n);
s1.at(2);

/* 6.插入和删除 */
// 1. string& insert(int pos, const char *s);
// 2. string& insert(int pos, const string& str);
// 3. string& insert(int pos, const char c);

// 1. string& erase(int pos, int n = npos)；

/* 7.提取字串 */
// 1. string& substr(int pos = 0, int n = npos)const;
```

## deque 容器

```cpp
#include<deque>

/* 1. 初始化容器 */
// deque<typename>deq;	默认构造
deque<int> deq1;

deque deq2(deq1);
deque deq2(deq1.begin(), deq1.end());
deque deq2(10, 100);

/* 2. 赋值操作 */
// 1. deque& operator=(const deque& deq);
// 2. assign(beg, end);		// [beg, end) 区间（迭代器）
// 3. assign(n, elem);		// n 个 elem

/* 3. deque 大小操作（deque没有容量概念） */
deque.empty();
deque.size();
deque.resize(int num);
deque.resize(int num, int elem);

/* 4. 插入和删除 */
deq1.push_back(elem); 		// 容器尾部插入
deq1.push_front(elem);		// 容器头部插入

deq1.pop_back();	// 容器尾部弹出
deq1.pop_front();	// 容器头部弹出

deq1.insert(pos, elem);		// pos 位置插入 elem
deq1.insert(pos, n, elem);	// pos 位置插入 n 个 elem
deq1.insert(pos, beg, end);	// pos 位置插入[beg, end) 数据

deq1.erase(beg, end);	// 删除 [beg, end) 区间的元素
deq1.erase(pos);

deq1.clear();		// 清空

/* 5. 访问元素 */
// 1. at(int index);
deq1.at(1);
// 2. operator[];	通过下标访问
deq1[1];
// 3. front();
// 4. back();
deq1.front();
deq1.back();

/* 6. 排序（针对随机访问迭代器） */
#include<algorithm>
// sort(iterator beg, iterator end);    [beg, end) 升序排列
sort(deq1.begin(), deq1.end())
```

## stack 容器（栈）

```cpp
/* 1. 初始化 */
stack<typename> stk;
stack(const stack& stk);

/* 2. 赋值 */
stack& operator=(const stack& stk);

/* 3. 其他操作 */
stk.push(elem);		// 入栈
stk.pop();			// 弹出栈顶元素
stk.top();			// 查看栈顶的元素

stk.size();			// 返回栈的大小
stk.empty();		// 判断栈是否为空
```

## queue 容器（队列）

```cpp
/* 1. 初始化 */
// 1. queue<typename> que;
queue<int> que1;
// 2. queue(const queue& que);
queue<int> que2(que1);

/* 2. 赋值 */
// 1. queue& operator=(const queue& que);
que1 = que2 

/* 3. 入队和出队 */
que.push(elem);			// 队尾入栈
que.pop();				// 队头出栈

que.front();	// 返回队列头部元素，即将弹出
que.back();		// 返回队列尾部元素，新入队

que.empty();	// 判断队列是否为空
que.size();		// 判断队列长度
```

## list 容器（链表）

```cpp
#include<list>

/* 1. 初始化链表 */
// 1. list<T> list1;
list<int> list1;
// 2. list(beg, end)
list<int> list2(list1.begin(), list1.end());
// 3. list(n, elem)
list<T> list2(10, 100);
// 4. list(const list& list)
list<T> list2(list1);

/* 2. 赋值和交换操作 */
// 1. assign(beg, end);
list2.assign(list1.begin(), list1.end());
// 2. assign(n, elem);
list2.assign(10, 100);

// 3. list& operator= (const list &list);
list2 = list1;

// swap(list);
list1.swap(list2);

/* 3. list 大小操作 */
list.size();
list.empty();
list.resize(num);
list.resize(num, elem);

/* 4. 插入和删除 */
list.push_back(elem);
list.push_front(elem);

list.pop_back();
list.pop_front();

list.insert(pos, elem);
list.insert(pos, n, elem);
list.insert(pos, beg, end);

list.clear();

list.erase(pos);
list.erase(beg, end);

list.remove(elem);		// 删除容器中所有与 elem 匹配的元素

/* 5. 数据存取 */
list.front();
list.back();

// 不支持 [] at 访问（不是随机访问迭代器）
list<T>::iterator it = list1.begin();
it++;
it--;

/* 6. 链表翻转和排序 */
list.reverse();			// 翻转链表
list.sort();			// 链表排序（升序）

// 自定义排序规则
bool myCompare(int v1, int v2)
{
    return v1 > v2;
}
list.sort(myCompare);
```

## set/multiset/unordered_set 容器（元素自动排序）

```cpp
#include<set>

/* 1. set初始化 */
set<T> s1;
// set(const set& st);
set<T> s2(s1);

/* 2. set 赋值 */
set& operator=(const set &st);

/* 3. set 插入和删除 */
s1.insert(elem);
s1.erase(pos);
s1.erase(beg, end);
s1.erase(elem);
s1.clear();

/* 4. 查找和统计 */
it = s1.find(key);		// 查找返回迭代器
s1.count(key);

/* 5. 改变内置排序类型 */
class MyCompare
{
public:
   	bool operator()(int v1, int v2)
    {
        return v1 > v2;
    }
};

set<int, MyCompare()> s2;
s2.insert(10);
```

## map/mutimap/unordered_map 容器

```cpp
pair<type, type> p(value1, value2);
pair<type, type> p = make_pair(value1, value2);

cout << p.first << p.second << endl;

map<T1, T2> map;
map(const map& mp);

map& operator=(const map &map);

/* 3. 插入和删除 */
mp.insert(pair<int, int>(1, 10));
mp.clear();

mp.erase(pos);
mp.erase(beg, end);
mp.erase(key)	// 删除容器中值为 key 的元素

/* 4. 大小和交换 */
mp.size();
mp.empty();
mp.swap(mp);

/* 5. 查找和统计 */
it = mp.find(key);
mp.count(key);

/* 6. map 排序 */
class MyCompare
{
public:
    bool operator()(int v1, int v2)
    {
        return v1 > v2;
    }
}
map<int, int, MyCompare()> m;
```

