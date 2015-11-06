#ifndef MB_APP_H
#define MB_APP_H
#include "stm32f4xx_hal.h"
#include "can_app.h"
#include "adc_app.h"

#define MB_INPUT_BUF_SIZE	126

typedef union
{
	struct 
	{
		stCANFMSParams CANFMSParams;
		uint16_t	adcData[ADC_CHANNELS_NUM];
	} params;
	
	uint16_t usRegInputBuf[MB_INPUT_BUF_SIZE];
}stMBInputRegParams;

extern stMBInputRegParams MBInputRegParams;

#endif

