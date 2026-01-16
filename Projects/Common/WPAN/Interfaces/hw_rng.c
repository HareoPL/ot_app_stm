/**
  ******************************************************************************
  * @file    hw_rng.c
  * @author  MCD Application Team
  * @brief   This file contains the RNG driver for STM32WBA
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "app_common.h"
#include "stm32wbaxx_ll_bus.h"
#include "stm32wbaxx_ll_rng.h"
#include "RTDebug.h"

#include "ot_app.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "timers.h"

#define TAG "hw_rng "

/*****************************************************************************/

extern void Error_Handler(void);

/*****************************************************************************/

__WEAK int RNG_MutexTake(void)
{
  return 0; /* This shall be implemented by user */
}

__WEAK int RNG_MutexRelease(void)
{
  return 0; /* This shall be implemented by user */
}

/*****************************************************************************/

__weak void RNG_KERNEL_CLK_ON(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the RNG_KERNEL_CLK_ON could be implemented in the user file
  */   
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() == 0)
  {
    LL_RCC_HSI_Enable();
  }
}

__weak void RNG_KERNEL_CLK_OFF(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the RNG_KERNEL_CLK_OFF could be implemented in the user file
  */   
}

/*****************************************************************************/

typedef struct
{
  uint32_t pool[HW_RNG_POOL_SIZE];
  uint8_t  size;
  uint8_t  run;
  uint8_t  clock_en;
  int      error;
} HW_RNG_VAR_T;

/*****************************************************************************/

static HW_RNG_VAR_T HW_RNG_var;
  
static uint8_t hw_rng_pool_threshold = HW_RNG_POOL_DEFAULT_THRESHOLD;

/*****************************************************************************/

static void HW_RNG_WaitingClockSynchronization( void );
static void HW_RNG_TimerStart(void);

/*****************************************************************************/

void HW_RNG_Disable( void )
{
  SYSTEM_DEBUG_SIGNAL_SET(RNG_DISABLE);

  LL_RNG_Disable( RNG );
  while(LL_RNG_IsEnabled(RNG)); // whait for RNGEN = 0

  uint32_t timeout = 1000;
  while((RNG->SR & (1 << 4)) && timeout--); // whait for BUSY = 0

  /* Disable RNG clocks */
  HW_RNG_DisableClock( 1 );

  SYSTEM_DEBUG_SIGNAL_RESET(RNG_DISABLE);
}

/*****************************************************************************/

void HW_RNG_EnableClock( uint8_t user_mask )
{
  HW_RNG_VAR_T* pv = &HW_RNG_var;

  RNG_KERNEL_CLK_ON();

  UTILS_ENTER_CRITICAL_SECTION( );

  if ( pv->clock_en == 0 )
  {
    LL_AHB2_GRP1_EnableClock( LL_AHB2_GRP1_PERIPH_RNG );
  }

  pv->clock_en |= user_mask;

  UTILS_EXIT_CRITICAL_SECTION( );
}

/*****************************************************************************/

void HW_RNG_DisableClock( uint8_t user_mask )
{
  HW_RNG_VAR_T* pv = &HW_RNG_var;

  {
    UTILS_ENTER_CRITICAL_SECTION( );

    pv->clock_en &= ~user_mask;

    UTILS_EXIT_CRITICAL_SECTION( );
  }

  /* It does not matter much if the temporisation is executed even though
   * in the meantime pv->clock_en has been updated and is not more equal to 0
   */
  if ( pv->clock_en == 0 )
  {
    HW_RNG_WaitingClockSynchronization( );
  }

  {
    UTILS_ENTER_CRITICAL_SECTION( );

    if ( pv->clock_en == 0 )
    {
      LL_AHB2_GRP1_DisableClock( LL_AHB2_GRP1_PERIPH_RNG );
    }

    UTILS_EXIT_CRITICAL_SECTION( );
  }

  RNG_KERNEL_CLK_OFF();
}

/*****************************************************************************/
/*
 * Wait for 2 RNG kernel clock.
 * Loop is sized with worst case : RNG kernel clock = 32Khz
 */
static void HW_RNG_WaitingClockSynchronization( void )
{
  /* RNG busy flag is not available in STM32WBA5xxx */
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || \
    defined(STM32WBA5Mxx) || defined(STM32WBA62xx) || defined(STM32WBA63xx) || \
    defined(STM32WBA64xx) || defined(STM32WBA65xx)
  volatile unsigned int cpt;
  
  for(cpt = 178 ; cpt!=0 ; --cpt);
#else
  /* wait until RNG_SR_BUSY (bit 4) is cleared */
  while(RNG->SR & (1 << 4));
#endif /* defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA5Mxx) */  
}

