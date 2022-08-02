#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "DefaultTask.hpp"
#include "TaskMonitor.hpp"
#include "Task1.hpp"
#include "Task2.hpp"
#include "Task3.hpp"

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

constexpr uint32_t TASK_1_STACK_SIZE = 2048;
constexpr UBaseType_t TASK_1_PRIORITY = configMAX_PRIORITIES - 2;

constexpr uint32_t TASK_2_STACK_SIZE = 2048;
constexpr UBaseType_t TASK_2_PRIORITY = configMAX_PRIORITIES - 3;

constexpr uint32_t TASK_3_STACK_SIZE = 2048;
constexpr UBaseType_t TASK_3_PRIORITY = configMAX_PRIORITIES - 4;

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
// StartTask1
//------------------------------------------------------------------
void StartTask1(void *argument)
{
    Task1 task;
    task.Initialize();
    task.Run();
}

//------------------------------------------------------------------
// StartTask2
//------------------------------------------------------------------
void StartTask2(void *argument)
{
    Task2 task;
    task.Initialize();
    task.Run();
}

//------------------------------------------------------------------
// StartTask3
//------------------------------------------------------------------
void StartTask3(void *argument)
{
    Task3 task;
    task.Initialize();
    task.Run();
}

//------------------------------------------------------------------
// Main Task
//------------------------------------------------------------------
void app_main(void)
{
    TaskHandle_t taskHandle = nullptr;

    // Task monitor
    xTaskCreate(&StartTaskMonitor, "TaskMonitor", TASK_MONITOR_STACK_SIZE, NULL, TASK_MONITOR_PRIORITY, &taskHandle);

    // Default task
    xTaskCreate(&StartDefaultTask, "DefaultTask", DEFAULT_TASK_STACK_SIZE, NULL, DEFAULT_TASK_PRIORITY, &taskHandle);

    // Task 1
    xTaskCreate(&StartTask1, "Task1", TASK_1_STACK_SIZE, NULL, TASK_1_PRIORITY, &taskHandle);

    // task 2
    xTaskCreate(&StartTask2, "Task2", TASK_2_STACK_SIZE, NULL, TASK_2_PRIORITY, &taskHandle);

    // task 3
    xTaskCreate(&StartTask3, "Task3", TASK_3_STACK_SIZE, NULL, TASK_3_PRIORITY, &taskHandle);
}

