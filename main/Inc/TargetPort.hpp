#pragma once

/// @file TargetPort.hpp
/// @brief OS and target abstraction layer.
/// @details Wraps the RTOS and target calls used by the task monitor and the example tasks, so the code can be
///          ported to a different RTOS or target by changing only this file and TargetPort.cpp. This implementation
///          is for FreeRTOS on ESP-IDF.

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

#define MAX_DELAY portMAX_DELAY     ///< Wait forever (when used as a timeout)

/// @brief Assert that a condition is true
/// @note Asserts are intended to stay enabled, even in production builds. They can be disabled through the
///       project configuration (which defines NDEBUG), so never put required code inside them.
#define ASSERT(x) assert(x)

/// @brief Log an error message
/// @param tag - tag identifying the source of the message
/// @param format - printf style format string, followed by its arguments
#define LOG_ERROR(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)

/// @brief Convert milliseconds to OS ticks
#define MS_TO_TICKS(x) pdMS_TO_TICKS(x)

/// @brief Block the calling task for a number of OS ticks
#define DELAY(x) vTaskDelay(x)

typedef TaskHandle_t TaskHandle;    ///< Handle to a task
typedef QueueHandle_t QHandle;      ///< Handle to a message queue
typedef BaseType_t StatusType;      ///< Status returned by OS calls (PASS / FAIL)

/// @brief Get the handle of the calling task
/// @return handle of the currently running task
TaskHandle GetCurrTaskHandle();

/// @brief Get the name of a task
/// @param[in] taskId - task to get the name of
/// @return name of the task
const char* GetTaskName(TaskHandle taskId);

/// @brief Get the time since the OS started
/// @details Wraps around. Use unsigned subtraction to get elapsed times.
/// @return time in milliseconds
uint32_t GetTimeMs();

/// @brief Add the calling task to the watchdog
/// @details From then on, the calling task must call WatchdogReset() more often than the watchdog timeout, or the
///          device resets.
void StartWatchdog();

/// @brief Reset (kick) the watchdog for the calling task
void WatchdogReset();

/// @brief Create a message queue
/// @param[in] qLength - maximum number of items the queue can hold
/// @param[in] itemSize - size of each item in the queue, in bytes
/// @return handle for the queue created, or nullptr if it couldn't be created
QHandle CreateQ(uint32_t qLength, uint32_t itemSize);

/// @brief Put a message in a queue
/// @details Doesn't block. Fails immediately if the queue is full.
/// @param[in] qId - queue to put the message in
/// @param[in] buf - pointer to the item to copy into the queue
/// @return PASS if successful, FAIL otherwise
StatusType QSend(QHandle qId, const void* buf);

/// @brief Get a message out of a queue
/// @param[in] qId - queue to get the message from
/// @param[out] buf - pointer to where the item from the queue is copied
/// @param[in] timeout - max time to wait for a message, in OS ticks (see MS_TO_TICKS, MAX_DELAY)
/// @return PASS if a message was received, FAIL if the timeout expired
StatusType QRecv(QHandle qId, void* buf, uint32_t timeout);

/// @brief Write a table of all tasks (name, state, priority, stack high-water mark, number, core) to a buffer
/// @param[out] buf - buffer to write the null-terminated table to
/// @param[in] size - size of the buffer, in bytes
void GetTaskList(char* buf, size_t size);

/// @brief Write the run-time statistics of all tasks (name, absolute time, % time) to a buffer
/// @param[out] buf - buffer to write the null-terminated table to
/// @param[in] size - size of the buffer, in bytes
void GetRunTimeStats(char* buf, size_t size);
