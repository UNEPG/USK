#include "hw.h"
#include "rs485.h"

#define RS485_DMA_DISABLE_TIMEOUT       100

/* Handles */
static UART_HandleTypeDef   RS485_UartHandle;
static TIM_HandleTypeDef    RS485_Timer;
    
static void RS485_MspInit(UART_HandleTypeDef *huart);
static void DmaTxCpltCallback(DMA_HandleTypeDef *dma);
static void RS485_SetReceiveTimeout(uint32_t timeout);
static void RS485_TimerInit(void);
static void RS485_TimerStart(uint16_t timeout);
static void RS485_TimerStop(void);

static void (*RxCpltCallback)( void );
static void (*TxCpltCallback)( void );
static void (*RxTxErrorCallback)( void );
static void (*TimeoutCallback)( void );

static volatile uint16_t RS485_DMAReceiveLen = 0;

void RS485_Init(    void (*pRxCpltCallback)( void ), 
                    void (*pTxCpltCallback)( void ),
                    void (*pRxTxErrorCallback)( void ),
                    void (*pTimeoutCallback)( void ) )
{
    
    RS485_UartHandle.Instance        = RS485_USARTx;
    
    RS485_UartHandle.Init.BaudRate   = 115200;
    RS485_UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    RS485_UartHandle.Init.StopBits   = UART_STOPBITS_1;
    RS485_UartHandle.Init.Parity     = UART_PARITY_NONE;
    RS485_UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    RS485_UartHandle.Init.Mode       = UART_MODE_TX_RX;
    RS485_UartHandle.MspInitCallback = &RS485_MspInit;
    
    if (HAL_UART_Init(&RS485_UartHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }
    
    RxCpltCallback      = pRxCpltCallback;
    TxCpltCallback      = pTxCpltCallback;
    RxTxErrorCallback   = pRxTxErrorCallback;
    TimeoutCallback     = pTimeoutCallback;
    
    HAL_DMA_RegisterCallback(RS485_UartHandle.hdmatx, HAL_DMA_XFER_CPLT_CB_ID, DmaTxCpltCallback);
    
    RS485_TimerInit();
}

AF_Status RS485_Send(uint8_t* buf, uint8_t len, uint32_t timeout)
{
    if ( ( len == 0 ) || 
         ( buf == NULL ) )
    {
        return AF_ERROR;
    }
    
    RS485_TimerStop( );
    
    RS485_USARTx->CR1 &= ~((uint32_t)(  USART_CR1_RXNEIE |
                                        USART_CR1_TCIE |
                                        USART_CR1_TXEIE |
                                        USART_CR1_RTOIE |
                                        USART_CR1_RE ));
    
    RS485_TRANSCEIVER_SET_AS_TRANSMITTER();
    
    /* disable DMA requests */
    RS485_USARTx->CR3 &= ~((uint32_t)(USART_CR3_DMAT | USART_CR3_DMAR));
    
    /* disable DMA and half transfer int */
    RS485_USARTx_TX_DMA_CHANNEL->CCR &= ~(uint32_t)( DMA_CCR_EN | DMA_CCR_HTIE );
    
    /* на всякий случай */
    uint32_t st = HAL_GetTick();

    while ((RS485_USARTx_TX_DMA_CHANNEL->CCR & DMA_CCR_EN) != 0)
    {
        if ((HAL_GetTick() - st) >= RS485_DMA_DISABLE_TIMEOUT)
        {
            return AF_WAIT;
        }
    }
    
    RS485_USARTx_TX_DMA_CHANNEL->CPAR = (uint32_t)&RS485_USARTx->TDR;
    RS485_USARTx_TX_DMA_CHANNEL->CMAR = (uint32_t)buf;
    
    RS485_USARTx_TX_DMA_CHANNEL->CNDTR = len;
    RS485_USARTx_TX_DMA_CHANNEL->CCR |= (DMA_CCR_TCIE | DMA_CCR_TEIE);
    RS485_USARTx_TX_DMA_CHANNEL->CCR |= DMA_CCR_EN;
    
    RS485_USARTx->ICR = USART_ICR_TCCF;
    RS485_USARTx->CR3 |= USART_CR3_DMAT | USART_CR3_EIE;
    
    if( timeout != 0 )
    {
        RS485_TimerStart( timeout );
    }
    return AF_OK;
}

uint8_t RS485_GetRecvdLen(void)
{
    uint32_t tmp = RS485_USARTx_RX_DMA_CHANNEL->CNDTR;
    
    uint8_t res = (RS485_DMAReceiveLen > tmp) ? (RS485_DMAReceiveLen - tmp) : 0;
    
    return res;
}

AF_Status RS485_Receive(uint8_t* buf, uint16_t len, uint32_t timeout, uint32_t bit_timeout)
{
    
    if( (len == 0) || 
        (buf == NULL) )
    {
        return AF_ERROR;
    }
    
    RS485_TRANSCEIVER_SET_AS_RECEIVER();
    
    RS485_USARTx->ICR = USART_ICR_RTOCF;
    RS485_USARTx->RQR = USART_RQR_RXFRQ; // Receive data flush request
    RS485_USARTx->CR1 &= ~(uint32_t)(   USART_CR1_RXNEIE |
                                        USART_CR1_TCIE |
                                        USART_CR1_TXEIE |
                                        USART_CR1_RTOIE );
    
    RS485_USARTx->CR2 &= ~(uint32_t)( USART_CR2_RTOEN );
    
    /* disable DMA requests */
    RS485_USARTx->CR3 &= ~((uint32_t)USART_CR3_DMAR);
    
    /* disable DMA and half transfer int */
    RS485_USARTx_RX_DMA_CHANNEL->CCR &= ~((uint32_t)DMA_CCR_EN | DMA_CCR_HTIE);
    
    /* на всякий случай */
    uint32_t st = HAL_GetTick();
    
    while ((RS485_USARTx_RX_DMA_CHANNEL->CCR & DMA_CCR_EN) != 0)
    {
        if ((HAL_GetTick() - st) >= RS485_DMA_DISABLE_TIMEOUT)
        {
            return AF_TIMEOUT;
        }
    }

    RS485_DMAReceiveLen = len;
    
    RS485_USARTx_RX_DMA_CHANNEL->CPAR = (uint32_t)&RS485_USARTx->RDR;
    RS485_USARTx_RX_DMA_CHANNEL->CMAR = (uint32_t)buf;
    
    RS485_USARTx_RX_DMA_CHANNEL->CNDTR = len;
    RS485_USARTx_RX_DMA_CHANNEL->CCR |= (DMA_CCR_TCIE | DMA_CCR_TEIE);
    RS485_USARTx_RX_DMA_CHANNEL->CCR |= DMA_CCR_EN;
    
    // Таймаут фрейма, для детектирования конца передачи
    if ( bit_timeout != 0 )
    {
        RS485_SetReceiveTimeout( bit_timeout );
        RS485_USARTx->CR1 |= (USART_CR1_RTOIE | USART_CR1_PEIE | USART_CR1_RE);
        RS485_USARTx->CR2 |= USART_CR2_RTOEN;
    }
    else
    {
        RS485_USARTx->CR1 |= USART_CR1_PEIE | USART_CR1_RE;
    }
    
    // DMA request + Error Int enable
    RS485_USARTx->CR3 |= USART_CR3_DMAR | USART_CR3_EIE;
    
    if( timeout != 0 )
    {
        RS485_TimerStart( timeout );
    }
    return AF_OK;
}

static void DmaTxCpltCallback(DMA_HandleTypeDef *dma)
{
    RS485_USARTx->CR1 |= USART_CR1_TCIE;
}

void RS485_USARTx_DMA_RXTX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(RS485_UartHandle.hdmarx);
    HAL_DMA_IRQHandler(RS485_UartHandle.hdmatx);
}

