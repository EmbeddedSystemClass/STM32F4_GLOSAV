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

void startUARTRcv(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif
#endif /*__ usart_app_H */

