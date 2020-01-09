#include "cc2500.h"
#include "spi.h"
#include "delay.h"
#include "frsky_d16.h"
#define FRSKYD8_CONFIG_CNTS  36

static const uint8_t cc2500_conf[FRSKYD8_CONFIG_CNTS][2]=
{
	{ CC2500_02_IOCFG0   , 0x06 },	// GDO0 指示同步码发送/接收情况
	{ CC2500_00_IOCFG2   , 0x06 },	// GDO2 指示同步码发送/接收情况
	{ CC2500_17_MCSM1    , 0x0c },  // 接收完一个数据包后继续在RX模式 ， 发送完一个数据包后进入 IDLE 模式
	{ CC2500_18_MCSM0    , 0x18 },  // 从 IDLE 进入 TX/RX 模式前，自动校准频率
	{ CC2500_06_PKTLEN   , 0x19 },  // 数据包长度 : 25 Byte
	{ CC2500_07_PKTCTRL1 , 0x05 },  // 接收数据包中时钟包含两 Byte 数据(RSSI 、LQI) ， 确认地址匹配
	{ CC2500_08_PKTCTRL0 , 0x05 },  // 数据包长度由同步码第一个字节决定(开启CRC校验)
	{ CC2500_3E_PATABLE  , 0xff },  // PA功率 : 设置PA最大发射功率
	{ CC2500_0B_FSCTRL1  , 0x08 },  // 中值频率 :   Fif = 26M/1024 * 10 = 203125Hz
	{ CC2500_0C_FSCTRL0  , 0x00 },	// 频率补偿 :   0
	{ CC2500_0D_FREQ2    , 0x5c },	// 载波频率 :   Fc  = 26M/65536*(0x5C7627) =  24039998474.412109375Hz         
	{ CC2500_0E_FREQ1    , 0x76 },
	{ CC2500_0F_FREQ0    , 0x27 },
	{ CC2500_10_MDMCFG4  , 0xAA },	// 采样带宽 :   BW = 26M / 8*(4+2)*4 = 135417 Hz 
	{ CC2500_11_MDMCFG3  , 0x39 },  // 数据率 :  Rdata = (256 + 0x39)*1024/268435456 * 26M = 31044 Baud   
	{ CC2500_12_MDMCFG2  , 0x11 },  // GFSK  15/16 Bit 同步码
	{ CC2500_13_MDMCFG1  , 0x23 },  // 2Byte 序文   
	{ CC2500_14_MDMCFG0  , 0x7a },  // 频道间隔 : f = 26M/262144 * (256 + 0x7A) * 8 = 299926 Hz
	{ CC2500_15_DEVIATN  , 0x42 },  // MSK 模式有效
	{ CC2500_1B_AGCCTRL2 , 0x03 },	// bind ? 0x43 : 0x03	
	{ CC2500_19_FOCCFG   , 0x16 },  // 频率偏移补偿配置
	{ CC2500_1A_BSCFG    , 0x6c },	// 同步位设置
	{ CC2500_1C_AGCCTRL1 , 0x40 },  // AGC控制  最高位增益关闭  
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

void CC2592_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA , ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void CC2500_WaitingReady()
{
	uint16_t cnts = 0;
	while(((SPI_MISO_PORT->IDR & SPI_PIN_MISO) != (uint32_t)Bit_RESET) && ++cnts < 10000);
}


void CC2500_SetPower(uint8_t power)
{
//	static uint8_t prev_power = CC2500_POWER_1;
//	if(prev_power != power)
//	{
	//	prev_power = power;
		CC2500_WriteReg(CC2500_3E_PATABLE, power);
	//}
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
	CC2592_Init();
	PA_EN;
	LNA_DISEN;
	HGM_DISEN;
	bool CC2500RestError_flag = false;
	spi_init();
	if(!CC2500_Reset())
	{
		CC2500RestError_flag = true;	
	}
	delay_ms(1);
	if(!CC2500RestError_flag)
	{
		for(uint8_t i=0 ;i < FRSKYD8_CONFIG_CNTS ; ++i)
		{
			CC2500_WriteReg(cc2500_conf[i][0],cc2500_conf[i][1]);  //FCC
			delay_us(20);
		}
		CC2500_Strobe(CC2500_SIDLE);
		delay_us(20);
		//CC2500_SetTxRxMode(TX_EN);
		CC2500_SetPower(RF_POWER);
		CC2500_Strobe(CC2500_SIDLE);
		delay_ms(10);
	}
	return CC2500RestError_flag;
}


