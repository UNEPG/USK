//           _      ______       _____      _______ 
//     /\   | |    |  ____/\    / ____|  /\|__   __|
//    /  \  | |    | |__ /  \  | (___   /  \  | |   
//   / /\ \ | |    |  __/ /\ \  \___ \ / /\ \ | |   
//  / ____ \| |____| | / ____ \ ____) / ____ \| |   
// /_/    \_\______|_|/_/    \_\_____/_/    \_\_|   
//                                                  
//
// Author: Nursultan Uzbekov
// Date:  10.12.2019
//

#include "board_sx127x.h"
#include "board_rs485.h"
#include "board_eeprom.h"
#include "radio.h"
#include "sx1276.h"
#include "cmsis_os.h"
#include "utilities.h"
#include "timeServer.h"
#include "unisat_utils.h"
#include "unisat_protocol.h"
#include "unisat_processing.h"

#define RADIO_RX_BUF_LEN                    (AFBUS_RADIO_MAX_LEN)
#define RADIO_MESSAGE_MAX_LEN               (RADIO_RX_BUF_LEN + 1)  // ID field
#define RADIO_RX_QUEUE_ELEMENT_SIZE         (sizeof(struct radiobus_rx_queue_element))
#define RADIO_TX_QUEUE_ELEMENT_SIZE         (sizeof(struct radiobus_tx_queue_element))
#define RADIO_RX_QUEUE_ELEMENTS             (5)
#define RADIO_TX_QUEUE_ELEMENTS             (10)

#define RF_FREQUENCY_MIN                    433050000
#define RF_FREQUENCY_STD                    434000000   // Hz
#define RF_FREQUENCY_MAX                    434790000
#define RF_BANDWIDTH_STD                    125000
#define RF_POWER_MIN                        5
#define RF_POWER_STD                        5           // dBm
#define RF_POWER_MAX                        20

#define RX_TIMEOUT_VALUE                    5000
#define LORA_TX_TIMEOUT                     9000
#define TX_MINIMAL_DUTY_CYCLE               1000
#define TX_NORMAL_DUTY_CYCLE                5000

#define LORA_FIX_LENGTH_PAYLOAD_OFF         false
#define LORA_IQ_INVERSION_OFF               false
#define LORA_CRC_ON                         true



struct __attribute__ ((__packed__)) radiobus_rx_queue_element
{
    uint8_t len;
    uint8_t data[RADIO_RX_BUF_LEN];
    int8_t snr;
    int16_t rssi;
};

struct __attribute__ ((__packed__)) radiobus_tx_queue_element
{
    uint8_t len;
    uint8_t data[RADIO_RX_BUF_LEN];
};

struct __attribute__ ((__packed__)) radio_setting
{
    uint32_t frequency;
    int8_t power;
    uint8_t mode;       // FF - user settings
    uint32_t dataRate;
    uint8_t codeRate;
    uint16_t preambleLen;
    uint32_t byteTimeout;
    
    uint8_t padding[ 45 ];  // выравнивание до блока 
    
    uint16_t crc;
};

struct radio_mode
{
    uint32_t datarate;
    uint8_t coderate;
    uint16_t preambleLen;
    uint32_t timeout;
};

/* Variables */
static RadioEvents_t RadioEvents;
static struct radiobus_rx_queue_element radio_rx_message;
static struct radiobus_tx_queue_element radio_tx_message;
static xQueueHandle Radio_RxQueue;
static xQueueHandle Radio_TxQueue;
static xSemaphoreHandle Radio_RxErrorSemaphore;
static xSemaphoreHandle Radio_TxErrorSemaphore;
static xSemaphoreHandle Radio_TimeoutSemaphore;
static xSemaphoreHandle Radio_RxReInitSemaphore;
osThreadId TxDutyCycleHandle;
static struct radio_setting RadioSetting;


