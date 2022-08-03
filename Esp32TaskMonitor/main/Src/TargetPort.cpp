
#include "TargetPort.hpp"

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
char* GetTaskName(TaskHandle taskId)
{
    return pcTaskGetName(taskId);
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
StatusType QSend(QHandle qId, void* const buf)
{
    return xQueueSend(qId, buf, 0);
}

//------------------------------------------------------------------
// QRecv
//------------------------------------------------------------------
StatusType QRecv(QHandle qId, void* const buf, uint32_t timeout)
{
    return xQueueReceive(qId, buf, timeout);
}

//------------------------------------------------------------------
// StartWatchdog
//------------------------------------------------------------------
void StartWatchdog()
{
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
}
