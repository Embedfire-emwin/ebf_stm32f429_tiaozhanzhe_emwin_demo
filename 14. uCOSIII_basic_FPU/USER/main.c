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
/*
*********************************************************************************************************
* 为让例程支持浮点运算，需要修改启动文件(startup_stm32f429_439xx.s)，
* 同时需要在工程选项硬件浮点功能选择“USB Single Precision”
*********************************************************************************************************

; Reset handler
Reset_Handler    PROC
                 EXPORT  Reset_Handler             [WEAK]
				 IMPORT  SystemInit
				 IMPORT  __main
                 
				 IF {FPU} != "SoftVFP"
                                                ; Enable Floating Point Support at reset for FPU
                 LDR.W   R0, =0xE000ED88         ; Load address of CPACR register
                 LDR     R1, [R0]                ; Read value at CPACR
                 ORR     R1,  R1, #(0xF <<20)    ; Set bits 20-23 to enable CP10 and CP11 coprocessors
                                                ; Write back the modified CPACR value
                 STR     R1, [R0]                ; Wait for store to complete
                 DSB
                
                                                ; Disable automatic FP register content
                                                ; Disable lazy context switch
                 LDR.W   R0, =0xE000EF34         ; Load address to FPCCR register
                 LDR     R1, [R0]
                 AND     R1,  R1, #(0x3FFFFFFF)  ; Clear the LSPEN and ASPEN bits
                 STR     R1, [R0]
                 ISB                             ; Reset pipeline now the FPU is enabled
                 ENDIF
					
                 LDR     R0, =SystemInit
                 BLX     R0
                 LDR     R0, =__main
                 BX      R0
                 ENDP
*********************************************************************************************************
*/
#include "includes.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
#define  APP_TASK_EQ_0_ITERATION_NBR              16u
#define  APP_TASK_EQ_1_ITERATION_NBR              18u

/* --------------- APPLICATION GLOBALS ---------------- */
static  OS_TCB       AppTaskStartTCB;
static  CPU_STK      AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB       AppTaskObj0TCB;
static  CPU_STK      AppTaskObj0Stk[APP_CFG_TASK_OBJ_STK_SIZE];

static  OS_TCB       AppTaskObj1TCB;
static  CPU_STK      AppTaskObj1Stk[APP_CFG_TASK_OBJ_STK_SIZE];

static  OS_TCB       AppTaskEq0FpTCB;
static  CPU_STK      AppTaskEq0FpStk[APP_CFG_TASK_EQ_STK_SIZE];

static  OS_TCB       AppTaskEq1FpTCB;
static  CPU_STK      AppTaskEq1FpStk[APP_CFG_TASK_EQ_STK_SIZE];

#if (OS_CFG_SEM_EN > 0u)
static  OS_SEM       AppTraceSem;
#endif

#if (OS_CFG_SEM_EN > 0u)
static  OS_SEM       AppTaskObjSem;
#endif

#if (OS_CFG_MUTEX_EN > 0u)
static  OS_MUTEX     AppTaskObjMutex;
#endif

#if (OS_CFG_Q_EN > 0u)
static  OS_Q         AppTaskObjQ;
#endif

#if (OS_CFG_FLAG_EN > 0u)
static  OS_FLAG_GRP  AppTaskObjFlag;
#endif

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void  AppTaskStart (void  *p_arg);
static  void  AppTaskCreate(void);
static  void  AppObjCreate (void);

static  void  AppTaskObj0    (void  *p_arg);
static  void  AppTaskObj1    (void  *p_arg);
static  void  AppTaskEq0Fp  (void  *p_arg);             /* Floating Point Equation 0 task. */
static  void  AppTaskEq1Fp   (void  *p_arg);                   /* Floating Point Equation 1 task.  */

