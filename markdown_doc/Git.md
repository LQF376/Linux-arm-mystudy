# Git

## 1. 版本控制

### 1.1 什么是版本控制？

版本控制（Revision control）是一种在开发的过程中用于管理我们对文件、目录或工程等内容的修改历史，方便查看更改历史记录，备份以便恢复以前的版本的软件工程技术

- 实现跨区域多人协同开发
- 追踪和记载一个或者多个文件的历史记录
- 组织和保护你的源代码和文档
- 统计工作量
- 并行开发、提高开发效率
- 跟踪记录整个软件的开发过程

简单说就是用于管理多人协同开发项目的技术

### 1.2 常见的版本控制

主流的版本控制器有如下这些：

- **Git**
- **SVN**（Subversion）
- CVS（Concurrent Versions System）

### 1.3 版本控制分类

1. 本地版本控制

   记录文件每次的更新，可以对每个版本做一个快照，或是记录补丁文件，适合个人用，如RCS

2. 集中版本控制 SVN

   所有的版本数据都保存在服务器上，协同开发者从服务器上同步更新或上传自己的修改

   所有的版本数据都存在服务器上，用户的本地只有自己以前所同步的版本，如果不连网的话，用户就看不到历史版本，也无法切换版本验证问题，或在不同分支工作。而且，所有数据都保存在单一的服务器上，有很大的风险这个服务器会损坏，这样就会丢失所有的数据，当然可以定期备份。代表产品：SVN、CVS、VSS

3. **分布式版本控制 	Git**

   每个人都拥有全部的代码！安全隐患！

   所有版本信息仓库全部同步到本地的每个用户，这样就可以在本地查看所有版本历史，可以离线在本地提交，只需在连网时push到相应的服务器或其他用户那里。由于每个用户那里保存的都是所有的版本数据，只要有一个用户的设备没有问题就可以恢复所有的数据，但这增加了本地存储空间的占用。

   不会因为服务器损坏或者网络问题，造成不能工作的情况！

### 1.4 Git 和 SVN 的区别

SVN是集中式版本控制系统，版本库是集中放在中央服务器的，而工作的时候，用的都是自己的电脑，所以首先要从中央服务器得到最新的版本，然后工作，完成工作后，需要把自己做完的活推送到中央服务器。集中式版本控制系统是必须联网才能工作，对网络带宽要求较高。

Git是分布式版本控制系统，没有中央服务器，每个人的电脑就是一个完整的版本库，工作的时候不需要联网了，因为版本都在自己电脑上。协同的方法是这样的：比如说自己在电脑上改了文件A，其他人也在电脑上改了文件A，这时，你们两之间只需把各自的修改推送给对方，就可以互相看到对方的修改了。Git可以直接看到更新了哪些代码和文件！

## 2. Git 的历史

Linux 内核开源项目有着为数众广的参与者。绝大多数的 Linux 内核维护工作都花在了提交补丁和保存归档的繁琐事务上(1991－2002年间)。到 2002 年，整个项目组开始启用一个专有的分布式版本控制系统 BitKeeper 来管理和维护代码。

Linux社区中存在很多的大佬！破解研究 BitKeeper ！

到了 2005 年，开发 BitKeeper 的商业公司同 Linux 内核开源社区的合作关系结束，他们收回了 Linux 内核社区免费使用 BitKeeper 的权力。这就迫使 Linux 开源社区(特别是 Linux 的缔造者 Linus Torvalds)基于使用 BitKeeper 时的经验教训，开发出自己的版本系统。（2周左右！） 也就是后来的 Git！

**Git是目前世界上最先进的分布式版本控制系统**

## 3. 安装 Git

**Git Bash**：Unix与Linux风格的命令行，使用最多，推荐最多

**Git CMD**： Windows风格的命令行

**Git GUI**：图形界面的Git，不建议初学者使用，尽量先熟悉常用命令

## 4. 常用的 Linux 命令

- cd : 改变目录
- pwd : 显示当前所在的目录路径
- ls(ll):  都是列出当前目录中的所有文件，只不过ll(两个ll)列出的内容更为详细
- touch : 新建一个文件 如 touch index.js 就会在当前目录下新建一个index.js文件
- rm:  删除一个文件, rm index.js 就会把index.js文件删除
- mkdir:  新建一个目录,就是新建一个文件夹
- rm -r :  删除一个文件夹, rm -r src 删除src目录
- mv：移动文件
- reset 重新初始化终端/清屏
- clear 清屏
- history 查看命令历史
- help 帮助
- exit 退出
- #表示注释

### Git 配置

所有的配置文件，其实都保存在本地！

```
git config -l # 查看配置
```

![1687609472078](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1687609472078.png)

查看不同级别的配置文件：

```
git config --system --list	# 查看系统
git config --global --list	# 查看当前用户(global)配置
```

**Git 相关的配置文件**

> Git\etc\gitconfig  ：Git 安装目录下的 gitconfig     --system 系统级
>
> C:\Users\Administrator\ .gitconfig    只适用于当前登录用户的配置  --global 全局

