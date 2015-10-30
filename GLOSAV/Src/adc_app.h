#ifndef ADC_APP_H
#define ADC_APP_H

#include "stm32f4xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define ADC_CHANNELS_NUM	3

extern uint16_t adc_channels_data[ADC_CHANNELS_NUM];
extern SemaphoreHandle_t	xADC_DataMutex;

void ADC_App_Init(void);

#endif