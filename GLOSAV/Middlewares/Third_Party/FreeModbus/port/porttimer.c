/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: porttimer.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

/* ----------------------- Platform includes --------------------------------*/
#include "cmsis_os.h"
#include "port.h"
#include "tim.h"
#include "config.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvTIMERExpiredISR( void );

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBPortTimersInit( USHORT usTim1Timerout50us )
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  //Настройка таймера, требуется чтобы он сгенерировал прерывание через 50uS * usTim1Timerout50us
  //при этом требуется чтобы он не переполнялся в течении секунды  TIM_ClockConfigTypeDef sClockSourceConfig;

	HAL_TIM_Base_Stop(&htim2);
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = (uint32_t)3600 * usTim1Timerout50us;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim2);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

  return TRUE;
}


inline void
vMBPortTimersEnable(  )
{
    /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	HAL_TIM_Base_Start_IT(&htim2);
}

inline void
vMBPortTimersDisable(  )
{
    /* Disable any pending timers. */
	HAL_TIM_Base_Stop(&htim2);
	__HAL_TIM_SET_COUNTER(&htim2, 0);
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */
static void prvvTIMERExpiredISR( void )
{
    BOOL bTaskWoken = FALSE;
	
    vMBPortSetWithinException( TRUE );
//  pxMBFrameCBByteReceived(  );
    bTaskWoken = pxMBPortCBTimerExpired(  );
    vMBPortSetWithinException( FALSE );

    portEND_SWITCHING_ISR( bTaskWoken ? pdTRUE : pdFALSE );
}

// Callback функция прерывания от таймеров
void HAL_TIM_PeriodElapsedCallback ( TIM_HandleTypeDef * htimer)
{
	switch((uint32_t)(htimer->Instance)){
		case (uint32_t)TIM2:
			prvvTIMERExpiredISR();
			//led2_toggle();
			break;
	}
}
