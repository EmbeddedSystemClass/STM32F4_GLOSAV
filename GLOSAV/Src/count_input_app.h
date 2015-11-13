#ifndef COUNT_INPUT_APP_H
#define COUNT_INPUT_APP_H
#include "stm32f4xx_hal.h"

#define COUNT_INPUTS_NUM	2

enum enCountType
{
	COUNT_REDGE,
	COUNT_FEDGE,
};

typedef struct
{
	GPIO_TypeDef 			*port;
	uint32_t 					pin;
	uint8_t 					mode;
	uint8_t   				discreteState;
	uint8_t   				discreteStateOld;
}stCountInput;

void Count_Input_App_Init(void);
void Count_Input_SetMode(uint8_t mode);
void Count_Input_InterruptHandler(void);

#endif