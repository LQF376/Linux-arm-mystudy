# 回溯算法

```cpp
void backtracking(参数) {
    if (终止条件) {
        存放结果;
        return;
    }

    for (选择：本层集合中元素（树中节点孩子的数量就是集合的大小）) {
        处理节点;
        backtracking(路径，选择列表); // 递归
        回溯，撤销处理结果
    }
}
```

![image-20230831191123234](D:\typora_doc\typora_pic\image-20230831191123234.png)

```cpp
class Solution {
private:
    vector<vector<int>> result;
    vector<int> path;

    void backtracking(int n, int k, int startIndex)
    {
        if(path.size() == k)        // 递归退出条件
        {
            result.push_back(path);
            return;
        }

        for(int i = startIndex; i <= n; i++)
        {
            path.push_back(i);
            backtracking(n, k, i+1);
            path.pop_back();
        }
    }

public:
    vector<vector<int>> combine(int n, int k) {
        backtracking(n, k, 1);
        return result;
    }
};
```

