#ifndef __INC_unisat_PROCESSING_H__
#define __INC_unisat_PROCESSING_H__

#include "unisat_types.h"

#define NO_NEED_CRC             0
#define NEED_CALC_CRC           1


void AFBus_PacketProcessingInit( void );
AF_Status AFBus_PacketProcessing( uint8_t* data, uint8_t len, uint8_t* pAnswer, uint8_t* lenAnswer, uint8_t crc_calc );

#endif
