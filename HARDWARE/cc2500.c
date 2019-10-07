#include "cc2500.h"
#include "spi.h"
#include "delay.h"

#define FRSKYD16_CONFIG_CNTS  36

static const uint8_t cc2500_conf[FRSKYD16_CONFIG_CNTS][2]=
{
	{ CC2500_02_IOCFG0   , 0x06 },	
	{ CC2500_00_IOCFG2   , 0x06 },	
#ifdef LBT
	{ CC2500_17_MCSM1    , 0x0E },   //接收数据后保持在RX 状态，发送数据后保持在TX
#else
	{ CC2500_17_MCSM1    , 0x0c },	 //接收数据后保持在RX 状态，发送数据后保持在空闲
#endif	
	{ CC2500_18_MCSM0    , 0x18 },
	
#ifdef LBT
	{ CC2500_06_PKTLEN   , 0x23 },    //数据包长度为35
#else 
	{ CC2500_06_PKTLEN   , 0x1E },    //数据包长度为30
#endif		 
	{ CC2500_07_PKTCTRL1 , 0x04 },    
	{ CC2500_08_PKTCTRL0 , 0x01 },  
	{ CC2500_3E_PATABLE  , 0xff }, 
#ifdef LBT
	{ CC2500_0B_FSCTRL1  , 0x08 },    //配置中频（IF）频率 = F(xosc)/2^10*FREQ_IF
#else
	{ CC2500_0B_FSCTRL1  , 0x0A },    //配置中频（IF）频率 = F(xosc)/2^10*FREQ_IF
#endif	
	{ CC2500_0C_FSCTRL0  , 0x00 },	
	{ CC2500_0D_FREQ2    , 0x5c },	

#ifdef LBT
	{ CC2500_0E_FREQ1    , 0x80 },    //频率控制
#else
	{ CC2500_0E_FREQ1    , 0x76 },  
#endif	
	
#ifdef LBT
	{ CC2500_0F_FREQ0    , 0x00 },     //频率控制
#else
	{ CC2500_0F_FREQ0    , 0x27 }, 
#endif			
	{ CC2500_10_MDMCFG4  , 0x7B },
	
#ifdef LBT
	{ CC2500_11_MDMCFG3  , 0xF8 },     //调制解调器速率配置
#else
	{ CC2500_11_MDMCFG3  , 0x61 }, 
#endif	
	    
#ifdef LBT
	{ CC2500_12_MDMCFG2  , 0x03 },      //调制解调器的调制方式设置
#else
	{ CC2500_12_MDMCFG2  , 0x13 },  
#endif		
	
	{ CC2500_13_MDMCFG1  , 0x23 },  
	{ CC2500_14_MDMCFG0  , 0x7a },
	
#ifdef LBT
	{ CC2500_15_DEVIATN  , 0x53 },
#else
	{ CC2500_15_DEVIATN  , 0x51 },  
#endif
	
	{ CC2500_19_FOCCFG   , 0x16 }, 
	{ CC2500_1A_BSCFG    , 0x6c },	
	{ CC2500_1B_AGCCTRL2 , 0x43 },	
	{ CC2500_1C_AGCCTRL1 , 0x40 },    
	{ CC2500_1D_AGCCTRL0 , 0x91 },  
	{ CC2500_21_FREND1   , 0x56 },
	{ CC2500_22_FREND0   , 0x10 },
	{ CC2500_23_FSCAL3   , 0xa9 },
	{ CC2500_24_FSCAL2   , 0x0A },
	{ CC2500_25_FSCAL1   , 0x00 },
	{ CC2500_26_FSCAL0   , 0x11 },
	{ CC2500_29_FSTEST   , 0x59 },
	{ CC2500_2C_TEST2    , 0x88 },
	{ CC2500_2D_TEST1    , 0x31 },
	{ CC2500_2E_TEST0    , 0x0B },
	{ CC2500_03_FIFOTHR  , 0x07 },
	{ CC2500_09_ADDR     , 0x00 }
	
};

