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

#include <finsh.h>
#include "sst25.h"
#include "jt808.h"

#define DF_CS_0 GPIO_ResetBits( GPIOD, GPIO_Pin_14 )
#define DF_CS_1 GPIO_SetBits( GPIOD, GPIO_Pin_14 )

#define DF_WP_1 GPIO_SetBits( GPIOD, GPIO_Pin_15 )
#define DF_WP_0 GPIO_SetBits( GPIOD, GPIO_Pin_15 )

#define DBSY 0X80

#define SST25_AAI		0xAD
#define SST25_BP		0x02
#define SST25_RDSR		0x05
#define SST25_WREN		0x06    //Write Enable
#define SST25_WRDI		0x04
#define SectorErace_4KB 0x20    //扇区擦除
#define SST25_READ		0x03

//uint8_t	mem_Manufacturer_ID = 0;		//存储芯片的厂家代码,SST(0xBF),WINBOND(0xEF)
//uint8_t	mem_Device_ID = 0;				//存储芯片的厂家代码,SST25VF032(0x4A),W25Q64FV(0x16)

uint16_t	mem_ID = 0;				//存储芯片的厂家代码,SST25VF032(0xBF4A),W25Q64FV(0xEF16)

struct rt_semaphore sem_dataflash;

static void delay(uint32_t delay_count)
{
	while(delay_count)
		{
		delay_count--;
		}
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
uint8_t spi_tx( uint8_t data )
{
	while( SPI_I2S_GetFlagStatus( SPI2, SPI_I2S_FLAG_TXE ) == RESET )
	{
		;
	}
	SPI_I2S_SendData( SPI2, data );
	while( SPI_I2S_GetFlagStatus( SPI2, SPI_I2S_FLAG_RXNE ) == RESET )
	{
		;
	}
	return SPI_I2S_ReceiveData( SPI2 );
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
static uint8_t sst25_info( void )
{
	//uint8_t n;
	DF_CS_0;
	spi_tx( 0x90 );
	spi_tx( 0x00 );
	spi_tx( 0x00 );
	spi_tx( 0x00 );
	mem_ID = spi_tx( 0xff )<<8;
	mem_ID |= spi_tx( 0xff );
	DF_CS_1;
	//rt_kprintf("\n MEM_ID=%04X",mem_ID);
}

#if 0

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void sst25_init( void )
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	SPI_InitTypeDef		SPI_InitStructure;

	/* Enable DF_SPI Periph clock */
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE );
	GPIO_PinAFConfig( GPIOB, GPIO_PinSource13, GPIO_AF_SPI2 );
	GPIO_PinAFConfig( GPIOB, GPIO_PinSource14, GPIO_AF_SPI2 );
	GPIO_PinAFConfig( GPIOB, GPIO_PinSource15, GPIO_AF_SPI2 );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD, ENABLE );

	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_DOWN;

	/*!< SPI SCK pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init( GPIOB, &GPIO_InitStructure );

	/*!< SPI MOSI pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init( GPIOB, &GPIO_InitStructure );

	/*!< SPI MISO pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init( GPIOB, &GPIO_InitStructure );

	//-------  CS _pin
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init( GPIOD, &GPIO_InitStructure );
	
	//PA4 pin: CS		SD_CARD
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init( GPIOA, &GPIO_InitStructure );
	GPIO_SetBits(GPIOA,GPIO_Pin_4);

	//-------  WP  _pin
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init( GPIOD, &GPIO_InitStructure );

	/*------------------------ DF_SPI configuration ------------------------*/
	SPI_InitStructure.SPI_Direction			= SPI_Direction_2Lines_FullDuplex;  //SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_Mode				= SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize			= SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL				= SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA				= SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS				= SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;          /* 42M/4=10.5M */
	SPI_InitStructure.SPI_FirstBit			= SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial		= 7;

	//SPI1->CR2=0x04;                                   //NSS ---SSOE
	//   SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	//SPI_InitStructure.SPI_Mode = SPI_Mode_Master;       //MSTR

	SPI_I2S_DeInit( SPI2 );
	SPI_Init( SPI2, &SPI_InitStructure );

	/* Enable SPI_MASTER */
	SPI_Cmd( SPI2, ENABLE );
	SPI_CalculateCRC( SPI2, DISABLE );

	DF_CS_1;
	DF_WP_0;

	DF_CS_0;
	spi_tx( 0x50 );
	DF_CS_1;
	delay(3);
	DF_CS_0;
	spi_tx( 0x01 );
	spi_tx( 0x00 );
	DF_CS_1;
	delay(3);
	DF_CS_0;
	spi_tx( DBSY );
	DF_CS_1;
	//sst25_info( );
	//rt_sem_init( &sem_dataflash, "sem_df", 0, RT_IPC_FLAG_FIFO );
	//rt_sem_release( &sem_dataflash );
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
static uint8_t sst25_busy( void )
{
	uint8_t st;
	DF_CS_0;
	spi_tx( 0x05 );
	st = spi_tx( 0xff );
	DF_CS_1;
	return st & 0x01;
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
void sst25_read( uint32_t addr, uint8_t *p, uint16_t len )
{
	uint8_t		*pdata		= p;
	uint32_t	readaddr	= addr;

	while( sst25_busy( ) )
	{
		;
	}
	DF_CS_0;
	spi_tx( SST25_READ );
	spi_tx( ( readaddr & 0xFF0000 ) >> 16 );
	spi_tx( ( readaddr & 0xFF00 ) >> 8 );
	spi_tx( readaddr & 0xFF );
	while( len-- )
	{
		*pdata = spi_tx( 0xA5 );
		pdata++;
	}
	DF_CS_1;
}

/*搽除一个扇区，不需要连续删除*/
void sst25_erase_4k( uint32_t addr )
{
	while( sst25_busy( ) )
	{
		;
	}
	DF_CS_0;
	spi_tx( SST25_WREN );
	DF_CS_1;
	DF_CS_0;
	spi_tx( SectorErace_4KB );
	spi_tx( ( addr & 0xffFFFF ) >> 16 );
	spi_tx( ( addr & 0xffFF ) >> 8 );
	spi_tx( addr & 0xFF );
	DF_CS_1;
	while( sst25_busy( ) )
	{
		;
	}
}

FINSH_FUNCTION_EXPORT( sst25_erase_4k, erase );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/

static uint8_t buf[4096];


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void sst25_bytewrite( uint32_t addr, uint8_t data )
{
	volatile uint8_t st;
	DF_CS_0;
	spi_tx( SST25_WREN );
	DF_CS_1;

	DF_CS_0;
	spi_tx( SST25_BP );
	spi_tx( ( addr & 0xffffff ) >> 16 );
	spi_tx( ( addr & 0xff00 ) >> 8 );
	spi_tx( addr & 0xff );
	spi_tx( data );
	DF_CS_1;
	while( sst25_busy( ) )
	{
		;
	}
#if 0
	DF_CS_0;
	spi_tx( SST25_RDSR );
	do
	{
		st = spi_tx( 0xA5 );
	}
	while( st & 0x01 );
	DF_CS_1;
#endif
}

/*
   不判断是否要删除，直接写
   避免跨4k写
 */

void sst25_write_through( uint32_t addr, uint8_t *p, uint16_t len )
{
	volatile uint8_t	st = 0;
	uint32_t			wr_addr;
	uint8_t				*pdata	= p;
	uint16_t			remain	= len;
	wr_addr = addr;
	while( sst25_busy( ) )
	{
		;
	}
/*字节写的方式*/
#if 0
	while( remain-- )
	{
		sst25_bytewrite( wr_addr, *pdata++ );
		wr_addr++;
	}
#else
/*AAI方式写,要判读地址的奇偶*/
	if( len == 1 )
	{
		sst25_bytewrite( wr_addr, *pdata );
		return;
	}
	if( wr_addr & 0x01 ) /*AAI需要从A0=0开始*/
	{
		sst25_bytewrite( wr_addr++, *pdata++ );
		remain--;
	}

	DF_CS_0;
	spi_tx( SST25_WREN );
	DF_CS_1;
	DF_CS_0;
	spi_tx( SST25_AAI ); /**/
	spi_tx( wr_addr >> 16 );
	spi_tx( wr_addr >> 8 );
	spi_tx( wr_addr & 0xff );
	spi_tx( *pdata++ );
	spi_tx( *pdata++ );
	DF_CS_1;
	while( sst25_busy( ) )
	{
		;                       //判忙
	}
	remain -= 2;
	while( remain > 1 )
	{
		DF_CS_0;
		spi_tx( SST25_AAI );    /**/
		spi_tx( *pdata++ );
		spi_tx( *pdata++ );
		DF_CS_1;
		while( sst25_busy( ) )
		{
			;                   //判忙
		}
		remain -= 2;
	}

	DF_CS_0;
	spi_tx( SST25_WRDI );       //WRDI用于退出AAI写模式 所谓AAI 就是地址自动加
	DF_CS_1;
	if( remain )
	{
		sst25_bytewrite( wr_addr + len - 1, *pdata );
	}
#endif
}

/*
   回写，读出数据，
   限制写入大小，不能跨多个扇区
 */

void sst25_write_back( uint32_t addr, uint8_t *p, uint16_t len )
{
	uint32_t	i, wr_addr;
	uint16_t	offset;
	uint8_t		*pbuf;


/*找到当前地址对应的4k=0x1000边界*/
	wr_addr = addr & 0xFFFFF000;        /*对齐到4k边界*/
	offset	= addr & 0xfff;             /*4k内偏移*/

	sst25_read( wr_addr, buf, 4096 );   /*读出数据*/
	sst25_erase_4k( wr_addr );          /*清除扇区*/

	memcpy( buf + offset, p, len );
	pbuf = buf;
	DF_CS_0;
	spi_tx( SST25_WREN );
	DF_CS_1;
	DF_CS_0;
	spi_tx( SST25_AAI ); /**/
	spi_tx( wr_addr >> 16 );
	spi_tx( wr_addr >> 8 );
	spi_tx( wr_addr & 0xff );
	spi_tx( *pbuf++ );
	spi_tx( *pbuf++ );
	DF_CS_1;
	while( sst25_busy( ) )
	{
		;                    
	}
	for( i = 0; i < 4094; i += 2 )
	{
		DF_CS_0;
		spi_tx( SST25_AAI );    /**/
		spi_tx( *pbuf++ );
		spi_tx( *pbuf++ );
		DF_CS_1;
		while( sst25_busy( ) )
		{
			;                
		}
	}
	DF_CS_0;
	spi_tx( SST25_WRDI );       //WRDI用于退出AAI写模式 所谓AAI 就是地址自动加
	DF_CS_1;
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
int df_read( uint32_t addr, uint16_t size )
{
	uint8_t		tbl[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	int			i, count = 0;
	uint8_t		c;
	uint8_t		*p		= RT_NULL;
	uint8_t		*pdata	= RT_NULL;
	uint8_t		printbuf[100];
	int32_t		len = size;
	uint32_t	pos = addr;

	p = rt_malloc( len );
	if( p == RT_NULL )
	{
		return 0;
	}
	pdata = p;
	sst25_read( addr, pdata, len );

	while( len > 0 )
	{
		count = ( len < 16 ) ? len : 16;
		memset( printbuf, 0x20, 70 );
		for( i = 0; i < count; i++ )
		{
			c					= *pdata;
			printbuf[i * 3]		= tbl[c >> 4];
			printbuf[i * 3 + 1] = tbl[c & 0x0f];
			if(( c < 0x20 )||(c==0xFF))
			{
				c = '.';
			}
			/*
			if( c > 0x7f )
			{
				c = '.';
			}
			*/
			printbuf[50 + i] = c;
			pdata++;
		}
		printbuf[69] = 0;
		rt_kprintf( "%08x %s\r\n", pos, printbuf );
		len -= count;
		pos += count;
	}
	if( p != RT_NULL )
	{
		rt_free( p );
		p = RT_NULL;
	}
	return size;
}

FINSH_FUNCTION_EXPORT( df_read, read from serial flash );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void df_format( uint16_t start_addr, uint16_t sectors )
{
	uint16_t	i;
	uint32_t	addr = start_addr;
	rt_sem_take( &sem_dataflash, RT_TICK_PER_SECOND * FLASH_SEM_DELAY );
	for( i = 0; i < sectors; i++ )
	{
		sst25_erase_4k( addr );
		addr += 4096;
	}
	rt_sem_release( &sem_dataflash );
}

FINSH_FUNCTION_EXPORT( df_format, format serial flash );
#else

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void sst25_init( void )
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	SPI_InitTypeDef		SPI_InitStructure;
	uint32_t			delay_counter;

	/* Enable DF_SPI Periph clock */
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE );
	GPIO_PinAFConfig( GPIOB, GPIO_PinSource13, GPIO_AF_SPI2 );
	GPIO_PinAFConfig( GPIOB, GPIO_PinSource14, GPIO_AF_SPI2 );
	GPIO_PinAFConfig( GPIOB, GPIO_PinSource15, GPIO_AF_SPI2 );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD, ENABLE );

	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_DOWN;

	/*!< SPI SCK pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init( GPIOB, &GPIO_InitStructure );

	/*!< SPI MOSI pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init( GPIOB, &GPIO_InitStructure );

	/*!< SPI MISO pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init( GPIOB, &GPIO_InitStructure );

	//-------  CS _pin
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init( GPIOD, &GPIO_InitStructure );
	
	//PA4 pin: CS		SD_CARD
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init( GPIOA, &GPIO_InitStructure );
	GPIO_SetBits(GPIOA,GPIO_Pin_4);

	//-------  WP  _pin
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init( GPIOD, &GPIO_InitStructure );

	/*------------------------ DF_SPI configuration ------------------------*/
	SPI_InitStructure.SPI_Direction			= SPI_Direction_2Lines_FullDuplex;  //SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_Mode				= SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize			= SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL				= SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA				= SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS				= SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;          /* 42M/4=10.5M */
	SPI_InitStructure.SPI_FirstBit			= SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial		= 7;

	//SPI1->CR2=0x04;                                   //NSS ---SSOE
	//   SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	//SPI_InitStructure.SPI_Mode = SPI_Mode_Master;       //MSTR

	SPI_I2S_DeInit( SPI2 );
	SPI_Init( SPI2, &SPI_InitStructure );

	/* Enable SPI_MASTER */
	SPI_Cmd( SPI2, ENABLE );
	SPI_CalculateCRC( SPI2, DISABLE );

	DF_CS_1;
	DF_WP_0;

	DF_CS_0;
	spi_tx( 0x50 );
	DF_CS_1;
	delay(3);
	DF_CS_0;
	spi_tx( 0x01 );
	spi_tx( 0x00 );
	DF_CS_1;

	//delay(3);
	//DF_CS_0;
	//spi_tx( DBSY );
	//DF_CS_1;
	sst25_info( );
	//rt_sem_init( &sem_dataflash, "sem_df", 0, RT_IPC_FLAG_FIFO );
	//rt_sem_release( &sem_dataflash );
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
static uint8_t sst25_busy( void )
{
	uint8_t st;
	DF_CS_0;
	spi_tx( 0x05 );
	st = spi_tx( 0xff );
	DF_CS_1;
	return st & 0x01;
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
void sst25_read( uint32_t addr, uint8_t *p, uint16_t len )
{
	uint8_t		*pdata		= p;
	uint32_t	readaddr	= addr;

	while( sst25_busy( ) )
	{
		;
	}
	DF_CS_0;
	spi_tx( SST25_READ );
	spi_tx( ( readaddr & 0xFF0000 ) >> 16 );
	spi_tx( ( readaddr & 0xFF00 ) >> 8 );
	spi_tx( readaddr & 0xFF );
	while( len-- )
	{
		*pdata = spi_tx( 0xA5 );
		pdata++;
	}
	DF_CS_1;
}

/*搽除一个扇区，不需要连续删除*/
void sst25_erase_4k( uint32_t addr )
{
	while( sst25_busy( ) )
	{
		;
	}
	DF_CS_0;
	spi_tx( SST25_WREN );
	DF_CS_1;
	while( sst25_busy( ) )
	{
		;
	}
	DF_CS_0;
	spi_tx( SectorErace_4KB );
	spi_tx( ( addr & 0xffFFFF ) >> 16 );
	spi_tx( ( addr & 0xffFF ) >> 8 );
	spi_tx( addr & 0xFF );
	DF_CS_1;
	while( sst25_busy( ) )
	{
		;
	}
}

FINSH_FUNCTION_EXPORT( sst25_erase_4k, erase );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/

static uint8_t buf[4096];


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void sst25_bytewrite( uint32_t addr, uint8_t data )
{
	volatile uint8_t st;
	DF_CS_0;
	spi_tx( SST25_WREN );
	DF_CS_1;
	delay(3);
	DF_CS_0;
	spi_tx( SST25_BP );
	spi_tx( ( addr & 0xffffff ) >> 16 );
	spi_tx( ( addr & 0xff00 ) >> 8 );
	spi_tx( addr & 0xff );
	spi_tx( data );
	DF_CS_1;
	while( sst25_busy( ) )
	{
		;
	}
#if 0
	DF_CS_0;
	spi_tx( SST25_RDSR );
	do
	{
		st = spi_tx( 0xA5 );
	}
	while( st & 0x01 );
	DF_CS_1;
#endif
}

/*
   不判断是否要删除，直接写
   避免跨4k写
 */

void sst25_write_through( uint32_t addr, uint8_t *p, uint16_t len )
{
	volatile uint8_t	st = 0;
	uint32_t			wr_addr;
	uint8_t				*pdata	= p;
	uint16_t			remain	= len;
	uint32_t			delay_counter;
	wr_addr = addr;
	while( sst25_busy( ) )
	{
		;
	}
	if(mem_ID == 0xBF4A)		///存储设备为sst25
	{
		/*AAI方式写,要判读地址的奇偶*/
		if( len == 1 )
		{
			sst25_bytewrite( wr_addr, *pdata );
			return;
		}
		if( wr_addr & 0x01 ) /*AAI需要从A0=0开始*/
		{
			sst25_bytewrite( wr_addr++, *pdata++ );
			remain--;
		}

		DF_CS_0;
		spi_tx( SST25_WREN );
		DF_CS_1;
		delay(3);
		DF_CS_0;
		spi_tx( SST25_AAI ); /**/
		spi_tx( wr_addr >> 16 );
		spi_tx( wr_addr >> 8 );
		spi_tx( wr_addr & 0xff );
		spi_tx( *pdata++ );
		spi_tx( *pdata++ );
		DF_CS_1;
		while( sst25_busy( ) )
		{
			;                       //判忙
		}
		remain -= 2;
		while( remain > 1 )
		{
			DF_CS_0;
			spi_tx( SST25_AAI );    /**/
			spi_tx( *pdata++ );
			spi_tx( *pdata++ );
			DF_CS_1;
			while( sst25_busy( ) )
			{
				;                   //判忙
			}
			remain -= 2;
		}

		DF_CS_0;
		spi_tx( SST25_WRDI );       //WRDI用于退出AAI写模式 所谓AAI 就是地址自动加
		DF_CS_1;
		if( remain )
		{
			sst25_bytewrite( wr_addr + len - 1, *pdata );
		}
	}
	else
	{
		DF_CS_0;
		spi_tx( SST25_WREN );
		DF_CS_1;
		while( sst25_busy( ) )
		{
			;
		}
		DF_CS_0;
		spi_tx( SST25_BP ); /**/
		spi_tx( ( wr_addr & 0xffffff ) >> 16 );
		spi_tx( ( wr_addr & 0xff00 ) >> 8 );
		spi_tx( wr_addr & 0xff );
		while( remain )
		{
			wr_addr++;
			remain--;
			spi_tx( *pdata++ );
			if(remain && ((wr_addr & 0xFF) == 0))
			{
				//delay_counter = 100;
				DF_CS_1;
				while( sst25_busy( ) )
				{
					;
				}
				DF_CS_0;
				spi_tx( SST25_WREN );
				DF_CS_1;
				while( sst25_busy( ) )
				{
					;
				}
				DF_CS_0;
				spi_tx( SST25_BP ); /**/
				spi_tx( ( wr_addr & 0xffffff ) >> 16 );
				spi_tx( ( wr_addr & 0xff00 ) >> 8 );
				spi_tx( wr_addr & 0xff );
			}
		}
		DF_CS_1;
		while( sst25_busy( ) )
		{
			;
		}
	}
}

/*
   回写，读出数据，
   限制写入大小，不能跨多个扇区
 */

void sst25_write_back( uint32_t addr, uint8_t *p, uint16_t len )
{
	uint32_t	i, wr_addr;
	uint16_t	offset;
	uint8_t		*pbuf;


/*找到当前地址对应的4k=0x1000边界*/
	wr_addr = addr & 0xFFFFF000;        /*对齐到4k边界*/
	offset	= addr & 0xfff;             /*4k内偏移*/

	sst25_read( wr_addr, buf, 4096 );   /*读出数据*/
	sst25_erase_4k( wr_addr );          /*清除扇区*/

	memcpy( buf + offset, p, len );
	pbuf = buf;
	sst25_write_through(wr_addr,pbuf,4096);
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
int df_read( uint32_t addr, uint16_t size )
{
	uint8_t		tbl[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	int			i, count = 0;
	uint8_t		c;
	uint8_t		*p		= RT_NULL;
	uint8_t		*pdata	= RT_NULL;
	uint8_t		printbuf[100];
	int32_t		len = size;
	uint32_t	pos = addr;

	p = rt_malloc( len );
	if( p == RT_NULL )
	{
		return 0;
	}
	pdata = p;
	sst25_read( addr, pdata, len );

	while( len > 0 )
	{
		count = ( len < 16 ) ? len : 16;
		memset( printbuf, 0x20, 70 );
		for( i = 0; i < count; i++ )
		{
			c					= *pdata;
			printbuf[i * 3]		= tbl[c >> 4];
			printbuf[i * 3 + 1] = tbl[c & 0x0f];
			if(( c < 0x20 )||(c==0xFF))
			{
				c = '.';
			}
			/*
			if( c > 0x7f )
			{
				c = '.';
			}
			*/
			printbuf[50 + i] = c;
			pdata++;
		}
		printbuf[69] = 0;
		rt_kprintf( "%08x %s\r\n", pos, printbuf );
		len -= count;
		pos += count;
	}
	if( p != RT_NULL )
	{
		rt_free( p );
		p = RT_NULL;
	}
	return size;
}

FINSH_FUNCTION_EXPORT( df_read, read from serial flash );

void df_write( uint32_t addr, char* data )
{
 sst25_write_through(addr,data,strlen(data));
}	
FINSH_FUNCTION_EXPORT( df_write, write from serial flash );
/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void df_format( uint16_t start_addr, uint16_t sectors )
{
	uint16_t	i;
	uint32_t	addr = start_addr;
	rt_sem_take( &sem_dataflash, RT_TICK_PER_SECOND * FLASH_SEM_DELAY );
	for( i = 0; i < sectors; i++ )
	{
		sst25_erase_4k( addr );
		addr += 4096;
	}
	rt_sem_release( &sem_dataflash );
}

FINSH_FUNCTION_EXPORT( df_format, format serial flash );
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////

