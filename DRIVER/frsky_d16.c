#include "frsky_d16.h"
#include "cc2500.h"
#include "delay.h"
#include "function.h"
#include <stdlib.h>
#include "sbus.h"
#include "adc.h"
#include "led.h"

#define FRSKYD8_PACKET_LEN  18
#define FRSKYD8_BINDCHANNEL 47 						//The 47th channel is fixed as a bound channel 

bool     Bind_flg = false ; 
uint16_t FRSKYD8_BindCounts = 0; 						// 对码数据包发送个数
uint8_t  FRSKYD8_Counts = 0;
uint8_t	 FRSKYD8_ChannelShip = 0;       				// 跳频间隔(前后两次频段间隔)
uint8_t  FRSKYD8_ctr = 0 ; 							
uint8_t  FRSKYD8_CountsRst = 0; 						
uint8_t  FRSKYD8_HOPChannel[50] ; 						// 跳频列表(根据遥控器ID计算出47个跳频号(后三个频点无效))	
uint8_t  FRSKYD8_calData[50];							// 记录跳频通道频率值

uint8_t  FRSKYD8_Channel_Num = 0   ; 					// 跳频通道号
bool CC2500_Error_flg = false ; 
bool HighThrottle_flg = true ; 							//高油门标志位
uint16_t TransmitterID ; 							    //遥控器唯一ID
uint8_t  SendPacket[18] ; 							    //发送数据包缓存 (1) 对码数据包14Byte   (2)发送遥控数据包 28Byte(8 + 16CH*2 = 40)

typedef enum 
{
	FRSKYD8_BIND  		    = 0x00 , 
	//FRSKYD16_BIND_PASSBACK	= 0x01 , 
	FRSKYD8_DATA  		    = 0x02 ,	
    //FRSKYD16_TUNE  		    = 0x03 ,
}FRSKYD8PhaseTypeDef ;

FRSKYD8PhaseTypeDef FRSKYD8Phase = FRSKYD8_DATA ; 

//Channel values are 12-bit values between 988 and 2012, 1500 is the middle.
uint16_t Channel_DataBuff[8]  = { 1500 , 1500 , 988 , 1500 , 1500 , 1500 , 1500 , 1500};

//FRSKYD16 Channel order
const uint8_t  FRSKYD8_CH_Code[8] = {AILERON, ELEVATOR, THROTTLE, RUDDER, AUX1, AUX2, AUX3, AUX4};


//==============================================================================
//			FRSKYD8 初始化器件地址
//==============================================================================
static void __attribute__((unused)) FRSKYD8_InitDeviceAddr(bool IsBindFlg)
{
	CC2500_WriteReg(CC2500_18_MCSM0,    0x08) ;	
	CC2500_WriteReg(CC2500_09_ADDR , IsBindFlg ? 0x03 : (TransmitterID >> 8)&0xFF);
	CC2500_WriteReg(CC2500_07_PKTCTRL1,0x05);
}

//==============================================================================
//			FRSKYD8 设置发送通道
//==============================================================================
static void __attribute__((unused)) FRSKYD8_TuneChannel(uint8_t Channel)
{
	CC2500_Strobe(CC2500_SIDLE);						                //进入闲置状态
	CC2500_WriteReg(CC2500_25_FSCAL1, FRSKYD8_calData[Channel]);		//设置发送通道
	CC2500_WriteReg(CC2500_0A_CHANNR, FRSKYD8_HOPChannel[Channel]);	    //设置发送通道
	CC2500_Strobe(CC2500_SCAL);						                    //校准频率合成器并关闭
}

/*-------------------------------------------------------------
							CRC 
---------------------------------------------------------------
static const uint16_t CRC_Short[16] = 	//CRC
{
	0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
	0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7 
};

static uint16_t CRCTable(uint8_t val)
{
	uint16_t word ;
	word = CRC_Short[val&0x0F] ;
	val /= 16 ;
	return word ^ (0x1081 * val) ;
}

static uint16_t crc_x(uint8_t *data, uint8_t len)
{
	uint16_t crc = 0;
	for(uint8_t i=0; i < len; i++)
	{
		crc = (crc<<8) ^ CRCTable((uint8_t)(crc>>8) ^ *data++);
	}
	return crc;
}

//通道数据换算
static uint16_t  __attribute__((unused)) CalChannelData( uint8_t channel)
{	
  	if(channel > 15) channel = 15 ;
  	uint16_t DataTemp = Channel_DataBuff[channel]; 
	DataTemp = (((DataTemp*15)>>4)+1290) - 1226;
  	if(channel>7)
		DataTemp |= 2048;
	
	return DataTemp;		//mapped 0 ,2140(125%) range to 64,1984 ;
}

*/

