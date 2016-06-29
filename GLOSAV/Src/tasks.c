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

osThreadId myTaskMODBUSHandle;

osTimerId myTimer01Handle;

void StartTaskMODBUS(void const * argument);
void Callback01(void const * argument);

void startUserTasks(void)
{
  /* USER CODE BEGIN RTOS_TIMERS */
	
	xMBInputRegParamsMutex = xSemaphoreCreateMutex();
	xMBHoldingRegParamsMutex= xSemaphoreCreateMutex();
	
  /* start timers, add new ones, ... */
  osTimerDef(myTimer01, Callback01);
  myTimer01Handle = osTimerCreate(osTimer(myTimer01), osTimerPeriodic, NULL);
  /* USER CODE END RTOS_TIMERS */

  /* definition and creation of myTaskMODBUS */
  osThreadDef(myTaskMODBUS, StartTaskMODBUS, osPriorityRealtime, 0, TASK_MODBUS_STACK_SIZE);
  myTaskMODBUSHandle = osThreadCreate(osThread(myTaskMODBUS), NULL);
//	   if( pdPASS != xTaskCreate( vTaskMODBUS, "MODBUS", TASK_MODBUS_STACK_SIZE, NULL, TASK_MODBUS_PRIORITY, NULL ) )

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
}


void StartTaskMODBUS( void const * argument)
{
    const UCHAR     ucSlaveID[] = { 0xAA, 0xBB, 0xCC };
    eMBErrorCode    eStatus;

    for( ;; )
    {
//        if( MB_ENOERR != ( eStatus = eMBInit( MB_ASCII, 0x0A, 1, 38400, MB_PAR_EVEN ) ) )
        if( MB_ENOERR != ( eStatus = eMBInit( MB_RTU, 0x0A, 1, 3000000, UART_PARITY_NONE ) ) )
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
//								startUARTRcv(&huart1);	// to BeagleBone 
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



