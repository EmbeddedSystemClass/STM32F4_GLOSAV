#include "mfunc_app.h"
#include "can_app.h"
#include "adc.h"
#include "mb_app.h"
#include "tim.h"


#define ADC_POLL_PERIOD		1000

//uint16_t adc_channels_data[ADC_CHANNELS_NUM];
//const uint8_t adc_channel_list[MFUNC_CHANNELS_NUM]={ADC_CHANNEL_8,ADC_CHANNEL_14,ADC_CHANNEL_15};
//SemaphoreHandle_t	xADC_DataMutex;

stMfunc MfuncInputs[MFUNC_CHANNELS_NUM];

static void Mfunc_Task(void *pvParameters);

void Mfunc_App_Init(void)
{
		MfuncInputs[0].port=GPIOF;
		MfuncInputs[0].pin=GPIO_PIN_4;
		MfuncInputs[0].adc=&hadc3;
		MfuncInputs[0].adcChn=ADC_CHANNEL_14;
		MfuncInputs[0].mode=MFUNC_ADC;
	
		MfuncInputs[1].port=GPIOF;
		MfuncInputs[1].pin=GPIO_PIN_5;
		MfuncInputs[1].adc=&hadc3;
		MfuncInputs[1].adcChn=ADC_CHANNEL_15;
		MfuncInputs[1].mode=MFUNC_ADC;
	
		MfuncInputs[2].port=GPIOF;
		MfuncInputs[2].pin=GPIO_PIN_6;
		MfuncInputs[2].adc=&hadc3;
		MfuncInputs[2].adcChn=ADC_CHANNEL_4;
		MfuncInputs[2].mode=MFUNC_ADC;
	
		MfuncInputs[3].port=GPIOF;
		MfuncInputs[3].pin=GPIO_PIN_7;
		MfuncInputs[3].adc=&hadc3;
		MfuncInputs[3].adcChn=ADC_CHANNEL_5;
		MfuncInputs[3].mode=MFUNC_ADC;
		
		MfuncInputs[4].port=GPIOF;
		MfuncInputs[4].pin=GPIO_PIN_8;
		MfuncInputs[4].adc=&hadc3;
		MfuncInputs[4].adcChn=ADC_CHANNEL_6;
		MfuncInputs[4].mode=MFUNC_ADC;
		
		MfuncInputs[5].port=GPIOF;
		MfuncInputs[5].pin=GPIO_PIN_9;
		MfuncInputs[5].adc=&hadc3;
		MfuncInputs[5].adcChn=ADC_CHANNEL_7;
		MfuncInputs[5].mode=MFUNC_ADC;
		
		//---
		
		MfuncInputs[6].port=GPIOA;
		MfuncInputs[6].pin=GPIO_PIN_0;
		MfuncInputs[6].adc=&hadc3;
		MfuncInputs[6].adcChn=ADC_CHANNEL_0;
		MfuncInputs[6].mode=MFUNC_ADC;
		
		MfuncInputs[7].port=GPIOA;
		MfuncInputs[7].pin=GPIO_PIN_1;
		MfuncInputs[7].adc=&hadc3;
		MfuncInputs[7].adcChn=ADC_CHANNEL_1;
		MfuncInputs[7].mode=MFUNC_ADC;
		
		MfuncInputs[8].port=GPIOA;
		MfuncInputs[8].pin=GPIO_PIN_2;
		MfuncInputs[8].adc=&hadc3;
		MfuncInputs[8].adcChn=ADC_CHANNEL_2;
		MfuncInputs[8].mode=MFUNC_ADC;
		
		MfuncInputs[9].port=GPIOA;
		MfuncInputs[9].pin=GPIO_PIN_3;
		MfuncInputs[9].adc=&hadc3;
		MfuncInputs[9].adcChn=ADC_CHANNEL_3;
		MfuncInputs[9].mode=MFUNC_ADC;
		
		MfuncInputs[10].port=GPIOA;
		MfuncInputs[10].pin=GPIO_PIN_4;
		MfuncInputs[10].adc=&hadc2;
		MfuncInputs[10].adcChn=ADC_CHANNEL_4;
		MfuncInputs[10].mode=MFUNC_ADC;
		
		MfuncInputs[11].port=GPIOA;
		MfuncInputs[11].pin=GPIO_PIN_5;
		MfuncInputs[11].adc=&hadc2;
		MfuncInputs[11].adcChn=ADC_CHANNEL_5;
		MfuncInputs[11].mode=MFUNC_ADC;
		
		MfuncInputs[12].port=GPIOA;
		MfuncInputs[12].pin=GPIO_PIN_6;
		MfuncInputs[12].adc=&hadc2;
		MfuncInputs[12].adcChn=ADC_CHANNEL_6;
		MfuncInputs[12].mode=MFUNC_ADC;
		
		MfuncInputs[13].port=GPIOA;
		MfuncInputs[13].pin=GPIO_PIN_7;
		MfuncInputs[13].adc=&hadc2;
		MfuncInputs[13].adcChn=ADC_CHANNEL_7;
		MfuncInputs[13].mode=MFUNC_ADC;
		
		Mfunc_Input_SetMode(0xC000000);
		
	HAL_TIM_Base_Start_IT(&htim6);
		xTaskCreate(Mfunc_Task,(signed char*)"Mfunc polling",128,NULL, tskIDLE_PRIORITY + 1, NULL);
}

