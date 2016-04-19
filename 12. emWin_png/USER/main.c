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
#include "./drivers/fatfs_flash_spi.h"

extern void drawpng(void);
extern void drawbmp(void);
extern void drawJPG(void);

FATFS   fs[3];													  /* Work area (file system object) for logical drives */
FIL     xbffile;

KEY  Key1,Key2;
GUI_XBF_DATA XBF_Data;
GUI_FONT     XBF_Font;

// USER START (Optionally insert additional static code)
#if 0
/*
*********************************************************************************************************
*
*       _cbGetData
*
* Function description
*   Callback function for getting font data
*
* Parameters:
*   Off      - Position of XBF file to be read
*   NumBytes - Number of requested bytes
*   pVoid    - Application defined pointer
*   pBuffer  - Pointer to buffer to be filled by the function
*
* Return value:
*   0 on success, 1 on error
*********************************************************************************************************
*/
static int _cbGetData(U32 Offset, U16 NumBytes, void * pVoid, void * pBuffer)
{
	FRESULT result;
	FIL *FontFile;

	/* The pVoid pointer may be used to get a file handle */
	FontFile = (FIL *)pVoid;

	/*
	* Set file pointer to the required position
	*/
	result =f_lseek(FontFile, Offset);
	if (result != FR_OK) 
	{
		return 1; /* Error */
	}

	/*
	* Read data into buffer
	*/
	result = f_read(FontFile, pBuffer, NumBytes,0);
	if(result!=FR_OK)
	{
		return 1; /* Error */
	}
	return 0;

}

/*
*********************************************************************************************************
*	? ? ?: _ShowXBF
*	????: ??XBF??
*	?    ?:?
*	? ? ?: ?
*********************************************************************************************************
*/
static void _ShowXBF(void) 
{
	FRESULT result; 
	result = f_open(&xbffile, "0:xinsongti25.xbf", FA_OPEN_EXISTING | FA_READ | FA_OPEN_ALWAYS);
	if (result != FR_OK)
	{
		return;

	}
	//
	// Create XBF font
	//
	GUI_XBF_CreateFont(&XBF_Font,       // Pointer to GUI_FONT structure in RAM
					 &XBF_Data,         // Pointer to GUI_XBF_DATA structure in RAM
					 GUI_XBF_TYPE_PROP_EXT, 		// Font type to be created
					 _cbGetData,        // Pointer to callback function
					 &xbffile);        // Pointer to be passed to GetData function
}

#else
//
/*
*********************************************************************************************************
*
*       _cbGetData
*
* Function description
*   Callback function for getting font data
*
* Parameters:
*   Off      - Position of XBF file to be read
*   NumBytes - Number of requested bytes
*   pVoid    - Application defined pointer
*   pBuffer  - Pointer to buffer to be filled by the function
*
* Return value:
*   0 on success, 1 on error
*********************************************************************************************************
*/
static int _cbGetData(U32 Offset, U16 NumBytes, void * pVoid, void * pBuffer)
{
	SPI_FLASH_BufferRead(pBuffer,60*4096+Offset,NumBytes);
	return 0;
}

static void _ShowXBF(void) 
{
	//
	// Create XBF font
	//
	GUI_XBF_CreateFont(&XBF_Font,       // Pointer to GUI_FONT structure in RAM
					   &XBF_Data,         // Pointer to GUI_XBF_DATA structure in RAM
					   GUI_XBF_TYPE_PROP_EXT, 		// Font type to be created
					   _cbGetData,        // Pointer to callback function
					   0);        // Pointer to be passed to GetData function
}
#endif
// USER END

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
	FRESULT result; 

	/*CRC和emWin没有关系，只是他们为了库的保护而做的，这样STemWin的库只能用在ST的芯片上面，别的芯片是无法使用的。 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
  /*初始化SDRAM模块*/
  SDRAM_Init();
	
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
	
	/* 挂载文件系统 */
  result = f_mount(&fs[0],"0:",1);
	result = f_mount(&fs[1],"1:",1);	/* Mount a logical drive */
	/* Init the STemWin GUI Library */
  GUI_Init();
	
	
	if(result==FR_OK)
	{
		_ShowXBF();
		GUI_UC_SetEncodeUTF8();
		GUI_SetDefaultFont(&XBF_Font);
	}
	GUI_SetBkColor(GUI_WHITE);
  GUI_Clear(); 
  GUI_SetFont(GUI_FONT_32B_ASCII);
  GUI_SetColor(GUI_BLUE);
  GUI_DispStringAt("KEY1:show png file",100,100);
  GUI_DispStringAt("KEY2:show bmp file",100,140);
	while(1)
	{
		Key_RefreshState(&Key1);//刷新按键状态
		Key_RefreshState(&Key2);//刷新按键状态
		
		if(Key_AccessTimes(&Key1,KEY_ACCESS_READ)!=0)//按键被按下过
		{
			LED2_TOGGLE;
      drawpng();
			Key_AccessTimes(&Key1,KEY_ACCESS_WRITE_CLEAR);
		}
		if(Key_AccessTimes(&Key2,KEY_ACCESS_READ)!=0)//按键被按下过
		{
			LED3_TOGGLE;	   
      drawbmp();   //  drawJPG();//  
			Key_AccessTimes(&Key2,KEY_ACCESS_WRITE_CLEAR);
		}		
	  GUI_Delay(10);
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
