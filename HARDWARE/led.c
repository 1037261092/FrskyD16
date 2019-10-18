#include "led.h"

void led_Init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef PORT_LED;
	PORT_LED.GPIO_Pin=GPIO_Pin_9;
	PORT_LED.GPIO_Mode=GPIO_Mode_OUT;
	PORT_LED.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Init(GPIOA,&PORT_LED);
}

void Led_On_Off(uint8_t status)
{
	if(0 == status)
	{
		LED_OFF;
	}
	else
	{
		LED_ON;
	}
}

