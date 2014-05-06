/************************************************************
 * Copyright (C), 2008-2012,
 * FileName:		// �ļ���
 * Author:			// ����
 * Date:			// ����
 * Description:		// ģ������
 * Version:			// �汾��Ϣ
 * Function List:	// ��Ҫ�������书��
 *     1. -------
 * History:			// ��ʷ�޸ļ�¼
 *     <author>  <time>   <version >   <desc>
 *     David    96/10/12     1.0     build this moudle
 ***********************************************************/

#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>

#include "stm32f4xx.h"
#include "gps.h"
#include "jt808.h"
#include "jt808_gps.h"
#include "jt808_param.h"
#include "hmi.h"

#include <finsh.h>

#define GPS_PWR_PORT	GPIOD
#define GPS_PWR_PIN		GPIO_Pin_10

#define GPS_GPIO_TX			GPIO_Pin_12 // PC12
#define GPS_GPIO_RX			GPIO_Pin_2  // PD2
#define GPS_GPIO_TxC		GPIOC
#define GPS_GPIO_RxD		GPIOD
#define RCC_APBPeriph_UART5 RCC_APB1Periph_UART5

/*����һ��gps�豸*/
struct rt_device dev_gps;

/*���ڽ��ջ���������*/
#define UART5_RX_SIZE 256

typedef __packed struct
{
	uint16_t	wr;
	uint8_t		body[UART5_RX_SIZE];
}LENGTH_BUF;

static LENGTH_BUF uart5_rxbuf;

//static uint8_t	uart5_rxbuf[UART5_RX_SIZE];	/*Ԥ��ǰ�����ֽڣ����泤��*/
//static uint16_t uart5_rxbuf_wr = 2;

/*gpsԭʼ��Ϣ����������*/
#define GPS_RAWINFO_SIZE	2048
#define NEMA_SIZE			96 /*NEMA���ĳ���*/
static uint8_t					gps_rawinfo[GPS_RAWINFO_SIZE];
static struct rt_messagequeue	mq_gps;

uint8_t							flag_bd_upgrade_uart = 0;
uint16_t						gps_reset_time = 200;			///gpsģ��೤ʱ�䲻��λ��Ҫ��λ����λΪ��


extern struct rt_device			dev_vuart;

static rt_uint8_t				*ptr_mem_packet = RT_NULL;
#if 0
//*****************************************
//CRC16 ���ֽڱ�
//*****************************************

static unsigned char CRC16TabH[256] =
{
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

//*****************************************
//CRC16 ���ֽڱ�
//*****************************************
static unsigned char CRC16TabL[256] =
{
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2,
	0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
	0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
	0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
	0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
	0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6,
	0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
	0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
	0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE,
	0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
	0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA,
	0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
	0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
	0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62,
	0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
	0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE,
	0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
	0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
	0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76,
	0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
	0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
	0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
	0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
	0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A,
	0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86,
	0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};
//*************************************************
//  ����CRCУ���
// unsigned char* src �C  ��У������
// int startpoint �C  У������ƫ��
// int len �C  У�����ݳ���
//************************************************
unsigned short  CalcCRC16( unsigned char*  src, int startpoint, int len )
{
	unsigned short	res		= 0;
	unsigned char	crc_h	= 0;
	unsigned char	crc_l	= 0;

	unsigned char	tmp;
	int				i;

	for( i = startpoint; i < ( len + startpoint ); i++ )
	{
		tmp		= crc_h ^ src[i];
		crc_h	= crc_l ^ CRC16TabH[tmp];
		crc_l	= CRC16TabL[tmp];
	}
	res = ( crc_h << 8 ) | crc_l;
	return res;
}

#endif


/*
   gps�����жϴ������յ�\n��Ϊ�յ�һ��
   �յ�һ�����ô�������
 */

void UART5_IRQHandler( void )
{
	static uint8_t	last_ch = 0;
	uint8_t			ch;
	rt_interrupt_enter( );
	if( USART_GetITStatus( UART5, USART_IT_RXNE ) != RESET )
	{
		ch = USART_ReceiveData( UART5 );

		uart5_rxbuf.body[uart5_rxbuf.wr++]	= ch;
		uart5_rxbuf.wr						&= ( UART5_RX_SIZE - 1 );
		uart5_rxbuf.body[uart5_rxbuf.wr]	= 0;

		if( ( ch == 0x0a ) && ( last_ch == 0x0d ) ) /*����0d 0a ��������*/
		{
			if( uart5_rxbuf.wr < NEMA_SIZE )
			{
				rt_mq_send( &mq_gps, (void*)&uart5_rxbuf, uart5_rxbuf.wr + 2 );
			}
			uart5_rxbuf.wr = 0;
		}
		last_ch = ch;
		USART_ClearITPendingBit( UART5, USART_IT_RXNE );
	}
	rt_interrupt_leave( );
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
void gps_baud( int baud )
{
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate				= baud;
	USART_InitStructure.USART_WordLength			= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits				= USART_StopBits_1;
	USART_InitStructure.USART_Parity				= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode					= USART_Mode_Rx | USART_Mode_Tx;
	USART_Init( UART5, &USART_InitStructure );
}

FINSH_FUNCTION_EXPORT( gps_baud, config gsp_baud );

/*��ʼ��*/
static rt_err_t dev_gps_init( rt_device_t dev )
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC, ENABLE );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD, ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_UART5, ENABLE );

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_2MHz;

	GPIO_InitStructure.GPIO_Pin = GPS_PWR_PIN;
	GPIO_Init( GPS_PWR_PORT, &GPIO_InitStructure );
	GPIO_ResetBits( GPS_PWR_PORT, GPS_PWR_PIN );

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	//GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_4;
	GPIO_Init( GPIOD, &GPIO_InitStructure );

	//GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init( GPIOB, &GPIO_InitStructure );

