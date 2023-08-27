# platform 匹配过程

## platform 设备驱动结构体

```c
static const struct i2c_device_id xxx_id[] = {
	{"xxx", 0},
	{}
};

static const struct of_device_id xxx_of_match[] = {
	{.compatible = "xxx"},
	{/* Sentinel */}
};

struct platform_driver {
	.prob = xxx_probe,
	.remove = xxx_remove,
	.driver = {
		.name = "xxx",
		.of_match_table = xxx_of_match,
	},
	.id_table = xxx_id,
};
```

```c
struct platform_device {
	.name = ,
    .dev = ,
	.resource = ,
	.driver_override = ,
}
```

## 2. platform 匹配方法

1. 非他不嫁
2. 设备树匹配方法
3. id_table 匹配方法（常规非设备树）
4. 名字匹配的方法（常规非设备树）

> platform_device.driver_override  <===> platform_driver.driver.name
>
> ------
>
> platform_device.dev.of_node <===> platform_driver.driver.of_match_table
>
> ------
>
> platform_device.name <===> platform_driver.id_table[i].name
>
> ------
>
> platform_device.name <===> platform_driver.driver.name

