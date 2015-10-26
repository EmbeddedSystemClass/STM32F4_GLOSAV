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
#include "gpio.h"
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "tasks.h"

extern void prvvUARTTxReadyISR( void );
extern void prvvUARTRxISR( void );


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
		case (uint32_t)USART1:
			HAL_UART_Receive_IT(&huart1, (uint8_t*)&chUART1, 1);
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
// Обработчик приема символов по СОМ портам (терминал и СпМ) в режиме прерываний
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) // from interrupt called!
{ 
//	HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_14);
	switch((uint32_t)UartHandle->Instance)
	{
		case (uint32_t)USART1:
//			HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_10);
			prvvUARTRxISR();	// modbus - char is in "chUART_"
//			xQueueSendFromISR( myQueueUart1RxHandle , UartHandle->pRxBuffPtr, NULL  );  
//			xQueueSendFromISR( myQueueMbUart1RxHandle , UartHandle->pRxBuffPtr, NULL  );  
//			xQueueSend( RecQHandle, &m, portMAX_DELAY  );
		  // Do this need? 
      //HAL_UART_Receive_IT(&huart1, &b1,1);
		  break;
		case (uint32_t)USART2:
			xQueueSendFromISR( myQueueUart2RxHandle , UartHandle->pRxBuffPtr, NULL  );  
		  break;
		case (uint32_t)USART3:
			xQueueSendFromISR( myQueueUart3RxHandle , UartHandle->pRxBuffPtr, NULL  );  
			break;			
		case (uint32_t)UART4:
			xQueueSendFromISR( myQueueUart4RxHandle , UartHandle->pRxBuffPtr, NULL  );  
//			prvvUARTRxISR();	// modbus - char is in "chUART_"
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
/*			__disable_irq ();
			if( !FIFO_IS_EMPTY( Trml_tx_fifo ) ){ // есть что передать?
				txTrmlCh = FIFO_FRONT( Trml_tx_fifo );
				FIFO_POP( Trml_tx_fifo );
				HAL_UART_Transmit_IT(&huart4, (uint8_t *)&txTrmlCh, 1); // заряжаем...
			}
			__enable_irq ();*/
			break;
	}
}

