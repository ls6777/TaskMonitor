
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/// @brief A sample task doing some processing for example using the TaskMonitor
class Task1
{
    public:

        /// @brief Post initialize message to this task
        void Initialize();

        /// @brief Send message for task to do processing
        void Process();

        /// @brief task for the task manager
        /// @details Main loop that waits for messages to get posted to the queue and process them
        void Run();

        /// @brief Used for the task monitor. Used to create a message for this task to respond to the task monitor
        static void TaskCheckin();

        /// @brief send msg to shutdown this task
        void Shutdown();

    private:

        /// @brief Initialize the default task
        void HandleInitialize();

        /// @brief Do some processing on this task
        void HandleProcess();

        /// @brief Shutdown this task
        void HandleShutdown();

        static QueueHandle_t msgQHandle;
};

