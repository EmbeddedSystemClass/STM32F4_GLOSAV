/**
  ******************************************************************************
  * File Name          : tasks.h
  * Date               : 31/07/2015 10:26:00
  * Description        : 
  ******************************************************************************
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __tasks_H
#define __tasks_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

extern osThreadId myTaskTrmlRxHandle;
extern osThreadId myTaskSIMRxHandle;
extern osThreadId myTaskTrmlTxHandle;

extern osMessageQId myQueueSIMRxHandle;
extern osMessageQId myQueueUart1TxHandle;
extern osMessageQId myQueueUart1RxHandle;
extern osMessageQId myQueueUart2TxHandle;
extern osMessageQId myQueueUart2RxHandle;
extern osMessageQId myQueueUart3TxHandle;
extern osMessageQId myQueueUart3RxHandle;
extern osMessageQId myQueueUart4TxHandle;
extern osMessageQId myQueueUart4RxHandle;
extern osMessageQId myQueueUart5TxHandle;
extern osMessageQId myQueueUart5RxHandle;
extern osMessageQId myQueueUart6TxHandle;
extern osMessageQId myQueueUart6RxHandle;

	 
extern osTimerId myTimer01Handle;

void startUserTasks(void);

#ifdef __cplusplus
}
#endif
#endif /*__ tasks_H */

