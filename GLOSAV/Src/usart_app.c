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

extern void prvvUARTTxReadyISR( void );
extern void prvvUARTRxISR( void );
extern volatile UCHAR ucRTUBuf[];


FILE *sim;
volatile uint8_t chUART3, chUART1, chUART2, chUART4, chUART5, chUART6;

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
		xQueueSend( myQueueUart2TxHandle, &ch, portMAX_DELAY  );
//		HAL_UART_Transmit(&huart4, (uint8_t *)&ch, 1, 0xFFFF);
	}
  return ch;
}

void startUARTRcv(UART_HandleTypeDef *huart)
{
// пспользуем модифицированную функцию приема по прерыванию, поэтому здесь просто запускаем прием по интерфейсам
	// Обработка принятых байтов - в функции HAL_UART_RxCpltCallback()
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
			HAL_UART_Receive_IT(&huart2, (uint8_t*)&chUART2, 1);
			break;
		case (uint32_t)USART3:
			HAL_UART_Receive_IT(&huart3, (uint8_t*)&chUART3, 1);
			break;
		case (uint32_t)UART4:
			HAL_UART_Receive_IT(&huart4, (uint8_t*)&chUART4, 1);
			break;
		case (uint32_t)UART5:
			HAL_UART_Receive_IT(&huart5, (uint8_t*)&chUART5, 1);
			break;
		case (uint32_t)USART6:
			HAL_UART_Receive_IT(&huart6, (uint8_t*)&chUART6, 1);
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
	int i, cnt;
	uint8_t *pData;
	switch((uint32_t)UartHandle->Instance)
	{
		case (uint32_t)USART1: // При обмене с ВВ по DMA, определим сначала длину фрейма и передадим на обработку
		  cnt=getUART_BB_RcvDMAPktLength();
		  for(i=0, pData=UartHandle->pRxBuffPtr; i<cnt;i++,pData++){
				xQueueSendFromISR( myQueueUart6TxHandle , pData, NULL  );  
			}
		  break;
		case (uint32_t)USART2:
			xQueueSendFromISR( myQueueUart2RxHandle , UartHandle->pRxBuffPtr, NULL  );  
		  break;
		case (uint32_t)USART3:
			xQueueSendFromISR( myQueueUart3RxHandle , UartHandle->pRxBuffPtr, NULL  );  
			break;			
		case (uint32_t)UART4:
			xQueueSendFromISR( myQueueUart4RxHandle , UartHandle->pRxBuffPtr, NULL  );  
		  break;
		case (uint32_t)UART5:
			xQueueSendFromISR( myQueueUart5RxHandle , UartHandle->pRxBuffPtr, NULL  );  
		  break;
		case (uint32_t)USART6:
			xQueueSendFromISR( myQueueUart6RxHandle , UartHandle->pRxBuffPtr, NULL  );  
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
	}
}


UART_HandleTypeDef *huart[UART_NUM]={&huart2,&huart3,&huart4,&huart5,&huart6};

#define USART_MIN_BAUDRATE	1200
#define USART_MAX_BAUDRATE	115200

void USARTs_SetSpeed(uint32_t *uartBaudRate)
{
	uint8_t uart_count=0;
	
	for(uart_count=0;uart_count<UART_NUM;uart_count++)
	{
		if((huart[uart_count]->Init.BaudRate!=uartBaudRate[uart_count])&&(uartBaudRate[uart_count]>=USART_MIN_BAUDRATE)&&(uartBaudRate[uart_count]<=USART_MAX_BAUDRATE))
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

