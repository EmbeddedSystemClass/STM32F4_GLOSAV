#ifndef CAN_APP_H
#define CAN_APP_H
#include "stm32f4xx_hal.h"

void CAN_App_Init(void);

typedef struct 
{
	uint16_t 	Weight;
	uint16_t 	CombinationWeight;
	uint8_t 	Speed;
} stCAN_FSM_Params;

#endif