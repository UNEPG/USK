#ifndef __BOARD_SCHEDULER_H__
#define __BOARD_SCHEDULER_H__

#include "unisat_types.h"

//**** Defines ****//
#define SCHEDULER_CHANNELS_COUNT            16

//**** Typdef/enumes ****//


//**** Functions ****//
AF_Status Scheduler_LoadSettings(void);
AF_Status Scheduler_SaveSettings(void);
AF_Status Scheduler_LoadDefaults(void);
AF_Status Scheduler_SetAct( uint8_t ch, uint8_t act );
AF_Status Scheduler_SetPeriod( uint8_t ch, uint16_t period );
AF_Status Scheduler_SetSettings( uint8_t ch, 
                                    uint8_t act, 
                                    uint8_t cond, 
                                    uint8_t to_node, 
                                    uint16_t period, 
                                    uint16_t mask, 
                                    uint32_t pFunc );


//**** FreeRTOS Task ****//
void SchedulerTask(void const * argument);

#endif /* __BOARD_EE__BOARD_SCHEDULER_H__PROM_H__ */