static void CC2500_WaitingReady()
{
	uint16_t cnts = 0;
	while(((SPI_MISO_PORT->IDR & SPI_PIN_MISO) != (uint32_t)Bit_RESET) && ++cnts < 10000);
}


void CC2500_SetPower(uint8_t power)
{
	static uint8_t prev_power = CC2500_POWER_1;
	if(prev_power != power)
	{
		prev_power = power;
		CC2500_WriteReg(CC2500_3E_PATABLE, power);
	}
}

void CC2500_Strobe(uint8_t state)
{
	CC2500_NSS_LOW;
	CC2500_WaitingReady();  
	delay_us(1);
	SPI_WriteByte(state);   
	delay_us(1);
	CC2500_NSS_HIGH;
}

void CC2500_WriteReg(uint8_t address, uint8_t data)
{
	CC2500_NSS_LOW;       	//delay_us(1);
	CC2500_WaitingReady();
	SPI_WriteByte(address);// delay_us(1);
	SPI_WriteByte(data);    //delay_us(1);
	CC2500_NSS_HIGH;
	delay_us(5);
}

void CC2500_WriteReglistrMulti(uint8_t address, const uint8_t data[], uint8_t length)
{
	CC2500_NSS_LOW;          delay_us(1);
	CC2500_WaitingReady();
	SPI_WriteByte(CC2500_WRITE_BURST | address);
	for(uint8_t i=0; i < length ; i++)
	{
		SPI_WriteByte(data[i]);
		delay_us(1);
	}
	delay_us(1);
	CC2500_NSS_HIGH;
}

uint8_t CC2500_ReadReg(uint8_t address)
{
	uint8_t result;
	CC2500_NSS_LOW;           delay_us(2);
	CC2500_WaitingReady();
	result = SPI_Transfer(CC2500_READ_SINGLE | address);
	CC2500_NSS_HIGH;
	return result;
}

uint8_t CC2500_Reset()
{
	CC2500_Strobe(CC2500_SRES);
	delay_ms(1);
	CC2500_SetTxRxMode(TXRX_OFF);
	return (CC2500_ReadReg(CC2500_0E_FREQ1) == 0xC4);//check if reset
}

void CC2500_SetTxRxMode(uint8_t mode)
{
	if(mode == TX_EN)
	{//from deviation firmware
		CC2500_WriteReg(CC2500_00_IOCFG2, 0x2F);
		CC2500_WriteReg(CC2500_02_IOCFG0, 0x2F | 0x40);
	}
	else
		if (mode == RX_EN)
		{
			CC2500_WriteReg(CC2500_02_IOCFG0, 0x2F);
			CC2500_WriteReg(CC2500_00_IOCFG2, 0x2F | 0x40);
		}
		else
		{
			CC2500_WriteReg(CC2500_02_IOCFG0, 0x2F);
			CC2500_WriteReg(CC2500_00_IOCFG2, 0x2F);
		}
}

void CC2500_WriteData(uint8_t *dpbuffer, uint8_t len)
{
	CC2500_Strobe(CC2500_SFTX);
	CC2500_WriteReglistrMulti(CC2500_3F_TXFIFO,dpbuffer,len);
	CC2500_Strobe(CC2500_STX);
}


bool CC2500_Init(void)
{
	bool CC2500RestError_flag = false;
	spi_init();
	if(!CC2500_Reset())
	{
		CC2500RestError_flag = true;	
	}
	delay_ms(1);
	if(!CC2500RestError_flag)
	{
		for(uint8_t i=0 ;i < FRSKYD16_CONFIG_CNTS ; ++i)
		{
			CC2500_WriteReg(cc2500_conf[i][0],cc2500_conf[i][1]);
			delay_us(20);
		}
		//CC2500_Strobe(CC2500_SIDLE);
		delay_us(10);
		CC2500_SetTxRxMode(TX_EN);
		CC2500_SetPower(CC2500_POWER_15);
		CC2500_Strobe(CC2500_SIDLE);
		delay_us(10);
	}
	return CC2500RestError_flag;
}


