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

#include "FreeRTOS.h"
#include "task.h"
#include "log.h"
#include "uart1.h"

#define DEBUG_MODULE "APP"
#include "debug.h"

void appMain()
{
    DEBUG_PRINT("Hello! I am the stm_athena_uart app\n");
    uart1Init(115200);
    uint8_t counter[2];
    while (1)
    {
        // 等待1秒钟
        vTaskDelay(M2T(1000));
        // 尝试接收2字节的数据
        if (uart1GetDataWithDefaultTimeout(counter))
        {
            // 打印接收到的数据
            DEBUG_PRINT("Received data from Athena: %02x %02x\n", counter[0], counter[1]);
        }
        else
        {
            // 如果未成功接收数据，打印错误信息
            DEBUG_PRINT("Failed to receive data from Athena\n");
        }
    }
}