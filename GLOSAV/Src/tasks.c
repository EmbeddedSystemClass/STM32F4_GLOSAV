/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "usart.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "mb_app.h"

/* ----------------------- Defines ------------------------------------------*/
//#define REG_INPUT_START                 ( 1000 )
//#define REG_INPUT_NREGS                 ( 64 )

//#define REG_HOLDING_START               ( 1 )
//#define REG_HOLDING_NREGS               ( 32 )

#define TASK_MODBUS_STACK_SIZE          ( 256 )
//#define TASK_MODBUS_PRIORITY            ( osPriorityAboveNormal )

/* Private variables ---------------------------------------------------------*/

/* ----------------------- Static variables ---------------------------------*/
//static USHORT   usRegInputStart = REG_INPUT_START;
//extern USHORT   usRegInputBuf[];
////static USHORT   usRegHoldingStart = REG_HOLDING_START;
//extern USHORT   usRegHoldingBuf[];

osThreadId myTaskTrmlRxHandle;
osThreadId myTaskSIMRxHandle;
osThreadId myTaskTrmlTxHandle;
osThreadId myTaskMODBUSHandle;
osThreadId myTaskUart6TxHandle;
osThreadId myTaskUart4TxHandle;
osThreadId myTaskUart5TxHandle;

osMessageQId myQueueUart1RxHandle;
osMessageQId myQueueUart1TxHandle;

osMessageQId myQueueUart2RxHandle;
osMessageQId myQueueUart2TxHandle;
osMessageQId myQueueUart3RxHandle;
osMessageQId myQueueUart6TxHandle;
osMessageQId myQueueUart4RxHandle;
osMessageQId myQueueUart4TxHandle;
osMessageQId myQueueUart5RxHandle;
osMessageQId myQueueUart5TxHandle;
osMessageQId myQueueUart6RxHandle;
osMessageQId myQueueUart6TxHandle;

osTimerId myTimer01Handle;

void StartTaskTrmlRx(void const * argument);
void StartTaskSIMRx(void const * argument);
void StartTaskTrmlTx(void const * argument);
void StartTaskUart6Tx(void const * argument);
//void StartTaskUart4Tx(void const * argument);
void StartTaskUart5Tx(void const * argument);
void StartTaskMODBUS(void const * argument);
void Callback01(void const * argument);