void RS485_USARTx_IRQHandler(void)
{
    // parity error
    if ( ((RS485_USARTx->ISR & USART_ISR_PE) != 0) &&
         ((RS485_USARTx->CR1 & USART_CR1_PEIE) != 0) )
    {
        RS485_USARTx->ICR = USART_ICR_PECF;
        
        RS485_TimerStop();
        
        if( RxTxErrorCallback != NULL )
        {
            RxTxErrorCallback();
        }
    }
    
    // overrun, noise error
    if ( ((RS485_USARTx->ISR & (USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE)) != 0 ) &&
         ((RS485_USARTx->CR3 & USART_CR3_EIE) != 0 ) )
    {
        RS485_USARTx->ICR = USART_ICR_FECF | USART_ICR_NCF | USART_ICR_ORECF;
        
        RS485_TimerStop();
        
        if( RxTxErrorCallback != NULL )
        {
            RxTxErrorCallback();
        }
    }
    
    // transmit complete
    if ( ((RS485_USARTx->ISR & USART_ISR_TC) != 0) &&
         ((RS485_USARTx->CR1 & USART_CR1_TCIE) != 0) )
    {
        
        RS485_TRANSCEIVER_SET_AS_RECEIVER();
        RS485_USARTx->CR1 &= ~((uint32_t)USART_CR1_TCIE);
        RS485_USARTx->CR3 &= ~((uint32_t)USART_CR3_DMAT);
        RS485_USARTx->ICR = USART_ICR_TCCF;
        
        RS485_TimerStop();

        if( TxCpltCallback != NULL )
        {
            TxCpltCallback();
        }
    }

    // receive byte timeout
    if ( ((RS485_USARTx->ISR & USART_ISR_RTOF) != 0) &&
         ((RS485_USARTx->CR1 & USART_CR1_RTOIE) != 0) )
    {
        RS485_USARTx->ICR = USART_ICR_RTOCF;
        RS485_USARTx->CR1 &= ~((uint32_t)USART_CR1_RTOIE);
        RS485_USARTx->CR2 &= ~((uint32_t)USART_CR2_RTOEN);
        
        RS485_USARTx->CR3 &= ~((uint32_t)USART_CR3_DMAR);
        RS485_USARTx_RX_DMA_CHANNEL->CCR &= ~((uint32_t)DMA_CCR_EN);
        
        RS485_TimerStop();
        
        if( RxCpltCallback != NULL )
        {
            RxCpltCallback();
        }
    }
}

