#ifndef __INC_unisat_COMMANDS_H__
#define __INC_unisat_COMMANDS_H__

#include "unisat_types.h"
#include "unisat_protocol.h"


enum NODE_PAYLOAD_IDS
{
    PAYLOAD_ID_PING_ECH0 = 0,
    PAYLOAD_ID_UPTIME,
    PAYLOAD_ID_TS,
    PAYLOAD_ID_RADIO_SETTINGS,
    PAYLOAD_ID_RADIO_SETTINGS_EXT,
    PAYLOAD_ID_LOCATION_SHORT,
    PAYLOAD_ID_NAVIGATION,
};

enum NODE_COMMAND_IDS
{
    COMMAND_PING = 0,
    COMMAND_RESET_SYSTEM,
    COMMAND_GET_INFORMATION,
    COMMAND_GET_TIMESTAMP,
    COMMAND_SAVE_RADIO_SETT,
    COMMAND_LOAD_RADIO_SETT,
    COMMAND_LOAD_DEF_RADIO_SETT,
    COMMAND_GET_RADIO_SETT,
    COMMAND_SET_RADIO_MODE,
    COMMAND_SET_RADIO_POWER,
    COMMAND_SET_RADIO_FREQ,
    COMMAND_SET_RADIO_DATARATE,
    COMMAND_SET_RADIO_PREMBLEN,
    COMMAND_SET_RADIO_CODERATE,
    COMMAND_SET_RADIO_BTIMEOUT,
    COMMAND_GET_LOCATION_SHORT,
    COMMAND_GET_LOCATION_EXTEN,
    COMMAND_SET_SCHEDULER_ACT,
    COMMAND_SET_SCHEDULER_PER,
    COMMAND_SET_SCHEDULER_SET,
    COMMAND_LOAD_DEF_SCHED_SETT,
    COMMAND_SAVE_SCHEDULER_SETT,
    //
    COMMANDS_TOTAL_COUNT
};

struct unisat_command
{
    enum NODE_COMMAND_IDS cmd_id;
    uint8_t len;
    bool (*pFunc)(uint8_t*, uint8_t*, uint8_t*);
};

// Короткие данные нацигации
struct __attribute__ ((__packed__)) gps_data_var1
{
    float lon;
    float lat;
    uint16_t alt;
};

// Расширенные данные навигации
struct __attribute__ ((__packed__)) gps_data_var2
{
    float lon;
    float lat;
    uint16_t alt;
    uint16_t spd;
    uint16_t crs;
};

extern struct unisat_command cmd_list[COMMANDS_TOTAL_COUNT];

//// Board processing commands ////
bool command_Echo( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_ResetSystem( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_GetInformation( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_GetTimestamp( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_SaveRadioSettings( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_LoadRadioSettings( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_LoadRadioDefSettings( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_GetRadioSettings( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_SetRadioMode( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_SetRadioFreq( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_SetRadioPower( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_SetRadioDataRate( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_SetRadioPreambleLen( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_SetRadioCodeRate( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_SetRadioBtimeout( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_GetLocationShort( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_GetLocationExten( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_SetSchedulerAct( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_SetSchedulerPeriod( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_SetSchedulerSetting( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_LoadSchedulerSettings( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
bool command_SaveSchedulerSettings( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );

#endif
