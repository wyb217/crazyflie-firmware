#include "cpx_temp_tool.h"
#define DEBUG_MODULE "ATHENA"

static CPXPacket_t rxPacket;
static uint8_t rxBuffer[CPX_MAX_PAYLOAD_SIZE + CPX_HEADER_SIZE + 2]; // 包含包头和包尾
SemaphoreHandle_t UartRxReady = NULL;

void appMain()
{
  DEBUG_PRINT("Hello! I am the stm_athena_cpx app\n");
  uart2Init(115200);

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