void RS485_DeInit(void)
{
    HAL_UART_DeInit(&RS485_UartHandle);
}

void RS485_MspInit(UART_HandleTypeDef *huart)
{
  if (huart->Instance == RS485_USARTx)
  {
    static DMA_HandleTypeDef hdma_rx;
    static DMA_HandleTypeDef hdma_tx;


    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    RS485_USARTx_TX_GPIO_CLK_ENABLE();
    RS485_USARTx_RX_GPIO_CLK_ENABLE();

    /* Enable USARTx clock */
    RS485_USARTx_CLK_ENABLE();

    /* Enable DMA clock */
    RS485_DMAx_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* UART  pin configuration  */
    RS485_IoInit();

    /*##-3- Configure the DMA ##################################################*/
    /* Configure the DMA handler for Transmission process */
    hdma_rx.Instance                 = RS485_USARTx_RX_DMA_CHANNEL;
    hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx.Init.Mode                = DMA_NORMAL;
    hdma_rx.Init.Priority            = DMA_PRIORITY_LOW;
    hdma_rx.Init.Request             = RS485_USARTx_RX_DMA_REQUEST;
    HAL_DMA_Init(&hdma_rx);
    
    hdma_tx.Instance                 = RS485_USARTx_TX_DMA_CHANNEL;
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx.Init.Mode                = DMA_NORMAL;
    hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
    hdma_tx.Init.Request             = RS485_USARTx_TX_DMA_REQUEST;
    HAL_DMA_Init(&hdma_tx);

    /* Associate the initialized DMA handle to the UART handle */
    __HAL_LINKDMA(huart, hdmarx, hdma_rx);
    __HAL_LINKDMA(huart, hdmatx, hdma_tx);

    /*##-4- Configure the NVIC for DMA #########################################*/
    /* NVIC configuration for DMA transfer complete interrupt (USART1_TX) */
    HAL_NVIC_SetPriority(RS485_USARTx_DMA_RXTX_IRQn, RS485_USARTx_DMA_Priority, 1);
    HAL_NVIC_EnableIRQ  (RS485_USARTx_DMA_RXTX_IRQn);

    /* NVIC for USART, to catch the TX complete */
    HAL_NVIC_SetPriority(RS485_USARTx_IRQn, RS485_USARTx_Priority, 1);
    HAL_NVIC_EnableIRQ  (RS485_USARTx_IRQn);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  RS485_IoDeInit();
  /*##-1- Reset peripherals ##################################################*/
  RS485_USARTx_FORCE_RESET();
  RS485_USARTx_RELEASE_RESET();

  /*##-3- Disable the DMA #####################################################*/
  /* De-Initialize the DMA channel associated to reception process */
  if (huart->hdmarx != 0)
  {
    HAL_DMA_DeInit(huart->hdmarx);
  }
  /* De-Initialize the DMA channel associated to transmission process */
  if (huart->hdmatx != 0)
  {
    HAL_DMA_DeInit(huart->hdmatx);
  }

  /*##-4- Disable the NVIC for DMA ###########################################*/
  HAL_NVIC_DisableIRQ(RS485_USARTx_DMA_RXTX_IRQn);
}

void RS485_IoInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct = {0};
  /* Enable GPIO TX/RX clock */
  RS485_USARTx_TX_GPIO_CLK_ENABLE();
  RS485_USARTx_RX_GPIO_CLK_ENABLE();
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = RS485_USARTx_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = RS485_USARTx_TX_AF;

  HAL_GPIO_Init(RS485_USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = RS485_USARTx_RX_PIN;
  GPIO_InitStruct.Alternate = RS485_USARTx_RX_AF;

  HAL_GPIO_Init(RS485_USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
  
  if( RS485_DE_GPIO != 0 )
  {
      GPIO_InitStruct.Pin = RS485_DE_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
      
      HAL_GPIO_Init(RS485_DE_GPIO, &GPIO_InitStruct);
  }        
}

void RS485_IoDeInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure = {0};

  RS485_USARTx_TX_GPIO_CLK_ENABLE();

  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;

  GPIO_InitStructure.Pin =  RS485_USARTx_TX_PIN ;
  HAL_GPIO_Init(RS485_USARTx_TX_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.Pin =  RS485_USARTx_RX_PIN ;
  HAL_GPIO_Init(RS485_USARTx_RX_GPIO_PORT, &GPIO_InitStructure);
}

static void RS485_SetReceiveTimeout(uint32_t timeout)
{
    uint32_t tmp = RS485_USARTx->RTOR;
    tmp &= ~((uint32_t)USART_RTOR_RTO);
    RS485_USARTx->RTOR = tmp | (timeout & 0xFFFFFFUL);
}


static void RS485_TimerInit( void )
{
    RS485_TIMER_CLK_ENA();
    
    SystemCoreClockUpdate();
    uint32_t clk = HAL_RCC_GetPCLK1Freq();
    
    TIM_MasterConfigTypeDef RS485_TimerConfig = {0};
    
    RS485_Timer.Instance                = RS485_TIMx;
    RS485_Timer.Init.Prescaler          = (uint32_t) ((clk / 1000) - 1);
    RS485_Timer.Init.CounterMode        = TIM_COUNTERMODE_UP;
    RS485_Timer.Init.Period             = 1000;
    RS485_Timer.Init.AutoReloadPreload  = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&RS485_Timer) != HAL_OK)
    {
        Error_Handler();
    }
    
    if (HAL_TIM_OnePulse_Init(&RS485_Timer, TIM_OPMODE_SINGLE) != HAL_OK)
    {
        Error_Handler();
    }
    
    RS485_TimerConfig.MasterOutputTrigger   = TIM_TRGO_RESET;
    RS485_TimerConfig.MasterSlaveMode       = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&RS485_Timer, &RS485_TimerConfig) != HAL_OK)
    {
        Error_Handler();
    }
    
    HAL_NVIC_SetPriority(RS485_TIMx_IRQn, RS485_TIMx_IRQ_Priority, 0);
    HAL_NVIC_EnableIRQ  (RS485_TIMx_IRQn);
}

