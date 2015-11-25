#ifndef MB_APP_H
#define MB_APP_H
#include "stm32f4xx_hal.h"
#include "can_app.h"
#include "mfunc_app.h"
#include "usart_app.h"
#include "count_input_app.h"

#define MB_INPUT_BUF_SIZE	125
#define MB_HOLDING_BUF_SIZE	125

//typedef union
//{
//	struct 
//	{
//		stCANFMSParams CANFMSParams;
//		uint16_t	adcData[MFUNC_CHANNELS_NUM];
//	} params;
//	
//	uint16_t usRegInputBuf[MB_INPUT_BUF_SIZE];
//}stMBInputRegParams;



typedef union
{
	struct 
	{
		struct
		{
				uint32_t uartBaudRate[UART_NUM];
				uint8_t discreteOutputs;
				uint32_t mfuncMode;
				uint8_t  countInputsMode;
		}write;
		
		stCANFMSParams CANFMSParams;
		uint16_t mfuncDiscrete;
		uint16_t mfuncADC[MFUNC_CHANNELS_NUM];
		uint32_t mfuncCount[MFUNC_CHANNELS_NUM];
		uint16_t statusInputs;
		uint32_t countInputs[COUNT_INPUTS_NUM];
		uint16_t version;
	} params;
	
	uint16_t usRegHoldingBuf[MB_HOLDING_BUF_SIZE];
}stMBHoldingRegParams;


//extern stMBInputRegParams MBInputRegParams;
extern SemaphoreHandle_t	xMBInputRegParamsMutex;

extern stMBHoldingRegParams MBHoldingRegParams;
extern SemaphoreHandle_t	xMBHoldingRegParamsMutex;

#endif

