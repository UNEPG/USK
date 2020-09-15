/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: contains hardaware configuration Macros and Constants

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
/**
  ******************************************************************************
  * @file    stm32l0xx_hw_conf.h
  * @author  MCD Application Team
  * @brief   contains hardaware configuration Macros and Constants
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_CONF_L0_H__
#define __HW_CONF_L0_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

//#define RADIO_DIO_4
//#define RADIO_DIO_5

/* --------------------------- RADIO HW definition -------------------------------- */

#define RADIO_RESET_PORT                          GPIOA
#define RADIO_RESET_PIN                           GPIO_PIN_6

#define RADIO_MOSI_PORT                           GPIOB
#define RADIO_MOSI_PIN                            GPIO_PIN_15

#define RADIO_MISO_PORT                           GPIOB
#define RADIO_MISO_PIN                            GPIO_PIN_14

#define RADIO_SCLK_PORT                           GPIOB
#define RADIO_SCLK_PIN                            GPIO_PIN_13

#define RADIO_NSS_PORT                            GPIOC
#define RADIO_NSS_PIN                             GPIO_PIN_6

#define RADIO_DIO_0_PORT                          GPIOA
#define RADIO_DIO_0_PIN                           GPIO_PIN_7

#define RADIO_DIO_1_PORT                          GPIOC
#define RADIO_DIO_1_PIN                           GPIO_PIN_4

#define RADIO_DIO_2_PORT                          GPIOC
#define RADIO_DIO_2_PIN                           GPIO_PIN_5

#define RADIO_DIO_3_PORT                          GPIOB
#define RADIO_DIO_3_PIN                           GPIO_PIN_0

#ifdef RADIO_DIO_4
#define RADIO_DIO_4_PORT                          GPIOA
#define RADIO_DIO_4_PIN                           GPIO_PIN_9
#endif

#ifdef RADIO_DIO_5
#define RADIO_DIO_5_PORT                          GPIOC
#define RADIO_DIO_5_PIN                           GPIO_PIN_7
#endif



#define RADIO_ANT_SWITCH_PORT                     0
#define RADIO_ANT_SWITCH_PIN                      0

/*  SPI MACRO redefinition */

#define RADIO_SPI_CLK_ENABLE()                  __HAL_RCC_SPI2_CLK_ENABLE()

#define RADIO_SPI_AF                            GPIO_AF0_SPI2

/* ADC MACRO redefinition */

#ifdef USE_STM32L0XX_NUCLEO
#define ADC_READ_CHANNEL                ADC_CHANNEL_4
#define ADCCLK_ENABLE()                 __HAL_RCC_ADC1_CLK_ENABLE() ;
#define ADCCLK_DISABLE()                __HAL_RCC_ADC1_CLK_DISABLE() ;
#endif


/* --------------------------- RTC HW definition -------------------------------- */

#define RTC_OUTPUT                              DBG_RTC_OUTPUT

#define RTC_Alarm_IRQn                          RTC_IRQn

/* --------------------------- RS485 HW definition -------------------------------*/
#define RS485_USARTx                            USART1
#define RS485_USARTx_CLK_ENABLE()               __USART1_CLK_ENABLE();
#define RS485_USARTx_RX_GPIO_CLK_ENABLE()       __GPIOA_CLK_ENABLE()
#define RS485_USARTx_TX_GPIO_CLK_ENABLE()       __GPIOA_CLK_ENABLE()
#define RS485_DMAx_CLK_ENABLE()                 __HAL_RCC_DMA1_CLK_ENABLE()

#define RS485_USARTx_FORCE_RESET()              __USART1_FORCE_RESET()
#define RS485_USARTx_RELEASE_RESET()            __USART1_RELEASE_RESET()

#define RS485_USARTx_TX_PIN                     GPIO_PIN_9
#define RS485_USARTx_TX_GPIO_PORT               GPIOA
#define RS485_USARTx_TX_AF                      GPIO_AF4_USART1
#define RS485_USARTx_RX_PIN                     GPIO_PIN_10
#define RS485_USARTx_RX_GPIO_PORT               GPIOA
#define RS485_USARTx_RX_AF                      GPIO_AF4_USART1

