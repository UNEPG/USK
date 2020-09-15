//           _      ______       _____      _______ 
//     /\   | |    |  ____/\    / ____|  /\|__   __|
//    /  \  | |    | |__ /  \  | (___   /  \  | |   
//   / /\ \ | |    |  __/ /\ \  \___ \ / /\ \ | |   
//  / ____ \| |____| | / ____ \ ____) / ____ \| |   
// /_/    \_\______|_|/_/    \_\_____/_/    \_\_|   
//                                                  
//
// Author: Nursultan Uzbekov
// Date:   22.03.2019
//

#include "unisat_protocol.h"




void GetunisatHeader( uint8_t* pData, struct unisat_header* header)
{
    uint16_t data = (pData[0] | (pData[1]<<8));
    
    header->destination_node = ( data >> HEADER_DESTINATION_NODE_Pos ) & HEADER_DESTINATION_NODE_Msk;
    header->source_node      = ( data >> HEADER_SOURCE_NODE_Pos      ) & HEADER_SOURCE_NODE_Msk;
    //***
    header->priority         = ( data >> HEADER_PRIORITY_Pos         ) & HEADER_PRIORITY_Msk;
    /************************************************************************************************/
    header->len = pData[LEN_FIELD_POS];
}


void SetunisatHeader( uint8_t* pData, struct unisat_header* header)
{
    uint32_t unisat_header = 0;
    unisat_header |= (header->destination_node & HEADER_DESTINATION_NODE_Msk) << HEADER_DESTINATION_NODE_Pos;
    unisat_header |= (header->source_node & HEADER_SOURCE_NODE_Msk ) << HEADER_SOURCE_NODE_Pos;
    unisat_header |= (header->priority & HEADER_PRIORITY_Msk) << HEADER_PRIORITY_Pos;
    //*****
    *pData++ = unisat_header & 0xFF;
    *pData++ = (unisat_header>>8) & 0xFF;
}
