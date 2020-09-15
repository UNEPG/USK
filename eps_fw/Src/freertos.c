/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "adc.h"
#include "tim.h"
#include "string.h"
#include "stdbool.h"
#include "math.h"
#include "stm32l0xx_it.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


//******************* ISL9237 CONTROL REGISTER*********************//

#define I2C_DEV_WRITE_ADR 0x12 // ISL9237_HEX format !!!
#define I2C_DEV_READ_ADR 0x13  // Read 

#define ISL_WRITE 0x12 // isl9237 i2c write adress  
#define ISL_READ  0x13 // isl9237 i2c read adress

#define DEFAULT 0x00 //

#define ADAPTER1_ADR 0x3f //  AdapterCurrentLimit1 REGISTER
#define ADAPTER2_ADR 0x3b //  AdapterCurrentLimit2 REGISTER

#define CHARGE_ADR  0x14 //   ChargeCurrentLimit REGISTER
#define INF_REGISTOR 0x3A //  INFORMATION REGISTER

#define PROCHOT_AC 0x47 //  PROCHOT# Threshold for Adapter Overcurrent Condition
#define PROCHOT_DC 0x48 //  PROCHOT# Threshold for Battery Over Discharging Current Condition

#define CONTROL0_ADR 0x39 // Configure various charger options 
#define IN_REG_ENB 0x04  //  disables or enables the input voltage regulation loop
#define SMB_TIMEOUT_DIS 0x80  //Disable the SMBus timeout function

#define CONTROL1_ADR 0x3c // CONTROL1 REGISTER 0x3CH
#define LEARN_MODE_ENB 0x1000 // the Battery Learn mode - enables
#define BMON_ENG  0x10 //  BMON as the output 
#define PSYS_ENB  0x08 //  enables or disable system power monitor PSYS function

#define CONTROL2_ADR 0x3d // CONTROL2 REGISTER 0x3DH
#define ADP_CURENT_ENG 0x1000 // enables or disables the two-level adapter current limit function

//******************** define mcu's adc calibrate **********************// 
#define VREFINT_CAL_ADDR ((uint16_t*)((uint32_t)0x1FF80078))
#define VDDD_MCU_VALUE 3300.0

#define TEMP130_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FF8007E))
#define TEMP30_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FF8007A))
#define VDD_CALIB ((uint16_t) (300))
#define VDD_APPLI ((uint16_t) (330))

#define ADC_CHANNELS 18
#define ADC_MAX_VALUE 4095.0

//******************* define adc divider coefficient  ******************//

#define     ADAPT_COEFT     4.22

#define     BAT1C_COEFT     2 
#define     BAT2C_COEFT     3.55 

#define     BC_5V_COEFT     3   
#define     BC_3V_COEFT     1.5

#define     SOLAR_COEFT     1.4979
#define     VDD_MCU_COEFT   3000 

#define     SHUNT_RES10     0.01
#define     SHUNT_RES20     0.02
#define     GAIN_CS         100.0
#define     GAIN_ISL        18.0


#define     TEMP_CONST1     5.506
#define     TEMP_CONST2     0.00176
#define     TEMP_CONST3     870.6

//******************* define PID control begin *************************//

#define T_POINT  20
#define Kp  290
#define Ti 19
#define TD 600
#define POWER_MAX 4095
#define POWER_MIN  0 
#define PID_MAX  1000
#define PID_MIN 0
#define ADC_Kp  4.095

//******************* define PID control end ***************************//
extern I2C_HandleTypeDef hi2c3;
extern TIM_HandleTypeDef htim3;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
uint16_t adc_data[ADC_CHANNELS];
uint16_t adc_data_copy[ADC_CHANNELS];


bool i2c_flag = false ;
volatile bool cplt_flag = false ; // interupt



uint16_t current_ch1 ;
uint16_t current_ch2 ;
uint16_t current_ch3 ;
uint16_t current_ch4 ;
uint16_t current_ch5 ;


uint16_t solar_ch_x ;
uint16_t solar_ch_y ; 
uint16_t solar_ch_z ;