**设置用户名和邮箱（初始化git必要！用户标识）**

--global 为全局配置，会使用该信息来处理你在系统的一切操作；不加，则为某个项目的特定配置

```
git config --global user.name "lvqifeng"
git config --global user.email "lvqifeng376@126.com"
```

## 5. Git 基本理论

### 5.1 三个区域

![9360a99445ea43d39ca88079c783b701.png](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/9360a99445ea43d39ca88079c783b701.png)

Git本地有三个工作区域：**工作目录**（Working Directory）、**暂存区**(Stage/Index)、**资源库**(Repository或Git Directory)。如果在加上**远程的git仓库**(Remote Directory)就可以分为四个工作区域。文件在这四个区域之间的转换关系如下：

> - Workspace：工作区，就是你平时存放项目代码的地方
> - Index / Stage：暂存区，用于临时存放你的改动，事实上它只是一个文件，保存即将提交到文件列表信息
> - Repository：仓库区（或本地仓库），就是安全存放数据的位置，这里面有你提交到所有版本的数据。其中HEAD指向最新放入仓库的版本
> - Remote：远程仓库，托管代码的服务器，可以简单的认为是你项目组中的一台电脑用于远程数据交换

本地的三个区域确切的说应该是git仓库中HEAD指向的版本：

> - Directory：使用Git管理的一个目录，也就是一个仓库，包含我们的工作空间和Git的管理空间
> - WorkSpace：需要通过Git进行版本控制的目录和文件，这些目录和文件组成了工作空间
> - .git：存放Git管理信息的目录，初始化仓库的时候自动创建
> - Index/Stage：暂存区，或者叫待提交更新区，在提交进入repo之前，我们可以把所有的更新放在暂存区
> - Local Repo：本地仓库，一个存放在本地的版本库；HEAD会只是当前的开发分支（branch）
> - Stash：隐藏，是一个工作状态保存栈，用于保存/恢复WorkSpace中的临时状态

### 5.2 工作流程

git 的工作流程一般是这样的：

1. 在工作目录中添加、修改文件
2. 将需要进行版本管理的文件放入暂存区域
3. 将暂存区域的文件提交到git仓库

因此，git管理的文件有三种状态：已修改（modified）,已暂存（staged）,已提交(committed)

## 6. Git 项目搭建

### 6.1 创建工作目录与常用指令

工作目录（WorkSpace)一般就是你希望Git帮助你管理的文件夹，可以是你项目的目录，也可以是一个空目录，建议不要有中文

日常使用只需要记住图下6个命令：

![v2-3bc9d5f2c49a713c776e69676d7d56c5_r.png](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/v2-3bc9d5f2c49a713c776e69676d7d56c5_r.png)

### 6.2 仓库的搭建

创建本地仓库的方法有两种：1. 创建全新的仓库；2.克隆远程仓库

1. 创建全新的仓库

   创建完成后，项目目录下会多出一个 .git 目录，记录版本等信息

```
git init	# 在当前目录新建一个 git 代码库
```

2. 克隆远程的仓库

   将远程服务器上的仓库完全镜像一份至本地！

```
git clone [url] # https://github.com/LQF376/Linux-arm-mystudy.git
```

## 7. Git 文件操作

### 7.1 文件的四种状态

版本控制就是对文件的版本控制，要对文件进行修改、提交等操作，首先要知道文件当前在什么状态，不然可能会提交了现在还不想提交的文件，或者要提交的文件没提交上

- Untracked: 未跟踪, 此文件在文件夹中, 但并没有加入到git库, 不参与版本控制. 通过git add 状态变为Staged.
- Unmodify: 文件已经入库, 未修改, 即版本库中的文件快照内容与文件夹中完全一致. 这种类型的文件有两种去处, 如果它被修改, 而变为Modified. 如果使用git rm移出版本库, 则成为Untracked文件
- Modified: 文件已修改, 仅仅是修改, 并没有进行其他的操作. 这个文件也有两个去处, 通过git add可进入暂存staged状态, 使用git checkout 则丢弃修改过, 返回到unmodify状态, 这个git checkout即从库中取出文件, 覆盖当前修改 !
- Staged: 暂存状态. 执行git commit则将修改同步到库中, 这时库中的文件和本地文件又变为一致, 文件为Unmodify状态. 执行git reset HEAD filename取消暂存, 文件状态为Modified

### 7.2 查看文件状态

```
git status [filename]	# 查看指定文件状态
git status				# 查看所有文件状态
```

```
git add . 		# 添加所有文件到暂存区
git commit -m "消息内存"	# 提交暂存区中的内容到本地仓库 -m 提交信息
```

### 7.3 忽略文件

有些时候我们不想把某些文件纳入版本控制中，比如数据库文件，临时文件，设计文件等

在主目录下建立".gitignore"文件，此文件有如下规则：

