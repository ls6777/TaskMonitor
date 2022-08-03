#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_task_wdt.h"

#define PASS pdPASS
#define FAIL pdFAIL

#define TRUE pdTRUE
#define FALSE pdFALSE

#define MAX_DELAY portMAX_DELAY

#define ASSERT(x) assert(x)
#define LOG_ERROR(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)

#define MS_TO_TICKS(x) pdMS_TO_TICKS(x)
#define DELAY(x) vTaskDelay(x)

typedef TaskHandle_t TaskHandle;
typedef QueueHandle_t QHandle;
typedef BaseType_t StatusType;

/// @brief Get Current Task Handle
/// @return Handle for currently running task
TaskHandle GetCurrTaskHandle();

/// @brief Get Task Name
/// @param taskId - task Id of desired task
/// @return name for the task
char* GetTaskName(TaskHandle taskId);

/// @brief Reset the watchdog
void WatchdogReset();

/// @brief Create the Q
/// @param QLength - Length of the Q
/// @param ItemSize - Size of each item in the Q
/// @return Handle for the Q created
QHandle CreateQ(uint32_t QLength, uint32_t ItemSize);

/// @brief Put a message in the Q
/// @param qId - Id of the Q to put the message in
/// @param buf - pointer to the item to copy into the q
/// @return PASS if successful, FALSE otherwise
StatusType QSend(QHandle qId, void* const buf);

/// @brief Get a message out of the Q
/// @param qId - Id of the Q to put the message in
/// @param buf - pointer to the item from the q
/// @param timeout - max time to wait for something to get out of the Q
/// @return PASS if successful, FALSE otherwise
StatusType QRecv(QHandle qId, void* const buf, uint32_t timeout);

/// @brief Start the watchdog
void StartWatchdog();


