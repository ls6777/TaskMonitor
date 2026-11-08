#pragma once

/// @file TaskMonitor.hpp
/// @brief Task monitor: makes sure every registered task keeps checking in, and owns the watchdog.

#include "CheckinTracker.hpp"
#include "Message.hpp"
#include "TargetPort.hpp"

/// @brief Task monitor
/// @details Monitors all other registered tasks in the system. Each task can have a different timeout period. The
///          monitor keeps track of when each task last checked in, when a checkin request was last sent to it, and
///          how long it has been waiting for the response.
///
///          If a task hasn't checked in for longer than its timeout (e.g. 10 seconds), its checkin callback is called
///          to request a checkin. The monitor then waits another timeout period for the response. If the task doesn't
///          respond in time, it's in violation and the monitor logs the task's name and stops kicking the watchdog,
///          so the device resets. This can be replaced with another action (see OnViolation() in TaskMonitor.cpp).
///
///          The worst case time to detect a violation is therefore twice the timeout. If a task must be caught
///          within 10 seconds, give it a 5 second timeout. This avoids flooding tasks with requests and gives each
///          task a full timeout period to respond.
///
///          The resolution of the timeouts is the monitor's evaluation period (MSG_Q_TIMEOUT in TaskMonitor.cpp).
///
/// @note This should be the highest priority task.
/// @note Only this task should kick the watchdog.
/// @note This task needs to evaluate more than twice as fast as the shortest task timeout. e.g. if TaskX has a
///       100 ms timeout, this task needs to run faster than every 50 ms. 10x is a good rule of thumb.
/// @note All OS calls go through TargetPort.hpp, so only that layer needs changing for another RTOS or target.
class TaskMonitor
{
    public:

        /// @brief Callback used to ask a task to check in
        using CheckinCall = CheckinTracker<TaskHandle>::CheckinCall;

        /// @brief Message posted to the task monitor's queue
        class TaskMonitorMessage : public Message
        {
            public:

                /// @brief Constructor
                /// @param[in] id - ID of this message
                /// @param[in] tId - task sending the message
                /// @param[in] maxTime - max time (ms) to wait for a checkin (REGISTER only)
                /// @param[in] callback - called to request a checkin from the task (REGISTER only)
                TaskMonitorMessage(MsgId id = INVALID, TaskHandle tId = nullptr, uint32_t maxTime = 0,
                                   CheckinCall callback = nullptr)
                    : Message(id), taskId{tId}, maxResponseTime{maxTime}, checkinCall{callback} {}

                TaskHandle taskId;          ///< Task sending the message
                uint32_t maxResponseTime;   ///< Max time (ms) to wait for a checkin (REGISTER only)
                CheckinCall checkinCall;    ///< Called to request a checkin from the task (REGISTER only)
        };

        /// @brief Create the task monitor's message queue and post the initialize message to it
        /// @note Must be called before any task calls Register() or TaskCheckin(), and before Run(). Call it before
        ///       creating the tasks, so the queue exists however the tasks get scheduled.
        static void Initialize();

        /// @brief Execution of the task monitor
        /// @details Main loop that processes messages and checks for expired tasks. Never returns.
        void Run();

        /// @brief Check in with the task monitor
        /// @details Call from the task that is checking in, either in response to a checkin request or at any time.
        ///          The calling task is identified automatically.
        static void TaskCheckin();

        /// @brief Register the calling task with the monitor
        /// @param[in] maxResponseTime - max time (ms) the monitor waits before requesting a checkin, and then for the
        ///                              response
        /// @param[in] checkinCall - called (from the task monitor's task) to request a checkin from the calling task.
        ///                          Must be quick and non-blocking, e.g. post a message to the task's queue.
        static void Register(uint32_t maxResponseTime, CheckinCall checkinCall);

        /// @brief Post a message to shut down this task
        void Shutdown();

    private:

        /// @brief Post a message to the task monitor's queue
        /// @param[in] msg - message to send
        static void SendMessage(const TaskMonitorMessage& msg);

        /// @brief Handle a checkin from a task
        /// @param[in] id - task checking in
        void HandleTaskCheckin(TaskHandle id);

        /// @brief Update the elapsed times, request checkins that are due, act on violations and kick the watchdog
        void CheckExpirations();

        /// @brief Print the longest time each task has taken to respond to a checkin request
        void PrintMaxResponseTimes();

        /// @brief Handle the initialize message: start the watchdog
        void HandleInitialize();

        /// @brief Handle the registration of a task with the monitor
        /// @param[in] id - task to monitor
        /// @param[in] maxResponseTime - max time (ms) to wait before requesting a checkin, and then for the response
        /// @param[in] checkinCall - called to request a checkin from the task
        void HandleRegister(TaskHandle id, uint32_t maxResponseTime, CheckinCall checkinCall);

        /// @brief Handle the shutdown message: print a message and stop processing
        void HandleShutdown();

        static QHandle msgQHandle;  ///< The task monitor's message queue
};
