#ifndef __INC_BOARD_SIM8XX_H__
#define __INC_BOARD_SIM8XX_H__

#include "unisat_types.h"
#include <time.h>

#define     SIMCOM_TASK_STACK_SIZE          512
#define     SIMCOM_TASK_PRIORITY            osPriorityNormal

#define     SECONDS_SINCE_1970_TO_2000      946663200
#define     SYSTEM_TIME_CORRECTION_TH       15


typedef enum 
{
    GSM_Init = 0,
    GSM_Register,
    GSM_WaitRegister,
    GSM_OperatorIdent,
    GSM_GettingBalance,
    GSM_Idle,
    GSM_VoiceCalling,
    GSM_WaitResponse,
    GSM_SmsSending,
    GSM_SmsWaitResponse,
    GSM_LowPower
    
} GSM_States;

typedef enum 
{
    GPS_STATE_Init = 0,
    GPS_STATE_Activation,
    GPS_STATE_WaitActivation,
    GPS_STATE_DataWaiting,
    GPS_STATE_DataParcing,
    GPS_STATE_LowPower
    
} GPS_States;

// Задача RTOS
void SimcomTask(void const *arguments);

// Методы
bool SIM8xx_checkSystemTimeAndGPS(bool force_update);
bool SIM8xx_getGpsTime( struct tm* pTime, time_t* ts );

#endif
