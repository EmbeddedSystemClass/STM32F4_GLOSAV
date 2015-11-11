#ifndef CAN_APP_H
#define CAN_APP_H
#include "stm32f4xx_hal.h"
#include "mfunc_app.h"

void CAN_App_Init(void);


typedef	struct 
{
	uint16_t 	vehicleSpeed;
	uint8_t 	calcLoad;
	uint32_t 	totalFuelCons;
	uint8_t  	fuelLevelRel;
	uint16_t	engSpeed;
	uint16_t  axleWeight2;
	uint32_t 	calcTotalEngTime;
	uint16_t 	serviceDist;
	uint16_t 	fuelRate;
} stCANFMSParams;


#endif