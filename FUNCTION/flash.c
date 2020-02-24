#include "flash.h"

uint8_t Flash_WaitBusy(void)
{
	uint16_t T = 1000;
	do{
		if(!(FLASH->SR & FLASH_SR_BSY))
			return 0;
	}while(--T);
	return 0xFF;
}


void Flash_LockControl(uint8_t Ltype)
{
	if(Ltype == 0){
		if(FLASH->CR&FLASH_CR_LOCK){
			FLASH->KEYR = 0x45670123;
			FLASH->KEYR = 0xCDEF89AB;
		}
	}
	else
	{
		FLASH->CR |= FLASH_CR_LOCK;
	}
}

uint8_t Flash_EreasePage(uint16_t PageIndex, uint16_t PageCount)
{
	uint8_t R;
	if(PageCount==0)
		return 0xFF;
	Flash_LockControl(0);                                   
	if((PageIndex == 0xFFFF) && (PageCount == 0xFFFF))      
	{
		FLASH->CR |= FLASH_CR_MER;                          
		FLASH->CR |= FLASH_CR_STRT;                         
		R = Flash_WaitBusy();                               
		if(!(FLASH->SR & FLASH_SR_EOP))
			R = 0xFF;
		FLASH->SR |= FLASH_SR_EOP;
		FLASH->CR=(~(FLASH_CR_STRT | FLASH_CR_MER));
		Flash_LockControl(1);                               
		return R;
	}
	while(PageCount--){
		FLASH->CR |= FLASH_CR_PER;                          
		FLASH->AR = (uint32_t)PageIndex * FLASH_PAGE_SIZE; 
		FLASH->CR |= FLASH_CR_STRT;                         
		R = Flash_WaitBusy();                              
		if(R!=0)break;
		if(!(FLASH->SR & FLASH_SR_EOP)) break;
		FLASH->SR |= FLASH_SR_EOP;
		PageIndex++;
		if(PageIndex >= FLASH_PAGE_COUNT)
			PageCount = 0;
	}
	FLASH->CR &= (~(FLASH_CR_STRT | FLASH_CR_PER));
	Flash_LockControl(1);
	return R;
}

uint8_t Flash_WriteDatas(uint32_t Addr,uint16_t *Buffer, uint16_t Length)
{
	uint8_t R = 0;
	uint16_t *FlashAddr = (uint16_t *)Addr;
	Flash_LockControl(0);  
    FLASH_ErasePage(Addr);	
	while(Length--){
		FLASH->CR |= FLASH_CR_PG;
		*FlashAddr++ = *Buffer++;              
		R = Flash_WaitBusy();                  
		if(R!=0)break;
		if(!(FLASH->SR & FLASH_SR_EOP))break;  
		FLASH->SR |= FLASH_SR_EOP;
	}
	Flash_LockControl(1);
	return R;
}


void FLASH_ReadDatas(uint32_t Addr,uint16_t *Buffer, uint16_t Length)
{
	uint16_t *FlashAddr = (uint16_t *)Addr;
	while(Length--)
		*Buffer++=*FlashAddr++;
}




