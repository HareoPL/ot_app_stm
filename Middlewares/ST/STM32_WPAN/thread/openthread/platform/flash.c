#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_compiler.h"
#include "platform/settings.h"
#include "flash.h"
#include OPENTHREAD_CONFIG_FILE

#include "ot_app.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "timers.h"
/******************************************************************************
 * NON VOLATILE STORAGE BUFFER
 *
 * This temporary buffer is located in SRAM and is then copied in FLASH in
 * order to store the non volatile data requested by Thread.
 *
 * Each pages has a size of  SETTINGS_CONFIG_PAGE_SIZE bytes.
 *
 * These two values SETTINGS_CONFIG_PAGE_NUM and SETTINGS_CONFIG_PAGE_SIZE are
 * defined by OpenThread (Refer to stm32wb-openthread-ftd-config.h and
 * stm32wb-openthread-mtd-config.h).
 *
 * 

Table 40. Flash module 2-Mbyte dual bank organization (RM0515)

| Flash area  | Bank   | Flash memory address secure(1)   | Flash memory address nonsecure(1) | Size (bytes) | Name     |
|-------------|--------|----------------------------------|-----------------------------------|--------------|----------|
| Main memory | Bank 1 | 0x0C00 0000 - 0x0C00 1FFF        | 0x0800 0000 - 0x0800 1FFF         | 8 K          | Page 0   |
|             |        | 0x0C00 2000 - 0x0C00 3FFF        | 0x0800 2000 - 0x0800 3FFF         | 8 K          | Page 1   |
|             |        | ...                              | ...                               | ...          | ...      |
|             |        | 0x0C0F E000 - 0x0C0F FFFF        | 0x080F E000 - 0x080F FFFF         | 8 K          | Page 127 |
|             | Bank 2 | 0x0C10 0000 - 0x0C10 1FFF        | 0x0810 0000 - 0x0810 1FFF         | 8 K          | Page 1   |
|             |        | 0x0C10 2000 - 0x0C10 3FFF        | 0x0810 2000 - 0x0810 3FFF         | 8 K          | Page 2   |
|             |        | ...                              | ...                               | ...          | ...      |
|             |        | 0x0C1F E000 - 0x0C1F FFFF        | 0x081F E000 - 0x081F FFFF         | 8 K          | Page 127 |

 ***************************************************************************************************************************/

#define TAG "flash "

// #define FLASH_PAGE_SIZE (1024 * 8)  // STM32WBA6
#define FLASH_CHUNK       16        // STM32WBA6 QUADWORD (128-bit/16) flash programming

#define NVM_SLOT_TOTAL_SIZE (1024 * 2)  // must be divisible by 2
#define NVM_NUM_OF_SLOTS (FLASH_PAGE_SIZE / NVM_SLOT_TOTAL_SIZE) 

#define NVM_TIMER_DELAY   10000     // debounce for saving in RAM (ms)
#define NVM_TASK_STACK  (256 * 4)

#define THREAD_SETTINGS_RESET_FLAG 0x0784EAD0
#define NVM_MAGIC_NUM             THREAD_SETTINGS_RESET_FLAG

typedef struct OT_TOOL_PACKED_END {
    uint32_t magicNum;   
    uint32_t blockLenght;   
    uint32_t crc;
} nvmSlotInfo_t; // saved into flash, on the last 12 bits of block

typedef struct OT_TOOL_PACKED_END {
  uint8_t dataBlock[NVM_SLOT_TOTAL_SIZE - sizeof(nvmSlotInfo_t)];
  nvmSlotInfo_t slotInfo;
} nvm_t;

struct settingsBlock
{
    uint16_t key;
    uint16_t length;
} OT_TOOL_PACKED_END;

#define THREAD_SETTINGS_BUFFER_SIZE (NVM_SLOT_TOTAL_SIZE - sizeof(nvmSlotInfo_t))

static TimerHandle_t nvmTimer; // Handle to control the nvm timer
static osThreadId_t NVM_TaskHandle;

/* Flag to know whether we are in reset */
HRO_SEC_NOINIT PRIVATE uint32_t sSettingsIsReset;

/* Pointer to Buf currently used */
HRO_SEC_NOINIT PRIVATE uint32_t sSettingsBufPos;


// HRO_SEC_NOINIT_AL16
HRO_ALIGN_16 PRIVATE nvm_t nvmRam;
HRO_SEC_NVM_AL16 PRIVATE nvm_t nvmFlash; // reserved space in flash for building
PRIVATE nvm_t *nvmFlashAddr;      // actual address. 
PRIVATE nvm_t *nvmFlashAddrLast;  // last address of page. 


