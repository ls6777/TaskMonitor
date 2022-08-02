
#include "Task2.hpp"
#include "TaskMonitor.hpp"
#include "Message.hpp"
#include "Utils.hpp"

static constexpr uint32_t TIMEOUT = 1 * MS_PER_SEC; // 1 second

QueueHandle_t Task2::msgQHandle;

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void Task2::TaskCheckin()
{
    Message msg(Message::TASK_CHECKIN);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Process
//------------------------------------------------------------------
void Task2::Process()
{
    Message msg(Message::PROCESS);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void Task2::Shutdown()
{
    Message msg(Message::SHUTDOWN);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void Task2::Initialize()
{
    msgQHandle = xQueueCreate(10, sizeof(Message));
    Message msg(Message::INITIALIZE);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void Task2::Run()
{
    constexpr uint32_t MSG_Q_TIMEOUT = 100; // ms
    BaseType_t status = pdFALSE;
    Message msg;

    while (true)
    {
        // Wait for new message to process
        status = xQueueReceive(msgQHandle, &msg, pdMS_TO_TICKS(MSG_Q_TIMEOUT));

        // Check status of message Q result
        if (pdTRUE == status)
        {
            // process message
            switch (msg.msgId)
            {
                case Message::TASK_CHECKIN:
                    TaskMonitor::TaskCheckin();
                    break;

                case Message::INITIALIZE:
                    HandleInitialize();
                    break;

                case Message::PROCESS:
                    HandleProcess();
                    break;

                case Message::SHUTDOWN:
                    HandleShutdown();
                    break;

                default:
                    printf("task2 - unknown msgId: %d\r\n", msg.msgId);
                    break;
            }
        }
        else
        {
            Process();
        }
    }
}

//---------------------------------------------------
// HandleInitialize
//---------------------------------------------------
void Task2::HandleInitialize()
{
    TaskMonitor::Register(TIMEOUT, &TaskCheckin);
    printf("Task2 Intialized\r\n");
}

//------------------------------------------------------------------
// HandleProcess
//------------------------------------------------------------------
void Task2::HandleProcess()
{
    static uint32_t count = 0;

    count++;

    if ((count % 100) == 0)
    {
        printf("Task2 stuff: %u\r\n", count);
    }

    TaskMonitor::TaskCheckin();
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void Task2::HandleShutdown()
{
    printf("Task2 Shutdown\r\n");
    vTaskDelay(portMAX_DELAY);
}
