#include "count_input_app.h"
#include "mb_app.h"
#include "stm32f4xx_hal_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define COUNT_INPUT_1_PORT GPIOE
#define COUNT_INPUT_1_PIN	GPIO_PIN_9

#define COUNT_INPUT_2_PORT GPIOE
#define COUNT_INPUT_2_PIN	GPIO_PIN_11

stCountInput CountInput[COUNT_INPUTS_NUM];

static void Count0_Input_Task(void *pvParameters);
static void Count1_Input_Task(void *pvParameters);

void Count_Input_App_Init(void)
{
	 CountInput[0].port=COUNT_INPUT_1_PORT;
	 CountInput[0].pin=COUNT_INPUT_1_PIN;
	 CountInput[0].mode=COUNT_REDGE;
	 CountInput[0].event=0;
	
	 CountInput[1].port=COUNT_INPUT_2_PORT;
	 CountInput[1].pin=COUNT_INPUT_2_PIN;
	 CountInput[1].mode=COUNT_REDGE;
	 CountInput[1].event=0;
	
	xTaskCreate(Count0_Input_Task,(signed char*)"Cnt0 input polling",128,NULL, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(Count1_Input_Task,(signed char*)"Cnt2 input polling",128,NULL, tskIDLE_PRIORITY + 1, NULL);
}

void Count_Input_SetMode(uint8_t mode)
{
	/*uint8_t input_counter;
	for(input_counter=0;input_counter<COUNT_INPUTS_NUM;input_counter++)
	{
			CountInput[input_counter].mode=mode&0x3;
			mode>>=2;
	}*/
	GPIO_InitTypeDef GPIO_InitStruct;
	
	if((mode&0x3)==COUNT_REDGE)
	{
			GPIO_InitStruct.Pin = GPIO_PIN_9;
			GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	}
	else
	{
		  GPIO_InitStruct.Pin = GPIO_PIN_9;
			GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	}
	
  if(((mode>>2)&0x3)==COUNT_REDGE)
	{
			GPIO_InitStruct.Pin = GPIO_PIN_11;
			GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	}
	else
	{
		  GPIO_InitStruct.Pin = GPIO_PIN_11;
			GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	}
}

//#define DELAY_CNT	1000
//uint32_t delay_cnt0,delay_cnt1;

void Count_Input_0_Handler(void)
{
	CountInput[0].event=1;
	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
//	delay_cnt0=DELAY_CNT;
}

void Count_Input_1_Handler(void)
{
	CountInput[1].event=1;
	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
//	delay_cnt1=DELAY_CNT;
}

static void Count0_Input_Task(void *pvParameters)
{
	while(1)
	{		
			if(CountInput[0].event)	
			{			
				vTaskDelay(1);
				MBHoldingRegParams.params.countInputs[0]++;
				CountInput[0].event=0;
				HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
				HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
			}
	}
}

static void Count1_Input_Task(void *pvParameters)
{
	while(1)
	{		
			if(CountInput[1].event)	
			{
				vTaskDelay(1);
				MBHoldingRegParams.params.countInputs[1]++;
				CountInput[1].event=0;
				HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
				HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
			}
	}
}
//void Count_Input_InterruptHandler(void)
//{
//	uint8_t input_counter;
//	for(input_counter=0;input_counter<COUNT_INPUTS_NUM;input_counter++)
//	{
//		CountInput[input_counter].discreteStateOld=CountInput[input_counter].discreteState;	
//		CountInput[input_counter].discreteState=HAL_GPIO_ReadPin(CountInput[input_counter].port,CountInput[input_counter].pin);
//		
//		if(CountInput[input_counter].mode==COUNT_REDGE)
//		{
//				if((CountInput[input_counter].discreteState==GPIO_PIN_SET)&&(CountInput[input_counter].discreteStateOld==GPIO_PIN_RESET))
//				{
//						MBHoldingRegParams.params.countInputs[input_counter]++;
//				}
//		}
//		else if(CountInput[input_counter].mode==COUNT_FEDGE)
//		{
//				if((CountInput[input_counter].discreteState==GPIO_PIN_RESET)&&(CountInput[input_counter].discreteStateOld==GPIO_PIN_SET))
//				{
//						MBHoldingRegParams.params.countInputs[input_counter]++;
//				}					
//		}	

//	}
//}
