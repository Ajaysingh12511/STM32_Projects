/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : FreeRTOS Queue Example (Temp → Fan Control)
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>

/* ================== RTOS OBJECTS ================== */

/* Temp Check Task */
osThreadId_t tempCheckTaskHandle;
const osThreadAttr_t tempCheckTask_attributes = {
  .name = "tempCheckTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Fan Control Task */
osThreadId_t fanControlTaskHandle;
const osThreadAttr_t fanControlTask_attributes = {
  .name = "fanControlTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Message Queue */
osMessageQueueId_t tempQueueHandle;
const osMessageQueueAttr_t tempQueue_attributes = {
  .name = "tempQueue"
};

/* ================== FUNCTION PROTOTYPES ================== */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void StartTempCheckTask(void *argument);
void StartFanControlTask(void *argument);

/* ================== MAIN ================== */

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  osKernelInitialize();

  /* Create Queue */
  tempQueueHandle = osMessageQueueNew(5, sizeof(int), &tempQueue_attributes);

  /* Create Tasks */
  tempCheckTaskHandle = osThreadNew(StartTempCheckTask, NULL, &tempCheckTask_attributes);
  fanControlTaskHandle = osThreadNew(StartFanControlTask, NULL, &fanControlTask_attributes);

  osKernelStart();

  while (1)
  {
    /* Never reaches here */
  }
}

/* ================== TASK 1 : TEMP SENSOR ================== */

void StartTempCheckTask(void *argument)
{
  int currentTemp = 25;

  for (;;)
  {
    /* Dummy temperature simulation */
    currentTemp = (currentTemp < 35) ? currentTemp + 1 : 25;

    /* Send temperature to queue */
    osMessageQueuePut(tempQueueHandle, &currentTemp, 0, osWaitForever);

    printf("[TEMP SENSOR] Temp = %d C\r\n", currentTemp);

    osDelay(1000);   // 1 second
  }
}

/* ================== TASK 2 : FAN CONTROL ================== */

void StartFanControlTask(void *argument)
{
  int receivedTemp;
  int fanState = -1;   // -1 = undefined, 0 = OFF, 1 = ON

  for (;;)
  {
    /* BLOCKING wait for temperature */
    osMessageQueueGet(tempQueueHandle, &receivedTemp, NULL, osWaitForever);

    int newState = (receivedTemp >= 30) ? 1 : 0;

    if (newState != fanState)
    {
      fanState = newState;

      if (fanState)
        printf("[FAN] FAN ON  (Temp = %d C)\r\n", receivedTemp);
      else
        printf("[FAN] FAN OFF (Temp = %d C)\r\n", receivedTemp);
    }
  }
}

/* ================== CLOCK CONFIG ================== */

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType =
      RCC_CLOCKTYPE_HCLK |
      RCC_CLOCKTYPE_SYSCLK |
      RCC_CLOCKTYPE_PCLK1 |
      RCC_CLOCKTYPE_PCLK2;

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
}

/* ================== GPIO INIT ================== */

static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
}

/* ================== SWO PRINTF ================== */

int __io_putchar(int ch)
{
  ITM_SendChar(ch);
  return ch;
}

/* ================== ERROR HANDLER ================== */

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
