/*
Файл общей конфигурации проекта
*/
#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"	// там в том числе определены и типы данных, поэтому включим везде этот файл
#include "cmsis_os.h"

#define _DEBUG_

#define SYS_TICK		0.01		// период тика таймера (сек)
#define PERIOD_CYCLE	1.0			// период опроса состояния и выполнения действий (в секундах) (ПРОВЕРЬТЕ ЗНАЧЕНИЕ OCR!)


#define PRIBOR_NAME_MAX_LEN	30

#define DEBUG_LEVEL_1	1
#define DEBUG_LEVEL_2	2
#define DEBUG_LEVEL_3	3
#define DEBUG_LEVEL_4	4
#define DEBUG_LEVEL_5	5
#define DEBUG_LEVEL_6	6
#define DEBUG_LEVEL_7	7
#define DEBUG_LEVEL_8	8
#define DEBUG_LEVEL_9	9
#define DEBUG_LEVEL_10	10
#ifdef _DEBUG_
	extern uint8_t debugLevel;	// уровень выводимой отладочной информации
	#define D_PRINTF(DLevel, param...)	if(debugLevel >= DLevel){putchar('\n');for(int i=0;i<DLevel;i++){putchar('\t');} printf("D:"); printf(param);}
#else
	#define D_PRINTF(DLevel, param...)		;
#endif

/*
Defines GPIO ports names
*/
//#define setSIM_pwr(x) (HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, x))

#define UART1_DIR_PORT		GPIOA
#define UART1_DIR_PIN			GPIO_PIN_8
#define UART1_DIR_SEND() (HAL_GPIO_WritePin(UART1_DIR_PORT, UART1_DIR_PIN, GPIO_PIN_SET))
#define UART1_DIR_RCV() (HAL_GPIO_WritePin(UART1_DIR_PORT, UART1_DIR_PIN, GPIO_PIN_RESET))

#define UART5_DIR_PORT		GPIOD
#define UART5_DIR_PIN			GPIO_PIN_3
#define UART5_DIR_SEND() (HAL_GPIO_WritePin(UART5_DIR_PORT, UART5_DIR_PIN, GPIO_PIN_SET))
#define UART5_DIR_RCV() (HAL_GPIO_WritePin(UART5_DIR_PORT, UART5_DIR_PIN, GPIO_PIN_RESET))

#define UART6_DIR_PORT		GPIOG
#define UART6_DIR_PIN			GPIO_PIN_10
#define UART6_DIR_SEND() (HAL_GPIO_WritePin(UART6_DIR_PORT, UART6_DIR_PIN, GPIO_PIN_SET))
#define UART6_DIR_RCV() (HAL_GPIO_WritePin(UART6_DIR_PORT, UART6_DIR_PIN, GPIO_PIN_RESET))

#define UART3_DIR_PORT		GPIOD
#define UART3_DIR_PIN			GPIO_PIN_11
#define UART3_DIR_SEND() (HAL_GPIO_WritePin(UART3_DIR_PORT, UART3_DIR_PIN, GPIO_PIN_SET))
#define UART3_DIR_RCV() (HAL_GPIO_WritePin(UART3_DIR_PORT, UART3_DIR_PIN, GPIO_PIN_RESET))

#define UART4_DIR_PORT		GPIOD
#define UART4_DIR_PIN			GPIO_PIN_4
#define UART4_DIR_SEND() (HAL_GPIO_WritePin(UART4_DIR_PORT, UART4_DIR_PIN, GPIO_PIN_SET))
#define UART4_DIR_RCV() (HAL_GPIO_WritePin(UART4_DIR_PORT, UART4_DIR_PIN, GPIO_PIN_RESET))
/*
#define LED1_CLK_ENABLE  __GPIOF_CLK_ENABLE
#define LED1_PORT			GPIOF
#define LED1_PIN			GPIO_PIN_6
*/
#define LED1_CLK_ENABLE  __GPIOE_CLK_ENABLE
#define LED1_PORT			GPIOE
#define LED1_PIN			GPIO_PIN_8

#define led1_on() 		(HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_SET))
#define led1_off() 		(HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_RESET))
#define led1_toggle()	(HAL_GPIO_TogglePin(LED1_PORT, LED1_PIN))

/*#define LED2_CLK_ENABLE  __GPIOF_CLK_ENABLE
#define LED2_PORT			GPIOF
#define LED2_PIN			GPIO_PIN_7*/
#define LED2_CLK_ENABLE  __GPIOE_CLK_ENABLE
#define LED2_PORT			GPIOE
#define LED2_PIN			GPIO_PIN_9
#define led2_on() 		(HAL_GPIO_WritePin(LED2_PORT, LED2_PIN, GPIO_PIN_SET))
#define led2_off() 		(HAL_GPIO_WritePin(LED2_PORT, LED2_PIN, GPIO_PIN_RESET))
#define led2_toggle()	(HAL_GPIO_TogglePin(LED2_PORT, LED2_PIN))

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H_INCLUDED */

