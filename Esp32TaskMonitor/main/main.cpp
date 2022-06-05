#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "DefaultTask.hpp"
#include "TaskMonitor.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
    void app_main(void);
#ifdef __cplusplus
}
#endif

constexpr uint32_t DEFAULT_TASK_STACK_SIZE = 2048;
constexpr UBaseType_t DEFAULT_TASK_PRIORITY = 2;

constexpr uint32_t TASK_MONITOR_STACK_SIZE = 2048;
constexpr UBaseType_t TASK_MONITOR_PRIORITY = configMAX_PRIORITIES - 1;

//------------------------------------------------------------------
// StartDefaultTask
//------------------------------------------------------------------
void StartDefaultTask(void *argument)
{
    DefaultTask dTask;
    dTask.Initialize();
    dTask.Run();
}

//------------------------------------------------------------------
// StartTaskMonitor
//------------------------------------------------------------------
void StartTaskMonitor(void *argument)
{
    TaskMonitor taskMon;
    taskMon.Initialize();
    taskMon.Run();
}

//------------------------------------------------------------------
// Main Task
//------------------------------------------------------------------
void app_main(void)
{
    TaskHandle_t taskHandle = nullptr;

    // Task monitor
    xTaskCreate(&StartTaskMonitor, "taskMonitor", TASK_MONITOR_STACK_SIZE, NULL, TASK_MONITOR_PRIORITY, &taskHandle);

    // default task
    xTaskCreate(&StartDefaultTask, "defaultTask", DEFAULT_TASK_STACK_SIZE, NULL, DEFAULT_TASK_PRIORITY, &taskHandle);
}