/*****************************************************************************/

/*
 * HW_RNG_Run: this function must be called in loop.
 * It implenments a simple state machine that enables the RNG,
 * fills the pool with generated random numbers and then disables the RNG.
 * It always returns 0 in normal conditions. In error conditions, it returns
 * an error code different from 0.
 */
static int HW_RNG_Run(HW_RNG_VAR_T* pv)
{
  int i, error = HW_OK;

  /* check for RNG clock error */
  if (LL_RNG_IsActiveFlag_CECS(RNG))
  {
    /* Clear RNG clock error interrupt status flags */
    LL_RNG_ClearFlag_CEIS(RNG);

    error = HW_RNG_CLOCK_ERROR;
  }

  /* check for RNG seed error */
  if (LL_RNG_IsActiveFlag_SEIS(RNG))
  {
    /* Clear RNG seed error interrupt status flags */
    LL_RNG_ClearFlag_SEIS(RNG);

    /* Discard 12 words from RNG_DR in order to clean the pipeline */
    for ( i = 12; i > 0; i-- )
    {
      LL_RNG_ReadRandData32(RNG);
    }

    error = HW_RNG_NOISE_ERROR;
  }

  /* if the pool is not full, read the H/W generated values until the pool is full */
  UTILS_ENTER_CRITICAL_SECTION();
  SYSTEM_DEBUG_SIGNAL_SET(RNG_GEN_RAND_NUM);

  while (pv->size < HW_RNG_POOL_SIZE)
  {
    if (LL_RNG_IsActiveFlag_DRDY(RNG))
    {
      pv->pool[pv->size] = LL_RNG_ReadRandData32(RNG);
      pv->size++;
    }
  }

  SYSTEM_DEBUG_SIGNAL_RESET(RNG_GEN_RAND_NUM);
  UTILS_EXIT_CRITICAL_SECTION( );

  /* pool is full, disable the RNG and its RCC clock */
  // BUG, I think, RNG shouldn't be disable immediately after generated, because it may be needed for next using IMMEDIATELY
  // TODO implementation of delayed HW_RNG_Disable. start freertos timer -> freertos timer in callback -> run suspended RNG_task  -> RNG_task - execute, disable RNG.
  
  // HW_RNG_Disable( ); // Here not disable but refresh freertos timmer 
  HW_RNG_TimerStart(); // start counting to disable HW_RNG, if some process needs necessary use RNG, timer will be restart
  
  /* Reset flag indicating that the RNG is ON */
  pv->run = FALSE;
  
  return error;
}

/*****************************************************************************/

void HW_RNG_Start( void )
{
  HW_RNG_VAR_T* pv = &HW_RNG_var;

  /* Reset global variables */
  pv->size = 0;
  pv->run = FALSE;
  pv->error = HW_OK;
  pv->clock_en = 0;

  if (0 != RNG_MutexTake())
  {
	  Error_Handler();
  }

  /* Fill the random numbers pool by calling the "run" function */
  do
  {
    pv->error = HW_RNG_Run( pv );
  }
  while ( pv->run && !pv->error );

  if (0 != RNG_MutexRelease())
  {
    Error_Handler();
  }
}

/*****************************************************************************/

void HW_RNG_Get( uint8_t n, uint32_t* val )
{
  HW_RNG_VAR_T* pv = &HW_RNG_var;
  uint32_t pool_value;

  while ( n-- )
  {
    UTILS_ENTER_CRITICAL_SECTION( );

    if ( pv->size == 0 )
    {
      pv->error = HW_RNG_UFLOW_ERROR;
      pool_value = ~pv->pool[HW_RNG_POOL_SIZE - n];
    }
    else
    {
      pool_value = pv->pool[--pv->size];
    }

    UTILS_EXIT_CRITICAL_SECTION( );

    *val++ = pool_value;
  }

  /* Call the process callback function to fill the pool offline */
  HWCB_RNG_Process( );
}

/*****************************************************************************/

int HW_RNG_Process( void )
{
  HW_RNG_VAR_T* pv = &HW_RNG_var;
  int status = HW_OK;

  if (0 != RNG_MutexTake())
  {
    status = HW_BUSY;
  }  
  else 
  {
    /* Check if the process is not done or if the pool is not full */
    if (pv->size < hw_rng_pool_threshold)
    {
      HW_RNG_Init();
      UTILS_ENTER_CRITICAL_SECTION( );

      /* Check if an error occurred during a previous call to HW_RNG API */
      status = pv->error;
      pv->error = HW_OK;

      UTILS_EXIT_CRITICAL_SECTION( );

      if ( status == HW_OK )
      {
        /* Call the "run" function that generates random data */
        status = HW_RNG_Run( pv );

        /* If the process is not done, return "busy" status */
        if ( (status == HW_OK) && pv->run )
        {
          status = HW_BUSY;
        }
      }
    }
    
    if (0 != RNG_MutexRelease())
    {
      Error_Handler();
    }
  }

  if (status != HW_OK)
  {
    HWCB_RNG_Process( );
  }

  /* Return status */
  return status;
}

