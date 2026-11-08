/// @file Task1.cpp
/// @brief Example Task1 implementation (see Task1.hpp).

#include "Task1.hpp"
#include "TaskMonitor.hpp"
#include "Utils.hpp"

#include <cstdio>

namespace
{
    constexpr uint32_t MSG_Q_LENGTH = 10;                   ///< Number of messages the queue can hold
    constexpr uint32_t CHECKIN_TIMEOUT = 10 * MS_PER_SEC;   ///< Task monitor timeout (ms)
    constexpr uint32_t PROCESS_DELAY = 1000;                ///< Delay (ms) in each process step
}

QHandle Task1::msgQHandle = nullptr;

//------------------------------------------------------------------
// SendMessage
//------------------------------------------------------------------
void Task1::SendMessage(Message::MsgId id)
{
    const Message msg(id);
    const StatusType status = QSend(msgQHandle, &msg);
    ASSERT(PASS == status);
    (void)status;
}

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void Task1::TaskCheckin()
{
    SendMessage(Message::TASK_CHECKIN);
}

//------------------------------------------------------------------
// Process
//------------------------------------------------------------------
void Task1::Process()
{
    SendMessage(Message::PROCESS);
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void Task1::Shutdown()
{
    SendMessage(Message::SHUTDOWN);
}

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void Task1::Initialize()
{
    msgQHandle = CreateQ(MSG_Q_LENGTH, sizeof(Message));
    ASSERT(nullptr != msgQHandle);
    SendMessage(Message::INITIALIZE);
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void Task1::Run()
{
    StatusType status = FALSE;
    Message msg;

    while (true)
    {
        // Wait for new message to process indefinitely
        status = QRecv(msgQHandle, &msg, MAX_DELAY);

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
    // Register this task with the Task Monitor
    TaskMonitor::Register(CHECKIN_TIMEOUT, &TaskCheckin);

    printf("Task1 Initialized\r\n");
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
        printf("Task1 stuff: %lu\r\n", (unsigned long)count);
    }
    DELAY_MS(PROCESS_DELAY);
    Process();
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void Task1::HandleShutdown()
{
    printf("Task1 Shutdown\r\n");
    // wait forever
    DELAY(MAX_DELAY);
}
