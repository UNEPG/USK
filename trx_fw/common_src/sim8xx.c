//           _      ______       _____      _______ 
//     /\   | |    |  ____/\    / ____|  /\|__   __|
//    /  \  | |    | |__ /  \  | (___   /  \  | |   
//   / /\ \ | |    |  __/ /\ \  \___ \ / /\ \ | |   
//  / ____ \| |____| | / ____ \ ____) / ____ \| |   
// /_/    \_\______|_|/_/    \_\_____/_/    \_\_|   
//
//
// Author: Nursultan Uzbekov
// Date:   22.08.2019
//
#include "sim8xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>  
#include <stdio.h>
#include "stm32l0xx_hal.h"
#include "cmsis_os.h"


#define SIMCOM_USE_INTEGRATED_GPS       1       // Sim868 with integrated GPS
#define SIMCOM_USE_GPS_GSM_CONNECTED    1       // Sim868 GPS connected to GSM

/* Buffer's length must be select according to real messages frequency */
#define RXBUF_LEN            512 // must be power of 2
#define TXBUF_LEN            512 // must be power of 2
#define RXBUF_MSK            (RXBUF_LEN-1)
#define TXBUF_MSK            (TXBUF_LEN-1)

#define SIM8xx_PWRKEY_SET()        SIM8xx_PWRKEY_GPIOx->BSRR = SIM8xx_PWRKEY_GPIO_Pin
#define SIM8xx_PWRKEY_RESET()      SIM8xx_PWRKEY_GPIOx->BRR  = SIM8xx_PWRKEY_GPIO_Pin

static UART_HandleTypeDef* gsm_uart_handle; 

GPIO_TypeDef* SIM8xx_PWRKEY_GPIOx;
uint16_t SIM8xx_PWRKEY_GPIO_Pin;


static struct sim8xx_data simcom_data;

static volatile HAL_StatusTypeDef read_status;
static volatile uint8_t sb;
static char buffer[256];

// Cyrcle buffer variables
static volatile uint8_t rx_buf[RXBUF_LEN], tx_buf[TXBUF_LEN];
/* xx_i - counter of input bytes (tx - pushed for transmit, rx - received)
   xx_o - counter of output bytes (tx - transmitted, rx - parsed)
   xx_e - counter of echoed bytes */
static volatile uint16_t rx_i = 0, tx_o = 0;
static uint16_t rx_o = 0, tx_i = 0;
static volatile uint8_t tx_busy = 0;

// Static function prototypes
static void GSM_println(char* str);
static bool atCommandAndWait( char* cmd, char* waitStr, uint16_t timeout);
static bool atCommandAndWaitCNT( char* cmd, char* waitStr, int count, uint16_t timeout);
static bool getResponseValue( char* prefix, int index);
static void freeGSMBuffer(void);
static bool waitEx( char* str, uint16_t TimeOut);

// GSM Uart function prototypes
static bool GSM_available(void);
static uint8_t GSM_read(void);
static void transmit(uint8_t byte);

#if (SIMCOM_USE_INTEGRATED_GPS == 1)

#define SIM8xx_GPSEN_SET()        SIM8xx_GPSEN_GPIOx->BSRR = SIM8xx_GPSEN_GPIO_Pin
#define SIM8xx_GPSEN_RESET()      SIM8xx_GPSEN_GPIOx->BRR  = SIM8xx_GPSEN_GPIO_Pin

//static UART_HandleTypeDef* gps_uart_handle; 

GPIO_TypeDef* SIM8xx_GPSEN_GPIOx;
uint16_t SIM8xx_GPSEN_GPIO_Pin;
GPIO_TypeDef* SIM8xx_1PPS_BP_GPIOx;
uint16_t SIM8xx_1PPS_BP_GPIO_Pin;

#endif


struct gsm_operators_info
{
    char* name;
    uint16_t id;
    char* bal_cmd;
    char* bal_name_str;
    char* bal_name_end;
};

