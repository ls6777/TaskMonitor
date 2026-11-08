#pragma once

/// @file DefaultTask.hpp
/// @brief Example task that periodically prints task and run-time statistics.

#include "Message.hpp"
#include "TargetPort.hpp"

/// @brief Example task that prints the task list and run-time statistics every 30 seconds
/// @details Registers with the task monitor with a 60 second timeout and responds to its checkin requests.
class DefaultTask
{
    public:

        /// @brief Create this task's message queue and post the initialize message to it
        /// @note Must be called before Run().
        void Initialize();

        /// @brief Execution of DefaultTask
        /// @details Main loop that waits for messages posted to the queue and processes them. Never returns.
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

        /// @brief Print the task list and run-time statistics
        void PrintStats();

        /// @brief Handle the shutdown message: print a message and stop processing
        void HandleShutdown();

        static QHandle msgQHandle;  ///< This task's message queue
};
