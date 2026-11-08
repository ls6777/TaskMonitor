/// @file DefaultTask.cpp
/// @brief Example DefaultTask implementation (see DefaultTask.hpp).

#include "DefaultTask.hpp"
#include "TaskMonitor.hpp"
#include "Utils.hpp"

#include <cstdio>

namespace
{
    constexpr uint32_t MSG_Q_LENGTH = 10;                   ///< Number of messages the queue can hold
    constexpr uint32_t MSG_Q_TIMEOUT = 1000;                ///< Max time (ms) to wait for a message
    constexpr uint32_t STATS_PERIOD = 30 * MS_PER_SEC;      ///< Period (ms) for printing the statistics
    constexpr uint32_t CHECKIN_TIMEOUT = 60 * MS_PER_SEC;   ///< Task monitor timeout (ms)

    char taskListBuf[1024];     ///< Output of TargetPort::GetTaskList()
    char taskStatsBuf[1024];    ///< Output of TargetPort::GetRunTimeStats()
}

QHandle DefaultTask::msgQHandle = nullptr;

//------------------------------------------------------------------
// SendMessage
//------------------------------------------------------------------
void DefaultTask::SendMessage(Message::MsgId id)
{
    const Message msg(id);
    const StatusType status = TargetPort::QSend(msgQHandle, &msg);
    ASSERT(PASS == status);
    (void)status;
}

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void DefaultTask::TaskCheckin()
{
    SendMessage(Message::TASK_CHECKIN);
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void DefaultTask::Shutdown()
{
    SendMessage(Message::SHUTDOWN);
}

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void DefaultTask::Initialize()
{
    msgQHandle = TargetPort::CreateQ(MSG_Q_LENGTH, sizeof(Message));
    ASSERT(nullptr != msgQHandle);
    SendMessage(Message::INITIALIZE);
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void DefaultTask::Run()
{
    StatusType status = FALSE;
    Message msg;

    uint32_t prevTime = TargetPort::GetTimeMs();

    while (true)
    {
        // wait for message up to MSG_Q_TIMEOUT
        status = TargetPort::QRecv(msgQHandle, &msg, MS_TO_TICKS(MSG_Q_TIMEOUT));

        if (TRUE == status)
        {
            switch (msg.msgId)
            {
                case Message::TASK_CHECKIN:
                    TaskMonitor::TaskCheckin();
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

        if ((TargetPort::GetTimeMs() - prevTime) > STATS_PERIOD)
        {
            prevTime = TargetPort::GetTimeMs();
            PrintStats();
        }
    }
}

//------------------------------------------------------------------
// PrintStats
//------------------------------------------------------------------
void DefaultTask::PrintStats()
{
    TargetPort::GetTaskList(taskListBuf, sizeof(taskListBuf));
    printf("\r\nTASK INFO**********************************************\r\n");
    printf("Name          State  Priority   Stack   Num    Core\r\n");
    printf("*******************************************************\r\n");
    printf("%s\r\n", taskListBuf);
    printf("\r\n");

    TargetPort::GetRunTimeStats(taskStatsBuf, sizeof(taskStatsBuf));
    printf("TASK STATS INFO****************************************\r\n");
    printf("Name             Abs Time       %% Time\r\n");
    printf("*******************************************\r\n");
    printf("%s\r\n", taskStatsBuf);
    printf("\r\n");
}

//---------------------------------------------------
// HandleInitialize
//---------------------------------------------------
void DefaultTask::HandleInitialize()
{
    // Register this task with the Task Monitor
    TaskMonitor::Register(CHECKIN_TIMEOUT, &TaskCheckin);
    printf("Default Task Initialized\r\n");
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void DefaultTask::HandleShutdown()
{
    printf("Default Task Shutdown\r\n");
    // wait forever
    DELAY(MAX_DELAY);
}
