
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
            Message(MsgId id = INVALID, osThreadId_t tId = nullptr, uint32_t seq = 0) : msgId{id}, taskId{tId}, seqNum{seq} {}

            MsgId msgId;
            osThreadId_t taskId;
            uint32_t seqNum;
        };

        /// @brief Only need one instance of the TaskMonitor
        /// @return instance to this object
        static TaskMonitor& GetInstance();

        /// @brief Post initialize message to this task
        void Initialize();

        /// @brief task for the task monitor
        /// @details Main loop that waits for messages to get posted to the queue and process them
        void Run();

        /// @brief checkin function for other tasks to use to checkin with the task monitor
        void TaskCheckin();

        /// @brief send msg to shutdown this task
        /// @param seqNum - sequence number of this transaction
        void Shutdown(uint32_t seqNum);

        // Unused
        TaskMonitor(const TaskMonitor&) = delete;
        TaskMonitor& operator=(const TaskMonitor&) = delete;

    private:

        /// @brief constructor
        TaskMonitor() {}

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
        /// @param seqNum - sequence number for this transaction
        void HandleShutdown(uint32_t seqNum);
};

#endif // TASK_MONITOR_HPP_
