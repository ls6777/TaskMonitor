#pragma once

/// @file CheckinTracker.hpp
/// @brief Target independent bookkeeping for the task monitor.
/// @details Tracks, for every registered task, how long it has been since it last checked in and how long the
///          monitor has been waiting for a response to a checkin request. Has no RTOS or target dependencies, so it
///          can be unit tested on a PC.

#include <cstdint>
#include <map>

/// @brief Tracks task checkins and decides when to request a checkin and when a task is in violation
/// @details The tracker doesn't read a clock. The owner calls Update() periodically with the time elapsed since the
///          previous call. For each task:
///          - Once more than its timeout has elapsed since its last checkin, the task's checkin callback is called
///            (once) to request a checkin, and the tracker starts waiting for the response.
///          - If more than its timeout then elapses without a checkin, the task is in violation and the violation
///            handler passed to Update() is called.
///          - A checkin at any time (requested or not) resets both times.
///
///          This means the worst case time to detect a stuck task is twice its timeout.
/// @tparam TaskId - type used to identify a task (e.g. an RTOS task handle). Must be usable as a std::map key.
template <typename TaskId>
class CheckinTracker
{
    public:

        /// @brief Callback used to ask a task to check in
        using CheckinCall = void (*)();

        /// @brief Monitoring state of one task
        struct Info
        {
            uint32_t timeout;                   ///< Max time (ms) to wait before requesting, and then for, a checkin
            uint32_t elapsedWaitingForResponse; ///< Time (ms) waiting for a response to the current checkin request
            uint32_t elapsedSinceLastCheckin;   ///< Time (ms) since the task last checked in
            uint32_t maxWaitingForResponse;     ///< Longest time (ms) the task took to respond to a request (statistics)
            bool waitingOnResponse;             ///< True if a checkin request was sent and no response received yet
            CheckinCall checkinCall;            ///< Called to request a checkin from the task
        };

        /// @brief Map of task ID to its monitoring state
        using TaskMap = std::map<TaskId, Info>;

        /// @brief Add a task, or reset it if it's already registered
        /// @param[in] id - task to monitor
        /// @param[in] timeout - max time (ms) to wait before requesting a checkin, and then for the response
        /// @param[in] checkinCall - called to request a checkin from the task. Must not be nullptr.
        void Register(TaskId id, uint32_t timeout, CheckinCall checkinCall)
        {
            tasks[id] = Info{timeout, 0, 0, 0, false, checkinCall};
        }

        /// @brief Record a checkin from a task
        /// @param[in] id - task checking in
        /// @return true if the task is registered, false otherwise (the checkin is ignored)
        bool Checkin(TaskId id)
        {
            const auto it = tasks.find(id);
            if (it == tasks.end())
            {
                return false;
            }

            it->second.elapsedWaitingForResponse = 0;
            it->second.elapsedSinceLastCheckin = 0;
            it->second.waitingOnResponse = false;
            return true;
        }

        /// @brief Advance time for all tasks, request checkins that are due and report violations
        /// @param[in] elapsedMs - time (ms) since the previous call
        /// @param[in] onViolation - called as onViolation(id, info) for every task that has waited longer than its
        ///                          timeout for a response. Called again on every Update() until the task checks in
        ///                          or is registered again.
        template <typename ViolationHandler>
        void Update(uint32_t elapsedMs, ViolationHandler&& onViolation)
        {
            for (auto& [id, info] : tasks)
            {
                info.elapsedSinceLastCheckin += elapsedMs;

                if (info.waitingOnResponse)
                {
                    info.elapsedWaitingForResponse += elapsedMs;

                    if (info.elapsedWaitingForResponse > info.maxWaitingForResponse)
                    {
                        info.maxWaitingForResponse = info.elapsedWaitingForResponse;
                    }

                    if (info.elapsedWaitingForResponse > info.timeout)
                    {
                        onViolation(id, static_cast<const Info&>(info));
                    }
                }
                else if (info.elapsedSinceLastCheckin > info.timeout)
                {
                    info.elapsedWaitingForResponse = 0;
                    info.waitingOnResponse = true;
                    info.checkinCall();
                }
            }
        }

        /// @brief Get the monitoring state of all registered tasks (e.g. to print statistics)
        /// @return map of task ID to its monitoring state
        const TaskMap& Tasks() const
        {
            return tasks;
        }

    private:

        TaskMap tasks;  ///< Monitoring state of every registered task
};