uint16_t buck_5v ;
uint16_t buck_3v3;

uint16_t bat_volts_2c;
uint16_t bat_volts_1c;

float temp_left_bat;
float temp_right_bat;

uint16_t adapt_voltage;
uint16_t adap_current_isl;
uint16_t bat_current_isl;

uint16_t temp_mcu ;
uint16_t vdda_mcu ;

////////////// Begin ISL9237 Charger I2C variables //////////////////

HAL_StatusTypeDef i2c_write ;
HAL_StatusTypeDef i2c_read ;

 uint8_t read_adap1[]={0,0};
 uint8_t read_adap2[]={0,0};
 
 uint8_t read_charge_limit[]={0,0};

 uint8_t read_control0[]={0,0};
 uint8_t read_control1[]={0,0};
 uint8_t read_control2[]={0,0};
 
 uint8_t read_inf_registor[]={0,0};
 
 uint8_t read_ac_prochot[]={0,0};
 uint8_t read_dc_prochot[]={0,0};
 
 ////////////////// Write Settings//////////////////////////////////
 uint16_t write_adap1_usb = 500;
 uint16_t write_adap1 =1000;
 uint16_t write_adap2 =1500;
 uint16_t write_charge_limit = 1000;
 
 uint16_t write_ac_prochot = 2000;
 uint16_t write_dc_prochot = 3000;
 
 
 uint16_t write_control0 = 0x00; 
 uint16_t write_control1 = 0x0000 | BMON_ENG | PSYS_ENB ; 
 uint16_t write_control1_togle = 0x0000 | PSYS_ENB ;
 uint16_t write_control2 = 0x0000 | ADP_CURENT_ENG ; 
 
 uint16_t control1_reg_mod = 0x00;
 
 ////////////// End ISL9237 Charger I2C variables //////////////////
 
 
////////////// Begin PID regulator for Climate control /////////////

    float error, error_0, Integral ,Integral_regul,  cInt, differential, PID;
    int16_t power ;

///////////// End PID regulator for Climate control ////////////////
/****************Start Buck Regulator variables *******************/

bool pg_3v3_flag = false;
bool pg_5v_flag = false;

bool sys_ok_flag = false;

/******************End Buck Regulator variables *******************/
/*struct fault_manage 
{
char chanell_name [60];
bool is_fault_channell ;
uint16_t is_fault_count ;
GPIO_TypeDef* en_gpio;
uint16_t en_pin;
GPIO_TypeDef* reset_gpio;
uint16_t reset_pin;
};*/
struct fault ch [] = 
{
    {.fault_state = 0, .fault_cnt = 0, .en_gpio = EN_CH1_GPIO_Port , .en_pin = EN_CH1_Pin ,.rst_gpio = RST_CH1_GPIO_Port, .rst_pin = RST_CH1_Pin},  // CH1
    {.fault_state = 0, .fault_cnt = 0, .en_gpio = EN_CH2_GPIO_Port , .en_pin = EN_CH2_Pin ,.rst_gpio = RST_CH2_GPIO_Port, .rst_pin = RST_CH2_Pin},  // CH2
    {.fault_state = 0, .fault_cnt = 0, .en_gpio = EN_CH3_GPIO_Port , .en_pin = EN_CH3_Pin ,.rst_gpio = RST_CH3_GPIO_Port, .rst_pin = RST_CH3_Pin},  // CH3
    {.fault_state = 0, .fault_cnt = 0, .en_gpio = EN_CH4_GPIO_Port , .en_pin = EN_CH4_Pin ,.rst_gpio = RST_CH4_GPIO_Port, .rst_pin = RST_CH4_Pin},  // CH4
    {.fault_state = 0, .fault_cnt = 0, .en_gpio = EN_CH5_GPIO_Port , .en_pin = EN_CH5_Pin ,.rst_gpio = RST_CH5_GPIO_Port, .rst_pin = RST_CH5_Pin},  // CH5
    {.fault_state = 0, .fault_cnt = 0, .en_gpio = EN_3V3BC_GPIO_Port , .en_pin = EN_3V3BC_Pin ,.rst_gpio = NULL, .rst_pin = 0},     //      3V3_CH
    {.fault_state = 0, .fault_cnt = 0, .en_gpio = EN_5VBC_GPIO_Port ,  . en_pin = EN_5VBC_Pin ,.rst_gpio = NULL, .rst_pin = 0},       //      5V_CH
    {.fault_state = 0, .fault_cnt = 0, .en_gpio = EN_7_4VCH_GPIO_Port , .en_pin = EN_7_4VCH_Pin ,.rst_gpio = NULL, .rst_pin = 0},   //     7V4_CH
} ;


