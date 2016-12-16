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
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
#include "includes.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/* --------------- APPLICATION GLOBALS ---------------- */
static  OS_TCB       AppTaskStartTCB;
static  CPU_STK      AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];


static  OS_TCB       AppTaskGUITCB;
__align(8) static  CPU_STK      AppTaskGUIStk[APP_CFG_TASK_GUI_STK_SIZE];

static OS_TCB        AppTaskPICSHOTTCB;
__align(8) static CPU_STK       AppTaskPICSHOTStk[APP_TASK_PICSHOT_STK_SIZE];
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void  AppTaskStart   (void  *p_arg);
static  void  AppTaskCreate  (void);
static  void  AppObjCreate   (void);


static  void  AppTaskGUI     (void  *p_arg);
static  void  AppTaskPICSHOT    (void  *p_arg);

uint8_t shot_flag=0;
KEY Key1,Key2;
FIL     f_file;
FRESULT f_result; 
UINT    f_num;
uint8_t   tpad_flag=0;
OS_SEM   SEM_SHOT;
static uint8_t Pic_Name=0;
/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*
*********************************************************************************************************
*/

int main(void)
{
	OS_ERR   err;
#if (CPU_CFG_NAME_EN == DEF_ENABLED)
	CPU_ERR  cpu_err;
#endif                 

	Mem_Init();                                                 /* Initialize Memory Managment Module                   */
	Math_Init();                                                /* Initialize Mathematical Module                       */

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
	CPU_NameSet((CPU_CHAR *)"STM32F429II",
							(CPU_ERR  *)&cpu_err);
#endif

	BSP_IntDisAll();                                            /* Disable all Interrupts.                              */

	OSInit(&err);                                               /* Init uC/OS-III.                                      */

	App_OS_SetAllHooks();

	OSTaskCreate(&AppTaskStartTCB,                              /* Create the start task                                */
								"App Task Start",
								AppTaskStart,
								0u,
								APP_CFG_TASK_START_PRIO,
								&AppTaskStartStk[0u],
								AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE / 10u],
								APP_CFG_TASK_START_STK_SIZE,
								0u,
								0u,
								0u,
							 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
							 &err);
	OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */
	
	while (DEF_ON) {                                            /* Should Never Get Here.                               */
			;
	}
}

//按键定时扫描的定时器
void inputscan(void)
{
  OS_ERR  err;
  
  OSSchedLock(&err);
	GT9xx_GetOnePiont();//触摸屏定时扫描
  OSSchedUnlock(&err); 
  
	Key_RefreshState(&Key1);//刷新按键状态
	Key_RefreshState(&Key2);//刷新按键状态
	if(Key_AccessTimes(&Key1,KEY_ACCESS_READ)!=0)//按键被按下过
	{
		LED2_TOGGLE;
    OSSemPost ((OS_SEM  *)&SEM_SHOT,
               (OS_OPT   )OS_OPT_POST_ALL,
               (OS_ERR  *)&err);
		Key_AccessTimes(&Key1,KEY_ACCESS_WRITE_CLEAR);
	}
	if(Key_AccessTimes(&Key2,KEY_ACCESS_READ)!=0)//按键被按下过
	{
		LED3_TOGGLE;
		Key_AccessTimes(&Key2,KEY_ACCESS_WRITE_CLEAR);
	}
}

/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{

	OS_ERR      err;
 (void)p_arg;
	CPU_Init();                                                 /* Initialize the uC/CPU services                       */
 
	BSP_Init();                                                 /* Initialize BSP functions                             */

#if OS_CFG_STAT_TASK_EN > 0u
	OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
	CPU_IntDisMeasMaxCurReset();
#endif

#if (APP_CFG_SERIAL_EN == DEF_ENABLED)
	APP_TRACE_DBG(("Creating Application kernel objects\n\r"));
#endif	
	AppObjCreate(); 
/* Create Applicaiton kernel objects                    */
#if (APP_CFG_SERIAL_EN == DEF_ENABLED)
	APP_TRACE_DBG(("Creating Application Tasks\n\r"));
#endif	
	AppTaskCreate();                                            /* Create Application tasks                             */

	
	while (DEF_TRUE)
	{                                          /* Task body, always written as an infinite loop.       */
		OSTimeDlyHMSM(0u, 0u, 10u, 0u,
									OS_OPT_TIME_HMSM_STRICT,
									&err);
	}
}

