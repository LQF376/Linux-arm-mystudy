/*
本实验主要学习如何配置I.MX6U的系统时钟，I.MX6UL内部boot
代码默认将主频设置为396M，但是I.MX6U标准主频为528MHz(最高
可到696MHz)，本实验就讲解如何将I.MX6U的主频设置到528MHz 
*/

#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_led.h"
#include "bsp_beep.h"
#include "bsp_key.h"
#include "bsp_int.h"
#include "bsp_exit.h"

/*
 * @description	: main函数
 * @param 		: 无
 * @return 		: 无
 */
int main(void)
{
	unsigned char state = OFF;

	int_init(); 		/* 初始化中断(一定要最先调用！) */
	imx6u_clkinit();	/* 初始化系统时钟 			*/
	clk_enable();		/* 使能所有的时钟 			*/
	led_init();			/* 初始化led 			*/
	beep_init();		/* 初始化beep	 		*/
	key_init();			/* 初始化key 			*/
	exit_init();		/* 初始化按键中断			*/

	while(1)			
	{	
		state = !state;
		led_switch(LED0, state);
		delay(500);
	}

	return 0;
}
