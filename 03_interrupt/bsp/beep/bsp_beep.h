#ifndef __BSP_BEEP_H
#define __BSP_BEEP_H

/*  蜂鸣器驱动头文件 */

#include "imx6ul.h"


/* 函数声明 */
void beep_init(void);
void beep_switch(int status);
#endif

