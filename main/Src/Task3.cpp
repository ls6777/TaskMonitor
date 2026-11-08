/// @file Task3.cpp
/// @brief Example Task3 implementation (see Task3.hpp).

#include "Task3.hpp"
#include "TaskMonitor.hpp"
#include "Utils.hpp"

#include <cstdio>

namespace
{
    constexpr uint32_t MSG_Q_LENGTH = 10;       ///< Number of messages the queue can hold
    constexpr uint32_t CHECKIN_TIMEOUT = 100;   ///< Task monitor timeout (ms)
    constexpr uint32_t PROCESS_DELAY = 10;      ///< Delay (ms) in each process step
}

QHandle Task3::msgQHandle = nullptr;

//------------------------------------------------------------------
// SendMessage
//------------------------------------------------------------------
void Task3::SendMessage(Message::MsgId id)
{
    const Message msg(id);
    const StatusType status = QSend(msgQHandle, &msg);
    ASSERT(PASS == status);
    (void)status;
}

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void Task3::TaskCheckin()
{
    SendMessage(Message::TASK_CHECKIN);
}

//------------------------------------------------------------------
// Process
//------------------------------------------------------------------
void Task3::Process()
{
    SendMessage(Message::PROCESS);
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void Task3::Shutdown()
{
    SendMessage(Message::SHUTDOWN);
}

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void Task3::Initialize()
{
    msgQHandle = CreateQ(MSG_Q_LENGTH, sizeof(Message));
    ASSERT(nullptr != msgQHandle);
    SendMessage(Message::INITIALIZE);
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void Task3::Run()
{
    StatusType status = FALSE;
    Message msg;

    while (true)
    {
        // Wait for new message to process
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
                    printf("task3 - unknown msgId: %d\r\n", msg.msgId);
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
    // Register this task with the Task Monitor
    TaskMonitor::Register(CHECKIN_TIMEOUT, &TaskCheckin);

    printf("Task3 Initialized\r\n");
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
        printf("Task3 stuff: %lu\r\n", (unsigned long)count);
    }

    DELAY_MS(PROCESS_DELAY);
    Process();
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void Task3::HandleShutdown()
{
    printf("Task3 Shutdown\r\n");
    // wait forever
    DELAY(MAX_DELAY);
}
