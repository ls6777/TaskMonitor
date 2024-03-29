

#include "task2.hpp"
#include "TaskMonitor.hpp"

#include "main.h"
#include "stdio.h"

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void task2::TaskCheckin()
{
    Message msg(TASK_CHECKIN);
    ASSERT(osOK == osMessageQueuePut(task1QHandle, &msg, 0, 0));
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void task2::Shutdown()
{
    Message msg(SHUTDOWN);
    ASSERT(osOK == osMessageQueuePut(task1QHandle, &msg, 0, 0));
}

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void task2::Initialize()
{
    Message msg(INITIALIZE);
    ASSERT(osOK == osMessageQueuePut(task1QHandle, &msg, 0, 0));
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void task2::Run()
{
    constexpr uint32_t MSG_Q_TIMEOUT = 100; // ms
    osStatus_t status = osError;
    Message msg;

    while (true)
    {
        // Check for new data ready message indicating RAM buffer is full
        status = osMessageQueueGet(task1QHandle, &msg, NULL, pdMS_TO_TICKS(MSG_Q_TIMEOUT));

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
                    printf("task1 - unknown msgId: %d\r\n", msg.msgId);
                    break;
            }
        }
    }
}

//---------------------------------------------------
// HandleInitialize
//---------------------------------------------------
void task2::HandleInitialize()
{
    printf("Default Task Intialized\r\n");
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void task2::HandleShutdown()
{
    DELAY_MS(osWaitForever);
}
