
#include "TaskMonitor.hpp"
#include "Utils.hpp"

#include "esp_log.h"
#include "esp_task_wdt.h"
#include <map>

#define ENABLE_TASK_MAX_CHECKIN_TIMES

/// @brief struct to hold the Task Monitor info for a particular task
struct TaskMonitorInfo
{
    uint32_t timeout;
    uint32_t elapsedTimeWaitingForResponse;
    uint32_t elapsedTimeSinceLastCheckin;
    uint32_t maxElapsedTimeWaitingForResponse;
    bool waitingOnResponse;
    void (*checkinCall)();
};

// Timeout values are from when a message is sent to the task, how long it has to respond. Messages are only sent once per time period.
//static constexpr uint32_t task2Timeout = 1 * MS_PER_SEC;            // 1 second
//static constexpr uint32_t task3Timeout = 100;                       // 100 ms
//static constexpr uint32_t defaultTaskTimeout = 60 * MS_PER_SEC;     // 60 seconds

static std::map<TaskHandle_t, TaskMonitorInfo> taskMonInfo;

QueueHandle_t TaskMonitor::msgQHandle;

//------------------------------------------------------------------
// Initialize
//------------------------------------------------------------------
void TaskMonitor::Initialize()
{
    msgQHandle = xQueueCreate(10, sizeof(TaskMonitorMessage));
    TaskMonitorMessage msg(Message::INITIALIZE);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Shutdown
//------------------------------------------------------------------
void TaskMonitor::Shutdown()
{
    TaskMonitorMessage msg(Message::SHUTDOWN);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void TaskMonitor::TaskCheckin()
{
    TaskHandle_t taskId = xTaskGetCurrentTaskHandle();
    TaskMonitorMessage msg(Message::TASK_CHECKIN, taskId);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// TaskCheckin
//------------------------------------------------------------------
void TaskMonitor::Register(uint32_t maxResponseTime, void (*checkinCall)())
{
    TaskHandle_t taskId = xTaskGetCurrentTaskHandle();
    TaskMonitorMessage msg(Message::REGISTER, taskId, maxResponseTime, checkinCall);
    assert(pdPASS == xQueueSend(msgQHandle, &msg, 0));
}

//------------------------------------------------------------------
// Run
//------------------------------------------------------------------
void TaskMonitor::Run()
{
    constexpr uint32_t MSG_Q_TIMEOUT = 10; // ms - should be about 10x faster than fastest timeout task requirement
    BaseType_t status = pdFALSE;
    TaskMonitorMessage msg;

#ifdef ENABLE_TASK_MAX_CHECKIN_TIMES
    const uint32_t STATS_DELAY = 30 * MS_PER_SEC; // 30 seconds
    uint32_t prevTime = KERNEL_TICKS_IN_MS();
#endif

    // Task loop - never exits
    while (true)
    {
        // Wait for new message up to the Timeout period
        status = xQueueReceive(msgQHandle, &msg, pdMS_TO_TICKS(MSG_Q_TIMEOUT));

        // if it wasn't a timeout, then we have a message to process
        if (pdTRUE == status)
        {
            // handle the message received
            switch (msg.msgId)
            {
                case Message::TASK_CHECKIN:
                    HandleTaskCheckin(msg.taskId);
                    break;

                case Message::INITIALIZE:
                    HandleInitialize();
                    break;

                case Message::REGISTER:
                    HandleRegister(msg.taskId, msg.maxResponseTime, msg.checkinCall);
                    break;

                case Message::SHUTDOWN:
                    HandleShutdown();
                    break;

                default:
                    printf("TaskMonitor - unknown msgId: %d\r\n", msg.msgId);
                    break;
            }
        }

        // Check for any expired tasks
        CheckExpirations();

#ifdef ENABLE_TASK_MAX_CHECKIN_TIMES

        // print out max times that tasks took to respond to task monitor message
        if (KERNEL_TICKS_IN_MS() - prevTime > STATS_DELAY)
        {
            for (auto& [key, val] : taskMonInfo)
            {
                printf("TaskId: 0x%x, MAX time = %u ms, name: %s\r\n", reinterpret_cast<unsigned int>(key), val.maxElapsedTimeWaitingForResponse, pcTaskGetName(key));
            }
            prevTime = KERNEL_TICKS_IN_MS();
        }
#endif
    }
}

//------------------------------------------------------------------
// HandleInitialize
//------------------------------------------------------------------
void TaskMonitor::HandleInitialize()
{
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    printf("Task Monitor Intialized\r\n");
}

//------------------------------------------------------------------
// HandleRegister
//------------------------------------------------------------------
void TaskMonitor::HandleRegister(TaskHandle_t id, uint32_t maxResponseTime, void (*checkinCall)())
{
    taskMonInfo[id] = {maxResponseTime, 0, 0, 0, false, checkinCall};
}

//------------------------------------------------------------------
// HandleShutdown
//------------------------------------------------------------------
void TaskMonitor::HandleShutdown()
{
    printf("Task Monitor Shutdown\r\n");
    vTaskDelay(portMAX_DELAY);
}

//------------------------------------------------------------------
// CheckExpirations
//------------------------------------------------------------------
void TaskMonitor::CheckExpirations()
{
    static uint32_t prevTime = KERNEL_TICKS_IN_MS();

    uint32_t currTime = KERNEL_TICKS_IN_MS();
    uint32_t elapsedTime = currTime - prevTime;

    // Go through all the tasks and check for status
    for (auto& [taskId, monInfo] : taskMonInfo)
    {
        // add to elapsed time since last checkin
        monInfo.elapsedTimeSinceLastCheckin += elapsedTime;

        // check for active task checkin state
        if (true == monInfo.waitingOnResponse)
        {
            // add elapsed time waiting for Response
            monInfo.elapsedTimeWaitingForResponse += elapsedTime;

            // check if this has exceed the max response time
            if (monInfo.elapsedTimeWaitingForResponse > monInfo.maxElapsedTimeWaitingForResponse)
            {
                // record max time to respond - This is just for debugging/user knowledge
                monInfo.maxElapsedTimeWaitingForResponse = monInfo.elapsedTimeWaitingForResponse;
            }

            // check for expired task checkins... if there was a violation, we can print the name of the violator and then wait until the watchdog bites
            if (monInfo.elapsedTimeWaitingForResponse > monInfo.timeout)
            {
                ESP_LOGE("TaskMonitor", "Task: %s - Exceeded Checkin Time. Elapsed time %u ms, Timeout %u ms",
                        pcTaskGetName(taskId), monInfo.elapsedTimeWaitingForResponse, monInfo.timeout);

                while(1) {}
            }

            // Designer can also replace the above with their own logic that can reset tasks, end tasks, or take other actions
            // based on violations. Example below:
            // if (val.elapsedTimeWaitingForResponse > val.timeout)
            // {
            //    // Do action, such as restart the task that violated the timeout
            // }

        }
        else // see if any any tasks are ready for another checkin
        {
            if (monInfo.elapsedTimeSinceLastCheckin > monInfo.timeout)
            {
                SendTaskCheckInMsg(taskId);
            }
        }
    }

    // kick the watchdog - replace with watchdog reset for your specific target
    esp_task_wdt_reset();

    // update prevTime for next eval
    prevTime = currTime;
}

//------------------------------------------------------------------
// SendTaskCheckInMsg
//------------------------------------------------------------------
void TaskMonitor::SendTaskCheckInMsg(TaskHandle_t id)
{
    assert(taskMonInfo.find(id) != taskMonInfo.end());
    assert(taskMonInfo[id].checkinCall != nullptr);

    taskMonInfo[id].checkinCall();
    taskMonInfo[id].elapsedTimeWaitingForResponse = 0;
    taskMonInfo[id].waitingOnResponse = true;
}

//------------------------------------------------------------------
// HandleTaskCheckin
//------------------------------------------------------------------
void TaskMonitor::HandleTaskCheckin(TaskHandle_t id)
{
    assert(taskMonInfo.find(id) != taskMonInfo.end());

    taskMonInfo[id].elapsedTimeWaitingForResponse = 0;
    taskMonInfo[id].elapsedTimeSinceLastCheckin = 0;
    taskMonInfo[id].waitingOnResponse = false;
}
