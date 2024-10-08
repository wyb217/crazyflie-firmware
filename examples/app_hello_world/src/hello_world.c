#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "app.h"
#include "FreeRTOS.h"
#include "task.h"
#include "log.h"

#include "uart2.h"
#include "semphr.h"
#include "uart_syslink.h"
#include "timers.h"
#include "debug.h"
#include "log.h"
#include "param.h"
#include "cpx.h"

#define DEBUG_MODULE "ATHENA"

static CPXPacket_t rxPacket;
static uint8_t rxBuffer[CPX_MAX_PAYLOAD_SIZE + CPX_HEADER_SIZE + 2]; // 包含包头和包尾
SemaphoreHandle_t UartRxReady = NULL;

void parseCPXPacket(CPXPacket_t *rxPacket, uint8_t *rxBuffer, uint32_t length)
{
  if (length < CPX_HEADER_SIZE + 2)
  {
    DEBUG_PRINT("Error: Received data length too short\n");
    return;
  }
  // 提取数据长度，减去路由头部和包头包尾
  rxPacket->dataLength = length;
  DEBUG_PRINT("rxPacket->dataLength: %d\n", rxPacket->dataLength);

  // 复制数据到 CPX 数据包
  memcpy(rxPacket->data, rxBuffer, rxPacket->dataLength);
}

void appMain()
{
  DEBUG_PRINT("Hello! I am the stm_athena_cpx app\n");
  UartRxReady = xSemaphoreCreateMutex();
  uart2Init(115200);

  while (1)
  {
    uint32_t receivedLength = uart2GetData(10, rxBuffer);
    if (receivedLength >= CPX_HEADER_SIZE + 2)
    {
      // 解包
      parseCPXPacket(&rxPacket, rxBuffer, receivedLength);
      DEBUG_PRINT("Received CPX data from Athena: ");
      for (int i = 0; i < rxPacket.dataLength; i++)
      {
        DEBUG_PRINT("%u\t", rxPacket.data[i]);
      }
      DEBUG_PRINT("\n");
    }
    else
    {
      DEBUG_PRINT("Error: Received data length too short\n");
    }

    vTaskDelay(M2T(1));
  }
}
