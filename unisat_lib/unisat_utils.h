#ifndef __INC_unisat_UTILS_H__
#define __INC_unisat_UTILS_H__

#include "unisat_types.h"


uint16_t Calc_CRC16( const uint8_t *input_str, size_t num_bytes );
AF_Status Check_CRC16( const uint8_t *input_str, size_t num_bytes );

#endif
