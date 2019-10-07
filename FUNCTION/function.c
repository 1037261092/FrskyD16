#include "function.h"

void Get_ChipID(union ChipID *chipID)
{
	chipID->ChipUniqueID[0] = *(__IO uint32_t *)(0X1FFFF7AC);  // low byte
	chipID->ChipUniqueID[1] = *(__IO uint32_t *)(0X1FFFF7B8);  // 
	chipID->ChipUniqueID[2] = *(__IO uint32_t *)(0X1FFFF7B4);  // high byte
}

uint16_t GetUniqueID(void)
{
	uint16_t ID = 0 ; 
	union ChipID chipID;
	Get_ChipID(&chipID);
	ID = chipID.IDbyte[0] + chipID.IDbyte[2] + chipID.IDbyte[4] + chipID.IDbyte[6] + chipID.IDbyte[8] + chipID.IDbyte[10];
	ID = (ID << 8) + chipID.IDbyte[1] + chipID.IDbyte[3] + chipID.IDbyte[5] + chipID.IDbyte[7] + chipID.IDbyte[9] + chipID.IDbyte[11];
	return ID;
}



