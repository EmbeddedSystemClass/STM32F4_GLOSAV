#ifndef MFUNC_APP_H
#define MFUNC_APP_H

#include "stm32f4xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define MFUNC_CHANNELS_NUM	16




enum enMfuncType
{
	MFUNC_ADC=0,
	MFUNC_DIN,
	MFUNC_REDGE,
	MFUNC_FEDGE,
};

typedef struct
{
	GPIO_TypeDef 			*port;
	uint32_t 					pin;
	ADC_HandleTypeDef *adc;
	uint32_t 					adcChn;
	uint8_t 					mode;
	uint8_t   				discreteState;
	uint8_t   				discreteStateOld;
}stMfunc;


void Mfunc_App_Init(void);
void Mfunc_Input_SetMode(uint32_t mode);
void Mfunc_TimerInterruptHandler(void);

#endif
