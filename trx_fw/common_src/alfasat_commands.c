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
//
//

#include "unisat_commands.h"
#include "unisat_protocol.h"
#include "board_scheduler.h"
#include "board_commands.h"
#include "unisat_types.h"
#include "board_sx127x.h"
#include "board_sim8xx.h"
#include "cmsis_os.h"
#include "sim8xx.h"
#include "hw.h"

extern struct sim8xx_data* simcom;

struct unisat_command cmd_list[COMMANDS_TOTAL_COUNT] =
{
    { .cmd_id = COMMAND_PING,                   .pFunc = command_Echo,                  .len = 0  },
    { .cmd_id = COMMAND_RESET_SYSTEM,           .pFunc = command_ResetSystem,           .len = 0  },
    { .cmd_id = COMMAND_GET_INFORMATION,        .pFunc = command_GetInformation,        .len = 1  },
    { .cmd_id = COMMAND_GET_TIMESTAMP,          .pFunc = command_GetTimestamp,          .len = 0  },
    { .cmd_id = COMMAND_SAVE_RADIO_SETT,        .pFunc = command_SaveRadioSettings,     .len = 0  },
    { .cmd_id = COMMAND_LOAD_RADIO_SETT,        .pFunc = command_LoadRadioSettings,     .len = 0  },
    { .cmd_id = COMMAND_LOAD_DEF_RADIO_SETT,    .pFunc = command_LoadRadioDefSettings,  .len = 0  },
    { .cmd_id = COMMAND_GET_RADIO_SETT,         .pFunc = command_GetRadioSettings,      .len = 0  },
    { .cmd_id = COMMAND_SET_RADIO_MODE,         .pFunc = command_SetRadioMode,          .len = 1  },
    { .cmd_id = COMMAND_SET_RADIO_FREQ,         .pFunc = command_SetRadioFreq,          .len = 4  },
    { .cmd_id = COMMAND_SET_RADIO_POWER,        .pFunc = command_SetRadioPower,         .len = 1  },
    { .cmd_id = COMMAND_SET_RADIO_DATARATE,     .pFunc = command_SetRadioDataRate,      .len = 4  },
    { .cmd_id = COMMAND_SET_RADIO_PREMBLEN,     .pFunc = command_SetRadioPreambleLen,   .len = 2  },
    { .cmd_id = COMMAND_SET_RADIO_CODERATE,     .pFunc = command_SetRadioCodeRate,      .len = 1  },
    { .cmd_id = COMMAND_SET_RADIO_BTIMEOUT,     .pFunc = command_SetRadioBtimeout,      .len = 4  },
    { .cmd_id = COMMAND_GET_LOCATION_SHORT,     .pFunc = command_GetLocationShort,      .len = 0  },
    { .cmd_id = COMMAND_GET_LOCATION_EXTEN,     .pFunc = command_GetLocationExten,      .len = 0  },
    { .cmd_id = COMMAND_SET_SCHEDULER_ACT,      .pFunc = command_SetSchedulerAct,       .len = 2  },
    { .cmd_id = COMMAND_SET_SCHEDULER_PER,      .pFunc = command_SetSchedulerPeriod,    .len = 3  },
    { .cmd_id = COMMAND_SET_SCHEDULER_SET,      .pFunc = command_SetSchedulerSetting,   .len = 14 },
    { .cmd_id = COMMAND_LOAD_DEF_SCHED_SETT,    .pFunc = command_LoadSchedulerSettings, .len = 0  },
    { .cmd_id = COMMAND_SAVE_SCHEDULER_SETT,    .pFunc = command_SaveSchedulerSettings, .len = 0  },
};



bool command_Echo( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    uint8_t* pstart = answer_data;
    
    *answer_data++ = PAYLOAD_ID_PING_ECH0;
    *answer_data++ = 0x00;
    *answer_len = (answer_data - pstart - 1);
    return true;
}

bool command_ResetSystem( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    NVIC_SystemReset();
    return true;
}

bool command_GetInformation( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    uint8_t len, flag = *data;
    
    if( flag == 0 )
    {
        uint32_t uptime = xTaskGetTickCount();
        uptime /= 1000;
        *answer_data++ = PAYLOAD_ID_UPTIME;
        *answer_data++ = (uptime >> 0) & 0xFF;
        *answer_data++ = (uptime >> 8) & 0xFF;
        *answer_data++ = (uptime >> 16) & 0xFF;
        *answer_data++ = (uptime >> 24) & 0xFF;
        *answer_len = sizeof(uint32_t);
        return true;
    }
    else
    {
        // TODO
        *answer_len = len;
    }
    return false;
}

bool command_GetTimestamp( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    uint32_t ts = 0;
    
    getSystemTime(&ts);
    
    if( ts > 0 )
    {
        *answer_data++ = PAYLOAD_ID_TS;
        *answer_data++ = (ts>>0) & 0xFF;
        *answer_data++ = (ts>>8) & 0xFF;
        *answer_data++ = (ts>>16) & 0xFF;
        *answer_data++ = (ts>>24) & 0xFF;
        *answer_len = sizeof(uint32_t);
        return true;
    }
    return false;
}

bool command_SaveRadioSettings( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    if( Radio_SaveSettings() == AF_OK )
    {
        return true;
    }
    return false;
}

bool command_LoadRadioSettings( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    if( Radio_LoadSettings() == AF_OK )
    {
        return true;
    }
    return false;
}

bool command_LoadRadioDefSettings( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    if( Radio_LoadDefaults() == AF_OK )
    {
        return true;
    }
    return false;
}

