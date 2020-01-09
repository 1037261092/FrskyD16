#ifndef __FRSKY_D16_
#define __FRSKY_D16_

#include <stdbool.h>
#include "stm32f0xx.h"

#define THR_Output_Max 1995 
#define THR_Output_Mid 1500
#define THR_Output_Min 1005

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// 发送最大值 必须大于 发送最小值
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define Output_Max 2012 
#define Output_Mid 1500
#define Output_Min 988

extern uint8_t Version_select_flag;

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
}ChannelTypeDef ;


extern bool CommunicationError_flg ;

#define RF_TypeVersion		      0x44						//遥控器类型 'F' -> FRSKYD16
#define MasterInitProtocolVersion 0x01						//无线协议版本号
#define PTOTOCOL_MAX_CHANNEL      8						//协议支持最大发送通道 8 通道(固定发送16通道数据)
#define TRANSMITTER_CHANNEL       8

extern bool CommunicationError_flg ; 
extern bool HighThrottle_flg ; 
extern bool Bind_flg ; 
extern uint16_t TransmitterID ; 
////////////////////////////////////////////////////////////////////////////////
//SFHSS 8通 无线数据发送数据
////////////////////////////////////////////////////////////////////////////////
extern uint16_t Channel_DataBuff[8];
extern const uint8_t FRSKYD8_CH_Code[8];
void initFRSKYD8(void);
uint16_t ReadFRSKYD8(void);
void SetBind(void) ; 
void FRSKYD8_build_Data_packet(void);

#endif
