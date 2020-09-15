#ifndef __INC_unisat_PROTOCOL_H__
#define __INC_unisat_PROTOCOL_H__

#include "unisat_types.h"

#ifdef TRX_BOARD
    #define THIS_BOARD_ADDRESS              NODE_TRX
#elif defined GS_BOARD
    #define THIS_BOARD_ADDRESS              NODE_GS_BOARD
    #define THIS_AREA_MIN_ADDRESS           NODE_GS_BOARD    // min
    #define THIS_AREA_MAX_ADDRESS           NODE_GS_MAX
#elif defined TOP_BOARD
    #define CSP_THIS_BOARD_ADDRESS          NODE_TOP
#elif defined EPS_BOARD
    #define CSP_THIS_BOARD_ADDRESS          NODE_EPS
#endif

#ifndef THIS_AREA_MAX_ADDRESS
    #define THIS_AREA_MAX_ADDRESS           NODE_MAX
#endif
#ifndef THIS_AREA_MIN_ADDRESS
    #define THIS_AREA_MIN_ADDRESS           NODE_OBC
#endif

#if defined(TRX_BOARD) || defined(GS_BOARD)
    #define RADIO_DEFAULT_ID                (0x01)
#endif

#define AFBUS_BUS_MAX_LEN                   (256)
#define AFBUS_RADIO_MAX_LEN                 (64)

#define HEADER_PRIORITY_Pos                 (15)
#define HEADER_PRIORITY_Msk                 (0x01)
#define HEADER_PRIORITY                     (HEADER_PRIORITY_Msk << HEADER_PRIORITY_Pos)

#define HEADER_SOURCE_NODE_Pos              (4)
#define HEADER_SOURCE_NODE_Msk              (0x0F)
#define HEADER_SOURCE_NODE                  (HEADER_SOURCE_NODE_Msk << HEADER_SOURCE_NODE_Pos)

#define HEADER_DESTINATION_NODE_Pos         (0)
#define HEADER_DESTINATION_NODE_Msk         (0x0F)
#define HEADER_DESTINATION_NODE             (HEADER_DESTINATION_NODE_Msk << HEADER_DESTINATION_NODE_Pos)


#define HEADER_FIELD_LEN                    (2)
#define LEN_FIELD_LEN                       (1)
#define CRC_FIELD_LEN                       (2)
#define PACKET_START_FIELDS_LEN             (HEADER_FIELD_LEN + LEN_FIELD_LEN)
#define PACKET_BASIC_FIELDS_LEN             (HEADER_FIELD_LEN + LEN_FIELD_LEN + CRC_FIELD_LEN)
#define RS485_PACKET_MINIMAL_LEN            (PACKET_BASIC_FIELDS_LEN)
#define RADIO_PACKET_MINIMAL_LEN            (PACKET_BASIC_FIELDS_LEN - CRC_FIELD_LEN + 1) // +1 RadioID
#define LEN_FIELD_POS                       (2)
#define PAYLOAD_FIELD_POS                   (3)

#define unisat_PROTOCOL_COMMANDS_MASK      (0x80) //(1000 0000)
#define unisat_PROTOCOL_ID_LEN             (0x01)


enum NODE_ADRESSES
{
    NODE_OBC = 0,  // 0
    NODE_EPS = 1,  // 1
    NODE_TRX = 2,  // 2
    NODE_TOP = 3,  // 3
    NODE_SNB = 4,  // 4
    //////////////////////
    NODE_MAX = 9,
//---------------------------//
    NODE_GS_BOARD = 12,      // (0x0C)
    NODE_GROUNDSTATION = 13, // (0x0D)
    //////////////////////////
    NODE_GS_MAX = 15
    
};

struct unisat_packet_struct
{
    uint16_t header;
    uint8_t len;
};

struct unisat_header
{
    uint8_t source_node;
    uint8_t destination_node;
    //...
    uint8_t priority;
    /************************/
    uint8_t len;
};

/////////////////////////////////////////////////////////////////////////////
void GetunisatHeader( uint8_t* pData, struct unisat_header* header);
void SetunisatHeader( uint8_t* pData, struct unisat_header* header);

#endif
