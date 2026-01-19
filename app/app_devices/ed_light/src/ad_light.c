/**
 * @file ad_light.c
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief 
 * @version 0.1
 * @date 06-09-2025
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


#include "ad_light.h"
#include "string.h"
#include "ot_app_drv.h"
#include "ad_light_uri.h"

#define TAG "ad_light "

/**
 * @brief Device name group buffer
 * 
 * @details
 * Internal buffer storing the device name group identifier used for network pairing.
 * 
 * **Buffer Properties:**
 * - Size: OTAPP_DEVICENAME_SIZE (10 bytes)
 * - Maximum string length: 9 characters + null terminator
 * - Initialized by ad_button_Init(char *deviceNameGroup)
 * - Used by OpenThread framework for device discovery and pairing
 * 
 * **Purpose:**
 * - Identifies this button device on the OpenThread network
 * - Filters pairing: only devices with matching name group can pair
 * - Enables logical grouping of devices (e.g., by room or function)
 * 
 * @note This buffer persists for the lifetime of the application
 * @note Content is set once during initialization via strcpy()
 * @note Buffer is assigned to drv->deviceName during ad_button_Init()
 * 
 * @see ad_button_Init() for initialization
 * @see OTAPP_DEVICENAME_SIZE for buffer size definition
 */
static char ad_button_deviceNameTab[OTAPP_DEVICENAME_SIZE];

static const otapp_deviceType_t ad_light_deviceType = OTAPP_LIGHTING;
static ot_app_devDrv_t *drv;

//////////////////////
// RULES 
// OTAPP_PAIR_RULES_ALLOWED_SIZE = 10
static otapp_pair_rule_t ad_light_deviceRules_all_allowed = {
    .allowed = {OTAPP_PAIR_NO_RULES, OTAPP_PAIR_END_OF_RULES} // NO RULES, pair every incoming device
};
static otapp_pair_rule_t ad_light_deviceRules_no_allowed = {
    .allowed = {OTAPP_PAIR_NO_ALLOWED, OTAPP_PAIR_END_OF_RULES} // every devices are not allowed
};
static otapp_pair_rule_t ad_light_deviceRules = {
    .allowed = {OTAPP_LIGHTING, OTAPP_LIGHTING_ON_OFF, OTAPP_LIGHTING_DIMM, OTAPP_LIGHTING_RGB, OTAPP_PAIR_END_OF_RULES}   
};

otapp_pair_rule_t *ad_light_pairRulesGetList_all_allowed()
{
    return &ad_light_deviceRules_all_allowed;
}

otapp_pair_rule_t *ad_light_pairRulesGetList_no_allowed()
{
    return &ad_light_deviceRules_no_allowed;
}

otapp_pair_rule_t *ad_light_pairRulesGetList()
{
    return &ad_light_deviceRules;
}

//////////////////////
//observer 

void ad_light_pairedCallback(otapp_pair_Device_t *newDevice)
{    
    OTAPP_PRINTF(TAG, "Dev Light detect NEW DEVICE! %s \n", newDevice->devNameFull);
    OTAPP_PRINTF(TAG, "      uri 0: %s\n", newDevice->urisList[0].uri);
    OTAPP_PRINTF(TAG, "      uri 1: %s\n", newDevice->urisList[1].uri);
    OTAPP_PRINTF(TAG, "      uri 2: %s\n", newDevice->urisList[2].uri);
   
}

void ad_light_subscribedUrisCallback(oac_uri_dataPacket_t *data) 
{
    OTAPP_PRINTF(TAG, "@ Dev Light from subs: \n");
    OTAPP_PRINTF(TAG, " @--> token: 0x%x 0x%x 0x%x 0x%x\n", data->token[0], data->token[1], data->token[2], data->token[3]);
    OTAPP_PRINTF(TAG, " @--> data: %d\n", data->buffer[0]);
}

void ad_light_task()
{
   
}

//////////////////////
// init
void ad_light_init(char *deviceNameGroup)
{    
    if(deviceNameGroup == NULL) return;
    strcpy(ad_button_deviceNameTab, deviceNameGroup);

    drv = ot_app_drv_getInstance();
    
    if(drv->api.nvs.init == NULL) return ;
    drv->api.nvs.init();
    
    ad_light_uri_init(drv);

    // drv->pairRuleGetList_clb = ad_light_pairRulesGetList_all_allowed;     // if you want to pair all devices
    drv->pairRuleGetList_clb = ad_light_pairRulesGetList_no_allowed;      // if you do not want to pair devices
    // drv->pairRuleGetList_clb = ad_light_pairRulesGetList;                    // if you have some rules
    
    drv->uriGetList_clb = ad_light_uri_getList;
    drv->uriGetListSize = ad_light_uri_getListSize();

    drv->obs_pairedDevice_clb = ad_light_pairedCallback;
    drv->obs_subscribedUri_clb = ad_light_subscribedUrisCallback;

    drv->deviceName = ad_button_deviceNameTab;
    drv->deviceType = &ad_light_deviceType;
    drv->task = ad_light_task;
}

