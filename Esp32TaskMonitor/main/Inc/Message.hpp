#pragma once

/// @brief Message class
class Message
{
    public:

        /// @brief Type of messages for the Ecg Data Flash Saver
        enum MsgId
        {
            INVALID,        ///< INVALID
            INITIALIZE,     ///< Initialize the task
            TASK_CHECKIN,   ///< Checkin Message
            SHUTDOWN,       ///< message to shutdown this task
            NUM_MSG_IDS
        };

        /// @brief constructor
        /// @param id - ID of this message
        Message(MsgId id = INVALID) : msgId{id} {}

        MsgId msgId;
};
