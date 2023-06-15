#ifndef _BSP_INT_H
#define _BSP_INT_H
#include "imx6ul.h"

/*
中断驱动头文件
*/

/* 中断服务函数形式 */ 
typedef void (*system_irq_handler_t) (unsigned int giccIar, void *param);

 
/* 中断服务函数结构体*/
typedef struct _sys_irq_handle
{
    system_irq_handler_t irqHandler; /* 中断服务函数 */
    void *userParam;                 /* 中断服务函数参数 */
} sys_irq_handle_t;


/* 函数声明 */
void int_init(void);
void system_irqtable_init(void);
void system_register_irqhandler(IRQn_Type irq, system_irq_handler_t handler, void *userParam);
void system_irqhandler(unsigned int giccIar); 
void default_irqhandler(unsigned int giccIar, void *userParam); 



#endif
