/// @file main.cpp
/// @brief Example application: creates the task monitor and the example tasks.

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "DefaultTask.hpp"
#include "TaskMonitor.hpp"
#include "Task1.hpp"
#include "Task2.hpp"
#include "Task3.hpp"

extern "C" void app_main(void); ///< ESP-IDF application entry point (C linkage)

constexpr uint32_t TASK_MONITOR_STACK_SIZE = 2048;                      ///< TaskMonitor stack size, in bytes
constexpr UBaseType_t TASK_MONITOR_PRIORITY = configMAX_PRIORITIES - 1; ///< TaskMonitor priority (highest)

constexpr uint32_t DEFAULT_TASK_STACK_SIZE = 2048;                      ///< DefaultTask stack size, in bytes
constexpr UBaseType_t DEFAULT_TASK_PRIORITY = 2;                        ///< DefaultTask priority (low)

constexpr uint32_t TASK_1_STACK_SIZE = 2048;                            ///< Task1 stack size, in bytes
constexpr UBaseType_t TASK_1_PRIORITY = configMAX_PRIORITIES - 2;       ///< Task1 priority

constexpr uint32_t TASK_2_STACK_SIZE = 2048;                            ///< Task2 stack size, in bytes
constexpr UBaseType_t TASK_2_PRIORITY = configMAX_PRIORITIES - 3;       ///< Task2 priority

constexpr uint32_t TASK_3_STACK_SIZE = 2048;                            ///< Task3 stack size, in bytes
constexpr UBaseType_t TASK_3_PRIORITY = configMAX_PRIORITIES - 4;       ///< Task3 priority

//------------------------------------------------------------------
// StartTaskMonitor
//------------------------------------------------------------------
/// @brief Entry point for the TaskMonitor task. Never returns.
/// @param[in] argument - task argument (unused)
static void StartTaskMonitor(void* argument)
{
    TaskMonitor taskMon;
    taskMon.Run();
}

//------------------------------------------------------------------
// StartDefaultTask
//------------------------------------------------------------------
/// @brief Entry point for the DefaultTask task. Never returns.
/// @param[in] argument - task argument (unused)
static void StartDefaultTask(void* argument)
{
    DefaultTask task;
    task.Initialize();
    task.Run();
}

//------------------------------------------------------------------
// StartTask1
//------------------------------------------------------------------
/// @brief Entry point for the Task1 task. Never returns.
/// @param[in] argument - task argument (unused)
static void StartTask1(void* argument)
{
    Task1 task;
    task.Initialize();
    task.Run();
}

//------------------------------------------------------------------
// StartTask2
//------------------------------------------------------------------
/// @brief Entry point for the Task2 task. Never returns.
/// @param[in] argument - task argument (unused)
static void StartTask2(void* argument)
{
    Task2 task;
    task.Initialize();
    task.Run();
}

//------------------------------------------------------------------
// StartTask3
//------------------------------------------------------------------
/// @brief Entry point for the Task3 task. Never returns.
/// @param[in] argument - task argument (unused)
static void StartTask3(void* argument)
{
    Task3 task;
    task.Initialize();
    task.Run();
}

//------------------------------------------------------------------
// Main Task
//------------------------------------------------------------------
/// @brief Application entry point: creates the task monitor and the example tasks
void app_main(void)
{
    // Create the task monitor's queue before any task can register with it
    TaskMonitor::Initialize();

    // Task monitor
    xTaskCreate(&StartTaskMonitor, "TaskMonitor", TASK_MONITOR_STACK_SIZE, nullptr, TASK_MONITOR_PRIORITY, nullptr);

    // Default task
    xTaskCreate(&StartDefaultTask, "DefaultTask", DEFAULT_TASK_STACK_SIZE, nullptr, DEFAULT_TASK_PRIORITY, nullptr);

    // Task 1
    xTaskCreate(&StartTask1, "Task1", TASK_1_STACK_SIZE, nullptr, TASK_1_PRIORITY, nullptr);

    // Task 2
    xTaskCreate(&StartTask2, "Task2", TASK_2_STACK_SIZE, nullptr, TASK_2_PRIORITY, nullptr);

    // Task 3
    xTaskCreate(&StartTask3, "Task3", TASK_3_STACK_SIZE, nullptr, TASK_3_PRIORITY, nullptr);
}
