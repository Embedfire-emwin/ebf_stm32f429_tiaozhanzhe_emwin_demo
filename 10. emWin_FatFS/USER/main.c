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
#include "./Bsp/touch/gt9xx.h"
#include "./Bsp/usb/usbh_bsp.h"
#include "GUI.h"
#include "DIALOG.h"


extern void SpeedTest(void);
extern WM_HWIN Createfatfs(void);

KEY Key1,Key2;



/*
*********************************************************************************************************
* Description : 初始化BL8782 wifi模块使能引脚，并关闭使能
* Argument(s) : none.
* Return(s)   : none.
*********************************************************************************************************
*/
static void BL8782_PDN_INIT(void)
{
  /*定义一个GPIO_InitTypeDef类型的结构体*/
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOG, ENABLE); 	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
  GPIO_Init(GPIOG, &GPIO_InitStructure);	
  
  GPIO_ResetBits(GPIOG,GPIO_Pin_9);  //关闭BL_8782wifi使能
}
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
//	static uint8_t count=0;
	/*CRC和emWin没有关系，只是他们为了库的保护而做的，这样STemWin的库只能用在ST的芯片上面，别的芯片是无法使用的。 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
	BL8782_PDN_INIT();
  
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
	
	/* 初始化触摸屏 */
	GTP_Init_Panel();	//触摸屏初始化必须优先于系统滴答定时器初始化
	
	/* 初始化系统滴答定时器 */
	SysTick_Init();
	
	TPAD_Init();
	
  /*初始化SDRAM模块*/
  SDRAM_Init();

	/* Init the STemWin GUI Library */
  GUI_Init();
	
	Createfatfs();
	
	/* Init Host Library */
	USBH_Init(&USB_OTG_Core,
			USB_OTG_HS_CORE_ID,
            &USB_Host,
            &USBH_MSC_cb,
            &USR_cb);
	while(1)
	{
		USBH_Process(&USB_OTG_Core, &USB_Host);
		Key_RefreshState(&Key1);//刷新按键状态
		Key_RefreshState(&Key2);//刷新按键状态
		
		if(Key_AccessTimes(&Key1,KEY_ACCESS_READ)!=0)//按键被按下过
		{
			LED2_TOGGLE;			
			Key_AccessTimes(&Key1,KEY_ACCESS_WRITE_CLEAR);
		}
		if(Key_AccessTimes(&Key2,KEY_ACCESS_READ)!=0)//按键被按下过
		{
			LED3_TOGGLE;			
			Key_AccessTimes(&Key2,KEY_ACCESS_WRITE_CLEAR);
		}
		
	  GUI_Delay(10);
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
