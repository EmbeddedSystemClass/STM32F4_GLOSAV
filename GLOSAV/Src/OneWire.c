#include "OneWire.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "mb_app.h"

#define ONEWIRE_READ_PERIOD	500
#define ONEWIRE_STACK_SIZE		128

#define ONEWIRE_RW_HARDWARE_TIMER

enum
{
	OW_STATE_IDLE=0,
	OW_STATE_TIMESLOT_START,
	OW_STATE_WRITE_0,
	OW_STATE_WRITE_1,
	OW_STATE_READ,
	OW_STATE_TIMESLOT_END,
};

enum
{
	OW_NONE,
	OW_WRITE,
	OW_READ,
};

uint8_t ow_state=OW_STATE_IDLE;
uint8_t ow_operation=OW_NONE;
uint8_t ow_bit=0;
uint8_t ow_flag_done=0;

OneWire_t OneWireStruct;
extern TIM_HandleTypeDef htim7;
extern stMBHoldingRegParams MBHoldingRegParams;

void OneWire_Input(void);
void OneWire_Output(void);
void Delay(uint32_t delay);
void OneWire_ReadROM(OneWire_t* OneWireStruct);
uint8_t OneWire_Read_iButton(OneWire_t* OneWireStruct,uint8_t *data);
static void OneWire_Task(void *pvParameters);


void Delay(uint32_t delay) 
{
		delay=delay*39;
    while (delay--);
}

void OneWire_Input(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = in_1_wire_in_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(in_1_wire_in_GPIO_Port, &GPIO_InitStruct);
}

void OneWire_Output(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = in_1_wire_in_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(in_1_wire_in_GPIO_Port, &GPIO_InitStruct);
}

void OneWire_StrongPullUp_On(void)
{
	ONEWIRE_HIGH();
	HAL_GPIO_WritePin(out_1_Wire_ctrl_GPIO_Port, out_1_Wire_ctrl_Pin, GPIO_PIN_SET);	
}

void OneWire_StrongPullUp_Off(void)
{
	HAL_GPIO_WritePin(out_1_Wire_ctrl_GPIO_Port, out_1_Wire_ctrl_Pin, GPIO_PIN_RESET);
}

void OneWire_Init(void) 
{	
	HAL_GPIO_WritePin(out_1_Wire_ctrl_GPIO_Port, out_1_Wire_ctrl_Pin, GPIO_PIN_RESET);
	xTaskCreate(OneWire_Task,(signed char*)"OneWire Task",ONEWIRE_STACK_SIZE,NULL, tskIDLE_PRIORITY + 1, NULL);
}

uint8_t OneWire_Reset(OneWire_t* OneWireStruct) {
	uint8_t i;
	
	/* Line low, and wait 480us */
	ONEWIRE_LOW();
	OneWire_Output();
	ONEWIRE_DELAY(/*480*/600);
	
	/* Release line and wait for 70us */
	OneWire_Input();
	ONEWIRE_DELAY(70);
	
	/* Check bit value */
	i = HAL_GPIO_ReadPin(in_1_wire_in_GPIO_Port, in_1_wire_in_Pin);
	
	/* Delay for 410 us */
	ONEWIRE_DELAY(410);
	
	/* Return value of presence pulse, 0 = OK, 1 = ERROR */
	return i;
}

#ifndef ONEWIRE_RW_HARDWARE_TIMER
void OneWire_WriteBit(OneWire_t* OneWireStruct, uint8_t bit) 
{
		OneWire_Output();
		if (bit) 
		{
			ONEWIRE_LOW();
			ONEWIRE_DELAY(10);
			ONEWIRE_HIGH();
			ONEWIRE_DELAY(55);
		} 
		else 
		{
			ONEWIRE_LOW();
			ONEWIRE_DELAY(65);
			ONEWIRE_HIGH();
			ONEWIRE_DELAY(5);
		}
		OneWire_Input();
}

uint8_t OneWire_ReadBit(OneWire_t* OneWireStruct) 
{
	uint8_t bit = 0;
	OneWire_Output();

	ONEWIRE_LOW();
	ONEWIRE_DELAY(3);
	OneWire_Input();
	ONEWIRE_DELAY(10);
	
	if (HAL_GPIO_ReadPin(in_1_wire_in_GPIO_Port, in_1_wire_in_Pin)) 
	{
		bit = 1;
	}
	ONEWIRE_DELAY(50);
	return bit;
}
#else
void OneWire_WriteBit(OneWire_t* OneWireStruct, uint8_t bit)
{
	ow_bit=bit;
	ow_operation=OW_WRITE;
	ow_state=OW_STATE_TIMESLOT_START;
	HAL_TIM_Base_Start_IT(&htim7);
	while(ow_state!=OW_STATE_IDLE);
}

uint8_t OneWire_ReadBit(OneWire_t* OneWireStruct)
{
	ow_bit=0;
	ow_operation=OW_READ;
	ow_state=OW_STATE_TIMESLOT_START;
	HAL_TIM_Base_Start_IT(&htim7);
	while(ow_state!=OW_STATE_IDLE);
	return ow_bit; 
}
#endif

void OneWire_WriteByte(OneWire_t* OneWireStruct, uint8_t byte) {
	uint8_t i = 8;
	/* Write 8 bits */
	while (i--) {
		/* LSB bit is first */
		OneWire_WriteBit(OneWireStruct, byte & 0x01);
		byte >>= 1;
	}
}

