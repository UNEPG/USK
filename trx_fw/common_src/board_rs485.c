#include "hw.h"
#include "rs485.h"
#include "cmsis_os.h"
#include "board_rs485.h"
#include "board_sx127x.h"
#include "unisat_utils.h"
#include "unisat_protocol.h"
#include "unisat_processing.h"


#define AFBUS_RX_BUF_LEN            (AFBUS_BUS_MAX_LEN)
#define AFBUS_QUEUE_ELEMENT_SIZE    (sizeof(struct afbus_queue_element))
#define AFBUS_RX_QUEUE_ELEMENTS     (5)
#define AFBUS_TX_QUEUE_ELEMENTS     (3)

#define AFBUS_TX_TIMEOUT            (100)
#define AFBUS_RX_BYTE_TIMEOUT       (45)


struct afbus_queue_element
{
    uint8_t len;
    uint8_t data[AFBUS_RX_BUF_LEN];
    uint8_t padding8;
    uint16_t padding16;
};

/* Variables */
static struct afbus_queue_element rx_message;
static struct afbus_queue_element tx_message;
static xQueueHandle Rx_Queue;
static xQueueHandle Tx_Queue;
static xSemaphoreHandle Error_Semaphore;
static xSemaphoreHandle Timeout_Semaphore;
static xSemaphoreHandle RxReInit_Semaphore;


/* Static Functions definition */
static void AFBus_TxComplete(void);
static void AFBus_RxComplete(void);
static void AFBus_RxTxError(void);
static void AFBus_Timeout(void);
static void AFBus_StartRX(void);

void AFBus_Init(void)
{
    RS485_Init( &AFBus_RxComplete, 
                &AFBus_TxComplete,
                &AFBus_RxTxError,
                &AFBus_Timeout);
    
    Rx_Queue = xQueueCreate( AFBUS_RX_QUEUE_ELEMENTS, AFBUS_QUEUE_ELEMENT_SIZE);
    Tx_Queue = xQueueCreate( AFBUS_TX_QUEUE_ELEMENTS, AFBUS_QUEUE_ELEMENT_SIZE);
    
    if( ( Rx_Queue == 0 ) || ( Tx_Queue == 0 ) )
    {
        //vTaskDelete(NULL);
    }
    
    Error_Semaphore     = xSemaphoreCreateBinary();
    Timeout_Semaphore   = xSemaphoreCreateBinary();
    RxReInit_Semaphore  = xSemaphoreCreateBinary();
}

void AFBusTask(void const *arguments)
{
    
    AFBus_Init();
    AFBus_PacketProcessingInit();
    
    uint8_t rx_buffer[AFBUS_QUEUE_ELEMENT_SIZE];
    
    xSemaphoreGive( RxReInit_Semaphore );
    
    while(1)
    {
        
        osDelay(5);
        
        if( uxQueueMessagesWaiting( Rx_Queue ) != 0 )
        {
            
            if( xQueueReceive( Rx_Queue, &rx_buffer, 0 ) == pdTRUE )
            {
                struct afbus_queue_element* message = (struct afbus_queue_element*) &rx_buffer;
                
                if( Check_CRC16( message->data, message->len) == AF_OK )
                {
                    message->len -= CRC_FIELD_LEN;
                    
                    struct unisat_header af_head;
                    GetunisatHeader( rx_message.data, &af_head);
                    
                    if( af_head.destination_node == THIS_BOARD_ADDRESS )
                    {
                        
                        AFBus_PacketProcessing(message->data, message->len, (uint8_t*) &tx_message.data, &tx_message.len, NEED_CALC_CRC);
                        if( tx_message.len != 0 )
                        {
                            //osDelay(10);
                            AFBus_PushTxQueue( (uint8_t*) &tx_message );
                        }
                    }
                    else
                    {
                        Radio_PushTxQueue( (uint8_t*) &rx_buffer );
                    }
                    
                }
            }
            
        }
        
        if( uxQueueMessagesWaiting( Tx_Queue ) != 0 )
        {
            if( xQueueReceive( Tx_Queue, &tx_message, 0 ) == pdTRUE )
            {
                
                xSemaphoreTake( Error_Semaphore, 0 );
                xSemaphoreTake( Timeout_Semaphore, 0 );
                xSemaphoreTake( RxReInit_Semaphore, 0 );
                
                RS485_Send( tx_message.data, tx_message.len, 10000);
            }
        }
        
        if( xSemaphoreTake( Error_Semaphore, 0 ) == pdTRUE )
        {
            AFBus_StartRX();
        }
        
        if( xSemaphoreTake( Timeout_Semaphore, 0 ) == pdTRUE )
        {
            AFBus_StartRX();
        }
        
        if( xSemaphoreTake( RxReInit_Semaphore, 0 ) == pdTRUE )
        {
            AFBus_StartRX();
        }
        
    }
    
}

void AFBus_PushTxQueue(uint8_t* to_push)
{
    if( xQueueSend( Tx_Queue, to_push, 0 ) == errQUEUE_FULL )
    {
        __nop();
    }
}

static void AFBus_TxComplete(void)
{
    
    xSemaphoreGiveFromISR( RxReInit_Semaphore, NULL );
}

static void AFBus_RxComplete(void)
{
    
    rx_message.len = RS485_GetRecvdLen();
    
    struct unisat_header af_head;
    GetunisatHeader( rx_message.data, &af_head);
    
    if( ( af_head.destination_node >= THIS_AREA_MIN_ADDRESS ) && 
        ( af_head.destination_node <= THIS_AREA_MAX_ADDRESS ) &&
        ( af_head.destination_node != THIS_BOARD_ADDRESS ) )
    {
        xSemaphoreGiveFromISR( RxReInit_Semaphore, NULL );
        return;
    }
    
    if( xQueueSendFromISR( Rx_Queue, (const void *) &rx_message, NULL ) == errQUEUE_FULL )
    {
        __nop();
    }
    
    xSemaphoreGiveFromISR( RxReInit_Semaphore, NULL );
    
}

static void AFBus_RxTxError(void)
{
    xSemaphoreGiveFromISR( Error_Semaphore, NULL );
}

static void AFBus_Timeout(void)
{
    xSemaphoreGiveFromISR( Timeout_Semaphore, NULL );
}

static void AFBus_StartRX(void)
{
    memset( (uint8_t*) &rx_message, 0, sizeof(rx_message) );
    RS485_Receive( rx_message.data, AFBUS_RX_BUF_LEN, 10000, AFBUS_RX_BYTE_TIMEOUT);
}
