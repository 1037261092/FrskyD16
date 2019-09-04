#include "cc2500.h"
#include "spi.h"

uint8_t CC2500_Init(void)
{
	return 0;
}

void CC2500_SetPower(uint8_t power)
{

}

void CC2500_Strobe(uint8_t state)
{


}

void CC2500_WriteReg(uint8_t address, uint8_t data)
{

	
}

uint8_t CC2500_ReadReg(uint8_t address)
{

	return 0;
}

void CC2500_SetTxRxMode(uint8_t mode)
{

}

void CC2500_WriteData(uint8_t *dpbuffer, uint8_t len)
{

}

//void delay_us(uint16_t us)
//{
//	for( ; us ; us--)
//	{
//		asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
//		asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
//	}
//}

//void delay_ms(uint16_t ms)
//{
//	for( ; ms ; ms--)
//	{
//		delay_us(1000) ;
//	}
//}

