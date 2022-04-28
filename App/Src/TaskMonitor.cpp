
#include <Task1.hpp>
#include "TaskMonitor.hpp"
#include "defaultTask.hpp"
#include "task2.hpp"
#include "task3.hpp"

#include "main.h"
#include "stdio.h"
#include "iwdg.h"
#include <map>

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

// Timeout values are from when a message is sent to the task, how long it has to respond. Messages are only sent once per time period.
static constexpr uint32_t task2Timeout = 1 * MS_PER_SEC;            // 1 second
static constexpr uint32_t task3Timeout = 100;                       // 100 ms
static constexpr uint32_t defaultTaskTimeout = 60 * MS_PER_SEC;     // 60 seconds

static std::map<osThreadId_t, TaskMonitorInfo> taskMonInfo;

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void TaskMonitor::Initialize()
{
    Message msg(INITIALIZE);
    ASSERT(osOK == osMessageQueuePut(taskMonitorQHandle, &msg, 0, 0));
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void TaskMonitor::Shutdown()
{
    Message msg(SHUTDOWN);
    ASSERT(osOK == osMessageQueuePut(taskMonitorQHandle, &msg, 0, 0));
}

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void TaskMonitor::TaskCheckin()
{
    osThreadId_t taskId = osThreadGetId();
    Message msg(TASK_CHECKIN, taskId);
    ASSERT(osOK == osMessageQueuePut(taskMonitorQHandle, &msg, 0, 0));
}

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void TaskMonitor::Register(uint32_t maxResponseTime, void (*checkinCall)())
{
    osThreadId_t taskId = osThreadGetId();
    Message msg(REGISTER, taskId, maxResponseTime, checkinCall);
    ASSERT(osOK == osMessageQueuePut(taskMonitorQHandle, &msg, 0, 0));
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void TaskMonitor::Run()
{
    constexpr uint32_t MSG_Q_TIMEOUT = 10; // ms - should be about 10x faster than fastest timeout task requirement
    osStatus_t status = osError;
    Message msg;

#ifdef ENABLE_TASK_MAX_CHECKIN_TIMES
    const uint32_t STATS_DELAY = pdMS_TO_TICKS(30 * MS_PER_SEC); // 30 seconds
    uint32_t prevTime = KERNEL_TICKS_IN_MS();
#endif

    // Task loop - never exits
    while (true)
    {
        // Wait for new message up to the Timeout period
        status = osMessageQueueGet(taskMonitorQHandle, &msg, NULL, pdMS_TO_TICKS(MSG_Q_TIMEOUT));

        // Check status of message Q result
        if (status != osOK && status != osErrorTimeout)
        {
            // just continue if the status was not OK or Timeout
            continue;
        }

        // if it wasn't a timeout, then we have a message to process
        if (status != osErrorTimeout)
        {
            // handle the message received
            switch (msg.msgId)
            {
                case TASK_CHECKIN:
                    HandleTaskCheckin(msg.taskId);
                    break;

                case INITIALIZE:
                    HandleInitialize();
                    break;

                case REGISTER:
                    HandleRegister(msg.taskId, msg.maxResponseTime, msg.checkinCall);
                    break;

                case SHUTDOWN:
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
                printf("TaskId: 0x%x, MAX time = %lu ms, name: %s\r\n", reinterpret_cast<unsigned int>(key), val.maxElapsedTimeWaitingForResponse, osThreadGetName(key));
            }
            prevTime = KERNEL_TICKS_IN_MS();
        }
#endif
    }
}

//#define TASK_MON_ENTRY(x) taskMonInfo[x##Handle] = {x##Handle, pdMS_TO_TICKS(x##Timeout), 0, 0, 0, false, &x::TaskCheckin}

//------------------------------------------------------------------
// HandleInitialize
//------------------------------------------------------------------
void TaskMonitor::HandleInitialize()
{
//    // Init map with task info
//    TASK_MON_ENTRY(defaultTask);
//    TASK_MON_ENTRY(task1);
//    TASK_MON_ENTRY(task2);
//    TASK_MON_ENTRY(task3);

    printf("Task Monitor Intialized\r\n");
}

//------------------------------------------------------------------
// HandleRegister
//------------------------------------------------------------------
void TaskMonitor::HandleRegister(osThreadId_t id, uint32_t maxResponseTime, void (*checkinCall)())
{
    taskMonInfo[id] = {pdMS_TO_TICKS(maxResponseTime), 0, 0, 0, false, checkinCall};
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void TaskMonitor::HandleShutdown()
{
    printf("Task Monitor Shutdown\r\n");
    DELAY_MS(osWaitForever);
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

        // check for active task checkin state
        if (true == monInfo.waitingOnResponse)
        {
            // add elapsed time waiting for Response
            monInfo.elapsedTimeWaitingForResponse += elapsedTime;

            // check if this has exceed the max response time
            if (monInfo.elapsedTimeWaitingForResponse > monInfo.maxElapsedTimeWaitingForResponse)
            {
                // record max time to respond - This is just for debugging/user knowledge
                monInfo.maxElapsedTimeWaitingForResponse = monInfo.elapsedTimeWaitingForResponse;
            }

            // check for expired task checkins... if there was a violation, we can ASSERT which basically disables the system until the watchdog bites
            ASSERT(monInfo.elapsedTimeWaitingForResponse < monInfo.timeout);

            // Designer can also replace the above with their own logic that can reset tasks, end tasks, or take other actions
            // based on violations. Example below:
            // if (val.elapsedTimeWaitingForResponse > val.timeout)
            // {
            //    // Do action, such as restart the task that violated the timeout
            // }

        }
        else // see if any any tasks are ready for another checkin
        {
            if (monInfo.elapsedTimeSinceLastCheckin > monInfo.timeout)
            {
                SendTaskCheckInMsg(taskId);
            }
        }
    }

#ifndef DISABLE_WATCHDOG
    // kick the watchdog
    HAL_IWDG_Refresh(&hiwdg);
#endif

    // update prevTime for next eval
    prevTime = currTime;
}

//------------------------------------------------------------------
// SendTaskCheckInMsg
//------------------------------------------------------------------
void TaskMonitor::SendTaskCheckInMsg(osThreadId_t id)
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
void TaskMonitor::HandleTaskCheckin(osThreadId_t id)
{
    ASSERT(taskMonInfo.find(id) != taskMonInfo.end());

    taskMonInfo[id].elapsedTimeWaitingForResponse = 0;
    taskMonInfo[id].elapsedTimeSinceLastCheckin = 0;
    taskMonInfo[id].waitingOnResponse = false;
}
