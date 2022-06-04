//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"

#include "DefaultTask.hpp"
#include "Message.hpp"
//#include "TaskMonitor.hpp"
#include "Utils.hpp"


static char taskListBuf[500];
static char taskStatsBuf[500];

QueueHandle_t DefaultTask::msgQHandle;

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void DefaultTask::TaskCheckin()
{
    Message msg(Message::TASK_CHECKIN);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void DefaultTask::Shutdown()
{
    Message msg(Message::SHUTDOWN);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void DefaultTask::Initialize()
{
    msgQHandle = xQueueCreate(10, sizeof(Message));
    Message msg(Message::INITIALIZE);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void DefaultTask::Run()
{
    constexpr uint32_t MSG_Q_TIMEOUT = 1000; // ms
    BaseType_t status = pdFALSE;
    Message msg;

    const uint32_t STATS_DELAY = 30000; // ms

    uint32_t prevTime = KERNEL_TICKS_IN_MS();

    while (true)
    {
        // Check for new data ready message indicating RAM buffer is full
        status = xQueueReceive(msgQHandle, &msg, pdMS_TO_TICKS(MSG_Q_TIMEOUT));

        if (pdTRUE == status)
        {
            switch (msg.msgId)
            {
                case Message::TASK_CHECKIN:
//                    TaskMonitor::TaskCheckin();
                    break;

                case Message::INITIALIZE:
                    HandleInitialize();
                    break;

                case Message::SHUTDOWN:
                    HandleShutdown();
                    break;

                default:
                    printf("defaultTask - unknown msgId: %d\r\n", msg.msgId);
                    break;
            }
        }

        uint32_t currTime = KERNEL_TICKS_IN_MS();
        if ((currTime - prevTime) > STATS_DELAY)
        {
            prevTime = KERNEL_TICKS_IN_MS();

            vTaskList(taskListBuf);
            printf("TASK INFO**********************************************\r\n");
            printf("Name          State  Priority   Stack   Num    Core\r\n");
            printf("*******************************************************\r\n");
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
void DefaultTask::HandleInitialize()
{
    printf("Default Task Intialized\r\n");
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void DefaultTask::HandleShutdown()
{
    DELAY_MS(portMAX_DELAY);
}
