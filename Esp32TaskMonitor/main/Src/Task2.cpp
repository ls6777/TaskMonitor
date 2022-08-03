
#include "Task2.hpp"
#include "TaskMonitor.hpp"
#include "Message.hpp"
#include "Utils.hpp"

static constexpr uint32_t TIMEOUT = 1 * MS_PER_SEC; // 1 second

QHandle Task2::msgQHandle;

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void Task2::TaskCheckin()
{
    Message msg(Message::TASK_CHECKIN);
    ASSERT(PASS == QSend(msgQHandle, &msg));
}

//------------------------------------------------------------------
// Process
//------------------------------------------------------------------
void Task2::Process()
{
    Message msg(Message::PROCESS);
    ASSERT(PASS == QSend(msgQHandle, &msg));
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void Task2::Shutdown()
{
    Message msg(Message::SHUTDOWN);
    ASSERT(PASS == QSend(msgQHandle, &msg));
}

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void Task2::Initialize()
{
    msgQHandle = CreateQ(10, sizeof(Message));
    Message msg(Message::INITIALIZE);
    ASSERT(PASS == QSend(msgQHandle, &msg));
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void Task2::Run()
{
    constexpr uint32_t MSG_Q_TIMEOUT = 100; // ms
    StatusType status = FALSE;
    Message msg;

    while (true)
    {
        // Wait for new message to process up to MSG_Q_TIMEOUT
        status = QRecv(msgQHandle, &msg, MS_TO_TICKS(MSG_Q_TIMEOUT));

        // Check status of message Q result
        if (TRUE == status)
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
    // Register this task with the Task Monitor
    TaskMonitor::Register(TIMEOUT, &TaskCheckin);
    printf("Task2 Initialized\r\n");
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
    // wait forever
    DELAY(MAX_DELAY);
}
