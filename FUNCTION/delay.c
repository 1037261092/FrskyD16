#include "delay.h"

static uint8_t fac_us=0;

void delay_init(uint8_t sysclk)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;      
	fac_us = sysclk;
}


void delay_us(uint32_t nus)
{
	uint32_t ticks;
	uint32_t told = 0,tnow = 0,tcnt = 0;
	uint32_t reload = SysTick->LOAD;
	ticks = nus*fac_us;
	told = SysTick->VAL;
	//while(1)
	do{
		tnow = SysTick->VAL;
		if(tnow != told)
		{
			if(tnow < told)
			{
				tcnt += told - tnow;
			}
			else
			{
				tcnt += reload - tnow + told;
			}
			told = tnow;
		}
	}while(tcnt < ticks);
}


void delay_ms(uint16_t nms)
{
	uint32_t i;
	for(i=0 ; i < nms ; i++)
	{
		delay_us(1000);
	}
}
	

