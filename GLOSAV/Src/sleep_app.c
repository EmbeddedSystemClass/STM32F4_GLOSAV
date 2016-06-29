#include "sleep_app.h"
#include "stm32f4xx_hal.h"

#include "adc.h"
#include "gpio.h"
#include "can.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"

typedef enum 
{
	MODE_RUN,
	MODE_SLEEP
} enSleep;


volatile enSleep sleep;

void Sleep_Mode_Sleep(void)
{
	/*отключить все прерывания, кроме UART1*/
	HAL_ADC_DeInit(&hadc2);
	HAL_ADC_DeInit(&hadc3);
	HAL_CAN_DeInit(&hcan1);
	HAL_CAN_DeInit(&hcan2);
	HAL_I2C_DeInit(&hi2c2);
	HAL_TIM_Base_DeInit(&htim2);
	HAL_TIM_Base_DeInit(&htim6);
	
	HAL_UART_DeInit(&huart2);
	HAL_UART_DeInit(&huart3);
	HAL_UART_DeInit(&huart4);
	HAL_UART_DeInit(&huart5);
	HAL_UART_DeInit(&huart6);
	
	SysTick->CTRL&=	~(SysTick_CTRL_TICKINT_Msk  | SysTick_CTRL_ENABLE_Msk);

	sleep=MODE_SLEEP;
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI);
}


void Sleep_Mode_WakeUp(void)
{
	sleep=MODE_RUN;
	
	HAL_ADC_Init(&hadc2);
	HAL_ADC_Init(&hadc3);
	HAL_CAN_Init(&hcan1);
	HAL_CAN_Init(&hcan2);
	HAL_I2C_Init(&hi2c2);
	HAL_TIM_Base_Init(&htim2);
	HAL_TIM_Base_Init(&htim6);
	
	HAL_UART_Init(&huart2);
	HAL_UART_Init(&huart3);
	HAL_UART_Init(&huart4);
	HAL_UART_Init(&huart5);
	HAL_UART_Init(&huart6);
	
	SysTick->CTRL|=	(SysTick_CTRL_TICKINT_Msk  | SysTick_CTRL_ENABLE_Msk);
}