PRIVATE nvm_t *nvm_flash_checkNewSlot(void)
{
  nvm_t *nvmFlashAddr_current = &nvmFlash;
  nvm_t *nvmFlashAddr_newest = NULL;

  // looking for the newest nvm slot
  for (uint8_t i = 0; i < NVM_NUM_OF_SLOTS; i++)
  {
    if(nvmFlashAddr_current->slotInfo.magicNum == NVM_MAGIC_NUM)
    {
      nvmFlashAddr_newest = nvmFlashAddr_current;
    }
    
    nvmFlashAddr_current ++;  
  }

  if(nvmFlashAddr_newest == NULL)
  {
    return &nvmFlash;
  }

  return nvmFlashAddr_newest;
}

PRIVATE void nvm_flash_settingsLoadFromFlash(void)
{ 
  // copy nvm_t struct  
    memcpy(&nvmRam, nvmFlashAddr, sizeof(nvm_t));
}

PRIVATE void nvm_flash_settingsSaveToFlash(void)
{
    uint32_t flashAddr; 
    uint32_t dataAddr = (uint32_t)&nvmRam;

    uint32_t page_error = 0;
    uint32_t i;
    uint32_t chunkCount = 0;

    uint8_t erasePage = 0;
    HAL_StatusTypeDef status;

    FLASH_EraseInitTypeDef erase = {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .Banks     = FLASH_BANK_2,
        .Page      = 127,
        .NbPages   = 1  
    };
    
    nvmFlashAddr ++;

    if(nvmFlashAddr >= nvmFlashAddrLast)
    {
      nvmFlashAddr = &nvmFlash;
      erasePage = 1; // DO ERASE
    }

    flashAddr = (uint32_t)nvmFlashAddr;
    
    // set new slotInfo into ram
    nvmRam.slotInfo.blockLenght = sSettingsBufPos;
    nvmRam.slotInfo.magicNum = NVM_MAGIC_NUM;
//    nvmRam.slotInfo.crc

    HAL_FLASH_Unlock();

    // clear error flags
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

   if(erasePage)
   {
      status = HAL_FLASHEx_Erase(&erase, &page_error);
      if(status != HAL_OK)
      {
          OTAPP_PRINTF(TAG, "Erase FAIL status=%d page_error=0x%lX\n", status, page_error);
          HAL_FLASH_Lock();
          return;
      }
   }
    
    for(i = 0; i < NVM_SLOT_TOTAL_SIZE / FLASH_CHUNK; i++) 
    {
        // flash programming  128-bits (QUADWORD)
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, flashAddr, dataAddr);

        if(status != HAL_OK)
        {
            OTAPP_PRINTF(TAG, "Program FAIL chunk=%lu error=0x%lX\n", i, HAL_FLASH_GetError());
            break; 
        }
        
        chunkCount++;
        flashAddr += FLASH_CHUNK; // shift 16 bytes (128 bits)
        dataAddr  += FLASH_CHUNK;
    }

    HAL_FLASH_Lock();
    OTAPP_PRINTF(TAG, "Flash OK: %lu chunks (%dB)\n", chunkCount, chunkCount * 16);
}

void NVM_task(void *argument)
{

  while(1)
  {
    // compare ram vs flash
    if(memcmp(nvmRam.dataBlock, nvmFlashAddr->dataBlock, THREAD_SETTINGS_BUFFER_SIZE) != 0 )
    {
      nvm_flash_settingsSaveToFlash();
      OTAPP_PRINTF(TAG, "NVM: settings saved \n");
    }
    
    osThreadSuspend(NVM_TaskHandle);
  }
}

static void timerNvmCallback(TimerHandle_t xTimer) 
{
  osThreadResume(NVM_TaskHandle);  // wake up nvm task 
  OTAPP_PRINTF(TAG, "Timer NVM: resume NVM task \n");
}


static void APP_THREAD_NvmTimerStart(void)
{
  xTimerReset(nvmTimer, 0);
}

static void APP_THREAD_NvmTimerInit(void)
{
  nvmTimer = xTimerCreate(
      "NVM_Timer",            
      pdMS_TO_TICKS(NVM_TIMER_DELAY),    
      pdFALSE,                // pdFALSE = One-shot , pdTRUE = Auto-reload 
      (void *) 0,             // ID timer 
      timerNvmCallback        
  );

  if(nvmTimer == NULL) 
  {
    OTAPP_PRINTF(TAG, "Timer NVM: no more heap \n"); 
  }
}

