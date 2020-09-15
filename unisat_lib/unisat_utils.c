//           _      ______       _____      _______ 
//     /\   | |    |  ____/\    / ____|  /\|__   __|
//    /  \  | |    | |__ /  \  | (___   /  \  | |   
//   / /\ \ | |    |  __/ /\ \  \___ \ / /\ \ | |   
//  / ____ \| |____| | / ____ \ ____) / ____ \| |   
// /_/    \_\______|_|/_/    \_\_____/_/    \_\_|   
//                                                  
//
// Author: Nursultan Uzbekov
// Date:   19.01.2020
//

#include "unisat_utils.h"

#define CRC16_POLY              0xA001
#define CRC16_START             0xFFFF

static bool crc_initilized = false;
static uint16_t tab_crc16[256];

static void Init_CRC16( void );

AF_Status Check_CRC16( const uint8_t *input_str, size_t len )
{
    uint8_t* data = (uint8_t*) input_str;
    len -= 2;
    input_str += len;
    uint16_t crc = *input_str++;
    crc |= (*input_str<<8);
    
    if( crc == Calc_CRC16(data, len) )
    {
        return AF_OK;
    }
    return AF_ERROR;
}

uint16_t Calc_CRC16( const uint8_t *input_str, size_t len )
{
    
	uint16_t crc;
	const unsigned char *ptr;
	size_t a;
    
	if ( ! crc_initilized ) Init_CRC16();
    
	crc = CRC16_START;
	ptr = input_str;
    
	if ( ptr != NULL ) for (a=0; a<len; a++)
    {
		crc = (crc >> 8) ^ tab_crc16[ (crc ^ (uint16_t) *ptr++) & 0x00FF ];
	}
    
	return crc;
}

static void Init_CRC16( void )
{
    
	uint16_t crc;
	uint16_t c;
    
	for (int crci=0; crci<256; crci++) {

		crc = 0;
		c = crci;
        
		for (int crcj=0; crcj<8; crcj++)
        {
            
			if ( (crc ^ c) & 0x0001 ) 
            {
                crc = ( crc >> 1 ) ^ CRC16_POLY;
            }
			else
            {
                crc = crc >> 1;
            }
            
			c = c >> 1;
		}

		tab_crc16[crci] = crc;
	}

	crc_initilized = true;

}
