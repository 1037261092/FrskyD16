#ifndef __LED_H_
#define __LED_H_

#include "stm32f0xx.h"

#define LED_PORT     GPIOA
#define LED_PIN      GPIO_Pin_9
 
#define LED_OFF	  (LED_PORT->BRR  = GPIO_Pin_9)
#define LED_ON    (LED_PORT->BSRR = GPIO_Pin_9)

void led_Init(void);
void Led_On_Off(uint8_t status);

#endif