1. 忽略文件中的空行或以井号（#）开始的行将会被忽略
2. 可以使用Linux通配符。例如：星号（*）代表任意多个字符，问号（？）代表一个字符，方括号（[abc]）代表可选字符范围，大括号（{string1,string2,...}）代表可选的字符串等
3. 如果名称的最前面有一个感叹号（!），表示例外规则，将不被忽略
4. 如果名称的最前面是一个路径分隔符（/），表示要忽略的文件在此目录下，而子目录中的文件不忽略
5. 如果名称的最后面是一个路径分隔符（/），表示要忽略的是此目录下该名称的子目录，而非文件（默认文件或目录都忽略）

```
#为注释
*.txt        #忽略所有 .txt结尾的文件,这样的话上传就不会被选中！
!lib.txt     #但lib.txt除外
/temp        #仅忽略项目根目录下的TODO文件,不包括其它目录temp
build/       #忽略build/目录下的所有文件
doc/*.txt    #会忽略 doc/notes.txt 但不包括 doc/server/arch.txt
```

## 8.码云的使用

### 8.1 添加公钥

设置本机绑定SSH公钥，实现免密码登录！（免密码登录，这一步挺重要的，码云是远程仓库，我们是平时工作在本地仓库！)

![1687681088516](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1687681088516.png)

```
# 进入 C:\Users\Administrator\.ssh 目录
# 生成公钥
ssh-keygen # -t rsa 选择加密算法
```

![1687681342220](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1687681342220.png)

将公钥信息 public key 添加到码云账户中即可！

![1687681434770](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1687681434770.png)

克隆到本地即可！

## 9. Git 分支

```
# 列出所有本地分支
git branch

# 列出所有远程分支
git branch -r

# 新建一个分支，但依然停留在当前分支
git branch [branch-name]

# 新建一个分支，并切换到该分支
git checkout -b [branch]

# 合并指定分支到当前分支
$ git merge [branch]

# 删除分支
$ git branch -d [branch-name]

# 删除远程分支
$ git push origin --delete [branch-name]
$ git branch -dr [remote/branch]
```

如果同一个文件在合并分支时都被修改了则会引起冲突：解决的办法是我们可以修改冲突文件后重新提交！选择要保留他的代码还是你的代码！

master主分支应该非常稳定，用来发布新版本，一般情况下不允许在上面工作，工作一般情况下在新建的dev分支上工作，工作完后，比如上要发布，或者说dev分支代码稳定后可以合并到主分支master上来。

## 10. Git 相关命令合集

![1687685698803](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1687685698803.png)

```
/* 1.0 初始化一个项目 */
git init   # 本地初始化一个git项目
git clone [url]	# 从远程服务器上克隆一个项目

/* 2.0 文件被跟踪 */
git add <name>	# 文件被跟踪
git rm <name>	# 删掉文件使文件不被跟踪
git rm --cache <name>	# 保留在目录里但是不被跟踪
# 对跟踪的文件进行修改，修改完后 利用 git add 文件状态 已修改->缓存
git add <file-name>	# 设置成缓存状态
git reset HEAD <name> # 取消缓存状态 缓存->已修改

/* 3.0 上传本地，提交此次修改 */
git commit    # 会自动进入编辑器
git commit -m 'message'  # 本次提交的大致内容

git reset head~ --soft	# 来取消本次提交（不能撤销第一次提交）
--soft commit 提交的没有了，但文件还是暂存态

/* 4.0 查看文件状态 */
git status		# 查看文件状态
git diff 		# 查看文件哪里被修改了
git log			# 查看历史提交
git log --pretty=oneline 	# 美化输出，一行
git log --pretty=format:"%h- %an,%ar:%s" 	# 自定义格式
# %h 哈希值
# %an 作者名字
# %ar 修订日期（距今）
# %ad 修订日期
# %s 提交说明
git log --graph		# 图形化

/* 4.0 远程仓库 remote */
git remote add origin [url]		# 链接远程的一个仓库
git remote 		# 查看

git push origin master # 将本地的 master 推送到远程

/* 5.0 分支 */
git branch --list	# 查看分支
git branch [feature1]	# 创建分支 feature1
git checkout feature1	# 切换到 feature1
git checkout -b feature2 # 新建并切换到 feature2 分支
/* 5.1 合并分支 */
git merge feature1   # 当前的 master 分支和 feature1 分支合并

/* 5.2 推送分支至远程 */
git push -u origin feature1	# 将 feature1 推送到远程 -u 推送一次后下次默认

/* 6.0 储藏功能 */
git stash	# 切换分支前储藏现有的代码（保存功能）
git stash apply		# 切换分支完后，恢复储藏
git stash list 		# 查看储藏的列表
git stash apply stash@{2}	# 恢复指定的储藏次数
git stash pop 		# 恢复完后直接删掉，相当于弹出
git stash drop stash@{0 }	# 删除指定储藏

/* 7.0 重置 reset */
....
```

## 11. 相关完整笔记

https://www.runoob.com/git/git-install-setup.html