AF_Status Radio_Init(void);
void Radio_StartRX(void);
AF_Status Radio_Transmit(uint8_t* buffer, uint8_t size);
static void TxDutyCycleTask(void const * argument);
static void OnTxDone(void);
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
static void OnTxTimeout(void);
static void OnRxTimeout(void);
static void OnRxError(void);


static const struct radio_mode radio_modes[RADIO_MODES_COUNT] =
{
    { .datarate = 12,   .preambleLen = 32,    .coderate = 4,  .timeout = 5 },
    { .datarate = 11,   .preambleLen = 32,    .coderate = 4,  .timeout = 5 },
    { .datarate = 10,   .preambleLen = 32,    .coderate = 4,  .timeout = 5 },
    { .datarate =  9,   .preambleLen = 32,    .coderate = 4,  .timeout = 5 },
    { .datarate =  8,   .preambleLen = 32,    .coderate = 4,  .timeout = 5 },
    { .datarate =  7,   .preambleLen = 32,    .coderate = 4,  .timeout = 5 },
};



void RadioTask(void const * argument)
{
    
    Radio_LoadSettings();
    
    Radio_Init();
    Radio.Sleep();
    
    AFBus_PacketProcessingInit();
    
    uint8_t rx_buffer[RADIO_RX_QUEUE_ELEMENT_SIZE];
    
    Radio_RxQueue = xQueueCreate( RADIO_RX_QUEUE_ELEMENTS, RADIO_RX_QUEUE_ELEMENT_SIZE);
    Radio_TxQueue = xQueueCreate( RADIO_TX_QUEUE_ELEMENTS, RADIO_TX_QUEUE_ELEMENT_SIZE);
    
    if( ( Radio_RxQueue == NULL ) || ( Radio_TxQueue == NULL ) )
    {
        vTaskSuspend(NULL);
    }
    
    Radio_RxErrorSemaphore = xSemaphoreCreateBinary();
    Radio_TxErrorSemaphore = xSemaphoreCreateBinary();
    Radio_TimeoutSemaphore   = xSemaphoreCreateBinary();
    Radio_RxReInitSemaphore  = xSemaphoreCreateBinary();
    
    if( ( Radio_RxErrorSemaphore == NULL ) || ( Radio_TxErrorSemaphore == NULL ) ||
        ( Radio_TimeoutSemaphore == NULL ) || ( Radio_RxReInitSemaphore == NULL) )
    {
        vTaskSuspend(NULL);
    }
    
    osThreadDef(TxDutyCycle, TxDutyCycleTask, osPriorityBelowNormal, 0, configMINIMAL_STACK_SIZE);
    TxDutyCycleHandle = osThreadCreate(osThread(TxDutyCycle), NULL);
    
    if( TxDutyCycleHandle == NULL )
    {
        vTaskSuspend(NULL);
    }
    
    xSemaphoreGive( Radio_RxReInitSemaphore );
    
    while(1)
    {
        
        osDelay(5);
        
        if( uxQueueMessagesWaiting( Radio_RxQueue ) != 0 )
        {
            
            if( xQueueReceive( Radio_RxQueue, &rx_buffer, 0 ) == pdTRUE )
            {
                struct radiobus_rx_queue_element* message = (struct radiobus_rx_queue_element*) &rx_buffer;
                
                struct unisat_header af_head;
                GetunisatHeader( radio_rx_message.data, &af_head);
                
                if( af_head.destination_node == THIS_BOARD_ADDRESS )
                {
                    
                    AFBus_PacketProcessing(message->data, message->len, (uint8_t*) &radio_tx_message.data, &radio_tx_message.len, NO_NEED_CRC);
                    
                    if( radio_tx_message.len != 0 )
                    {
                        osDelay(2000);
                        
                        Radio_PushTxQueue( (uint8_t*) &radio_tx_message );
                    }
                }
                else
                {
                    uint8_t* data = message->data;
                    
                    uint16_t crc = Calc_CRC16(data, message->len);
                    data += message->len;
                    
                    *data++ = crc & 0xFF;
                    *data = (crc>>8) & 0xFF;
                    message->len += CRC_FIELD_LEN;
                    
                    AFBus_PushTxQueue( (uint8_t*) &rx_buffer );
                }
            }
            
        }
        
        if( xSemaphoreTake( Radio_RxErrorSemaphore, 0 ) == pdTRUE )
        {
            Radio_StartRX();
        }
        
        if( xSemaphoreTake( Radio_TimeoutSemaphore, 0 ) == pdTRUE )
        {
            Radio_StartRX();
        }
        
        if( xSemaphoreTake( Radio_RxReInitSemaphore, 0 ) == pdTRUE )
        {
            Radio_StartRX();
        }
        
        if( xSemaphoreTake( Radio_TxErrorSemaphore, 0 ) == pdTRUE )
        {
            Radio_ReInit(true, true);
        }
    }    
}

