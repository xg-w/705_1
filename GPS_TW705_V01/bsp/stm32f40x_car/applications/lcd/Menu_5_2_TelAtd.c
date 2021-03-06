/************************************************************
 * Copyright (C), 2008-2012,
 * FileName:		// 文件名
 * Author:			// 作者
 * Date:			// 日期
 * Description:		// 模块描述
 * Version:			// 版本信息
 * Function List:	// 主要函数及其功能
 *     1. -------
 * History:			// 历史修改记录
 *     <author>  <time>   <version >   <desc>
 *     David    96/10/12     1.0     build this moudle
 ***********************************************************/
#include "Menu_Include.h"
#include "sed1520.h"

extern uint8_t tel_state;

u8 tel_screen = 0;


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void msg( void *p )
{
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void show( void )
{
	pMenuItem->tick=rt_tick_get();

	lcd_fill( 0 );
	lcd_text12( 20, 0, "一键回拨号码", 12, LCD_MODE_SET );
	lcd_text12( 20, 16, jt808_param.id_0x0040, strlen(jt808_param.id_0x0040), LCD_MODE_SET );
	lcd_update_all( );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void keypress( unsigned int key )
{
	switch( key )
	{
		case KEY_MENU:
			pMenuItem = &Menu_5_other;
			pMenuItem->show( );
			break;
		case KEY_OK:
			if( tel_state == 0 )
			{
				//OneKeyCallFlag = 1;

				lcd_fill( 0 );
				lcd_text12( 42, 10, "回拨中", 6, LCD_MODE_SET );
				lcd_update_all( );
				//---------  一键拨号------

				tel_state = 1;
					
				/*OneKeyCallFlag=1;
				   One_largeCounter=0;
				   One_smallCounter=0;*/
#if NEED_TODO
				if( DataLink_Status( ) && ( CallState == CallState_Idle ) ) //电话空闲且在线情况下
				{
					Speak_ON;                                               //开启功放
					rt_kprintf( "\n  一键回拨(监听号码)-->普通通话\n" );
				}
				CallState = CallState_rdytoDialLis;                         // 准备开始拨打监听号码
#endif
			}

			break;
		case KEY_UP:
			break;
		case KEY_DOWN:
			break;
	}
}


MENUITEM Menu_5_2_TelAtd =
{
	"一键回拨",
	8,0,
	&show,
	&keypress,
	&timetick_default,
	&msg,
	(void*)0
};

/************************************** The End Of File **************************************/
