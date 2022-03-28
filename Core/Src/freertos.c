/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "lptim.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static char taskListBuf[500];
static char taskStatsBuf[500];
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
uint32_t defaultTaskBuffer[ 256 ];
osStaticThreadDef_t defaultTaskControlBlock;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .cb_mem = &defaultTaskControlBlock,
  .cb_size = sizeof(defaultTaskControlBlock),
  .stack_mem = &defaultTaskBuffer[0],
  .stack_size = sizeof(defaultTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for taskMonitor */
osThreadId_t taskMonitorHandle;
uint32_t taskMonitorBuffer[ 256 ];
osStaticThreadDef_t taskMonitorControlBlock;
const osThreadAttr_t taskMonitor_attributes = {
  .name = "taskMonitor",
  .cb_mem = &taskMonitorControlBlock,
  .cb_size = sizeof(taskMonitorControlBlock),
  .stack_mem = &taskMonitorBuffer[0],
  .stack_size = sizeof(taskMonitorBuffer),
  .priority = (osPriority_t) osPriorityRealtime7,
};
/* Definitions for Task1 */
osThreadId_t Task1Handle;
uint32_t Task1Buffer[ 256 ];
osStaticThreadDef_t Task1ControlBlock;
const osThreadAttr_t Task1_attributes = {
  .name = "Task1",
  .cb_mem = &Task1ControlBlock,
  .cb_size = sizeof(Task1ControlBlock),
  .stack_mem = &Task1Buffer[0],
  .stack_size = sizeof(Task1Buffer),
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for Task2 */
osThreadId_t Task2Handle;
uint32_t Task2Buffer[ 256 ];
osStaticThreadDef_t Task2ControlBlock;
const osThreadAttr_t Task2_attributes = {
  .name = "Task2",
  .cb_mem = &Task2ControlBlock,
  .cb_size = sizeof(Task2ControlBlock),
  .stack_mem = &Task2Buffer[0],
  .stack_size = sizeof(Task2Buffer),
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Task3 */
osThreadId_t Task3Handle;
uint32_t Task3Buffer[ 256 ];
osStaticThreadDef_t Task3ControlBlock;
const osThreadAttr_t Task3_attributes = {
  .name = "Task3",
  .cb_mem = &Task3ControlBlock,
  .cb_size = sizeof(Task3ControlBlock),
  .stack_mem = &Task3Buffer[0],
  .stack_size = sizeof(Task3Buffer),
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for defaultTaskQ */
osMessageQueueId_t defaultTaskQHandle;
uint8_t defaultTaskQBuffer[ 10 * sizeof( uint8_t ) ];
osStaticMessageQDef_t defaultTaskQControlBlock;
const osMessageQueueAttr_t defaultTaskQ_attributes = {
  .name = "defaultTaskQ",
  .cb_mem = &defaultTaskQControlBlock,
  .cb_size = sizeof(defaultTaskQControlBlock),
  .mq_mem = &defaultTaskQBuffer,
  .mq_size = sizeof(defaultTaskQBuffer)
};
/* Definitions for taskMonitorQ */
osMessageQueueId_t taskMonitorQHandle;
uint8_t taskMonitorQBuffer[ 10 * sizeof( uint8_t ) ];
osStaticMessageQDef_t taskMonitorQControlBlock;
const osMessageQueueAttr_t taskMonitorQ_attributes = {
  .name = "taskMonitorQ",
  .cb_mem = &taskMonitorQControlBlock,
  .cb_size = sizeof(taskMonitorQControlBlock),
  .mq_mem = &taskMonitorQBuffer,
  .mq_size = sizeof(taskMonitorQBuffer)
};
/* Definitions for task1Q */
osMessageQueueId_t task1QHandle;
uint8_t task1QBuffer[ 10 * sizeof( uint8_t ) ];
osStaticMessageQDef_t task1QControlBlock;
const osMessageQueueAttr_t task1Q_attributes = {
  .name = "task1Q",
  .cb_mem = &task1QControlBlock,
  .cb_size = sizeof(task1QControlBlock),
  .mq_mem = &task1QBuffer,
  .mq_size = sizeof(task1QBuffer)
};
/* Definitions for task2Q */
osMessageQueueId_t task2QHandle;
uint8_t task2QBuffer[ 10 * sizeof( uint8_t ) ];
osStaticMessageQDef_t task2QControlBlock;
const osMessageQueueAttr_t task2Q_attributes = {
  .name = "task2Q",
  .cb_mem = &task2QControlBlock,
  .cb_size = sizeof(task2QControlBlock),
  .mq_mem = &task2QBuffer,
  .mq_size = sizeof(task2QBuffer)
};
/* Definitions for task3Q */
osMessageQueueId_t task3QHandle;
uint8_t task3QBuffer[ 10 * sizeof( uint8_t ) ];
osStaticMessageQDef_t task3QControlBlock;
const osMessageQueueAttr_t task3Q_attributes = {
  .name = "task3Q",
  .cb_mem = &task3QControlBlock,
  .cb_size = sizeof(task3QControlBlock),
  .mq_mem = &task3QBuffer,
  .mq_size = sizeof(task3QBuffer)
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTaskMonitor(void *argument);
void StartTask1(void *argument);
void StartTask2(void *argument);
void StartTask3(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{
    HAL_LPTIM_Counter_Start(&hlptim1, 0xFFFF);
}

static unsigned long lpCounter = 0;

__weak unsigned long getRunTimeCounterValue(void)
{

    static unsigned long prevLpCounter = 0;

    unsigned long currentLpCounter = HAL_LPTIM_ReadCounter(&hlptim1);;

    if (prevLpCounter > currentLpCounter)
    {
        lpCounter += (currentLpCounter + (0xFFFF - prevLpCounter));
    }
    else
    {
        lpCounter += (currentLpCounter - prevLpCounter);
    }

    prevLpCounter = currentLpCounter;

    return lpCounter;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of defaultTaskQ */
  defaultTaskQHandle = osMessageQueueNew (10, sizeof(uint8_t), &defaultTaskQ_attributes);

  /* creation of taskMonitorQ */
  taskMonitorQHandle = osMessageQueueNew (10, sizeof(uint8_t), &taskMonitorQ_attributes);

  /* creation of task1Q */
  task1QHandle = osMessageQueueNew (10, sizeof(uint8_t), &task1Q_attributes);

  /* creation of task2Q */
  task2QHandle = osMessageQueueNew (10, sizeof(uint8_t), &task2Q_attributes);

  /* creation of task3Q */
  task3QHandle = osMessageQueueNew (10, sizeof(uint8_t), &task3Q_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of taskMonitor */
  taskMonitorHandle = osThreadNew(StartTaskMonitor, NULL, &taskMonitor_attributes);

  /* creation of Task1 */
  Task1Handle = osThreadNew(StartTask1, NULL, &Task1_attributes);

  /* creation of Task2 */
  Task2Handle = osThreadNew(StartTask2, NULL, &Task2_attributes);

  /* creation of Task3 */
  Task3Handle = osThreadNew(StartTask3, NULL, &Task3_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
      printf("LP Counter: %lu\r\n\r\n", lpCounter);

      vTaskList(taskListBuf);
      printf("TASK INFO**********************************************\r\n");
      printf("Name          State  Priority   Stack   Num\r\n");
      printf("*******************************************\r\n");
      printf("%s\r\n", taskListBuf);
      printf("\r\n");

      vTaskGetRunTimeStats(taskStatsBuf);
      printf("TASK STATS INFO****************************************\r\n");
      printf("Name             Abs Time       %% Time\r\n");
      printf("*******************************************\r\n");
      printf("%s\r\n", taskStatsBuf);
      printf("\r\n");

      osDelay(10000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTaskMonitor */
/**
* @brief Function implementing the taskMonitor thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskMonitor */
void StartTaskMonitor(void *argument)
{
  /* USER CODE BEGIN StartTaskMonitor */
  /* Infinite loop */
  for(;;)
  {
    osDelay(10);
  }
  /* USER CODE END StartTaskMonitor */
}

/* USER CODE BEGIN Header_StartTask1 */
/**
* @brief Function implementing the Task1 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask1 */
void StartTask1(void *argument)
{
  /* USER CODE BEGIN StartTask1 */
    int count = 0;
  /* Infinite loop */
  for(;;)
  {
//      printf("Task1: %d\r\n", count++);
    osDelay(500);
  }
  /* USER CODE END StartTask1 */
}

/* USER CODE BEGIN Header_StartTask2 */
/**
* @brief Function implementing the Task2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask2 */
void StartTask2(void *argument)
{
  /* USER CODE BEGIN StartTask2 */
    int count = 0;
  /* Infinite loop */
  for(;;)
  {
//      printf("Task2: %d\r\n", count++);
    osDelay(250);
  }
  /* USER CODE END StartTask2 */
}

/* USER CODE BEGIN Header_StartTask3 */
/**
* @brief Function implementing the Task3 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask3 */
void StartTask3(void *argument)
{
  /* USER CODE BEGIN StartTask3 */
    int count = 0;
  /* Infinite loop */
  for(;;)
  {
//      printf("Task3: %d\r\n", count++);
    osDelay(100);
  }
  /* USER CODE END StartTask3 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

