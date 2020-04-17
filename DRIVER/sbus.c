#include "sbus.h"

#define RX_BUFF_SIZE 64							//SPEK_FRAME_SIZE 16  

//Channel values are 12-bit values between 988 and 2012, 1500 is the middle.
uint16_t Channel_DataBuff[16]  = { 1500 , 1500 , 988 , 1500 , 1500 , 1500 , 1500 , 1500 , 1500 , 1500 , 1500 , 1500 , 1500 , 1500 , 1500 , 1500};

uint8_t rx_buffer[RX_BUFF_SIZE];    //spekFrame[SPEK_FRAME_SIZE]
uint8_t rx_start = 0;
uint8_t rx_end = 0;
uint16_t rx_time[RX_BUFF_SIZE];		
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
uint8_t SbusDataRx[25];

int failsafe = 0;
int rxmode = 0;
int rx_ready = 0;
int channels[16];
extern uint16_t Channel_DataBuff[];
void sbus_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
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
    USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    USART_InitStructure.USART_StopBits = USART_StopBits_2;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx ;//USART_Mode_Rx | USART_Mode_Tx;
#ifdef  DMARx
	if(SBUS_INVERT) 
	{
		USART_InvPinCmd(USART1, USART_InvPin_Rx , ENABLE );
	}
    DMA_InitTypeDef DMA_InitStructure; 
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//ENABLE IDLE interrupt
    USART_Init(USART1, &USART_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);	
	
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);
    USART_Cmd(USART1, ENABLE);
    RCC_AHBPeriphClockCmd(RC_DMA_Rx_RCC,ENABLE);                       
	DMA_DeInit(RC_DMA_Rx_Ch);  
    DMA_InitStructure.DMA_PeripheralBaseAddr =  (uint32_t)(&USART1->RDR);         
    DMA_InitStructure.DMA_MemoryBaseAddr     =  (uint32_t) SbusDataRx;            
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                     
    DMA_InitStructure.DMA_BufferSize = SbusLength;                                
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;       
    DMA_InitStructure.DMA_MemoryInc =DMA_MemoryInc_Enable;                  
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;        
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                       
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                    
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                           
    DMA_Init(RC_DMA_Rx_Ch,&DMA_InitStructure);   
	DMA_ClearFlag(RC_Dma_RxFlagTC);
	DMA_Cmd(RC_DMA_Rx_Ch,ENABLE);   
#else

    USART_Init(USART1, &USART_InitStructure);
	
	if(SBUS_INVERT) 
	{
		USART_InvPinCmd(USART1, USART_InvPin_Rx|USART_InvPin_Tx , ENABLE );
	}

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    USART_Cmd(USART1, ENABLE);

    

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
	framestarted = 0;
