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
static void init()
{
  cpxUARTTransportInit();
  cpxInternalRouterInit();
  cpxExternalRouterInit();
  cpxInit();
}

void parseCPXPacket(CPXRoutablePacket_t *rxPacket, uint8_t *rxBuffer)
{
  uint16_t length = 0;
  memcpy(&length, rxBuffer, 2);
  rxPacket->dataLength = length - CPX_HEADER_SIZE;
  DEBUG_PRINT("rxPacket->dataLength: %d\n", rxPacket->dataLength);
  rxPacket->route.destination = (rxBuffer[2] >> 5) & 0x07;
  rxPacket->route.source = (rxBuffer[2] >> 2) & 0x07;
  rxPacket->route.lastPacket = (rxBuffer[2] >> 1) & 0x01;
  rxPacket->route.function = (rxBuffer[3] >> 2) & 0x3F;
  rxPacket->route.version = rxBuffer[3] & 0x03;
  // 复制数据到 CPX 数据包
  memcpy(rxPacket->data, rxBuffer + 4, rxPacket->dataLength);
  xQueueSend(mixedQueue, rxPacket, portMAX_DELAY);
}

void receiveData(uint8_t *buffer, uint32_t maxLength)
{
  uint16_t totalReceived = 0;
  uint8_t byte;

  // 循环接收数据，直到接收到完整的 CPX 数据包
  while (totalReceived < maxLength)
  {
    if (uart2GetDataWithTimeout(1, &byte, M2T(1000)))
    {
      buffer[totalReceived++] = byte;
    }
    else
    {
      // 接收完就停止接收
      break;
    }
  }
}

void appMain()
{
  init();
  DEBUG_PRINT("Hello! I am the stm_athena_cpx app\n");

  // Register a callback for CPX packets.
  // Packets sent to destination=CPX_T_STM32 and function=CPX_F_APP will arrive here
  cpxRegisterAppMessageHandler(cpxPacketCallback);

  while (1)
  {
    vTaskDelay(M2T(2000));
    receiveData(rxbuffer, CPX_MAX_PAYLOAD_SIZE);
    parseCPXPacket(&rxPacket, rxbuffer);
    // cpxInitRoute(CPX_T_STM32, CPX_T_ATHENA, CPX_F_APP, &txPacket.route);
    // txPacket.data[0] = counter;
    // txPacket.dataLength = 1;

    // cpxSendPacketBlocking(&txPacket);
    // DEBUG_PRINT("Sent packet to Athena (%u)\n", counter);
    // counter++;
  }
}

static void cpxPacketCallback(const CPXPacket_t *cpxRx)
{
  DEBUG_PRINT("Got packet from Athena (%u)\n", cpxRx->data[0]);
}