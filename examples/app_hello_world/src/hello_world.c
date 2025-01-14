/**
 * ,---------,       ____  _ __
 * |  ,-^-,  |      / __ )(_) /_______________ _____  ___
 * | (  O  ) |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * | / ,--´  |    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *    +------`   /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2023 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * App layer application that communicates with the GAP8 on an AI deck.
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "app.h"

#include "cpx.h"
#include "cpx_internal_router.h"
#include "cpx_external_router.h"
#include "cpx_uart_transport.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "uart2.h"
#define DEBUG_MODULE "APP"
#include "debug.h"
typedef struct
{
  CPXRoutablePacket_t txp;
} RouteContext_t;

extern xQueueHandle mixedQueue;
// Callback that is called when a CPX packet arrives
static void cpxPacketCallback(const CPXPacket_t *cpxRx);
uint8_t *rxbuffer;
CPXRoutablePacket_t rxPacket;
static CPXPacket_t txPacket;
static void init()
{
  cpxUARTTransportInit();
  cpxInternalRouterInit();
  cpxExternalRouterInit();
  cpxInit();
}

void appMain()
{
  uint8_t test_data[] = {0, 1, 2, 100, 101, 102};
  init();
  //  vTaskDelay(M2T(5000));
  // uart2Init(115200);
  DEBUG_PRINT("Hello! I am the stm_athena_cpx app\n");
  // Register a callback for CPX packets.
  // Packets sent to destination=CPX_T_STM32 and function=CPX_F_APP will arrive here
  cpxRegisterAppMessageHandler(cpxPacketCallback);
  DEBUG_PRINT("registered callback\n");
  while (1)
  {
    vTaskDelay(M2T(2000));
    // uart2GetData(1, &test);
    // DEBUG_PRINT("%u\n", test);
    cpxInitRoute(CPX_T_STM32, CPX_T_GAP8, CPX_F_APP, &txPacket.route);
    memcpy(txPacket.data, test_data, sizeof(test_data));
    txPacket.dataLength = 6;
    cpxSendPacketBlocking(&txPacket);
    DEBUG_PRINT("Sent packet to Athena: [");
    for (size_t i = 0; i < txPacket.dataLength; i++)
    {
      DEBUG_PRINT("%d%s", txPacket.data[i], (i < txPacket.dataLength - 1) ? ", " : "");
    }
    DEBUG_PRINT("]\n");
  }
}

static void cpxPacketCallback(const CPXPacket_t *cpxRx)
{
  DEBUG_PRINT("Got packet from Athena (%d)\n", cpxRx->data[0]);
}