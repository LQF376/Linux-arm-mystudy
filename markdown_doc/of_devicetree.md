 

## 设备树常用 OF 操作函数

用 device_node 表示设备节点，用 of 函数找到对应节点，然后读出该节点对应的属性值即可，property 表示属性值；resource 表示内存空间类型

#### 1. 查找节点的 OF 函数（用于从设备树中到到对应节点 device_node）

**linux 内核使用 device_node 结构体来描述一个节点（include/linux/of.h）**

```c
struct device_node {
	const char *name; 				/* 节点名字 */
	const char *type; 				/* 设备类型 */
	phandle phandle;
	const char *full_name; 			/* 节点全名 */
	struct fwnode_handle fwnode;
	struct property *properties; 	/* 属性  重点 */
	struct property *deadprops; 	/* removed 属性 */
	struct device_node *parent; 	/* 父节点 */
	struct device_node *child; 		/* 子节点 */
	struct device_node *sibling;
	struct kobject kobj;
	unsigned long _flags;
	void *data;
#if defined(CONFIG_SPARC)
	const char *path_component_name;
	unsigned int unique_id;
	struct of_irq_controller *irq_trans;
#endif
};
```

**of_find_node_by_name**

```c
struct device_node *of_find_node_by_name(struct device_node *from,
							const char *name);
from: 开始查找的节点，NULL表示从根节点开始查找
name：要查找的节点名字
返回值：找到的节点，NULL表示查找失败
```

**of_find_node_by_type 通过 device_type 属性查找指定节点**

```c
struct device_node *of_find_node_by_type(struct device_node *from, const char *type)
from：开始查找的节点
type：要查找的节点对应的type字符串
返回值：找到的节点，NULL表示查找失败
```

**of_find_compatible_node 根据 device_type 和 compatible 两个属性来查找节点**

```c
struct device_node *of_find_compatible_node(struct device_node *from,
										const char *type,
										const char *compatible)
from：开始查找的节点
type：要查找的节点对应的 type 字符串，可以为NULL
compatibel：要查找的节点的对应的 compatible 属性列表
返回值：找到的节点，NULL表示查找失败
```

**of_find_matching_node_and_match 通过 of_device_id 匹配表来查找指定的节点 **

```c
struct device_node *of_find_matching_node_and_match(struct device_node *from,
										const struct of_device_id *matches,
										const struct of_device_id **match)
from：开始查找的节点
matches：of_device_id 匹配表，在匹配表里面查找节点
match：找到的匹配的 of_device_id
返回值：找到的节点，NULL 表示查找失败
```

**of_find_node_by_path 根据路径来查找指定的节点**

```c
inline struct device_node *of_find_node_by_path(const char *path)
path:带有全路径的节点名
返回值：找到的节点，NULL 表示查找失败
```

#### 2. 查找父子结点的 OF 函数

**of_get_parent 获取指定节点的父节点**

```c
struct device_node *of_get_parent(const struct device_node *node)
node：要查找父节点的节点
返回值：找到的父节点
```

**of_get_next_child 迭代方式查找子节点**

```c
struct device_node *of_get_next_child(const struct device_node *node,
									struct device_node *prev)
node：父节点
prev：前一个子节点，NULL 表示从第一个子节点开始
返回值：找到的下一个子节点
```

#### 3. 提取属性值的 OF 函数

Linux 内核使用 property 结构体来表示属性（include/linux/of.h）

```c
struct property {
	char *name;			 /* 属性名字 */
	int length; 			/* 属性长度 */
	void *value; 			/* 属性值 */
	struct property *next; 	/* 下一个属性 */
	unsigned long _flags;
	unsigned int unique_id;
	struct bin_attribute attr;
};
```

**1. of_find_property 查找指定的属性**

```c
property *of_find_property(const struct device_node *np,
						const char *name,
						int *lenp)
np：node 节点
name：属性名字
lenp：属性值的字节数
返回值：找到的属性
```

**2. of_property_count_elems_of_size 获取属性中元素的数量 **

```c
int of_property_count_elems_of_size(const struct device_node *np,
								const char *propname,
								int elem_size);
np：设备节点
proname：需要统计的元素数量的属性名字
elem_size：元素长度
返回值：得到属性元素数量
```

**3. of_propety_read_u32_index 用于获取指定标号的 u32 类型数据值**

```c
int of_property_read_u32_index(const struct device_node *np,
						const char *propname,
						u32 index,
						u32 *out_value)
np：设备节点
propname：属性名字
index：要读取的值标号
out_value：读取到的值
返回值： 0 读取成功； 负值 读取失败； EINVAL 表示属性不存在；ENODATA 表示没有要读取的数据；EOVERFLOW 表示属性值列表太小
```

**4. of_property_read_uX_array 函数**