static void APP_THREAD_NvmTaskInit(void)
{
  osThreadAttr_t nvmTask_attr = {
    .name = "NVMTask",
    .priority = osPriorityLow,
    .stack_size = NVM_TASK_STACK,
  };
  NVM_TaskHandle = osThreadNew(NVM_task, NULL, &nvmTask_attr);
  osThreadSuspend(NVM_TaskHandle);  // START SUSPENDED!
}

static void APP_THREAD_NvmInit(void)
{
  APP_THREAD_NvmTaskInit();
  APP_THREAD_NvmTimerInit();
}

void APP_THREAD_SettingsUpdated(settings_type_t SettingType)
{
  APP_THREAD_NvmTimerStart();
}

uint32_t GetSettingsBuffer_Base(void)
{
  return (uint32_t)nvmRam.dataBlock;
}

uint32_t GetSettingsBuffer_MaxSize(void)
{
  return (THREAD_SETTINGS_BUFFER_SIZE);
}


void otPlatSettingsInit(otInstance *aInstance, const uint16_t *aSensitiveKeys, uint16_t aSensitiveKeysLength)
{
  OT_UNUSED_VARIABLE(aInstance);
  OT_UNUSED_VARIABLE(aSensitiveKeys);
  OT_UNUSED_VARIABLE(aSensitiveKeysLength);

  nvmFlashAddrLast = (nvm_t *)((uintptr_t)&nvmFlash + FLASH_PAGE_SIZE);

  nvmFlashAddr = nvm_flash_checkNewSlot();

  nvm_flash_settingsLoadFromFlash();

  // Scan the buffer to find a place for sSettingsBufPos
  // uint32_t current = GetSettingsBuffer_Base();
  // uint32_t limit = current + THREAD_SETTINGS_BUFFER_SIZE;  
  // while (current + sizeof(struct settingsBlock) <= limit)
  // {
  //     struct settingsBlock *block = (struct settingsBlock *)current;
  //     if (block->key == 0xFFFF || block->key == 0x0000) break;
      
  //     uint32_t next = current + sizeof(struct settingsBlock) + block->length;
  //     if (next > limit) break;
  //     current = next;
  // }
  // sSettingsBufPos = current; 


  if(nvmRam.slotInfo.blockLenght == 0xFFFFFFFF || nvmRam.slotInfo.blockLenght == 0x00000000)
  {
    sSettingsBufPos = GetSettingsBuffer_Base();
  }else{
    sSettingsBufPos = nvmRam.slotInfo.blockLenght;
  }
  sSettingsIsReset = THREAD_SETTINGS_RESET_FLAG;

  APP_THREAD_NvmInit();
}

void otPlatSettingsDeinit(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
}

otError otPlatSettingsGet(otInstance *aInstance, uint16_t aKey, int aIndex, uint8_t *aValue, uint16_t *aValueLength)
{
  OT_UNUSED_VARIABLE(aInstance);

  const struct settingsBlock *currentBlock;
  uint32_t buf_pos = GetSettingsBuffer_Base();
  int currentIndex = 0;
  uint16_t readLength;
  uint16_t valueLength = 0U;
  otError error = OT_ERROR_NOT_FOUND;

  while (buf_pos < sSettingsBufPos)
  {
    currentBlock = (struct settingsBlock *)(buf_pos);

    if (aKey == currentBlock->key)
    {
      if (currentIndex == aIndex)
      {
        readLength = currentBlock->length;

        // Perform read only if an input buffer was passed in
        if (aValue != NULL && aValueLength != NULL)
        {
          // Adjust read length if input buffer size is smaller
          if (readLength > *aValueLength)
          {
            readLength = *aValueLength;
          }
          memcpy(aValue, (uint8_t *)(buf_pos + sizeof(struct settingsBlock)), readLength);
        }
        valueLength = currentBlock->length;
        error = OT_ERROR_NONE;
        break;
      }
      currentIndex++;
    }
    buf_pos += (sizeof(struct settingsBlock) + currentBlock->length);
  }

  if (aValueLength != NULL)
  {
    *aValueLength = valueLength;
  }

  return error;
}

