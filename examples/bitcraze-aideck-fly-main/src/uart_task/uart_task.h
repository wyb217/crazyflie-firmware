#ifndef __UART_TASK_H
#define __UART_TASK_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stm32f4xx.h"


#ifdef __cplusplus
}
#endif



void uartTask(void *param);
bool ctlGetUartInfo(float* steer, float* coll, float* sign);
#endif