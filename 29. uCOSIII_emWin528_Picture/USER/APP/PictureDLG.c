/*********************************************************************
*                                                                    *
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
*                                                                    *
**********************************************************************
*                                                                    *
* C-file generated by:                                               *
*                                                                    *
*        GUI_Builder for emWin version 5.28                          *
*        Compiled Jan 30 2015, 16:41:06                              *
*        (c) 2015 Segger Microcontroller GmbH & Co. KG               *
*                                                                    *
**********************************************************************
*                                                                    *
*        Internet: www.segger.com  Support: support@segger.com       *
*                                                                    *
**********************************************************************
*/
// USER START (Optionally insert additional includes)
#include "includes.h"
#include "app.h"
#include "DIALOG.h"
#include "string.h"
// USER END

/**************************************************************************************
*
*       Defines
*
***************************************************************************************
*/

// USER START (Optionally insert additional defines)
/*--------------------  (ʹ��U2C.exeС��������)  ------------------------------------*/
#define UTF8_PICTUREDLG        "\xe7\x85\xa7\xe7\x89\x87"//��Ƭ

/*-----------------------------------------------------------------------------------*/

#define g_ucPicNum    8
#define PATHLENTH	    40
#define _DF1S	        0x81
#define imageWinX     680
#define imageWinY     447
// USER END

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

// USER START (Optionally insert additional static data)
static char BMP_Name[g_ucPicNum][PATHLENTH];
static int8_t BMP_Num=0;
static int8_t BMP_Index=0;
static char JPG_Name[g_ucPicNum][PATHLENTH];
static int8_t JPG_Num=0;
static int8_t JPG_Index=0;
static char GIF_Name[g_ucPicNum][PATHLENTH];
static int8_t GIF_Num=0;
static int8_t GIF_Index=0;
static uint8_t g_ucPicType = 0;

static char picpath[PATHLENTH*2]="1:/Picture";