/* Definition for USARTx's NVIC */
#define RS485_USARTx_IRQn                       USART1_IRQn
#define RS485_USARTx_IRQHandler                 USART1_IRQHandler

/* Definition for USARTx's DMA */
#define RS485_USARTx_TX_DMA_CHANNEL             DMA1_Channel2
#define RS485_USARTx_RX_DMA_CHANNEL             DMA1_Channel3

/* Definition for USARTx's DMA Request */
#define RS485_USARTx_TX_DMA_REQUEST             DMA_REQUEST_3
#define RS485_USARTx_RX_DMA_REQUEST             DMA_REQUEST_3

/* Definition for USARTx's NVIC */
#define RS485_USARTx_DMA_RXTX_IRQn              DMA1_Channel2_3_IRQn
#define RS485_USARTx_DMA_RXTX_IRQHandler        DMA1_Channel2_3_IRQHandler

#define RS485_USARTx_Priority                   1
#define RS485_USARTx_DMA_Priority               1

/* Definition for RS485 transceiver*/
#define RS485_DE_GPIO                           GPIOC
#define RS485_DE_PIN                            GPIO_PIN_8
#define RS485_TRANSCEIVER_SET_AS_TRANSMITTER()  RS485_DE_GPIO->BSRR = RS485_DE_PIN
#define RS485_TRANSCEIVER_SET_AS_RECEIVER()     RS485_DE_GPIO->BRR  = RS485_DE_PIN

/* Definition for RS485 timeout timer*/
#define RS485_TIMER_CLK_ENA()                   __TIM6_CLK_ENABLE()

#define RS485_TIMx                              TIM6

#define RS485_TIMx_IRQn                         TIM6_DAC_IRQn
#define RS485_TIMx_IRQHandler                   TIM6_DAC_IRQHandler
#define RS485_TIMx_IRQ_Priority                 2


/* --------------------------- GSM HW definition -------------------------------*/
#define GSM_PWRKEY_PIN                          GPIO_PIN_15
#define GSM_PWRKEY_PORT                         GPIOA
#define GSM_PWRKEY_GPIO_CLK_ENABLE()            __GPIOA_CLK_ENABLE()

#define GSM_RI_PIN                              GPIO_PIN_3
#define GSM_RI_PORT                             GPIOB
//#define GSM_RI_AF                               
#define GSM_RI_GPIO_CLK_ENABLE()                __GPIOB_CLK_ENABLE()

#define GSM_USARTx                              USART5
#define GSM_USARTx_CLK_ENABLE()                 SET_BIT(RCC->APB1ENR, (RCC_APB1ENR_USART5EN))
#define GSM_USARTx_RX_GPIO_CLK_ENABLE()         __GPIOD_CLK_ENABLE()
#define GSM_USARTx_TX_GPIO_CLK_ENABLE()         __GPIOC_CLK_ENABLE()
//#define GSM_DMAx_CLK_ENABLE()                 __HAL_RCC_DMA1_CLK_ENABLE()

#define GSM_USARTx_FORCE_RESET()                SET_BIT   (RCC->APB1RSTR, (RCC_APB1RSTR_USART5RST))
#define GSM_USARTx_RELEASE_RESET()              CLEAR_BIT (RCC->APB1RSTR, (RCC_APB1RSTR_USART5RST))

#define GSM_USARTx_TX_PIN                       GPIO_PIN_12
#define GSM_USARTx_TX_GPIO_PORT                 GPIOC
#define GSM_USARTx_TX_AF                        GPIO_AF2_USART5
#define GSM_USARTx_RX_PIN                       GPIO_PIN_2
#define GSM_USARTx_RX_GPIO_PORT                 GPIOD
#define GSM_USARTx_RX_AF                        GPIO_AF6_USART5

/* Definition for USARTx's NVIC */
#define GSM_USARTx_IRQn                         USART4_5_IRQn
#define GSM_USARTx_IRQHandler                   USART4_5_IRQHandler

/* Definition for USARTx's DMA */
//#define GSM_USARTx_TX_DMA_CHANNEL             DMA1_Channel2

