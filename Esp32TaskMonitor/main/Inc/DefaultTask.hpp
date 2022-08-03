#pragma once

#include "TargetPort.hpp"

/// @brief default task
class DefaultTask
{
    public:

        /// @brief Post initialize message to this task
        void Initialize();

        /// @brief task for the default task
        /// @details Main loop that waits for messages to get posted to the queue and process them
        void Run();

        /// @brief Used for the task monitor. Used to create a message for this task to respond to the task monitor
        static void TaskCheckin();

        /// @brief send msg to shutdown this task
        void Shutdown();

    private:

        /// @brief Initialize the default task
        void HandleInitialize();

        /// @brief Shutdown this task
        void HandleShutdown();

        static QHandle msgQHandle;
};
