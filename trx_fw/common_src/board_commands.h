#ifndef __INC_BOARD_COMMANDS_H__
#define __INC_BOARD_COMMANDS_H__

#include "unisat_types.h"
#include "unisat_protocol.h"
#include <time.h>


// Методы
bool getSystemTime( time_t* ts );
bool updateSystemTime(struct tm* pTime);

#endif
