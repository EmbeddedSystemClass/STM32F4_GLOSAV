#ifndef CAN_APP_H
#define CAN_APP_H
#include "stm32f4xx_hal.h"

void CAN_App_Init(void);

#define CAN_STRUCT_MB_BUF_SIZE	32

typedef union
{
	struct 
	{
		uint16_t 	Weight;
		uint16_t 	CombinationWeight;
		uint16_t 	Speed;
		uint16_t 	FuelLevel;
	} params;
	
	uint16_t can_mb_buf[CAN_STRUCT_MB_BUF_SIZE];
}stCAN_FSM_Params;

#endif