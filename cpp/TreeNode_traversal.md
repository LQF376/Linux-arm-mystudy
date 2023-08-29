# 二叉树遍历方式

## 1. 深度优先遍历

从遍历的种类来说：

> - 前序遍历（中 左 右）
> - 中序遍历（左 中 右）
> - 后序遍历（左 右 中）

从实现的方法来说：

> - 递归法（递归调用来实现 简单）
> - 利用栈先进后出的特点来实现（前序后序实现方式相同，中序得先找左节点）

### 1.1.1 前序遍历（利用递归法来实现）

```cpp
class Solution {
public:
    void orderTraversal(vector<int>& vec, TreeNode* node)
    {
        if(node == NULL)
            return;
        vec.push_back(node->val);           // 中
        orderTraversal(vec, node->left);    // 左
        orderTraversal(vec, node->right);   // 右

    }

    vector<int> preorderTraversal(TreeNode* root) {
        vector<int> ret;
        if(root == NULL)
            return ret;
        orderTraversal(ret, root);
        return ret;
    }
};
```

### 1.1.2 中序遍历（利用递归法来实现）

```cpp
class Solution {
public:
    void orderTraversal(vector<int> &vec, TreeNode* node)
    {
        if(node == NULL)
            return;
        orderTraversal(vec, node->left);
        vec.push_back(node->val);
        orderTraversal(vec, node->right);
    }

    vector<int> inorderTraversal(TreeNode* root) {
        vector<int> ret;
        orderTraversal(ret, root);
        return ret;
    }
};
```

### 1.1.3 后序遍历（利用递归法来实现）

```cpp
class Solution {
public:
    void orderTraversal(vector<int> &vec, TreeNode* node)
    {
        if(node == NULL)
            return;
        orderTraversal(vec, node->left);
        orderTraversal(vec, node->right);
        vec.push_back(node->val);
    }
    vector<int> postorderTraversal(TreeNode* root) {
        vector<int> ret;
        orderTraversal(ret, root);
        return ret;
    }
};
```

### 1.2.1 前序遍历（利用 stack 实现）

```cpp
class Solution {
public:
    vector<int> preorderTraversal(TreeNode* root) {
        stack<TreeNode*> st;
        vector<int> ret;
        if(root == NULL)
            return ret;
        st.push(root);
        while(!st.empty())
        {
            TreeNode* temp = st.top();      // 中
            st.pop();
            ret.push_back(temp->val);
            if(temp->right != NULL)
                st.push(temp->right);           // 右
            if(temp->left != NULL)
                st.push(temp->left);            // 左
        }
        return ret;
    }
};
```

### 1.2.2 中序遍历（利用 stack 实现）

```cpp
class Solution {
public:
    vector<int> inorderTraversal(TreeNode* root) {
        stack<TreeNode*> st;
        vector<int> ret;
        if(root == NULL)
            return ret;
        TreeNode* cur = root;
        while(cur != NULL || (!st.empty()))
        {
            if(cur != NULL)
            {
                st.push(cur);
                cur = cur->left;        // 左
            }
            else			// 直到左边走到当前节点为NULL，开始退一步
            {
                TreeNode* temp = st.top();		// 中
                st.pop();
                ret.push_back(temp->val);
                cur = temp->right;				// 右
            }
        }
        return ret;
    }
};
```

### 1.2.3后序遍历（利用 stack 实现）

整体思路：后序遍历（左 右 中）；利用前序遍历（中 右 左）进行调转来实现

```cpp
class Solution {
public:
    vector<int> postorderTraversal(TreeNode* root) {
        stack<TreeNode*> st;
        vector<int> ret;

        if(root == NULL)
            return ret;

        st.push(root);
        while(!st.empty())
        {
            TreeNode* temp = st.top();
            st.pop();
            ret.push_back(temp->val);      // 中
            if(temp->left != NULL)
                st.push(temp->left);     // 左
            if(temp->right != NULL)
                st.push(temp->right);   // 右
        }
        reverse(ret.begin(), ret.end());        // 中左右 前序遍历的方式 进行取反结果就是后序

        return ret;
    }
};
```

## 2. 广度优先遍历

### 2.1 层序遍历（递归法实现）

```cpp
class Solution {
public:
    void order(vector<vector<int>> &vec, TreeNode *node, int depth)
    {
        if(node == NULL)	// 递归推出条件
            return;
        if(vec.size() == depth)
            vec.push_back(vector<int>());		// 插入新的一层
        vec[depth].push_back(node->val);
        order(vec, node->left, depth+1);
        order(vec, node->right, depth+1);
    }


    vector<vector<int>> levelOrderBottom(TreeNode* root) {
        vector<vector<int>> ret;
        int depth = 0;
        
        order(ret, root, depth);
        
        return ret;
    }
};
```

### 2.2 层序遍历（利用 queue 实现）

利用 queue 队列来实现广度优先遍历

```cpp
class Solution {
public:
    vector<vector<int>> levelOrderBottom(TreeNode* root) {
        queue<TreeNode*> que;
        vector<vector<int>> ret;
        
        if(root == NULL)
            return ret;
        
        que.push(root);
        while(!que.empty())
        {
            int size = que.size();		// que.size() 会动态变化，这里必须确认每层的个数
            vector<int> vec;
            for(int i = 0; i < size; i++)	// 每次送出该层的个数
            {
                TreeNode *temp = que.front();
                que.pop();
                vec.push_back(temp->val);
                if(temp->left)
                    que.push(temp->left);
                if(temp->right)
                    que.push(temp->right);
            }
            ret.push_back(vec);
        }
        return ret;
    }
};
```

