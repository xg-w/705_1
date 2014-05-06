/*
 * File      : startup.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-08-31     Bernard      first implementation
 * 2011-06-05     Bernard      modify for STM32F107 version
 */

#include <rthw.h>
#include <rtthread.h>

#include "stm32f4xx.h"
#include "board.h"
#include "jt808.h"
#include "hmi.h"
#include "sle4442.h"
#include "printer.h"
#include "rs485.h"

#include "gps.h"
#include "jt808_param.h"

/**
 * @addtogroup STM32
 */

/*@{*/

//extern int  rt_application_init(void);
#ifdef RT_USING_FINSH
extern uint16_t	mem_ID;				//�洢оƬ�ĳ��Ҵ���,SST25VF032(0xBF4A),W25Q64FV(0xEF16)
extern void finsh_system_init( void );


extern void finsh_set_device( const char* device );


#endif

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define STM32_SRAM_BEGIN ( &Image$$RW_IRAM1$$ZI$$Limit )
#elif __ICCARM__
#pragma section="HEAP"
#define STM32_SRAM_BEGIN ( __segment_end( "HEAP" ) )
#else
extern int __bss_end;
#define STM32_SRAM_BEGIN ( &__bss_end )
#endif


/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed( u8* file, u32 line )
{
	rt_kprintf( "\nWrong parameter value detected on\n" );
	rt_kprintf( "       file  %s\n", file );
	rt_kprintf( "       line  %d\n", line );

	while( 1 )
	{
		;
	}
}


void PowerON_OFF_GSM_GPS(void)
{
	uint32_t			counter=0;
 	GPIO_InitTypeDef	GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD, ENABLE );

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init( GPIOD, &GPIO_InitStructure );
	GPIO_SetBits( GPIOD, GPIO_Pin_10 );			///��GPSģ��

	while(counter++ < 10000000)
		{
		;
		}
	counter = 0;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init( GPIOD, &GPIO_InitStructure );
	GPIO_SetBits( GPIOD, GPIO_Pin_13 );			///��GSMģ��
	
	
	while(counter++ < 15000000)
		{
		;
		}
	GPIO_ResetBits( GPIOD, GPIO_Pin_13 );		///�ر�GSMģ��
}


/**
 * This function will startup RT-Thread RTOS.
 */
void rtthread_startup( void )
{
	rt_hw_board_init( );
	rt_show_version( );
	PowerON_OFF_GSM_GPS( );
	rt_system_tick_init( );
	rt_system_object_init( );
	rt_system_timer_init( );

	rt_system_heap_init( (void*)STM32_SRAM_BEGIN, (void*)STM32_SRAM_END );
	rt_system_scheduler_init( );

	rt_device_init_all( );
	rt_kprintf( "\nrcc.csr=%08x", RCC->CSR );

	key_lcd_port_init( );
	sst25_init( );      /*�ڴ˳�ʼ��,gsm���ܶ�ȡ����������app_thread�в�����ִ��*/
	if(mem_ID == 0xBF4A)
	{
		rt_kprintf("\n �洢оƬ:SST25VF032");
	}
	else
	{
		rt_kprintf("\n �洢оƬ:W25Q64FV");
	}
	rt_kprintf("\n MEM_ID=%04X",mem_ID);
	rt_sem_init( &sem_dataflash, "sem_df", 0, RT_IPC_FLAG_FIFO );
	rt_sem_release( &sem_dataflash );
	param_load();		/*����ϵͳ������û��ʹ���ź���*/
	data_load();		/*������Ҫ���ڱ�����豸����*/
	
	//rt_kprintf("\n%d>�����ͺ�:TD%04x",rt_tick_get(),jt808_param.id_0xF013);
	rt_kprintf("\n%d>Ӳ       ��:HV%d",rt_tick_get(),HARD_VER);
	rt_kprintf("\n%d>BIN�ļ��汾:V%d.%02d",rt_tick_get(),SOFT_VER/100,SOFT_VER%100);
	update_send_info();
	gps_init( );

	printer_driver_init();

	usbh_init( );
	
	//spi_sd_init( );
	//SD_test_init();
	rt_application_init( );
	RS485_init( );

	gsm_init( );
	hmi_init( );
	jt808_init( );
	
#ifdef RT_USING_FINSH
	/* init finsh */
	finsh_system_init( );
	finsh_set_device( FINSH_DEVICE_NAME );
#endif

	/* init timer thread */
	rt_system_timer_thread_init( );

	/* init idle thread */
	rt_thread_idle_init( );

	/* start scheduler */
	rt_system_scheduler_start( );

	/* never reach here */
	return;
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
int main( void )
{
	/* disable interrupt first */
	rt_hw_interrupt_disable( );
	/* startup RT-Thread RTOS */
	rtthread_startup( );

	return 0;
}

/*@}*/