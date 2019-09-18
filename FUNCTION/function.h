#ifndef __FUNCTION_H_
#define __FUNCTION_H_

#include "stm32f0xx.h"

union ChipID{
	uint32_t ChipUniqueID[3];
	uint8_t IDbyte[12];
};

void Get_ChipID(union ChipID *chipID);
uint16_t GetUniqueID(void);
#endif