```c
int of_property_read_u8_array(const struct device_node *np,
								const char *propname,
								u8 *out_values,
								size_t sz)
int of_property_read_u16_array(const struct device_node *np,
								const char *propname,
								u16 *out_values,
								size_t sz)
int of_property_read_u32_array(const struct device_node *np,
								const char *propname,
								u32 *out_values,
								size_t sz)
int of_property_read_u64_array(const struct device_node *np,
								const char *propname,
								u64 *out_values,
								size_t sz)
np：设备节点
property：要读取的属性名字
out_value：读取到的数组值，u8 u16 u32 u64
返回值：0 读取成功； 负值 读取失败； EINVAL 表示属性不存在；ENODATA 表示没有要读取的数据；EOVERFLOW 表示属性值列表太小
```

**5. of_property_read_uX 函数**

```c
int of_property_read_u8(const struct device_node *np,
						const char *propname,
						u8 *out_value)
int of_property_read_u16(const struct device_node *np,
						const char *propname,
						u16 *out_value)
int of_property_read_u32(const struct device_node *np,
						const char *propname,
						u32 *out_value)
int of_property_read_u64(const struct device_node *np,
						const char *propname,
						u64 *out_value)
np：设备节点
proname：读取的属性名字
out_value：读取到的数组值
返回值：0 读取成功； 负值 读取失败； EINVAL 表示属性不存在；ENODATA 表示没有要读取的数据；EOVERFLOW 表示属性值列表太小
```

**6. of_property_read_string 读取属性中字符串值 **

```c
int of_property_read_string(struct device_node *np,
						const char *propname,
						const char **out_string)
np：设备节点
propname：读取的属性名字
out_string：读到的字符串值
返回值：0，读取成功，负值，读取失败
```

**7. of_n_addr_cells  获取 #address-cells 属性值 **

```c
int of_n_addr_cells(struct device_node *np)
np：设备节点
返回值：获取到的#address-cells 属性值
```

**8. of_n_size_cells 函数 获取 #size-cell 属性值 **

```c
int of_n_size_cells(struct device_node *np)
np：设备节点
返回值：获取到的#size-cells 属性值
```

#### 4. 其他常用 OF 函数

**1. of_device_is_compatible 函数 查看节点的 compatible 属性是否有包含 compat 指定的字符串，也就是检查设备节点的兼容性**

```c
int of_device_is_compatible(const struct device_node *device,
						const char *compat)
device：设备节点
compat：要查看的字符串
返回值： 0，节点的 compatible 属性中不包含 compat 指定的字符串；正数，节点的 compatible 属性中包含 compat 指定的字符串
```

**2. of_get_address  获取地址相关属性，主要是“reg”或者“assigned-addresses”属性值 **

```c
const __be32 *of_get_address(struct device_node *dev,
							int index,
							u64 *size,
							unsigned int *flags)
dev：设备节点
index：读取的地址标号
size：地址长度
flags：参数，IORESOURCE_IO、 IORESOURCE_MEM 等
返回值：读取到的地址数据首地址，为 NULL 的话表示读取失败
```

**3. of_translate_address 将从设备树读取到的地址转换为物理地址**

```c
u64 of_translate_address(struct device_node *dev,
						const __be32 *in_addr)
dev：设备节点
in_addr：要转换的地址
返回值：得到的物理地址，如果为 OF_BAD_ADDR 的话表示转换失败
```

**4. of_address_to_resource  **

Linux 内核使用 resource 结构体来描述一段内存空间（include/linux/ioport.h）；对于32 位的 SOC 来说，resource_size_t 是 u32 类型的。 

```c
struct resource {
	resource_size_t start;	// 开始地址 u32类型
	resource_size_t end;	// 结束地址
	const char *name;		// 资源名字
	unsigned long flags;	// 资源标志位，表资源类型 include/linux/ioport.h
	struct resource *parent, *sibling, *child;
};

/* 常用资源类型 */
#define IORESOURCE_MEM 0x00000200
#define IORESOURCE_REG 0x00000300
#define IORESOURCE_IRQ 0x00000400
```

```c
int of_address_to_resource(struct device_node *dev,
						int index,
						struct resource *r)
dev：设备节点
index：地址资源标号
r：得到的 resource 类型的资源值
返回值：0，成功；负值，失败
```

**5. of_iomap 直接内存映射，直接通过 of_iomap 函数来获取内存地址所对应的虚拟地址（不需要使用 ioremap 函数来完成物理地址到虚拟地址的映射 ） **

```c
void __iomem *of_iomap(struct device_node *np,
					int index)
np：设备节点
index：reg 属性中要完成内存映射的段，如果 reg 属性只有一段的话 index 就设置为 0
返回值：经过内存映射后的虚拟内存首地址，如果为 NULL 的话表示内存映射失败
```

