
#include "Task3.hpp"
#include "TaskMonitor.hpp"
#include "Message.hpp"
#include "Utils.hpp"

static constexpr uint32_t TIMEOUT = 100; // ms

QueueHandle_t Task3::msgQHandle;

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void Task3::TaskCheckin()
{
    Message msg(Message::TASK_CHECKIN);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Process
//------------------------------------------------------------------
void Task3::Process()
{
    Message msg(Message::PROCESS);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void Task3::Shutdown()
{
    Message msg(Message::SHUTDOWN);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void Task3::Initialize()
{
    msgQHandle = xQueueCreate(10, sizeof(Message));
    Message msg(Message::INITIALIZE);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void Task3::Run()
{
    BaseType_t status = pdFALSE;
    Message msg;

    while (true)
    {
        // Wait for new message to process
        status = xQueueReceive(msgQHandle, &msg, portMAX_DELAY);

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
                    printf("task1 - unknown msgId: %d\r\n", msg.msgId);
                    break;
            }
        }
    }
}

//---------------------------------------------------
// HandleInitialize
//---------------------------------------------------
void Task3::HandleInitialize()
{
    TaskMonitor::Register(TIMEOUT, &TaskCheckin);
    printf("Task3 Intialized\r\n");
    Process();
}

//------------------------------------------------------------------
// HandleProcess
//------------------------------------------------------------------
void Task3::HandleProcess()
{
    static uint32_t count = 0;

    count++;

    if ((count % 1000) == 0)
    {
        printf("Task3 stuff: %u\r\n", count);
    }

    DELAY_MS(10);
    Process();
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void Task3::HandleShutdown()
{
    printf("Task3 Shutdown\r\n");
    vTaskDelay(portMAX_DELAY);
}
