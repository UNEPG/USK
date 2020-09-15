#ifndef __INC_unisat_TYPES_H__
#define __INC_unisat_TYPES_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef enum 
{
    AF_OK = 0,
    AF_BUSY,
    AF_WAIT,
    AF_FULL,
    AF_ERROR,
    AF_TIMEOUT,
} AF_Status;

#endif
