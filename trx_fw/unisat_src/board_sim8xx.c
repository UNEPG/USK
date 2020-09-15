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

#include "stm32l0xx_hw_conf.h"
#include "board_commands.h"
#include "board_sim8xx.h"
#include "cmsis_os.h"
#include "sim8xx.h"
#include <stdlib.h>
#include "hw.h"


GSM_States GSM_State;
GPS_States GPS_State;
struct sim8xx_data* simcom;

UART_HandleTypeDef gsm_usart5;

bool gsm_needs_reinit = false;
uint32_t gsm_ts = 0;
uint32_t gps_ts = 0;
uint32_t timestamp = 0;

static void SimCom_Init(bool gps_uart_init);
static void GSM_UARTx_Init(void);
static void GSM_MspInit(UART_HandleTypeDef *huart);
static void GSM_IoInit(void);
static void GSM_IoDeInit(void);
static void GSM_SimComIoInit(void);

static void GPS_SimComIoInit(void);


// FreeRTOS variables
osThreadId NavigationTaskHandle;
osThreadId CellularTaskHandle;



static void SimCom_Init(bool gps_uart_init)
{
    GSM_State = GSM_Init;
    GSM_SimComIoInit();
    SIM8xx_basicInit(&gsm_usart5, &simcom, GSM_PWRKEY_PORT, GSM_PWRKEY_PIN, true);
    GSM_UARTx_Init();
    
    if(gps_uart_init)
    {
        GPS_State = GPS_STATE_Init;
        GPS_SimComIoInit();
        SIM8xx_NavigationBasicInit(NULL, GPS_PWREN_PORT, GPS_PWREN_PIN);
        SIM8xx_enable1PPS(GPS_1PPS_LED_PORT, GPS_1PPS_LED_PIN);
    }
}

void SimcomTask(void const *arguments)
{
    SimCom_Init( true );
    
    SIM8xx_togglePwrKey();
    
    osDelay(500);
    
    while( !SIM8xx_checkModem() )
    {
        osDelay(500);
    }
    
    simcom->error = true;
    int tries = 5;
    while( tries != 0 && simcom->error)
    {
        simcom->error = !SIM8xx_modemInit();
        osDelay(2000);
        tries--;
    }
    
    if( !simcom->error )
    {
        SIM8xx_getOperatorID();
        simcom->balance = SIM8xx_getBalance();
        SIM8xx_enableGPS();
    }
    
    while(1)
    {
        //SIM8XX_callVoice();
        if( SIM8xx_updateGPS() )
        {
            //GPS OK
        }
        SIM8xx_checkSystemTimeAndGPS(false);
        osDelay(10000);
    }
    
}

/************************************************************/
/******************** Function methods **********************/
/************************************************************/

bool SIM8xx_checkSystemTimeAndGPS(bool force_update)
{
    struct tm gpsTime;
    time_t sys_ts = 0;
    time_t gps_ts = 0;
    
    bool sys_time_ok = getSystemTime(&sys_ts);
    
    bool gps_time_ok = SIM8xx_getGpsTime(&gpsTime, &gps_ts);
    
    if( (sys_time_ok) && 
        (gps_time_ok) && 
        (force_update || (labs((int32_t)(sys_ts - gps_ts)) > SYSTEM_TIME_CORRECTION_TH)) )
    {
        // Если получилось взять время GPS
        // Если принудительное обновление времени системы или разница времени больше TH
        if( updateSystemTime(&gpsTime) )
        {
            return true;
        }
    }
    return false;
}

bool SIM8xx_getGpsTime( struct tm* pTime, time_t* ts )
{
    if ( strlen(simcom->gps_datetime) != 0 )
    {
        uint8_t* pntr = (uint8_t*) &simcom->gps_datetime[2];
        
        pTime->tm_year = (*pntr++ - '0') * 10;
        pTime->tm_year += (*pntr++ - '0');
        pTime->tm_year += 100;  // Для библиотеки time.h
        pTime->tm_mon = (*pntr++ - '0') * 10;
        pTime->tm_mon += (*pntr++ - '0');
        pTime->tm_mon -= 1;     // Для библиотеки time.h
        pTime->tm_wday = 0;
        pTime->tm_mday = (*pntr++ - '0') * 10;
        pTime->tm_mday += (*pntr++ - '0');
        
        pTime->tm_hour = (*pntr++ - '0') * 10;
        pTime->tm_hour += (*pntr++ - '0');
        pTime->tm_min = (*pntr++ - '0') * 10;
        pTime->tm_min += (*pntr++ - '0');
        pTime->tm_sec = (*pntr++ - '0') * 10;
        pTime->tm_sec += (*pntr++ - '0');
        
        *ts = mktime(pTime);
        return true;
    }
    return false;
}

/************************************************************/
/********************** Init methods ************************/
/************************************************************/

void GSM_UARTx_Init(void)
{
    /*** Configure the UART peripheral ***/
    gsm_usart5.Instance        = GSM_USARTx;
    gsm_usart5.Init.BaudRate   = 115200;
    gsm_usart5.Init.WordLength = UART_WORDLENGTH_8B;
    gsm_usart5.Init.StopBits   = UART_STOPBITS_1;
    gsm_usart5.Init.Parity     = UART_PARITY_NONE;
    gsm_usart5.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    gsm_usart5.Init.Mode       = UART_MODE_TX_RX;
    gsm_usart5.MspInitCallback = &GSM_MspInit;

    if (HAL_UART_Init(&gsm_usart5) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }
    
//    GSM_USARTx->CR3 |= USART_CR3_EIE;
//    GSM_USARTx->CR1 |= (USART_CR1_PEIE | USART_CR1_RXNEIE);
    __HAL_UART_ENABLE_IT(&gsm_usart5, UART_IT_RXNE);
    HAL_NVIC_SetPriority(USART4_5_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART4_5_IRQn);
}