const struct gsm_operators_info gsm_operators_infos[OPERATORS_COUNT] =
{
    { .name = "Beeline",    .id = 40101, .bal_cmd = "*102#", .bal_name_str = "\0",        .bal_name_end = "tg" },
    { .name = "K-Cell",     .id = 40102, .bal_cmd = "*111#", .bal_name_str = "\0",        .bal_name_end = "\0" },              // TODO
    { .name = "Altel",      .id = 40107, .bal_cmd = "*111#", .bal_name_str = "\0",        .bal_name_end = "\0" },              // TODO
    { .name = "Tele2",      .id = 40177, .bal_cmd = "*111#", .bal_name_str = "Balans: ",  .bal_name_end = "KZT" },
};


void SIM8xx_basicInit( UART_HandleTypeDef *huart, struct sim8xx_data** data, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, bool with_pwrkey )
{
    if( huart == NULL )
    {
        //UsageFault_Handler();
    }
    
    *data = (struct sim8xx_data*) &simcom_data;
    gsm_uart_handle = huart;
    SIM8xx_PWRKEY_GPIOx = GPIOx;
    SIM8xx_PWRKEY_GPIO_Pin = GPIO_Pin;
    
    if(with_pwrkey)
    {
        SimCom_Delay(600);
        SIM8xx_PWRKEY_SET();
        SimCom_Delay(1100);
        SIM8xx_PWRKEY_RESET();
        SimCom_Delay(3500);
        atCommandAndWaitCNT("AT", "OK", 5, SIMCOM_DEFAULT_TERMINAL_TIMEOUT);
    }
    else
    {
        SIM8xx_PWRKEY_RESET();
    }
}

void SIM8xx_togglePwrKey( void )
{
    SimCom_Delay(100);
    SIM8xx_PWRKEY_RESET();
    SimCom_Delay(100);
    SIM8xx_PWRKEY_SET();
    SimCom_Delay(500);
}

bool SIM8xx_checkModem(void)
{
    int success = 0;
    for(int i=0; i<5; i++)
    {
        if (atCommandAndWait("AT", "OK", 100))
        {
            success++;
        }
        osDelay(100);
    }
    
    return (success == 5);
}

bool SIM8xx_modemInit( void )
{
    
    memset( (uint8_t*) &simcom_data, 0, sizeof(simcom_data));
    
    GSM_println("AT");
    SimCom_Delay(100);
    
    GSM_println("ATE0");
    if (isConnected())
    {
        SimCom_Delay(100);
        atCommandAndWait("AT+CSCS=\"IRA\"", "OK", SIMCOM_DEFAULT_TERMINAL_TIMEOUT);
        return initPrefs();
    }
    return false;
}

bool initPrefs( void )
{
    SIM8xx_getIMEI();
    if ( SIM8xx_getIMEI() && 
       ( strlen(simcom_data.IMEI) == 15) )
    {
        return true;
    }
    return false;
}

bool reInitSAPBR( void )
{
//    atCommandAndWait("AT+SAPBR=0,1", "OK", SIMCOM_DEFAULT_TERMINAL_TIMEOUT);
//    SimCom_Delay(1000);
//    bool r = atCommandAndWaitCNT("AT+SAPBR=1,1", "OK", SIMCOM_DEFAULT_TERMINAL_TIMEOUT);
//    SimCom_Delay(1000);
//    return r;
    if( atCommandAndWait("AT+SAPBR=1,1", "OK", SIMCOM_DEFAULT_TERMINAL_TIMEOUT) )
    {
        return true;
    }
    return false;
}

bool SIM8xx_getIMEI( void )
{
    //String res="";
    memset(simcom_data.IMEI, 0, sizeof(simcom_data.IMEI));
    if (atCommandAndWait("AT+GSN", "OK", SIMCOM_DEFAULT_TERMINAL_TIMEOUT))
    {
        getResponseValue("", 3);
        int zi=0;
        for(int zz=0; zz<strlen(simcom_data.response); zz++)
        {
            if(simcom_data.response[zz] >= '0' && simcom_data.response[zz] <= '9')
            {
                simcom_data.IMEI[zi] = simcom_data.response[zz];
                zi++;
            }
        }
        //memcpy(simcom_data.IMEI, simcom_data.response, strlen(simcom_data.response));
        return true;
    }
    return false;
}

