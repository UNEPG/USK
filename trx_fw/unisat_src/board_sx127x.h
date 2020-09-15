#ifndef __INC_BOARD_SX127x_H__
#define __INC_BOARD_SX127x_H__

#include "unisat_types.h"

#define     RADIO_TASK_STACK_SIZE       512
#define     RADIO_TASK_PRIORITY         osPriorityNormal

enum RADIO_MODES
{
    RADIO_MODE_SF12 = 0,
    RADIO_MODE_SF11,
    RADIO_MODE_SF10,
    RADIO_MODE_SF9,
    RADIO_MODE_SF8,
    RADIO_MODE_SF7,
    
    RADIO_MODES_COUNT
};

AF_Status Radio_LoadDefaults(void);
AF_Status Radio_LoadSettings(void);
AF_Status Radio_SaveSettings(void);
AF_Status Radio_ReInit(bool rx_reinit, bool tx_reinit);
AF_Status Radio_SetMode(uint8_t mode);
uint8_t Radio_GetMode(void);
AF_Status Radio_SetPower(int8_t power);
int8_t Radio_GetPower(void);
AF_Status Radio_SetFrequency(uint32_t freq);
uint32_t Radio_GetFrequency(void);
AF_Status Radio_SetDataRate(uint32_t value);
uint32_t Radio_GetDataRate(void);
AF_Status Radio_SetPreambleLen(uint16_t value);
uint16_t Radio_GetPreambleLen(void);
AF_Status Radio_SetCodeRate(uint8_t value);
uint8_t Radio_GetCodeRate(void);
AF_Status Radio_SetByteTimeout(uint32_t value);
uint8_t Radio_GetByteTimeout(void);




// FreeRTOS Task
void RadioTask(void const * argument);
void Radio_PushTxQueue(uint8_t* to_push);

#endif
