//           _      ______       _____      _______ 
//     /\   | |    |  ____/\    / ____|  /\|__   __|
//    /  \  | |    | |__ /  \  | (___   /  \  | |   
//   / /\ \ | |    |  __/ /\ \  \___ \ / /\ \ | |   
//  / ____ \| |____| | / ____ \ ____) / ____ \| |   
// /_/    \_\______|_|/_/    \_\_____/_/    \_\_|   
//                                                  
//
// Author: Nursultan Uzbekov
// Date:   17.06.2020
//

#include "hw.h"
#include "board_scheduler.h"
#include "unisat_utils.h"
#include "unisat_protocol.h"
#include "board_eeprom.h"
#include "unisat_commands.h"
#include "board_rs485.h"
#include "board_sx127x.h"
#include <stdlib.h>

//**** Defines ****//
#define SCHEDULER_PARAMS_CNT        ((sizeof(SchedulerParams)) / (sizeof(SchedulerParams[0])))

//**** Typdef/enumes ****//
enum SCHEDULER_CONDITION
{
    SCHEDULER_CONDITION_PERIODIC = 0,
    SCHEDULER_CONDITION_UNTIL_ELAPSED,
    SCHEDULER_CONDITION_ALTITUDE_ABOVE,
    SCHEDULER_CONDITION_ALTITUDE_BELOW,
    SCHEDULER_CONDITION_COUNT
};

typedef void (*pUserFunction)( void );

struct __attribute__ ((__packed__)) scheduler_channel
{
    bool                        enabled;
    enum SCHEDULER_CONDITION    condition;
    uint8_t                     to_node;
    uint16_t                    period;
    uint32_t                    mask;
    pUserFunction               pFunc;
};

struct __attribute__ ((__packed__)) scheduler_settings
{
    struct scheduler_channel sched_chan[SCHEDULER_CHANNELS_COUNT];
    
    uint8_t padding[ 2 ];  // выравнивание до блока 
    
    uint16_t crc;
};

struct __attribute__ ((__packed__)) sender_info
{
    uint8_t* data;
    uint8_t* data_start;
    uint8_t* pac_len;
    uint8_t* p_crc;
    uint8_t* pay_len;
    uint8_t data_len;
    uint8_t* part_list;
    uint8_t  part_cnt;
    int8_t to_node;
};

struct scheduler_param
{
    bool (*pParamFunc)( uint8_t* data, uint8_t* answer_data, uint8_t* answer_len );
};

/* Variables */
TimerHandle_t SchedulerTimer;
static struct scheduler_settings SchedulerSettings;
static uint16_t delay[ SCHEDULER_CHANNELS_COUNT ];
static struct sender_info sender = 
{ 
    .data = NULL, .data_len = 0, .to_node = -1
};

static void SchedulerTimer_Callback( TimerHandle_t xTimer );
static void SchedulerSender( struct scheduler_channel* scheduled );
static void SchedulerSenderCheckAndFree( bool freeup );

struct scheduler_param SchedulerParams[] =
{
    /* 00 */    { .pParamFunc = &command_GetTimestamp       },
    /* 01 */    { .pParamFunc = &command_GetLocationShort   },
};

void SchedulerTask(void const * argument)
{
    
    Scheduler_LoadSettings();
    
    SchedulerTimer = xTimerCreate
                 ( "Scheduler",
                   10000,
                   pdTRUE,
                   (void*) 0,
                   SchedulerTimer_Callback );
    
    if ( xTimerStart(SchedulerTimer, 1000 ) != pdPASS ) 
    {
        // TODO:
    }
    
    for( int channel = 0; channel < SCHEDULER_CHANNELS_COUNT; channel++ )
    {
        if( SchedulerSettings.sched_chan[channel].enabled )
        {
            delay[channel] = xTimerGetPeriod( SchedulerTimer );
        }
    }
    
    /* Infinite loop */
    while(1)
    {
        osDelay( 10000 );
    }
}