bool SIM8xx_getGsmLocation( void )
{
    //String res="";
    if (atCommandAndWait("AT+CIPGSMLOC=1,1", "OK", 5000))
    {
        //String err=getResponseValue("+CIPGSMLOC: ", 0);
        getResponseValue("+CIPGSMLOC: ", 0);
        //GSM_UART.println("RES:"+err);
        if (strncmp(simcom_data.result, "601", 3) != 0)
        {
            //GSM_UART.println("Make string");
            ///gsm_long=
            getResponseValue( "+CIPGSMLOC: ",1);
            memcpy(simcom_data.gsm_long, simcom_data.result, strlen(simcom_data.result));
            //gsm_lat =
            getResponseValue( "+CIPGSMLOC: ",2);
            memcpy(simcom_data.gsm_latt, simcom_data.result, strlen(simcom_data.result));
            //gsm_time=
            getResponseValue( "+CIPGSMLOC: ",3);
            memcpy(simcom_data.gsm_date, simcom_data.result, strlen(simcom_data.result));
            
            getResponseValue( "+CIPGSMLOC: ",4);
            memcpy(simcom_data.gsm_time, simcom_data.result, strlen(simcom_data.result));
            
            snprintf(simcom_data.gsm_location, sizeof(simcom_data.gsm_location), "\"l\":\"%s\",\"w\":\"%s\"", simcom_data.gsm_long, simcom_data.gsm_latt);
            //res="\"l\":\""+gsm_long+"\",\"w\":\""+gsm_lat+"\"";
        }
        else
        {
            reInitSAPBR();
        }
    }
    //GSM_UART.println(res);
    //return res;
    return true;     
}

bool isConnected( void )
{
    SimCom_Delay(100);
    if ( atCommandAndWait( "AT+CREG?", "OK", SIMCOM_DEFAULT_TERMINAL_TIMEOUT) )
    {
        getResponseValue( "+CREG: ", 1);// == "1";
        if(strncmp(simcom_data.result, "1", 1) == 0)
        {
            return true;
        }
    }
    return false;
}

bool SIM8XX_sendSMS( void )
{
    memset(buffer, 0, sizeof(buffer));
    char phone_number[] = "+77000949407";
    char text_sms[] = "It`s unisat TRX";
    
    
    if ( !atCommandAndWait( "AT+CMGF=1", "OK", SIMCOM_DEFAULT_TERMINAL_TIMEOUT))
    {
        return false;
    }
    
    snprintf(buffer, sizeof(buffer), "AT+CMGS=\"%s\"%c", phone_number, 13);
    if ( !atCommandAndWait( buffer, ">", 1000))
    {
        return false;
    }
    SimCom_Delay(100);
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%s%c", text_sms, 26);
    if ( !atCommandAndWait( buffer, "OK", 1000))
    {
        return false;
    }
    return true;    
}

bool SIM8XX_callVoice( void )
{
    memset(buffer, 0, sizeof(buffer));
    char phone_number[] = "+77000949407";
    
    snprintf(buffer, sizeof(buffer), "ATD+ %s;%c", phone_number, 13);
    if ( !atCommandAndWait( buffer, "OK", 1000))
    {
        return false;
    }
    SimCom_Delay(12000);
    if ( !atCommandAndWait( "ATH\r", "OK", 100))
    {
        return false;
    }
    return true;    
}

int SIM8xx_getBattery( void )
{
    if (atCommandAndWait( "AT+CBC", "OK", 100)) 
    {
        getResponseValue( "+CBC: ", 1);
        int bat = strtol(simcom_data.result, NULL, 10);
        //strtoi
        return bat;
    }
    return -1;
}

int SIM8xx_getSignal( void )
{
    if (atCommandAndWait( "AT+CSQ", "OK", 100)) 
    {
        getResponseValue( "+CSQ: ", 0);
        int sig = strtol(simcom_data.result, NULL, 10);
        //strtoint
        return sig;
    }
    return -1;
}

