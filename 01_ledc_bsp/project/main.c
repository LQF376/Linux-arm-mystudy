#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_led.h"

/*
 * @description	: mian函数
 * @param 		: 无
 * @return 		: 无
 */
int main(void)
{
	clk_enable();		/* 使能所有的时钟 			*/
	led_init();			/* 初始化led 			*/

	while(1)			
	{	
		/* 打开LED0 */
		led_switch(LED0,ON);		
		delay(500);

		/* 关闭LED0 */
		led_switch(LED0,OFF);	
		delay(500);
	}

	return 0;
}
