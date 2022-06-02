

#include "Task1.hpp"
#include "TaskMonitor.hpp"

#include "main.h"
#include "stdio.h"

static constexpr uint32_t TIMEOUT = 10 * MS_PER_SEC; // 10 seconds

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void Task1::TaskCheckin()
{
    Message msg(TASK_CHECKIN);
    ASSERT(osOK == osMessageQueuePut(task1QHandle, &msg, 0, 0));
}

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void Task1::Process()
{
    Message msg(PROCESS);
    ASSERT(osOK == osMessageQueuePut(task1QHandle, &msg, 0, 0));
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void Task1::Shutdown()
{
    Message msg(SHUTDOWN);
    ASSERT(osOK == osMessageQueuePut(task1QHandle, &msg, 0, 0));
}

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void Task1::Initialize()
{
    Message msg(INITIALIZE);
    ASSERT(osOK == osMessageQueuePut(task1QHandle, &msg, 0, 0));
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void Task1::Run()
{
    osStatus_t status = osError;
    Message msg;

    while (true)
    {
        // Check for new data ready message indicating RAM buffer is full
        status = osMessageQueueGet(task1QHandle, &msg, NULL, osWaitForever);

        // Check status of message Q result
        if (status != osOK && status != osErrorTimeout)
        {
            continue;
        }

        switch (msg.msgId)
        {
            case TASK_CHECKIN:
                TaskMonitor::TaskCheckin();
                break;

            case INITIALIZE:
                HandleInitialize();
                break;

            case PROCESS:
                HandleProcess();
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

//------------------------------------------------------------------
// HandleInitialize
//------------------------------------------------------------------
void Task1::HandleInitialize()
{
    TaskMonitor::Register(TIMEOUT, &TaskCheckin);
    printf("Default Task Intialized\r\n");
    Process();
}

//------------------------------------------------------------------
// HandleProcess
//------------------------------------------------------------------
void Task1::HandleProcess()
{
    printf("Do Some task 1 stuff\r\n");
    DELAY_MS(1000);
    Process();
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void Task1::HandleShutdown()
{
    printf("Task 1 Shutdown\r\n");
    DELAY_MS(osWaitForever);
}
