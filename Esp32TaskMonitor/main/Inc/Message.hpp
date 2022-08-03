#pragma once

/// @brief Message class
class Message
{
    public:

        /// @brief Type of messages
        enum MsgId
        {
            INVALID,        ///< INVALID
            INITIALIZE,     ///< Initialize the task
            TASK_CHECKIN,   ///< Task Monitor Checkin Message
            REGISTER,       ///< Register
            PROCESS,        ///< Do some processing
            SHUTDOWN,       ///< shutdown message
            NUM_MSG_IDS
        };

        /// @brief constructor
        /// @param id - ID of this message
        Message(MsgId id = INVALID) : msgId{id} {}

        MsgId msgId;
};