/////////////// SemaphoreHandle FAULT detect //////////////////////
xSemaphoreHandle  alert ;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId ADCTaskHandle;
osThreadId LedTaskHandle;
osThreadId myClimateHandle;
osThreadId Fault_TaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void comp_adc (struct __DMA_HandleTypeDef * hdma);
void isl9237_i2c (void);
void start_pross (void);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
extern void Start_ADCTask(void const * argument);
void StartTask03(void const * argument);
void Start_PIDtask(void const * argument);
void StartTask_Fault05(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
  
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}                   
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 64);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of ADCTask */
  osThreadDef(ADCTask, Start_ADCTask, osPriorityNormal, 0, 128);
  ADCTaskHandle = osThreadCreate(osThread(ADCTask), NULL);

  /* definition and creation of LedTask */
  osThreadDef(LedTask, StartTask03, osPriorityIdle, 0, 64);
  LedTaskHandle = osThreadCreate(osThread(LedTask), NULL);

  /* definition and creation of myClimate */
  osThreadDef(myClimate, Start_PIDtask, osPriorityNormal, 0, 64);
  myClimateHandle = osThreadCreate(osThread(myClimate), NULL);

  /* definition and creation of Fault_Task */
  osThreadDef(Fault_Task, StartTask_Fault05, osPriorityHigh, 0, 128);
  Fault_TaskHandle = osThreadCreate(osThread(Fault_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  alert = xSemaphoreCreateCounting( 5, 0 );
  
  
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  
 isl9237_i2c ();
  /* Infinite loop */
  for(;;)
  {   
         if (adapt_voltage > 4500 && adapt_voltage <= 5100)
             {
                i2c_flag = true ;
                i2c_write = HAL_I2C_Mem_Write(&hi2c3,ISL_WRITE, CONTROL1_ADR,1,(uint8_t*)&write_control1_togle,2,100);
                i2c_write = HAL_I2C_Mem_Write(&hi2c3,ISL_WRITE, ADAPTER1_ADR,1,(uint8_t*)&write_adap1_usb,2,100);
                i2c_write = HAL_I2C_Mem_Write(&hi2c3, ISL_WRITE, CHARGE_ADR, 1, (uint8_t*) &write_charge_limit, 2, 100); 
                
                if(sys_ok_flag == true){
                
                // Activ task ;
                } else { 
                    
                    // disactive task
                    
                }
                
              } 
               else if (adapt_voltage > 5100)
              {
               i2c_flag = true ;
               i2c_write = HAL_I2C_Mem_Write(&hi2c3,ISL_WRITE, CONTROL1_ADR,1,(uint8_t*)&write_control1_togle,2,100);
               i2c_write = HAL_I2C_Mem_Write(&hi2c3,ISL_WRITE, ADAPTER1_ADR,1,(uint8_t*)&write_adap1,2,100);
               i2c_write = HAL_I2C_Mem_Write(&hi2c3, ISL_WRITE, CHARGE_ADR, 1, (uint8_t*) &write_charge_limit, 2, 100);
               if(sys_ok_flag == true){
                
                // Activ task ;
                }
                   
                  
              }
              else 
                {
               i2c_flag = false ;
               i2c_write = HAL_I2C_Mem_Write(&hi2c3,ISL_WRITE, CONTROL1_ADR,1,(uint8_t*)&write_control1,2,100);
                }
        
    osDelay(10000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the LedTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void const * argument)
{
  /* USER CODE BEGIN StartTask03 */

  /* Infinite loop */
  for(;;)
  {
    
    HAL_GPIO_TogglePin(INT_IN_GPIO_Port, INT_IN_Pin);
  
      
       osDelay(300);
    
  }

  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_Start_PIDtask */
/**
* @brief Function implementing the myClimate thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_PIDtask */
void Start_PIDtask(void const * argument)
{
  /* USER CODE BEGIN Start_PIDtask */
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3); 
  /* Infinite loop */
  for(;;)
  {
        error_0 = error; 
        error = T_POINT - temp_right_bat; 
        //Integral = Integral+(error+ error_0)/2 ; // integral sinhro
        //Integral_regul =  Integral * (1/Ti);     // Integral regul 
        Integral_regul +=(Ti*error) ; 
      if(Integral_regul > PID_MAX) Integral_regul = PID_MAX;
      else if(Integral_regul < PID_MIN) Integral_regul = PID_MIN ;
        
       // differential =   TD * (error - error_0);    
      differential = (error - error_0)/100 ;
        PID = (Kp*error +Integral_regul + TD*differential ) ; 
        power = PID * ADC_Kp ;
       if(power >= POWER_MAX) power = POWER_MAX ;
       if(power <= 0) power = POWER_MIN ;
       htim3.Instance -> CCR3 = (uint32_t)power;

    osDelay(250);
  }
  /* USER CODE END Start_PIDtask */
}

/* USER CODE BEGIN Header_StartTask_Fault05 */
/**
* @brief Function implementing the Fault_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask_Fault05 */
void StartTask_Fault05(void const * argument)
{
  /* USER CODE BEGIN StartTask_Fault05 */
  /* Infinite loop */
  for(;;)
  {
    if(xSemaphoreTake (alert ,portMAX_DELAY) == pdTRUE){
         
      for(int i = 0; i<=(sizeof(ch)/ sizeof(ch)[0]); i++){
      if ( ch[i].fault_state == true){
          ch[i].fault_cnt ++;
          ch[i].fault_state = false ; 
        if (ch[i].rst_gpio != NULL){  
          switch (ch[i].fault_cnt) {
              case 1 :
              HAL_GPIO_WritePin(ch[i].en_gpio,ch[i].en_pin,GPIO_PIN_RESET);
                 osDelay(2000);
              HAL_GPIO_WritePin(ch[i].rst_gpio,ch[i].rst_pin,GPIO_PIN_SET);
                    osDelay(1000);
              HAL_GPIO_WritePin(ch[i].rst_gpio,ch[i].rst_pin,GPIO_PIN_RESET);
              HAL_GPIO_WritePin(ch[i].en_gpio,ch[i].en_pin,GPIO_PIN_SET); 
              break;
              case 2 :
                  HAL_GPIO_WritePin(ch[i].en_gpio,ch[i].en_pin,GPIO_PIN_RESET);
                  osDelay(3000);
                  HAL_GPIO_WritePin(ch[i].rst_gpio,ch[i].rst_pin,GPIO_PIN_SET);
                  osDelay(1000);
                  HAL_GPIO_WritePin(ch[i].rst_gpio,ch[i].rst_pin,GPIO_PIN_RESET);
                  HAL_GPIO_WritePin(ch[i].en_gpio,ch[i].en_pin,GPIO_PIN_SET); 
              break ;
              case  3:
                  HAL_GPIO_WritePin(ch[i].en_gpio,ch[i].en_pin,GPIO_PIN_RESET);
                  osDelay(1000);
                  HAL_GPIO_WritePin(ch[i].rst_gpio,ch[i].rst_pin,GPIO_PIN_SET);
                  osDelay(1000);
                  HAL_GPIO_WritePin(ch[i].rst_gpio,ch[i].rst_pin,GPIO_PIN_RESET);
                  HAL_GPIO_WritePin(ch[i].en_gpio,ch[i].en_pin,GPIO_PIN_RESET);
              break;
              default:
                  ch[i].fault_cnt = 0;
                  ch[i].fault_state = false ;
                  HAL_GPIO_WritePin(ch[i].rst_gpio,ch[i].rst_pin,GPIO_PIN_SET);
                  osDelay(1000);
                  HAL_GPIO_WritePin(ch[i].rst_gpio,ch[i].rst_pin,GPIO_PIN_RESET);
                  HAL_GPIO_WritePin(ch[i].en_gpio,ch[i].en_pin,GPIO_PIN_RESET);
              break;
             }
        } else {
            
              switch (ch[i].fault_cnt) {
              case 1 :
              HAL_GPIO_WritePin(ch[i].en_gpio,ch[i].en_pin,GPIO_PIN_RESET);
                 osDelay(1000);
              HAL_GPIO_WritePin(ch[i].en_gpio,ch[i].en_pin,GPIO_PIN_SET);
              break;
               case 2 :
              HAL_GPIO_WritePin(ch[i].en_gpio,ch[i].en_pin,GPIO_PIN_RESET);
                 osDelay(2000);
              HAL_GPIO_WritePin(ch[i].en_gpio,ch[i].en_pin,GPIO_PIN_SET);
              break;
               case 3 :
              HAL_GPIO_WritePin(ch[i].en_gpio,ch[i].en_pin,GPIO_PIN_RESET);
                 osDelay(3000);
              HAL_GPIO_WritePin(ch[i].en_gpio,ch[i].en_pin,GPIO_PIN_SET); 
              break;
              default:
                  ch[i].fault_cnt = 0;
                  ch[i].fault_state = false ;
                  HAL_GPIO_WritePin(ch[i].en_gpio,ch[i].en_pin,GPIO_PIN_RESET);
              break;
        }
              
  }
}else {__NOP;}
            }
               
    }  
    osDelay(1);
  }
  /* USER CODE END StartTask_Fault05 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void Start_ADCTask(void const * argument)
{
    uint32_t VDDA_CALIB_DATA = *VREFINT_CAL_ADDR;
    
    HAL_ADCEx_Calibration_Start(&hadc,ADC_SINGLE_ENDED);
    HAL_TIM_Base_Start(&htim6);
    HAL_ADC_Start_DMA(&hadc, (uint32_t*) &adc_data, ADC_CHANNELS);
    /* Infinite loop */
    for(;;)
    {   
        hadc.DMA_Handle->XferCpltCallback = comp_adc;
        while( cplt_flag != true )
        {
            
        }
        cplt_flag = false;
        
        vdda_mcu = (uint16_t) (( (uint32_t)3000 * VDDA_CALIB_DATA ) / (adc_data_copy [16]));
        adapt_voltage = (uint16_t)  (( ( (float) adc_data_copy [6]* VDDD_MCU_VALUE)/ ADC_MAX_VALUE))* ADAPT_COEFT;
        
        bat_volts_1c = (uint16_t) ( ( ( (float) adc_data_copy [9] * (float) vdda_mcu) / ADC_MAX_VALUE ))* BAT1C_COEFT;
        bat_volts_2c = (uint16_t) ( ( ( (float) adc_data_copy [14] * (float) vdda_mcu) / ADC_MAX_VALUE ))* BAT2C_COEFT;
        
        buck_5v =     (uint16_t) ( ( ( (float) adc_data_copy [10] *  VDDD_MCU_VALUE) / ADC_MAX_VALUE ))* BC_5V_COEFT;
        buck_3v3 =    (uint16_t) ( ( ( (float) adc_data_copy [11] *  VDDD_MCU_VALUE) / ADC_MAX_VALUE ))* BC_3V_COEFT;
        
        current_ch1 = (uint16_t) ( ( ( ( (float) adc_data_copy [12] * (float) VDDD_MCU_VALUE)/ADC_MAX_VALUE)) /GAIN_CS)/SHUNT_RES10;
        current_ch2 = (uint16_t) ( ( ( ( (float) adc_data_copy [13] * (float) VDDD_MCU_VALUE)/ADC_MAX_VALUE)) /GAIN_CS)/SHUNT_RES20;
        current_ch3 = (uint16_t) ( ( ( ( (float) adc_data_copy [0] *  (float) VDDD_MCU_VALUE)/ADC_MAX_VALUE)) /GAIN_CS)/SHUNT_RES20;        
        current_ch4 = (uint16_t) ( ( ( ( (float) adc_data_copy [1] *  (float) VDDD_MCU_VALUE)/ADC_MAX_VALUE)) /GAIN_CS)/SHUNT_RES20;
        current_ch5 = (uint16_t) ( ( ( ( (float) adc_data_copy [2] *  (float) VDDD_MCU_VALUE)/ADC_MAX_VALUE)) /GAIN_CS)/SHUNT_RES20;
        
        solar_ch_x =  (uint16_t) ( ( ( (float) adc_data_copy [3] * (float) VDDD_MCU_VALUE) / ADC_MAX_VALUE )) * SOLAR_COEFT;
        solar_ch_y =  (uint16_t) ( ( ( (float) adc_data_copy [4] * (float) VDDD_MCU_VALUE) / ADC_MAX_VALUE )) * SOLAR_COEFT;
        solar_ch_z =  (uint16_t) ( ( ( (float) adc_data_copy [5] * (float) VDDD_MCU_VALUE) / ADC_MAX_VALUE )) * SOLAR_COEFT;
        temp_left_bat = (((TEMP_CONST1 - sqrt( pow((-TEMP_CONST1),2)+ 4*TEMP_CONST2 *(TEMP_CONST3-( ( (float) adc_data_copy [15]*VDDD_MCU_VALUE) / ADC_MAX_VALUE ))))/(2*(-TEMP_CONST2)))+30);
        temp_right_bat =(((TEMP_CONST1 - sqrt( pow((-TEMP_CONST1),2)+ 4*TEMP_CONST2 *(TEMP_CONST3-( ( (float) adc_data_copy [8]*VDDD_MCU_VALUE) / ADC_MAX_VALUE ))))/(2*(-TEMP_CONST2)))+30);
        
        temp_mcu = ((((((float)adc_data_copy [17]) * VDD_APPLI/VDD_CALIB )- (int32_t)*TEMP30_CAL_ADDR)*(int32_t)(130-30))/(int32_t)(*TEMP130_CAL_ADDR -*TEMP30_CAL_ADDR))+30 ;
        adap_current_isl = (uint16_t) ( ( ( ( (float) adc_data_copy [7]*VDDD_MCU_VALUE) / ADC_MAX_VALUE)/GAIN_ISL)/SHUNT_RES20);
          
        if (i2c_flag == true){
        adap_current_isl = (uint16_t) ( ( ( ( (float) adc_data_copy [7]*VDDD_MCU_VALUE) / ADC_MAX_VALUE)/GAIN_ISL)/SHUNT_RES20);
        bat_current_isl = 0 ;    
        } else {
        bat_current_isl = (uint16_t) ( ( ( ( (float) adc_data_copy [7]*VDDD_MCU_VALUE) / ADC_MAX_VALUE)/GAIN_ISL)/SHUNT_RES10);
        adap_current_isl = 0 ;    
        }
        osDelay(100);
        }
  /* USER CODE END StartDefaultTask */
}


void comp_adc (struct __DMA_HandleTypeDef * hdma)
{
    cplt_flag = true;
    memcpy( (uint8_t*) &adc_data_copy, (uint8_t*) &adc_data, sizeof(adc_data));
    //end
    hadc.DMA_Handle->XferCpltCallback = NULL;
}

void isl9237_i2c (void){
    
     i2c_read = HAL_I2C_Mem_Read(&hi2c3, ISL_READ, CHARGE_ADR, 1, (uint8_t*) &read_charge_limit, 2, 1000);     // Read  Charge Current Limit value Settings
     i2c_write = HAL_I2C_Mem_Write(&hi2c3, ISL_WRITE, CHARGE_ADR, 1, (uint8_t*) &write_charge_limit, 2, 1000); //  Write Set Charge Current Limit value Settings
     
     i2c_write = HAL_I2C_Mem_Write(&hi2c3,ISL_WRITE, ADAPTER1_ADR,1,(uint8_t*)&write_adap1,2,100); // APAPTER_CURRENT_1 WRITE
     i2c_write = HAL_I2C_Mem_Write(&hi2c3,ISL_WRITE, ADAPTER2_ADR,1,(uint8_t*)&write_adap2,2,100); // APAPTER_CURRENT_2 WRITE
    
     i2c_read = HAL_I2C_Mem_Read(&hi2c3,ISL_READ, ADAPTER1_ADR, 1,(uint8_t*) &read_adap1, 2, 100); // APAPTER_CURRENT_1 READ
     i2c_read = HAL_I2C_Mem_Read(&hi2c3,ISL_READ, ADAPTER2_ADR, 1,(uint8_t*) &read_adap2, 2, 100); // APAPTER_CURRENT_2 READ
      
     i2c_write = HAL_I2C_Mem_Write(&hi2c3,ISL_WRITE, PROCHOT_AC,1,(uint8_t*)&write_ac_prochot,2,100);  // Write AC_PROCHOT Settings
     i2c_read = HAL_I2C_Mem_Read(&hi2c3,ISL_READ, PROCHOT_AC, 1,(uint8_t*) &read_ac_prochot, 2, 100);  // Read  AC_PROCHOT Settings
   
     i2c_write = HAL_I2C_Mem_Write(&hi2c3,ISL_WRITE, PROCHOT_DC,1,(uint8_t*)&write_dc_prochot,2,100);  // Write DC_PROCHOT Settings
     i2c_read = HAL_I2C_Mem_Read(&hi2c3,ISL_READ, PROCHOT_DC, 1,(uint8_t*) &read_dc_prochot, 2, 100);  // Read  DC_PROCHOT Settings  
    
     i2c_write = HAL_I2C_Mem_Write(&hi2c3,ISL_WRITE, CONTROL0_ADR,1,(uint8_t*)&write_control0,2,100);  //  Write REG Control_0 Settings 
     i2c_read = HAL_I2C_Mem_Read(&hi2c3,ISL_READ, CONTROL0_ADR, 1,(uint8_t*) &read_control0, 2, 100);  //  Read  REG Control_0 Settings      
    
     i2c_read = HAL_I2C_Mem_Read(&hi2c3,ISL_READ, CONTROL1_ADR, 1,(uint8_t*) &read_control1, 2, 100);  //  Read  REG Control_1 Settings
     i2c_write = HAL_I2C_Mem_Write(&hi2c3,ISL_WRITE, CONTROL1_ADR,1,(uint8_t*)&write_control1,2,100);  //  Write REG Control_1 Settings
    
     i2c_read = HAL_I2C_Mem_Read(&hi2c3,ISL_READ, CONTROL2_ADR, 1,(uint8_t*)&read_control2, 2, 100);   //  Read  REG Control_2 Settings
     i2c_write = HAL_I2C_Mem_Write(&hi2c3,ISL_WRITE, CONTROL2_ADR,1,(uint8_t*)&write_control2,2,100);  //  Write REG Control_2 Settings
    
     i2c_read = HAL_I2C_Mem_Read(&hi2c3,ISL_READ, INF_REGISTOR, 1,(uint8_t*)&read_inf_registor, 2, 100); // Read information various charger status    
     
}
void start_pross (void)
{
    
    HAL_GPIO_WritePin(EN_5V_GPIO_Port,EN_5V_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(EN_3V3_GPIO_Port,EN_3V3_Pin,GPIO_PIN_SET);
    
    osDelay(10);
    
    if (buck_5v > 4750 && buck_3v3 > 3000) {
    
        if(pg_5v_flag == false && pg_3v3_flag == false){
            
            // SYS_OK
        
        } else if (pg_5v_flag == true){
        
            // SYS_ERROR_5V Buck converter line
        
        } else if (pg_3v3_flag == true){
        
           // SYS_ERROR_3V3 Buck converter line
        }
    
    } else {
    
    //CHARGE_Mode and will Wait give SYS_OK_FLAG
    }
    
    
    

}


/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
