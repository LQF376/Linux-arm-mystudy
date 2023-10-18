# 常见算法

## 1. 队列-模板实现

- 使用模板实现队列，用节点

```cpp
#ifndef QUEUETP_H_
#define QUEUETP_H_

template<class Item>
class QueueTP
{
private:
	enum {Q_SIZE = 10};
	class Node
	{
	public:
		Item item;
		Node* next;
		Node(const Item & i):item(i), next(0){}
	};
	Node* front;
	Node* rear;
	int items;
	const int qsize;
	QueueTP(const QueueTP & q):qsize(0) {}
	QueueTP& operator=(const QueueTP& q) { return *this;}
public:
	QueueTP(int qs = Q_SIZE);
	~QueueTP();
	bool isempty() const
	{
		return items == 0;
	}
	bool isfull() const
	{
		return items == qsize;
	}
	bool queuecount() const
	{
		return items;
	}
	bool enqueue(const Item& item);
	bool dequeue(Item& item);
};

template<class Item>
QueueTP<Item>::QueueTP(int qs) :qsize(qs)
{
	front = rear = 0;
	items = 0;
}

template<class Item>
QueueTP<Item>::~QueueTP()
{
	Node* temp;
	while (front != 0)
	{
		temp = front;
		front = front->next;
		delete temp;
	}
}

template<class Item>
bool QueueTP<Item>::enqueue(const Item& item)
{
	if (isfull())
		return false;
	Node* add = new Node(item);
	items++;
	if (front == 0)
		front = add;
	else
		rear->next = add;
	rear = add;
	return true;
}

template<class Item>
bool QueueTP<Item>::dequeue(Item& item)
{
	if (front == 0)
		return false;
	item = front->item;
	items--;
	Node* temp = front;
	front = front->next;
	delete temp;
	if (items == 0)
		rear = 0;
	return true;
}
#endif
```
## 2.异常类 继承使用

- Sales类存放 具体年份的销售数据
- LabeledSales类 添加了一个具体的标签
- 其中涉及到异常类的继承

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