otError otPlatSettingsAdd(otInstance *aInstance, uint16_t aKey, const uint8_t *aValue, uint16_t aValueLength)
{
  OT_UNUSED_VARIABLE(aInstance);
  OT_UNUSED_VARIABLE(aValue);

  otError error;
  struct settingsBlock *currentBlock;
  const uint16_t newBlockLength = sizeof(struct settingsBlock) + aValueLength;

  if ( (sSettingsBufPos +  newBlockLength) <= (GetSettingsBuffer_Base() + GetSettingsBuffer_MaxSize()) )
  {
    currentBlock         = (struct settingsBlock *)sSettingsBufPos;
    currentBlock->key    = aKey;
    currentBlock->length = aValueLength;

    memcpy((uint8_t*) (sSettingsBufPos + sizeof(struct settingsBlock)), aValue, aValueLength);
    
    /* Update current position on Buffer */
    sSettingsBufPos += newBlockLength;
    
    /* Callback to prevent user settings has been added */
    APP_THREAD_SettingsUpdated(SETTINGS_ADDED);

    error = OT_ERROR_NONE;
  }
  else
  {
    error = OT_ERROR_NO_BUFS;
  }

  return error;
}

otError otPlatSettingsDelete(otInstance *aInstance, uint16_t aKey, int aIndex)
{
  OT_UNUSED_VARIABLE(aInstance);

  const struct settingsBlock *currentBlock;
  uint32_t buf_pos = GetSettingsBuffer_Base();
  int currentIndex = 0;
  uint16_t currentBlockLength;
  uint32_t nextBlockStart;
  otError error = OT_ERROR_NOT_FOUND;

  while (buf_pos < sSettingsBufPos)
  {
    currentBlock = (struct settingsBlock *)(buf_pos);
    currentBlockLength = sizeof(struct settingsBlock) + currentBlock->length;
    
    if (aKey == currentBlock->key)
    {
      if ((currentIndex == aIndex)||(aIndex == -1))
      {
        nextBlockStart = buf_pos + currentBlockLength;
        if (nextBlockStart < sSettingsBufPos)
        {
          memmove((uint8_t*)buf_pos, (uint8_t*)nextBlockStart, (sSettingsBufPos - nextBlockStart));
        }

        sSettingsBufPos -= currentBlockLength;
        
        /* Callback to prevent user settings has been added */
        APP_THREAD_SettingsUpdated(SETTINGS_REMOVED);
        error = OT_ERROR_NONE;
        break;
      }
      else
      {
        currentIndex++;
      }
    }
    buf_pos += currentBlockLength;
  }
  return error;
}

otError otPlatSettingsSet(otInstance *aInstance, uint16_t aKey, const uint8_t *aValue, uint16_t aValueLength)
{
  OT_UNUSED_VARIABLE(aInstance);

  const struct settingsBlock *currentBlock;
  uint32_t buf_pos = GetSettingsBuffer_Base();
  uint16_t currentBlockLength;
  uint32_t nextBlockStart;

  while (buf_pos < sSettingsBufPos)
  {
    currentBlock = (struct settingsBlock *)(buf_pos);
    currentBlockLength = sizeof(struct settingsBlock) + currentBlock->length;
    if (aKey == currentBlock->key)
    {
      nextBlockStart = buf_pos + currentBlockLength;
      if (nextBlockStart < sSettingsBufPos)
      {
        memmove((uint8_t *)(buf_pos), (uint8_t*)nextBlockStart, (sSettingsBufPos - nextBlockStart));
      }
      sSettingsBufPos -= currentBlockLength;
    }
    else
    {
      buf_pos += currentBlockLength;
    }
  }

  return otPlatSettingsAdd(aInstance, aKey, aValue, aValueLength);
}

void otPlatSettingsWipe(otInstance *aInstance)
{
  /* Reset pos & reset flag like at first init */
  sSettingsIsReset = THREAD_SETTINGS_RESET_FLAG;
  sSettingsBufPos = GetSettingsBuffer_Base();
  APP_THREAD_SettingsUpdated(SETTINGS_MASSERASE);
}

otError APP_THREAD_KeySave(uint16_t aKey, const uint8_t *aValue, uint16_t aValueLength)
{
  if(aValue == NULL || aValueLength == 0)
  {
    return OT_ERROR_FAILED;
  }
  return otPlatSettingsSet(NULL, aKey, aValue, aValueLength);
}

otError APP_THREAD_KeyRead(uint16_t aKey, uint8_t *aValueOut, uint16_t *aValueLengthOut)
{
  if(aValueOut == NULL || aValueLengthOut == 0)
  {
    return OT_ERROR_FAILED;
  }
  return otPlatSettingsGet(NULL, aKey, 0, aValueOut, aValueLengthOut);
}

otError APP_THREAD_KeyDelete(uint16_t aKey)
{
  return otPlatSettingsDelete(NULL, aKey, 0);
}


