#include "can_app.h"

#include "stm32f4xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

static void CAN1_Listening_Task(void *pvParameters);
static void CAN2_Sending_Task(void *pvParameters);

void CAN_App_Init(void)
{
		xTaskCreate(CAN1_Listening_Task,(signed char*)"CAN1 Listening",128,NULL, tskIDLE_PRIORITY + 1, NULL);
		xTaskCreate(CAN2_Sending_Task,(signed char*)"CAN2 Sending",128,NULL, tskIDLE_PRIORITY + 5, NULL);
}

static void CAN1_Listening_Task(void *pvParameters)
{
	
	while(1)
	{
			vTaskDelay(1000);
	}
}

static void CAN2_Sending_Task(void *pvParameters)
{
		TickType_t xLastWakeTime;
		xLastWakeTime = xTaskGetTickCount();
	
			CanTxMsgTypeDef TxMess;
			hcan2.pTxMsg = &TxMess;
			TxMess.StdId = 0x321;
			TxMess.ExtId = 0x01;
			TxMess.RTR = CAN_RTR_DATA;
			TxMess.IDE = CAN_ID_STD;
			TxMess.DLC = 8;
			TxMess.Data[0]=0xAA;
			TxMess.Data[1]=0xFF;
			TxMess.Data[2]=0xAA;
			TxMess.Data[3]=0xFF;
			TxMess.Data[4]=0xAA;
			TxMess.Data[5]=0xFF;
			TxMess.Data[6]=0xAA;
			TxMess.Data[7]=0xFF;
	

	
		while(1)
		{
				HAL_CAN_Transmit(&hcan2,10);
				vTaskDelayUntil( &xLastWakeTime, 100 );
		}
}