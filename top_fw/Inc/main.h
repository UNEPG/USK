/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EXTLED_1_Pin GPIO_PIN_13
#define EXTLED_1_GPIO_Port GPIOC
#define KNIFE_2_Pin GPIO_PIN_14
#define KNIFE_2_GPIO_Port GPIOC
#define ANT_CTRL_2_Pin GPIO_PIN_15
#define ANT_CTRL_2_GPIO_Port GPIOC
#define RS_DE_Pin GPIO_PIN_2
#define RS_DE_GPIO_Port GPIOA
#define KNIFE_3_Pin GPIO_PIN_3
#define KNIFE_3_GPIO_Port GPIOA
#define ADC_VSYS_Pin GPIO_PIN_4
#define ADC_VSYS_GPIO_Port GPIOA
#define EXTLED_2_Pin GPIO_PIN_5
#define EXTLED_2_GPIO_Port GPIOA
#define ANT_CTRL_3_Pin GPIO_PIN_6
#define ANT_CTRL_3_GPIO_Port GPIOA
#define VBUS_DET_Pin GPIO_PIN_7
#define VBUS_DET_GPIO_Port GPIOA
#define ANT_CTRL_4_Pin GPIO_PIN_12
#define ANT_CTRL_4_GPIO_Port GPIOB
#define EXTLED_3_Pin GPIO_PIN_13
#define EXTLED_3_GPIO_Port GPIOB
#define EXTLED_4_Pin GPIO_PIN_14
#define EXTLED_4_GPIO_Port GPIOB
#define ANT_CTRL_1_Pin GPIO_PIN_15
#define ANT_CTRL_1_GPIO_Port GPIOB
#define USB_PULL_Pin GPIO_PIN_8
#define USB_PULL_GPIO_Port GPIOA
#define KNIFE_4_Pin GPIO_PIN_9
#define KNIFE_4_GPIO_Port GPIOA
#define KNIFE_1_Pin GPIO_PIN_10
#define KNIFE_1_GPIO_Port GPIOA
#define ARM_PIN_Pin GPIO_PIN_6
#define ARM_PIN_GPIO_Port GPIOB
#define ACT_LED_Pin GPIO_PIN_7
#define ACT_LED_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
