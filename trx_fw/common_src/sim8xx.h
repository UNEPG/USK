#ifndef __INC_SIM8XX_H__
#define __INC_SIM8XX_H__

#include "stm32l0xx_hal.h"
#include "stdint.h"
#include "stdbool.h"

#define SimCom_Delay    osDelay
#define SimCom_GetTick  xTaskGetTickCount

#define SIMCOM_DEFAULT_TERMINAL_TIMEOUT     1000

enum gsm_state
{
    FIRST_LOOP = 0,
    INIT_STATE,
    GSM_CONNECTED,
    GPRS_CONNECTED,
    TCP_CONNECTED,
    ERROR_CONNECTION,
    
};

enum op_id
{
    OPERATOR_BEELINE = 10,
    OPERATOR_KCELL,
    OPERATOR_ALTEL,
    OPERATOR_TELE2,
    /////////////////////
    OPERATORS_COUNT
};

struct sim8xx_data
{
    enum gsm_state state;
    bool error;
    
    // GSM
    char IMEI[15];
    enum op_id operator_type;
    uint16_t operator_id;
    uint16_t balance;
    char gsm_long[16];
    char gsm_latt[16];
    char gsm_date[16];
    char gsm_time[16];
    char gsm_location[40];
    
    // GPS
    char    gps_datetime[20];
    bool    gps_fix_status;
    float   gps_longitude;
    float   gps_latitude;
    float   gps_altitude;
    float   gps_speed_kph;
    float   gps_coarse;
    float   gps_hdop;
    float   gps_pdop;
    float   gps_vdop;
    uint8_t gps_in_view;
    uint8_t gps_in_used;
    uint8_t glo_in_view;
    uint8_t gps_cn;         //carrier-to-noise-density
    
    char response[512];
    char result[512];
};

void SIM8xx_basicInit( UART_HandleTypeDef *huart, struct sim8xx_data** data, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, bool with_pwrkey );
bool SIM8xx_modemInit( void );
void SIM8xx_togglePwrKey( void );
bool SIM8xx_checkModem(void);
bool initPrefs( void );
bool SIM8xx_getIMEI( void );
bool getLocation( void );
bool isConnected( void );
bool doInit( void );

int SIM8xx_getBattery( void );
int SIM8xx_getSignal( void );
float SIM8xx_getBalance( void );
uint16_t SIM8xx_getOperatorID(void);
bool SIM8xx_getGsmLocation( void );
bool SIM8XX_sendSMS( void );
bool SIM8XX_callVoice( void );
uint16_t SIM8XX_UCS2Char(char *ucs2, char *simbol, uint16_t len);

// GPS
void SIM8xx_NavigationBasicInit(UART_HandleTypeDef *huart, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
bool SIM8xx_enableGPS(void);
bool SIM8xx_updateGPS(void);
void SIM8xx_handler1PPS(void);
void SIM8xx_enable1PPS(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);


//Должна быть вызвана в прерывании
void GSM_IRQHandler(void);
#endif
