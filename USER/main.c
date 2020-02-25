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
#include "frsky_d8.h"
#include "sbus.h"
#include "timer.h"
#include "key.h"
#include "iwdog.h"
#include "adc.h"
#include "led.h"
#include "stdio.h"
#include "flash.h"
#define GPIOA_1_Read()  GPIOA->IDR & GPIO_Pin_1
#define GPIOA_10_Read() GPIOA->IDR & GPIO_Pin_10
#define FLASH_ADDR 0x08007C00 
uint8_t Version_select_flag = 0,Low_power = 1;    //射频默认高功率
uint8_t RF_POWER;
uint16_t protocol_Index;

void (*RF_Init)(void);
uint16_t (*RF_Process)(void);

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
	key_init();
	led_Init();
	GPIOA_Pin_1_10_Init();
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1))
	{
		FLASH_ReadDatas(FLASH_ADDR,&protocol_Index,1);
	}
	else
	{
		FLASH_ReadDatas(FLASH_ADDR,&protocol_Index,1);
		protocol_Index = protocol_Index + 1;
		if(protocol_Index >= 3)
		{
			protocol_Index = 0;
		}
		Flash_WriteDatas(FLASH_ADDR,&protocol_Index,1);
	}
	for(uint8_t a=0; a<=protocol_Index; a++)
	{
		LED_ON;
		delay_ms(500);
		LED_OFF;
		delay_ms(500);
	}
	Version_select_flag = protocol_Index;
	switch(Version_select_flag)
	{
		case 0: RF_Init = initFRSKYD16;
				RF_Process = ReadFRSKYD16;
				break;
		case 1: RF_Init = initFRSKYD16;
				RF_Process = ReadFRSKYD16;
				break;
		case 2: RF_Init = initFRSKYD8;
				RF_Process = ReadFRSKYD8;
				break;
		default:
				break;
	}
	
	if(GPIOA_10_Read())
	{
		RF_POWER = CC2500_POWER_17;
	}
	else
	{
		RF_POWER = CC2500_POWER_1;
	}
	RF_Init();
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