void startUserTasks(void)
{
  /* USER CODE BEGIN RTOS_TIMERS */
	
	xMBInputRegParamsMutex = xSemaphoreCreateMutex();
	
  /* start timers, add new ones, ... */
  osTimerDef(myTimer01, Callback01);
  myTimer01Handle = osTimerCreate(osTimer(myTimer01), osTimerPeriodic, NULL);
  /* USER CODE END RTOS_TIMERS */

  osThreadDef(myTaskTrmlRx, StartTaskTrmlRx, osPriorityHigh, 0, 128);
  myTaskTrmlRxHandle = osThreadCreate(osThread(myTaskTrmlRx), NULL);

  /* definition and creation of myTaskTrmlTx */
  osThreadDef(myTaskTrmlTx, StartTaskTrmlTx, osPriorityNormal, 0, 128);
  myTaskTrmlTxHandle = osThreadCreate(osThread(myTaskTrmlTx), NULL);

  /* definition and creation of myTaskUart6Tx */
  osThreadDef(myTaskUart6Tx, StartTaskUart6Tx, osPriorityNormal, 0, 128);
  myTaskUart6TxHandle = osThreadCreate(osThread(myTaskUart6Tx), NULL);
  /* definition and creation of myTaskUart4Tx */
//  osThreadDef(myTaskUart4Tx, StartTaskUart4Tx, osPriorityNormal, 0, 128);
//  myTaskUart4TxHandle = osThreadCreate(osThread(myTaskUart4Tx), NULL);
  /* definition and creation of myTaskUart4Tx */
  osThreadDef(myTaskUart5Tx, StartTaskUart5Tx, osPriorityNormal, 0, 128);
  myTaskUart5TxHandle = osThreadCreate(osThread(myTaskUart5Tx), NULL);
	
  /* definition and creation of myTaskSIMRx */
  osThreadDef(myTaskSIMRx, StartTaskSIMRx, osPriorityHigh, 0, 128);
  myTaskSIMRxHandle = osThreadCreate(osThread(myTaskSIMRx), NULL);

  /* definition and creation of myTaskMODBUS */
  osThreadDef(myTaskMODBUS, StartTaskMODBUS, osPriorityAboveNormal, 0, TASK_MODBUS_STACK_SIZE);
  myTaskMODBUSHandle = osThreadCreate(osThread(myTaskMODBUS), NULL);
//	   if( pdPASS != xTaskCreate( vTaskMODBUS, "MODBUS", TASK_MODBUS_STACK_SIZE, NULL, TASK_MODBUS_PRIORITY, NULL ) )

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  osMessageQDef(myQueueUart1Rx, 64, uint8_t);
  myQueueUart1RxHandle = osMessageCreate(osMessageQ(myQueueUart1Rx), NULL);
	osMessageQDef(myQueueUart1Tx, 128, uint8_t);
  myQueueUart1TxHandle = osMessageCreate(osMessageQ(myQueueUart1Tx), NULL);

  osMessageQDef(myQueueUart2Rx, 64, uint8_t);
  myQueueUart2RxHandle = osMessageCreate(osMessageQ(myQueueUart2Rx), NULL);
	osMessageQDef(myQueueUart2Tx, 128, uint8_t);
  myQueueUart2TxHandle = osMessageCreate(osMessageQ(myQueueUart2Tx), NULL);

  osMessageQDef(myQueueUart3Rx, 64, uint8_t);
  myQueueUart3RxHandle = osMessageCreate(osMessageQ(myQueueUart3Rx), NULL);
	osMessageQDef(myQueueUart3Tx, 128, uint8_t);
  myQueueUart6TxHandle = osMessageCreate(osMessageQ(myQueueUart3Tx), NULL);

  osMessageQDef(myQueueUart4Rx, 64, uint8_t);
  myQueueUart4RxHandle = osMessageCreate(osMessageQ(myQueueUart4Rx), NULL);
	osMessageQDef(myQueueUart4Tx, 128, uint8_t);
  myQueueUart4TxHandle = osMessageCreate(osMessageQ(myQueueUart4Tx), NULL);

  osMessageQDef(myQueueUart5Rx, 64, uint8_t);
  myQueueUart5RxHandle = osMessageCreate(osMessageQ(myQueueUart5Rx), NULL);
	osMessageQDef(myQueueUart5Tx, 128, uint8_t);
  myQueueUart5TxHandle = osMessageCreate(osMessageQ(myQueueUart5Tx), NULL);

  osMessageQDef(myQueueUart6Rx, 64, uint8_t);
  myQueueUart6RxHandle = osMessageCreate(osMessageQ(myQueueUart6Rx), NULL);
	osMessageQDef(myQueueUart6Tx, 128, uint8_t);
  myQueueUart6TxHandle = osMessageCreate(osMessageQ(myQueueUart6Tx), NULL);

}

/* StartTaskTrmlRx function 
Это задача только для запуска приема по СОМ порту в режиме прерываний. пз прерывания вызывется CALLBACK функция
которая уже принимает символ и отдает его куда нужно...
Так сделано для того, чтобы поток не занимал процессорное время с учетом организации HAL уровня от STM.
По идее, было бы лучше навсегда включить прерывния по порту и из CALLBACK функции раскидывать и не делать эти задачи, 
тратя на них время и ресурсы...
*/
void StartTaskTrmlRx(void const * argument)
{
	uint8_t ch;
  /* Infinite loop */
  for(;;)
  {
		if(xQueueReceive( myQueueUart2RxHandle, &ch, portMAX_DELAY ) == pdTRUE )
		{
//			putchar(ch);
//			HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_9);
//			xQueueSend( myQueueUart3RxHandle, &ch, portMAX_DELAY  );	// for test only
		}
	}
}

