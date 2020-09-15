/*!
 * \file      main.c
 *
 * \brief     Ping-Pong implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @brief   this is the main!
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

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "rs485.h"
#include "board_sx127x.h"
#include "board_sim8xx.h"
#include "freertos.h"
#include "cmsis_os.h"

/* Timers objects*/

/* Private function prototypes -----------------------------------------------*/
/*    */
void osInit(void);
/**
 * Main application entry point.
 */
int main(void)
{
    
    HAL_Init();

    SystemClock_Config();

    DBG_Init();

    HW_Init();

    /*Disbale Stand-by mode*/
    LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);
    
    osInit();
    osKernelStart();
    
    while (1)
    {
        
    }
}
