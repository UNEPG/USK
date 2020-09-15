#ifndef __BOARD_EEPROM_H__
#define __BOARD_EEPROM_H__

#include "unisat_types.h"

//#define EEPROM_SETTINGS_BASE_ADDRESS      (DATA_EEPROM_BANK2_BASE)
#define EEPROM_SETTINGS_BASE_ADDRESS            DATA_EEPROM_BASE
#define EEPROM_RADIO_SETTINGS_ADDRESS           (EEPROM_SETTINGS_BASE_ADDRESS + 0UL)
#define EEPROM_RADIO_SETTINGS_SIZE              ( 64 )
#define EEPROM_SIMCOM_SETTINGS_ADDRESS          (EEPROM_SETTINGS_BASE_ADDRESS + 128UL)
#define EEPROM_SIMCOM_SETTINGS_SIZE             ( 128 )
#define EEPROM_SCHEDULER_SETTING_ADDR           (EEPROM_SETTINGS_BASE_ADDRESS + 256UL)
#define EEPROM_SCHEDULER_SETTING_SIZE           ( 128 )

// Funtions
bool EEPROM_isValidBlock(uint32_t address, uint16_t size);
bool EEPROM_writeBlock(uint32_t address, uint8_t* data, uint16_t size);
bool EEPROM_readBlock(uint32_t address, uint8_t* data, uint16_t size);

#endif /* __BOARD_EEPROM_H__ */
