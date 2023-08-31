# 动态规划

## 1. 动态规划注意点

> 1. 确定 dp 数组以及下标的含义
> 2. 确定递推公式
> 3. dp 数组如何初始化
> 4. 确定遍历顺序
> 5. 举例推导 dp 数组

个人理解：个人认为动态规划的题目重点在于 dp 数组的构造，dp 数组代表的每一步的结果，这里面包含一定的递推关系，dp 数组后面的元素可以通过前面的元素进行推出来；找到递推关系，并依次推出 dp 数组是问题解法的关键！

## 2. 斐波那契数列

```cpp
/* 动态规划 */
class Solution {
public:
    int fib(int N) {
        if (N <= 1) return N;
        vector<int> dp(N + 1);
        dp[0] = 0;
        dp[1] = 1;
        for (int i = 2; i <= N; i++) {
            dp[i] = dp[i - 1] + dp[i - 2];
        }
        return dp[N];
    }
};
```

```cpp
/* 递归实现 */
class Solution {
public:
    int fib(int N) {
        if (N < 2) return N;
        return fib(N - 1) + fib(N - 2);
    }
};
```

## 3. 爬楼梯

一次可以选择爬一格楼梯或者两格楼梯，问爬到 n 层楼有多少种爬楼梯的方法

```cpp
class Solution {
public:
    int climbStairs(int n) {
        if(n < 2)
            return n;
        vector<int> dp(n+1, 0);
        dp[1] = 1;
        dp[2] = 2;
        for(int i = 3; i <= n; i++)
        {
            dp[i] = dp[i-1] + dp[i-2]; 
        }
        return dp[n];
    }
};
```

## 4. 不同路径问题

一个 mxn 网格内，每次只能向下或者向右一格，请问有多少种方法能达到 [m, n] 坐标点

```cpp
class Solution {
public:
    int uniquePaths(int m, int n) {     // m 行 n列
        vector<vector<int>> dp(m, vector<int> (n, 0));

        for(int i = 0; i < m; i++)
        {
            for(int j = 0; j < n; j++)
            {
                // cout << i << j<< endl;
                if(i == 0)
                {
                    dp[0][j] = 1;
                }
                else if(j == 0)
                {
                    dp[i][0] = 1;
                }
                else
                {
                    dp[i][j] = dp[i-1][j] + dp[i][j-1];
                }
            }
        }
        return dp[m-1][n-1];
    }
};
```

## 4.1 不同路径问题

mxn 网格路径中有障碍物阻挡情况下

```cpp
class Solution {
public:
    int uniquePathsWithObstacles(vector<vector<int>>& obstacleGrid) {
        int m = obstacleGrid.size();
        int n = obstacleGrid[0].size();
	if (obstacleGrid[m - 1][n - 1] == 1 || obstacleGrid[0][0] == 1) //如果在起点或终点出现了障碍，直接返回0
            return 0;
        vector<vector<int>> dp(m, vector<int>(n, 0));
        for (int i = 0; i < m && obstacleGrid[i][0] == 0; i++) dp[i][0] = 1;
        for (int j = 0; j < n && obstacleGrid[0][j] == 0; j++) dp[0][j] = 1;
        for (int i = 1; i < m; i++) {
            for (int j = 1; j < n; j++) {
                if (obstacleGrid[i][j] == 1) continue;
                dp[i][j] = dp[i - 1][j] + dp[i][j - 1];
            }
        }
        return dp[m - 1][n - 1];
    }
};
```

## 5.0-1背包

一个背包有限定重量，有若干物体，每种物体都有各自的价值及重量，只能拿物体只能拿一份，问背包能装的最大价值

dp 初始化状态：横坐标表示背包的重量；纵坐标表示物品的种类

dp递推关系： dp\[i][j] = max(dp\[i - 1][j], dp\[i - 1][j - weight[i]] + value[i]);

![image-20230831163328869](D:\typora_doc\typora_pic\image-20230831163328869.png)

![image-20230831141431866](D:\typora_doc\typora_pic\image-20230831141431866.png)

![image-20230831163305768](D:\typora_doc\typora_pic\image-20230831163305768.png)

```cpp
void test_2_wei_bag_problem1() {
    vector<int> weight = {1, 3, 4};
    vector<int> value = {15, 20, 30};
    int bagweight = 4;

    // 二维数组
    vector<vector<int>> dp(weight.size(), vector<int>(bagweight + 1, 0));

    // 初始化
    for (int j = weight[0]; j <= bagweight; j++) {
        dp[0][j] = value[0];
    }

    // weight数组的大小 就是物品个数
    for(int i = 1; i < weight.size(); i++) { // 遍历物品
        for(int j = 0; j <= bagweight; j++) { // 遍历背包容量
            if (j < weight[i]) dp[i][j] = dp[i - 1][j];
            else dp[i][j] = max(dp[i - 1][j], dp[i - 1][j - weight[i]] + value[i]);

        }
    }

    cout << dp[weight.size() - 1][bagweight] << endl;
}

int main() {
    test_2_wei_bag_problem1();
}
```

