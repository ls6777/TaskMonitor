
#include "TaskMonitor.hpp"

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

static constexpr uint32_t DeviceManager_TIMEOUT = pdMS_TO_TICKS(60 * Conversion::MS_PER_SEC);
static constexpr uint32_t EcgDataCollector_TIMEOUT = pdMS_TO_TICKS(10 * Conversion::MS_PER_SEC);
static constexpr uint32_t EcgDataFlashSaver_TIMEOUT = pdMS_TO_TICKS(10 * Conversion::MS_PER_SEC);
static constexpr uint32_t WanTransferManager_TIMEOUT = pdMS_TO_TICKS(60 * Conversion::MS_PER_SEC);
static constexpr uint32_t LogManager_TIMEOUT = pdMS_TO_TICKS(10 * Conversion::MS_PER_SEC);
static constexpr uint32_t StudyManager_TIMEOUT = pdMS_TO_TICKS(60 * Conversion::MS_PER_SEC);
static constexpr uint32_t DefaultMgr_TIMEOUT = pdMS_TO_TICKS(10 * Conversion::MS_PER_SEC);
static constexpr uint32_t PushButtonMgr_TIMEOUT = pdMS_TO_TICKS(10 * Conversion::MS_PER_SEC);
static constexpr uint32_t LedManager_TIMEOUT = pdMS_TO_TICKS(10 * Conversion::MS_PER_SEC);

std::map<osThreadId_t, TaskMonitorInfo> taskMonInfo;

//---------------------------------------------------
// GetInstance
//---------------------------------------------------
TaskMonitor& TaskMonitor::GetInstance()
{
    static TaskMonitor instance;
    return instance;
}

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
void TaskMonitor::Shutdown(uint32_t seqNum)
{
    Message msg(SHUTDOWN, nullptr, seqNum);
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
    constexpr uint32_t MSG_Q_TIMEOUT = 1000; // ms
    osStatus_t status = osError;
    Message msg;

#ifdef ENABLE_TASK_MAX_CHECKIN_TIMES
    const uint16_t STATS_DELAY = 30000/MSG_Q_TIMEOUT;
    uint16_t count = 0;
#endif


    while (true)
    {
        // Allow Sleep when waiting for a new message
        SleepManager::GetInstance().AllowSleep(SleepManager::TASK_MONITOR);

        // Check for new data ready message indicating RAM buffer is full
        status = osMessageQueueGet(taskMonitorQHandle, &msg, NULL, pdMS_TO_TICKS(MSG_Q_TIMEOUT));

        // Disable sleep while processing
        SleepManager::GetInstance().DisallowSleep(SleepManager::TASK_MONITOR);

        // Check status of message Q result
        if (status != osOK && status != osErrorTimeout)
        {
            continue;
        }

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
                    HandleShutdown(msg.seqNum);
                    break;

                default:
                    LOG(LogCode::TASK_MONITOR_ERROR, "unknown msgId: ", msg.msgId);
                    break;
            }
        }

        // Check for any expired tasks and expired RTC Timers
        CheckExpirations();

#ifdef ENABLE_TASK_MAX_CHECKIN_TIMES
        ++count;

        if (count == STATS_DELAY)
        {
            count = 0;

            for (auto& [key, val] : taskMonInfo)
            {
                PRINTLN("TaskId: 0x%x, MAX time = %d ms, name: %s,", key, val.maxElapsedTime, osThreadGetName(key));
            }
        }
#endif
    }
}

#define TASK_MON_ENTRY(x, y) taskMonInfo[x] = {x, y##_TIMEOUT, 0, 0, 0, false, &y::TaskCheckin}

//---------------------------------------------------
// HandleInitialize
//---------------------------------------------------
void TaskMonitor::HandleInitialize()
{
    // Init map with task info
    TASK_MON_ENTRY(defaultTaskHandle, DefaultMgr);
    TASK_MON_ENTRY(deviceMgrTaskHandle, DeviceManager);
    TASK_MON_ENTRY(ecgCollectTaskHandle, EcgDataCollector);
    TASK_MON_ENTRY(ecgFlashSavTaskHandle, EcgDataFlashSaver);
    TASK_MON_ENTRY(wanXferMgrTaskHandle, WanTransferManager);
    TASK_MON_ENTRY(logTaskHandle, LogManager);
    TASK_MON_ENTRY(studyMgrTaskHandle, StudyManager);
    TASK_MON_ENTRY(pushButtonTaskHandle, PushButtonMgr);
    TASK_MON_ENTRY(ledMgrTaskHandle, LedManager);
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void TaskMonitor::HandleShutdown(uint32_t seqNum)
{
    DeviceManager::TaskMonitorShutdownComplete(seqNum);
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
