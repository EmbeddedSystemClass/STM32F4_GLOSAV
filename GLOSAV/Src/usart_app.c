/**
  ******************************************************************************
  * File Name          : USART_app.c
  * Date               : 31/07/2015 10:26:00
  * Description        : 
  ******************************************************************************
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "usart_app.h"
#include "gpio.h"
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "tasks.h"
#include "port.h"
#include "fifo.h"

#define UART2_RX_FIFO_SIZE		128
#define UART2_TX_FIFO_SIZE		128
#define UART3_RX_FIFO_SIZE		128
#define UART3_TX_FIFO_SIZE		128
#define UART4_RX_FIFO_SIZE		128
#define UART4_TX_FIFO_SIZE		128
#define UART5_RX_FIFO_SIZE		128
#define UART5_TX_FIFO_SIZE		128
#define UART6_RX_FIFO_SIZE		128
#define UART6_TX_FIFO_SIZE		128


extern void prvvUARTTxReadyISR( void );
extern void prvvUARTRxISR( void );
extern volatile UCHAR ucRTUBuf[];


FILE *sim;
// Буферы приема передачи по UART

rxfifo_t uart2RxFifo;
txfifo_t uart2TxFifo;
uint8_t uart2Rxbuf[UART2_RX_FIFO_SIZE];
uint8_t uart2Txbuf1[UART2_TX_FIFO_SIZE];
uint8_t uart2Txbuf2[UART2_TX_FIFO_SIZE];

rxfifo_t uart3RxFifo;
txfifo_t uart3TxFifo;
uint8_t uart3Rxbuf[UART3_RX_FIFO_SIZE];
uint8_t uart3Txbuf1[UART3_TX_FIFO_SIZE];
uint8_t uart3Txbuf2[UART3_TX_FIFO_SIZE];

rxfifo_t uart4RxFifo;
txfifo_t uart4TxFifo;
uint8_t uart4Rxbuf[UART4_RX_FIFO_SIZE];
uint8_t uart4Txbuf1[UART4_TX_FIFO_SIZE];
uint8_t uart4Txbuf2[UART4_TX_FIFO_SIZE];

rxfifo_t uart5RxFifo;
txfifo_t uart5TxFifo;
uint8_t uart5Rxbuf[UART5_RX_FIFO_SIZE];
uint8_t uart5Txbuf1[UART5_TX_FIFO_SIZE];
uint8_t uart5Txbuf2[UART5_TX_FIFO_SIZE];

rxfifo_t uart6RxFifo;
txfifo_t uart6TxFifo;
uint8_t uart6Rxbuf[UART6_RX_FIFO_SIZE];
uint8_t uart6Txbuf1[UART6_TX_FIFO_SIZE];
uint8_t uart6Txbuf2[UART6_TX_FIFO_SIZE];

void uart4dirSet (void){ UART4_DIR_SEND(); }
void uart5dirSet (void){ UART5_DIR_SEND(); }
void uart6dirSet (void){ UART6_DIR_SEND(); }

void uart4dirReset (void){ UART4_DIR_RCV(); }
void uart5dirReset (void){ UART5_DIR_RCV(); }
void uart6dirReset (void){ UART6_DIR_RCV(); }

int fputc(int ch, FILE *f)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
//  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

	if(f==sim)
	{
//		HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
	}
	else
	{
//		xQueueSend( myQueueUart2TxHandle, &ch, portMAX_DELAY  );
		txFifoPutChar(&uart2TxFifo, (uint8_t)ch);
//		HAL_UART_Transmit(&huart4, (uint8_t *)&ch, 1, 0xFFFF);
	}
  return ch;
}

void uartsFifoInit(void)
{
	rxFifoInit(&huart2, &uart2RxFifo, uart2Rxbuf, sizeof(uart2Rxbuf));
	txFifoInit(&huart2, &uart2TxFifo, uart2Txbuf1, uart2Txbuf2, sizeof(uart2Txbuf1));

	rxFifoInit(&huart3, &uart3RxFifo, uart3Rxbuf, sizeof(uart3Rxbuf));
	txFifoInit(&huart3, &uart3TxFifo, uart3Txbuf1, uart3Txbuf2, sizeof(uart3Txbuf1));

	rxFifoInit(&huart4, &uart4RxFifo, uart4Rxbuf, sizeof(uart4Rxbuf));
	txFifoInit(&huart4, &uart4TxFifo, uart4Txbuf1, uart4Txbuf2, sizeof(uart4Txbuf1));
	uart4TxFifo.pBeforeTransmitCallback = uart4dirSet;
	uart4TxFifo.pEndTransmitCallback = uart4dirReset;

	rxFifoInit(&huart5, &uart5RxFifo, uart5Rxbuf, sizeof(uart5Rxbuf));
	txFifoInit(&huart5, &uart5TxFifo, uart5Txbuf1, uart5Txbuf2, sizeof(uart5Txbuf1));
	uart5TxFifo.pBeforeTransmitCallback = uart5dirSet;
	uart5TxFifo.pEndTransmitCallback = uart5dirReset;

	rxFifoInit(&huart6, &uart6RxFifo, uart6Rxbuf, sizeof(uart6Rxbuf));
	txFifoInit(&huart6, &uart6TxFifo, uart6Txbuf1, uart6Txbuf2, sizeof(uart6Txbuf1));
	uart6TxFifo.pBeforeTransmitCallback = uart6dirSet;
	uart6TxFifo.pEndTransmitCallback = uart6dirReset;
}

void startUARTRcv(UART_HandleTypeDef *huart)
{
// используем функцию приема по DMA, поэтому здесь просто запускаем прием по интерфейсам
	switch((uint32_t)huart->Instance)
	{
		case (uint32_t)USART1:	// Порт к ВВ работает по DMA!
		/*!!!!!! MB_SER_PDU_SIZE_MAX = 256 - mbrtu.c. Константу согласовывать! Bykov_DMA !!!!!!*/
			HAL_UART_Receive_DMA(&huart1, (uint8_t*)&ucRTUBuf, 256);
    /* Enable the UART Data Register not empty Interrupt 
		Разрешим прерывания для контроля паузы между приемом байтов (конец фрейма) с помощью таймера */
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
		break;
		case (uint32_t)USART2:
			rxFifoStart(&uart2RxFifo);
			break;
		case (uint32_t)USART3:
			rxFifoStart(&uart3RxFifo);
			break;
		case (uint32_t)UART4:
			rxFifoStart(&uart4RxFifo);
			break;
		case (uint32_t)UART5:
			rxFifoStart(&uart5RxFifo);
			break;
		case (uint32_t)USART6:
			rxFifoStart(&uart6RxFifo);
			break;
	}
}

