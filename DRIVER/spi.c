#include "spi.h"

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
	
	SPI1_PINS_InitStruct.GPIO_Pin = SPI1_PINS_MOSI | SPI1_PINS_MISO | SPI1_PINS_SCK;
	SPI1_PINS_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	SPI1_PINS_InitStruct.GPIO_OType = GPIO_OType_PP;
	SPI1_PINS_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	SPI1_PINS_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(SPI1_PORTS,&SPI1_PINS_InitStruct);
	
	GPIO_PinAFConfig(SPI1_PORTS,GPIO_PinSource5, GPIO_AF_0);
	GPIO_PinAFConfig(SPI1_PORTS,GPIO_PinSource6, GPIO_AF_0);
	GPIO_PinAFConfig(SPI1_PORTS,GPIO_PinSource7, GPIO_AF_0);
	
	SPI1_PINS_InitStruct.GPIO_Pin = SPI1_PINS_NSS;
	SPI1_PINS_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	SPI1_PINS_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(SPI1_PORTS,&SPI1_PINS_InitStruct);
	
	SPI_I2S_DeInit(SPI1);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;    
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                         
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;   
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;    
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;   
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;    
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;   
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;    
    SPI_InitStructure.SPI_CRCPolynomial = 7;    
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_QF);    //重要，把应答数据位设置为 8 位
    SPI_Cmd(SPI1, ENABLE);
}

void SPI1_WriteByte(uint8_t TxData)
{
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_SendData8(SPI1, TxData);
}

uint8_t SPI1_ReadByte(void)
{
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);    
    return(SPI_ReceiveData8(SPI1));
}

uint8_t SPI1_Transfer(uint8_t data)
{
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_SendData8(SPI1, data);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_ReceiveData8(SPI1);
}
