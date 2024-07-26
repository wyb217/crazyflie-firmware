/*-----------------------------------------------------------------------------
 Copyright (C) 2020-2021 ETH Zurich, Switzerland, University of Bologna, Italy.
 All rights reserved.

 File:    main.c
 Author:  Vlad Niculescu      <vladn@iis.ee.ethz.ch>
 Date:    15.03.2021
-------------------------------------------------------------------------------*/

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "app.h"
#include "FreeRTOS.h"
#include "task.h"
#include "debug.h"
#include "log.h"
#include "uart_dma_pulp.h"

#define DEBUG_MODULE "UART"
#include "debug.h"
#define BUFFERSIZE 1
#define DMASIZE 3
uint8_t aideckRxBuffer[BUFFERSIZE];
uint8_t aideckRxDMA[DMASIZE];

volatile uint8_t dma_flag = 0;
uint8_t log_counter = 0;
static uint8_t flag;

typedef struct
{
	float steer;
	float coll;
	float sign;
	bool refresh;
} control_data_t;
static control_data_t control_data;

bool ctlGetUartInfo(float *steer, float *coll, float *sign)
{
	if (control_data.refresh == true)
	{
		control_data.refresh == false;
		*steer = control_data.steer;
		*coll = control_data.coll;
		*sign = control_data.sign;
		memset(&control_data.steer, 0, sizeof(control_data.steer));
		memset(&control_data.sign, 0.5, sizeof(control_data.sign));
		return (true);
	}
	else
	{
		return (false);
	}
}

void uartTask(void *param)
{
	DEBUG_PRINT("uart task started! \n");
	USART_DMA_Start(115200, aideckRxBuffer, BUFFERSIZE);

	while (1)
	{
		vTaskDelay(M2T(100));
		if (log_counter <= 15)
		{
			memset(aideckRxBuffer, 0, BUFFERSIZE); // clear the dma buffer
			memset(aideckRxDMA, 0, DMASIZE);	   // clear the dma buffer
			continue;
		}
		if (dma_flag == 1)
		{
			dma_flag = 0; // clear the flag
			aideckRxDMA[flag++] = aideckRxBuffer[0];
			memset(aideckRxBuffer, 0, BUFFERSIZE); // clear the dma buffer
			if (flag == DMASIZE)
			{
				control_data.steer = (double)aideckRxDMA[0] / 100 - 1;
				control_data.coll = (double)aideckRxDMA[1] / 100;
				control_data.sign = (double)aideckRxDMA[2] / 100;
				control_data.refresh = true;
				// DEBUG_PRINT("steer:%.2f \tcollision:%.2f \t  sign: %.f\n", control_data.steer,control_data.coll,control_data.sign);
				memset(aideckRxDMA, 0, DMASIZE); // clear the dma buffer
				flag = 0;
			}
		}
	}
}

void __attribute__((used)) DMA1_Stream1_IRQHandler(void)
{
	DMA_ClearFlag(DMA1_Stream1, UART3_RX_DMA_ALL_FLAGS);
	dma_flag = 1;
	log_counter++;
}

LOG_GROUP_START(log_test)
LOG_ADD(LOG_UINT8, test_variable_x, &log_counter)
LOG_GROUP_STOP(log_test)
