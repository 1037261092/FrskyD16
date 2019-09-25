#include "iwdog.h"



void WDG_Config(void)
{
	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
	{
		RCC_ClearFlag();            //clear flag
	}
	
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);    
	IWDG_SetPrescaler(IWDG_Prescaler_4);
	
	IWDG_SetReload(1000);             //100ms feed dogs
	IWDG_ReloadCounter();
	
	IWDG_Enable();
}