uint8_t OneWire_ReadByte(OneWire_t* OneWireStruct) {
	uint8_t i = 8, byte = 0;
	while (i--) {
		byte >>= 1;
		byte |= (OneWire_ReadBit(OneWireStruct) << 7);
	}
	
	return byte;
}

uint8_t OneWire_CRC8(uint8_t *addr, uint8_t len) {
	uint8_t crc = 0, inbyte, i, mix;
	
	while (len--) {
		inbyte = *addr++;
		for (i = 8; i; i--) {
			mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) {
				crc ^= 0x8C;
			}
			inbyte >>= 1;
		}
	}
	return crc;
}


uint8_t temperature[2];

uint8_t OneWire_DS18b20(OneWire_t* OneWireStruct)
{
	if(OneWire_Reset(OneWireStruct)==0)
	{
		OneWire_WriteByte(OneWireStruct,0xCC);	
		OneWire_WriteByte(OneWireStruct,0x44);
		OneWire_StrongPullUp_On();	
		vTaskDelay(1000);
		OneWire_StrongPullUp_Off();	
		OneWire_Reset(OneWireStruct);
		OneWire_WriteByte(OneWireStruct,0xCC);	
		OneWire_WriteByte(OneWireStruct,0xBE);	
		
		temperature[0]=OneWire_ReadByte(OneWireStruct);
		temperature[1]=OneWire_ReadByte(OneWireStruct);
		return 0;
	}		
	else
	{
			return 1;
	}		
}

uint8_t OneWire_Read_iButton(OneWire_t* OneWireStruct,uint8_t *data)
{
	uint8_t i=0,result=0;
	
	if(OneWire_Reset(OneWireStruct)==0)
	{
			OneWire_StrongPullUp_On();	
			vTaskDelay(10);
			OneWire_StrongPullUp_Off();	
		
		
			OneWire_WriteByte(OneWireStruct,ONEWIRE_CMD_READROM);
			
			for(i=0;i<8;i++)
			{
				data[i]=OneWire_ReadByte(OneWireStruct);
			}
			
			if(data[0]!=0)
			{	
				if(OneWire_CRC8(&data[0],7)==data[7])
				{
					result= 0;
				}
				else
				{
					result= 1;
				}
			}
			else
			{
				result= 1;
			}
	}		
	else
	{
			result= 1;
	}	
	
	return result;
}

uint8_t iButtonData[8];

static void OneWire_Task(void *pvParameters)
{
	uint8_t i=0;
	uint8_t result=0;
	while(1)
	{  
		//OneWire_DS18b20(&OneWireStruct);
		result=OneWire_Read_iButton(&OneWireStruct,iButtonData);
		if(result==0)
		{
			for(i=0;i<8;i++)
			{			
					MBHoldingRegParams.params.iButtonID[i]=iButtonData[i];
			}
		}
		else
		{
			for(i=0;i<8;i++)
			{			
					MBHoldingRegParams.params.iButtonID[i]=0;
			}
		}
		vTaskDelay(ONEWIRE_READ_PERIOD);
	}
}


void TIM7_IRQHandler(void)
{
	HAL_TIM_Base_Stop_IT(&htim7);
	__HAL_TIM_CLEAR_FLAG(&htim7, TIM_IT_UPDATE);
	
	TIM7->CNT=0;

	switch(ow_state)
	{
		case OW_STATE_IDLE:
		{

		}
		break;
		
		case OW_STATE_TIMESLOT_START:
		{
				OneWire_Output();
				ONEWIRE_LOW();
				TIM7->ARR=15;
			  HAL_TIM_Base_Start_IT(&htim7);
			
				switch(ow_operation)
				{
					case OW_NONE:  break;
					
					case OW_WRITE: 
					{
						if(ow_bit)
						{
								ow_state=OW_STATE_WRITE_1; 
						}
						else
						{
							  ow_state=OW_STATE_WRITE_0; 
						}
					}
					break;
					
					case OW_READ: ow_state=OW_STATE_READ; break;
				}
		}
		break;
		
		case OW_STATE_WRITE_0:
		{
				ONEWIRE_LOW();
				TIM7->ARR=45;
			  HAL_TIM_Base_Start_IT(&htim7);
				ow_state=OW_STATE_TIMESLOT_END;
		}
		break;
		
		case OW_STATE_WRITE_1:
		{
				ONEWIRE_HIGH();
				TIM7->ARR=45;
			  HAL_TIM_Base_Start_IT(&htim7);	
				ow_state=OW_STATE_TIMESLOT_END;			
		}
		break;
		
		case OW_STATE_READ:
		{
				OneWire_Input();
				ONEWIRE_HIGH();
				ow_bit=HAL_GPIO_ReadPin(in_1_wire_in_GPIO_Port, in_1_wire_in_Pin);
				TIM7->ARR=45;
			  HAL_TIM_Base_Start_IT(&htim7);	
				ow_state=OW_STATE_TIMESLOT_END;		
		}
		break;
		
		case OW_STATE_TIMESLOT_END:
		{
				OneWire_Output();
				ONEWIRE_HIGH();
				ow_state=OW_STATE_IDLE;
		}
		break;
	}
}