static void SchedulerTimer_Callback( TimerHandle_t xTimer )
{
    
    // Отнимаем время периода
    uint16_t ms = xTimerGetPeriod( SchedulerTimer );
    for( int channel = 0; channel < SCHEDULER_CHANNELS_COUNT; channel++ )
    {
        if( SchedulerSettings.sched_chan[channel].enabled )
        {
            delay[channel] -= ms;
        }
    }
    
    uint8_t active_cnt = 0x00;
    uint8_t output_cnt = 0x00;
    uint8_t min_dest = 0xFF;
    uint8_t min_dest_pos = 0x00;
    uint8_t dests[SCHEDULER_CHANNELS_COUNT];
    uint8_t sort_dests[SCHEDULER_CHANNELS_COUNT];
    
    // Проверяем какая сработала, и группируем их
    for( int channel = 0; channel < SCHEDULER_CHANNELS_COUNT; channel++ )
    {
        if( SchedulerSettings.sched_chan[channel].enabled &&    // Если активна
            delay[channel] == 0 )                               // Если осталось 0 милисекунд
        {
            // Выполнение запланированной задачи 
            switch( SchedulerSettings.sched_chan[channel].condition )
            {
                /*
                    Периодическая отправка
                */
                case SCHEDULER_CONDITION_PERIODIC:
                    dests[active_cnt] = channel;
                    active_cnt++;
                break;
                
                /*
                    Периодическая отправка, до истечения определенной времени
                */
                case SCHEDULER_CONDITION_UNTIL_ELAPSED:
                    
                break;
                
                /*
                    Периодическая отправка, при условии (высота выше)
                */
                case SCHEDULER_CONDITION_ALTITUDE_ABOVE:
                    
                break;
                
                /*
                    Периодическая отправка, при условии (высота ниже)
                */
                case SCHEDULER_CONDITION_ALTITUDE_BELOW:
                    
                break;
                
                case SCHEDULER_CONDITION_COUNT:
                    
                break;
            }
            
            // Следующий период
              delay[channel] = SchedulerSettings.sched_chan[channel].period * 1000;
        }
    }  
    
    while(output_cnt != active_cnt)
    {
        min_dest = 0xFF;
        min_dest_pos = 0xFF;
        for( int channel = 0; channel < active_cnt; channel++ )
        {
            if( (dests[channel] != 0xFF) && 
                (SchedulerSettings.sched_chan[ dests[channel] ].to_node < min_dest) )
            {
                min_dest = SchedulerSettings.sched_chan[ dests[channel] ].to_node;
                min_dest_pos = channel;
            }
        }
        sort_dests[output_cnt] = dests[min_dest_pos];
        dests[min_dest_pos] = 0xFF;
        output_cnt++;
    }
    
    // Проверяем какая сработала
    for( int i = 0; i < output_cnt; i++ )
    {
        SchedulerSender( &SchedulerSettings.sched_chan[ sort_dests[i] ] );
    }
    
    SchedulerSenderCheckAndFree( true );
    
    // Расчитываем ближайщий период
    uint16_t min_period = 0xFFFF;    // 0xFFFF
    for( int channel = 0; channel < SCHEDULER_CHANNELS_COUNT; channel++ )
    {
        if( ( SchedulerSettings.sched_chan[channel].enabled ) &&
            ( delay[channel] < min_period ) )
        {
            min_period = delay[channel];
        }
    }
    
    if( xTimerChangePeriod( SchedulerTimer, min_period, 0 ) != pdTRUE )
    {
        osDelay( 1000 );
    }
}

static void SchedulerSender( struct scheduler_channel* scheduled )
{
    if( sender.data_start == NULL || sender.to_node != scheduled->to_node )
    {
        struct unisat_header header;
        
        if( sender.data_start == NULL || sender.part_list == NULL )
        {
            free(sender.data_start);
            free(sender.part_list);
            sender.data_start = (uint8_t*) malloc( AFBUS_RADIO_MAX_LEN );
            sender.part_list = (uint8_t*) malloc( AFBUS_RADIO_MAX_LEN / 2 );
            if( sender.data_start == NULL || sender.part_list == NULL)
            {
                return;
            }
        }
        else if( sender.to_node != scheduled->to_node )
        {
            SchedulerSenderCheckAndFree(false);
        }
        
        sender.data = sender.data_start;
        sender.pac_len = sender.data++;
        sender.p_crc = sender.data;
        
        sender.to_node          = scheduled->to_node;
        header.destination_node = scheduled->to_node;
        header.source_node      = THIS_BOARD_ADDRESS;
        SetunisatHeader( sender.data, &header);
        
        sender.data += HEADER_FIELD_LEN;
        sender.pay_len = sender.data;
        sender.data += LEN_FIELD_LEN;
        sender.data_len = 0;
        sender.part_cnt = 0;
    }
    
    bool found = false;
    uint8_t temp_len = 0;
    uint32_t mask = scheduled->mask;
    
    for(int p=0; p<SCHEDULER_PARAMS_CNT; p++)
    {
        if( ((mask>>p) & 0x01) == 0 )
        {
            continue;
        }
        
        temp_len = 0;
        found = false;
        
        for( int i=0; i<sender.part_cnt; i++)
        {
            if( sender.part_list[ i ] == p )
            {
                found = true;
            }
        } 
        
        if( !found )
        {
            SchedulerParams[ p ].pParamFunc( NULL, sender.data, &temp_len);
            temp_len++;
            sender.part_list[ sender.part_cnt ] = p;
            sender.part_cnt++;
            sender.data += temp_len;
            sender.data_len += temp_len;
        }
    }
}

static void SchedulerSenderCheckAndFree( bool freeup )
{
    
    if( sender.data_len > 0 )
    {
        *sender.pay_len = sender.data_len;
        sender.data_len += HEADER_FIELD_LEN;
        sender.data_len += LEN_FIELD_LEN;
        
        uint16_t crc = Calc_CRC16(sender.p_crc, sender.data_len);
        *sender.data++ = (crc & 0xff);
        *sender.data++ = ((crc>>8) & 0xff);
        sender.data_len += CRC_FIELD_LEN;
        
        *sender.pac_len = sender.data_len;
        
        if( sender.to_node >= THIS_AREA_MIN_ADDRESS &&
            sender.to_node <= THIS_AREA_MAX_ADDRESS )
        {
            AFBus_PushTxQueue( sender.data_start );
        }
        else
        {
            Radio_PushTxQueue( sender.data_start );
        }
        
        sender.data_len = 0;
    }
    
    if( freeup )
    {
        free(sender.data_start);
        free(sender.part_list);
        sender.data_start = NULL;
        sender.part_list = NULL;
    }
}
//**

