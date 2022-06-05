#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "Message.hpp"

/// @brief Task monitor
/// @details This class is monitoring all other tasks in the system. Each task can have a different timeout period. This class keeps track
///          of when the last checkin was received, as well as, when the last time a message was sent to a given task, and how long it's been
///          waiting for a response.
///
///          The way in which this monitor handles the timings is to monitor how long it's been since the last checkin from a given task. If it's
///          been longer than the timeout assigned (e.g. 10 seconds), then a message is sent to the task requesting a checkin. At this point,
///          the monitor will wait another timeout period (e.g. 10 seconds) for the task in question to respond to the request. If the task
///          in question does not respond in the given timeout period from the sent request (e.g. 10 seconds), then the task is considered in
///          violation of alloted time and the task monitor will ASSERT OR do other actions based on the intended user implementation.
///
///          This monitor will wait 1 full timeout period, from last checkin, to send a request to a given task. Then the monitor will wait
///          another 1 full timeout period for a response. This means that the worst case time out for violation will actually be twice the
///          timeout period.  So if a timeout period of 10 seconds is a hard requirement, then the user should set the timeout period to 5 seconds
///          to guarantee that the violation will happen within the 10 seconds.  The reason for this implementation is to reduce flooding of
///          messages to tasks and give the full timeout period to respond to a message sent to a task.
///
///          The accuracy/resolution of the task monitor is also defined by it's evaluation period. If the task monitor runs every 10ms, then the
///          timeout accuracy will be +/- 10ms. If the task monitor runs every 100ms, then the timeout accuracy will be +/- 100ms. It's up to the
///          user to determine how often they need this task to evaluate based on the accuracy needs.
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

        /// @brief Struct for messages for the Task Monitor
        class TaskMonitorMessage : public Message
        {
            public:

                TaskMonitorMessage(MsgId id = INVALID, TaskHandle_t tId = nullptr, uint32_t maxTime = 0, void (*callback)() = nullptr)
                    : Message(id), taskId{tId}, maxResponseTime{maxTime}, checkinCall{callback} {}

                TaskHandle_t taskId;
                uint32_t maxResponseTime;
                void (*checkinCall)();
        };

        /// @brief Post initialize message to this task
        void Initialize();

        /// @brief task for the task monitor
        /// @details Main loop that waits for messages to get posted to the queue and process them
        void Run();

        /// @brief checkin function for other tasks to use to checkin with the task monitor
        static void TaskCheckin();

        /// @brief Register a task with the monitor
        /// @param maxResponseTime - Maximum time the task monitor will wait for a response from a task, from a checkin request
        /// @param checkinCall - function pointer to the checkin call to use when task monitor sends a message to the task
        static void Register(uint32_t maxResponseTime, void (*checkinCall)());

        /// @brief send msg to shutdown this task
        void Shutdown();

    private:

        /// @brief handle checkin message from other tasks
        /// @param msg - msg received
        void HandleTaskCheckin(TaskHandle_t id);

        /// @brief Checks for task expirations
        void CheckExpirations();

        /// @brief Send a message to a task for a checkin response
        /// @param id - id of task to send message to
        void SendTaskCheckInMsg(TaskHandle_t id);

        /// @brief Handle initialization message
        void HandleInitialize();

        /// @brief Handle the Registration of a task with the monitor
        /// @param id- thread id for the task
        /// @param maxResponseTime - Maximum time the task monitor will wait for a response from a task, from a checkin request
        /// @param checkinCall - function pointer to the checkin call to use when task monitor sends a message to the task
        void HandleRegister(TaskHandle_t id, uint32_t maxResponseTime, void (*checkinCall)());

        /// @brief Shutdown this task
        void HandleShutdown();

        static QueueHandle_t msgQHandle;
};