uint16_t convert_channel_frsky(uint8_t num)
{
	uint16_t val=Channel_DataBuff[num];
	return ((val*15)>>4)+1290;
}
/*--------------------------------------------------------------
					frequency hopping 
---------------------------------------------------------------*/
static void __attribute__((unused)) FRSKYD8_tune_chan_fast(void)
{
	CC2500_Strobe(CC2500_SIDLE);
	CC2500_WriteReg(CC2500_0D_FREQ2 , Fre_Carrier_H);
	CC2500_WriteReg(CC2500_0E_FREQ1 , Fre_Carrier_M);
	CC2500_WriteReg(CC2500_0F_FREQ0 , Fre_Carrier_L);
	delay_us(2);
	CC2500_Strobe(CC2500_SIDLE);
	CC2500_WriteReg(CC2500_0D_FREQ2 , Fre_Carrier_H);
	CC2500_WriteReg(CC2500_0E_FREQ1 , Fre_Carrier_M);
	CC2500_WriteReg(CC2500_0F_FREQ0 , Fre_Carrier_L);
	CC2500_WriteReg(CC2500_25_FSCAL1, FRSKYD8_calData[FRSKYD8_Counts%47]);
	CC2500_WriteReg(CC2500_0A_CHANNR, FRSKYD8_HOPChannel[FRSKYD8_Counts%47]);
}


/*-------------------------------------------------------------------------------
	                      get next channel
--------------------------------------------------------------------------------
static void FRSKYD8_calc_next_chan(void)
{
    	FRSKYD8_Channel_Num = (FRSKYD8_Channel_Num + FRSKYD8_ChannelShip) % 47 ;
}
*/

/*--------------------------------------------------------------------------------
						 build bind packet
---------------------------------------------------------------------------------*/
static void __attribute__((unused)) Frsky_D8_build_Bind_packet(void)
{
		//固定码

	SendPacket[0] = 0x11;
	SendPacket[1] = 0x03;
	SendPacket[2] = 0x01;
	//遥控器ID
	SendPacket[3] = (TransmitterID >> 8) & 0xFF  ;
	SendPacket[4] = TransmitterID & 0xFF ;
	
	uint8_t  idx 	= (FRSKYD8_BindCounts % 10) * 5 ;
	SendPacket[5]   = idx;	
	SendPacket[6]   = FRSKYD8_HOPChannel[idx++];
	SendPacket[7]   = FRSKYD8_HOPChannel[idx++];
	SendPacket[8]   = FRSKYD8_HOPChannel[idx++];
	SendPacket[9]   = FRSKYD8_HOPChannel[idx++];
	SendPacket[10]  = FRSKYD8_HOPChannel[idx++];
	SendPacket[11]  = 0x00;
	SendPacket[12]  = 0x00; 
	SendPacket[13] 	= 0x00;
	SendPacket[14] 	= 0x00;
	SendPacket[15] 	= 0x00;
	SendPacket[16] 	= 0x00;
	SendPacket[17] 	= 0x00;	
}

/*---------------------------------------------------------------------
			build control data package					
----------------------------------------------------------------------*/
void  __attribute__((unused)) FRSKYD8_build_Data_packet()
{
	sbus_checkrx();
	//固定码 + 遥控ID
	SendPacket[0] = 0x11;

	//telemetry radio ID
	SendPacket[1]   = (TransmitterID >> 8) & 0xFF  ;
	SendPacket[2]   = TransmitterID & 0xFF ;           
	SendPacket[3] 	= FRSKYD8_Counts;
	SendPacket[4] = 0x00;
	SendPacket[5] = 0x01;
	
	SendPacket[10] = 0;
	SendPacket[11] = 0;
	SendPacket[16] = 0;
	SendPacket[17] = 0;
	
	for(uint8_t i=0; i < 8; i++)
	{
		//uint16_t value = Channel_DataBuff[i];// + (uint16_t)((float)Channel_DataBuff[i]/2+0.5f);
		uint16_t value;
		value = convert_channel_frsky(i);
		if(i < 4)
		{
			SendPacket[6+i] = value & 0xff;
			SendPacket[10+(i>>1)] |= ((value >> 8) & 0x0f) << (4 *(i & 0x01));
		}
		else
		{
			SendPacket[8+i] = value & 0xff;
			SendPacket[16+((i-4)>>1)] |= ((value >> 8) & 0x0f) << (4 * ((i-4) & 0x01));
		}
	}
}