KEY Key1,Key2;

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

    BSP_Init();                                                 /* Initialize BSP functions                             */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

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
        LED3_TOGGLE;
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
				if(TPAD_Scan(0))
				{
					BEEP_ON;
					OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
				}
				BEEP_OFF;
        OSTimeDlyHMSM(0u, 0u, 1u, 0u,
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
    OSTaskCreate(&AppTaskObj0TCB,
                 "Kernel Objects Task 0",
                  AppTaskObj0,
                  0,
                  APP_CFG_TASK_OBJ_PRIO,
                 &AppTaskObj0Stk[0],
                  AppTaskObj0Stk[APP_CFG_TASK_OBJ_STK_SIZE / 10u],
                  APP_CFG_TASK_OBJ_STK_SIZE,
                  0u,
                  0u,
                  0,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &os_err);
	 OSTaskCreate(&AppTaskObj1TCB,
                 "Kernel Objects Task 1",
                  AppTaskObj1,
                  0,
                  APP_CFG_TASK_OBJ_PRIO,
                 &AppTaskObj1Stk[0],
                  AppTaskObj1Stk[APP_CFG_TASK_OBJ_STK_SIZE / 10u],
                  APP_CFG_TASK_OBJ_STK_SIZE,
                  0u,
                  0u,
                  0,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &os_err);
		/* ------------- CREATE FLOATING POINT TASK ----------- */
    OSTaskCreate((OS_TCB      *) &AppTaskEq0FpTCB,
                 (CPU_CHAR    *) "FP  Equation 1",
                 (OS_TASK_PTR  ) AppTaskEq0Fp, 
                 (void        *) 0,
                 (OS_PRIO      ) APP_CFG_TASK_EQ_STK_SIZE,
                 (CPU_STK     *) &AppTaskEq0FpStk[0],
                 (CPU_STK_SIZE ) AppTaskEq0FpStk[APP_CFG_TASK_EQ_STK_SIZE / 10u],
                 (CPU_STK_SIZE ) APP_CFG_TASK_EQ_STK_SIZE,
                 (OS_MSG_QTY   ) 0u,
                 (OS_TICK      ) 0u,
                 (void        *) 0,
                 (OS_OPT       ) (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 (OS_ERR      *) &os_err);
								 
    OSTaskCreate((OS_TCB      *) &AppTaskEq1FpTCB,
                 (CPU_CHAR    *) "FP  Equation 1",
                 (OS_TASK_PTR  ) AppTaskEq1Fp,
                 (void        *) 0,
                 (OS_PRIO      ) APP_CFG_TASK_EQ_PRIO,
                 (CPU_STK     *) &AppTaskEq1FpStk[0],
                 (CPU_STK_SIZE ) AppTaskEq1FpStk[APP_CFG_TASK_EQ_STK_SIZE / 10u],
                 (CPU_STK_SIZE ) APP_CFG_TASK_EQ_STK_SIZE,
                 (OS_MSG_QTY   ) 0u,
                 (OS_TICK      ) 0u,
                 (void        *) 0,
                 (OS_OPT       ) (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 (OS_ERR      *) &os_err);
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
	OS_ERR  os_err;


#if (OS_CFG_SEM_EN > 0u)
    OSSemCreate(&AppTaskObjSem,
                "Sem Test",
                 0u,
                &os_err);

    OSSemCreate(&AppTraceSem,
                "Trace Lock",
                 1u,
                &os_err);
#endif

#if (OS_CFG_MUTEX_EN > 0u)
    OSMutexCreate(&AppTaskObjMutex,
                  "Mutex Test",
                  &os_err);
#endif

#if (OS_CFG_Q_EN > 0u)
    OSQCreate(&AppTaskObjQ,
              "Queue Test",
               1,
              &os_err);
#endif

#if (OS_CFG_FLAG_EN > 0u)
    OSFlagCreate(&AppTaskObjFlag,
                 "Flag Test",
                  DEF_BIT_NONE,
                 &os_err);
#endif
  KeyCreate(&Key1,GetPinStateOfKey1);
	KeyCreate(&Key2,GetPinStateOfKey2);
}
/*
*********************************************************************************************************
*                                          AppTaskObj0()
*
* Description : Test uC/OS-III objects.
*
* Argument(s) : p_arg is the argument passed to 'AppTaskObj0' by 'OSTaskCreate()'.
*
* Return(s)   : none
*
* Caller(s)   : This is a task
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppTaskObj0 (void  *p_arg)
{
    OS_ERR  os_err;

    (void)p_arg;


    while (DEF_TRUE) {
#if (OS_CFG_SEM_EN > 0u)
        OSSemPost(&AppTaskObjSem,
                   OS_OPT_POST_1,
                  &os_err);
#endif

#if (OS_CFG_MUTEX_EN > 0u)
        OSMutexPend(&AppTaskObjMutex,
                     0,
                     OS_OPT_PEND_BLOCKING,
                     0,
                    &os_err);

        OSTimeDlyHMSM( 0u, 0u, 0u, 100u,
                       OS_OPT_TIME_HMSM_STRICT,
                      &os_err);

        OSMutexPost(&AppTaskObjMutex,
                     OS_OPT_POST_NONE,
                    &os_err);
#endif

#if (OS_CFG_Q_EN > 0u)
        OSQPost(        &AppTaskObjQ,
                (void *) 1u,
                         1u,
                         OS_OPT_POST_FIFO,
                        &os_err);

#endif

#if (OS_CFG_FLAG_EN > 0u)
        OSFlagPost(&AppTaskObjFlag,
                    DEF_BIT_00,
                    OS_OPT_POST_FLAG_SET,
                   &os_err);
#endif
        OSTimeDlyHMSM( 0u, 0u, 1u, 0u,
                       OS_OPT_TIME_HMSM_STRICT,
                      &os_err);
        APP_TRACE_INFO(("Object test task 0 running ....\n"));
    }
}


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

static  void  AppTaskObj1 (void  *p_arg)
{
    OS_ERR       os_err;
    OS_MSG_SIZE  msg_size;


    (void)p_arg;

    while (DEF_TRUE) {

#if (OS_CFG_SEM_EN > 0u)
        OSSemPend(&AppTaskObjSem,
                   0,
                   OS_OPT_PEND_BLOCKING,
                   0,
                  &os_err);
#endif

#if (OS_CFG_MUTEX_EN > 0u)
        OSMutexPend(&AppTaskObjMutex,
                     0,
                     OS_OPT_PEND_BLOCKING,
                     0,
                    &os_err);

        OSMutexPost(&AppTaskObjMutex,
                     OS_OPT_POST_NONE,
                    &os_err);

#endif

#if (OS_CFG_Q_EN > 0u)
        OSQPend(&AppTaskObjQ,
                 0,
                 OS_OPT_PEND_BLOCKING,
                &msg_size,
                 0,
                &os_err);
#endif

#if (OS_CFG_FLAG_EN > 0u)
        OSFlagPend(&AppTaskObjFlag,
                    DEF_BIT_00,
                    0,
                    OS_OPT_PEND_FLAG_SET_ALL + OS_OPT_PEND_FLAG_CONSUME + OS_OPT_PEND_BLOCKING,
                    0,
                   &os_err);
#endif
        OSTimeDlyHMSM( 0u, 0u, 1u, 0u,
                       OS_OPT_TIME_HMSM_STRICT,
                      &os_err);
        APP_TRACE_INFO(("Object test task 1 running ....\n"));
    }
}

/*
*********************************************************************************************************
*                                             App_TaskEq0Fp()
*
* Description : This task finds the root of the following equation.
*               f(x) =  e^-x(3.2 sin(x) - 0.5 cos(x)) using the bisection mehtod
*
* Argument(s) : p_arg   is the argument passed to 'App_TaskEq0Fp' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  AppTaskEq0Fp (void  *p_arg)
{
		OS_ERR       os_err;
    CPU_FP32    a;
    CPU_FP32    b;
    CPU_FP32    c;
    CPU_FP32    eps;
    CPU_FP32    f_a;
    CPU_FP32    f_c;
    CPU_FP32    delta;
    CPU_INT08U  iteration;
    RAND_NBR    wait_cycles;
        
    
    while (DEF_TRUE) {
        eps       = 0.00001;
        a         = 3.0; 
        b         = 4.0;
        delta     = a - b;
        iteration = 0u;
        if (delta < 0) {
            delta = delta * -1.0;
        }
        
        while (((2.00 * eps) < delta) || 
               (iteration    > 20u  )) {
            c   = (a + b) / 2.00;
            f_a = (exp((-1.0) * a) * (3.2 * sin(a) - 0.5 * cos(a)));
            f_c = (exp((-1.0) * c) * (3.2 * sin(c) - 0.5 * cos(c)));
            
            if (((f_a > 0.0) && (f_c < 0.0)) || 
                ((f_a < 0.0) && (f_c > 0.0))) {
                b = c;
            } else if (((f_a > 0.0) && (f_c > 0.0)) || 
                       ((f_a < 0.0) && (f_c < 0.0))) {
                a = c;           
            } else {
                break;
            }
                
            delta = a - b;
            if (delta < 0) {
               delta = delta * -1.0;
            }
            iteration++;

            wait_cycles = Math_Rand();
            wait_cycles = wait_cycles % 1000;

            while (wait_cycles > 0u) {
                wait_cycles--;
            }

            if (iteration > APP_TASK_EQ_0_ITERATION_NBR) {
                APP_TRACE_INFO(("App_TaskEq0Fp() max # iteration reached\n"));
                break;
            }            
        }

        APP_TRACE_INFO(("Eq0 Task Running ....\n"));
        
        if (iteration == APP_TASK_EQ_0_ITERATION_NBR) {
            APP_TRACE_INFO(("Root = %f; f(c) = %f; #iterations : %d\n", c, f_c, iteration));
        }
				
				OSTimeDlyHMSM( 0u, 0u, 2u, 0u,
                       OS_OPT_TIME_HMSM_STRICT,
                      &os_err);
    }
}

/*
*********************************************************************************************************
*                                             AppTaskEq1Fp()
*
* Description : This task finds the root of the following equation.
*               f(x) = x^2 -3 using the bisection mehtod
*
* Argument(s) : p_arg   is the argument passed to 'AppTaskEq()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Note(s)     : 1) Atollic TrueSTUDIO needs special settings to properly output floating point format;
*                  however, IAR and KeilMDK should work properly.
*********************************************************************************************************
*/

void  AppTaskEq1Fp (void  *p_arg)
{
	  OS_ERR       os_err;
    CPU_FP32    a;
    CPU_FP32    b;
    CPU_FP32    c;
    CPU_FP32    eps;
    CPU_FP32    f_a;
    CPU_FP32    f_c;
    CPU_FP32    delta;
    CPU_INT08U  iteration;
    RAND_NBR    wait_cycles;


    while (DEF_TRUE) {
        eps       = 0.00001;
        a         = 1.0;
        b         = 4.0;
        delta     = a - b;
        iteration = 0u;

        if (delta < 0) {
            delta = delta * -1.0;
        }

        while ((2.00 * eps) < delta) {
            c   = (a + b) / 2.0;
            f_a = a * a - 3.0;
            f_c = c * c - 3.0;

            if (((f_a > 0.0) && (f_c < 0.0)) ||
                ((f_a < 0.0) && (f_c > 0.0))) {
                b = c;
            } else if (((f_a > 0.0) && (f_c > 0.0)) ||
                       ((f_a < 0.0) && (f_c < 0.0))) {
                a = c;
            } else {
                break;
            }

            delta = a - b;
            if (delta < 0) {
               delta = delta * -1.0;
            }
            iteration++;

            wait_cycles = Math_Rand();
            wait_cycles = wait_cycles % 1000;

            while (wait_cycles > 0u) {
                wait_cycles--;
            }

            if (iteration > APP_TASK_EQ_1_ITERATION_NBR) {
                APP_TRACE_INFO(("AppTaskEq1Fp() max # iteration reached\n"));
                break;
            }
        }

        APP_TRACE_INFO(("Eq1 Task Running ....\n"));

        if (iteration == APP_TASK_EQ_1_ITERATION_NBR)
				{ 
          APP_TRACE_INFO(("Root = %f; f(c) = %f; #iterations : %d\n", c, f_c, iteration));
        }
				OSTimeDlyHMSM( 0u, 0u, 2u, 0u,
                       OS_OPT_TIME_HMSM_STRICT,
                      &os_err);
    }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
