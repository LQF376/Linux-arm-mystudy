#ifndef _BSP_KEY_H
#define _BSP_KEY_H
#include "imx6ul.h"
/*
按键驱动头文件
*/


/* 定义按键值 */
enum keyvalue{
	KEY_NONE   = 0,
	KEY0_VALUE,
	KEY1_VALUE,
	KEY2_VALUE,
};

/* 函数声明 */
void key_init(void);
int key_getvalue(void);


#endif
