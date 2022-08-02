
#include "Task1.hpp"
#include "TaskMonitor.hpp"
#include "Message.hpp"
#include "Utils.hpp"

static constexpr uint32_t TIMEOUT = 10 * MS_PER_SEC; // 10 seconds

QueueHandle_t Task1::msgQHandle;

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void Task1::TaskCheckin()
{
    Message msg(Message::TASK_CHECKIN);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Process
//------------------------------------------------------------------
void Task1::Process()
{
    Message msg(Message::PROCESS);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void Task1::Shutdown()
{
    Message msg(Message::SHUTDOWN);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void Task1::Initialize()
{
    msgQHandle = xQueueCreate(10, sizeof(Message));
    Message msg(Message::INITIALIZE);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void Task1::Run()
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

//------------------------------------------------------------------
// HandleInitialize
//------------------------------------------------------------------
void Task1::HandleInitialize()
{
    TaskMonitor::Register(TIMEOUT, &TaskCheckin);
    printf("Task1 Intialized\r\n");
    Process();
}

//------------------------------------------------------------------
// HandleProcess
//------------------------------------------------------------------
void Task1::HandleProcess()
{
    static uint32_t count = 0;

    count++;

    if ((count % 10) == 0)
    {
        printf("Task1 stuff: %u\r\n", count);
    }
    DELAY_MS(1000);
    Process();
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void Task1::HandleShutdown()
{
    printf("Task1 Shutdown\r\n");
    vTaskDelay(portMAX_DELAY);
}
