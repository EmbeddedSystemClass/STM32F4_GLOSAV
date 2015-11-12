#ifndef USART_APP_H
#define USART_APP_H
#include "stm32f4xx_hal.h"

#define UART_NUM		5

void USARTs_SetSpeed(uint32_t *uartBaudRate);

#endif