static void TxDutyCycleTask(void const * argument)
{
    
    uint8_t tx_len = 0;
    osDelay(1000);
    
    while(1)
    {
        if( xQueueReceive( Radio_TxQueue, &radio_tx_message, portMAX_DELAY) == pdTRUE )
        {
            tx_len = radio_tx_message.len + 1;
            radio_tx_message.len = RADIO_DEFAULT_ID;
            Radio.Send( (uint8_t*)&radio_tx_message.len, tx_len);
            osDelay(TX_NORMAL_DUTY_CYCLE);
        }
    }
}

void Radio_PushTxQueue(uint8_t* to_push)
{
    if( xQueueSend( Radio_TxQueue, to_push, 0 ) != pdPASS )
    {
        __nop();
    }
}

static void OnTxDone(void)
{
    Radio.Sleep();
    xSemaphoreGiveFromISR( Radio_RxReInitSemaphore, NULL );
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    Radio.Sleep();
    
    if( (*payload == RADIO_DEFAULT_ID) &&
        (size > RADIO_PACKET_MINIMAL_LEN) &&
         (size <= RADIO_MESSAGE_MAX_LEN) ) 
    {
        payload++;
        radio_rx_message.len = size;
        radio_rx_message.rssi = rssi;
        radio_rx_message.snr = snr;
        memcpy( radio_rx_message.data, payload, size);
        
        struct unisat_header af_head;
        GetunisatHeader( radio_rx_message.data, &af_head);
        
        if( ( af_head.destination_node >= THIS_AREA_MIN_ADDRESS ) && 
            ( af_head.destination_node <= THIS_AREA_MAX_ADDRESS ) &&
             ((af_head.len + RADIO_PACKET_MINIMAL_LEN) == radio_rx_message.len ) )
        {
            radio_rx_message.len--;
            if( xQueueSendFromISR( Radio_RxQueue, (const void *) &radio_rx_message, NULL ) == errQUEUE_FULL )
            {
                __nop();
            }
        }
    }
    
    xSemaphoreGiveFromISR( Radio_RxReInitSemaphore, NULL );
}

static void OnTxTimeout(void)
{
    Radio.Sleep();
    xSemaphoreGiveFromISR( Radio_TxErrorSemaphore, NULL );
}

static void OnRxTimeout(void)
{
    Radio.Sleep();
    xSemaphoreGiveFromISR( Radio_TimeoutSemaphore, NULL );
}

static void OnRxError(void)
{
    Radio.Sleep();
    xSemaphoreGiveFromISR( Radio_RxErrorSemaphore, NULL );
}


