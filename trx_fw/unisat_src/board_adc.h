#ifndef __INC_BOARD_ADC_H__
#define __INC_BOARD_ADC_H__

#include "unisat_types.h"

AF_Status ReadVref(uint16_t* pvref, uint16_t timeout);
uint16_t GetVref(void);
AF_Status ReadTemperature(int16_t* ptemp, uint16_t timeout);
uint16_t GetTemperature(void);

#endif
