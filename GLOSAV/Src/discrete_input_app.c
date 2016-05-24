#include "discrete_input_app.h"

#include "stm32f4xx_hal.h"
#include "mb_app.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


#define DISCR_INPUT_DELAY		100

enum
{
	DISCR_IN_IGN_BIT=0,
	DISCR_IN_BUTTON_CALL_BIT,
	DISCR_IN_CASE_OPEN_SW_BIT,
	DISCR_IN_CASE_OPEN_SW2_BIT,
	DISCR_IN_CASE_OPEN_O_BIT,
	DISCR_IN_CASE_OPEN_O2_BIT,
};

#define DISCR_IN_IGN_PORT							GPIOF
#define DISCR_IN_IGN_PIN							GPIO_PIN_3

#define DISCR_IN_BUTTON_CALL_PORT			GPIOE
#define DISCR_IN_BUTTON_CALL_PIN			GPIO_PIN_6

#define DISCR_IN_CASE_OPEN_SW_PORT		GPIOF
#define DISCR_IN_CASE_OPEN_SW_PIN			GPIO_PIN_14

#define DISCR_IN_CASE_OPEN_SW2_PORT		GPIOG
#define DISCR_IN_CASE_OPEN_SW2_PIN		GPIO_PIN_1

#define DISCR_IN_CASE_OPEN_O_PORT			GPIOF
#define DISCR_IN_CASE_OPEN_O_PIN			GPIO_PIN_13

#define DISCR_IN_CASE_OPEN_O2_PORT		GPIOG
#define DISCR_IN_CASE_OPEN_O2_PIN			GPIO_PIN_0

static void Discrete_Input_Task(void *pvParameters);

void Discrete_Input_Init(void)
{
	xTaskCreate(Discrete_Input_Task,(signed char*)"Discr input polling",128,NULL, tskIDLE_PRIORITY + 1, NULL);
}


static void Discrete_Input_Task(void *pvParameters)
{
	uint8_t temp_bit;
	while(1)
	{
			vTaskDelay(DISCR_INPUT_DELAY);
				
			temp_bit=HAL_GPIO_ReadPin(DISCR_IN_IGN_PORT,DISCR_IN_IGN_PIN);
			vTaskDelay(1);
			if(temp_bit==HAL_GPIO_ReadPin(DISCR_IN_IGN_PORT,DISCR_IN_IGN_PIN))
			{
					//xSemaphoreTake( xMBHoldingRegParamsMutex, portMAX_DELAY );
					{	
							MBHoldingRegParams.params.statusInputs&=(~(1<<DISCR_IN_IGN_BIT));
							MBHoldingRegParams.params.statusInputs|=(temp_bit<<DISCR_IN_IGN_BIT);
					}
					//xSemaphoreGive( xMBHoldingRegParamsMutex );
			}
			
			
			temp_bit=HAL_GPIO_ReadPin(DISCR_IN_BUTTON_CALL_PORT,DISCR_IN_BUTTON_CALL_PIN);
			vTaskDelay(1);
			if(temp_bit==HAL_GPIO_ReadPin(DISCR_IN_BUTTON_CALL_PORT,DISCR_IN_BUTTON_CALL_PIN))
			{
					//xSemaphoreTake( xMBHoldingRegParamsMutex, portMAX_DELAY );
					{	
							MBHoldingRegParams.params.statusInputs&=(~(1<<DISCR_IN_BUTTON_CALL_BIT));
							MBHoldingRegParams.params.statusInputs|=(temp_bit<<DISCR_IN_BUTTON_CALL_BIT);
					}
					//xSemaphoreGive( xMBHoldingRegParamsMutex );
			}
			
			
			temp_bit=HAL_GPIO_ReadPin(DISCR_IN_CASE_OPEN_SW_PORT,DISCR_IN_CASE_OPEN_SW_PIN);
			vTaskDelay(1);
			if(temp_bit==HAL_GPIO_ReadPin(DISCR_IN_CASE_OPEN_SW_PORT,DISCR_IN_CASE_OPEN_SW_PIN))
			{
					//xSemaphoreTake( xMBHoldingRegParamsMutex, portMAX_DELAY );
					{	
							MBHoldingRegParams.params.statusInputs&=(~(1<<DISCR_IN_CASE_OPEN_SW_BIT));
							MBHoldingRegParams.params.statusInputs|=(temp_bit<<DISCR_IN_CASE_OPEN_SW_BIT);
					}
					//xSemaphoreGive( xMBHoldingRegParamsMutex );
			}
			
			temp_bit=HAL_GPIO_ReadPin(DISCR_IN_CASE_OPEN_SW2_PORT,DISCR_IN_CASE_OPEN_SW2_PIN);
			vTaskDelay(1);
			if(temp_bit==HAL_GPIO_ReadPin(DISCR_IN_CASE_OPEN_SW2_PORT,DISCR_IN_CASE_OPEN_SW2_PIN))
			{
					//xSemaphoreTake( xMBHoldingRegParamsMutex, portMAX_DELAY );
					{	
							MBHoldingRegParams.params.statusInputs&=(~(1<<DISCR_IN_CASE_OPEN_SW2_BIT));
							MBHoldingRegParams.params.statusInputs|=(temp_bit<<DISCR_IN_CASE_OPEN_SW2_BIT);
					}
					//xSemaphoreGive( xMBHoldingRegParamsMutex );
			}
			
			temp_bit=HAL_GPIO_ReadPin(DISCR_IN_CASE_OPEN_O_PORT,DISCR_IN_CASE_OPEN_O_PIN);
			vTaskDelay(1);
			if(temp_bit==HAL_GPIO_ReadPin(DISCR_IN_CASE_OPEN_O_PORT,DISCR_IN_CASE_OPEN_O_PIN))
			{
					//xSemaphoreTake( xMBHoldingRegParamsMutex, portMAX_DELAY );
					{	
							MBHoldingRegParams.params.statusInputs&=(~(1<<DISCR_IN_CASE_OPEN_O_BIT));
							MBHoldingRegParams.params.statusInputs|=(temp_bit<<DISCR_IN_CASE_OPEN_O_BIT);
					}
					//xSemaphoreGive( xMBHoldingRegParamsMutex );
			}
			
			temp_bit=HAL_GPIO_ReadPin(DISCR_IN_CASE_OPEN_O2_PORT,DISCR_IN_CASE_OPEN_O2_PIN);
			vTaskDelay(1);
			if(temp_bit==HAL_GPIO_ReadPin(DISCR_IN_CASE_OPEN_O2_PORT,DISCR_IN_CASE_OPEN_O2_PIN))
			{
					//xSemaphoreTake( xMBHoldingRegParamsMutex, portMAX_DELAY );
					{	
							MBHoldingRegParams.params.statusInputs&=(~(1<<DISCR_IN_CASE_OPEN_O2_BIT));
							MBHoldingRegParams.params.statusInputs|=(temp_bit<<DISCR_IN_CASE_OPEN_O2_BIT);
					}
					//xSemaphoreGive( xMBHoldingRegParamsMutex );
			}
	}
}