static void RS485_TimerStart(uint16_t timeout)
{
    __HAL_TIM_DISABLE(&RS485_Timer);
    __HAL_TIM_CLEAR_FLAG(&RS485_Timer, TIM_FLAG_UPDATE);
    __HAL_TIM_SET_COUNTER(&RS485_Timer, 0);
    __HAL_TIM_SET_AUTORELOAD(&RS485_Timer, (timeout - 1));
    __HAL_TIM_ENABLE_IT(&RS485_Timer, TIM_IT_UPDATE);
    __HAL_TIM_ENABLE(&RS485_Timer);
}

static void RS485_TimerStop( void )
{
    __HAL_TIM_DISABLE(&RS485_Timer);
    __HAL_TIM_DISABLE_IT(&RS485_Timer, TIM_IT_UPDATE);
    __HAL_TIM_SET_COUNTER(&RS485_Timer, 0);
}

void RS485_TIMx_IRQHandler(void)
{
    if ( ((RS485_TIMx->SR & TIM_SR_UIF) != 0) &&
         ((RS485_TIMx->DIER & TIM_DIER_UIE) != 0) )
    {
        RS485_TIMx->SR &= ~((uint32_t)TIM_SR_UIF);
        RS485_TRANSCEIVER_SET_AS_RECEIVER();

        if ( TimeoutCallback != NULL)
        {
            TimeoutCallback();
        }
    }
}