void Mfunc_Input_SetMode(uint32_t mode)
{
	uint8_t mfunc_count;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	for(mfunc_count=0;mfunc_count<MFUNC_CHANNELS_NUM;mfunc_count++)
	{
			if(MfuncInputs[mfunc_count].mode!=(mode&0x3))
			{										
					MfuncInputs[mfunc_count].mode=mode&0x3;
							
					if(MfuncInputs[mfunc_count].mode==MFUNC_ADC)
					{
						  GPIO_InitStruct.Pin = MfuncInputs[mfunc_count].pin;
							GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
							GPIO_InitStruct.Pull = GPIO_NOPULL;
							HAL_GPIO_Init(MfuncInputs[mfunc_count].port, &GPIO_InitStruct);						
					}
					else
					{
						  GPIO_InitStruct.Pin = MfuncInputs[mfunc_count].pin;
							GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
							GPIO_InitStruct.Pull = GPIO_PULLUP;
							GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
							HAL_GPIO_Init(MfuncInputs[mfunc_count].port, &GPIO_InitStruct);		
					}						
			}
			mode=mode>>2;
	}
}

void Mfunc_TimerInterruptHandler(void)
{
	uint8_t mfunc_count;
	for(mfunc_count=0;mfunc_count<MFUNC_CHANNELS_NUM;mfunc_count++)
	{
			if(MfuncInputs[mfunc_count].mode!=MFUNC_ADC)
			{
					MfuncInputs[mfunc_count].discreteStateOld=MfuncInputs[mfunc_count].discreteState;
					MfuncInputs[mfunc_count].discreteState=HAL_GPIO_ReadPin(MfuncInputs[mfunc_count].port,MfuncInputs[mfunc_count].pin);
				
					if(MfuncInputs[mfunc_count].discreteState)
					{
							MBHoldingRegParams.params.mfuncDiscrete|=(uint16_t)(1<<mfunc_count);
					}
					else
					{
							MBHoldingRegParams.params.mfuncDiscrete&=(~((uint16_t)(1<<mfunc_count)));
					}
					
					if(MfuncInputs[mfunc_count].mode==MFUNC_REDGE)
					{
							if((MfuncInputs[mfunc_count].discreteState==GPIO_PIN_SET)&&(MfuncInputs[mfunc_count].discreteStateOld==GPIO_PIN_RESET))
							{
									MBHoldingRegParams.params.mfuncCount[mfunc_count]++;
							}
					}
					else if(MfuncInputs[mfunc_count].mode==MFUNC_FEDGE)
					{
							if((MfuncInputs[mfunc_count].discreteState==GPIO_PIN_RESET)&&(MfuncInputs[mfunc_count].discreteStateOld==GPIO_PIN_SET))
							{
									MBHoldingRegParams.params.mfuncCount[mfunc_count]++;
							}					
					}				
			}
	}
}

static void Mfunc_Task(void *pvParameters)
{
		TickType_t xLastWakeTime;
		uint8_t adc_chn_count=0;
	
		ADC_ChannelConfTypeDef sConfig;
	//	sConfig.Channel = adc_channel_list[0];
		sConfig.Rank = 1;
		sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	  HAL_ADC_ConfigChannel(&hadc3, &sConfig);
	
		xLastWakeTime = xTaskGetTickCount();
		
	
		while(1)
		{	
			for(adc_chn_count=0;adc_chn_count<MFUNC_CHANNELS_NUM;adc_chn_count++)
			{
					if(MfuncInputs[adc_chn_count].mode==MFUNC_ADC)
					{	
							sConfig.Channel = MfuncInputs[adc_chn_count].adcChn;
							HAL_ADC_ConfigChannel(MfuncInputs[adc_chn_count].adc, &sConfig);
							HAL_ADC_Start(MfuncInputs[adc_chn_count].adc);
							
							//while(!(__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_EOC)));
							
							if (HAL_ADC_PollForConversion(MfuncInputs[adc_chn_count].adc, 10) == HAL_OK && HAL_ADC_GetState(MfuncInputs[adc_chn_count].adc) == HAL_ADC_STATE_EOC_REG)
							{
							//	xSemaphoreTake( xMBInputRegParamsMutex, portMAX_DELAY );
								{
									MBHoldingRegParams.params.mfuncADC[adc_chn_count] = HAL_ADC_GetValue(MfuncInputs[adc_chn_count].adc);
								}
							//	xSemaphoreGive( xMBInputRegParamsMutex );
							}
						
							HAL_ADC_Stop(MfuncInputs[adc_chn_count].adc);
					}
			}
					
			vTaskDelayUntil( &xLastWakeTime, ADC_POLL_PERIOD );			
		}
}