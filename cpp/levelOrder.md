二叉树的层序遍历

```cpp
struct TreeNode{
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x): val(x), left(NULL), right(NULL) {}
};
```

```cpp
/* 层序遍历 */
// 1. 利用队列来实现层序遍历
class Soultion{
public:
    vector<vector<int>> levelOrder(TreeNode* root)
    {
        queue<TreeNode*> que;
        vector<vector<int>> result;
        if(root != NULL)
            que.push(root);
        while(!que.empty())
        {
            int size = que.size();			// 每一层要弹出的个数
            vector<int> vec;				// 存放每个层的元素
            for(int i = 0; i < size; i++)	// 不能用 que.size() 动态变化的
            {
                TreeNode *node = que.front();
                que.pop();
                vec.push_back(node->val);
                if(node->left)
                	que.push(node->left);
                if(node->right)
                	que.push(node->right);
            }
            result.push_back(vec);
        }
        return result;
    }
};

// 2. 递归实现层序遍历
class Soultion{
public:
    void order(TreeNode* cur, vector<vector<int>>& result, int depth)
    {
        if(cur == NULL)			// 返回条件
            return;
        if(result.size() == depth)
            result.push(vector<int>());
        result[depth].push_back(cur->val);
        order(cur->left, result, depth + 1);
        order(cur->left, result, depth + 1);
    }
    
    vector<vector<int>> levelOrder(TreeNode *root)
    {
        vector<vector<int>> result;
        int depth = 0;
        order(root, result, depth);
        return result;
    }
};


// 3. 二叉树的层序遍历 从底层开始向上层序 就是对result取反
class Soultion{
public:
    vector<vector<int>> levelOrder(TreeNode* root)
    {
        queue<TreeNode*> que;
        vector<vector<int>> result;
        if(root != NULL)
            que.push(root);
        while(!que.empty())
        {
            int size = que.size();			// 每一层要弹出的个数
            vector<int> vec;				// 存放每个层的元素
            for(int i = 0; i < size; i++)	// 不能用 que.size() 动态变化的
            {
                TreeNode *node = que.front();
                que.pop();
                vec.push_back(node->val);
                if(node->left)
                	que.push(node->left);
                if(node->right)
                	que.push(node->right);
            }
            result.push_back(vec);
        }
        reverse(result.begin(), result.end());			// 反转一下
        return result;
    }
};
```




