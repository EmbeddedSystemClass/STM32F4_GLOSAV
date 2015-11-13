#include "count_input_app.h"
#include "mb_app.h"

#define COUNT_INPUT_1_PORT GPIOE
#define COUNT_INPUT_1_PIN	GPIO_PIN_9

#define COUNT_INPUT_2_PORT GPIOE
#define COUNT_INPUT_2_PIN	GPIO_PIN_11

stCountInput CountInput[COUNT_INPUTS_NUM];

void Count_Input_App_Init(void)
{
	 CountInput[0].port=COUNT_INPUT_1_PORT;
	 CountInput[0].pin=COUNT_INPUT_1_PIN;
	 CountInput[0].mode=COUNT_REDGE;
	
	 CountInput[1].port=COUNT_INPUT_2_PORT;
	 CountInput[1].pin=COUNT_INPUT_2_PIN;
	 CountInput[1].mode=COUNT_REDGE;
}

void Count_Input_SetMode(uint8_t mode)
{
	uint8_t input_counter;
	for(input_counter=0;input_counter<COUNT_INPUTS_NUM;input_counter++)
	{
			CountInput[input_counter].mode=mode&0x3;
			mode>>=2;
	}
}

void Count_Input_InterruptHandler(void)
{
	uint8_t input_counter;
	for(input_counter=0;input_counter<COUNT_INPUTS_NUM;input_counter++)
	{
		CountInput[input_counter].discreteStateOld=CountInput[input_counter].discreteState;	
		CountInput[input_counter].discreteState=HAL_GPIO_ReadPin(CountInput[input_counter].port,CountInput[input_counter].pin);
		
		if(CountInput[input_counter].mode==COUNT_REDGE)
		{
				if((CountInput[input_counter].discreteState==GPIO_PIN_SET)&&(CountInput[input_counter].discreteStateOld==GPIO_PIN_RESET))
				{
						MBHoldingRegParams.params.countInputs[input_counter]++;
				}
		}
		else if(CountInput[input_counter].mode==COUNT_REDGE)
		{
				if((CountInput[input_counter].discreteState==GPIO_PIN_RESET)&&(CountInput[input_counter].discreteStateOld==GPIO_PIN_SET))
				{
						MBHoldingRegParams.params.countInputs[input_counter]++;
				}					
		}	

	}
}