void GSM_MspInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == GSM_USARTx)
    {
        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /* Enable GPIO TX/RX clock */
        GSM_USARTx_TX_GPIO_CLK_ENABLE();
        GSM_USARTx_RX_GPIO_CLK_ENABLE();

        /* Enable USARTx clock */
        GSM_USARTx_CLK_ENABLE();

        /*##-2- Configure peripheral GPIO ##########################################*/
        /* UART  pin configuration  */
        GSM_IoInit();
        
		//__HAL_UART_ENABLE_IT(&gsm_usart5, UART_IT_RXNE);
        
        /* NVIC for USART */
        //HAL_NVIC_SetPriority(GSM_USARTx_IRQn, GSM_USARTx_Priority, 1);
        //HAL_NVIC_EnableIRQ(GSM_USARTx_IRQn);
    }
}

void GSM_MspDeInit(UART_HandleTypeDef *huart)
{
    GSM_IoDeInit();

    GSM_USARTx_FORCE_RESET();
    GSM_USARTx_RELEASE_RESET();
}

static void GSM_IoInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct = {0};
    /* Enable GPIO clocks */
    GSM_USARTx_TX_GPIO_CLK_ENABLE();
    GSM_USARTx_RX_GPIO_CLK_ENABLE();
        
    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = GSM_USARTx_TX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GSM_USARTx_TX_AF;

    HAL_GPIO_Init(GSM_USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

    /* UART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = GSM_USARTx_RX_PIN;
    GPIO_InitStruct.Alternate = GSM_USARTx_RX_AF;

    HAL_GPIO_Init(GSM_USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
}

static void GSM_SimComIoInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct = {0};
    /* Enable GPIO clocks */
    GSM_PWRKEY_GPIO_CLK_ENABLE();
    GSM_RI_GPIO_CLK_ENABLE();
    
    GSM_PWRKEY_PORT->BRR = GSM_PWRKEY_PIN;

    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_LOW;

    /* GSM Power Key pin configuration  */
    GPIO_InitStruct.Pin = GSM_PWRKEY_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    
    HAL_GPIO_Init(GSM_PWRKEY_PORT, &GPIO_InitStruct);
    
    /* GSM Ring indicator pin configuration  */
    GPIO_InitStruct.Pin = GSM_RI_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    
    HAL_GPIO_Init(GSM_RI_PORT, &GPIO_InitStruct);
    
    HAL_NVIC_SetPriority(EXTI2_3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
}

void GSM_IoDeInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    
    GSM_USARTx_TX_GPIO_CLK_ENABLE();
    
    GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    
    GPIO_InitStructure.Pin =  GSM_USARTx_TX_PIN ;
    HAL_GPIO_Init(GSM_USARTx_TX_GPIO_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.Pin =  GSM_USARTx_RX_PIN ;
    HAL_GPIO_Init(GSM_USARTx_RX_GPIO_PORT, &GPIO_InitStructure);
}


static void GPS_SimComIoInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct = {0};
    /* Enable GPIO clocks */
    GPS_PWREN_GPIO_CLK_ENABLE();
    GPS_1PPS_GPIO_CLK_ENABLE();
    
    GPS_PWREN_PORT->BRR = GPS_PWREN_PIN;

    
    /* GPS Power En pin configuration  */
    GPIO_InitStruct.Pull    = GPIO_NOPULL;
    GPIO_InitStruct.Speed   = GPIO_SPEED_LOW;
    GPIO_InitStruct.Pin     = GPS_PWREN_PIN;
    GPIO_InitStruct.Mode    = GPIO_MODE_OUTPUT_PP;
    
    HAL_GPIO_Init(GPS_PWREN_PORT, &GPIO_InitStruct);
    
    /* GPS 1PPS pin configuration  */
    GPIO_InitStruct.Pull    = GPIO_PULLDOWN;
    GPIO_InitStruct.Pin     = GPS_1PPS_PIN;
    GPIO_InitStruct.Mode    = GPIO_MODE_IT_RISING;
    
    HAL_GPIO_Init(GPS_1PPS_PORT, &GPIO_InitStruct);
    
    HW_GPIO_SetIrq( GPS_1PPS_PORT, GPS_1PPS_PIN, 1, (GpioIrqHandler*) SIM8xx_handler1PPS );

#ifdef GS_BOARD
    /* GPS 1PPS LED pin configuration  */
    GPS_1PPS_LED_GPIO_CLK_ENABLE();
    GPS_1PPS_LED_PORT->BRR  = GPS_1PPS_LED_PIN;
    
    GPIO_InitStruct.Pull    = GPIO_NOPULL;
    GPIO_InitStruct.Speed   = GPIO_SPEED_LOW;
    GPIO_InitStruct.Pin     = GPS_1PPS_LED_PIN;
    GPIO_InitStruct.Mode    = GPIO_MODE_OUTPUT_PP;
    
    HAL_GPIO_Init(GPS_1PPS_LED_PORT, &GPIO_InitStruct);
#endif

    HAL_NVIC_SetPriority(EXTI4_15_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

void GSM_USARTx_IRQHandler(void)
{
    GSM_IRQHandler();
}
