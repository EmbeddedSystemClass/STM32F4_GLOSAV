/* 
 * FreeModbus Libary: A portable Modbus implementation for Modbus ASCII/RTU.
 * Copyright (c) 2006 Christian Walter <wolti@sil.at>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * File: $Id: mbfuncuser.c,v 1.1 2015/08/06 23:48:22 Bykov Exp $
 */
/*
Изменения в протоколе от 10.06.2016 для передачи свободного места в буфере передачи.
*/
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbconfig.h"

/* ----------------------- Defines ------------------------------------------*/

#define MB_PDU_FUNC_USER100_DATA_OFF     	( MB_PDU_DATA_OFF + 1 )
#define MB_PDU_FUNC_USER100_SIZE_MIN          ( 1 ) // минимально должно быть поле длины данных для функции 100

/* ----------------------- Static functions ---------------------------------*/
eMBException    prveMBError2Exception( eMBErrorCode eErrorCode );

/* ----------------------- Start implementation -----------------------------*/

/*
packet format:
func - length_data - dataSpecific
func=100
read-write Rx-TxFIFO Com ports: 
command data: comPortNumber*32+N_bytes - TxFreeSpaceSrc - Byte1 - ... ByteN - comPortNumber*32+N_bytes - ... // data to TX FIFO
answer data: comPortNumber*32+N_bytes - TxFreeSpaceDst - Byte1 - ... ByteN - comPortNumber*32+N_bytes - ...	// data from RX FIFO

Для обеспечения согласования скоростей COM портов и предупреждения переполнения буферов:
TxFreeSpaceSrc - свободное место в буфере указанного порта на передачу у мастера
TxFreeSpaceDst - свободное место в буфере указанного порта на передачу у ведомого (нас)
*/

eMBException
eMBFuncUser100( UCHAR * pucFrame, USHORT * usLen )
{
//    USHORT          usRegReadAddress;
//    USHORT          usRegReadCount;
//    USHORT          usRegWriteAddress;
//    USHORT          usRegWriteCount;
    UCHAR           ucByteCnt;
    UCHAR          	*pucFrameCur;
    SHORT          	idx, bytesLeft;
	  UCHAR						errFormat = 0;
    SHORT           portDataCnt;

    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;

    if( *usLen >= ( MB_PDU_FUNC_USER100_SIZE_MIN + MB_PDU_SIZE_MIN ) )
    {
			// проверим сначала корректность данных (посчитаем порты и данные)
			idx = MB_PDU_FUNC_USER100_DATA_OFF;
			bytesLeft = *usLen - idx;
			ucByteCnt = 0;
			while(bytesLeft && !errFormat){
				portDataCnt = pucFrame[idx] & 0x1F;
				idx+=2; bytesLeft-=2; ucByteCnt+=2; // учитываем поле порта и статуса (TxFreeSpace)
				if(portDataCnt > bytesLeft){
					errFormat = 1;
					break;
				}
				idx+=portDataCnt;
				ucByteCnt+=portDataCnt;
				bytesLeft-=portDataCnt;
			}
			if(errFormat){
				/* Can't be a valid request because the length is incorrect. */
				eStatus = MB_EX_ILLEGAL_DATA_VALUE;
				return eStatus;
			}
			
      /* Make callback to update the TX FIFO values. */
      eRegStatus = eMBUser100ComPortCB( &pucFrame[MB_PDU_FUNC_USER100_DATA_OFF], &ucByteCnt, MB_REG_WRITE );
			if( eRegStatus == MB_ENOERR )	{
					/* Set the current PDU data pointer to the beginning. */
					pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
					*usLen = MB_PDU_FUNC_OFF;

					/* First byte contains the function code. */
					*pucFrameCur++ = MB_FUNC_USER_100;
					*usLen += 1;

					/* Second byte in the response contain the length of data field. */
					*pucFrameCur++ = 0;
					*usLen += 1;

					/* Make the read callback. */
					eRegStatus =
							eMBUser100ComPortCB( pucFrameCur, &ucByteCnt, MB_REG_READ );
					if( eRegStatus == MB_ENOERR )	{
							*usLen += ucByteCnt;
							pucFrame[MB_PDU_FUNC_OFF+1] = ucByteCnt; /* Second byte in the response contain the length of data field. */
					}
			}
			if( eRegStatus != MB_ENOERR )
			{
					eStatus = prveMBError2Exception( eRegStatus );
			}
		}
		else
		{
				eStatus = MB_EX_ILLEGAL_DATA_VALUE;
		}
    return eStatus;
}


