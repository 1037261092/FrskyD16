#ifndef __SPI_H_
#define __SPI_H_

#include "stm32f0xx.h"
#include "delay.h"

#define SPI1_PORTS         GPIOA
#define SPI1_PINS_MOSI     GPIO_Pin_7
#define SPI1_PINS_MISO     GPIO_Pin_6
#define SPI1_PINS_SCK      GPIO_Pin_5
#define SPI1_PINS_NSS      GPIO_Pin_4

#define SPI1_NSS_LOW  (SPI1_PORTS->BRR  = SPI1_PINS_NSS)
#define SPI1_NSS_HIGH (SPI1_PORTS->BSRR = SPI1_PINS_NSS)

void spi_init(void);
void SPI1_WriteByte(uint8_t TxData);
void SPI1_WriteBytes(uint8_t *TxData,uint8_t length);
uint8_t SPI1_ReadByte(void);
uint8_t SPI1_Transfer(uint8_t data);

#endif
