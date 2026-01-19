/**
 * @file flash.c
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief OpenThread Non-Volatile Storage (NVM) implementation for STM32WBA6. Based on ST example.
 * @version 0.1
 * @date 31-12-2025
 * 
 * @copyright The MIT License (MIT) Copyright (c) 2025 
 * 
 * @section NVM_DESIGN NVM Design Overview
 * 
 * Implements OpenThread `otPlatSettings` API using dedicated flash page in STM32WBA6.
 * Thread settings (SRP ECDSA keys, network keys, datasets) stored in **Bank2 Page127**.
 * 
 * @subsection FLASH_LAYOUT Flash Memory Layout (RM0515 Table 40)
 * 
 * | Flash area    | Bank    | Non-secure Address          | Size    | Name         |
 * |---------------|---------|-----------------------------|---------|--------------|
 * | Main memory   | Bank1   | 0x08000000 - 0x080FFFFF     | 1MB     | Pages 0-127  |
 * | Main memory   | Bank2   | 0x08100000 - 0x081FFFFF     | 1MB     | Pages 1-127  |
 * | **NVM Area**  |**Bank2**| **0x081FE000 - 0x081FFFFF** | **8kB** | **Page 127** |
 * 
 * @subsection LINKER_SECTION Linker Script Configuration
 * 
 * Dedicated NVM section in linker script (`STM32WBA65RIVX_FLASH.ld`):
 * 
 * ~~~ld
 * MEMORY
 * {
 *   FLASH  (rx) : ORIGIN = 0x08000000, LENGTH = 2040K
 *   NVM_KEYS(r) : ORIGIN = 0x081FE000, LENGTH = 8K
 * }
 * 
 * .nvm_section (NOLOAD):
 * {
 *   . = ALIGN(8);
 *   KEEP *(.nvm_keys)
 *   KEEP *(.nvm_keys*)
 *   . = ALIGN(8);
 * } >NVM_KEYS
 * ~~~
 * 
 * variable `nvsFlash[]` lands at **0x081FE000** (map file):
 * ~~~
 * .nvm_keys      0x081fe000       0x400 ./main.o
 *                0x081fe000                nvsFlash
 * ~~~
 * 
 * @subsection FLASH_OPS Flash Operations
 * 
 * - **Erase**: `HAL_FLASHEx_Erase(Bank2, Page=127, 1 page)` → 8kB
 * - **Program**: `HAL_FLASH_Program(QUADWORD)` → 128-bit/16B chunks
 * 
 * 
 * @section NOTES Important Notes
 * 
 * - NVM **protected** against reflash (Bank2, CubeProgrammer does not touch it)
 * - **Erase before every writing** (Flash requires it )
 * - **QUADWORD** = 16B chunks (`i += 16`) for STM32WBA6
 * - **Wear-leveling**: manual, by erasint the entire page (8kB)
 */

/**
 ******************************************************************************
  * File Name          : flash.h
  * Description        : OpenThread platform flash header
  ******************************************************************************
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FLASH_H
#define FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <openthread/instance.h>
  
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

// Keys in range 0x0001-0x000e are reserved for OT_SETTINGS_KEY use.
// Keys in range 0x0100-0x010A are reserved for ot_app use.
// Keys in range 0x8000-0xffff are reserved for vendor-specific use.
#define OT_APP_KEY_MIN = 0x0100,
#define OT_APP_KEY_MAX = 0x010A,
#define OT_APP_KEY_RESERVED_MIN_L = 0x0001,
#define OT_APP_KEY_RESERVED_MAX_L = 0x000e,
#define OT_APP_KEY_RESERVED_MIN_H = 0x8000,
#define OT_APP_KEY_RESERVED_MAX_H = 0xffff,

typedef enum{
  SETTINGS_ADDED,
  SETTINGS_REMOVED,
  SETTINGS_MASSERASE
}settings_type_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Exported types ------------------------------------------------------------*/

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief this callback will be called each time settings have been added or removed
 * depending of setting update types user can define action
 *
 * @param SettingType
 * SETTINGS_ADDED : Advice to add settings in ROM by task in low prio to keep better ot performances
 * SETTINGS_REMOVED : Advice to remove settings in ROM by task in low prio to keep better ot performances
 * SETTINGS_MASSERASE : ROM need to be erased right now in blocking way, device will reset just after otPlatSettingsWipe
 *
 * Please use GetSettingBuffer() to get OT settings buffer & current size to copy in ROM
*/
void APP_THREAD_SettingsUpdated(settings_type_t SettingType);

otError APP_THREAD_KeySave(uint16_t aKey, const uint8_t *aValue, uint16_t aValueLength);
otError APP_THREAD_KeyRead(uint16_t aKey, uint8_t *aValueOut, uint16_t *aValueLengthOut);
otError APP_THREAD_KeyDelete(uint16_t aKey);

/**
 * @brief return maximal size of setting buffer
*/
uint32_t GetSettingsBuffer_MaxSize(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* FLASH_H */



