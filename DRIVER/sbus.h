#ifndef __SBUS_H_
#define __SBUS_H_

#include "stm32f0xx.h"

#define SERIAL_RX_PIN GPIO_Pin_3
#define SERIAL_RX_PORT GPIOA
#define SERIAL_BAUDRATE 100000
#define SBUS_INVERT 1

#define SERIAL_RX_SOURCE GPIO_PinSource3
#define SERIAL_RX_CHANNEL GPIO_AF_1

void sbus_init(void);
void sbus_checkrx(void);
#endif
