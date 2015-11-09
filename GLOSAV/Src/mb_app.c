/**
  ******************************************************************************
  * File Name          : mb_app.c
  * Date               : 31/07/2015 10:26:00
  * Description        : 
  ******************************************************************************
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

#include "mb.h"
#include "mbport.h"

#include "tasks.h"

//#include "can_app.h"
#include "mb_app.h"

#include <string.h>

#define REG_INPUT_START                 ( 1000 )
#define REG_INPUT_NREGS                 MB_INPUT_BUF_SIZE//( 64 )

#define REG_HOLDING_START               ( 1 )
#define REG_HOLDING_NREGS               ( 32 )

// Указатели на FIFO RX от СОМ портов в порядке их нумерации. Если порта нет, то NULL
#define MAX_COM_PORTS_CNT		(16) // максимальное количество адресуемых портов
#define MAX_COM_PORT_BUFFER	(15) // максимальное количество байтов от/к одному порту

//extern stCAN_FSM_Params CAN_FSM_Params;

/* ----------------------- Static variables ---------------------------------*/
USHORT   usRegInputStart = REG_INPUT_START;
//USHORT   *usRegInputBuf[REG_INPUT_NREGS];
USHORT   usRegHoldingStart = REG_HOLDING_START;
USHORT   usRegHoldingBuf[REG_HOLDING_NREGS] = {0x11, 0x22, 0x33, 0x44, 0x66, 0x77};

stMBInputRegParams MBInputRegParams;
stMBHoldingRegParams MBHoldingRegParams;


static osMessageQId* RX_FIFO_Handlers[MAX_COM_PORTS_CNT]={
	&myQueueUart4RxHandle,
	&myQueueUart5RxHandle,
	&myQueueUart6RxHandle,
	NULL,
	NULL, NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, NULL,	NULL,
};

static osMessageQId* TX_FIFO_Handlers[MAX_COM_PORTS_CNT]={
	&myQueueUart4TxHandle,
	&myQueueUart5TxHandle,
	&myQueueUart6TxHandle,
	NULL,
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,
};

SemaphoreHandle_t	xMBInputRegParamsMutex;
SemaphoreHandle_t	xMBHoldingRegParamsMutex;

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_INPUT_START ) && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {
						xSemaphoreTake( xMBInputRegParamsMutex, portMAX_DELAY );
						{	
								*pucRegBuffer++ = ( unsigned char )( /*usRegInputBuf*/MBInputRegParams.usRegInputBuf[iRegIndex] >> 8 );
								*pucRegBuffer++ = ( unsigned char )( /*usRegInputBuf*/MBInputRegParams.usRegInputBuf[iRegIndex] & 0xFF );
						}
						xSemaphoreGive( xMBInputRegParamsMutex );

            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_HOLDING_START ) && ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        switch ( eMode )
        {
        case MB_REG_READ:
            while( usNRegs > 0 )
            {
                *pucRegBuffer++ = ( unsigned char )( MBHoldingRegParams.usRegHoldingBuf[iRegIndex] >> 8 );
                *pucRegBuffer++ = ( unsigned char )( MBHoldingRegParams.usRegHoldingBuf[iRegIndex] & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
            break;

        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                MBHoldingRegParams.usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                MBHoldingRegParams.usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/*
Чтение - запись FIFO СОМ портов

Write mode data: comPortNumber*16+N_bytes - Byte1 - ... ByteN - comPortNumber*16+N_bytes - ... // data to TX FIFO
Read mode data: comPortNumber*16+N_bytes - Byte1 - ... ByteN - comPortNumber*16+N_bytes - ...	// data from RX FIFO
+ ucBytesReading in read mode.
Не более 16 номеров портов и не более 16 байтов в каждом
Формат входных данных уже проверен в головной функции
*/
eMBErrorCode
eMBUser100ComPortCB( UCHAR * pucBuffer, UCHAR * ucBytes, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
		osMessageQId* 	fifo;
		int16_t  bytesLeft; 	// на всякий случай сделал знаковым, т.к. а вдруг неверный формат, чтобы не зависнуть
		uint8_t byte;
		uint8_t portByteIdx, rxCnt, idx, portNumber, portDataCnt;


		switch ( eMode )
		{
			// считаем содержимое RX_FIFO сом портов и сформируем ответный пакет. 
			// Т.к. у нас не может быть 16 портов, то проверку на длину пакета не выполняем !!!
			case MB_REG_READ: 
				idx = 0;
				for(portNumber=0; portNumber<MAX_COM_PORTS_CNT; portNumber++){
					fifo = RX_FIFO_Handlers[portNumber];
					if(fifo != NULL){
						portByteIdx = idx;
						rxCnt = 0;
						while((rxCnt < MAX_COM_PORT_BUFFER) && (xQueueReceive( *fifo, &byte, 0 ) == pdTRUE )){
							idx++;
							rxCnt++;
							pucBuffer[idx] = byte;
						}
						if(rxCnt != 0){	// если что-то считали и положили, то тогда сформируем заголовок
							pucBuffer[portByteIdx] = (portNumber << 4) |  rxCnt;
							idx++;
						}
					}
				}
				//--------------------------
//				pucBuffer[idx]=(MAX_COM_PORTS_CNT << 4) | sizeof(stCAN_FSM_Params);
//				xSemaphoreTake( xCAN1_DataMutex, portMAX_DELAY );//сформируем пакет по CANу
//				{	
//					 memcpy(&pucBuffer[idx+1],(uint8_t*)(&CAN_FSM_Params),sizeof(stCAN_FSM_Params));
//				}
//				xSemaphoreGive( xCAN1_DataMutex );
//				idx+=(1+sizeof(stCAN_FSM_Params));
				//---------------------------
				
				* ucBytes = idx;
				break;

		case MB_REG_WRITE:
				idx = 0;
				bytesLeft = *ucBytes;
				while(bytesLeft > 0){
					portNumber = pucBuffer[idx] >> 4;
					fifo = TX_FIFO_Handlers[portNumber];
					portDataCnt = pucBuffer[idx] & 0x0F;
					idx++; bytesLeft--;
					while(portDataCnt){
						if(fifo){
							xQueueSend(*fifo , &pucBuffer[idx], 0 );
						}
						idx++; bytesLeft--; portDataCnt--;
					}
				}
		}
    return eStatus;
}


eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{
    return MB_ENOREG;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    return MB_ENOREG;
}
