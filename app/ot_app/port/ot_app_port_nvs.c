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

#include "ot_app_port_nvs.h"
#include "stdio.h"
#include <string.h>

#define OT_APP_NVS_BUF_SIZE     (5 + 3 + 1) // nvsMainKey + 1byte have max 3 char(0-255) + EOC

static const char *NAMESPACE = "otapp_device";
static const char *nvsMainKey = "otapp";

static char *ot_app_nvs_keyMake(char *buff, uint8_t keyId)
{
    snprintf(buff, OT_APP_NVS_BUF_SIZE, "%s%d", nvsMainKey, keyId);

    return buff;
}


#ifdef ESP_PLATFORM
    #include "nvs_flash.h"
    #include "nvs.h"


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

    void ot_app_nvs_test()
    {
        char buff[128];

        const char *STRING_TO_WRITE_0 = "otapp_0 TEST_0";
        const char *STRING_TO_WRITE_1 = "otapp_1 TEST_1";
        const char *STRING_TO_WRITE_2 = "otapp_2 TEST_2";

        ot_app_nvs_saveString(STRING_TO_WRITE_0, 0);
        ot_app_nvs_saveString(STRING_TO_WRITE_1, 1);
        ot_app_nvs_saveString(STRING_TO_WRITE_2, 2);

        printf("check if saved:\n");
        ot_app_nvs_readString(buff, 128, 0);    
        printf("   && NVS TEST read: %s \n", buff );
        ot_app_nvs_readString(buff, 128, 1);    
        printf("   && NVS TEST nfs read: %s \n", buff );
        ot_app_nvs_readString(buff, 128, 2);    
        printf("   && NVS TEST nfs read: %s \n", buff );
        
        printf("\nClear... \n");
        ot_app_nvs_saveString("\0", 0);
        ot_app_nvs_saveString("\0", 1);
        ot_app_nvs_saveString("\0", 2);
        buff[0] = '\0';

        printf("check if cleared: \n");
        ot_app_nvs_readString(buff, 128, 0);    
        printf("   && NVS TEST read: %s \n", buff );
        ot_app_nvs_readString(buff, 128, 1);    
        printf("   && NVS TEST nfs read: %s \n", buff );
        ot_app_nvs_readString(buff, 128, 2);    
        printf("   && NVS TEST nfs read: %s \n", buff );


    }

#elif defined(STM_PLATFORM)

    int8_t ot_app_nvs_init(void)
    {

    }

    int8_t ot_app_nvs_saveString(const char *inData, const uint8_t keyId)
    {

    }

    int8_t ot_app_nvs_readString(char *outBuff, uint8_t outBuffSize, const uint8_t keyId) 
    {

    }

    void ot_app_nvs_test()
    {
        
    }
#endif