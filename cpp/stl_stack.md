# STL  stack

- 容器适配器（一个封装了序列容器的类模板，它在一般序列容器的基础上提供了一些不同的功能）

![1691725413165](E:\typora\markdownImage\1691725413165.png)

- stack 容器适配器的模板有两个参数：第一个参数是存储对象的类型，第二个参数是底层容器的类型；
- stack<T> 的底层容器默认是 deque<T> 容器；通过指定第二个模板类型参数，可以使用任意类型的底层容器，只要它们支持 back()、push_back()、pop_back()、empty()、size() 这些操作。下面展示了如何定义一个使用 list<T> 的堆栈

```c++
stack<typename T, typename Container=deque<T>> name;

std::stack<std::string> words;
std::stack<std::string,std::list<std::string>> fruit;
```

创建堆栈时，不能在初始化列表中用对象来初始化，但是可以用另一个容器来初始化，只要堆栈的底层容器类型和这个容器的类型相同

```c++
std::list<double> values {1.414, 3.14159265, 2.71828};
std::stack<double,std::list<double>> my_stack (values);
```

stack<T> 模板定义了拷贝构造函数，因而可以复制现有的 stack 容器；在使用拷贝构造函数时，既可以用初始化列表，也可以用圆括号

```c++
std::stack<double,std::list<double>>copy_stack {my_stack}
```

## 堆栈操作

- top()：返回一个栈顶元素的引用，类型为 T&。如果栈为空，返回值未定义
- push(const T& obj)：可以将对象副本压入栈顶。这是通过调用底层容器的 push_back() 函数完成的
- push(T&& obj)：以移动对象的方式将对象压入栈顶。这是通过调用底层容器的有右值引用参数的 push_back() 函数完成的
- pop()：弹出栈顶元素
- size()：返回栈中元素的个数
- empty()：在栈中没有元素的情况下返回 true
- emplace()：用传入的参数调用构造函数，在栈顶生成对象
- swap(stack<T> & other_stack)：将当前栈中的元素和参数中的元素交换。参数所包含元素的类型必须和当前栈的相同。对于 stack 对象有一个特例化的全局函数 swap() 可以使用
