#include "hw.h"
#include "board_eeprom.h"
#include "unisat_utils.h"
#include "unisat_protocol.h"


#define ADDR            (0x08080000)


struct __attribute__ ((__packed__)) afbus_queue_element
{
    uint8_t len;
    uint16_t padding16;
};

/* Variables */



bool EEPROM_isValidBlock(uint32_t address, uint16_t size)
{
    size -= 2;
    uint16_t temp = Calc_CRC16( (uint8_t*) &*(uint8_t*)address, size);
    uint16_t crc = *(uint8_t*)(address + size);
    crc |= *(uint8_t*)(address + size + 1) << 8;
    return (crc == temp);
}

bool EEPROM_writeBlock(uint32_t address, uint8_t* data, uint16_t size)
{
    uint32_t word;
    uint8_t word_len = sizeof(word);
    if( size % word_len != 0 )
    {
        return false;
    }
    
    if( HAL_FLASHEx_DATAEEPROM_Unlock() != HAL_OK )
    {
        return false;
    }
    
    for( int i=0; i< ( size / word_len ); i++ )
    {
        HAL_FLASHEx_DATAEEPROM_Erase( address );
        memcpy( (uint8_t*) &word, data, word_len);
        HAL_FLASHEx_DATAEEPROM_Program( FLASH_TYPEPROGRAMDATA_WORD, address, word);
        address += word_len;
        data += word_len;
    }
    
    if( HAL_FLASHEx_DATAEEPROM_Lock() != HAL_OK )
    {
        return false;
    }
    return true;
}

bool EEPROM_readBlock(uint32_t address, uint8_t* data, uint16_t size)
{

    if( __HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) )
    {
        return false;
    }

    for( int i=0; i< ( size ); i++ )
    {
        *data++ = *(uint8_t*)address++;
    }

    return true;
}