/*
*********************************************************************************************************
*                                          AppTaskCreate()
*
* Description : Create Application Tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
	OS_ERR  os_err;

	/* ---------- CREATE KERNEL OBJECTS TEST TASK --------- */
  OSTaskCreate((OS_TCB       *)&AppTaskGUITCB,
               (CPU_CHAR     *)"Kernel Objects Task 0",
               (OS_TASK_PTR   )AppTaskGUI,
               (void         *)0,
               (OS_PRIO       )APP_CFG_TASK_GUI_PRIO,
               (CPU_STK      *)&AppTaskGUIStk[0],
               (CPU_STK_SIZE  )AppTaskGUIStk[APP_CFG_TASK_GUI_STK_SIZE / 10u],
               (CPU_STK_SIZE  )APP_CFG_TASK_GUI_STK_SIZE,
               (OS_MSG_QTY    )0u,
               (OS_TICK       )0u,
               (void         *)0,
               (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               (OS_ERR       *)&os_err);
  OSTaskCreate((OS_TCB       *)&AppTaskPICSHOTTCB,
               (CPU_CHAR     *)"Picture shot Task",
               (OS_TASK_PTR   )AppTaskPICSHOT,
               (void         *)0,
               (OS_PRIO       )APP_TASK_PICSHOT_PRIO,
               (CPU_STK      *)&AppTaskPICSHOTStk[0],
               (CPU_STK_SIZE  )AppTaskPICSHOTStk[APP_TASK_PICSHOT_STK_SIZE / 10u],
               (CPU_STK_SIZE  )APP_TASK_PICSHOT_STK_SIZE,
               (OS_MSG_QTY    )0u,
               (OS_TICK       )0u,
               (void         *)0,
               (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               (OS_ERR       *)&os_err);
}


/*
*********************************************************************************************************
*                                          AppObjCreate()
*
* Description : Create Application Kernel Objects.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  AppObjCreate (void)
{
  OS_ERR      err;
  KeyCreate(&Key1,GetPinStateOfKey1);
	KeyCreate(&Key2,GetPinStateOfKey2);
   //创建一个信号量，按键按下的时候post一个信号量给其他任务
	OSSemCreate ((OS_SEM *)&SEM_SHOT,     //指向信号量变量的指针
			   (CPU_CHAR    *)"SEM_SHOT",    //信号量的名字
			   (OS_SEM_CTR   )0,             //信号量这里是指示事件发生，所以赋值为0，表示事件还没有发生
			   (OS_ERR      *)&err);         //错误类型
}

/*
*********************************************************************************************************
*	函 数 名: _WriteByte2File()
*	功能说明: 
*	形    参：  	
*	返 回 值: 
*********************************************************************************************************
*/
static void _WriteByte2File(uint8_t Data, void * p) 
{
	f_result = f_write(p, &Data, 1, &f_num);
}

static  void  AppTaskPICSHOT    (void  *p_arg)
{
  OS_ERR  err;
	CPU_TS 	Ts;
	uint8_t i;
	char 	buf[20];
	(void)p_arg;
	while(1)
	{
		//阻塞任务直到按键被按下！
		OSSemPend((OS_SEM   *)&SEM_SHOT,
						 (OS_TICK   )0,                     //如果这个参数设置为0就一直等待下去
						 (OS_OPT    )OS_OPT_PEND_BLOCKING,  //如果没有信号量可用就等待
						 (CPU_TS   *)&Ts,             //这个参数是指向存放信号量被提交、强制解除等待状态、或者信号量被删除的时间戳        
						 (OS_ERR   *)&err);
		i=0;
		while(i<0xff)
		{
			sprintf((char *)buf,"0:/picture%d.bmp",i);
			f_result=f_open(&f_file,(const TCHAR*)buf,FA_READ);
			if(f_result==FR_NO_FILE)break;	
			else
				f_close(&f_file);
			i++;
		}
		printf("start shot picture\n\r");
		/* 创建截图 */
		f_result = f_open(&f_file,buf, FA_WRITE|FA_CREATE_ALWAYS);
    printf("shot file:%s open result:%d\n",buf,f_result);
#if LCD_NUM_LAYERS == 2
    GUI_SelectLayer(1);
#endif 
		OSSchedLock(&err);
		/* 向SD卡绘制BMP图片 */
		GUI_BMP_Serialize(_WriteByte2File,&f_file);
		OSSchedUnlock(&err);
		/* 创建完成后关闭file */
		f_result = f_close(&f_file);
		printf("shot picture success\n\r");
		Pic_Name++;
    shot_flag=1;
	}  
}


extern void Camera(void);
/*
*********************************************************************************************************
*                                          AppTaskObj1()
*
* Description : Test uC/OS-III objects.
*
* Argument(s) : p_arg is the argument passed to 'AppTaskObj1' by 'OSTaskCreate()'.
*
* Return(s)   : none
*
* Caller(s)   : This is a task
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  AppTaskGUI (void  *p_arg)
{ 
  while(1)
  {
    Camera();    
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
