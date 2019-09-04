#include "delay.h"

static uint8_t fac_us=0;
static uint16_t fac_ms=0;

void delay_init(uint8_t sysclk)
{
	SysTick->CTRL &= 0xFFFFFFFb;
	fac_us = sysclk/8;
	fac_ms = (uint16_t)fac_us*1000;
}

void delay_ms(uint16_t ms)
{
	uint32_t temp;
	SysTick->LOAD = (uint32_t)ms*fac_ms;
	SysTick->VAL = 0x00;
	SysTick->CTRL = 0x01;
	do{
		temp = SysTick->CTRL;
	}while(temp&0x01 && !(temp&(1<<16)));
	SysTick->CTRL = 0x00;
	SysTick->VAL = 0x00;
}
	
void delay_ns(uint32_t ns)
{
	uint32_t temp;
	SysTick->LOAD = (uint32_t)ns*fac_us;
	SysTick->VAL = 0x00;
	SysTick->CTRL = 0x01;
	do{
		temp = SysTick->CTRL;
	}while(temp&0x01 && !(temp&(1<<16)));
	SysTick->CTRL = 0x00;
	SysTick->VAL = 0x00;
}
