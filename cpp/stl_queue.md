# STL  queue

- 只能访问 queue<T> 容器适配器的第一个和最后一个元素。只能在容器的末尾添加新元素，只能从头部移除元素

![1691734491455](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1691734491455.png)


## 初始化 queue

```c++
std::queue<std::string> words;

std::queue<std::string> copy_words {words}; 	// 拷贝构造函数 
```

stack<T>、queue<T> 这类适配器类都默认封装了一个 deque<T> 容器，也可以通过指定第二个模板类型参数来使用其他类型的容器；底层容器必须提供这些操作：front()、back()、push_back()、pop_front()、empty() 和 size()

```c++
std::queue<std::string, std::list<std::string>>words;
```

## queue 操作

- front()：返回 queue 中第一个元素的引用。如果 queue 是常量，就返回一个常引用；如果 queue 为空，返回值是未定义的
- back()：返回 queue 中最后一个元素的引用。如果 queue 是常量，就返回一个常引用；如果 queue 为空，返回值是未定义的
- push(const T& obj)：在 queue 的尾部添加一个元素的副本。这是通过调用底层容器的成员函数 push_back() 来完成的
- push(T&& obj)：以移动的方式在 queue 的尾部添加元素。这是通过调用底层容器的具有右值引用参数的成员函数 push_back() 来完成的
- pop()：删除 queue 中的第一个元素
- size()：返回 queue 中元素的个数
- empty()：如果 queue 中没有元素的话，返回 true
- emplace()：用传给 emplace() 的参数调用 T 的构造函数，在 queue 的尾部生成对象
- swap(queue<T> &other_q)：将当前 queue 中的元素和参数 queue 中的元素交换。它们需要包含相同类型的元素。也可以调用全局函数模板 swap() 来完成同样的操作
