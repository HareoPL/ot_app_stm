/**
 * @file ot_app_nvs.c
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief 
 * @version 0.1
 * @date 24-10-2025
 * 
 * @copyright The MIT License (MIT) Copyright (c) 2025 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 * 
 */

// nvs is initialize in otapp_cli_init(), ot_app.c
#include "ot_app.h"
#include "ot_app_port_nvs.h"
#include "hro_utils.h"
#include "stdio.h"
#include <string.h>

#define TAG "ot_app_nvs "

#ifdef ESP_PLATFORM
    #include "nvs_flash.h"
    #include "nvs.h"

    #define OT_APP_NVS_BUF_SIZE     (5 + 3 + 1) // nvsMainKey + 1byte have max 3 char(0-255) + EOC

    static const char *NAMESPACE = "otapp_device";
    static const char *nvsMainKey = "otapp";

    static char *ot_app_nvs_keyMake(char *buff, uint8_t keyId)
    {
        snprintf(buff, OT_APP_NVS_BUF_SIZE, "%s%d", nvsMainKey, keyId);

        return buff;
    }

    static esp_err_t err = ESP_OK;
    static nvs_handle_t nvsHandle = 0;

    int8_t ot_app_nvs_init(void)
    {
        err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
        {          
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
        }
        
        err = nvs_open(NAMESPACE, NVS_READWRITE, &nvsHandle);
        if (err != ESP_OK) 
        {      
            return OT_APP_NVS_ERROR;
        }
        
        return OT_APP_NVS_OK;
    }

    int8_t ot_app_nvs_saveString(const char *inData, const uint8_t keyId)
    {
        uint8_t strLen = 0;
        if(inData == NULL || nvsHandle == 0) return OT_APP_NVS_ERROR;

        char keyName[OT_APP_NVS_BUF_SIZE];
        ot_app_nvs_keyMake(keyName, keyId);
        
        strLen = strlen(inData);
        if(strLen == 0)
        {
            nvs_set_u8(nvsHandle, keyName, 0);
        }
        else
        {
            err = nvs_set_str(nvsHandle, keyName, inData);
            if (err != ESP_OK) 
            {
                return OT_APP_NVS_ERROR;
            }
        }
    
        err = nvs_commit(nvsHandle);
        if (err != ESP_OK) 
        {
        return OT_APP_NVS_ERROR;
        } 
        
        return err;
    }

    int8_t ot_app_nvs_readString(char *outBuff, uint8_t outBuffSize, const uint8_t keyId) 
    {
        if(outBuff == NULL || nvsHandle == 0) return OT_APP_NVS_ERROR;
        
        size_t required_size = outBuffSize;
        char keyName[OT_APP_NVS_BUF_SIZE];

        err = nvs_get_str(nvsHandle, ot_app_nvs_keyMake(keyName, keyId), outBuff, &required_size);    
        if (err != ESP_OK) 
        {
            return OT_APP_NVS_ERROR;
        } 

        return OT_APP_NVS_OK;
    }

    int8_t ot_app_nvs_deleteString(const uint8_t keyId) 
    {
        // todo implement delete function for esp32
       return OT_APP_NVS_ERROR;
    }
    

#elif defined(STM_PLATFORM)

    #include "flash.h"

    uint16_t ot_app_nvs_keyIdShift(uint8_t keyId)
    {   
        uint16_t keyIdSh;
        
        keyIdSh = keyId + 1; // same as ++keyId but less readable 
        keyIdSh = (keyIdSh << 8);

        return keyIdSh;
    }

    int8_t ot_app_nvs_init(void)
    {
        return OT_APP_NVS_OK;
    }

    int8_t ot_app_nvs_saveString(const char *inData, const uint8_t keyId)
    {
        if(inData == NULL)
        {
            return OT_APP_NVS_ERROR;
        }
        otError result; 
        uint16_t keyIdSh = ot_app_nvs_keyIdShift(keyId); 

        uint16_t strLen;
        if(*inData == '\0') // clear key
        {
            strLen = 1;
        }else{
            strLen = strlen(inData);
        }

        result = APP_THREAD_KeySave(keyIdSh, (uint8_t*)inData, strLen);
        if(result != OT_ERROR_NONE)
        {
            return OT_APP_NVS_IS_NO_SPACE;
        }

        return OT_APP_NVS_OK;
    }

    int8_t ot_app_nvs_readString(char *outBuff, uint8_t outBuffSize, const uint8_t keyId) 
    {
        if(outBuff == NULL)
        {
            return OT_APP_NVS_ERROR;
        }
        otError result; 
        uint16_t keyIdSh = ot_app_nvs_keyIdShift(keyId); 
        uint16_t valueLenght = outBuffSize;

        result = APP_THREAD_KeyRead(keyIdSh, (uint8_t*)outBuff, &valueLenght);

        if(result == OT_ERROR_NOT_FOUND)
        {
            return OT_APP_NVS_IS_NOT;
        }

        if(result != OT_ERROR_NONE || valueLenght >= outBuffSize)
        {
            return OT_APP_NVS_ERROR;
        }

        *(outBuff + valueLenght + 1) = '\0';

        return OT_APP_NVS_OK;
    }

    int8_t ot_app_nvs_deleteString(const uint8_t keyId) 
    {
    	 uint16_t keyIdSh = ot_app_nvs_keyIdShift(keyId);

       return APP_THREAD_KeyDelete(keyIdSh);
    }
    
#endif

    
void ot_app_nvs_test()
    {
        char buff[128];
        int8_t result;

       const char *STRING_TO_WRITE[3] = {
            "otapp_0 TEST_0",
            "otapp_1 TEST_1",
            "otapp_2 TEST_2",
        };

        for (uint8_t i = 0; i < 3; i++)
        {
            OTAPP_PRINTF(TAG, "Test: %d/3 \n", i+1 );
            OTAPP_PRINTF(TAG, "     Save data \n");
            ot_app_nvs_saveString(STRING_TO_WRITE[i], i);            
            ot_app_nvs_readString(buff, sizeof(buff), i);
            OTAPP_PRINTF(TAG, "     Read data: %s \n", buff );
            
            OTAPP_PRINTF(TAG, "  Clear data: \n");
            ot_app_nvs_saveString("\0", i);
            ot_app_nvs_readString(buff, sizeof(buff), i);
            OTAPP_PRINTF(TAG, "     Read data if clear: %s \n", buff );

            OTAPP_PRINTF(TAG, "  Delete data: \n");
            ot_app_nvs_deleteString(i);
            result = ot_app_nvs_readString(buff, sizeof(buff), i);  

            if(result == OT_APP_NVS_IS_NOT)
            {
                OTAPP_PRINTF(TAG, "     key deleted \n\n" );
            }else
            {
                OTAPP_PRINTF(TAG, "     key NOT deleted \n\n");
            }
        }                
    }