//==============================================================================
//FRSKYD16 : 计算 FRSKYD16 通道(通过计算得到 47 个通道 。轮询时，在这47个通道间跳频)
//相邻两频段间隔在 5 以上
// 1  - 26  : 取 16 个频点
// 27 - 52  : 取 15 个频点
// 53 - 76  : 取 16 个频点
//==============================================================================
void Calc_FRSKYD8_Channel()
{
	uint8_t  idx = 0;
	uint16_t id_tmp = ~ TransmitterID; 					          
	
	while(idx < 47)
	{
		uint8_t i;
		uint8_t count_1_26 = 0, count_27_52 = 0, count_53_76 = 0;
		id_tmp = id_tmp * 0x0019660D + 0x3C6EF35F;			        // Randomization
		uint8_t next_ch = ((id_tmp >> 8) % 0x4B) + 1;			    // Use least-significant byte and must be larger than 1
		for (i = 0; i < idx; i++)
		{
			if(FRSKYD8_HOPChannel[i] == next_ch)    	
			{
				break;
			}
			if(FRSKYD8_HOPChannel[i] < 27) 		
			{
				count_1_26++;
			}
			else if(FRSKYD8_HOPChannel[i] < 53)  		
			{
				count_27_52++;
			}
			else 
			{				
				count_53_76++;
			}
		}
		if (i != idx)  
		{
			continue;				
		}
		
		if(idx)
		{
			uint8_t Temp = 0 ; 
			if(next_ch > FRSKYD8_HOPChannel[idx - 1]) 	
			{
				Temp = next_ch - FRSKYD8_HOPChannel[idx - 1] ;
			}				
			else 						
			{
				Temp = FRSKYD8_HOPChannel[idx - 1] - next_ch ;
			}				
		  	if(Temp < 5)
			{
				continue ;
			}				
		}
		
		if(next_ch == 0)  break; 
		
		//get frequency point
		if (((next_ch < 27) && (count_1_26 < 16)) || ((next_ch >= 27) && (next_ch < 53) && (count_27_52 < 15)) || ((next_ch >= 53) && (count_53_76 < 16)))
		{
			FRSKYD8_HOPChannel[idx++] = next_ch;
		}
	}
	
	FRSKYD8_HOPChannel[FRSKYD8_BindCounts] = 0 ;                  //Band band binding is 0
}


uint16_t ReadFRSKYD8(void)
{
	if(!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1))
	{
		SetBind();
	}
	else
	{
	
	}
	
	switch(FRSKYD8Phase)
	{
		//send bind data
		case FRSKYD8_BIND : 
		  	if(FRSKYD8_BindCounts < 1200)
			{
				FRSKYD8_TuneChannel(FRSKYD8_BINDCHANNEL) ; 
				CC2500_SetPower(CC2500_POWER_3);
				CC2500_Strobe(CC2500_SFRX);
				Frsky_D8_build_Bind_packet();
				CC2500_Strobe(CC2500_SIDLE);
				CC2500_Strobe(CC2500_SFTX);
				CC2500_WriteData(SendPacket, FRSKYD8_PACKET_LEN);
				++FRSKYD8_BindCounts ; 
				Led_On_Off(FRSKYD8_BindCounts & 0x10);
				
			}  
			else
			{
			  	Bind_flg = false ; 
				FRSKYD8_BindCounts = 0 ; 
				FRSKYD8_Counts = 0 ; 
				CC2500_SetPower(RF_POWER);           //设置发送功率
				FRSKYD8_InitDeviceAddr(Bind_flg) ;	
				FRSKYD8Phase = FRSKYD8_DATA ; 
				Led_On_Off(0);
				
			}
			return 8830 ;
		// Frsky D16 data
		case FRSKYD8_DATA :
			FRSKYD8_Counts = (FRSKYD8_Counts + 1) % 188;
			FRSKYD8_tune_chan_fast();
			FRSKYD8_build_Data_packet();
			CC2500_Strobe(CC2500_SIDLE);
			CC2500_WriteData(SendPacket, FRSKYD8_PACKET_LEN);	
			return 8830 ;  
	  
	}
	return 0 ; 
}

void SetBind(void)
{
  	FRSKYD8_BindCounts = 0 ; 
	FRSKYD8Phase = FRSKYD8_BIND ;
	FRSKYD8_TuneChannel(FRSKYD8_HOPChannel[FRSKYD8_BINDCHANNEL]) ; 
}


void initFRSKYD8(void)
{
  	//get chip ID
  	TransmitterID = GetUniqueID();

	//Get the frequency hopping by chip ID
	Calc_FRSKYD8_Channel();
	
	adc_init();
	ADC_StartOfConversion(ADC1);
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)==RESET);
	srand(ADC_GetConversionValue(ADC1));     //Gets a random number from the ADC
	FRSKYD8_ChannelShip = rand() % 46 + 1;  // Initialize it to random 0-47 inclusive
	while((FRSKYD8_ChannelShip - FRSKYD8_ctr) % 4) 
	{
		FRSKYD8_ctr = (FRSKYD8_ctr + 1) % 4 ;
	}
	FRSKYD8_CountsRst = (FRSKYD8_ChannelShip - FRSKYD8_ctr) >> 2 ; 
	
	CC2500_Error_flg = CC2500_Init() ; 
	if(CC2500_Error_flg == true)
	{
		//Initialization failed
	}
	else
	{
		// calibration frequency
		for (uint8_t i = 0 ; i < 48 ; i++)
		{
			CC2500_Strobe(CC2500_SIDLE);
			CC2500_WriteReg(CC2500_0A_CHANNR , FRSKYD8_HOPChannel[i]);
			CC2500_Strobe(CC2500_SCAL);
			delay_ms(1);
			FRSKYD8_calData[i]  =  CC2500_ReadReg(CC2500_25_FSCAL1);
		}
		FRSKYD8_InitDeviceAddr(Bind_flg);	
		FRSKYD8Phase = FRSKYD8_DATA ;
		FRSKYD8_TuneChannel(FRSKYD8_HOPChannel[0]); 
	}
}
