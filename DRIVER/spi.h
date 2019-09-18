#ifndef __SPI_H_
#define __SPI_H_

#include "stm32f0xx.h"
#include "delay.h"

//#define HARDWARE_SPI

#define SPI_MOSI_PORT         GPIOA
#define SPI_MISO_PORT         GPIOA
#define SPI_SCK_PORT          GPIOA
#define SPI_NSS_PORT          GPIOA

#define SPI_PIN_MOSI     GPIO_Pin_7
#define SPI_PIN_MISO     GPIO_Pin_6
#define SPI_PIN_SCK      GPIO_Pin_5
#define SPI_PIN_NSS      GPIO_Pin_4
 
#define SPI_MOSI_LOW	 (SPI_MOSI_PORT->BRR  = SPI_PIN_MOSI)
#define SPI_MOSI_HIGH    (SPI_MOSI_PORT->BSRR = SPI_PIN_MOSI)

#define SPI_MISO_LOW     (SPI_MISO_PORT->BRR  = SPI_PIN_MISO)
#define SPI_MISO_HIGH    (SPI_MISO_PORT->BSRR = SPI_PIN_MISO)

#define SPI_SCK_LOW      (SPI_SCK_PORT->BRR   = SPI_PIN_SCK)
#define SPI_SCK_HIGH     (SPI_SCK_PORT->BSRR  = SPI_PIN_SCK)

#define SPI_NSS_LOW  	 (SPI_NSS_PORT->BRR   = SPI_PIN_NSS)
#define SPI_NSS_HIGH 	 (SPI_NSS_PORT->BSRR  = SPI_PIN_NSS)



void spi_init(void);
void SPI_WriteByte(uint8_t TxData);
uint8_t SPI_ReadByte(void);
uint8_t SPI_Transfer(uint8_t data);

#endif
