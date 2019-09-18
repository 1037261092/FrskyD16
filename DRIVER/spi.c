#include "spi.h"

#ifdef HARDWARE_SPI
void spi_init(void)
{
	//SPI1_MOSI ----> PA7
	//SPI1_MISO ----> PA6
	//SPI1_SCK  ----> PA5
	//SPI1_NSS  ----> PA4
	GPIO_InitTypeDef SPI1_PINS_InitStruct;
	SPI_InitTypeDef   SPI_InitStructure;

	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 , ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA , ENABLE);
	
	SPI1_PINS_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	SPI1_PINS_InitStruct.GPIO_OType = GPIO_OType_PP;
	SPI1_PINS_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	SPI1_PINS_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	SPI1_PINS_InitStruct.GPIO_Pin = SPI_PIN_MOSI;
	GPIO_Init(SPI_MOSI_PORT,&SPI1_PINS_InitStruct);
	
	SPI1_PINS_InitStruct.GPIO_Pin = SPI_PIN_MISO;
	GPIO_Init(SPI_MISO_PORT,&SPI1_PINS_InitStruct);
	
	SPI1_PINS_InitStruct.GPIO_Pin = SPI_PIN_SCK;
	GPIO_Init(SPI_SCK_PORT,&SPI1_PINS_InitStruct);
	
	GPIO_PinAFConfig(SPI_SCK_PORT,GPIO_PinSource5, GPIO_AF_0);
	GPIO_PinAFConfig(SPI_MISO_PORT,GPIO_PinSource6, GPIO_AF_0);
	GPIO_PinAFConfig(SPI_MOSI_PORT,GPIO_PinSource7, GPIO_AF_0);
	
	SPI1_PINS_InitStruct.GPIO_Pin = SPI_PIN_NSS;
	SPI1_PINS_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	SPI1_PINS_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(SPI_NSS_PORT,&SPI1_PINS_InitStruct);
	
	SPI_I2S_DeInit(SPI1);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;    
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                         
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;   
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;    
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;   
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;    
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;   
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;    
    SPI_InitStructure.SPI_CRCPolynomial = 7;    
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_QF);    //重要，把应答数据位设置为 8 位
    SPI_Cmd(SPI1, ENABLE);
}

void SPI_WriteByte(uint8_t TxData)
{
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);	
    SPI_SendData8(SPI1, TxData);
}

uint8_t SPI_ReadByte(void)
{
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);    
    return(SPI_ReceiveData8(SPI1));
}

uint8_t SPI_Transfer(uint8_t data)
{
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_SendData8(SPI1, data);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_ReceiveData8(SPI1);
}

#else
void spi_init(void)
{
	//SPI1_MOSI ----> PA7
	//SPI1_MISO ----> PA6
	//SPI1_SCK  ----> PA5
	//SPI1_NSS  ----> PA4
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA , ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = SPI_PIN_MISO;
	GPIO_Init(SPI_MISO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = SPI_PIN_MOSI;
	GPIO_Init(SPI_MOSI_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = SPI_PIN_SCK;
	GPIO_Init(SPI_SCK_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = SPI_PIN_NSS;
	GPIO_Init(SPI_NSS_PORT, &GPIO_InitStructure);
}

void SPI_WriteByte(uint8_t data)
{
	for(int i=7 ; i >= 0;i--)
	{
		if((data>>i)&1)
		{
			SPI_MOSI_HIGH;
		}
		else
		{
			SPI_MOSI_LOW;
		}
		SPI_SCK_HIGH;
		delay_us(1);
		SPI_SCK_LOW;
		delay_us(1);
		
	}
}

uint8_t SPI_ReadByte(void)
{
	int recv = 0;
	for ( int i = 7 ; i >=0 ; i--)
	{						
		SPI_SCK_HIGH;
		delay_us(1);
		recv = recv<<1;
        recv = recv|((SPI_MISO_PORT->IDR & (int)SPI_PIN_MISO)?1:0);        
		SPI_SCK_LOW;
		delay_us(1);		
	}	
    return recv;
}

uint8_t SPI_Transfer(uint8_t data)
{
	
    SPI_WriteByte(data);
	delay_us(1);
    return SPI_ReadByte();
}
#endif

