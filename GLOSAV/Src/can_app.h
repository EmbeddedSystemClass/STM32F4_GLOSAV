#ifndef CAN_APP_H
#define CAN_APP_H
#include "stm32f4xx_hal.h"
#include "adc_app.h"

void CAN_App_Init(void);


typedef	struct 
{
	uint16_t 	weight;
	uint16_t 	combinationWeight;
	uint16_t 	speed;
	uint16_t 	fuelLevel;
} stCANFMSParams;


//extern stCAN_FSM_Params CAN_FSM_Params;

#endif