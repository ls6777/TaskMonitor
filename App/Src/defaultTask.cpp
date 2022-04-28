

#include "defaultTask.hpp"
#include "TaskMonitor.hpp"

#include "main.h"
#include "task.h"
#include "stdio.h"

static char taskListBuf[500];
static char taskStatsBuf[500];

extern unsigned long lpCounter;

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void defaultTask::TaskCheckin()
{
    Message msg(TASK_CHECKIN);
    ASSERT(osOK == osMessageQueuePut(defaultTaskQHandle, &msg, 0, 0));
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void defaultTask::Shutdown()
{
    Message msg(SHUTDOWN);
    ASSERT(osOK == osMessageQueuePut(defaultTaskQHandle, &msg, 0, 0));
}

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void defaultTask::Initialize()
{
    Message msg(INITIALIZE);
    ASSERT(osOK == osMessageQueuePut(defaultTaskQHandle, &msg, 0, 0));
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void defaultTask::Run()
{
    constexpr uint32_t MSG_Q_TIMEOUT = 1000; // ms
    osStatus_t status = osError;
    Message msg;

    const uint16_t STATS_DELAY = 30000/MSG_Q_TIMEOUT;
    uint16_t count = 0;

    while (true)
    {
        // Check for new data ready message indicating RAM buffer is full
        status = osMessageQueueGet(defaultTaskQHandle, &msg, NULL, pdMS_TO_TICKS(MSG_Q_TIMEOUT));

        // Check status of message Q result
        if (status != osOK && status != osErrorTimeout)
        {
            continue;
        }

        if (status != osErrorTimeout)
        {
            switch (msg.msgId)
            {
                case TASK_CHECKIN:
                    TaskMonitor::TaskCheckin();
                    break;

                case INITIALIZE:
                    HandleInitialize();
                    break;

                case SHUTDOWN:
                    HandleShutdown();
                    break;

                default:
                    printf("defaultTask - unknown msgId: %d\r\n", msg.msgId);
                    break;
            }
        }

        if (count == STATS_DELAY)
        {
            count = 0;

            printf("LP Counter: %lu\r\n\r\n", lpCounter);

            vTaskList(taskListBuf);
            printf("TASK INFO**********************************************\r\n");
            printf("Name          State  Priority   Stack   Num\r\n");
            printf("*******************************************\r\n");
            printf("%s\r\n", taskListBuf);
            printf("\r\n");

            vTaskGetRunTimeStats(taskStatsBuf);
            printf("TASK STATS INFO****************************************\r\n");
            printf("Name             Abs Time       %% Time\r\n");
            printf("*******************************************\r\n");
            printf("%s\r\n", taskStatsBuf);
            printf("\r\n");
        }
    }
}

//---------------------------------------------------
// HandleInitialize
//---------------------------------------------------
void defaultTask::HandleInitialize()
{
    printf("Default Task Intialized\r\n");
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void defaultTask::HandleShutdown()
{
    DELAY_MS(osWaitForever);
}
