/**
  ******************************************************************************
  * File Name          : gpio_app.c
  * Date               : 31/07/2015 10:25:57
  * Description        : This file provides code for the configuration
  *                      of user used GPIO pins.
  ******************************************************************************
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "config.h"


void MAIN_GPIO_Init(void)
{
/*  GPIO_InitTypeDef GPIO_InitStruct;


  GPIO_InitStruct.Pin = UART1_DIR_PIN;
  HAL_GPIO_Init(UART1_DIR_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = UART3_DIR_PIN;
  HAL_GPIO_Init(UART3_DIR_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = UART4_DIR_PIN;
  HAL_GPIO_Init(UART4_DIR_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = UART5_DIR_PIN;
  HAL_GPIO_Init(UART5_DIR_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = UART6_DIR_PIN;
  HAL_GPIO_Init(UART6_DIR_PORT, &GPIO_InitStruct);
*/
	HAL_GPIO_WritePin(GPIOE,	out_CAN1_SD_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE,	out_CAN2_SD_Pin, GPIO_PIN_RESET);
	
}