#endif
    
}
uint16_t DMA_Residual_length=0; 
uint32_t ErrorRX=0; 
uint32_t OkRX=0; 
uint64_t IDLEIQ=0;
void USART1_IRQHandler(void)
{
#ifdef  DMARx
    uint8_t RxTemp;
	if(USART_GetITStatus(USART1,USART_IT_IDLE)!=RESET)
	{
        IDLEIQ++;
		USART_ClearITPendingBit(USART1,USART_IT_IDLE);//Clear USART_IT_IDLE
		RxTemp=USART1->ISR;			// Clear USART1->ISR
		RxTemp=USART1->RDR;			// Clear USART2->DR
		
		DMA_ClearFlag(RC_Dma_RxFlagTC);		// Clear DMA1_FLAG_TC3
		DMA_Residual_length=DMA_GetCurrDataCounter(RC_DMA_Rx_Ch);//Get the residual length
		if((DMA_Residual_length==SbusLength) && (SbusDataRx[0] == 0x0f && SbusDataRx[24]==0x00))// Invalid frame,Set Curr Data Counter SbusLength
		{
            OkRX++;
            Channel_DataBuff[0]  = ((SbusDataRx[2]  & 0x07)<<8) | (SbusDataRx[1] & 0x7ff);                                    //3+8
            Channel_DataBuff[1]  = ((SbusDataRx[3]  & 0x3f)<<5) | ((SbusDataRx[2] >>3) & 0x1F);                               //6+5
            Channel_DataBuff[2]  = (((SbusDataRx[5]  & 0x01)<<10)| (SbusDataRx[4] <<2) | (SbusDataRx[3]>>6) ) & 0x7ff;        //1+8+2
            Channel_DataBuff[3]  = (((SbusDataRx[6]  & 0x0f)<<7) | (SbusDataRx[5] >>1)  ) & 0x7ff;                            //4+7
            Channel_DataBuff[4]  = (((SbusDataRx[7]  & 0x7f)<<4) | (SbusDataRx[6] >>4)  ) & 0x7ff;                            //7+4
            Channel_DataBuff[5]  = (((SbusDataRx[9]  & 0x03)<<9) | (SbusDataRx[8] <<1)  | (SbusDataRx[7]>>7) ) & 0x7ff;       //2+8+1
            Channel_DataBuff[6]  = (((SbusDataRx[10] & 0x1f)<<6) | (SbusDataRx[9] >>2)  ) & 0x7ff;                            //5+6
            Channel_DataBuff[7]  = ((SbusDataRx[11]<<3) | (SbusDataRx[10]>>5)) & 0x7ff;                                       //8+3 
            Channel_DataBuff[8]  = (((SbusDataRx[13] & 0x07)<<8) | SbusDataRx[12]) & 0x7ff;                                   //3+8
            Channel_DataBuff[9]  = (((SbusDataRx[14] & 0x3f)<<5) | (SbusDataRx[13] >>3) ) & 0x7ff;                            //6+5
            Channel_DataBuff[10] = (((SbusDataRx[16] & 0x01)<<10)| (SbusDataRx[15] <<2) | (SbusDataRx[14]>>6) )& 0x7ff;       //1+8+2
            Channel_DataBuff[11] = (((SbusDataRx[17] & 0x0f)<<7) | (SbusDataRx[16] >>1) ) & 0x7ff;                            //4+7
            Channel_DataBuff[12] = (((SbusDataRx[18] & 0x7f)<<4) | (SbusDataRx[17] >>4) ) & 0x7ff;                            //7+4
            Channel_DataBuff[13] = (((SbusDataRx[20] & 0x03)<<9) | (SbusDataRx[19] <<1) | (SbusDataRx[18]>>7)) & 0x7ff;       //2+8+1
            Channel_DataBuff[14] = (((SbusDataRx[21] & 0x1f)<<6) | (SbusDataRx[20] >>2) ) & 0x7ff;                            //5+6      
            Channel_DataBuff[15] = ((SbusDataRx[22]<<3) | (SbusDataRx[21]>>5)) & 0x7ff;    								   //8+3

			frame_received = 0; 

		}
		else
		{
            DMA_Cmd(RC_DMA_Rx_Ch,DISABLE);		// off DMA1_Channel3
            DMA_SetCurrDataCounter(RC_DMA_Rx_Ch,SbusLength);
			SbusDataRx[0]=0;
			SbusDataRx[24]=0xff;
			ErrorRX++;
            DMA_Cmd(RC_DMA_Rx_Ch,ENABLE);
		}
		
	}
#else
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
#endif
}

void sbus_checkrx(void)
{
	if ( framestarted == 0)
	{
		while (  rx_end != rx_start )            //判断缓冲区是否为空，当队列头等于队列尾的时候  说明缓冲区为空
		{ 
			if ( rx_buffer[rx_start] == 0x0f )    //如果不为空，那么我们就要先找到 SBUS 协议的头  0x0f
			{
				// start detected
				framestart = rx_start;            //找到头   开始接收
				framestarted = 1;                 //启动数据接收
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
			Channel_DataBuff[0]  = ((data[1]  |    data[2]<<8)                  & 0x07FF);
			Channel_DataBuff[1]  = ((data[2]  >>3 |data[3]<<5)                  & 0x07FF);
			Channel_DataBuff[2]  = ((data[3]  >>6 |data[4]<<2   |data[5]<<10)   & 0x07FF);
			Channel_DataBuff[3]  = ((data[5]  >>1 |data[6]<<7)                  & 0x07FF);
			Channel_DataBuff[4]  = ((data[6]  >>4 |data[7]<<4)                  & 0x07FF);
			Channel_DataBuff[5]  = ((data[7]  >>7 |data[8]<<1   |data[9]<<9)    & 0x07FF);
			Channel_DataBuff[6]  = ((data[9]  >>2 |data[10]<<6)                 & 0x07FF);
			Channel_DataBuff[7]  = ((data[10] >>5 |data[11]<<3)                 & 0x07FF);
			Channel_DataBuff[8]  = ((data[12] |    data[13]<<8)                 & 0x07FF);
			Channel_DataBuff[9]  = ((data[13] >>3 |data[14]<<5)                 & 0x07FF);
			Channel_DataBuff[10] = ((data[14] >>6 |data[15]<<2  |data[16]<<10)  & 0x07FF);
			Channel_DataBuff[11] = ((data[16] >>1 |data[17]<<7)                 & 0x07FF);
			Channel_DataBuff[12] = ((data[17] >>4 |data[18]<<4)                 & 0x07FF);
			Channel_DataBuff[13] = ((data[18] >>7 |data[19]<<1  |data[20]<<9)   & 0x07FF);
			Channel_DataBuff[14] = ((data[20] >>2 |data[21]<<6)                 & 0x07FF);
			Channel_DataBuff[15] = ((data[21] >>5 |data[22]<<3)                 & 0x07FF);

			frame_received = 0;    
		} // end frame received
	}
}
