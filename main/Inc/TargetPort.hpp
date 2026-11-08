#pragma once

/// @file TargetPort.hpp
/// @brief OS and target abstraction layer.
/// @details Everything the task monitor and the example tasks need from the RTOS and the target: types, constants
///          and macros below, and the TargetPort class. Porting to a different RTOS or target only means changing
///          this file and TargetPort.cpp. This implementation is for FreeRTOS on ESP-IDF.

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include <cassert>
#include <cstddef>
#include <cstdint>

#define PASS pdPASS     ///< Status returned when an operation succeeds
#define FAIL pdFAIL     ///< Status returned when an operation fails

#define TRUE pdTRUE     ///< Boolean true, as used by the OS
#define FALSE pdFALSE   ///< Boolean false, as used by the OS

#define MAX_DELAY portMAX_DELAY     ///< Wait forever (when used as a timeout, in OS ticks)

/// @brief Assert that a condition is true
/// @param x - condition that must be true
/// @note Asserts are intended to stay enabled, even in production builds. They can be disabled through the
///       project configuration (which defines NDEBUG), so never put required code inside them.
#define ASSERT(x) assert(x)

/// @brief Log an error message
/// @param tag - tag identifying the source of the message
/// @param format - printf style format string, followed by its arguments
#define LOG_ERROR(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)

/// @brief Convert milliseconds to OS ticks
/// @param x - time in milliseconds
#define MS_TO_TICKS(x) pdMS_TO_TICKS(x)

/// @brief Block the calling task for a number of OS ticks
/// @param x - time in OS ticks (see MS_TO_TICKS, MAX_DELAY)
#define DELAY(x) vTaskDelay(x)

using TaskHandle = TaskHandle_t;    ///< Handle to a task. Must be usable as a std::map key (see CheckinTracker).
using QHandle = QueueHandle_t;      ///< Handle to a message queue
using StatusType = BaseType_t;      ///< Status returned by OS calls (PASS / FAIL)

/// @brief OS and target functions
/// @details Static class: there is only one OS and target. Any state the implementation needs is kept private in
///          TargetPort.cpp, since it depends on the target.
class TargetPort
{
    public:

        TargetPort() = delete;

        /// @brief Get the handle of the calling task
        /// @return handle of the currently running task
        static TaskHandle GetCurrTaskHandle();

        /// @brief Get the name of a task
        /// @param[in] taskId - task to get the name of
        /// @return name of the task
        static const char* GetTaskName(TaskHandle taskId);

        /// @brief Get the time since the OS started
        /// @details Wraps around at 2^32 ms. Use unsigned subtraction to get elapsed times.
        /// @return time in milliseconds
        static uint32_t GetTimeMs();

        /// @brief Add the calling task to the watchdog
        /// @details From then on, the calling task must call WatchdogReset() more often than the watchdog timeout,
        ///          or the device resets.
        static void StartWatchdog();

        /// @brief Reset (kick) the watchdog for the calling task
        static void WatchdogReset();

        /// @brief Create a message queue
        /// @param[in] qLength - maximum number of items the queue can hold
        /// @param[in] itemSize - size of each item in the queue, in bytes
        /// @return handle for the queue created, or nullptr if it couldn't be created
        static QHandle CreateQ(uint32_t qLength, uint32_t itemSize);

        /// @brief Put a message in a queue
        /// @details Doesn't block. Fails immediately if the queue is full.
        /// @param[in] qId - queue to put the message in
        /// @param[in] buf - pointer to the item to copy into the queue
        /// @return PASS if successful, FAIL otherwise
        static StatusType QSend(QHandle qId, const void* buf);

        /// @brief Get a message out of a queue
        /// @param[in] qId - queue to get the message from
        /// @param[out] buf - pointer to where the item from the queue is copied
        /// @param[in] timeout - max time to wait for a message, in OS ticks (see MS_TO_TICKS, MAX_DELAY)
        /// @return PASS if a message was received, FAIL if the timeout expired
        static StatusType QRecv(QHandle qId, void* buf, uint32_t timeout);

        /// @brief Write a table of all tasks (name, state, priority, stack high-water mark, number, core) to a buffer
        /// @details Asserts if the buffer may be too small for the current number of tasks, and then writes a short
        ///          error message instead of the table.
        /// @param[out] buf - buffer to write the null-terminated table to
        /// @param[in] size - size of the buffer, in bytes
        static void GetTaskList(char* buf, size_t size);

        /// @brief Write the run-time statistics of all tasks (name, absolute time, % time) to a buffer
        /// @details Asserts if the buffer may be too small for the current number of tasks, and then writes a short
        ///          error message instead of the table.
        /// @param[out] buf - buffer to write the null-terminated table to
        /// @param[in] size - size of the buffer, in bytes
        static void GetRunTimeStats(char* buf, size_t size);
};
