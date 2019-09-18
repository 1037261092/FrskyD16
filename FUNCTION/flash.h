#ifndef __FLASH_H_
#define __FLASH_H_

#include "stm32f0xx.h"

#define FLASH_ADDR_STATRT             0x8000000              //flash start address
#define FLASH_PAGE_SIZE               1024        //page size is 400U (1024byte)
#define FLASH_PAGE_COUNT              16

#define PAGE_0_START_ADRESS         0x08000000
#define PAGE_1_START_ADRESS         0x08000400
#define PAGE_2_START_ADRESS         0x08000800
#define PAGE_3_START_ADRESS			0x08000C00
#define PAGE_4_START_ADRESS			0x08001000
#define PAGE_5_START_ADRESS			0x08001400
#define PAGE_6_START_ADRESS			0x08001800
#define PAGE_7_START_ADRESS			0x08001C00
#define PAGE_8_START_ADRESS			0x08002000
#define PAGE_9_START_ADRESS			0x08002400
#define PAGE_10_START_ADRESS		0x08002800
#define PAGE_11_START_ADRESS		0x08002C00
#define PAGE_12_START_ADRESS		0x08003000
#define PAGE_13_START_ADRESS		0x08003400
#define PAGE_14_START_ADRESS		0x08003800
#define PAGE_15_START_ADRESS		0x08003C00



uint8_t Flash_WaitBusy(void);
void Flash_LockControl(uint8_t Ltype);
uint8_t Flash_EreasePage(uint16_t PageIndex, uint16_t PageCount);
uint8_t Flash_WriteDatas(uint32_t addr,uint16_t *Buffer, uint16_t Length);
void FLASH_ReadDatas(uint32_t addr,uint16_t *Buffer, uint16_t Length);

#endif
