二叉树的深度遍历：

- 前序遍历（中左右）
- 中序遍历（左中右）
- 后序遍历（左右中）

```cpp
/* 二叉树链式存储的定义 */
struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x): val(x), left(NULL), right(NULL) {}
};
```

```cpp
/* 递归的方式进行遍历 */

// 1. 前序遍历
class Soultion {
public:
    // 递归实现
    void traversal(TreeNode * cur, vector<int>& vec)
    {
        if(cur == NULL) return;
        vec.push_back(cur->val);		// 中
        traversal(cur->left, vec);		// 左
        traversal(cur->right, vec);		// 右
    }
    
    vector<int> preorderTraversal(TreeNode* root)
    {
        vector<int> result;
        traversal(root, result);
        return result;
    }
};

// 2. 中序遍历
class Soultion {
public:
    // 递归实现
    void traversal(TreeNode * cur, vector<int>& vec)
    {
        if(cur == NULL) return;
        traversal(cur->left, vec);		// 左
        vec.push_back(cur->val);		// 中
        traversal(cur->right, vec);		// 右
    }
    
    vector<int> preorderTraversal(TreeNode* root)
    {
        vector<int> result;
        traversal(root, result);
        return result;
    }
};


// 3. 后序遍历
class Soultion {
public:
    // 递归实现
    void traversal(TreeNode * cur, vector<int>& vec)
    {
        if(cur == NULL) return;
        traversal(cur->left, vec);		// 左
        traversal(cur->right, vec);		// 右
        vec.push_back(cur->val);		// 中
    }
    
    vector<int> preorderTraversal(TreeNode* root)
    {
        vector<int> result;
        traversal(root, result);
        return result;
    }
};
```

```cpp
/* 迭代法实现树的遍历 */
// 1. 前序遍历（利用栈先进后出的特点实现）、
class Solution{
public:
    vector<int> preorderTraversal(TreeNode* root){
        vector<int> result;
        stack<TreeNode*> st;
        st.push(root);		// 先将中点进行压入 
        while(!st.empty())
        {
            TreeNode* node = st.top();		// 以弹出点作为节点压入右边和左边，弹出就是先弹左边再弹右边
            st.pop();
            result.push_back(node->val);
            if(node->right)
                st.push(node->right);	// 压入右边
            if(node->left)
                st.push(node->left);	// 压入左边
        }
        return result;
    }
};

// 2. 中序遍历
class Soultion{
public:
    vector<int> inorderTraversal(TreeNode* root)
    {
        vector<int> result;
        stack<TreeNode*> st;
        TreeNode *cur = root;
        while(cur != NULL || !st.empty())
        {
            if(cur != NULL)
            {
                st.push(cur);
                cur = cur->left;	// 左
            }
            else
            {
                cur = st.top();		// 中
                st.pop();
                result.push_back(cur->val);
                cur = cur->right;	// 右
            }
        }
        return result;
    }
};

// 3. 后序遍历
// 利用先序遍历 中左右  -->  中右左（利用先序的模式） ---> 左右中（取反）
class Solution{
public:
    vector<int> postorderTraversal(TreeNode *root)
    {
        vector<int> result;
        stack<TreeNode*> st;
        if(root == NULL)
            return result;
        st.push(root);
        while(!st.empty())
        {
            TreeNode* node = st.top();
            st.pop();
            result.push_back(node->val);
            if(node->left)
                st.push(node->left);
            if(node->right)
                st.push(node->right);
        }
        reverse(result.begin(), result.end());
        return result;
    }
};
```

