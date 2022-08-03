
#include "TaskMonitor.hpp"
#include "Utils.hpp"

#include <map>

#define ENABLE_TASK_MAX_CHECKIN_TIMES

/// @brief struct to hold the Task Monitor info for a particular task
struct TaskMonitorInfo
{
    uint32_t timeout;
    uint32_t elapsedTimeWaitingForResponse;
    uint32_t elapsedTimeSinceLastCheckin;
    uint32_t maxElapsedTimeWaitingForResponse;
    bool waitingOnResponse;
    void (*checkinCall)();
};

// map to hold monitor info for the tasks
static std::map<TaskHandle, TaskMonitorInfo> taskMonInfo;

QHandle TaskMonitor::msgQHandle;

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void TaskMonitor::Initialize()
{
    msgQHandle = CreateQ(10, sizeof(TaskMonitorMessage));
    TaskMonitorMessage msg(Message::INITIALIZE);
    ASSERT(PASS == QSend(msgQHandle, &msg));
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void TaskMonitor::Shutdown()
{
    TaskMonitorMessage msg(Message::SHUTDOWN);
    ASSERT(PASS == QSend(msgQHandle, &msg));
}

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void TaskMonitor::TaskCheckin()
{
    TaskHandle taskId = GetCurrTaskHandle();
    TaskMonitorMessage msg(Message::TASK_CHECKIN, taskId);
    ASSERT(PASS == QSend(msgQHandle, &msg));
}

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void TaskMonitor::Register(uint32_t maxResponseTime, void (*checkinCall)())
{
    TaskHandle taskId = GetCurrTaskHandle();
    TaskMonitorMessage msg(Message::REGISTER, taskId, maxResponseTime, checkinCall);
    ASSERT(PASS == QSend(msgQHandle, &msg));
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void TaskMonitor::Run()
{
    constexpr uint32_t MSG_Q_TIMEOUT = 10; // ms - should be about 10x faster than fastest timeout task requirement
    StatusType status = FALSE;
    TaskMonitorMessage msg;

#ifdef ENABLE_TASK_MAX_CHECKIN_TIMES
    const uint32_t STATS_DELAY = 30 * MS_PER_SEC; // 30 seconds
    uint32_t prevTime = KERNEL_TICKS_IN_MS();
#endif

    // Task loop - never exits
    while (true)
    {
        // Wait for new message up to the Timeout period
        status = QRecv(msgQHandle, &msg, MS_TO_TICKS(MSG_Q_TIMEOUT));

        // if it wasn't a timeout, then we have a message to process
        if (TRUE == status)
        {
            // handle the message received
            switch (msg.msgId)
            {
                case Message::TASK_CHECKIN:
                    HandleTaskCheckin(msg.taskId);
                    break;

                case Message::INITIALIZE:
                    HandleInitialize();
                    break;

                case Message::REGISTER:
                    HandleRegister(msg.taskId, msg.maxResponseTime, msg.checkinCall);
                    break;

                case Message::SHUTDOWN:
                    HandleShutdown();
                    break;

                default:
                    printf("TaskMonitor - unknown msgId: %d\r\n", msg.msgId);
                    break;
            }
        }

        // Check for any expired tasks
        CheckExpirations();

#ifdef ENABLE_TASK_MAX_CHECKIN_TIMES

        // print out max times that tasks took to respond to task monitor message
        if (KERNEL_TICKS_IN_MS() - prevTime > STATS_DELAY)
        {
            for (auto& [key, val] : taskMonInfo)
            {
                printf("TaskId: 0x%x, MAX time = %u ms, name: %s\r\n", reinterpret_cast<unsigned int>(key), val.maxElapsedTimeWaitingForResponse, pcTaskGetName(key));
            }
            prevTime = KERNEL_TICKS_IN_MS();
        }
#endif
    }
}

//------------------------------------------------------------------
// HandleInitialize
//------------------------------------------------------------------
void TaskMonitor::HandleInitialize()
{
    // Start watchdog
    StartWatchdog();

    printf("Task Monitor Intialized\r\n");
}

//------------------------------------------------------------------
// HandleRegister
//------------------------------------------------------------------
void TaskMonitor::HandleRegister(TaskHandle id, uint32_t maxResponseTime, void (*checkinCall)())
{
    // add the given task to the monitor map
    taskMonInfo[id] = {maxResponseTime, 0, 0, 0, false, checkinCall};
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void TaskMonitor::HandleShutdown()
{
    printf("Task Monitor Shutdown\r\n");

    // wait forever
    DELAY(MAX_DELAY);
}

//------------------------------------------------------------------
// CheckExpirations
//------------------------------------------------------------------
void TaskMonitor::CheckExpirations()
{
    static uint32_t prevTime = KERNEL_TICKS_IN_MS();

    uint32_t currTime = KERNEL_TICKS_IN_MS();
    uint32_t elapsedTime = currTime - prevTime;

    // Go through all the tasks and check for status
    for (auto& [taskId, monInfo] : taskMonInfo)
    {
        // add to elapsed time since last checkin
        monInfo.elapsedTimeSinceLastCheckin += elapsedTime;

        // check if we're currently waiting for a response from this task
        if (true == monInfo.waitingOnResponse)
        {
            // add elapsed time waiting for Response
            monInfo.elapsedTimeWaitingForResponse += elapsedTime;

            // check if this task has exceeded the max response time
            if (monInfo.elapsedTimeWaitingForResponse > monInfo.maxElapsedTimeWaitingForResponse)
            {
                // record max time to respond - This is just for debugging/user knowledge
                monInfo.maxElapsedTimeWaitingForResponse = monInfo.elapsedTimeWaitingForResponse;
            }

            // check for expired task checkins... if there was a violation, we can print the name of the violator and then wait until the watchdog bites
            if (monInfo.elapsedTimeWaitingForResponse > monInfo.timeout)
            {
                LOG_ERROR("TaskMonitor", "Task: %s - Exceeded Checkin Time. Elapsed time %u ms, Timeout %u ms",
                        GetTaskName(taskId), monInfo.elapsedTimeWaitingForResponse, monInfo.timeout);

                while(1) {}
            }

            // Designer can also replace the above with their own logic that can reset tasks, end tasks, or take other actions
            // based on violations. Example below:
            // if (monInfo.elapsedTimeWaitingForResponse > monInfo.timeout)
            // {
            //    // Do action, such as restart the task that violated the timeout
            // }
        }
        else // see if any any tasks are ready for another checkin request
        {
            if (monInfo.elapsedTimeSinceLastCheckin > monInfo.timeout)
            {
                SendTaskCheckInMsg(taskId);
            }
        }
    }

    // kick the watchdog
    WatchdogReset();

    // update prevTime for next eval
    prevTime = currTime;
}

//------------------------------------------------------------------
// SendTaskCheckInMsg
//------------------------------------------------------------------
void TaskMonitor::SendTaskCheckInMsg(TaskHandle id)
{
    ASSERT(taskMonInfo.find(id) != taskMonInfo.end());
    ASSERT(taskMonInfo[id].checkinCall != nullptr);

    taskMonInfo[id].checkinCall();
    taskMonInfo[id].elapsedTimeWaitingForResponse = 0;
    taskMonInfo[id].waitingOnResponse = true;
}

//------------------------------------------------------------------
// HandleTaskCheckin
//------------------------------------------------------------------
void TaskMonitor::HandleTaskCheckin(TaskHandle id)
{
    ASSERT(taskMonInfo.find(id) != taskMonInfo.end());

    taskMonInfo[id].elapsedTimeWaitingForResponse = 0;
    taskMonInfo[id].elapsedTimeSinceLastCheckin = 0;
    taskMonInfo[id].waitingOnResponse = false;
}
