
#include "TaskMonitor.hpp"
#include "defaultTask.hpp"
#include "task1.hpp"
#include "task2.hpp"
#include "task3.hpp"

#include "main.h"
#include "stdio.h"
#include "iwdg.h"
#include <map>

/// @brief struct to hold the Task Monitor info for a particular task
struct TaskMonitorInfo
{
    osThreadId_t taskId;
    uint32_t timeout;
    uint32_t elapsedTimeWaitingForResponse;
    uint32_t elapsedTimeSinceLastRequst;
    uint32_t maxElapsedTime;
    bool waitingOnResponse;
    void (*checkinCall)();
};

static constexpr uint32_t task1Timeout = pdMS_TO_TICKS(10 * MS_PER_SEC);           // 10 seconds
static constexpr uint32_t task2Timeout = pdMS_TO_TICKS(1 * MS_PER_SEC);            // 1 second
static constexpr uint32_t task3Timeout = pdMS_TO_TICKS(100);                                   // 100 ms
static constexpr uint32_t defaultTaskTimeout = pdMS_TO_TICKS(60 * MS_PER_SEC);     // 60 seconds

std::map<osThreadId_t, TaskMonitorInfo> taskMonInfo;

//---------------------------------------------------
// Initialize
//---------------------------------------------------
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
    Message msg(SHUTDOWN, nullptr);
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
// Run
//------------------------------------------------------------------
void TaskMonitor::Run()
{
    constexpr uint32_t MSG_Q_TIMEOUT = 10; // ms
    osStatus_t status = osError;
    Message msg;

#ifdef ENABLE_TASK_MAX_CHECKIN_TIMES
    const uint32_t STATS_DELAY = 30000/MSG_Q_TIMEOUT; // 30 seconds
    uint32_t count = 0;
#endif

    // Task loop - never exits
    while (true)
    {
        // Check for new data ready message indicating RAM buffer is full
        status = osMessageQueueGet(taskMonitorQHandle, &msg, NULL, pdMS_TO_TICKS(MSG_Q_TIMEOUT));

        // Check status of message Q result
        if (status != osOK && status != osErrorTimeout)
        {
            continue;
        }

        // handle any message received
        if (status != osErrorTimeout)
        {
            switch (msg.msgId)
            {
                case TASK_CHECKIN:
                    HandleTaskCheckin(msg.taskId);
                    break;

                case INITIALIZE:
                    HandleInitialize();
                    break;

                case SHUTDOWN:
                    HandleShutdown();
                    break;

                default:
                    printf("TaskMonitor - unknown msgId: %d\r\n", msg.msgId);
                    break;
            }
        }

        // Check for any expired tasks and expired RTC Timers
        CheckExpirations();

#ifdef ENABLE_TASK_MAX_CHECKIN_TIMES
        ++count;

        // print out max times that tasks took to respond to task monitor message
        if (count == STATS_DELAY)
        {
            count = 0;

            for (auto& [key, val] : taskMonInfo)
            {
                printf("TaskId: 0x%x, MAX time = %lu ms, name: %s\r\n", key, val.maxElapsedTime, osThreadGetName(key));
            }
        }
#endif
    }
}

#define TASK_MON_ENTRY(x) taskMonInfo[x##Handle] = {x##Handle, x##Timeout, 0, 0, 0, false, &x::TaskCheckin}

//---------------------------------------------------
// HandleInitialize
//---------------------------------------------------
void TaskMonitor::HandleInitialize()
{
    // Init map with task info
    TASK_MON_ENTRY(defaultTask);
    TASK_MON_ENTRY(task1);
    TASK_MON_ENTRY(task2);
    TASK_MON_ENTRY(task3);

    printf("Task Monitor Intialized\r\n");
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void TaskMonitor::HandleShutdown()
{
    DELAY_MS(osWaitForever);
}

//------------------------------------------------------------------
// CheckExpirations
//------------------------------------------------------------------
void TaskMonitor::CheckExpirations()
{
    static uint32_t currTime = KERNEL_TICKS_IN_MS();
    static uint32_t prevTime = currTime;

    currTime = KERNEL_TICKS_IN_MS();

    // Add time to elapsedTime
    // If no expired tasks, then kick the watchdog, otherwise something is wrong and we need to let the watchdog bite
    for (auto& [key, val] : taskMonInfo)
    {
        val.elapsedTimeSinceLastRequst += (currTime - prevTime);

        // check for active task checkin
        if (true == val.waitingOnResponse)
        {
            // if active, then add elapsed time
            val.elapsedTimeWaitingForResponse += (currTime - prevTime);

            // record max time to respond
            if (val.elapsedTimeWaitingForResponse > val.maxElapsedTime)
            {
                val.maxElapsedTime = val.elapsedTimeWaitingForResponse;
            }

            // check for expired task checkins
            ASSERT(val.elapsedTimeWaitingForResponse < val.timeout);

            // Designer can also replace the above with their own logic that can reset tasks, end tasks, or take other actions
            // based on violations. Example below:
            // if (val.elapsedTimeWaitingForResponse > val.timeout)
            // {
            //    // Do action, such as restart the task that violated the timeout
            // }

        }
        else // see if any any tasks are ready for another checkin
        {
            if (val.elapsedTimeSinceLastRequst > val.timeout)
            {
                val.elapsedTimeSinceLastRequst = 0;
                val.waitingOnResponse = true;
                SendTaskCheckInMsg(key);
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
    ASSERT(taskMonInfo[id].taskId == id);
    taskMonInfo[id].checkinCall();
}

//------------------------------------------------------------------
// HandleTaskCheckin
//------------------------------------------------------------------
void TaskMonitor::HandleTaskCheckin(osThreadId_t id)
{
    ASSERT(taskMonInfo[id].taskId == id);

    taskMonInfo[id].elapsedTimeWaitingForResponse = 0;
    taskMonInfo[id].waitingOnResponse = false;
}
