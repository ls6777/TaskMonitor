/// @file Task2.cpp
/// @brief Example Task2 implementation (see Task2.hpp).

#include "Task2.hpp"
#include "TaskMonitor.hpp"
#include "Utils.hpp"

#include <cstdio>

namespace
{
    constexpr uint32_t MSG_Q_LENGTH = 10;                   ///< Number of messages the queue can hold
    constexpr uint32_t MSG_Q_TIMEOUT = 100;                 ///< Max time (ms) to wait for a message
    constexpr uint32_t CHECKIN_TIMEOUT = 1 * MS_PER_SEC;    ///< Task monitor timeout (ms)
}

QHandle Task2::msgQHandle = nullptr;

//------------------------------------------------------------------
// SendMessage
//------------------------------------------------------------------
void Task2::SendMessage(Message::MsgId id)
{
    const Message msg(id);
    const StatusType status = TargetPort::QSend(msgQHandle, &msg);
    ASSERT(PASS == status);
    (void)status;
}

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void Task2::TaskCheckin()
{
    SendMessage(Message::TASK_CHECKIN);
}

//------------------------------------------------------------------
// Process
//------------------------------------------------------------------
void Task2::Process()
{
    SendMessage(Message::PROCESS);
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void Task2::Shutdown()
{
    SendMessage(Message::SHUTDOWN);
}

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void Task2::Initialize()
{
    msgQHandle = TargetPort::CreateQ(MSG_Q_LENGTH, sizeof(Message));
    ASSERT(nullptr != msgQHandle);
    SendMessage(Message::INITIALIZE);
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void Task2::Run()
{
    StatusType status = FALSE;
    Message msg;

    while (true)
    {
        // Wait for new message to process up to MSG_Q_TIMEOUT
        status = TargetPort::QRecv(msgQHandle, &msg, MS_TO_TICKS(MSG_Q_TIMEOUT));

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
    TaskMonitor::Register(CHECKIN_TIMEOUT, &TaskCheckin);
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
        printf("Task2 stuff: %lu\r\n", (unsigned long)count);
    }

    // Unsolicited checkin
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
