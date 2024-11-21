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
void parseCPXPacket(CPXPacket_t *rxPacket, uint8_t *rxBuffer, uint32_t length);
uint32_t receiveData(uint8_t *buffer, uint32_t maxLength);
