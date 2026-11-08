#pragma once

/// @file Task1.hpp
/// @brief Example task that responds to task monitor checkin requests.

#include "Message.hpp"
#include "TargetPort.hpp"

/// @brief Example task that processes once a second and responds to checkin requests
/// @details Registers with the task monitor with a 10 second timeout. Only checks in when the monitor asks.
class Task1
{
    public:

        /// @brief Create this task's message queue and post the initialize message to it
        /// @note Must be called before Run().
        void Initialize();

        /// @brief Post a message for this task to do processing
        static void Process();

        /// @brief Execution of Task1
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

        /// @brief Handle the initialize message: register with the task monitor and start processing
        void HandleInitialize();

        /// @brief Handle the process message: print the counter every 10 runs, delay 1 s, then post the next one
        void HandleProcess();

        /// @brief Handle the shutdown message: print a message and stop processing
        void HandleShutdown();

        static QHandle msgQHandle;  ///< This task's message queue
};