float SIM8xx_getBalance( void )
{
    if( simcom_data.operator_type == 0 )    // неизвестен оператор
    {
        return -1;
    }
    
    struct gsm_operators_info* info = (struct gsm_operators_info*) &gsm_operators_infos[ simcom_data.operator_type - 10 ];
    
    snprintf( buffer, sizeof(buffer), "AT+CUSD=1,\"%s\"", info->bal_cmd );
    if( atCommandAndWait( buffer, "OK", 5000 ) )
    {
        
        atCommandAndWait( "", "OK", 15000 );
        
        getResponseValue( "\r\n+CUSD:" , 0);
        if( strtol( simcom_data.result, NULL, 10) != 1 )
        {
            return -1;
        }
        
        getResponseValue( "\r\n+CUSD:" , 1);
        if( strlen(simcom_data.result) != 0 )
        {
            char buf[32];
            char* ptr = (char*) &simcom_data.result;
            
            if( *ptr !=  '\"' )
            {
                return -1;
            }
            
            ptr++;
            
            if( SIM8XX_UCS2Char( ptr, buf, sizeof(buf) ) == sizeof(buf) )
            {
                
                ptr = buf;
                
                char* pcmp = info->bal_name_str;
                uint8_t pcmp_len = strlen( pcmp );
                
                if( ( pcmp_len != 0 ) &&                        // если есть что проверить
                    ( strncmp( ptr, pcmp, pcmp_len) != 0 ) )    // если не совпадает
                {
                    return -1;
                }
                
                ptr += pcmp_len;
                
                int bal = strtol( ptr, &ptr, 10 );
                
                if( *ptr != '.' )
                {
                    return -1;
                }
                
                ptr++;
                
                int temp = strtol( ptr, &ptr, 10 );
                
                ptr++;
                
                pcmp = info->bal_name_end;
                pcmp_len = strlen( pcmp );
                
                if( ( pcmp_len != 0 ) &&
                    ( strncmp( ptr, pcmp, pcmp_len) != 0 ) )
                {
                    return -1;
                }
                
                return (uint16_t)( bal );
            }
        }
    }
    return -1;
}

uint16_t SIM8xx_getOperatorID(void)
{
    atCommandAndWait( "AT+COPS=3,2", "OK", 100 );
    
    if( ( atCommandAndWait( "AT+COPS?", "OK", 1000 ) ) &&
        ( getResponseValue( "\r\n+COPS:" , 2) ) )
    {
        char* ptr = (char*) &simcom_data.result;
        if( *ptr == '\"' )
        {
            ptr++;
            uint16_t  id = strtol(ptr, NULL, 10);
            if( id > 40100 )
            {
                for(int i=0; i<OPERATORS_COUNT; i++)
                {
                    if( id == gsm_operators_infos[i].id )
                    {
                        simcom_data.operator_type = (enum op_id) (i + 10);
                        simcom_data.operator_id = id;
                        return id;
                    }
                }
            }
        }     
    }
    return 0xFFFF;
}


/************************************************************/
/********************* GPS main functions *******************/
/************************************************************/

#if ( SIMCOM_USE_INTEGRATED_GPS == 1 )

void SIM8xx_NavigationBasicInit(UART_HandleTypeDef *huart, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    if(huart != NULL)
    {
        
    }
    SIM8xx_GPSEN_GPIOx = GPIOx;
    SIM8xx_GPSEN_GPIO_Pin = GPIO_Pin;
}

bool SIM8xx_enableGPS(void)
{
#if (SIMCOM_USE_GPS_GSM_CONNECTED == 1)
    
    if (atCommandAndWait( "AT+CGNSPWR=1", "OK", 5000)) 
    {
        
        atCommandAndWait( "AT+CGNSIPR?", "OK", 1000);
        atCommandAndWait( "AT+CGNSIPR=115200", "OK", 1000);
        
        SIM8xx_GPSEN_SET();
        SimCom_Delay(1600+100);
        // $PMTK353,1,1,0,0,0*2B
        atCommandAndWait( "AT+CRFLOC=\"43.130280,76.552875\"", "OK", 1000);        
        return true;
    }
    return false;
#else
    SIM8xx_GPSEN_SET();
#endif
}

