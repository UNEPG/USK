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
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal.h"

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
#define EN_CH3_Pin GPIO_PIN_2
#define EN_CH3_GPIO_Port GPIOE
#define RST_CH3_Pin GPIO_PIN_3
#define RST_CH3_GPIO_Port GPIOE
#define FAULT_CH4_Pin GPIO_PIN_4
#define FAULT_CH4_GPIO_Port GPIOE
#define FAULT_CH4_EXTI_IRQn EXTI4_15_IRQn
#define EN_CH4_Pin GPIO_PIN_5
#define EN_CH4_GPIO_Port GPIOE
#define RST_CH4_Pin GPIO_PIN_6
#define RST_CH4_GPIO_Port GPIOE
#define BC_5V_Pin GPIO_PIN_0
#define BC_5V_GPIO_Port GPIOC
#define BC_3V3_Pin GPIO_PIN_1
#define BC_3V3_GPIO_Port GPIOC
#define CS_1CH_Pin GPIO_PIN_2
#define CS_1CH_GPIO_Port GPIOC
#define CS_2CH_Pin GPIO_PIN_3
#define CS_2CH_GPIO_Port GPIOC
#define CS_3CH_Pin GPIO_PIN_0
#define CS_3CH_GPIO_Port GPIOA
#define CS_4CH_Pin GPIO_PIN_1
#define CS_4CH_GPIO_Port GPIOA
#define CS_5CH_Pin GPIO_PIN_2
#define CS_5CH_GPIO_Port GPIOA
#define SLR_X_Pin GPIO_PIN_3
#define SLR_X_GPIO_Port GPIOA
#define SLR_Y_Pin GPIO_PIN_4
#define SLR_Y_GPIO_Port GPIOA
#define SLR_Z_Pin GPIO_PIN_5
#define SLR_Z_GPIO_Port GPIOA
#define ADAPT_V_Pin GPIO_PIN_6
#define ADAPT_V_GPIO_Port GPIOA
#define AMON_BMON_Pin GPIO_PIN_7
#define AMON_BMON_GPIO_Port GPIOA
#define VBAT_2C_Pin GPIO_PIN_4
#define VBAT_2C_GPIO_Port GPIOC
#define TEMP_L_Pin GPIO_PIN_5
#define TEMP_L_GPIO_Port GPIOC
#define TEMP_R_Pin GPIO_PIN_0
#define TEMP_R_GPIO_Port GPIOB
#define VBAT_1C_Pin GPIO_PIN_1
#define VBAT_1C_GPIO_Port GPIOB
#define RST_CH5_Pin GPIO_PIN_2
#define RST_CH5_GPIO_Port GPIOB
#define EN_CH5_Pin GPIO_PIN_7
#define EN_CH5_GPIO_Port GPIOE
#define FAULT_CH5_Pin GPIO_PIN_8
#define FAULT_CH5_GPIO_Port GPIOE
#define FAULT_CH5_EXTI_IRQn EXTI4_15_IRQn
#define FAULT_3V3BC_Pin GPIO_PIN_10
#define FAULT_3V3BC_GPIO_Port GPIOE
#define FAULT_3V3BC_EXTI_IRQn EXTI4_15_IRQn
#define EN_3V3BC_Pin GPIO_PIN_11
#define EN_3V3BC_GPIO_Port GPIOE
#define EN_5VBC_Pin GPIO_PIN_13
#define EN_5VBC_GPIO_Port GPIOE
#define FAULT_5VBC_Pin GPIO_PIN_14
#define FAULT_5VBC_GPIO_Port GPIOE
#define FAULT_5VBC_EXTI_IRQn EXTI4_15_IRQn
#define EN_7_4VCH_Pin GPIO_PIN_10
#define EN_7_4VCH_GPIO_Port GPIOB
#define FAULT_7_4VCH_Pin GPIO_PIN_11
#define FAULT_7_4VCH_GPIO_Port GPIOB
#define FAULT_7_4VCH_EXTI_IRQn EXTI4_15_IRQn
#define CB_EN_Pin GPIO_PIN_11
#define CB_EN_GPIO_Port GPIOD
#define PG_5V_Pin GPIO_PIN_13
#define PG_5V_GPIO_Port GPIOD
#define PG_5V_EXTI_IRQn EXTI4_15_IRQn
#define PG_3V3_Pin GPIO_PIN_6
#define PG_3V3_GPIO_Port GPIOC
#define PG_3V3_EXTI_IRQn EXTI4_15_IRQn
#define HEAT_BATL_Pin GPIO_PIN_7
#define HEAT_BATL_GPIO_Port GPIOC
#define HEAT_BATR_Pin GPIO_PIN_8
#define HEAT_BATR_GPIO_Port GPIOC
#define S_SDA_Pin GPIO_PIN_9
#define S_SDA_GPIO_Port GPIOC
#define S_SCL_Pin GPIO_PIN_8
#define S_SCL_GPIO_Port GPIOA
#define EN_5V_Pin GPIO_PIN_9
#define EN_5V_GPIO_Port GPIOA
#define EN_3V3_Pin GPIO_PIN_10
#define EN_3V3_GPIO_Port GPIOA
#define INT_IN_Pin GPIO_PIN_15
#define INT_IN_GPIO_Port GPIOA
#define INT_GPIO_Pin GPIO_PIN_12
#define INT_GPIO_GPIO_Port GPIOC
#define BUSY_IN_Pin GPIO_PIN_0
#define BUSY_IN_GPIO_Port GPIOD
#define BUSY_IN_EXTI_IRQn EXTI0_1_IRQn
#define BUSY_GPIO_Pin GPIO_PIN_1
#define BUSY_GPIO_GPIO_Port GPIOD
#define FAULT_CH1_Pin GPIO_PIN_2
#define FAULT_CH1_GPIO_Port GPIOD
#define FAULT_CH1_EXTI_IRQn EXTI2_3_IRQn
#define EN_CH1_Pin GPIO_PIN_3
#define EN_CH1_GPIO_Port GPIOD
#define RST_CH1_Pin GPIO_PIN_4
#define RST_CH1_GPIO_Port GPIOD
#define FAULT_CH2_Pin GPIO_PIN_5
#define FAULT_CH2_GPIO_Port GPIOD
#define FAULT_CH2_EXTI_IRQn EXTI4_15_IRQn
#define EN_CH2_Pin GPIO_PIN_6
#define EN_CH2_GPIO_Port GPIOD
#define RST_CH2_Pin GPIO_PIN_7
#define RST_CH2_GPIO_Port GPIOD
#define M_SCL_Pin GPIO_PIN_8
#define M_SCL_GPIO_Port GPIOB
#define M_SDA_Pin GPIO_PIN_9
#define M_SDA_GPIO_Port GPIOB
#define FAULT_CH3_Pin GPIO_PIN_1
#define FAULT_CH3_GPIO_Port GPIOE
#define FAULT_CH3_EXTI_IRQn EXTI0_1_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
