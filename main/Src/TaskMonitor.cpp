/// @file TaskMonitor.cpp
/// @brief Task monitor implementation (see TaskMonitor.hpp).

#include "TaskMonitor.hpp"
#include "Utils.hpp"

#include <cstdio>

/// @brief Define to print the longest time each task took to respond to a checkin request, every STATS_PERIOD
#define ENABLE_TASK_MAX_CHECKIN_TIMES

namespace
{
    /// @brief Max time (ms) to wait for a message. This is the evaluation period, so the resolution of the timeouts.
    /// @details Should be about 10x shorter than the shortest task timeout.
    constexpr uint32_t MSG_Q_TIMEOUT = 10;

    /// @brief Number of messages the task monitor's queue can hold
    constexpr uint32_t MSG_Q_LENGTH = 10;

    /// @brief Period (ms) for printing the max checkin response times
    constexpr uint32_t STATS_PERIOD = 30 * MS_PER_SEC;

    /// @brief Monitoring state of all registered tasks
    CheckinTracker<TaskHandle> tracker;

    /// @brief Set once a task has violated its timeout. CheckExpirations() then stops kicking the watchdog, so the
    ///        watchdog resets the device.
    bool violationDetected = false;

    //------------------------------------------------------------------
    // OnViolation
    //------------------------------------------------------------------
    /// @brief Action taken when a task doesn't respond to a checkin request in time
    /// @details Called by CheckinTracker::Update() on every evaluation while a task is in violation. Logs the first
    ///          violation and sets violationDetected, so CheckExpirations() stops kicking the watchdog and the
    ///          watchdog resets the device. The task monitor keeps processing its queue until then, so tasks that
    ///          check in often don't fill it.
    ///          Replace with your own action if needed, e.g. restart the task that violated its timeout.
    /// @param[in] taskId - task in violation
    /// @param[in] info - monitoring state of the task
    void OnViolation(TaskHandle taskId, const CheckinTracker<TaskHandle>::Info& info)
    {
        if (!violationDetected)
        {
            LOG_ERROR("TaskMonitor", "Task: %s - Exceeded Checkin Time. Elapsed time %lu ms, Timeout %lu ms",
                      TargetPort::GetTaskName(taskId), (unsigned long)info.elapsedWaitingForResponse,
                      (unsigned long)info.timeout);
            violationDetected = true;
        }
    }
}

QHandle TaskMonitor::msgQHandle = nullptr;

//------------------------------------------------------------------
// SendMessage
//------------------------------------------------------------------
void TaskMonitor::SendMessage(const TaskMonitorMessage& msg)
{
    ASSERT(nullptr != msgQHandle);  // Initialize() not called yet
    const StatusType status = TargetPort::QSend(msgQHandle, &msg);
    ASSERT(PASS == status);
    (void)status;
}

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void TaskMonitor::Initialize()
{
    msgQHandle = TargetPort::CreateQ(MSG_Q_LENGTH, sizeof(TaskMonitorMessage));
    ASSERT(nullptr != msgQHandle);
    SendMessage(TaskMonitorMessage(Message::INITIALIZE));
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void TaskMonitor::Shutdown()
{
    SendMessage(TaskMonitorMessage(Message::SHUTDOWN));
}

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void TaskMonitor::TaskCheckin()
{
    SendMessage(TaskMonitorMessage(Message::TASK_CHECKIN, TargetPort::GetCurrTaskHandle()));
}

//------------------------------------------------------------------
// Register
//------------------------------------------------------------------
void TaskMonitor::Register(uint32_t maxResponseTime, CheckinCall checkinCall)
{
    ASSERT(nullptr != checkinCall);
    SendMessage(TaskMonitorMessage(Message::REGISTER, TargetPort::GetCurrTaskHandle(), maxResponseTime, checkinCall));
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void TaskMonitor::Run()
{
    StatusType status = FALSE;
    TaskMonitorMessage msg;

#ifdef ENABLE_TASK_MAX_CHECKIN_TIMES
    uint32_t prevStatsTime = TargetPort::GetTimeMs();
#endif

    // Task loop - never exits
    while (true)
    {
        // Wait for new message up to the evaluation period
        status = TargetPort::QRecv(msgQHandle, &msg, MS_TO_TICKS(MSG_Q_TIMEOUT));

        // if it wasn't a timeout, then we have a message to process
        if (TRUE == status)
        {
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
        if ((TargetPort::GetTimeMs() - prevStatsTime) > STATS_PERIOD)
        {
            PrintMaxResponseTimes();
            prevStatsTime = TargetPort::GetTimeMs();
        }
#endif
    }
}

//------------------------------------------------------------------
// PrintMaxResponseTimes
//------------------------------------------------------------------
void TaskMonitor::PrintMaxResponseTimes()
{
    for (const auto& [taskId, info] : tracker.Tasks())
    {
        printf("TaskId: %p, MAX time = %lu ms, name: %s\r\n",
               static_cast<void*>(taskId), (unsigned long)info.maxWaitingForResponse, TargetPort::GetTaskName(taskId));
    }
}

//------------------------------------------------------------------
// HandleInitialize
//------------------------------------------------------------------
void TaskMonitor::HandleInitialize()
{
    TargetPort::StartWatchdog();
    printf("Task Monitor Initialized\r\n");
}

//------------------------------------------------------------------
// HandleRegister
//------------------------------------------------------------------
void TaskMonitor::HandleRegister(TaskHandle id, uint32_t maxResponseTime, CheckinCall checkinCall)
{
    tracker.Register(id, maxResponseTime, checkinCall);
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
    static uint32_t prevTime = TargetPort::GetTimeMs();

    const uint32_t currTime = TargetPort::GetTimeMs();

    tracker.Update(currTime - prevTime, OnViolation);

    // After a violation, let the watchdog reset the device
    if (!violationDetected)
    {
        TargetPort::WatchdogReset();
    }

    prevTime = currTime;
}

//------------------------------------------------------------------
// HandleTaskCheckin
//------------------------------------------------------------------
void TaskMonitor::HandleTaskCheckin(TaskHandle id)
{
    const bool registered = tracker.Checkin(id);
    ASSERT(registered);     // a task must Register() before checking in
    (void)registered;
}