AF_Status Radio_Init(void)
{
    // Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init(&RadioEvents);
    
    Radio.SetChannel(RadioSetting.frequency);

    Radio.SetTxConfig(  MODEM_LORA, 
                        RadioSetting.power, 
                        0,  // ------------------------------> Fdev (only for FSK)
                        0,  // ------------------------------> Bandwidth 125 kHz
                        RadioSetting.dataRate, 
                        RadioSetting.codeRate,
                        RadioSetting.preambleLen, 
                        LORA_FIX_LENGTH_PAYLOAD_OFF,
                        LORA_CRC_ON, 
                        0, 
                        0, 
                        LORA_IQ_INVERSION_OFF,
                        LORA_TX_TIMEOUT  // ---------------------------->  timeout
                        );

    Radio.SetRxConfig(  MODEM_LORA, 
                        0,  // ------------------------------> Bandwidth 125 kHz
                        RadioSetting.dataRate,
                        RadioSetting.codeRate,
                        0,  // ------------------------------> AFC bandwidth (only for FSK)
                        RadioSetting.preambleLen,
                        RadioSetting.byteTimeout,
                        LORA_FIX_LENGTH_PAYLOAD_OFF,
                        0,  // ------------------------------> Payload len (only when fix_payload_len)
                        LORA_CRC_ON,
                        0,  // ------------------------------> Frequency Hopping off
                        0,  // ------------------------------> Frequency Hopping periode
                        LORA_IQ_INVERSION_OFF, 
                        true    // --------------------------> RxContinuous
                        );
    
    return AF_OK;
}

AF_Status Radio_ReInit( bool rx_reinit, bool tx_reinit )
{
    
    if( tx_reinit )
    {
        // переинициализация TX в случаи изменения мощности
        Radio.SetTxConfig(  MODEM_LORA, 
                            RadioSetting.power, 
                            0,  // ------------------------------> Fdev (only for FSK)
                            0,  // ------------------------------> Bandwidth 125 kHz
                            RadioSetting.dataRate, 
                            RadioSetting.codeRate,
                            RadioSetting.preambleLen, 
                            LORA_FIX_LENGTH_PAYLOAD_OFF,
                            LORA_CRC_ON, 
                            0, 
                            0, 
                            LORA_IQ_INVERSION_OFF,
                            LORA_TX_TIMEOUT  // ---------------------------->  timeout
                            );
    }

    if( rx_reinit )
    {
        Radio.SetRxConfig(  MODEM_LORA, 
                            0,  // ------------------------------> Bandwidth 125 kHz
                            RadioSetting.dataRate,
                            RadioSetting.codeRate,
                            0,  // ------------------------------> AFC bandwidth (only for FSK)
                            RadioSetting.preambleLen,
                            RadioSetting.byteTimeout, 
                            LORA_FIX_LENGTH_PAYLOAD_OFF,
                            0,  // ------------------------------> Payload len (only when fix_payload_len)
                            LORA_CRC_ON,
                            0,  // ------------------------------> Frequency Hopping off
                            0,  // ------------------------------> Frequency Hopping periode
                            LORA_IQ_INVERSION_OFF, 
                            true    // --------------------------> RxContinuous
                            );
    }
    
    return AF_OK;
    
}

static void Radio_StartRX(void)
{
    Radio.Rx(RX_TIMEOUT_VALUE);
}


//***********************************************************************
//********************* Settings, commands ******************************
//***********************************************************************

AF_Status Radio_SetMode(uint8_t mode)
{
    if( mode < RADIO_MODES_COUNT )
    {
        RadioSetting.mode = mode;
        RadioSetting.dataRate = radio_modes[mode].datarate;
        RadioSetting.codeRate = radio_modes[mode].coderate;
        RadioSetting.preambleLen = radio_modes[mode].preambleLen;
        RadioSetting.byteTimeout = radio_modes[mode].timeout;
        return AF_OK;
    }
    return AF_ERROR;
}

uint8_t Radio_GetMode(void)
{
    return RadioSetting.mode;
}

AF_Status Radio_SetPower(int8_t power)
{
    if( power >= RF_POWER_MIN && power <= RF_POWER_MAX )
    {
        RadioSetting.power = power;
        Radio_ReInit( false, true );
        return AF_OK;
    }   
    return AF_ERROR;
}

int8_t Radio_GetPower(void)
{
    return RadioSetting.power;
}

