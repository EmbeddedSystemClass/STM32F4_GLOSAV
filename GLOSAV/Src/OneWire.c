#include "OneWire.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

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

void OneWire_Input(void);
void OneWire_Output(void);
void Delay(uint32_t delay);
void OneWire_ReadROM(OneWire_t* OneWireStruct);
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

uint8_t OneWire_First(OneWire_t* OneWireStruct) {
	/* Reset search values */
	OneWire_ResetSearch(OneWireStruct);

	/* Start with searching */
	return OneWire_Search(OneWireStruct, ONEWIRE_CMD_SEARCHROM);
}

uint8_t OneWire_Next(OneWire_t* OneWireStruct) {
   /* Leave the search state alone */
   return OneWire_Search(OneWireStruct, ONEWIRE_CMD_SEARCHROM);
}

void OneWire_ResetSearch(OneWire_t* OneWireStruct) {
	/* Reset the search state */
	OneWireStruct->LastDiscrepancy = 0;
	OneWireStruct->LastDeviceFlag = 0;
	OneWireStruct->LastFamilyDiscrepancy = 0;
}

uint8_t OneWire_Search(OneWire_t* OneWireStruct, uint8_t command) {
	uint8_t id_bit_number;
	uint8_t last_zero, rom_byte_number, search_result;
	uint8_t id_bit, cmp_id_bit;
	uint8_t rom_byte_mask, search_direction;

	/* Initialize for search */
	id_bit_number = 1;
	last_zero = 0;
	rom_byte_number = 0;
	rom_byte_mask = 1;
	search_result = 0;

	// if the last call was not the last one
	if (!OneWireStruct->LastDeviceFlag) {
		// 1-Wire reset
		if (OneWire_Reset(OneWireStruct)) {
			/* Reset the search */
			OneWireStruct->LastDiscrepancy = 0;
			OneWireStruct->LastDeviceFlag = 0;
			OneWireStruct->LastFamilyDiscrepancy = 0;
			return 0;
		}

		// issue the search command 
		OneWire_WriteByte(OneWireStruct, command);  

		// loop to do the search
		do {
			// read a bit and its complement
			id_bit = OneWire_ReadBit(OneWireStruct);
			cmp_id_bit = OneWire_ReadBit(OneWireStruct);

			// check for no devices on 1-wire
			if ((id_bit == 1) && (cmp_id_bit == 1)) {
				break;
			} else {
				// all devices coupled have 0 or 1
				if (id_bit != cmp_id_bit) {
					search_direction = id_bit;  // bit write value for search
				} else {
					// if this discrepancy if before the Last Discrepancy
					// on a previous next then pick the same as last time
					if (id_bit_number < OneWireStruct->LastDiscrepancy) {
						search_direction = ((OneWireStruct->ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
					} else {
						// if equal to last pick 1, if not then pick 0
						search_direction = (id_bit_number == OneWireStruct->LastDiscrepancy);
					}
					
					// if 0 was picked then record its position in LastZero
					if (search_direction == 0) {
						last_zero = id_bit_number;

						// check for Last discrepancy in family
						if (last_zero < 9) {
							OneWireStruct->LastFamilyDiscrepancy = last_zero;
						}
					}
				}

				// set or clear the bit in the ROM byte rom_byte_number
				// with mask rom_byte_mask
				if (search_direction == 1) {
					OneWireStruct->ROM_NO[rom_byte_number] |= rom_byte_mask;
				} else {
					OneWireStruct->ROM_NO[rom_byte_number] &= ~rom_byte_mask;
				}
				
				// serial number search direction write bit
				OneWire_WriteBit(OneWireStruct, search_direction);

				// increment the byte counter id_bit_number
				// and shift the mask rom_byte_mask
				id_bit_number++;
				rom_byte_mask <<= 1;

				// if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
				if (rom_byte_mask == 0) {
					//docrc8(ROM_NO[rom_byte_number]);  // accumulate the CRC
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
		} while (rom_byte_number < 8);  // loop until through all ROM bytes 0-7

		// if the search was successful then
		if (!(id_bit_number < 65)) {
			// search successful so set LastDiscrepancy,LastDeviceFlag,search_result
			OneWireStruct->LastDiscrepancy = last_zero;

			// check for last device
			if (OneWireStruct->LastDiscrepancy == 0) {
				OneWireStruct->LastDeviceFlag = 1;
			}

			search_result = 1;
		}
	}

	// if no device found then reset counters so next 'search' will be like a first
	if (!search_result || !OneWireStruct->ROM_NO[0]) {
		OneWireStruct->LastDiscrepancy = 0;
		OneWireStruct->LastDeviceFlag = 0;
		OneWireStruct->LastFamilyDiscrepancy = 0;
		search_result = 0;
	}

	return search_result;
}

int OneWire_Verify(OneWire_t* OneWireStruct) {
	unsigned char rom_backup[8];
	int i,rslt,ld_backup,ldf_backup,lfd_backup;

	// keep a backup copy of the current state
	for (i = 0; i < 8; i++)
	rom_backup[i] = OneWireStruct->ROM_NO[i];
	ld_backup = OneWireStruct->LastDiscrepancy;
	ldf_backup = OneWireStruct->LastDeviceFlag;
	lfd_backup = OneWireStruct->LastFamilyDiscrepancy;

	// set search to find the same device
	OneWireStruct->LastDiscrepancy = 64;
	OneWireStruct->LastDeviceFlag = 0;

	if (OneWire_Search(OneWireStruct, ONEWIRE_CMD_SEARCHROM)) {
		// check if same device found
		rslt = 1;
		for (i = 0; i < 8; i++) {
			if (rom_backup[i] != OneWireStruct->ROM_NO[i]) {
				rslt = 1;
				break;
			}
		}
	} else {
		rslt = 0;
	}

	// restore the search state 
	for (i = 0; i < 8; i++) {
		OneWireStruct->ROM_NO[i] = rom_backup[i];
	}
	OneWireStruct->LastDiscrepancy = ld_backup;
	OneWireStruct->LastDeviceFlag = ldf_backup;
	OneWireStruct->LastFamilyDiscrepancy = lfd_backup;

	// return the result of the verify
	return rslt;
}

void OneWire_TargetSetup(OneWire_t* OneWireStruct, uint8_t family_code) {
   uint8_t i;

	// set the search state to find SearchFamily type devices
	OneWireStruct->ROM_NO[0] = family_code;
	for (i = 1; i < 8; i++) {
		OneWireStruct->ROM_NO[i] = 0;
	}
	
	OneWireStruct->LastDiscrepancy = 64;
	OneWireStruct->LastFamilyDiscrepancy = 0;
	OneWireStruct->LastDeviceFlag = 0;
}

void OneWire_FamilySkipSetup(OneWire_t* OneWireStruct) {
	// set the Last discrepancy to last family discrepancy
	OneWireStruct->LastDiscrepancy = OneWireStruct->LastFamilyDiscrepancy;
	OneWireStruct->LastFamilyDiscrepancy = 0;

	// check for end of list
	if (OneWireStruct->LastDiscrepancy == 0) {
		OneWireStruct->LastDeviceFlag = 1;
	}
}

uint8_t OneWire_GetROM(OneWire_t* OneWireStruct, uint8_t index) {
	return OneWireStruct->ROM_NO[index];
}

void OneWire_Select(OneWire_t* OneWireStruct, uint8_t* addr) {
	uint8_t i;
	OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_MATCHROM);
	
	for (i = 0; i < 8; i++) {
		OneWire_WriteByte(OneWireStruct, *(addr + i));
	}
}

void OneWire_SelectWithPointer(OneWire_t* OneWireStruct, uint8_t *ROM) {
	uint8_t i;
	OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_MATCHROM);
	
	for (i = 0; i < 8; i++) {
		OneWire_WriteByte(OneWireStruct, *(ROM + i));
	}	
}

void OneWire_GetFullROM(OneWire_t* OneWireStruct, uint8_t *firstIndex) {
	uint8_t i;
	for (i = 0; i < 8; i++) {
		*(firstIndex + i) = OneWireStruct->ROM_NO[i];
	}
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
	
	/* Return calculated CRC */
	return crc;
}

void OneWire_ReadROM(OneWire_t* OneWireStruct)
{
	OneWire_Reset(OneWireStruct);
	OneWire_WriteByte(OneWireStruct,ONEWIRE_CMD_READROM);
	for(uint8_t i=0; i<8; i++) 
	{
			OneWireStruct->ROM_NO[i]=OneWire_ReadByte(OneWireStruct);
	}
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


static void OneWire_Task(void *pvParameters)
{
//	OneWire_Output();
//	htim7.Init.Period = 45;
//	HAL_TIM_Base_Init(&htim7);
//	HAL_TIM_Base_Start_IT(&htim7);
	while(1)
	{  
		OneWire_DS18b20(&OneWireStruct);
	//	OneWire_WriteByte(&OneWireStruct,0x00);
		//OneWire_ReadByte(&OneWireStruct);
		vTaskDelay(ONEWIRE_READ_PERIOD);
		//HAL_TIM_Base_Start_IT(&htim7);
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
				//htim7.Init.Period = 15;
				//HAL_TIM_Base_Init(&htim7);
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
//				htim7.Init.Period = 45;
//				HAL_TIM_Base_Init(&htim7);
				TIM7->ARR=45;
			  HAL_TIM_Base_Start_IT(&htim7);
				ow_state=OW_STATE_TIMESLOT_END;
		}
		break;
		
		case OW_STATE_WRITE_1:
		{
				ONEWIRE_HIGH();
//				htim7.Init.Period = 45;
//				HAL_TIM_Base_Init(&htim7);
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
//				htim7.Init.Period = 45;
//				HAL_TIM_Base_Init(&htim7);
				TIM7->ARR=45;
			  HAL_TIM_Base_Start_IT(&htim7);	
				ow_state=OW_STATE_TIMESLOT_END;		
		}
		break;
		
		case OW_STATE_TIMESLOT_END:
		{
				OneWire_Output();
				ONEWIRE_HIGH();
			  //ONEWIRE_LOW();
				ow_state=OW_STATE_IDLE;
		}
		break;
	}
	
	

//	HAL_GPIO_TogglePin(in_1_wire_in_GPIO_Port, in_1_wire_in_Pin);
//	HAL_TIM_Base_Start_IT(&htim7);
}
