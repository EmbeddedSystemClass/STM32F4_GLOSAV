#ifndef CAN_APP_H
#define CAN_APP_H
#include "stm32f4xx_hal.h"
#include "mfunc_app.h"

#define AXLE_NUM	8

void CAN_App_Init(void);

#pragma pack(push,1) 
typedef	struct 
{
	uint16_t 	vehicleSpeed;//скорость тс 1/256 км/ч на бит, значение FMS 10250
	uint8_t 	calcLoad; //нагрузка на двигатель 1% на бит, значение FMS 255
	uint32_t 	totalFuelCons;//расход топлива расчетный 0.5л на бит, значение FMS 224778
	uint8_t  	fuelLevelRel;// относительный уровень топлива в баке 0.4% на бит, значение FMS 25
	uint16_t	engSpeed;//обороты двигателя 0.125 об/сек на бит, значение FMS 6400
	uint16_t  	axleWeight[AXLE_NUM];//нагрузка на оси, 0.5 кг на бит, значение FMS не определено
	uint32_t 	calcTotalEngTime;// моточасы расчетные, 0.05 часа на бит, значение FMS 132413
	uint16_t 	serviceDist;//Пробег до ТО, 5 км на бит,  значение FMS 36833
	uint16_t 	fuelRate;// расход топлива, 0,05 л/ч на бит, параметр не передается FMS
} stCANFMSParams;

#pragma pack(pop) 

#endif