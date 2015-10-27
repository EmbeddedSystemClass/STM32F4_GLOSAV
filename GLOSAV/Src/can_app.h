#ifndef CAN_APP_H
#define CAN_APP_H
#include "stm32f4xx_hal.h"

void CAN_App_Init(void);

typedef struct 
{
	uint32_t VehicleWeight;
	
} stCAN_FSM_Params;

#endif