/*uart5 �ܽ�����*/

	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_12;
	GPIO_Init( GPIOC, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init( GPIOD, &GPIO_InitStructure );

	GPIO_PinAFConfig( GPIOC, GPIO_PinSource12, GPIO_AF_UART5 );
	GPIO_PinAFConfig( GPIOD, GPIO_PinSource2, GPIO_AF_UART5 );

/*NVIC ����*/
	NVIC_InitStructure.NVIC_IRQChannel						= UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init( &NVIC_InitStructure );

	gps_baud( 9600 );
	USART_Cmd( UART5, ENABLE );
	USART_ITConfig( UART5, USART_IT_RXNE, ENABLE );

	GPIO_SetBits( GPS_PWR_PORT, GPS_PWR_PIN );

	return RT_EOK;
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
static rt_err_t dev_gps_open( rt_device_t dev, rt_uint16_t oflag )
{
	GPIO_SetBits( GPS_PWR_PORT, GPS_PWR_PIN );
	return RT_EOK;
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
static rt_err_t dev_gps_close( rt_device_t dev )
{
	GPIO_ResetBits( GPS_PWR_PORT, GPS_PWR_PIN );
	return RT_EOK;
}

/***********************************************************
* Function:gps_read
* Description:����ģʽ�¶�ȡ����
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_size_t dev_gps_read( rt_device_t dev, rt_off_t pos, void* buff, rt_size_t count )
{
	return RT_EOK;
}

/* write one character to serial, must not trigger interrupt */
static void uart5_putc( const char c )
{
	USART_SendData( UART5, c );
	while( !( UART5->SR & USART_FLAG_TXE ) )
	{
		;
	}
	UART5->DR = ( c & 0x1FF );
}

/***********************************************************
* Function:		gps_write
* Description:	����ģʽ�·������ݣ�Ҫ�����ݽ��з�װ
* Input:		const void* buff	Ҫ���͵�ԭʼ����
       rt_size_t count	Ҫ�������ݵĳ���
       rt_off_t pos		ʹ�õ�socket���
* Output:
* Return:
* Others:
***********************************************************/

static rt_size_t dev_gps_write( rt_device_t dev, rt_off_t pos, const void* buff, rt_size_t count )
{
	rt_size_t	len = count;
	uint8_t		*p	= (uint8_t*)buff;

	while( len )
	{
		USART_SendData( UART5, *p++ );
		while( USART_GetFlagStatus( UART5, USART_FLAG_TC ) == RESET )
		{
		}
		len--;
	}
	return RT_EOK;
}

/***********************************************************
* Function:		gps_control
* Description:	����ģ��
* Input:		rt_uint8_t cmd	��������
    void *arg       ����,����cmd�Ĳ�ͬ�����ݵ����ݸ�ʽ��ͬ
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t dev_gps_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
	int i = *(int*)arg;
	switch( cmd )
	{
		case CTL_GPS_OUTMODE:
			break;
		case CTL_GPS_BAUD:
			gps_baud( i );
	}
	return RT_EOK;
}

void repower_gps_modem(void)
{
	beep(2,2,3);
 	GPIO_ResetBits( GPIOD, GPIO_Pin_10 );                           /*off gps*/
	//while( rt_mq_recv( &mq_gps, (void*)&buf, NEMA_SIZE, RT_TICK_PER_SECOND / 20 ) == RT_EOK )
	//{
	//	rt_thread_delay( RT_TICK_PER_SECOND / 20 );
	//}
	rt_thread_delay( RT_TICK_PER_SECOND * 3 );
	GPIO_SetBits( GPIOD, GPIO_Pin_10 );                             /*on gps*/
}

ALIGN( RT_ALIGN_SIZE )
static char thread_gps_stack[2048]  __attribute__( ( section( "CCM_RT_STACK" ) ) );
struct rt_thread thread_gps;


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void rt_thread_entry_gps( void* parameter )
{
	rt_err_t	res;
	LENGTH_BUF	buf;
	uint16_t		count_check_antenna_short = 0;
	uint16_t		count_check_antenna_open = 0;
	uint16_t	    count_check_antenna_ok = 0;
	rt_tick_t	tick_lastrx			= rt_tick_get( );
	static uint8_t tempbuf[32];
	
	while( 1 )
	{
		wdg_thread_counter[1]=1;
		
		//rt_thread_delay( RT_TICK_PER_SECOND / 20 );                         /*Ϊ�˱�֤�ɿ��Ĺ����߳�*/
		
		res = rt_mq_recv( &mq_gps, (void*)&buf, NEMA_SIZE, RT_TICK_PER_SECOND / 20 );   //�ȴ�100ms,ʵ���Ͼ��Ǳ䳤����ʱ,�100ms
		if(device_control.off_counter)
		{
			gps_notfixed_count = 0;
			tick_lastrx = rt_tick_get( );
			continue;
		}
		if(( res == RT_EOK )&&(rt_tick_get() > RT_TICK_PER_SECOND * 3 ))               	//�յ�һ������,�����Ѿ�����������3����
		{
			tick_lastrx = rt_tick_get( );
			jt808_param_bk.car_alarm &= ~BIT_ALARM_GPS_ERR;
			if( flag_bd_upgrade_uart == 0 )
			{
				gps_rx( buf.body, buf.wr );
			}
			else
			{
				if( buf.body[0] == 0x40 )
				{
					rt_device_write( &dev_vuart, 0, buf.body, buf.wr );
				}
			}

			///�Զ��궨����ϵ����ֻ�е��ٶȴ���50����ÿСʱ�ű궨
			if(gps_speed<50)
			{
				car_data.pulse_mileage	= jt808_param_bk.car_mileage;
				car_data.pulse_cap_num	= 0;
			}
			else
			{
				if(((jt808_param_bk.car_mileage - car_data.pulse_mileage)/36000 > 20 )&&(jt808_param.id_0xF032 & BIT(31)))
				{
					float f1,f2;
					f1	= (jt808_param_bk.car_mileage - car_data.pulse_mileage)/36000;
					f2	= car_data.pulse_cap_num;
					f1	= f2/f1;
					jt808_param.id_0xF032	= car_data.pulse_cap_num*36000/(jt808_param_bk.car_mileage - car_data.pulse_mileage);
					//jt808_param.id_0xF032 	&= ~(BIT(31));
					param_save(1);
				}
			}
		}

		///��ʱ��û��������,ģ����
		if( ( rt_tick_get( ) - tick_lastrx ) > RT_TICK_PER_SECOND * 10 )
		{
			sd_write_console("GPS������,����GPS.");
			jt808_param_bk.car_alarm |= BIT_ALARM_GPS_ERR;
			repower_gps_modem();
			tick_lastrx = rt_tick_get( );
		}
		
		///����һ��ʱ��(gps_reset_time)δ��λ��gps_notfixed_count��λΪ�룬gps_reset_time�仯Ϊ200->400->600��֮��Ϊ600����λΪ��
		if( gps_notfixed_count > gps_reset_time )
		{
			sprintf(tempbuf,"%d��δ��λ,����GPS.",gps_reset_time);
			sd_write_console(tempbuf);
			repower_gps_modem();
			gps_notfixed_count = 0;
			if(gps_reset_time<600)
			{
				gps_reset_time += 200;
			}
		}
		// 2013-4-20	����PCB   ��PD4 : GPS ���߿�·		PB6 : GPS  ���߶�·
		if( GPIO_ReadOutputDataBit( GPS_PWR_PORT, GPS_PWR_PIN ) )   // ��GPS �е�ʱ��Ч
		{
			////GPIO_ResetBits( GPIOD, GPIO_Pin_4 );
			if( GPIO_ReadInputDataBit( GPIOD, GPIO_Pin_4 ) )        //��·���	1:���߿�·
			{
				//if((jt808_status&BIT_ALARM_GPS_OPEN)==0)
				count_check_antenna_open++;
				if(count_check_antenna_open==40)
				{
					//count_check_antenna_open=0;
					sd_write_console("GPS���߿�·");
					count_check_antenna_ok = 0;
					jt808_param_bk.car_alarm |= BIT_ALARM_GPS_OPEN;
				}
			}else
			{
				count_check_antenna_open=0;
				jt808_param_bk.car_alarm &= ~BIT_ALARM_GPS_OPEN;
				
				if( !GPIO_ReadInputDataBit( GPIOB, GPIO_Pin_6 ) ) //��·���  0:���߶�·
				{
					count_check_antenna_short++;
					if(count_check_antenna_short==40)
					{
						//count_check_antenna_short=0;
						sd_write_console("GPS���߶�·");
						count_check_antenna_ok = 0;
						jt808_param_bk.car_alarm |= BIT_ALARM_GPS_SHORT;
					}
				}else
				{
					count_check_antenna_short=0;
					jt808_param_bk.car_alarm &= ~BIT_ALARM_GPS_SHORT;
					if(count_check_antenna_ok == 0)
					{
						count_check_antenna_ok = 1;
						sd_write_console("GPS����OK");
					}
				}
			}
		}
	}
}

/*gps�豸��ʼ��*/
void gps_init( void )
{
	rt_mq_init( &mq_gps, "mq_gps", &gps_rawinfo[0], NEMA_SIZE, GPS_RAWINFO_SIZE, RT_IPC_FLAG_FIFO );

	rt_thread_init( &thread_gps,
	                "gps",
	                rt_thread_entry_gps,
	                RT_NULL,
	                &thread_gps_stack[0],
	                sizeof( thread_gps_stack ), 11, 5 );
	rt_thread_startup( &thread_gps );

	dev_gps.type	= RT_Device_Class_Char;
	dev_gps.init	= dev_gps_init;
	dev_gps.open	= dev_gps_open;
	dev_gps.close	= dev_gps_close;
	dev_gps.read	= dev_gps_read;
	dev_gps.write	= dev_gps_write;
	dev_gps.control = dev_gps_control;

	rt_device_register( &dev_gps, "gps", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE );
	rt_device_init( &dev_gps );
}

/*gps����*/
rt_err_t gps_onoff( uint8_t openflag )
{
	if( openflag == 0 )
	{
		GPIO_ResetBits( GPIOD, GPIO_Pin_10 );
		return 0;
	} else
	{
		GPIO_SetBits( GPIOD, GPIO_Pin_10 );
		return 1;
	}
}

FINSH_FUNCTION_EXPORT( gps_onoff, gps_onoff([1 | 0] ) );

/*�߳��˳���cleanup����*/
static void cleanup( struct rt_thread *tid )
{
	if( ptr_mem_packet != RT_NULL )
	{
		rt_free( ptr_mem_packet );
	}
}

/*����ʱ�ĵ������,��ǰ���޷���ӡ���,���ڱ�ռ��*/
static void msg_uart_usb( char *str )
{
	rt_kprintf( "bd>\n%s", str );
}

/*���±����̣߳�ʹ��U�̸���*/
void thread_gps_check_ver( void* parameter )
{
	void		( *msg )( void *p );
	int			count = 0;
	rt_uint8_t	*pdata; /*����*/

	char		buf[32];
	rt_err_t	res;
	LENGTH_BUF	uart_buf;

	uint8_t		ver[3]; /*�汾*/
	uint32_t	model;  /*�ͺ�*/
	uint8_t		ok = 0; /*�Ƿ�ִ����ȷ*/

	uint8_t		cmd_reset[11] = { 0x40, 0x34, 0xC0, 0x00, 0x34, 0x00, 0x01, 0x84, 0x6B, 0x0D, 0x0A };

	msg = parameter;

	msg( "I���ڲ�ѯ..." );
/*����ֹͣgps�߳�*/
	//GPIO_ResetBits( GPIOD, GPIO_Pin_10 );  /*�ȹ�ģ��*/
	while( rt_thread_suspend( &thread_gps ) != RT_EOK )
	{
		rt_kprintf( "S" );
		rt_thread_delay( 2 );
		count++;
		if( count > 200 )
		{
			msg( "E��ѯ�汾ʧ��" );
			//GPIO_SetBits( GPIOD, GPIO_Pin_10 );  /*��ģ��*/
			return;
		}
	}
	rt_kprintf( "\n�����߳�" );
	//GPIO_SetBits( GPIOD, GPIO_Pin_10 );  /*��ģ��*/
	//rt_thread_delay( RT_TICK_PER_SECOND);

/*��������е����ݣ��ٷ���*/
	while( rt_mq_recv( &mq_gps, (void*)&uart_buf, NEMA_SIZE, 0 ) == RT_EOK )
	{
		rt_thread_delay( RT_TICK_PER_SECOND / 10 );
	}

/*��������״̬*/
	memcpy( buf, "\x40\x30\xC0\x00\x03\x00\x01\x34\x21\x0D\x0A", 11 );
	dev_gps_write( &dev_gps, 0, buf, 11 );
	rt_thread_delay( RT_TICK_PER_SECOND );
/*�汾��ѯ*/
	count = 0;
	while( rt_mq_recv( &mq_gps, (void*)&uart_buf, NEMA_SIZE, 0 ) == RT_EOK )
	{
		rt_thread_delay( RT_TICK_PER_SECOND / 10 );
	}
	for( count = 0; count < 5; count++ )
	{
		dev_gps_write( &dev_gps, 0, "\x40\x10\xC0\x00\x10\x00\x01\xC2\x84\x0D\x0A", 11 );
		res = rt_mq_recv( &mq_gps, (void*)&uart_buf, NEMA_SIZE, RT_TICK_PER_SECOND * 5 );
		if( res == RT_EOK )                                             //�յ�һ������
		{
			if( ( uart_buf.wr == 15 ) && ( uart_buf.body[4] == 0x02 ) ) /*��������״̬*/
			{
				ver[0]	= ( uart_buf.body[7] & 0xf0 ) >> 4;
				ver[1]	= ( uart_buf.body[7] & 0xf );
				ver[2]	= uart_buf.body[8];
				ok		= 1;
				break;
			}else
			{
				rt_kprintf( "bd>%s", uart_buf.body );
			}
		}
	}

	if( ok == 0 )
	{
		msg( "E��ѯģ��汾����" );
	}

	for( count = 0; count < 5; count++ )
	{
		dev_gps_write( &dev_gps, 0, "\x40\x16\xC0\x00\x16\x00\x01\x22\xE3\x0D\x0A", 11 );
		res = rt_mq_recv( &mq_gps, (void*)&uart_buf, NEMA_SIZE, RT_TICK_PER_SECOND * 5 );
		if( res == RT_EOK )               //�յ�һ������
		{
			if( ( uart_buf.wr == 28 ) && ( uart_buf.body[5] == 0x0 ) )
			{
				model = ( uart_buf.body[6] << 24 ) | ( uart_buf.body[7] << 16 ) | ( uart_buf.body[8] << 8 ) | ( uart_buf.body[9] );
				ok++;
				break;
			}else
			{
				rt_kprintf( "bd>%s", uart_buf.body );
			}
		}
	}

	if( ok )
	{
		sprintf( buf, "Eģ��TD%d (%d.%d.%d)", model, ver[0], ver[1], ver[2] );
		msg( buf );
	}else
	{
		msg( "E��ѯģ���ͺŴ���" );
	}

lbl_check_ver_err:
	dev_gps_write( &dev_gps, 0, cmd_reset, 11 );

	rt_thread_delay( RT_TICK_PER_SECOND * 2 );
	while( rt_thread_resume( &thread_gps ) != RT_EOK )
	{
		rt_thread_delay( RT_TICK_PER_SECOND / 20 );
	}
	rt_kprintf( "\nbd>resume gps ok" );
}

/*���±����̣߳�ʹ��U�̸���*/
void thread_gps_upgrade_udisk( void* parameter )
{
#define READ_PACKET_SIZE 1012

	void		( *msg )( void *p );
	int			fd = -1, count = 0;
	rt_uint8_t	*pdata; /*����*/
	uint8_t		cmd_reset[11] = { 0x40, 0x34, 0xC0, 0x00, 0x34, 0x00, 0x01, 0x84, 0x6B, 0x0D, 0x0A };

	char		buf[32];
	rt_uint8_t	ch_h, ch_l;
	rt_err_t	res;
	LENGTH_BUF	uart_buf;

	rt_uint32_t file_datalen;       /*�����ļ�����*/
	rt_uint8_t	file_matchcode[2];  /*�ļ�ƥ����*/

	rt_uint16_t packet_num;

	rt_uint16_t crc;

	msg = parameter;

	ptr_mem_packet = rt_malloc( READ_PACKET_SIZE + 20 );
	if( ptr_mem_packet == RT_NULL )
	{
		msg( "E�ڴ治��" );
		return;
	}
/*����U��*/
	while( 1 )
	{
		if( rt_device_find( "udisk" ) == RT_NULL ) /*û���ҵ�*/
		{
			count++;
			if( count <= 10 )
			{
				msg( "I�ȴ�U�̲���" );
			}else
			{
				msg( "EU�̲�����" ); /*ָʾU�̲�����*/
				goto end_upgrade_usb_0;
			}
			rt_thread_delay( RT_TICK_PER_SECOND );
		}else
		{
			msg( "I���������ļ�" );
			break;
		}
	}

/*����ָ���ļ�BEIDOU.IMG*/
	fd = open( "/udisk/BEIDOU.IMG", O_RDONLY, 0 );
	if( fd >= 0 )
	{
		msg( "I�����ļ�" );
	}else
	{
		msg( "E�����ļ�������" );
		goto end_upgrade_usb_0;
	}

	read( fd, ptr_mem_packet, 16 );
	pdata = ptr_mem_packet;
	if( ( *pdata != 0x54 ) || ( *( pdata + 1 ) != 0x44 ) )
	{
		msg( "E�ļ�ͷ����" );
		goto end_upgrade_usb_2;
	}

	ch_h	= ( *( pdata + 9 ) & 0xf0 ) >> 4;
	ch_l	= ( *( pdata + 9 ) & 0xf );
	sprintf( buf, "I�ļ��汾:%d.%d.%d", ch_h, ch_l, *( pdata + 10 ) );
	msg( buf );
/*�������ݳ���*/
	file_datalen	= 0;
	file_datalen	= ( *( pdata + 11 ) ) << 24;
	file_datalen	|= ( *( pdata + 12 ) ) << 16;
	file_datalen	|= ( *( pdata + 13 ) ) << 8;
	file_datalen	|= *( pdata + 14 );
	rt_kprintf( "file_datalen=%x", file_datalen );
/*�ļ�ƥ������β��*/
	count	= 0;
	ch_h	= 0;

	do
	{
		res = read( fd, ptr_mem_packet, READ_PACKET_SIZE );
		if( res )
		{
			count = res;
		}
	}
	while( res > 0 );

	if( ( ptr_mem_packet[count - 1] != 0x54 ) || ( ptr_mem_packet[count - 2] != 0x44 ) )
	{
		msg( "E�ļ�β����" );
		goto end_upgrade_usb_2;
	}
	file_matchcode[0]	= ptr_mem_packet[count - 6];
	file_matchcode[1]	= ptr_mem_packet[count - 5];
	rt_kprintf( "file datalen=%x matchcode=%02x%02x", file_datalen, file_matchcode[0], file_matchcode[1] );
	close( fd );

	msg( "I���ö˿�" );

	fd = open( "/udisk/BEIDOU.IMG", O_RDONLY, 0 );
	if( fd < 0 )
	{
		msg( "E�����ļ�������" );
		goto end_upgrade_usb_0;
	}

/*��ʼ����*/
/*����ֹͣgps�߳�*/
	while( rt_thread_suspend( &thread_gps ) != RT_EOK )
	{
		rt_kprintf( "S" );
		rt_thread_delay( 2 );
		count++;
		if( count > 200 )
		{
			msg( "E��ѯ�汾ʧ��" );
			//GPIO_SetBits( GPIOD, GPIO_Pin_10 );  /*��ģ��*/
			return;
		}
	}

#if 0
	GPIO_ResetBits( GPIOD, GPIO_Pin_10 );   /*off gps*/
	rt_thread_delay( RT_TICK_PER_SECOND * 2 );
	GPIO_SetBits( GPIOD, GPIO_Pin_10 );     /*on gps*/
	rt_thread_delay( RT_TICK_PER_SECOND * 2 );
#endif

/*��������״̬*/
	memcpy( buf, "\x40\x30\xC0\x00\x03\x00\x01\x34\x21\x0D\x0A", 11 );
	dev_gps_write( &dev_gps, 0, buf, 11 );
	rt_thread_delay( RT_TICK_PER_SECOND * 2 );
/*�汾��ѯ*/
	count = 0;

/*��������е����ݣ��ٷ���*/
	while( rt_mq_recv( &mq_gps, (void*)&uart_buf, NEMA_SIZE, 0 ) == RT_EOK )
	{
		rt_thread_delay( RT_TICK_PER_SECOND / 10 );
	}

	dev_gps_write( &dev_gps, 0, "\x40\x10\xC0\x00\x10\x00\x01\xC2\x84\x0D\x0A", 11 );
	res = rt_mq_recv( &mq_gps, (void*)&uart_buf, NEMA_SIZE, RT_TICK_PER_SECOND * 5 );
	if( res == RT_EOK )                                             //�յ�һ������
	{
		if( ( uart_buf.wr == 15 ) && ( uart_buf.body[4] == 0x02 ) ) /*��������״̬*/
		{
			ch_h	= ( uart_buf.body[7] & 0xf0 ) >> 4;
			ch_l	= ( uart_buf.body[7] & 0xf );
			sprintf( buf, "Iģ��汾:%d.%d.%d", ch_h, ch_l, uart_buf.body[8] );
			msg( buf );
		}
	}else /*��ʱ*/
	{
		msg( "E������������" );
		goto end_upgrade_usb_2;
	}

/*�Ĳ�����*/
	dev_gps_write( &dev_gps, 0, "\x40\x41\xC0\x00\x00\x00\xE1\x00\x00\x04\x07\xFC\x0D\x0A", 14 );
	gps_baud( 57600 );
	rt_thread_delay( RT_TICK_PER_SECOND );
/*���Ӳ�ѯ*/
	dev_gps_write( &dev_gps, 0, "\x40\x15\xC0\x00\x01\x00\x01\x92\xD4\x0D\x0A", 11 );
	while( 1 )
	{
		res = rt_mq_recv( &mq_gps, (void*)&uart_buf, NEMA_SIZE, RT_TICK_PER_SECOND * 5 );
		if( res == RT_EOK ) //�յ�һ������
		{
			if( ( uart_buf.wr == 11 ) && ( uart_buf.body[4] == 0x15 ) )
			{
				break;
			}
		}else /*��ʱ*/
		{
			msg( "E�޸Ĳ����ʴ���" );
			goto end_upgrade_usb_2;
		}
	}

/*��ʼ��д,��дָ��*/
	memcpy( buf, "\x40\xF2\xC0\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x0D\x0A", 18 );
	buf[6]	= file_datalen >> 24;
	buf[7]	= file_datalen >> 16;
	buf[8]	= file_datalen >> 8;
	buf[9]	= file_datalen & 0xff;
	buf[10] = file_matchcode[0];
	buf[11] = file_matchcode[1];

	crc		= CalcCRC16( buf, 1, 13, 0 );
	buf[14] = ( crc & 0xff00 ) >> 8;
	buf[15] = crc & 0xff;

	dev_gps_write( &dev_gps, 0, buf, 18 );

	ch_l = 1;
	while( ch_l )
	{
		res = rt_mq_recv( &mq_gps, (void*)&uart_buf, NEMA_SIZE, RT_TICK_PER_SECOND * 10 );
		if( res == RT_EOK ) //�յ�һ������
		{
			if( ( uart_buf.wr == 11 ) && ( uart_buf.body[4] == 0xf2 ) )
			{
				ch_l = 0;
			}
		}else /*��ʱ*/
		{
			msg( "E��ʼ��������" );
			goto end_upgrade_usb_2;
		}
	}

/*�������ݰ�*/
	count		= 0;        /*���㷢���ֽ���*/
	packet_num	= 0;
	read( fd, buf, 15 );    /*�����ļ�ͷ*/
//	rt_kprintf( "fd=%02x\r\n", fd );
	while( 1 )
	{
		rt_thread_delay( RT_TICK_PER_SECOND / 2 );
		memcpy( ptr_mem_packet, "\x40\xf2\x00\x00\x03\x01", 6 );
		res = read( fd, ptr_mem_packet + 6, READ_PACKET_SIZE );
		if( res < 0 )
		{
			msg( "E��ȡ�ļ�����" );
			goto end_upgrade_usb_2;
		}

		count += res;
		if( res == READ_PACKET_SIZE )           /*�ж��Ƿ�Ϊ���һ��*/
		{
			if( count == file_datalen )         /*������Ϊ1012������*/
			{
				ptr_mem_packet[2]	= 0x80 | ( packet_num >> 8 );
				ptr_mem_packet[3]	= packet_num & 0xff;
				res					= res - 6;
			}else
			{
				if( count == READ_PACKET_SIZE ) /*��һ��*/
				{
					ptr_mem_packet[2]	= 0x40 | ( packet_num >> 8 );
					ptr_mem_packet[3]	= packet_num & 0xff;
				}else
				{
					ptr_mem_packet[2]	= packet_num >> 8;
					ptr_mem_packet[3]	= packet_num & 0xff;
				}
			}
		}else
		{
			ptr_mem_packet[2]	= 0x80 | ( packet_num >> 8 );
			ptr_mem_packet[3]	= packet_num & 0xff;
			/*ȥ���ļ�β��������*/
			res = res - 6;
		}
		packet_num++;
		ptr_mem_packet[res + 6]		= ( res + 2 ) >> 8;
		ptr_mem_packet[res + 7]		= ( res + 2 ) & 0xff;
		crc							= CalcCRC16( ptr_mem_packet, 1, res + 7, 0 );
		ptr_mem_packet[res + 8]		= ( crc & 0xff00 ) >> 8;
		ptr_mem_packet[res + 9]		= crc & 0xff;
		ptr_mem_packet[res + 10]	= 0x0d;
		ptr_mem_packet[res + 11]	= 0x0a;

		dev_gps_write( &dev_gps, 0, ptr_mem_packet, res + 12 );

		ch_l = 1;
		while( ch_l )
		{
			res = rt_mq_recv( &mq_gps, (void*)&uart_buf, NEMA_SIZE, RT_TICK_PER_SECOND * 12 );
			if( res == RT_EOK ) //�յ�һ������
			{
				if( uart_buf.wr == 11 )
				{
					if( uart_buf.body[4] == 0x02 )
					{
						//dev_gps_write( &dev_gps, 0, "\x40\x34\xC0\x00\x34\x00\x01\x84\x6B\x0D\x0A", 11 );
						dev_gps_write( &dev_gps, 0, cmd_reset, 11 );
						msg( "E�������" );
						rt_kprintf( "\n�������" );
						goto end_upgrade_usb_2;
					}
					sprintf( buf, "I���͵�%d��", packet_num );
					rt_kprintf( "%d", packet_num % 10 );
					msg( buf );
					ch_l = 0;
				}
			}else /*��ʱ*/
			{
				msg( "E��������" );
				goto end_upgrade_usb_2;
			}
		}
	}

end_upgrade_usb_2:
	if( fd >= 0 )
	{
		close( fd );
	}
end_upgrade_usb_0:
	rt_free( ptr_mem_packet );
	ptr_mem_packet = RT_NULL;
	rt_thread_delay( RT_TICK_PER_SECOND * 3 );
	//GPIO_ResetBits( GPIOD, GPIO_Pin_10 );
	gps_baud( 9600 );
	while( rt_thread_resume( &thread_gps ) != RT_EOK )
	{
		rt_thread_delay( RT_TICK_PER_SECOND / 20 );
	}
	rt_kprintf( "\nbd>resume gps ok" );

	rt_thread_delay( RT_TICK_PER_SECOND * 1 );
	//GPIO_SetBits( GPIOD, GPIO_Pin_10 );
}

/*gps����*/
rt_err_t gps_upgrade( void )
{
	rt_thread_t tid;
	tid = rt_thread_create( "upgrade", thread_gps_upgrade_udisk, (void*)msg_uart_usb, 1024, 5, 5 );
	if( tid != RT_NULL )
	{
		rt_thread_startup( tid );
	}else
	{
		rt_kprintf( "\n Upgrade from usb fail\n" );
	}
}

FINSH_FUNCTION_EXPORT( gps_upgrade, upgrade bd_gps );

static char *reset_str[] = {
	"$CCSIR,1,0*49\r\n",    /*������ BD*/
	"$CCSIR,2,0*4A\r\n",    /*������ GPS*/
	"$CCSIR,3,0*4B\r\n",    /*������ GN*/
	"$CCSIR,1,1*48\r\n",    /*������ BD*/
	"$CCSIR,2,1*4B\r\n",    /*������ GP*/
	"$CCSIR,3,1*4A\r\n",    /*������ GN*/
};

/**/
rt_size_t gps_mode( uint8_t mode )
{
	//dev_gps_write( &dev_gps, 0, reset_str[mode - 1], 15 );
	switch( mode )
	{
		case 0:
			break;
		case 1: /*BD*/
			dev_gps_write( &dev_gps, 0, "$CCSIR,1,0*49\r\n", 15 );
			break;
		case 2: /*GPS*/
			dev_gps_write( &dev_gps, 0, "$CCSIR,2,0*4A\r\n", 15 );
			break;
		case 3: /*BDGPS*/
			dev_gps_write( &dev_gps, 0, "$CCSIR,3,0*4B\r\n", 15 );
			break;
	}
}

FINSH_FUNCTION_EXPORT( gps_mode, change mode );

/**/
rt_size_t gps_reset( uint8_t start_mode )
{
	dev_gps_write( &dev_gps, 0, reset_str[start_mode * 3 + gps_status.mode - 1], 15 );
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
void gps_write( char* s )
{
	dev_gps_write( &dev_gps, 0, s, strlen( s ) );
}

FINSH_FUNCTION_EXPORT( gps_write, write to gps );

/************************************** The End Of File **************************************/