
#ifndef TASK_MONITOR_HPP_
#define TASK_MONITOR_HPP_

#include <stdint.h>
#include "cmsis_os2.h"

/// @brief Task monitor
/// @details This class is monitoring all other tasks in the system. Each task can have a different timeout period.
///
/// @note This should be the highest priority task.
/// @note Only this task should kick the watchdog.
/// @note This task needs to run more than 2x the speed of the fastest checkin period (e.g. TaskX has a 100ms timeout, then this task needs
///       to execute on a period faster than 50ms to ensure the task doesn't timeout unintentionally. 10x is a good rule of thumb to use.
/// @note All OS calls are specific to this implementation (i.e. FreeRTOS using CMSIS). To use with other RTOS, simply update OS call to
///       appropriate calls for the that specific RTOS or RTOS wrapper
class TaskMonitor
{
    public:

        /// @brief Type of messages
        enum MsgId
        {
            INVALID,            ///< INVALID
            INITIALIZE,         ///< Initialize the class/task
            TASK_CHECKIN,       ///< Checkin Message from other tasks
            SHUTDOWN,           ///< used to shutdown this task
            NUM_MSG_IDS
        };

        /// @brief Struct for messages for the Task Monitor
        struct Message
        {
            Message(MsgId id = INVALID, osThreadId_t tId = nullptr) : msgId{id}, taskId{tId}{}

            MsgId msgId;
            osThreadId_t taskId;
        };

        /// @brief Post initialize message to this task
        void Initialize();

        /// @brief task for the task monitor
        /// @details Main loop that waits for messages to get posted to the queue and process them
        void Run();

        /// @brief checkin function for other tasks to use to checkin with the task monitor
        static void TaskCheckin();

        /// @brief send msg to shutdown this task
        void Shutdown();

    private:

        /// @brief handle checkin message from other tasks
        /// @param msg - msg received
        void HandleTaskCheckin(osThreadId_t id);

        /// @brief Checks for task expirations
        void CheckExpirations();

        /// @brief Send a message to a task for a checkin response
        /// @param id - id of task to send message to
        void SendTaskCheckInMsg(osThreadId_t id);

        /// @brief Handle initialization message
        void HandleInitialize();

        /// @brief Shutdown this task
        void HandleShutdown();
};

#endif // TASK_MONITOR_HPP_
