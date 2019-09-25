#include "iwdog.h"



void WDG_Config(void)
{
	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
	{
		RCC_ClearFlag();
	}
	
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_32);
	
	IWDG_SetReload(40000/64);
	IWDG_ReloadCounter();
	
	IWDG_Enable();
}