/* Definition for USARTx's DMA Request */
//#define GSM_USARTx_TX_DMA_REQUEST             DMA_REQUEST_4

/* Definition for USARTx's NVIC */
//#define GSM_USARTx_DMA_TX_IRQn                DMA1_Channel2_3_IRQn
//#define GSM_USARTx_DMA_TX_IRQHandler          DMA1_Channel2_3_IRQHandler

#define GSM_USARTx_Priority                     0
//#define GSM_USARTx_DMA_Priority               0


/* --------------------------- GPS HW definition -------------------------------*/
#define GPS_PWREN_PIN                           GPIO_PIN_6
#define GPS_PWREN_PORT                          GPIOB
#define GPS_PWREN_GPIO_CLK_ENABLE()             __GPIOB_CLK_ENABLE()

#define GPS_1PPS_PIN                            GPIO_PIN_5
#define GPS_1PPS_PORT                           GPIOB
#define GPS_1PPS_GPIO_CLK_ENABLE()              __GPIOB_CLK_ENABLE()

#define GPS_1PPS_LED_PIN                        0
#define GPS_1PPS_LED_PORT                       0
#define GPS_1PPS_LED_GPIO_CLK_ENABLE()          __nop()

#define GPS_USARTx                              USART4
#define GPS_USARTx_CLK_ENABLE()                 SET_BIT(RCC->APB1ENR, (RCC_APB1ENR_USART4EN))
#define GPS_USARTx_RX_GPIO_CLK_ENABLE()         __GPIOC_CLK_ENABLE()
#define GPS_USARTx_TX_GPIO_CLK_ENABLE()         __GPIOC_CLK_ENABLE()
//#define GPS_DMAx_CLK_ENABLE()                 __HAL_RCC_DMA1_CLK_ENABLE()

#define GPS_USARTx_FORCE_RESET()                SET_BIT   (RCC->APB1RSTR, (RCC_APB1RSTR_USART4RST))
#define GPS_USARTx_RELEASE_RESET()              CLEAR_BIT (RCC->APB1RSTR, (RCC_APB1RSTR_USART4RST))

#define GPS_USARTx_TX_PIN                       GPIO_PIN_10
#define GPS_USARTx_TX_GPIO_PORT                 GPIOC
#define GPS_USARTx_TX_AF                        GPIO_AF6_USART4
#define GPS_USARTx_RX_PIN                       GPIO_PIN_11
#define GPS_USARTx_RX_GPIO_PORT                 GPIOC
#define GPS_USARTx_RX_AF                        GPIO_AF6_USART4

/* Definition for USARTx's NVIC */
#define GPS_USARTx_IRQn                         USART4_5_IRQn
#define GPS_USARTx_IRQHandler                   USART4_5_IRQHandler

/* Definition for USARTx's TX DMA */
#define GPS_USARTx_TX_DMA_CHANNEL               DMA1_Channel3
#define GPS_USARTx_TX_DMA_REQUEST               DMA_REQUEST_12
/* Definition for USARTx's RX DMA */
#define GPS_USARTx_RX_DMA_CHANNEL               DMA1_Channel2
#define GPS_USARTx_RX_DMA_REQUEST               DMA_REQUEST_12

/* Definition for USARTx's NVIC */
#define GPS_USARTx_DMA_TX_IRQn                  DMA1_Channel2_3_IRQn
#define GPS_USARTx_DMA_TX_IRQHandler            DMA1_Channel2_3_IRQHandler

#define GPS_USARTx_Priority                     1
//#define GPS_USARTx_DMA_Priority               0


/* --------------------------- Other HW definition -------------------------------*/
#define LED_PORT                                GPIOA
#define LED_PIN                                 GPIO_PIN_2
#define LED_GPIO_CLK_ENABLE()                   __GPIOA_CLK_ENABLE()

#define LED_Toggle()                            LED_PORT->ODR ^= LED_PIN
#define LED_On()                                LED_PORT->BSRR = LED_PIN
#define LED_Off()                               LED_PORT->BRR  = LED_PIN

#ifdef __cplusplus
}
#endif

#endif /* __HW_CONF_L0_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
