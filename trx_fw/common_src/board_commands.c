//           _      ______       _____      _______ 
//     /\   | |    |  ____/\    / ____|  /\|__   __|
//    /  \  | |    | |__ /  \  | (___   /  \  | |   
//   / /\ \ | |    |  __/ /\ \  \___ \ / /\ \ | |   
//  / ____ \| |____| | / ____ \ ____) / ____ \| |   
// /_/    \_\______|_|/_/    \_\_____/_/    \_\_|   
//                                                  
//
// Author: Nursultan Uzbekov
// Date:   28.03.2020
//s
//

#include "board_commands.h"
#include "unisat_protocol.h"
#include "unisat_types.h"
#include "hw.h"


bool getSystemTime( time_t* ts )
{
    RTC_HandleTypeDef* handler;
    HW_RTC_GetHandler( &handler );
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    struct tm systime;
    
    HAL_StatusTypeDef dstatus = HAL_RTC_GetDate(handler, &date, RTC_FORMAT_BIN);
    HAL_StatusTypeDef tstatus = HAL_RTC_GetTime(handler, &time, RTC_FORMAT_BIN);
    
    if( dstatus == HAL_OK && tstatus == HAL_OK )
    {
        systime.tm_year = date.Year + 100;
        systime.tm_mon = date.Month - 1;
        systime.tm_wday = 0;
        systime.tm_mday = date.Date;
        
        systime.tm_hour = time.Hours;
        systime.tm_min = time.Minutes;
        systime.tm_sec = time.Seconds;
        
        *ts = mktime(&systime);
        return true;
    }
    return false;
}

bool updateSystemTime(struct tm* pTime)
{
    RTC_HandleTypeDef* handler;
    HW_RTC_GetHandler( &handler );
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    
    date.Year = pTime->tm_year - 100;   // Из-за библиотеки time.h
    date.Month = pTime->tm_mon + 1;     // Из-за библиотеки time.h
    date.WeekDay = 0;
    date.Date = pTime->tm_mday;
    
    time.Hours = pTime->tm_hour;
    time.Minutes = pTime->tm_min;
    time.Seconds = pTime->tm_sec;
    time.SubSeconds = 0;
    time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    time.StoreOperation = RTC_STOREOPERATION_SET;
    
    
    if( IS_RTC_YEAR(date.Year) &&
        IS_RTC_MONTH(date.Month) &&
        IS_RTC_DATE(date.Date) )
    {
        HAL_RTC_SetDate(handler, &date, RTC_FORMAT_BIN);
        HAL_RTC_SetTime(handler, &time, RTC_FORMAT_BIN);
        return true;
    }
    return false;
}
