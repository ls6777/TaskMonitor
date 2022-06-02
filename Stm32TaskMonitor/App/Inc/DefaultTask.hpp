#ifndef DEFAULT_TASK_HPP_
#define DEFAULT_TASK_HPP_

/// @brief default task
class defaultTask
{
    public:

        /// @brief Type of messages for the Ecg Data Flash Saver
        enum MsgId
        {
            INVALID,        ///< INVALID
            INITIALIZE,     ///< Initialize the task manager and start all the other tasks
            TASK_CHECKIN,   ///< Checkin Message from other tasks
            SHUTDOWN,       ///< message to shutdown this task
            NUM_MSG_IDS
        };

        /// @brief Struct for messages for the default task
        struct Message
        {
            Message(MsgId id = INVALID) : msgId{id} {}

            MsgId msgId;
        };

        /// @brief Post initialize message to this task
        void Initialize();

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

        /// @brief Shutdown this task
        void HandleShutdown();
};


#endif // DEFAULT_TASK_HPP_
