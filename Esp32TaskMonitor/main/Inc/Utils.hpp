#pragma once

#define DELAY_MS(x) vTaskDelay(pdMS_TO_TICKS(x))
#define KERNEL_TICKS_IN_MS() pdTICKS_TO_MS(xTaskGetTickCount())