bool SIM8xx_updateGPS(void)
{
#if (SIMCOM_USE_GPS_GSM_CONNECTED == 1)
    if ( atCommandAndWait("AT+CGNSINF", "OK", 1000) )
    {
        if( (getResponseValue("+CGNSINF: ", 1)) &&
            (strncmp(simcom_data.result, "1", 1) == 0) )
        {

            // Index 2: Fix status
            simcom_data.gps_fix_status = true;
            
            // Index 3: UTC Date time            
            getResponseValue( "+CGNSINF: ", 2);
            if( strlen(simcom_data.result) == 18 )
            {
                memcpy(simcom_data.gps_datetime, simcom_data.result, strlen(simcom_data.result));
            }
            
            // Index 4: Latitude
            getResponseValue( "+CGNSINF: ", 3);
            if( strlen(simcom_data.result) > 8 )
            {
                simcom_data.gps_latitude = atof(simcom_data.result);
            }
            
            // Index 5: Longitude
            getResponseValue( "+CGNSINF: ", 4);
            if( strlen(simcom_data.result) > 8 )
            {
                simcom_data.gps_longitude = atof(simcom_data.result);
            }
            
            // Index 6: Altitude
            getResponseValue( "+CGNSINF: ", 5);
            if( strlen(simcom_data.result) >= 3 )
            {
                simcom_data.gps_altitude = atof(simcom_data.result);
            }
            
            // Index 7: Speed Over Ground
            getResponseValue( "+CGNSINF: ", 6);
            if( strlen(simcom_data.result) > 0 )
            {
                simcom_data.gps_speed_kph = atof(simcom_data.result);
            }
            
            // Index 8: Course Over Ground
            getResponseValue( "+CGNSINF: ", 7);
            if( strlen(simcom_data.result) > 0 )
            {
                simcom_data.gps_coarse = atof(simcom_data.result);
            }
            
            // Index 11: HDOP
            getResponseValue( "+CGNSINF: ", 10);
            if( strlen(simcom_data.result) > 0 )
            {
                simcom_data.gps_hdop = atof(simcom_data.result);
            }
            
            // Index 12: PDOP
            getResponseValue( "+CGNSINF: ", 11);
            if( strlen(simcom_data.result) > 0 )
            {
                simcom_data.gps_pdop = atof(simcom_data.result);
            }
            
            // Index 13: VDOP
            getResponseValue( "+CGNSINF: ", 12);
            if( strlen(simcom_data.result) > 0 )
            {
                simcom_data.gps_vdop = atof(simcom_data.result);
            }
            
            // Index 15: GPS Satellites in View
            getResponseValue( "+CGNSINF: ", 14);
            if( strlen(simcom_data.result) > 0 )
            {
                simcom_data.gps_in_view = strtol(simcom_data.result, NULL, 10);
            }
            
            // Index 16: GNSS Satellites Used
            getResponseValue( "+CGNSINF: ", 15);
            if( strlen(simcom_data.result) > 0 )
            {
                simcom_data.gps_in_used = strtol(simcom_data.result, NULL, 10);
            }
            
            // Index 17: GLONASS Satellites in View
            getResponseValue( "+CGNSINF: ", 16);
            if( strlen(simcom_data.result) > 0 )
            {
                simcom_data.glo_in_view = strtol(simcom_data.result, NULL, 10);
            }
            
            // Index 19: GLONASS Satellites in View
            getResponseValue( "+CGNSINF: ", 18);
            if( strlen(simcom_data.result) > 0 )
            {
                simcom_data.gps_cn = strtol(simcom_data.result, NULL, 10);
            }
            
            return true;
        }
        else if(strncmp(simcom_data.result, "0", 1) == 0)
        {
            simcom_data.gps_fix_status = false;
            
            if( (getResponseValue("+CGNSINF: ", 2)) &&
                (strlen(simcom_data.result) != 0) &&
                (strncmp(simcom_data.result, "20", 2) == 0) )
            {
                memcpy( simcom_data.gps_datetime, simcom_data.result, strlen(simcom_data.result));
            }
        }
        else if( (getResponseValue("+CGNSINF: ", 0)) &&
                 (strncmp(simcom_data.result, "0", 1) == 0) )
        {
            // Включаем GPS если выставлено что отключено
            SIM8xx_enableGPS();
        }
    }
    return false;
#else
    
#endif
}

void SIM8xx_enable1PPS(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    SIM8xx_1PPS_BP_GPIOx = GPIOx;
    SIM8xx_1PPS_BP_GPIO_Pin = GPIO_Pin;
}