/*
Останов приема и передачи при обмене с ВВ (остановить только прием при DMA проблематично)
Для полудуплекса вполне допустимо.
*/
void stopUART(UART_HandleTypeDef *huart)
{
	switch((uint32_t)huart->Instance)
	{
		case (uint32_t)USART1:
			HAL_UART_DMAStop(&huart1);
    __HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE);
		break;
	}
	// остальные порты не останавливаются!
}

/*
Возвращаем количество принятых байтов по DMA по порту BB
*/
uint16_t getUART_BB_RcvDMAPktLength(void)
{
	return huart1.RxXferSize - huart1.hdmarx->Instance->NDTR;
}

// Обработчик приема символов по СОМ портам (терминал и СпМ) в режиме прерываний
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) // from interrupt called!
{ 
//	int i, cnt;
//	uint8_t *pData;
	switch((uint32_t)UartHandle->Instance)
	{
		case (uint32_t)USART1: // При обмене с ВВ по DMA, попадем сюда после останова DMA по таймеру паузы
			// обработка буфера modbus идет в библиотеке и инициируется в обработчике таймера!
			
		// ДЛЯ ТЕСТА: определим сначала длину фрейма и передадим на обработку
/*		  cnt=getUART_BB_RcvDMAPktLength();
		  for(i=0, pData=UartHandle->pRxBuffPtr; i<cnt;i++,pData++){
				xQueueSendFromISR( myQueueUart6TxHandle , pData, NULL  );  
			}*/
		  break;
	}
}

// в случае ошибок просто взведем снова прием, т.к. иначе все остановится
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	startUARTRcv(huart);
}


// вызывается из прерывания после полного завершения передачи по СОМ порту
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	switch((uint32_t)huart->Instance)
	{
		case (uint32_t)USART1:	// modbus
			prvvUARTTxReadyISR();
			break;
		case (uint32_t)USART2:
			txFifoCpltCallback(&uart2TxFifo);
			break;
		case (uint32_t)USART3:
			txFifoCpltCallback(&uart3TxFifo);
			break;
		case (uint32_t)UART4:	
			txFifoCpltCallback(&uart4TxFifo);
			break;
		case (uint32_t)UART5:	
			txFifoCpltCallback(&uart5TxFifo);
			break;
		case (uint32_t)USART6:
			txFifoCpltCallback(&uart6TxFifo);
			break;
	}

}


UART_HandleTypeDef *huart[UART_NUM]={&huart4,&huart5,&huart6, NULL, NULL};

#define USART_MIN_BAUDRATE	1200
#define USART_MAX_BAUDRATE	115200

void USARTs_SetSpeed(uint32_t *uartBaudRate)
{
	uint8_t uart_count=0;
	
	for(uart_count=0;uart_count<UART_NUM;uart_count++)
	{
		if((huart[uart_count] != NULL) && 
			(huart[uart_count]->Init.BaudRate!=uartBaudRate[uart_count])&&
			(uartBaudRate[uart_count]>=USART_MIN_BAUDRATE)&&
			(uartBaudRate[uart_count]<=USART_MAX_BAUDRATE))
		{
				huart[uart_count]->Init.BaudRate=uartBaudRate[uart_count];
				HAL_UART_Init(huart[uart_count]);
		}
		else
		{
				uartBaudRate[uart_count]=huart[uart_count]->Init.BaudRate;
		}
	}	
}

