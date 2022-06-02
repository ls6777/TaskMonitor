/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern osThreadId_t defaultTaskHandle;
extern osThreadId_t task1Handle;
extern osThreadId_t task2Handle;
extern osThreadId_t task3Handle;

extern osMessageQueueId_t taskMonitorQHandle;
extern osMessageQueueId_t defaultTaskQHandle;
extern osMessageQueueId_t task1QHandle;
extern osMessageQueueId_t task2QHandle;
extern osMessageQueueId_t task3QHandle;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
static const uint32_t WORD_SIZE = sizeof(uint32_t);
static const uint32_t DOUBLE_WORD_SIZE = sizeof(uint64_t);

static const uint32_t MS_PER_SEC = 1000;
static const uint32_t SECS_PER_MIN = 60;
static const uint32_t MINS_PER_HOUR = 60;
static const uint32_t HOURS_PER_DAY = 24;

static const uint32_t BYTES_TO_KBYTES = 1024;
static const uint32_t KBYTES_TO_MBYTES = 1024;
static const uint32_t BYTES_TO_MBYTES = BYTES_TO_KBYTES * KBYTES_TO_MBYTES;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define ASSERT(x) assert_param(x)
#define DELAY_MS(x) osDelay(pdMS_TO_TICKS(x))
#define KERNEL_TICKS_IN_MS() pdMS_TO_TICKS(osKernelGetTickCount())
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

// Uncomment to enable Task Monitor Max times
#define ENABLE_TASK_MAX_CHECKIN_TIMES

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
