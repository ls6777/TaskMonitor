#pragma once

/// @file Task2.hpp
/// @brief Example task that checks in with the task monitor without being asked.

#include "Message.hpp"
#include "TargetPort.hpp"

/// @brief Example task that processes every 100 ms and checks in each time (unsolicited checkins)
/// @details Registers with the task monitor with a 1 second timeout. Because it checks in more often than its
///          timeout, the monitor never needs to send it a checkin request.
class Task2
{
    public:

        /// @brief Create this task's message queue and post the initialize message to it
        /// @note Must be called before Run().
        void Initialize();

        /// @brief Post a message for this task to do processing
        static void Process();

        /// @brief Execution of Task2
        /// @details Main loop that waits up to 100 ms for messages and processes them. Posts a process message
        ///          whenever the wait times out. Never returns.
        void Run();

        /// @brief Request a checkin from this task (task monitor checkin callback)
        /// @details Posts a checkin message to this task's queue. The task then checks in with the task monitor.
        static void TaskCheckin();

        /// @brief Post a message to shut down this task
        void Shutdown();

    private:

        /// @brief Post a message to this task's queue
        /// @param[in] id - ID of the message to send
        static void SendMessage(Message::MsgId id);

        /// @brief Handle the initialize message: register with the task monitor
        void HandleInitialize();

        /// @brief Handle the process message: print the counter every 100 runs and check in with the task monitor
        void HandleProcess();

        /// @brief Handle the shutdown message: print a message and stop processing
        void HandleShutdown();

        static QHandle msgQHandle;  ///< This task's message queue
};
