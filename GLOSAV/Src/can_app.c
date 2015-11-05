#include "can_app.h"



#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define CAN1_MESSAGE_QUEUE_MAX_LENGTH  10

#define FMS_PGN_LFC			0x00FEE9
#define FMS_PGN_DD			0x00FEFC
#define FMS_PGN_HRLFC		0x00FD09
#define FMS_PGN_AT1T1I	0x00FE56
#define FMS_PGN_VW			0x00FEEA
#define FMS_PGN_CVW			0x00FE70
#define FMS_PGN_CCVS		0x00FEF1


extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

static CanRxMsgTypeDef can1RxMessage;

QueueHandle_t xCAN1_MessageQueue;
SemaphoreHandle_t	xCAN1_DataMutex;


stCAN_FSM_Params CAN_FSM_Params;

static void CAN1_Listening_Task(void *pvParameters);
static void CAN2_Sending_Task(void *pvParameters);
void CAN1_Handling_Message(CanRxMsgTypeDef *can1msg);

#define CAN1_ADDRESS	(0x00000100)
#define CAN1_FILTER		(0xFFFFFFFF)

#define CAN1_LISTENING_STACK_SIZE		128

void CAN_App_Init(void)
{
		CAN_FilterConfTypeDef hcan1filter;
		hcan1filter.FilterIdHigh =0;// ((CAN1_ADDRESS<<3)>>16)&0xFFFF0000;
		hcan1filter.FilterIdLow = 0;// (CAN1_ADDRESS<<3 )&0x0000FFFF;
		hcan1filter.FilterMaskIdHigh =0;// ((CAN1_FILTER<<3)>>16)&0xFFFF0000;
		hcan1filter.FilterMaskIdLow = 0;// (CAN1_FILTER<<3 )&0x0000FFFF;
		hcan1filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
		hcan1filter.FilterNumber = 0;
		hcan1filter.FilterMode = CAN_FILTERMODE_IDMASK;
		hcan1filter.FilterScale = CAN_FILTERSCALE_32BIT;
		hcan1filter.FilterActivation = ENABLE;
		hcan1filter.BankNumber = 14;
		HAL_CAN_ConfigFilter(&hcan1, &hcan1filter);
	
		hcan1.pRxMsg = &can1RxMessage;

		xCAN1_MessageQueue = xQueueCreate( CAN1_MESSAGE_QUEUE_MAX_LENGTH, sizeof( CanRxMsgTypeDef ) );
		
		xCAN1_DataMutex = xSemaphoreCreateMutex();
	
		xTaskCreate(CAN1_Listening_Task,(signed char*)"CAN1 Listening",CAN1_LISTENING_STACK_SIZE,NULL, tskIDLE_PRIORITY + 1, NULL);
		xTaskCreate(CAN2_Sending_Task,(signed char*)"CAN2 Sending",128,NULL, tskIDLE_PRIORITY + 5, NULL);
		
		
		CAN_FSM_Params.params.Weight=0xAB;
}

static void CAN1_Listening_Task(void *pvParameters)
{
	CanRxMsgTypeDef RxMessage;	
	static uint8_t temp;
	while(1)
	{  
		xQueueReceive(xCAN1_MessageQueue, &RxMessage, portMAX_DELAY);
		CAN1_Handling_Message(&RxMessage);
	}
}

static void CAN2_Sending_Task(void *pvParameters)
{
		TickType_t xLastWakeTime;
		xLastWakeTime = xTaskGetTickCount();
	
			CanTxMsgTypeDef TxMess;
			hcan2.pTxMsg = &TxMess;
			TxMess.StdId = 0x321;
			TxMess.ExtId = /*(FMS_PGN_DD<<8)*/0x100;
			TxMess.RTR = CAN_RTR_DATA;
			TxMess.IDE = CAN_ID_EXT;
			TxMess.DLC = 1;
			TxMess.Data[0]=0x3A;
			TxMess.Data[1]=0xFF;
			TxMess.Data[2]=0xAA;
			TxMess.Data[3]=0xFF;
			TxMess.Data[4]=0xAA;
			TxMess.Data[5]=0xFF;
			TxMess.Data[6]=0xAA;
			TxMess.Data[7]=0xFF;
	
		while(1)
		{
				TxMess.ExtId = 0x18FA0010;
				HAL_CAN_Transmit(&hcan2,10);
				vTaskDelayUntil( &xLastWakeTime, 400 );
				TxMess.ExtId = 0x18FB0010;
				HAL_CAN_Transmit(&hcan2,10);
				vTaskDelayUntil( &xLastWakeTime, 400 );
		}
}

void HAL_CAN_TxCpltCallback(CAN_HandleTypeDef* hcan)
{
	static uint32_t i=0;
	i++;
}

/**
  * @brief  Transmission  complete callback in non blocking mode 
  * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{
	static uint8_t temp=0;
	temp=hcan->pRxMsg->Data[0];
	static portBASE_TYPE xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;

	xQueueSendFromISR(xCAN1_MessageQueue, hcan->pRxMsg, &xHigherPriorityTaskWoken);
	
	if(xHigherPriorityTaskWoken == pdTRUE)
	{
		taskYIELD();
	}
	
	HAL_CAN_Receive_IT(&hcan1, CAN_FIFO0);
}

/**
  * @brief  Error CAN callback.
  * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
	static uint32_t i=0;
	i++;
}

void CAN1_Handling_Message(CanRxMsgTypeDef *can1msg)
{
		static uint32_t i=0;
		uint32_t pgn=(can1msg->ExtId>>8)&0xFFFF;
	
	uint16_t test=0;
	
	xSemaphoreTake( xCAN1_DataMutex, portMAX_DELAY );
  {	
			switch(pgn)
			{
					case FMS_PGN_LFC:
					{

					}
					break;
					
					case FMS_PGN_DD:
					{
							CAN_FSM_Params.params.FuelLevel=can1msg->Data[1];
					}
					break;
					
					case FMS_PGN_HRLFC:
					{

					}
					break;
					
					case FMS_PGN_AT1T1I:
					{

					}
					break;
					
					case FMS_PGN_VW:
					{

					}
					break;
					
					case FMS_PGN_CVW:
					{

					}
					break;
					
					case FMS_PGN_CCVS:
					{

					}
					break;
					
					default:
					{
						
					}
			}
	}
  xSemaphoreGive( xCAN1_DataMutex );
}

