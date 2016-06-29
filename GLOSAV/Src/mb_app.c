/**
  ******************************************************************************
  * File Name          : mb_app.c
  * Date               : 31/07/2015 10:26:00
  * Description        : 
  ******************************************************************************
  *
  ******************************************************************************
  Изменения в протоколе от 10.06.2016 для передачи свободного места в буфере передачи.
	*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

#include "mb.h"
#include "mbport.h"

#include "tasks.h"

//#include "can_app.h"
#include "mb_app.h"
#include "discrete_output_app.h"
#include "count_input_app.h"
#include "usart_app.h"
#include "fifo.h"

#include <string.h>

#define REG_INPUT_START                 ( 1000 )
#define REG_INPUT_NREGS                 MB_INPUT_BUF_SIZE//( 64 )

#define REG_HOLDING_START               ( 1 )
#define REG_HOLDING_NREGS               ( MB_HOLDING_BUF_SIZE )

// Указатели на FIFO RX от СОМ портов в порядке их нумерации. Если порта нет, то NULL
#define MAX_COM_PORTS_CNT		(7) // максимальное количество адресуемых портов
#define MAX_COM_PORT_BUFFER	(30) // максимальное количество байтов от/к одному порту

//extern stCAN_FSM_Params CAN_FSM_Params;

/* ----------------------- Static variables ---------------------------------*/
USHORT   usRegInputStart = REG_INPUT_START;
//USHORT   *usRegInputBuf[REG_INPUT_NREGS];
USHORT   usRegHoldingStart = REG_HOLDING_START;
USHORT   usRegHoldingBuf[REG_HOLDING_NREGS] = {0x11, 0x22, 0x33, 0x44, 0x66, 0x77};

//stMBInputRegParams MBInputRegParams;
stMBHoldingRegParams MBHoldingRegParams;


static rxfifo_t* RX_FIFO_Handlers[MAX_COM_PORTS_CNT]={
	&uart4RxFifo,
	&uart5RxFifo,
	&uart6RxFifo,
	NULL,	NULL, NULL,	NULL,
};

static txfifo_t* TX_FIFO_Handlers[MAX_COM_PORTS_CNT]={
	&uart4TxFifo,
	&uart5TxFifo,
	&uart6TxFifo,
	NULL,	NULL,	NULL,	NULL,
};

static uint8_t TX_FIFO_MasterFreeSpace[MAX_COM_PORTS_CNT]={0,0,0,0,0,0,0};

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
//						xSemaphoreTake( xMBInputRegParamsMutex, portMAX_DELAY );
//						{	
//								*pucRegBuffer++ = ( unsigned char )( /*usRegInputBuf*/MBInputRegParams.usRegInputBuf[iRegIndex] >> 8 );
//								*pucRegBuffer++ = ( unsigned char )( /*usRegInputBuf*/MBInputRegParams.usRegInputBuf[iRegIndex] & 0xFF );
//						}
//						xSemaphoreGive( xMBInputRegParamsMutex );

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
							//	xSemaphoreTake( xMBHoldingRegParamsMutex, portMAX_DELAY );
								{	
									*pucRegBuffer++ = ( unsigned char )( MBHoldingRegParams.usRegHoldingBuf[iRegIndex] >> 8 );
									*pucRegBuffer++ = ( unsigned char )( MBHoldingRegParams.usRegHoldingBuf[iRegIndex] & 0xFF );
								}
							//	xSemaphoreGive( xMBHoldingRegParamsMutex );
                iRegIndex++;
                usNRegs--;
            }
            break;

        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
							//	xSemaphoreTake( xMBHoldingRegParamsMutex, portMAX_DELAY );
								{	
									MBHoldingRegParams.usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
									MBHoldingRegParams.usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
								}
						//		xSemaphoreGive( xMBHoldingRegParamsMutex );
                iRegIndex++;
                usNRegs--;
            }
						USARTs_SetSpeed(MBHoldingRegParams.params.write.uartBaudRate);
						DiscreteOutputs_SetState(MBHoldingRegParams.params.write.discreteOutputs);
						Mfunc_Input_SetMode(MBHoldingRegParams.params.write.mfuncMode);
						Count_Input_SetMode(MBHoldingRegParams.params.write.countInputsMode);
						
						break;
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

