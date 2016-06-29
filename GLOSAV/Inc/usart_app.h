/**
  ******************************************************************************
  * File Name          : USART_app.h
  * Date               : 31/07/2015 10:26:00
  * Description        : 
  ******************************************************************************
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __usart_app_H
#define __usart_app_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "fifo.h"
#define UART_NUM		5

extern rxfifo_t uart4RxFifo;
extern txfifo_t uart4TxFifo;
extern rxfifo_t uart5RxFifo;
extern txfifo_t uart5TxFifo;
extern rxfifo_t uart6RxFifo;
extern txfifo_t uart6TxFifo;
	 
	 
void uartsFifoInit(void);
void USARTs_SetSpeed(uint32_t *uartBaudRate);
void startUARTRcv(UART_HandleTypeDef *huart);
void stopUART(UART_HandleTypeDef *huart);
uint16_t getUART_BB_RcvDMAPktLength(void);

#ifdef __cplusplus
}
#endif
#endif /*__ usart_app_H */

