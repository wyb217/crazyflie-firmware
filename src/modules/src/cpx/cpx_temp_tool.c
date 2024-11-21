#include "cpx_temp_tool.h"
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

uint32_t receiveData(uint8_t *buffer, uint32_t maxLength)
{
    uint32_t totalReceived = 0;
    uint8_t byte;

    // 循环接收数据，直到接收到完整的 CPX 数据包
    while (totalReceived < maxLength)
    {
        if (uart2GetDataWithTimeout(1, &byte, M2T(1000)))
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