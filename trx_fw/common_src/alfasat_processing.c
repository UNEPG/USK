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

#include "unisat_processing.h"
#include "unisat_commands.h"
#include "unisat_protocol.h"
#include "unisat_types.h"
#include "unisat_utils.h"
#include "cmsis_os.h"
#include "rs485.h"

xSemaphoreHandle PacketProcessing_Busy;
static bool packet_processing_initilized = false;



void AFBus_PacketProcessingInit( void )
{
    if( !packet_processing_initilized )
    {
        PacketProcessing_Busy = xSemaphoreCreateBinary();
        xSemaphoreGive(PacketProcessing_Busy);
        packet_processing_initilized = true;
    }
}

AF_Status AFBus_PacketProcessing( uint8_t* data, uint8_t len, uint8_t* pAnswer, uint8_t* lenAnswer, uint8_t crc_calc )
{
    
    if( xSemaphoreTake(PacketProcessing_Busy, 0) != pdTRUE )
    {
        return AF_WAIT;
    }
    
    // unisat Header
    struct unisat_header header;
    GetunisatHeader( data, &header);
    data += HEADER_FIELD_LEN;
    len -= HEADER_FIELD_LEN;
    
    // Data len
    uint8_t dlen  = *data++;
    len--;
    
    if( dlen != len )
    {
        return AF_ERROR;
    }
    
    uint8_t* check_ptr = data;
    uint8_t  check_len = dlen;
    
    // Data checking
    while( check_len != 0 )
    {
        if( (*check_ptr & unisat_PROTOCOL_COMMANDS_MASK) == unisat_PROTOCOL_COMMANDS_MASK )
        {
            
            uint8_t cmd = *check_ptr;
            cmd &= ~(unisat_PROTOCOL_COMMANDS_MASK);
            
            if( (cmd >= COMMANDS_TOTAL_COUNT) ||
                (check_len < cmd_list[ cmd ].len) )
            {
                return AF_ERROR;
            }
            
            check_ptr += (cmd_list[ cmd ].len + 1);
            check_len -= (cmd_list[ cmd ].len + 1);
        }
        else
        {
            //
        }
    }
    
    // Data processing
    uint8_t answer_dlen = 0;
    uint8_t* pAnswer_start = pAnswer;
    uint8_t* pAnswer_crc = pAnswer;
    pAnswer += PAYLOAD_FIELD_POS;
    
    while( dlen != 0 )
    {
        if( (*data & unisat_PROTOCOL_COMMANDS_MASK) == unisat_PROTOCOL_COMMANDS_MASK )
        {
            uint8_t cmd = *data;
            cmd &= ~(unisat_PROTOCOL_COMMANDS_MASK);
            
            uint8_t func_alen = 0;
            data++;                 // передаем только данные
            if( cmd_list[ cmd ].pFunc( data, pAnswer, &func_alen) != true )
            {
                return AF_ERROR;
            }
            
            data += cmd_list[ cmd ].len;
            dlen -= (cmd_list[ cmd ].len + 1);
            func_alen = (func_alen != 0) ? (func_alen + unisat_PROTOCOL_ID_LEN) : 0;   // Добавляем длину ID
            pAnswer += func_alen;
            answer_dlen += func_alen;
            
        }
        else
        {
            
        }
    }
    
    if( answer_dlen != 0 )
    {
        header.destination_node = header.source_node;
        header.source_node      = THIS_BOARD_ADDRESS;
        SetunisatHeader( pAnswer_start, &header);
        pAnswer_start += HEADER_FIELD_LEN;
        
        *pAnswer_start = answer_dlen;
        answer_dlen += PACKET_START_FIELDS_LEN;
        
        if( crc_calc != 0 )
        {
            // Local bus
            uint16_t crc = Calc_CRC16( pAnswer_crc, answer_dlen);
            *pAnswer++ =  crc & 0xff;
            *pAnswer = (crc>>8) & 0xff;
            answer_dlen += CRC_FIELD_LEN;
        }
        
    }
    
    *lenAnswer = answer_dlen;
    
    xSemaphoreGive(PacketProcessing_Busy);
    
    return AF_OK;
    
}