void HW_RNG_Init(void)
{
 HW_RNG_VAR_T* pv = &HW_RNG_var;

 // if RNG already is running, do nothing - you will avoid CONDRST reset and BUSY error
 if (LL_RNG_IsEnabled(RNG) && (RNG->CR & RNG_CR_RNGEN))
 {
     return;
 }

 SYSTEM_DEBUG_SIGNAL_SET(RNG_ENABLE);

 LL_RCC_SetRNGClockSource(LL_RCC_RNG_CLKSOURCE_HSI);
 LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_RNG);
 LL_RNG_Disable(RNG);
 while(LL_RNG_IsEnabled(RNG));

 /* Recommended value for NIST compliance, refer to application note AN4230 */
 /* Using LL macros to set these values is not convenient as it's split
    in 3 parts and need some bit polling at each step to check completion.
    So, for efficiency, register direct access */
 WRITE_REG(RNG->CR, RNG_CR_NIST_VALUE | RNG_CR_CONDRST | RNG_CED_DISABLE);
 /* Recommended value for NIST compliance, refer to application note AN4230 */

 LL_RNG_DisableClkErrorDetect(RNG);
 LL_RNG_DisableCondReset(RNG);


#if !(defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || \
         defined(STM32WBA5Mxx) || defined(STM32WBA62xx) || defined(STM32WBA63xx) || \
         defined(STM32WBA64xx) || defined(STM32WBA65xx))
while((RNG->SR & (1 << 4)));
#endif

 while(LL_RNG_IsEnabledCondReset(RNG));

 LL_RNG_SetHealthConfig(RNG,RNG_HTCR_NIST_VALUE);

 LL_RNG_Enable(RNG);
 while(!LL_RNG_IsActiveFlag_DRDY(RNG)); /*wait for data to be ready*/

 pv->run = TRUE;
 SYSTEM_DEBUG_SIGNAL_RESET(RNG_ENABLE);
}
 
  
void HW_RNG_SetPoolThreshold(uint8_t threshold)
{
  if(threshold < HW_RNG_POOL_SIZE)
  {
    hw_rng_pool_threshold = threshold;
  }
}

#define HW_RNG_TIMER_DELAY   (10000 - 100)     // debounce for saving in RAM (ms)
#define HW_RNG_TASK_STACK  (256 * 4)

static TimerHandle_t HW_RNG_timer; // Handle to control the nvm timer
static osThreadId_t HW_RNG_taskHandle;


void HW_RNG_task(void *argument)
{

  while(1)
  {
    // disable RNG
    HW_RNG_Disable();   
    osThreadSuspend(HW_RNG_taskHandle);
  }
}

static void HW_RNG_TimerStart(void)
{
  xTimerReset(HW_RNG_timer, 0);
}

static void HW_RNG_timerCallback(TimerHandle_t xTimer) 
{
  osThreadResume(HW_RNG_taskHandle);  // wake up  task 
  OTAPP_PRINTF(TAG, "RNG_Timer: resume RNG_task \n");
}

static void HW_RNG_createTimer(void)
{
  HW_RNG_timer = xTimerCreate(
      "RNG_Timer",            
      pdMS_TO_TICKS(HW_RNG_TIMER_DELAY),    
      pdFALSE,                // pdFALSE = One-shot , pdTRUE = Auto-reload 
      (void *) 0,             // ID timer 
      HW_RNG_timerCallback        
  );

  if(HW_RNG_timer == NULL) 
  {
    OTAPP_PRINTF(TAG, "RNG_Timer: no more heap \n"); 
  }
}

static void HW_RNG_createTask(void)
{
  osThreadAttr_t task_attr = {
    .name = "RNG_task",
    .priority = osPriorityLow,
    .stack_size = HW_RNG_TASK_STACK,
  };
  HW_RNG_taskHandle = osThreadNew(HW_RNG_task, NULL, &task_attr);
  osThreadSuspend(HW_RNG_taskHandle);  // START SUSPENDED!
}

void HW_RNG_initTask(void)
{
  HW_RNG_createTask();
  HW_RNG_createTimer();
}