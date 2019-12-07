/**
  ******************************************************************************
  * @file    Project/STM32F0xx_StdPeriph_Templates/main.c 
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    05-December-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
#include "main.h"
#include "spi.h"
#include "cc2500.h"
#include "delay.h"
#include "frsky_d16.h"
#include "sbus.h"
#include "timer.h"
#include "key.h"
#include "iwdog.h"
#include "adc.h"
#include "led.h"
#include "stdio.h"
#define GPIOA_1_Read()  GPIOA->IDR & GPIO_Pin_1
#define GPIOA_10_Read() GPIOA->IDR & GPIO_Pin_10
uint8_t Version_select_flag = 0,Low_power = 1;    //射频默认高功率
uint8_t RF_POWER;
void GPIOA_Pin_1_10_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA , ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

int main(void)
{
	delay_init(48);
	GPIOA_Pin_1_10_Init();
	if(GPIOA_1_Read())
	{
		Version_select_flag = FCC;
	}
	else
	{
		Version_select_flag = LBT;
	}
	
	if(GPIOA_10_Read())
	{
		RF_POWER = CC2500_POWER_17;
	}
	else
	{
		RF_POWER = CC2500_POWER_1;
	}
	initFRSKYD16();
	led_Init();
	sbus_init();
	key_init();
	WDG_Config();
	TIM3_Int_Init(8815,48);    // open timer interrupt 
	while (1)
	{
		
	}
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
