#ifndef __SBUS_H_
#define __SBUS_H_

#include "stm32f0xx.h"

#define SERIAL_RX_PIN GPIO_Pin_3
#define SERIAL_RX_PORT GPIOA
#define SERIAL_BAUDRATE 100000
#define SBUS_INVERT 1

#define SERIAL_RX_SOURCE GPIO_PinSource3
#define SERIAL_RX_CHANNEL GPIO_AF_1

extern uint16_t Channel_DataBuff[16];

void sbus_init(void);
void sbus_checkrx(void);
#define DMARx

#ifdef  DMARx
#define SbusLength              25
#define RC_DMA_Rx_RCC           RCC_AHBPeriph_DMA1
#define RC_DMA_Rx_Ch 			DMA1_Channel3
#define RC_Dma_RxFlagTC			DMA1_FLAG_TC3
#endif

#endif
