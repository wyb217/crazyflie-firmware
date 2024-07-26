#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "app.h"
#include "FreeRTOS.h"
#include "task.h"
#include "debug.h"
#include "log.h"

#include "control_task.h"
#include "uart_task.h"

#define DEBUG_MODULE "THEAPP"
#include "debug.h"

#define UART_TASK_PRI 1
#define UART_STACK_SIZE 1000
TaskHandle_t uartTask_Handlar;

#define CONTROL_TASK_PRI 1
#define CONTROL_STACK_SIZE 1000
TaskHandle_t controlTask_Handlar;

#define APP_MAIN_PRI 1
#define APP_STACK_SIZE 1000
TaskHandle_t appMainTask_Handlar;

void appMainTask(void *param);

void appMain(){
    vTaskDelay(M2T(3000));
    BaseType_t app_return = pdPASS;
    app_return = xTaskCreate(appMainTask,"app main task",APP_STACK_SIZE,NULL,APP_MAIN_PRI,appMainTask_Handlar);
    // if (pdPASS == app_return)
    //     DEBUG_PRINT("Create appMainTask Successfully!\n");

}

void appMainTask(void *param)
{
    taskENTER_CRITICAL();
    
    BaseType_t uart_return = pdPASS;
    BaseType_t crtl_return = pdPASS;

    uart_return = xTaskCreate(uartTask, "uart task", UART_STACK_SIZE, NULL, UART_TASK_PRI, controlTask_Handlar);
    if (pdPASS == uart_return)
        DEBUG_PRINT("Create uartTask Successfully!\n");

    crtl_return = xTaskCreate(controlTask, "fly control task", CONTROL_STACK_SIZE, NULL, CONTROL_TASK_PRI, controlTask_Handlar);
    if (pdPASS == crtl_return)
        DEBUG_PRINT("Create controlTask Successfully!\n");

    vTaskDelete(appMainTask_Handlar);
    taskEXIT_CRITICAL();  
}