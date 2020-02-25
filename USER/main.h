/**
  ******************************************************************************
  * @file    Project/STM32F0xx_StdPeriph_Templates/main.h 
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    05-December-2014
  * @brief   Header for main.c module
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

#ifdef USE_STM320518_EVAL
 #include "stm320518_eval.h"
 #include "stm320518_eval_lcd.h"
#endif /* USE_STM320518_EVAL */

#ifdef USE_STM32072B_EVAL
 #include "stm32072b_eval.h"
 #include "stm32072b_eval_lcd.h"
#endif /* USE_STM32072B_EVAL */

/* Exported types ------------------------------------------------------------*/

//定义各通道名称(最大支持8通道)
typedef enum
{
  	RUDDER   	= 0 , 
	THROTTLE 	= 1 , 
	ELEVATOR 	= 2 , 
	AILERON  	= 3 , 
	AUX1  		= 4 , 
	AUX2		= 5 , 
	AUX3		= 6 , 
	AUX4		= 7 , 
	AUX5  		= 8 , 
	AUX6		= 9 , 
	AUX7		= 10 , 
	AUX8		= 11 , 
	AUX9  		= 12 , 
	AUX10		= 13 , 
	AUX11		= 14 , 
	AUX12		= 15 , 
}ChannelTypeDef ;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern void (*RF_Init)(void);
extern uint16_t (*RF_Process)(void);

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
