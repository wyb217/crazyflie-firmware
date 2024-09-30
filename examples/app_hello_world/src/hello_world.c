#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "app.h"
#include "FreeRTOS.h"
#include "task.h"
#include "log.h"

#include "uart1.h"
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

  // // 检查包头和包尾是否正确
  // if (rxBuffer[0] != 0xAA || rxBuffer[length - 1] != 0x55)
  // {
  //   DEBUG_PRINT("Error: Invalid packet start or end marker\n");
  //   return;
  // }

  // // 解析第一个字节为 destination 和 source
  // rxPacket->route.destination = rxBuffer[1] & 0x07;       // 低 3 位为 destination
  // rxPacket->route.source = (rxBuffer[1] >> 3) & 0x07;     // 中间 3 位为 source
  // rxPacket->route.lastPacket = (rxBuffer[1] >> 6) & 0x01; // 第 6 位为 lastPacket

  // // 解析第二个字节为 version 和 function
  // rxPacket->route.version = rxBuffer[2] & 0x03;         // 低 2 位为 version
  // rxPacket->route.function = (rxBuffer[2] >> 2) & 0x3F; // 高 6 位为 function

  // 提取数据长度，减去路由头部和包头包尾
  rxPacket->dataLength = length;
  DEBUG_PRINT("rxPacket->dataLength: %d\n", rxPacket->dataLength);

  // 复制数据到 CPX 数据包
  memcpy(rxPacket->data, rxBuffer, rxPacket->dataLength);
}

uint32_t receiveData(uint8_t *buffer, uint32_t maxLength)
{
  uint32_t totalReceived = 0;
  uint8_t byte;

  // 循环接收数据，直到接收到完整的 CPX 数据包
  while (totalReceived < maxLength)
  {
    if (uart1GetDataWithDefaultTimeout(&byte))
    {
      buffer[totalReceived++] = byte;

      // 检查是否收到包尾，确保接收完整
      if (totalReceived >= 2 && buffer[totalReceived - 1] == 0x55)
      {
        break; // 已经收到包尾，停止接收
      }
    }
    else
    {
      // 接收完就停止接收
      break;
    }
  }
  return totalReceived; // 返回长度
}

void appMain()
{
  DEBUG_PRINT("Hello! I am the stm_athena_cpx app\n");
  UartRxReady = xSemaphoreCreateMutex();
  uart1Init(115200);

  while (1)
  {
    uint32_t receivedLength = receiveData(rxBuffer, sizeof(rxBuffer));
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