AF_Status Scheduler_LoadSettings(void)
{
    
    if( !EEPROM_isValidBlock( EEPROM_SCHEDULER_SETTING_ADDR, sizeof(SchedulerSettings) ) )
    {
        Scheduler_LoadDefaults();
        
        SchedulerSettings.crc = Calc_CRC16( (uint8_t*) &SchedulerSettings, (sizeof(SchedulerSettings) - sizeof(SchedulerSettings.crc)) );
        
        EEPROM_writeBlock( EEPROM_SCHEDULER_SETTING_ADDR, (uint8_t*)&SchedulerSettings, sizeof(SchedulerSettings) );
    }
    else
    {
        EEPROM_readBlock( EEPROM_SCHEDULER_SETTING_ADDR, (uint8_t*)&SchedulerSettings, sizeof(SchedulerSettings) );
    }
    return AF_OK;
    
}

AF_Status Scheduler_SaveSettings(void)
{
    SchedulerSettings.crc = Calc_CRC16( (uint8_t*) &SchedulerSettings, (sizeof(SchedulerSettings) - sizeof(SchedulerSettings.crc)) );
    
    if( EEPROM_writeBlock( EEPROM_RADIO_SETTINGS_ADDRESS, (uint8_t*)&SchedulerSettings, sizeof(SchedulerSettings) ) )
    {
        return AF_OK;
    }
    return AF_ERROR;
}

AF_Status Scheduler_LoadDefaults( void )
{
    
    SchedulerSettings.sched_chan[0].enabled = true;
    SchedulerSettings.sched_chan[0].condition = SCHEDULER_CONDITION_PERIODIC;
    SchedulerSettings.sched_chan[0].to_node = NODE_GROUNDSTATION;
    SchedulerSettings.sched_chan[0].period = 10;
    SchedulerSettings.sched_chan[0].mask = 0x03;
    SchedulerSettings.sched_chan[0].pFunc = NULL;
    
    SchedulerSettings.sched_chan[1].enabled = true;
    SchedulerSettings.sched_chan[1].condition = SCHEDULER_CONDITION_PERIODIC;
    SchedulerSettings.sched_chan[1].to_node = NODE_GROUNDSTATION;
    SchedulerSettings.sched_chan[1].period = 20;
    SchedulerSettings.sched_chan[1].mask = 0x03;
    SchedulerSettings.sched_chan[1].pFunc = NULL;
    
    SchedulerSettings.sched_chan[2].enabled = true;
    SchedulerSettings.sched_chan[2].condition = SCHEDULER_CONDITION_PERIODIC;
    SchedulerSettings.sched_chan[2].to_node = NODE_GROUNDSTATION;
    SchedulerSettings.sched_chan[2].period = 30;
    SchedulerSettings.sched_chan[2].mask = 0x03;
    SchedulerSettings.sched_chan[2].pFunc = NULL;
    
    return AF_OK;
}

AF_Status Scheduler_SetAct( uint8_t ch, uint8_t act )
{
    if( (ch >= SCHEDULER_CHANNELS_COUNT) || (act > 1) )
    {
        return AF_ERROR;
    }
    
    SchedulerSettings.sched_chan[ch].enabled = (bool) act;
    return AF_OK;
}

AF_Status Scheduler_SetPeriod( uint8_t ch, uint16_t period )
{
    if( (ch >= SCHEDULER_CHANNELS_COUNT) || 
        (period > 3600) || (period == 0) )
    {
        return AF_ERROR;
    }
    
    SchedulerSettings.sched_chan[ch].period = period;
    return AF_OK;
}

AF_Status Scheduler_SetSettings( uint8_t ch, uint8_t act, uint8_t cond, uint8_t to_node, uint16_t period, uint16_t mask, uint32_t pFunc )
{
    if( (ch >= SCHEDULER_CHANNELS_COUNT) || 
        (act > 1) || 
        (cond >= SCHEDULER_CONDITION_COUNT) || 
        (to_node > NODE_GS_MAX) || 
        (period > 3600) || (period == 0) )
    {
        return AF_ERROR;
    }
    
    SchedulerSettings.sched_chan[ch].enabled = (bool) act;
    SchedulerSettings.sched_chan[ch].condition = (enum SCHEDULER_CONDITION) cond;
    SchedulerSettings.sched_chan[ch].to_node = to_node;
    SchedulerSettings.sched_chan[ch].period = period;
    SchedulerSettings.sched_chan[ch].mask = mask;
    SchedulerSettings.sched_chan[ch].pFunc = (pUserFunction) pFunc;
    
    return AF_OK;
}
