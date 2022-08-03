#include "DefaultTask.hpp"
#include "Message.hpp"
#include "TaskMonitor.hpp"
#include "Utils.hpp"

// buffers for task stats
static char taskListBuf[500];
static char taskStatsBuf[500];

QHandle DefaultTask::msgQHandle;

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void DefaultTask::TaskCheckin()
{
    Message msg(Message::TASK_CHECKIN);
    ASSERT(PASS == QSend(msgQHandle, &msg));
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void DefaultTask::Shutdown()
{
    Message msg(Message::SHUTDOWN);
    ASSERT(PASS == QSend(msgQHandle, &msg));
}

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void DefaultTask::Initialize()
{
    msgQHandle = CreateQ(10, sizeof(Message));
    Message msg(Message::INITIALIZE);
    ASSERT(PASS == QSend(msgQHandle, &msg));
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void DefaultTask::Run()
{
    constexpr uint32_t MSG_Q_TIMEOUT = 1000; // ms
    StatusType status = FALSE;
    Message msg;

    const uint32_t STATS_DELAY = 30 * MS_PER_SEC; // ms

    uint32_t prevTime = KERNEL_TICKS_IN_MS();

    while (true)
    {
        // wait for message up to MSG_Q_TIMEOUT
        status = QRecv(msgQHandle, &msg, MS_TO_TICKS(MSG_Q_TIMEOUT));

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

        uint32_t currTime = KERNEL_TICKS_IN_MS();
        if ((currTime - prevTime) > STATS_DELAY)
        {
            prevTime = KERNEL_TICKS_IN_MS();

            vTaskList(taskListBuf);
            printf("\r\nTASK INFO**********************************************\r\n");
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
    static constexpr uint32_t TASK_TIMEOUT = 60 * MS_PER_SEC;

    // Register this task with the Task Monitor
    TaskMonitor::Register(TASK_TIMEOUT, &TaskCheckin);
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