AF_Status Radio_SetFrequency(uint32_t freq)
{
    if((freq >= (RF_FREQUENCY_MIN + (RF_BANDWIDTH_STD/2))) && 
       (freq <= (RF_FREQUENCY_MAX - (RF_BANDWIDTH_STD/2))))
    {
        RadioSetting.frequency = freq;
        Radio.SetChannel(freq);
        return AF_OK;
    }
    return AF_ERROR;
}

uint32_t Radio_GetFrequency(void)
{
    return RadioSetting.frequency;
}

AF_Status Radio_SetDataRate(uint32_t value)
{
    if((value >= 7) && 
       (value <= 300000))
    {
        RadioSetting.mode = 0xFF;
        RadioSetting.dataRate = value;
        Radio_ReInit( true, true );
        return AF_OK;
    }
    return AF_ERROR;
}

uint32_t Radio_GetDataRate(void)
{
    return RadioSetting.dataRate;
}

AF_Status Radio_SetPreambleLen(uint16_t value)
{
    if(value >= 3)
    {
        RadioSetting.mode = 0xFF;
        RadioSetting.preambleLen = value;
        Radio_ReInit( true, true );
        return AF_OK;
    }
    return AF_ERROR;
}

uint16_t Radio_GetPreambleLen(void)
{
    return RadioSetting.preambleLen;
}

AF_Status Radio_SetCodeRate(uint8_t value)
{
    if( value <= 4 )
    {
        RadioSetting.mode = 0xFF;
        RadioSetting.codeRate = value;
        Radio_ReInit( true, true );
        return AF_OK;
    }
    return AF_ERROR;
}

uint8_t Radio_GetCodeRate(void)
{
    return RadioSetting.codeRate;
}

AF_Status Radio_SetByteTimeout(uint32_t value)
{
    if((value >= 4) &&
       (value <= 0xFFFFFFFF))
    {
        RadioSetting.mode = 0xFF;
        RadioSetting.byteTimeout = value;
        Radio_ReInit( true, true );
        return AF_OK;
    }
    return AF_ERROR;
}

uint8_t Radio_GetByteTimeout(void)
{
    return RadioSetting.byteTimeout;
}

AF_Status Radio_LoadDefaults( void )
{
    
    RadioSetting.mode = 0; 
    RadioSetting.dataRate = radio_modes[ RadioSetting.mode ].datarate;
    RadioSetting.codeRate = radio_modes[ RadioSetting.mode ].coderate;
    RadioSetting.preambleLen = radio_modes[ RadioSetting.mode ].preambleLen;
    RadioSetting.byteTimeout = radio_modes[ RadioSetting.mode ].timeout;
    RadioSetting.frequency = RF_FREQUENCY_STD;
    RadioSetting.power = RF_POWER_STD;
    
    return AF_OK;
}

AF_Status Radio_LoadSettings(void)
{
    
    if( !EEPROM_isValidBlock( EEPROM_RADIO_SETTINGS_ADDRESS, EEPROM_RADIO_SETTINGS_SIZE ) )
    {
        Radio_LoadDefaults();
        
        RadioSetting.crc = Calc_CRC16( (uint8_t*) &RadioSetting, (sizeof(RadioSetting) - sizeof(RadioSetting.crc)) );
        
        EEPROM_writeBlock( EEPROM_RADIO_SETTINGS_ADDRESS, (uint8_t*)&RadioSetting, sizeof(RadioSetting) );
    }
    else
    {
        EEPROM_readBlock( EEPROM_RADIO_SETTINGS_ADDRESS, (uint8_t*)&RadioSetting, sizeof(RadioSetting) );
    }
    return AF_OK;
    
}

AF_Status Radio_SaveSettings(void)
{
    RadioSetting.crc = Calc_CRC16( (uint8_t*) &RadioSetting, (sizeof(RadioSetting) - sizeof(RadioSetting.crc)) );
    
    if( EEPROM_writeBlock( EEPROM_RADIO_SETTINGS_ADDRESS, (uint8_t*)&RadioSetting, sizeof(RadioSetting) ) )
    {
        return AF_OK;
    }
    return AF_ERROR;
}
