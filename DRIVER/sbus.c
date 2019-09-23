#include "sbus.h"

#define RX_BUFF_SIZE 64							//SPEK_FRAME_SIZE 16  
uint8_t rx_buffer[RX_BUFF_SIZE];    //spekFrame[SPEK_FRAME_SIZE]
uint8_t rx_start = 0;
uint8_t rx_end = 0;
uint16_t rx_time[RX_BUFF_SIZE];			//????
int stat_overflow;

int framestarted = -1;
uint8_t framestart = 0;
// enable statistics
const int sbus_stats = 0;

unsigned long time_lastrx;
unsigned long time_siglost;
uint8_t last_rx_end = 0;
int last_byte = 0;
unsigned long time_lastframe;
int frame_received = 0;
int rx_state = 0;
int bind_safety = 0;
uint8_t data[25];

// statistics
int stat_framestartcount;
int stat_timing_fail;
int stat_garbage;
//int stat_timing[25];
int stat_frames_accepted = 0;
int stat_frames_second;
int stat_overflow;
uint8_t data[25];

int failsafe = 0;
int rxmode = 0;
int rx_ready = 0;
int channels[16];
extern uint16_t FRSKYD16_SendDataBuff[];
void sbus_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = SERIAL_RX_PIN;
    GPIO_Init(SERIAL_RX_PORT, &GPIO_InitStructure); 
    GPIO_PinAFConfig(SERIAL_RX_PORT, SERIAL_RX_SOURCE , SERIAL_RX_CHANNEL);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate = SERIAL_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_2;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx ;//USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
	
	if(SBUS_INVERT) 
	{
		USART_InvPinCmd(USART1, USART_InvPin_Rx|USART_InvPin_Tx , ENABLE );
	}

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    USART_Cmd(USART1, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
	framestarted = 0;
}

void USART1_IRQHandler(void)
{
    rx_buffer[rx_end] = USART_ReceiveData(USART1);
    // calculate timing since last rx
    unsigned long  maxticks = SysTick->LOAD;	
    unsigned long ticks = SysTick->VAL;	
    unsigned long elapsedticks;	    //逝去节拍数
    static unsigned long lastticks; //上一次的节拍
    if (ticks < lastticks)          //如果当前的节拍数小于小于上次记录的节拍数
	{
        elapsedticks = lastticks - ticks;	   //那么逝去的节拍数就等于 上次记录的节拍数-去现在记录的节拍数
	}
    else
    {// overflow ( underflow really)
		elapsedticks = lastticks + ( maxticks - ticks);   //当前获取的节拍数大于上次的节拍数，说明计数器已经溢出从新开始	
    }

    if ( elapsedticks < 65536 ) 
	{
		rx_time[rx_end] = elapsedticks; //
	}
    else 
	{
		rx_time[rx_end] = 65535;  //0xffff
	}
	
    lastticks = ticks;
       
    if ( USART_GetFlagStatus(USART1 , USART_FLAG_ORE ) )    //数据溢出中断
    {
		// overflow means something was lost 
		rx_time[rx_end]= 0xFFFe;
		USART_ClearFlag( USART1 , USART_FLAG_ORE );     //清除溢出中断标志位
		if ( sbus_stats ) 
		{
		  stat_overflow++;
		}
    }        
    rx_end++;
    rx_end%=(RX_BUFF_SIZE);            //构建循环缓存
}

void sbus_checkrx(void)
{
	if ( framestarted == 0)
	{
		while (  rx_end != rx_start )      //判断缓冲区是否为空，当队列头等于队列尾的时候  说明缓冲区为空
		{ 
			if ( rx_buffer[rx_start] == 0x0f )    //如果不为空，那么我们就要先找到 SBUS 协议的头  0x0f
			{
				// start detected
				framestart = rx_start;        //找到头   开始接收
				framestarted = 1;             //启动数据接收
				stat_framestartcount++; 
				break;                
			}         
			rx_start++;
			rx_start%=(RX_BUFF_SIZE);           
			stat_garbage++;
		}          
	}
	else if ( framestarted == 1)
	{
		// frame already has begun
		int size = 0;
		if (rx_end > framestart )
		{			
			size = rx_end - framestart;
		}
		else 
		{
			size = RX_BUFF_SIZE - framestart + rx_end;
		}
		
		if ( size >= 24 )
		{    
			int timing_fail = 0; 	
			for ( int i = framestart ; i <framestart + 25; i++  )  
			{
				data[ i - framestart] = rx_buffer[i%(RX_BUFF_SIZE)];
				int symboltime = rx_time[i%(RX_BUFF_SIZE)];         //rx_time 里存储了  每次接收数据的时间间隔
				//stat_timing[ i - framestart] = symboltime;
				if ( symboltime > 0x1690 &&  i - framestart > 0 ) 
				{
					timing_fail = 1;
				}
			}    

			if (!timing_fail) 
			{
				frame_received = 1;  
				last_byte = data[24];
				rx_start = rx_end;
				framestarted = 0;
				bind_safety++;
			} // end frame complete  
			
			rx_start = rx_end;
			framestarted = 0;
		
		}// end frame pending
		else
		{
			if ( framestarted < 0)
			{
				// initialize sbus
			  sbus_init();
			   // set in routine above "framestarted = 0;"    
			}
		}
		  
		if ( frame_received )
		{ 		
			FRSKYD16_SendDataBuff[0]  = ((data[1]|data[2]<< 8)                  & 0x07FF);
			FRSKYD16_SendDataBuff[1]  = ((data[2]>>3|data[3]<<5)                & 0x07FF);
			FRSKYD16_SendDataBuff[2]  = ((data[3]>>6|data[4]<<2|data[5]<<10)    & 0x07FF);
			FRSKYD16_SendDataBuff[3]  = ((data[5]>>1|data[6]<<7)                & 0x07FF);
			FRSKYD16_SendDataBuff[4]  = ((data[6]>>4|data[7]<<4)                & 0x07FF);
			FRSKYD16_SendDataBuff[5]  = ((data[7]>>7|data[8]<<1|data[9]<<9)     & 0x07FF);
			FRSKYD16_SendDataBuff[6]  = ((data[9]>>2|data[10]<<6)               & 0x07FF);
			FRSKYD16_SendDataBuff[7]  = ((data[10]>>5|data[11]<<3)              & 0x07FF);
			FRSKYD16_SendDataBuff[8]  = ((data[12]|data[13]<< 8)                & 0x07FF);
			FRSKYD16_SendDataBuff[9]  = ((data[13]>>3|data[14]<<5)              & 0x07FF);
			FRSKYD16_SendDataBuff[10] = ((data[14]>>6|data[15]<<2|data[16]<<10) & 0x07FF);
			FRSKYD16_SendDataBuff[11] = ((data[16]>>1|data[17]<<7)              & 0x07FF);
			FRSKYD16_SendDataBuff[12] = ((data[17]>>4|data[18]<<4)              & 0x07FF);
			FRSKYD16_SendDataBuff[13] = ((data[18]>>7|data[19]<<1|data[20]<<9)  & 0x07FF);
			FRSKYD16_SendDataBuff[14] = ((data[20]>>2|data[21]<<6)              & 0x07FF);
			FRSKYD16_SendDataBuff[15] = ((data[21]>>5|data[22]<<3)              & 0x07FF);

			frame_received = 0;    
		} // end frame received
	}
}
