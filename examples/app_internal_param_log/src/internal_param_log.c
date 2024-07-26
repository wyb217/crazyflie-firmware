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
#include <time.h>

#include "app.h"
#include "FreeRTOS.h"
#include "task.h"
#include "debug.h"
#include "uart_dma_pulp.h"
#include "log.h"

#define DEBUG_MODULE "UART_TASK"
#include "debug.h"

#define BUFFERSIZE 1
#define BUFFERDATA 2

uint8_t aideckRxBuffer[BUFFERSIZE];
volatile uint8_t dma_flag = 0;
uint8_t log_counter = 0;
static uint8_t aideckRxData[BUFFERDATA];
uint8_t flag = 0;

float steer;
float coll;

float test_vx = 0.0f;
float test_vy = 0.0f;
float test_height = 0.3f;

void uartTask(void *param)
{
  DEBUG_PRINT("uart task started! \n");
  USART_DMA_Start(115200, aideckRxBuffer, BUFFERSIZE);

  double curtime;

  while (1)
  {
    vTaskDelay(M2T(10));
    if (log_counter < 5)
    {
      // DEBUG_PRINT("log_counter: %d | %d\n", log_counter, aideckRxBuffer[0]);
      memset(aideckRxBuffer, 0, sizeof(uint8_t) * BUFFERSIZE); // clear the dma buffer
      memset(aideckRxData, 0, sizeof(uint8_t) * BUFFERDATA);
      continue;
    }
    if (dma_flag == 1)
    {
      // DEBUG_PRINT("log_counter: %d | %d\n", log_counter, aideckRxBuffer[0]);
      dma_flag = 0; // clear the flag
      aideckRxData[flag++] = aideckRxBuffer[0];
      memset(aideckRxBuffer, 0, sizeof(uint8_t) * BUFFERSIZE);
      if (flag == BUFFERDATA)
      {
        // DEBUG_PRINT("\tsteer: %d;coll: %d\n", aideckRxData[0],aideckRxData[1]);
        curtime = (double)xTaskGetTickCount() / 1000;
        steer = (double)aideckRxData[0] / 10 - 2;
        coll = (double)aideckRxData[1] / 100;
        // DEBUG_PRINT("time: %.3fms \t steer: %.2f \t coll: %.2f\n",curtime,(double)aideckRxData[0]/10-2,(double)aideckRxData[1]/100);
        memset(aideckRxData, 0, sizeof(uint8_t) * BUFFERDATA); // clear the dma buffer
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
LOG_ADD(LOG_UINT32, test_variable_x, &log_counter)
LOG_GROUP_STOP(log_test)