bool command_GetRadioSettings( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    uint8_t* pstart = answer_data;
    if( Radio_GetMode() != 0xFF )
    {
        *answer_data++ = PAYLOAD_ID_RADIO_SETTINGS;
        *answer_data++ = Radio_GetMode();
        *answer_data++ = Radio_GetPower();
        *answer_data++ = Radio_GetFrequency();
    }
    else
    {
        *answer_data++ = PAYLOAD_ID_RADIO_SETTINGS_EXT;
        *answer_data++ = Radio_GetMode();
        *answer_data++ = Radio_GetPower();
        *answer_data++ = Radio_GetFrequency();
        *answer_data++ = Radio_GetDataRate();
        *answer_data++ = Radio_GetCodeRate();
        *answer_data++ = Radio_GetPreambleLen();
        *answer_data++ = Radio_GetByteTimeout();
    }

    *answer_len = (answer_data - pstart - 1);

    return true;
}

bool command_SetRadioMode( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    uint8_t mode = 0;
    
    mode = *data++;
    
    if( (Radio_SetMode(mode) == AF_OK) &&
        (Radio_ReInit( true, true ) == AF_OK) )
    {
        return true;
    }
    return false;
}

bool command_SetRadioFreq( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    uint32_t freq = 0;
    
    freq = *data++;
    freq |= (*data++)<<8;
    freq |= (*data++)<<16;
    freq |= (*data++)<<24;
    
    if( Radio_SetFrequency(freq) == AF_OK )
    {
        return true;
    }
    return false;
}

bool command_SetRadioPower( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    if( Radio_SetPower(*data) == AF_OK )
    {
        return true;
    }
    return false;
}

bool command_SetRadioDataRate( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    uint32_t datarate = 0;
    
    datarate = *data++;
    datarate |= (*data++)<<8;
    datarate |= (*data++)<<16;
    datarate |= (*data++)<<24;
    
    if( Radio_SetDataRate(datarate) == AF_OK )
    {
        return true;
    }
    return false;
}

bool command_SetRadioPreambleLen( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    uint32_t plen = 0;
    
    plen = *data++;
    plen |= (*data++)<<8;
    plen |= (*data++)<<16;
    plen |= (*data++)<<24;
    
    if( Radio_SetPreambleLen(plen) == AF_OK )
    {
        return true;
    }
    return false;
}

bool command_SetRadioCodeRate( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    if( Radio_SetCodeRate(*data) == AF_OK )
    {
        return true;
    }
    return false;
}

bool command_SetRadioBtimeout( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    uint32_t byte_timeout = 0;
    
    byte_timeout = *data++;
    byte_timeout |= (*data++)<<8;
    
    if( Radio_SetByteTimeout(byte_timeout) == AF_OK )
    {
        return true;
    }
    return false;
}

bool command_GetLocationShort( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    if( simcom != NULL )
    {
        struct gps_data_var1 resp;
        resp.lon = simcom->gps_longitude;
        resp.lat = simcom->gps_latitude;
        resp.alt = (uint16_t) simcom->gps_altitude;
        
        *answer_data++ = PAYLOAD_ID_LOCATION_SHORT;
        memcpy( answer_data, (uint8_t*) &resp, sizeof(struct gps_data_var1));
        *answer_len = sizeof(struct gps_data_var1);
        return true;
    }
    return false;
}

bool command_GetLocationExten( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    if( simcom != NULL )
    {
        struct gps_data_var2 resp;
        resp.lon = simcom->gps_longitude;
        resp.lat = simcom->gps_latitude;
        resp.alt = (uint16_t) simcom->gps_altitude;
        resp.spd = (uint8_t) ((simcom->gps_speed_kph * 1000.0)/3600.0); // км/ч -> м/с
        resp.crs = (uint16_t) (simcom->gps_coarse * 10);
        
        *answer_data++ = PAYLOAD_ID_NAVIGATION;
        memcpy( answer_data, (uint8_t*) &resp, sizeof(struct gps_data_var2));
        *answer_len = sizeof(struct gps_data_var2);
        return true;
    }
    return false;
}

bool command_SetSchedulerAct( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    uint8_t ch, act;
    
    ch = *data++;
    act = *data++;
    
    if( Scheduler_SetAct(ch, act) == AF_OK )
    {
        return true;
    }
    return false;
}

bool command_SetSchedulerPeriod( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    uint8_t ch;
    uint16_t period;
    
    ch = *data++;
    period = *data++;
    period |= (*data++)<<8;
    
    if( Scheduler_SetPeriod(ch, period) == AF_OK )
    {
        return true;
    }
    return false;
}

bool command_SetSchedulerSetting( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    uint8_t ch, act, cond, to_node;
    uint16_t period;
    uint32_t mask, pFunc;
    
    ch = *data++;
    act = *data++;
    cond = *data++;
    period = *data++;
    period |= (*data++)<<8;
    
    mask = *data++;
    mask |= (*data++)<<8;
    mask |= (*data++)<<16;
    mask |= (*data++)<<24;
    
    pFunc = *data++;
    pFunc |= (*data++)<<8;
    pFunc |= (*data++)<<16;
    pFunc |= (*data++)<<24;
    
    if( Scheduler_SetSettings(ch, act, cond, to_node, period, mask, pFunc) == AF_OK )
    {
        return true;
    }
    return false;
}

bool command_LoadSchedulerSettings( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    if( Scheduler_LoadDefaults() == AF_OK )
    {
        return true;
    }
    return false;
}

bool command_SaveSchedulerSettings( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len )
{
    if( Scheduler_SaveSettings() == AF_OK )
    {
        return true;
    }
    return false;
}
