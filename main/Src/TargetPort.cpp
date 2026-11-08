/// @file TargetPort.cpp
/// @brief OS and target abstraction layer for FreeRTOS on ESP-IDF (see TargetPort.hpp).

#include "TargetPort.hpp"

#include "esp_task_wdt.h"

#include <cstdio>

namespace
{
    /// @brief Bytes reserved per task for vTaskList() / vTaskGetRunTimeStats() output
    /// @details These FreeRTOS functions don't take a buffer size, so the buffer is checked against the current
    ///          number of tasks before calling them.
    constexpr size_t BYTES_PER_TASK_LINE = 64;

    /// @brief Check that a buffer can hold one formatted line for every task
    /// @details Asserts if it can't, and writes a short error message to the buffer instead.
    /// @param[out] buf - buffer that will be written
    /// @param[in] size - size of the buffer, in bytes
    /// @return true if the buffer is large enough
    bool BufferFitsAllTasks(char* buf, size_t size)
    {
        const bool fits = (size >= (uxTaskGetNumberOfTasks() * BYTES_PER_TASK_LINE));
        ASSERT(fits);
        if (!fits)
        {
            snprintf(buf, size, "buffer too small\r\n");
        }
        return fits;
    }
}

//------------------------------------------------------------------
// GetCurrTaskHandle
//------------------------------------------------------------------
TaskHandle GetCurrTaskHandle()
{
    return xTaskGetCurrentTaskHandle();
}

//------------------------------------------------------------------
// GetTaskName
//------------------------------------------------------------------
const char* GetTaskName(TaskHandle taskId)
{
    return pcTaskGetName(taskId);
}

//------------------------------------------------------------------
// GetTimeMs
//------------------------------------------------------------------
uint32_t GetTimeMs()
{
    return pdTICKS_TO_MS(xTaskGetTickCount());
}

//------------------------------------------------------------------
// StartWatchdog
//------------------------------------------------------------------
void StartWatchdog()
{
    ESP_ERROR_CHECK(esp_task_wdt_add(nullptr));
}

//------------------------------------------------------------------
// WatchdogReset
//------------------------------------------------------------------
void WatchdogReset()
{
    esp_task_wdt_reset();
}

//------------------------------------------------------------------
// CreateQ
//------------------------------------------------------------------
QHandle CreateQ(uint32_t qLength, uint32_t itemSize)
{
    return xQueueCreate(qLength, itemSize);
}

//------------------------------------------------------------------
// QSend
//------------------------------------------------------------------
StatusType QSend(QHandle qId, const void* buf)
{
    return xQueueSend(qId, buf, 0);
}

//------------------------------------------------------------------
// QRecv
//------------------------------------------------------------------
StatusType QRecv(QHandle qId, void* buf, uint32_t timeout)
{
    return xQueueReceive(qId, buf, timeout);
}

//------------------------------------------------------------------
// GetTaskList
//------------------------------------------------------------------
void GetTaskList(char* buf, size_t size)
{
    if (BufferFitsAllTasks(buf, size))
    {
        vTaskList(buf);
    }
}

//------------------------------------------------------------------
// GetRunTimeStats
//------------------------------------------------------------------
void GetRunTimeStats(char* buf, size_t size)
{
    if (BufferFitsAllTasks(buf, size))
    {
        vTaskGetRunTimeStats(buf);
    }
}
