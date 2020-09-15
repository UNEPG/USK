/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include "stdio.h"
/* USER CODE END Includes */


/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
struct Channel 
{
   GPIO_TypeDef* led_gpio;
   uint16_t led_pin;
};

struct leds
{
    bool state[5];
    uint16_t durat;
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
TIM_HandleTypeDef htim7;
bool periodic_blink_flag = false;
bool non_periodic_blink_flag_finished = false;
uint8_t step = 0;
uint8_t blink_cnt = 0;
struct leds* blink;

struct Channel ch[5] = 
{
    { EXTLED_1_GPIO_Port,   EXTLED_1_Pin},
    { EXTLED_2_GPIO_Port,   EXTLED_2_Pin},
    { EXTLED_3_GPIO_Port,   EXTLED_3_Pin},
    { EXTLED_4_GPIO_Port,   EXTLED_4_Pin},
    { ACT_LED_GPIO_Port,    ACT_LED_Pin},
};

struct leds startup_blink[] =
{
    { .state = { 1, 1, 1, 1, 0 }, .durat = 100  },
    { .state = { 0, 0, 0, 0, 0 }, .durat = 500  },
    { .state = { 1, 0, 0, 0, 0 }, .durat = 100  },
    { .state = { 0, 1, 0, 0, 0 }, .durat = 100  },
    { .state = { 0, 0, 1, 0, 0 }, .durat = 100  },
    { .state = { 0, 0, 0, 1, 0 }, .durat = 100  },
    { .state = { 0, 0, 0, 0, 0 }, .durat = 100  },
};

struct leds periodic_blinks[] =
{
    { .state = { 0, 0, 0, 0, 1 }, .durat = 100  },
    { .state = { 0, 0, 0, 0, 0 }, .durat = 100  },
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
/* USER CODE BEGIN PFP */
void TurnOffAll(void);

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

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    
    htim7.Instance = TIM7;
    htim7.Init.Prescaler = 8000;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = 10;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
    {
        Error_Handler();
    }
    
    if (HAL_TIM_Base_Start_IT(&htim7) != HAL_OK)
    {
        Error_Handler();
    }
    
    HAL_NVIC_SetPriority(TIM7_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);
    
    /* Starting of code */
    TurnOffAll();

    periodic_blink_flag = false;
    non_periodic_blink_flag_finished = false;
    blink_cnt = (sizeof(startup_blink)/sizeof(startup_blink[0]));
    blink = (struct leds*) &startup_blink;
    
    while( !non_periodic_blink_flag_finished )
    {
        HAL_Delay(10);
    }
    
    periodic_blink_flag = true;
    blink_cnt = (sizeof(periodic_blinks)/sizeof(periodic_blinks[0]));
    blink = (struct leds*) &periodic_blinks;
    
    /* Infinite loop */
    while (1)
    {
        HAL_Delay(1000);
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

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void TurnOffAll(void)
{
    HAL_GPIO_WritePin(KNIFE_1_GPIO_Port,    KNIFE_1_Pin,    GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EXTLED_1_GPIO_Port,   EXTLED_1_Pin,   GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KNIFE_2_GPIO_Port,    KNIFE_2_Pin,    GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EXTLED_2_GPIO_Port,   EXTLED_2_Pin,   GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KNIFE_3_GPIO_Port,    KNIFE_3_Pin,    GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EXTLED_3_GPIO_Port,   EXTLED_3_Pin,   GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KNIFE_4_GPIO_Port,    KNIFE_4_Pin,    GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EXTLED_4_GPIO_Port,   EXTLED_4_Pin,   GPIO_PIN_RESET);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

    for(int i=0; i<5; i++)
    {
        if( blink[step].state[i] == true )
        {
            HAL_GPIO_WritePin(ch[i].led_gpio, ch[i].led_pin, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(ch[i].led_gpio, ch[i].led_pin, GPIO_PIN_RESET);
        }
    }
    
    htim7.Instance->ARR = blink[step].durat;
    step++;
    
    if( step == blink_cnt )
    {
        step = 0;
        if( periodic_blink_flag )
        {
            
        }
        else
        {
            non_periodic_blink_flag_finished = true;
        }
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
