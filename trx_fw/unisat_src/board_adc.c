//           _      ______       _____      _______ 
//     /\   | |    |  ____/\    / ____|  /\|__   __|
//    /  \  | |    | |__ /  \  | (___   /  \  | |   
//   / /\ \ | |    |  __/ /\ \  \___ \ / /\ \ | |   
//  / ____ \| |____| | / ____ \ ____) / ____ \| |   
// /_/    \_\______|_|/_/    \_\_____/_/    \_\_|   
//                                                  
//
// Author: Nursultan Uzbekov
// Date:   19.01.2020
//

#include "board_adc.h"
#include "main.h"
#include "adc.h"

#define VREFINT_CAL_ADDR        ((uint16_t*) ((uint32_t) 0x1FF80078))
#define TEMP130_CAL_ADDR        ((uint16_t*) ((uint32_t) 0x1FF8007E))
#define TEMP30_CAL_ADDR         ((uint16_t*) ((uint32_t) 0x1FF8007A))
#define VDD_CALIB               ((uint16_t) (300))
#define VDD_APPLI               ((uint16_t) (330))


static uint16_t vref;
static int16_t temperature;



AF_Status ReadVref(uint16_t* pvref, uint16_t timeout)
{
    ADC1->CFGR1 |= ADC_CFGR1_AUTOFF;
    ADC1->CHSELR = 0x00;
    ADC1->CHSELR = ADC_CHANNEL_VREFINT;
    ADC1->SMPR |= ADC_SMPR_SMP;
    ADC->CCR |= ADC_CCR_VREFEN;
    
    ADC1->CR |= ADC_CR_ADSTART;
    
    uint32_t time = HAL_GetTick();
    
    while( (ADC1->ISR & ADC_ISR_EOC) == 0)
    {
        if( HAL_GetTick() - time > timeout)
        {
            return AF_ERROR;
        }
    }
    
    uint32_t temp = ADC1->DR & 0xFFF;
    
    *pvref = vref = (uint16_t) ( ( (float) 3000.00 * (float)(*VREFINT_CAL_ADDR)) / (float) temp );
    
    ADC->CCR &= ~(ADC_CCR_VREFEN);
    
    return AF_OK;
}

uint16_t GetVref(void)
{
    return vref;
}

AF_Status ReadTemperature(int16_t* ptemp, uint16_t timeout)
{
    ADC1->CFGR1 |= ADC_CFGR1_AUTOFF;
    ADC1->CHSELR = 0x00;
    ADC1->CHSELR = ADC_CHANNEL_TEMPSENSOR;
    ADC1->SMPR |= ADC_SMPR_SMP;
    ADC->CCR |= ADC_CCR_TSEN;
    
    ADC1->CR |= ADC_CR_ADSTART;
    
    uint32_t time = HAL_GetTick();
    
    while( (ADC1->ISR & ADC_ISR_EOC) == 0)
    {
        if( HAL_GetTick() - time > timeout)
        {
            return AF_ERROR;
        }
    }
    
    float t = (float) (ADC1->DR & 0xFFF);
    t = (float) (( ( 130 - 30 ) / (float)( *TEMP130_CAL_ADDR - *TEMP30_CAL_ADDR) ) * ( t - *TEMP30_CAL_ADDR ) ) + 30.00;
    
    *ptemp = temperature = (int16_t) ( t * 100);
    
    ADC->CCR &= ~(ADC_CCR_TSEN);
    
    return AF_OK;
}

uint16_t GetTemperature(void)
{
    return temperature;
}
