#include "adc.h"

void adc_init()
{
	//时钟配置
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	
	GPIO_InitTypeDef PORT_ADC;
	PORT_ADC.GPIO_Pin=GPIO_Pin_1;
	PORT_ADC.GPIO_Mode=GPIO_Mode_AN;
	PORT_ADC.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA,&PORT_ADC);
	
	//ADC 参数配置
	ADC_InitTypeDef ADC_InitStuctrue;
	ADC_InitStuctrue.ADC_Resolution=ADC_Resolution_12b;    //12位精度
	ADC_InitStuctrue.ADC_ContinuousConvMode=DISABLE;       //单次ADC
	ADC_InitStuctrue.ADC_ExternalTrigConvEdge=ADC_ExternalTrigConvEdge_None;
	ADC_InitStuctrue.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_TRGO;
	ADC_InitStuctrue.ADC_DataAlign=ADC_DataAlign_Right;     //数据右对齐
	ADC_InitStuctrue.ADC_ScanDirection=ADC_ScanDirection_Backward;//数据覆盖
	ADC_Init(ADC1,&ADC_InitStuctrue);
	
	ADC_ChannelConfig(ADC1,ADC_Channel_1,ADC_SampleTime_239_5Cycles);
	ADC_GetCalibrationFactor(ADC1);

	//使能
	ADC_Cmd(ADC1,ENABLE);
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_ADEN)==RESET);
}