static GUI_HMEM hPicMemory;
char *_acBuffer;
GUI_GIF_INFO InfoGif1;
GUI_GIF_IMAGE_INFO InfoGif2;
GUI_JPEG_INFO JpegInfo;
// USER END

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreatePicture[] = {
  { FRAMEWIN_CreateIndirect, "PICTURE",               0, 0,   0,   800, 480, 0, 0x0, 0 },
  { BUTTON_CreateIndirect,   "BACK",     GUI_ID_BUTTON0, 5,   220, 40,  30,  0, 0x0, 0 },
  { BUTTON_CreateIndirect,   "NEXT",     GUI_ID_BUTTON1, 745, 220, 40,  30,  0, 0x0, 0 },
  { BUTTON_CreateIndirect,   "TYPE",     GUI_ID_BUTTON2, 5,   300, 40,  30,  0, 0x0, 0 },
  { BUTTON_CreateIndirect,   "ZOOM IN",  GUI_ID_BUTTON3, 750, 300, 30,  30,  0, 0x0, 0 },
  { BUTTON_CreateIndirect,   "ZOOM OUT", GUI_ID_BUTTON4, 750, 360, 30,  30,  0, 0x0, 0 },
  // USER START (Optionally insert additional widgets)
  // USER END
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
// USER START (Optionally insert additional static code)
/**
  * @brief  scan_files �ݹ�ɨ��sd���ڵĸ����ļ�
  * @param  path:��ʼɨ��·��
  * @retval result:�ļ�ϵͳ�ķ���ֵ
  */
static FRESULT scan_files (char* path) 
{ 
  FRESULT res; 		//�����ڵݹ���̱��޸ĵı���������ȫ�ֱ���	
  FILINFO fno; 
  DIR dir; 
  int i; 
  char *fn;
#if _USE_LFN 
  static char lfn[_MAX_LFN * (_DF1S ? 2 : 1) + 1]; 	//���ļ���֧��
  fno.lfname = lfn; 
  fno.lfsize = sizeof(lfn); 
#endif 
	
  res = f_opendir(&dir, path); //��Ŀ¼
  if (res == FR_OK) 
	{ 
    i = strlen(path); 
    while(1)
		{ 
      res = f_readdir(&dir, &fno); 										//��ȡĿ¼�µ�����
      if (res != FR_OK || fno.fname[0] == 0) break; 	//Ϊ��ʱ��ʾ������Ŀ��ȡ��ϣ�����
#if _USE_LFN 
      fn = *fno.lfname ? fno.lfname : fno.fname; 
#else 
      fn = fno.fname; 
#endif 
      if (*fn == '.') continue; 											//���ʾ��ǰĿ¼������			
      if (fno.fattrib & AM_DIR) 
			{ 																							//Ŀ¼���ݹ��ȡ
         sprintf(&path[i], "/%s", fn); 							//�ϳ�����Ŀ¼��
         res = scan_files(path);											//�ݹ���� 
         if (res != FR_OK)	break; 																		//��ʧ�ܣ�����ѭ��
         path[i] = 0; 
      } 
			else 
			{ 
				if(strstr(fn,".BMP")||strstr(fn,".bmp"))
				{
					if (strlen(path)+strlen(fn)<PATHLENTH)
					{			
            sprintf(BMP_Name[BMP_Num], "%s/%s", path, fn);
						BMP_Num++;//��¼�ļ�����
						if(BMP_Num>=g_ucPicNum)BMP_Num=g_ucPicNum-1;
					}
				}
				if(strstr(fn,".JPG")||strstr(fn,".jpg"))
				{
					if (strlen(path)+strlen(fn)<PATHLENTH)
					{
            sprintf(JPG_Name[JPG_Num], "%s/%s", path, fn);
						JPG_Num++;//��¼�ļ�����
						if(JPG_Num>=g_ucPicNum)JPG_Num=g_ucPicNum-1;
					}
				}
				if(strstr(fn,".GIF")||strstr(fn,".gif"))
				{
					if (strlen(path)+strlen(fn)<PATHLENTH)
					{	
            sprintf(GIF_Name[GIF_Num], "%s/%s", path, fn);            
						GIF_Num++;//��¼�ļ�����
						if(GIF_Num>=g_ucPicNum)GIF_Num=g_ucPicNum-1;
					}
		    }
      }//else
    } //for
  } 
  return res; 
}
// USER START (Optionally insert additional static code)
static uint8_t  _loadbuffer(const char * sFilename) 
{
	OS_ERR      	err;
	
	OSSchedLock(&err);	
	f_result = f_open(&f_file, sFilename, FA_READ);
	if ((f_result != FR_OK)||(f_file.fsize>1024*1024))
	{
    _acBuffer[0]='\0';        
		OSSchedUnlock(&err);
		return 0;
	}
	f_result = f_read(&f_file, _acBuffer, f_file.fsize, &f_num);
	if(f_result != FR_OK)
  {
		OSSchedUnlock(&err);
		return 0;
  }
  f_close(&f_file);	
	OSSchedUnlock(&err);
	return 1;
	
}
// USER END

/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialogPicture(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;
  OS_ERR      	err;
  // USER START (Optionally insert additional variables)
	static int bmp_amred = 100;
  static int jpg_amred = 100;
  // USER END

  switch (pMsg->MsgId) {
	case WM_DELETE:
		OS_INFO("PictureDLG delete\n");
		Flag_ICON10 = 0;
		BMP_Index=0;
		JPG_Index=0;
		GIF_Index=0;
		UserApp_Flag = 0;
    _acBuffer=NULL;
    GUI_ALLOC_Free(hPicMemory);  
		tpad_flag=0;
	break;
  case WM_INIT_DIALOG:
    //
    // Initialization of 'PICTURE'
    //
    hItem = pMsg->hWin;
    FRAMEWIN_SetText(hItem,UTF8_PICTUREDLG);
    FRAMEWIN_AddCloseButton(hItem,FRAMEWIN_BUTTON_RIGHT,0);
    //
    // Initialization of 'BACK'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_BUTTON0);
    BUTTON_SetFont(hItem, GUI_FONT_16B_ASCII);
		BUTTON_SetText(hItem,"<<<");
    //
    // Initialization of 'NEXT'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_BUTTON1);
    BUTTON_SetFont(hItem, GUI_FONT_16B_ASCII);
		BUTTON_SetText(hItem,">>>");
		//
    // Initialization of 'TYPE'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_BUTTON2);
    BUTTON_SetFont(hItem, GUI_FONT_16B_ASCII);
		BUTTON_SetText(hItem,"BMP");
		//
    // Initialization of 'ZOOM IN'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_BUTTON3);
    BUTTON_SetFont(hItem, GUI_FONT_16B_ASCII);
		BUTTON_SetText(hItem,"++");
		//
    // Initialization of 'ZOOM OUT'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_BUTTON4);
    BUTTON_SetFont(hItem, GUI_FONT_16B_ASCII);
		BUTTON_SetText(hItem,"--");
    // USER START (Optionally insert additional code for further widget initialization)
    // USER END
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case GUI_ID_BUTTON0: // Notifications sent by 'BACK'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        GUI_SetBkColor(APPBKCOLOR);
        GUI_Clear();
        if(g_ucPicType == 0)
        {
          BMP_Index--;
          if(BMP_Index < 0)BMP_Index=BMP_Num-1;
          if(_loadbuffer(BMP_Name[BMP_Index])==0)break;
          OSSchedLock(&err);			
          GUI_BMP_Draw(_acBuffer,
                 (imageWinX - GUI_BMP_GetXSize(_acBuffer))/2, 
                 (imageWinY - GUI_BMP_GetYSize(_acBuffer))/2);
          OSSchedUnlock(&err);																	
        }
        else if(g_ucPicType == 1)
        {
          JPG_Index--;
          if(JPG_Index < 0)JPG_Index=JPG_Num-1;
          _loadbuffer(JPG_Name[JPG_Index]);
          OSSchedLock(&err);
          GUI_JPEG_GetInfo(_acBuffer, f_file.fsize, &JpegInfo);
          
          GUI_JPEG_DrawScaled(_acBuffer,
                              f_file.fsize, 
                              (imageWinX - JpegInfo.XSize)/2, 
                              (imageWinY - JpegInfo.YSize)/2,
                              100,
                              100);
          OSSchedUnlock(&err);						
        }
        else
        {
          GIF_Index--;
          if(GIF_Index < 0)GIF_Index=GIF_Num-1;
          _loadbuffer(GIF_Name[GIF_Index]);
          GUI_GIF_GetInfo(_acBuffer, f_file.fsize, &InfoGif1);
        }
        //IMAGE_SetBMP(WM_GetDialogItem(pMsg->hWin, GUI_ID_IMAGE0),);
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case GUI_ID_BUTTON1: // Notifications sent by 'NEXT'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        GUI_SetBkColor(APPBKCOLOR);
        GUI_Clear();
        if(g_ucPicType == 0)
        {
          BMP_Index++;
          if(BMP_Index >= BMP_Num)BMP_Index=0;

          if(_loadbuffer(BMP_Name[BMP_Index])==0)break;
          
          OSSchedLock(&err);
          GUI_BMP_Draw(_acBuffer,
                 (imageWinX - GUI_BMP_GetXSize(_acBuffer))/2, 
                 (imageWinY - GUI_BMP_GetYSize(_acBuffer))/2);
          OSSchedUnlock(&err);																	
        }
        else if(g_ucPicType == 1)
        {
          JPG_Index++;
          if(JPG_Index >= JPG_Num)JPG_Index=0;
          _loadbuffer(JPG_Name[JPG_Index]);

          OSSchedLock(&err);
          GUI_JPEG_GetInfo(_acBuffer, f_file.fsize, &JpegInfo);
          GUI_JPEG_DrawScaled(_acBuffer,
                              f_file.fsize, 
                              (imageWinX - JpegInfo.XSize)/2, 
                              (imageWinY - JpegInfo.YSize)/2,
                              100,
                              100);
          OSSchedUnlock(&err);						
        }
        else
        {
          GIF_Index++;
          if(GIF_Index >= GIF_Num)GIF_Index=0;
          _loadbuffer(GIF_Name[GIF_Index]);
          GUI_GIF_GetInfo(_acBuffer, f_file.fsize, &InfoGif1);
        }
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
	  case GUI_ID_BUTTON2: // Notifications sent by 'TYPE'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        if(g_ucPicType == 0)
        {
          g_ucPicType = 1;
          BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_BUTTON2), "JPG");							
        }
        else if(g_ucPicType == 1)
        {
          g_ucPicType = 2;
          GUI_SetBkColor(APPBKCOLOR);
          GUI_Clear();
          _loadbuffer(GIF_Name[GIF_Index]);
          GUI_GIF_GetInfo(_acBuffer, f_file.fsize, &InfoGif1);
          BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_BUTTON2), "GIF");
        }
        else
        {
          g_ucPicType = 0;
          BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_BUTTON2), "BMP");    
        }
        //IMAGE_SetBMP(WM_GetDialogItem(pMsg->hWin, GUI_ID_IMAGE0),);
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
	  case GUI_ID_BUTTON3: // Notifications sent by 'ZOOM IN'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        if(g_ucPicType == 2)break;
        if(g_ucPicType == 0)
        {
          bmp_amred += 10;
          OSSchedLock(&err);
          GUI_BMP_DrawScaled(_acBuffer,
                             (imageWinX - GUI_BMP_GetXSize(_acBuffer)*bmp_amred/100)/2, 
                             (imageWinY - GUI_BMP_GetYSize(_acBuffer)*bmp_amred/100)/2,
                             bmp_amred, 
                             100);
          OSSchedUnlock(&err);																	
        }
        else if(g_ucPicType == 1)
        {
          jpg_amred +=10;
          OSSchedLock(&err);
          GUI_JPEG_DrawScaled(_acBuffer,
                              f_file.fsize, 
                              (imageWinX - JpegInfo.XSize*jpg_amred/100)/2, 
                              (imageWinY - JpegInfo.YSize*jpg_amred/100)/2,
                              jpg_amred,
                                    100); 
          OSSchedUnlock(&err);						
        }
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
	  case GUI_ID_BUTTON4: // Notifications sent by 'ZOOM OUT'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        if(g_ucPicType == 2)break;
        GUI_SetBkColor(APPBKCOLOR);
        GUI_Clear();
        if(g_ucPicType == 0)
        {
          bmp_amred -= 10;
                if(bmp_amred < 0) bmp_amred = 10; 
          OSSchedLock(&err);
          GUI_BMP_DrawScaled(_acBuffer,
                             (imageWinX - GUI_BMP_GetXSize(_acBuffer)*bmp_amred/100)/2, 
                             (imageWinY - GUI_BMP_GetYSize(_acBuffer)*bmp_amred/100)/2,
                             bmp_amred, 
                             100);
          OSSchedUnlock(&err);																	
        }
        else if(g_ucPicType == 1)
        {
          jpg_amred -= 10;
                if(jpg_amred < 0) jpg_amred = 10;
          OSSchedLock(&err);
          GUI_JPEG_DrawScaled(_acBuffer,
                              f_file.fsize, 
                              (imageWinX - JpegInfo.XSize*jpg_amred/100)/2, 
                              (imageWinY - JpegInfo.YSize*jpg_amred/100)/2,
                              jpg_amred,
                              100); 
          OSSchedUnlock(&err);						
        }
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    // USER START (Optionally insert additional code for further Ids)
    // USER END
    }
    break;
  // USER START (Optionally insert additional message handling)
  // USER END
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       CreatePicture
*/
void FUN_ICON10Clicked(void)
{
	WM_HWIN hWin;
	WM_HWIN himage;
	uint8_t i=0;
	OS_ERR  err;
	g_ucPicType = 0;
	OS_INFO("PictureDLG create\n");
	com_createdir(picpath);
	hWin=GUI_CreateDialogBox(_aDialogCreatePicture, GUI_COUNTOF(_aDialogCreatePicture), _cbDialogPicture, HDTWIN, 0, 0);
	himage=WM_CreateWindowAsChild(60,33,imageWinX,imageWinY,hWin,WM_CF_SHOW,NULL,0);
	WM_SelectWindow(himage);
  GUI_SetBkColor(APPBKCOLOR);
  GUI_Clear();
  GUI_Delay(100);
  hPicMemory = GUI_ALLOC_AllocZero(1024*1024);
  _acBuffer = GUI_ALLOC_h2p(hPicMemory);
  if(_acBuffer==NULL)  bsp_result|=BSP_ALLOC;
  BMP_Num=0;
	JPG_Num=0;
	GIF_Num=0;
	scan_files(picpath);
	if(BMP_Num)
	{
		if(_loadbuffer(BMP_Name[BMP_Index])==1)
    {
      OSSchedLock(&err);
      GUI_BMP_Draw(_acBuffer,
            (imageWinX - GUI_BMP_GetXSize(_acBuffer))/2, 
            (imageWinY - GUI_BMP_GetYSize(_acBuffer))/2);
      OSSchedUnlock(&err);
    }
	}
	while(Flag_ICON10)
	{
		if(g_ucPicType == 2)
		{
			if(i < InfoGif1.NumImages)
			{                                    
				OSSchedLock(&err);
				//GUI_GIF_GetImageInfo(_acBuffer2, file.fsize, &InfoGif2, i);
				GUI_GIF_DrawSub(_acBuffer, 
												f_file.fsize, 
												(imageWinX - InfoGif1.xSize)/2, 
												(imageWinY - InfoGif1.ySize)/2, 
												i++);                             
				OSSchedUnlock(&err);
				if(InfoGif2.Delay == 0)				
					GUI_Delay(100);
				else
					GUI_Delay(InfoGif2.Delay*10);                      						 
			}
			else
			{
				i = 0;
			}
			if(tpad_flag)WM_DeleteWindow(hWin);
		}
		else
		{
			if(tpad_flag)WM_DeleteWindow(hWin);
			GUI_Delay(10); 
		}    
	}
}

// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/