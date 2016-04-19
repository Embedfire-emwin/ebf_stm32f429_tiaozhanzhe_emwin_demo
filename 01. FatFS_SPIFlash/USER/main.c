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
/**
  ******************************************************************************
  *                              头文件
  ******************************************************************************
  */
#include "stm32f4xx.h"
#include "./Bsp/led/bsp_led.h" 
#include "./Bsp/usart/bsp_debug_usart.h"
#include "ff.h"
/**
  ******************************************************************************
  *                              定义变量
  ******************************************************************************
  */
FIL fnew;													/* file objects */
FATFS fs;													/* Work area (file system object) for logical drives */
FRESULT result; 
UINT br, bw;            					/* File R/W count */
BYTE buffer[1024]={0};       		  /* file copy buffer */
BYTE textFileBuffer[] = "欢迎使用野火STM32-V3 ISO开发板  今天是个好日子，新建文件系统测试文件\r\n";

/**
  ******************************************************************************
  *                                任务函数
  ******************************************************************************
  */
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{    
	/* 初始化LED */
	LED_GPIO_Config();
	
	/* 初始化调试串口，一般为串口1 */
	Debug_USART_Config();
	
  printf("\r\n this is a fatfs test demo \r\n");

/*************************   spi flash 文件系统   *********************************************/
	//在外部Flash挂载文件系统
	result = f_mount(&fs,"1:",1);
	printf("\r\n f_mount result=%d \r\n",result);
	
	//如果没有文件系统就格式化创建创建文件系统
	if(result == FR_NO_FILESYSTEM)
	{
		result=f_mkfs("1:",0,4096);							//格式化
		printf("\r\nmkfs result=%d",result);
		result = f_mount(&fs,"1:",0);						//格式化后，先取消挂载
		printf("\r\n f_mount result=%d \r\n",result);
		result = f_mount(&fs,"1:",1);						//重新挂载
		printf("\r\n f_mount result=%d \r\n",result);
	}
/**************************  flash   *****************************************/		
	//文件系统测试，写测试
	//打开文件，如果文件不参加则创建它
	result = f_open(&fnew, "1:flashnewfile.txt", FA_CREATE_ALWAYS | FA_WRITE );
	printf("\r\f_open result=%d",result);
	if ( result == FR_OK )
	{
		result = f_write(&fnew, textFileBuffer, sizeof(textFileBuffer), &bw);
		printf("\r\f_write result=%d",result);
		result = f_lseek(&fnew,sizeof(textFileBuffer)*50);
		result = f_write(&fnew, textFileBuffer, sizeof(textFileBuffer), &bw);
		printf("\r\f_write result=%d",result);
		f_close(&fnew);      
	}
	//读测试
	result = f_open(&fnew, "1:flashnewfile.txt", FA_OPEN_EXISTING | FA_READ); 	 
	printf("\r\f_open result=%d",result);
	result = f_read(&fnew, buffer, sizeof(buffer), &br); 
	printf("\r\f_read result=%d",result);
	printf("\r\n %s ", buffer);

	/* Close open files */
	result = f_close(&fnew);	
  
	//不再使用文件系统，取消挂载文件系统
	result = f_mount(NULL,"1:",1);
	while(1)
	{
	}

}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