void SIM8xx_handler1PPS(void)
{
    if( SIM8xx_1PPS_BP_GPIOx != NULL )
    {
        SIM8xx_1PPS_BP_GPIOx->ODR ^= SIM8xx_1PPS_BP_GPIO_Pin;
    }
}

#endif

/************************************************************/
/********************* Basic methods ************************/
/************************************************************/

bool atCommandAndWait( char* cmd, char* waitStr, uint16_t timeout)
{
    freeGSMBuffer();
    GSM_println(cmd);
    return waitEx( waitStr, timeout);
}

bool atCommandAndWaitCNT( char* cmd, char* waitStr, int count, uint16_t timeout)
{
    for (int i=0;i<count;i++)
    {
        freeGSMBuffer();
        SimCom_Delay(50);
        GSM_println(cmd);
        if ( waitEx( waitStr, 5000) )
        {
            return true;
        }
    }
    return false;
}


bool getResponseValue( char* prefix, int index)
{
    //String res = "";
    memset(simcom_data.result, 0, sizeof(simcom_data.result));
    int start = 0;
    char* p = simcom_data.response;
    int res_len = strlen(simcom_data.response);
    char ch;
    int result_cnt = 0;
    
    if ( strncmp(prefix, "", strlen(prefix)) != 0 )
    {
        p = strstr((const char*) simcom_data.response, prefix);
        p += strlen(prefix);
    }
    if ((p - simcom_data.response > 0) || strncmp(prefix, "", strlen(prefix)) == 0)
    {
        //Serial.println((String)start+">"+response.substring(start));
        uint8_t param = 0;
        for (int i = start; i < res_len; i++)
        {
            //ch = simcom_data.response[i];
            ch = *p++;
            if (ch == ',' || ch < ' ')
            {
                param++;
            }
            else
            {
                if (param == index)
                {
                  simcom_data.result[result_cnt] = ch;
                  result_cnt++;
                }
            }
            if (param > index || param > 250) {
            break;
            }
        }
    }
   
    return true; 
}

void freeGSMBuffer(void)
{
    while(GSM_available())
    {
        uint8_t flush = GSM_read();
    }
}

bool waitEx( char* str, uint16_t TimeOut)
{
    //response = "";
    memset(simcom_data.response, 0, sizeof(simcom_data.response));
    if (strncmp(str, "", strlen(str)) == 0)
    {
        return true;
    }
    uint16_t md_resp_cnt = 0;
    uint64_t timer = SimCom_GetTick();
    char c;
    bool found = false;
    int good = 0;
    uint8_t in_str_len = strlen(str);
    char strc[in_str_len];
    strncpy(strc, str, in_str_len);
    
    while (SimCom_GetTick() - timer < TimeOut) {
        SimCom_Delay(25);
        while (GSM_available())
        {
            //сохраняем входную строку в переменную v
            c = (char) GSM_read();
            if (c > 0)
            {
                //if (response.length()>20) {break;}
                simcom_data.response[md_resp_cnt] = (char) c;
                md_resp_cnt++;
                if (strc[good] == c)
                {
                    good++;
                    if (in_str_len == good)
                    {
                        found = true;
                        break;
                    }
                }
                else
                {
                  good = 0;
                }
            }
            if (found)
            {
                break;
            }
        }
        if (found)
        {
            break;
        }
        
        if (strstr(simcom_data.response, "ERROR") != NULL)
        {
            break;
        }
    }
    return found;
}

