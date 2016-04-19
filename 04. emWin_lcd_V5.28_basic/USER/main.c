/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   用1.5.1版本库建的工程模板
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火  STM32 F429 开发板 
  * 论坛    :http://www.chuxue123.com
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
#include "stm32f4xx.h"
#include "./Bsp/led/bsp_led.h" 
#include "./Bsp/beep/bsp_beep.h" 
#include "./Bsp/usart/bsp_debug_usart.h"
#include "./Bsp/systick/bsp_SysTick.h"
#include "./Bsp/TouchPad/bsp_touchpad.h"
#include "./Bsp/key/bsp_key.h"
#include "./Bsp/sdram/bsp_sdram.h"
#include "./Bsp/lcd/bsp_lcd.h"

#include "GUI.h"
#include "DIALOG.h"


KEY Key1,Key2;
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
	static uint8_t count=0;
	/*CRC和emWin没有关系，只是他们为了库的保护而做的，这样STemWin的库只能用在ST的芯片上面，别的芯片是无法使用的。 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);  
	
	/* 初始化LED */
	LED_GPIO_Config();
	/* 初始化蜂鸣器 */
	Beep_GPIO_Config();
	Key1_GPIO_Config();
	Key2_GPIO_Config();
	KeyCreate(&Key1,GetPinStateOfKey1);
	KeyCreate(&Key2,GetPinStateOfKey2);
	/* 初始化调试串口，一般为串口1 */
	Debug_USART_Config();
	/* 初始化系统滴答定时器 */
	SysTick_Init();
	printf("TouchPAD Demo\n");
	
	TPAD_Init();
	
  /*初始化SDRAM模块*/
  SDRAM_Init();

	/* Init the STemWin GUI Library */
  GUI_Init();
	
  /* Setup layer configuration during startup */
  GUI_SetBkColor(GUI_TRANSPARENT);
  GUI_SelectLayer(1);
  GUI_Clear();
  GUI_SetBkColor(GUI_TRANSPARENT); 
  GUI_SelectLayer(0);
	
	
	GUI_SelectLayer(1);
	GUI_SetColor(GUI_YELLOW);
	GUI_FillRect(50,300, 750, 450);
	GUI_FillRect(150,100, 250, 200);	
	
	GUI_SelectLayer(0);
	GUI_SetColor(GUI_RED);
	GUI_FillRect(50,60, 750, 250);
	GUI_SetFont(GUI_FONT_32B_ASCII);
	GUI_DispStringAt("Hello World",10,10);
	GUI_SetFont(GUI_FONT_13B_ASCII);
	GUI_DispStringAt("Hello World",200,10);
	
	while(1)
	{
		Key_RefreshState(&Key1);//刷新按键状态
		Key_RefreshState(&Key2);//刷新按键状态
		if(Key_AccessTimes(&Key1,KEY_ACCESS_READ)!=0)//按键被按下过
		{
			LED2_TOGGLE;
			Key_AccessTimes(&Key1,KEY_ACCESS_WRITE_CLEAR);
		}
		if(Key_AccessTimes(&Key2,KEY_ACCESS_READ)!=0)//按键被按下过
		{
			LED1_TOGGLE;
			Key_AccessTimes(&Key2,KEY_ACCESS_WRITE_CLEAR);
		}
		if(TPAD_Scan(0))
		{
			BEEP_ON;
			GUI_Delay(100);
		}
		if(count==100)
		{
			LED3_ON;
		}else if(count==110)
		{
			count=0;
			LED3_OFF;
		}
		count++;
		BEEP_OFF;			
		GUI_Delay(10);
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
