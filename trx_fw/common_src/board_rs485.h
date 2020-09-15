#ifndef __BOARD_RS485_H__
#define __BOARD_RS485_H__

#include "cmsis_os.h"

#define     AFBUS_TASK_STACK_SIZE       256
#define     AFBUS_TASK_PRIORITY         osPriorityNormal

// FreeRTOS Task
void AFBusTask(void const *arguments);
void AFBus_PushTxQueue(uint8_t* to_push);

#endif /* __BOARD_RS485_H__ */