Write mode data: comPortNumber*32+N_bytes - TxFreeSpaceSrc - Byte1 - ... ByteN - comPortNumber*32+N_bytes - ... // data to TX FIFO
Read mode data: comPortNumber*32+N_bytes - TxFreeSpaceDst - Byte1 - ... ByteN - comPortNumber*32+N_bytes - ...	// data from RX FIFO
+ ucBytesReading in read mode.
Не более 7 номеров портов и не более 30 байтов в каждом
Для обеспечения согласования скоростей COM портов и предупреждения переполнения буферов:
TxFreeSpaceSrc - свободное место в буфере указанного порта на передачу у мастера
TxFreeSpaceDst - свободное место в буфере указанного порта на передачу у ведомого (нас)
Формат входных данных уже проверен в головной функции
*/

//extern specData_t specDataIn, specDataOut;

eMBErrorCode
eMBUser100ComPortCB( UCHAR * pucBuffer, UCHAR * ucBytes, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
		txfifo_t* 	fifoTx;
		rxfifo_t* 	fifoRx;
		int16_t  bytesLeft; 	// на всякий случай сделал знаковым, т.к. а вдруг неверный формат, чтобы не зависнуть
		int16_t byte;
		uint8_t portByteIdx, rxCnt, idx, portNumber, portDataCnt;
		uint8_t fifoTxMasterFreeSpace;

		switch ( eMode )
		{
			// считаем содержимое RX_FIFO сом портов и сформируем ответный пакет. 
			// Т.к. у нас не может быть более 7 портов, то проверку на длину пакета не выполняем (не превысим длину в любом случае)!!!
			case MB_REG_READ: 
				idx = 0;
				for(portNumber=0; portNumber<MAX_COM_PORTS_CNT; portNumber++){
					fifoRx = RX_FIFO_Handlers[portNumber];
					fifoTxMasterFreeSpace = TX_FIFO_MasterFreeSpace[portNumber];
					if(fifoRx != NULL){
						portByteIdx = idx;
						idx++; // место для поля TxFreeSpace
						rxCnt = 0;
						while((rxCnt < MAX_COM_PORT_BUFFER) && (rxCnt < fifoTxMasterFreeSpace) && ((byte=rxFifoReadByte(fifoRx)) >= 0 )){
							idx++;
							rxCnt++;
							pucBuffer[idx] = (uint8_t)byte;
						}
//						if(rxCnt != 0){	// если что-то считали и положили, то тогда сформируем заголовок
							pucBuffer[portByteIdx] = (portNumber << 5) |  rxCnt;
							pucBuffer[portByteIdx+1] = txFifoFreeSpace(TX_FIFO_Handlers[portNumber]);
							idx++;
//						}
					}
				}
				* ucBytes = idx;
				break;

		case MB_REG_WRITE:
				idx = 0;
				bytesLeft = *ucBytes;
				while(bytesLeft > 0){
					portNumber = pucBuffer[idx] >> 5;
					fifoTx = TX_FIFO_Handlers[portNumber];
					portDataCnt = pucBuffer[idx] & 0x1F;
					TX_FIFO_MasterFreeSpace[portNumber]=pucBuffer[idx+1]; // сохраним наличие свободного места для передачи наверх данных по этому порту
					idx++; bytesLeft--; // поле порта
					idx++; bytesLeft--; // поле TxFreeSpace
					while(portDataCnt){
						if(fifoTx){
//							xQueueSend(*fifo , &pucBuffer[idx], 0 );
							txFifoPutChar(fifoTx, pucBuffer[idx]);
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
