#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "DefaultTask.hpp"
//#include "esp_system.h"
//#include "driver/gpio.h"

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
// Main Task
//------------------------------------------------------------------
void app_main(void)
{
    TaskHandle_t* taskHandle = nullptr;

    // default task
    xTaskCreate(&StartDefaultTask, "defaultTask", DEFAULT_TASK_STACK_SIZE, NULL, DEFAULT_TASK_PRIORITY, taskHandle);
    // Task monitor register
}

