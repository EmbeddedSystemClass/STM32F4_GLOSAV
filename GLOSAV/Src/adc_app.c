#include "adc_app.h"
#include "adc.h"



#define ADC_POLL_PERIOD		100

uint16_t adc_channels_data[ADC_CHANNELS_NUM];
const uint8_t adc_channel_list[ADC_CHANNELS_NUM]={ADC_CHANNEL_8,ADC_CHANNEL_14,ADC_CHANNEL_15};
SemaphoreHandle_t	xADC_DataMutex;

static void ADC_Task(void *pvParameters);

void ADC_App_Init(void)
{
		xADC_DataMutex = xSemaphoreCreateMutex();
		xTaskCreate(ADC_Task,(signed char*)"ADC polling",128,NULL, tskIDLE_PRIORITY + 1, NULL);
}

static void ADC_Task(void *pvParameters)
{
		TickType_t xLastWakeTime;
		uint8_t adc_chn_count=0;
	
		ADC_ChannelConfTypeDef sConfig;
		sConfig.Channel = adc_channel_list[0];
		sConfig.Rank = 1;
		sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	  HAL_ADC_ConfigChannel(&hadc3, &sConfig);
	
		xLastWakeTime = xTaskGetTickCount();
		
	
		while(1)
		{

			
			for(adc_chn_count=0;adc_chn_count<ADC_CHANNELS_NUM;adc_chn_count++)
			{
					sConfig.Channel = adc_channel_list[adc_chn_count];
					HAL_ADC_ConfigChannel(&hadc3, &sConfig);
					HAL_ADC_Start(&hadc3);
					
					//while(!(__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_EOC)));
					
					if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK && HAL_ADC_GetState(&hadc3) == HAL_ADC_STATE_EOC_REG)
					{
						xSemaphoreTake( xADC_DataMutex, portMAX_DELAY );
						{
							adc_channels_data[adc_chn_count] = HAL_ADC_GetValue(&hadc3);
						}
						xSemaphoreGive( xADC_DataMutex );
				  }
				
					HAL_ADC_Stop(&hadc3);
			}
					
			vTaskDelayUntil( &xLastWakeTime, ADC_POLL_PERIOD );			
		}
}