从递推关系可以看出，dp 的每一层数值都依赖于上一层，所以可以用滚动数组去解决这个问题

![image-20230831143837338](D:\typora_doc\typora_pic\image-20230831143837338.png)



注意：一下层右下角的值是取决于上一层左上角的值，所以用滚动数组的时候，要从后往前覆盖数据

````cpp
void test_1_wei_bag_problem() {
    vector<int> weight = {1, 3, 4};
    vector<int> value = {15, 20, 30};
    int bagWeight = 4;

    // 初始化
    vector<int> dp(bagWeight + 1, 0);
    for(int i = 0; i < weight.size(); i++) { 			// 遍历物品
        for(int j = bagWeight; j >= weight[i]; j--) { // 遍历背包容量
            dp[j] = max(dp[j], dp[j - weight[i]] + value[i]);		// 重点在这里循环顺序上，从右到左进行覆盖
        }
    }
    cout << dp[bagWeight] << endl;
}

int main() {
    test_1_wei_bag_problem();
}
````

## 5.1 完全背包

背包装的东西数量不在受限制，只是在0-1背包滚动数组的基础上，改变一下 循环顺序，从左到右进行覆盖，意味着可以取多次物品

```cpp
void test_CompletePack() {
    vector<int> weight = {1, 3, 4};
    vector<int> value = {15, 20, 30};
    int bagWeight = 4;
    vector<int> dp(bagWeight + 1, 0);
    for(int i = 0; i < weight.size(); i++) { // 遍历物品
        for(int j = weight[i]; j <= bagWeight; j++) { 			// 遍历背包容量
            dp[j] = max(dp[j], dp[j - weight[i]] + value[i]);
        }
    }
    cout << dp[bagWeight] << endl;
}
int main() {
    test_CompletePack();
}
```

## 6.目标和（！！！！超级妙）

强调！！！动态规划的题目重点在于dp数组！！！

dp 数组代表的含义不可以混淆，一般都是题目要求的结果；dp 数组确定后，一定要找出 dp 数组的递推关系；后面的 dp 一定能通过前面 dp 进行推导出来

题目：

给一个非负整数数组，通过在每个整数前面添加 “+” 或者 “-”，实现算术结果为 S；问有几种的实现方法？

题目问题转化！！！！

将数组拆分为两个子集且满足 left - right = S；通过对于给定数组 sum = left + right 是可以已知的！

left = (S+sum) / 2，也就是说只要能找出这样一个子集和为 left 就可以实现通过添加符号来实现算术结果为 S

**题目变为从给定数组中抽出子集实现加法和为S，有多少种方法！！！！！**

dp 横坐标表示加数结果（背包的容量限制，容量为S）纵坐标及是给定数组中每一个的元素（可以选择拿或者不拿！！）

dp[] 通常表示题目所求，表示有多少种方法

**重点来了，找dp之间的递推关系**

要拿一个数（下标为 i，可拿可不拿）组成 j（背包的限额值）有多少种结果

dp\[i][j] = dp\[i-1][j]（表示这轮的数我不拿有多少种方法) + d\[i-1][j-nums[i]]（拿了这个数，和为（S-本轮数值）有多少种方法） 

**初始化dp问题！！！**

dp 明显由前面（上一行，也可以理解为左上角）的 dp 推导出来的，那么 dp\[0][0] 表示背包限额为0的时候第一个数取不取的方法，dp\[0][0] = 1（代表不取）

 d\[0][nums[i]] = 1（代表取这个数，有1种方法）

**以 nums: [1, 1, 1, 1, 1], S: 3 为例！！遍历流程就是这样**

![image-20230831174247576](D:\typora_doc\typora_pic\image-20230831174247576.png)



```cpp
/* 简化为 滚动数组 实现 */
class Solution {
public:
    int findTargetSumWays(vector<int>& nums, int S) {
        int sum = 0;
        for (int i = 0; i < nums.size(); i++) sum += nums[i];
        if (abs(S) > sum) return 0; // 此时没有方案
        if ((S + sum) % 2 == 1) return 0; // 此时没有方案
        int bagSize = (S + sum) / 2;
        vector<int> dp(bagSize + 1, 0);
        dp[0] = 1;
        for (int i = 0; i < nums.size(); i++) {
            for (int j = bagSize; j >= nums[i]; j--) {
                dp[j] += dp[j - nums[i]];
            }
        }
        return dp[bagSize];
    }
};
```

