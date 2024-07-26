#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "app.h"
#include "commander.h"
#include "FreeRTOS.h"
#include "task.h"
#include "debug.h"
#include "log.h"
#include "param.h"
#include "crtp.h"
#include "crtp_commander_high_level.h"
#include "control_task.h"
#include "uart_task.h"

#define DEBUG_MODULE "CRTL"
#include "debug.h"

const float alpha = 0.7;
const float beta = 0.5;
const float PI = 180.0;
const float flyHeight = 0.3f; // fixed fly height
const float velMax = 0.2f;

static float velUsual;
static float angleTheta;

static float steer;
static float coll;
float sign = 0.5;

typedef enum
{
    idle,
    unlocked,
    stopping
} State;
static State state = idle;
typedef struct
{
    float_t steer_ctl[3];
    float_t coll_ctl[3];
    uint8_t index;
} median_data_t;

static median_data_t median_data;

static float_t median_filter_3(float_t *data)
{
    float_t middle;
    if ((data[0] <= data[1]) && (data[0] <= data[2]))
    {
        middle = (data[1] <= data[2]) ? data[1] : data[2];
    }
    else if ((data[1] <= data[0]) && (data[1] <= data[2]))
    {
        middle = (data[0] <= data[2]) ? data[0] : data[2];
    }
    else
    {
        middle = (data[0] <= data[1]) ? data[0] : data[1];
    }
    return middle;
}

static void setHoverSetpoint(setpoint_t *setpoint, float vx, float vy, float z, float yawrate)
{
    setpoint->mode.z = modeAbs;
    setpoint->position.z = z;
    setpoint->mode.yaw = modeVelocity;
    setpoint->attitudeRate.yaw = yawrate;
    setpoint->mode.x = modeVelocity;
    setpoint->mode.y = modeVelocity;
    setpoint->velocity.x = vx;
    setpoint->velocity.y = vy;
    setpoint->velocity_body = true;
}

void controlTask(void *param)
{
    static setpoint_t setpoint;
    DEBUG_PRINT("control task started!\n");

    while (1)
    {
        vTaskDelay(M2T(250));
        if (state == unlocked)
        {
            if (ctlGetUartInfo(&steer, &coll, &sign))
            {
                if(coll != 0)
                    median_data.coll_ctl[median_data.index++] = coll;
                if(median_data.index%3==0)
                    median_data.index = 0;
                DEBUG_PRINT("collision:%.2f \t",coll);
                coll = median_filter_3(median_data.coll_ctl);
                angleTheta = ((1 - beta) * angleTheta + beta * (PI / 3) *steer);
                velUsual = (1 - alpha) * velUsual + alpha * (1-coll) * velMax;
                DEBUG_PRINT("angleTheta:%.2f \t velUsual:%.2f \t sign: %.2f\n",angleTheta,velUsual,sign);
                // DEBUG_PRINT("\t media_coll:%.2f \t velocity: %.2f\n",coll,velUsual);
            }
            setHoverSetpoint(&setpoint, velUsual, 0, flyHeight, angleTheta);
            commanderSetSetpoint(&setpoint, 3);
            // TODO clear angle
            memset(&steer, 0, sizeof(steer));
            memset(&sign,0.5,sizeof(sign));
            
        }
        else if (state == stopping)
        {
            memset(&setpoint, 0, sizeof(setpoint_t));
            commanderSetSetpoint(&setpoint, 3);
        }
    }
}

PARAM_GROUP_START(aideck_fly)
PARAM_ADD(PARAM_UINT8, state, &state)
PARAM_GROUP_STOP(aideck_fly)