uint16_t SIM8XX_UCS2Char(char *ucs2, char *simbol, uint16_t len)
{
	uint16_t UCS2Code = 0x0000;
	uint8_t nr = 0;
    
	// process
	for (uint8_t s = 0; s < (len*4); s = s + 4)
	{
		if (ucs2[0+s] >= '0' && ucs2[0+s] <= '9') UCS2Code = (ucs2[0+s] - '0');
		if (ucs2[0+s] >= 'A' && ucs2[0+s] <= 'F') UCS2Code = (ucs2[0+s] + 10 - 'A');
		if (ucs2[1+s] >= '0' && ucs2[1+s] <= '9') UCS2Code = (UCS2Code << 4) + (ucs2[1+s] - '0');
		if (ucs2[1+s] >= 'A' && ucs2[1+s] <= 'F') UCS2Code = (UCS2Code << 4) + (ucs2[1+s] + 10 - 'A');
		if (ucs2[2+s] >= '0' && ucs2[2+s] <= '9') UCS2Code = (UCS2Code << 4) + (ucs2[2+s] - '0');
		if (ucs2[2+s] >= 'A' && ucs2[2+s] <= 'F') UCS2Code = (UCS2Code << 4) + (ucs2[2+s] + 10 - 'A');
		if (ucs2[3+s] >= '0' && ucs2[3+s] <= '9') UCS2Code = (UCS2Code << 4) + (ucs2[3+s] - '0');
		if (ucs2[3+s] >= 'A' && ucs2[3+s] <= 'F') UCS2Code = (UCS2Code << 4) + (ucs2[3+s] + 10 - 'A');
		
		if ((UCS2Code >= 32) && (UCS2Code <= 126)) simbol[nr] = UCS2Code;
		if ((UCS2Code >= 1040) && (UCS2Code <= 1103)) simbol[nr] = UCS2Code - 848;
		
		nr++;
	}
	return nr;
}

void GSM_println(char* str)
{
    int str_len = strlen(str);
    for(int istr = 0; istr<str_len; istr++)
    {
        transmit((uint8_t) *str++);
    }
    //transmit(0x0A);
    transmit(0x0D);
}

bool GSM_available(void)
{
    return (rx_i != rx_o);
}

uint8_t GSM_read(void)
{
    uint8_t data_to_return = rx_buf[RXBUF_MSK & rx_o];
    rx_o++;
    return data_to_return;
}


void transmit(uint8_t byte) 
{
    tx_buf[TXBUF_MSK & tx_i] = byte;
    tx_i++;
    tx_busy = 1;
    __HAL_UART_ENABLE_IT(gsm_uart_handle, UART_IT_TXE);
}

void GSM_IRQHandler(void)
{
    if( ( __HAL_UART_GET_FLAG(gsm_uart_handle, UART_FLAG_RXNE ) != RESET ) && 
        ( __HAL_UART_GET_IT_SOURCE(gsm_uart_handle, UART_IT_RXNE ) != RESET ) )
    {
        
        rx_buf[rx_i & RXBUF_MSK] = (uint8_t)(gsm_uart_handle->Instance->RDR & 0x00FF);
        rx_i++;
        /* Clear RXNE interrupt flag */
        __HAL_UART_CLEAR_FLAG(gsm_uart_handle, UART_FLAG_RXNE);
        
    }
    
    if( ( __HAL_UART_GET_FLAG(gsm_uart_handle, UART_FLAG_TXE) != RESET ) && 
        ( __HAL_UART_GET_IT_SOURCE(gsm_uart_handle, UART_IT_TXE) != RESET ) )
    {
        if (tx_i == tx_o)
        {
            __HAL_UART_DISABLE_IT(gsm_uart_handle, UART_IT_TXE);
            __HAL_UART_ENABLE_IT(gsm_uart_handle, UART_IT_TC);
        } 
        else 
        {
            gsm_uart_handle->Instance->TDR = (uint8_t)(tx_buf[TXBUF_MSK & tx_o] & (uint8_t)0xFF);
            tx_o++;
        }
    }
    
    if( ( __HAL_UART_GET_FLAG(gsm_uart_handle, UART_FLAG_TC) != RESET ) &&
        ( __HAL_UART_GET_IT_SOURCE(gsm_uart_handle, UART_IT_TC) != RESET ) )
    {
        tx_busy = 0;
        __HAL_UART_DISABLE_IT(gsm_uart_handle, UART_IT_TC);
    }
    
    if( ( gsm_uart_handle->Instance->ISR & USART_ISR_NE ) || 
        ( gsm_uart_handle->Instance->ISR & USART_ISR_FE ) || 
        ( gsm_uart_handle->Instance->ISR & USART_ISR_PE ) )
    {
        // TODO: Noise, Parity error
        __HAL_UART_CLEAR_NEFLAG(gsm_uart_handle);
        __HAL_UART_CLEAR_PEFLAG(gsm_uart_handle);
        __HAL_UART_CLEAR_FEFLAG(gsm_uart_handle);        
    }
    
    /* And never call default handler */
    HAL_UART_IRQHandler(gsm_uart_handle);
}
