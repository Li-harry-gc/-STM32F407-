/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "dht11.h"
#include "bh1750.h"
#include "oled.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

float temperature = 0;
float humidity = 0;
float lux = 0;
uint8_t dht11_ok = 0;

uint8_t current_mode = 0;     /* 0~3 */
uint8_t mode_changed = 1;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void delay_us(uint32_t us);
void Key_Scan(void);
void Sensor_ReadAndDisplay(void);
static void OLED_UpdateDisplay(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */

  OLED_Init();
  OLED_Clear();
  OLED_ShowString(8,  1, "JingSai Project");
  OLED_ShowString(8,  3, "STM32F407ZGT6");
  OLED_ShowString(20, 5, "Initializing...");
  OLED_Refresh();

  BH1750_Init();

  OLED_Clear();
  OLED_ShowString(20, 5, "Starting...");
  OLED_Refresh();
  HAL_Delay(500);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* 读传感器并显示 */
    Sensor_ReadAndDisplay();

    /* 接下来的 2 秒内，每毫秒扫一次按键 */
    for (uint32_t i = 0; i < 2000; i++)
    {
        Key_Scan();
        if (mode_changed)          /* 按键切换了模式 */
        {
            mode_changed = 0;
            OLED_UpdateDisplay();  /* 立即刷新显示，不等下次读数 */
        }
        HAL_Delay(1);
    }
    /* USER CODE END 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
 * @brief  微秒延时（基于 TIM6）
 *         注意：最大延时 65535µs（约 65ms），大于此值需用 HAL_Delay
 */
void delay_us(uint32_t us)
{
    if (us > 65530) us = 65530;   // 防止计数器溢出
    __HAL_TIM_SET_COUNTER(&htim6, 0);
    while (__HAL_TIM_GET_COUNTER(&htim6) < us);
}

/**
 * @brief  按键扫描（PE4，轮询，带消抖）
 */
void Key_Scan(void)
{
    static uint32_t last_tick = 0;
    static uint8_t last_state = 0;      /* 上次引脚状态，默认 LOW（空闲低电平） */
    uint32_t now = HAL_GetTick();
    uint8_t cur_state = HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin);

    /* 检测上升沿：从 LOW 变成 HIGH 才算一次按下 */
    if (last_state == 0 && cur_state == 1)
    {
        if (now - last_tick > 100)      /* 100ms 消抖 */
        {
            last_tick = now;
            current_mode = (current_mode + 1) % 4;
            mode_changed = 1;
        }
    }
    last_state = cur_state;
}

/**
 * @brief  把整数和一位小数转成字符串，避免 sprintf %f 卡死
 */
static void FloatToStr(int16_t val, int8_t decimal, char *out)
{
    uint8_t neg = 0;
    if (val < 0) { neg = 1; val = -val; }
    uint16_t int_part = val / 10;
    uint16_t dec_part = val % 10;
    if (neg) *out++ = '-';
    out += sprintf((char*)out, "%u.%u", int_part, dec_part);
    if (decimal >= 0) *out++ = decimal;
    *out = '\0';
}

/**
 * @brief  按当前模式刷新显示（用最近一次的传感器数据）
 */
static void OLED_UpdateDisplay(void)
{
    char buf[16];

    OLED_Clear();

    switch (current_mode)
    {
        case 0:
            if (dht11_ok)
            {
                OLED_ShowString(0, 0, "Temp:");
                FloatToStr((int16_t)(temperature * 10), 'C', buf);
                OLED_ShowString(36, 0, buf);
                OLED_ShowString(0, 3, "Humi:");
                FloatToStr((int16_t)(humidity * 10), '%', buf);
                OLED_ShowString(36, 3, buf);
            }
            else
            {
                OLED_ShowString(0, 0, "DHT11 Error!");
                OLED_ShowString(0, 3, "Check Sensor");
            }
            OLED_ShowString(0, 6, "Lux:");
            if (lux >= 0) {
                sprintf(buf, "%u", (uint16_t)lux);
                OLED_ShowString(36, 6, buf);
            } else {
                OLED_ShowString(36, 6, "---");
            }
            break;

        case 1:
            if (dht11_ok) {
                OLED_ShowString(20, 0, "Temperature");
                FloatToStr((int16_t)(temperature * 10), 'C', buf);
                OLED_ShowString(28, 3, buf);
            } else {
                OLED_ShowString(20, 3, "No Data");
            }
            break;

        case 2:
            if (dht11_ok) {
                OLED_ShowString(28, 0, "Humidity");
                FloatToStr((int16_t)(humidity * 10), '%', buf);
                OLED_ShowString(28, 3, buf);
            } else {
                OLED_ShowString(20, 3, "No Data");
            }
            break;

        case 3:
            OLED_ShowString(36, 0, "Illum");
            if (lux >= 0) {
                sprintf(buf, "%u", (uint16_t)lux);
                OLED_ShowString(20, 3, buf);
                OLED_ShowString(74, 3, "lux");
            } else {
                OLED_ShowString(20, 3, "Error");
            }
            break;
    }

    OLED_Refresh();
}

/**
 * @brief  读取传感器数据并刷新显示
 */
void Sensor_ReadAndDisplay(void)
{
    uint8_t err;

    /* 读传感器 */
    err = DHT11_Read(&temperature, &humidity);
    dht11_ok = (err == DHT11_OK);
    lux = BH1750_ReadLux();

    /* 显示 */
    OLED_UpdateDisplay();
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