/* StartTaskSIMRx function */
void StartTaskSIMRx(void const * argument)
{
//	uint8_t ch;
  for(;;)
  {
//		if(xQueueReceive( myQueueSIMRxHandle, &ch, portMAX_DELAY ) == pdTRUE )
		{
//			HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_9);
		}
		osDelay(1000); // which one is better? check youself!
	}
}

/* StartTaskTrmlRx function */
void StartTaskTrmlTx(void const * argument)
{
	uint8_t w;
  /* Infinite loop */
  for(;;)
  {
		xQueueReceive( myQueueUart2TxHandle, &w, portMAX_DELAY );
		HAL_UART_Transmit(&huart2,(uint8_t *)&w,1, 0xFFFF); //PB10
//		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
//		osDelay(1);
  }
}

/* StartTaskUart4Tx function */
// UART4 - modbus to BB
void StartTaskUart4Tx(void const * argument)
{
	uint8_t w;
  for(;;)  {
		xQueueReceive( myQueueUart4TxHandle, &w, portMAX_DELAY );
		UART4_DIR_SEND();
		HAL_UART_Transmit(&huart4,(uint8_t *)&w,1, 0xFFFF); //PB10
		UART4_DIR_RCV();
  }
}

/* StartTaskUart5Tx function */
void StartTaskUart5Tx(void const * argument)
{
	uint8_t w;
  for(;;)  {
		xQueueReceive( myQueueUart5TxHandle, &w, portMAX_DELAY );
		UART5_DIR_SEND();
		HAL_UART_Transmit(&huart5,(uint8_t *)&w,1, 0xFFFF); //PB10
		UART5_DIR_RCV();
  }
}

/* StartTaskUart6Tx function */
void StartTaskUart6Tx(void const * argument)
{
	uint8_t w;
  for(;;)  {
		xQueueReceive( myQueueUart6TxHandle, &w, portMAX_DELAY );
//		putchar(w); // to terminal for testing
		UART6_DIR_SEND();
		HAL_UART_Transmit(&huart6,(uint8_t *)&w,1, 0xFFFF); //PB10
		UART6_DIR_RCV();
  }
}

void StartTaskMODBUS( void const * argument)
{
    const UCHAR     ucSlaveID[] = { 0xAA, 0xBB, 0xCC };
    eMBErrorCode    eStatus;

    for( ;; )
    {
//        if( MB_ENOERR != ( eStatus = eMBInit( MB_ASCII, 0x0A, 1, 38400, MB_PAR_EVEN ) ) )
        if( MB_ENOERR != ( eStatus = eMBInit( MB_RTU, 0x0A, 1, 921600, UART_PARITY_NONE ) ) )
        {
            /* Can not initialize. Add error handling code here. */
        }
        else
        {
            if( MB_ENOERR != ( eStatus = eMBSetSlaveID( 0x34, TRUE, ucSlaveID, 3 ) ) )
            {
                /* Can not set slave id. Check arguments */
            }
            else if( MB_ENOERR != ( eStatus = eMBEnable(  ) ) )
            {
                /* Enable failed. */
            }
            else
            {
                //usRegHoldingBuf[0] = 1;
                do
                {
                    ( void )eMBPoll(  );

                    /* Here we simply count the number of poll cycles. */
                    //usRegInputBuf[0]++;
                }
                while(/* usRegHoldingBuf[0] */1);
            }
            ( void )eMBDisable(  );
            ( void )eMBClose(  );
        }
        vTaskDelay( 50 );
    }
}


/* Callback01 function */
void Callback01(void const * argument)
{
  /* USER CODE BEGIN Callback01 */
  
  /* USER CODE END Callback01 */
}



