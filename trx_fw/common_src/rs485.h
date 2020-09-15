#ifndef __RS485_H__
#define __RS485_H__

#include "hw.h"
#include "unisat_types.h"

void RS485_Init(    void (*pRxCpltCallback)( void ), 
                    void (*pTxCpltCallback)( void ),
                    void (*pRxTxErrorCallback)( void ),
                    void (*pTimeoutCallback)( void ) );

AF_Status RS485_Send(uint8_t* buf, uint8_t len, uint32_t timeout);

AF_Status RS485_Receive(uint8_t* buf, uint16_t len, uint32_t timeout, uint32_t bit_timeout);

uint8_t RS485_GetRecvdLen(void);

void RS485_DeInit(void);

void RS485_IoInit(void);

void RS485_IoDeInit(void);

void RS485_USARTx_IRQHandler(void);

void RS485_USARTx_DMA_RXTX_IRQHandler(void);

#endif /* __RS485_H__ */
