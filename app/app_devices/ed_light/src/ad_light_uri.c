/**
 * @file ad_light_uri.c
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief 
 * @version 0.1
 * @date 13-11-2025
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
#include "ad_light_uri.h"
#include "ad_light_control.h"

#define TAG "ad_light_uri "

static ot_app_devDrv_t *drv;

static uint8_t coapBuffer[OAC_URI_OBS_BUFFER_SIZE + 1];

void ad_light_uri_printResult(int8_t result);


void ad_light_uri_light_on_off_CoreHandle(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    int8_t result = 0;  

    result = drv->api.coap.processUriRequest(aMessage, aMessageInfo, OTAPP_LIGHTING_ON_OFF, coapBuffer, OAC_URI_OBS_BUFFER_SIZE);

    // if(result == OTAPP_COAP_OK_OBSERVER_REQUEST)
    if(result > 0 || result == OAC_URI_OBS_ADDED_NEW_DEVICE || result == OAC_URI_OBS_NO_NEED_UPDATE)
    {
        OTAPP_PRINTF(TAG, "@ DEVICE URI light_on_off: observer - processed \n");
        ad_light_uri_printResult(result);        

    }else if(result == OTAPP_COAP_OK)
    {
        // handle uri request here       
        ad_light_ctr_onOff(coapBuffer[0]);

        OTAPP_PRINTF(TAG, "@ DEVICE URI light_on_off payload: \n");
        OTAPP_PRINTF(TAG, "  on_off value %d \n", coapBuffer[0]);
    }else
    {
         OTAPP_PRINTF(TAG, "@ DEVICE URI light_on_off: error: %d \n", result);
    }    
}

void ad_light_uri_light_dimm_CoreHandle(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    int8_t result = 0;
    uint32_t dimm_ = 0;

    result = drv->api.coap.processUriRequest(aMessage, aMessageInfo, OTAPP_LIGHTING_DIMM, coapBuffer, OAC_URI_OBS_BUFFER_SIZE);

    if(result > 0 || result == OAC_URI_OBS_ADDED_NEW_DEVICE || result == OAC_URI_OBS_NO_NEED_UPDATE)
    {
        OTAPP_PRINTF(TAG, "@ DEVICE URI light_dimm: observer - processed \n");
        ad_light_uri_printResult(result);  
    }else if(result == OTAPP_COAP_OK)
    {
        // handle uri request here

        // Parse 32-bit state value from buffer (little-endian)    
        dimm_   |=  (uint32_t)coapBuffer[0];
        dimm_   |= ((uint32_t)coapBuffer[1] << 8);
        dimm_   |= ((uint32_t)coapBuffer[2] << 16);
        dimm_   |= ((uint32_t)coapBuffer[3] << 24);

        ad_light_ctr_dimSet(dimm_);

        OTAPP_PRINTF(TAG, "@ DEVICE URI light_dimm payload: \n");
        OTAPP_PRINTF(TAG, "   dimValue: %ld \n", dimm_);
    }else
    {
         OTAPP_PRINTF(TAG, "@ DEVICE URI light_dimm: error: %d \n", result);
    }      
}

void ad_light_uri_light_rgb_CoreHandle(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    int8_t result = 0;    
    uint8_t color_R_ = 0;
    uint8_t color_G_ = 0;
    uint8_t color_B_ = 0;

    result = drv->api.coap.processUriRequest(aMessage, aMessageInfo, OTAPP_LIGHTING_RGB, coapBuffer, OAC_URI_OBS_BUFFER_SIZE);

    if(result > 0 || result == OAC_URI_OBS_ADDED_NEW_DEVICE || result == OAC_URI_OBS_NO_NEED_UPDATE)
    {
        OTAPP_PRINTF(TAG, "@ DEVICE URI light_rgb: observer - processed \n");
        ad_light_uri_printResult(result);  

    }else if(result == OTAPP_COAP_OK)
    {
        // handle uri request here
        color_R_ = coapBuffer[2];
        color_G_ = coapBuffer[1];
        color_B_ = coapBuffer[0];

        ad_light_ctr_colorSet(color_R_, color_G_, color_B_);

        OTAPP_PRINTF(TAG, "@ DEVICE URI light_rgb payload: \n");
        OTAPP_PRINTF(TAG, "   R: %d, G: %d, B %d \n", color_R_, color_G_, color_B_);
    }else
    {
         OTAPP_PRINTF(TAG, "@ DEVICE URI light_rgb: error: %d \n", result);
    }  
}

// max uris:            OTAPP_PAIRED_URI_MAX
// max lengh uri name:  OTAPP_COAP_URI_MAX_LENGHT
static otapp_coap_uri_t ad_light_uri[] = {   
    {OTAPP_LIGHTING_ON_OFF,  {"light/on_off", ad_light_uri_light_on_off_CoreHandle, NULL, NULL},},
    {OTAPP_LIGHTING_DIMM,    {"light/dimm", ad_light_uri_light_dimm_CoreHandle, NULL, NULL},},
    {OTAPP_LIGHTING_RGB,     {"light/rgb", ad_light_uri_light_rgb_CoreHandle, NULL, NULL},},
 
};
#define AD_LIGHT_URI_SIZE (sizeof(ad_light_uri) / sizeof(ad_light_uri[0]))

otapp_coap_uri_t *ad_light_uri_getList(void)
{
    return ad_light_uri;
}

uint8_t ad_light_uri_getListSize(void)
{
    return AD_LIGHT_URI_SIZE;
}

void ad_light_uri_printResult(int8_t result)
{
    switch(result)
    {
    case 1: // OAC_URI_OBS_UPDATE_IP_ADDR_Msk
        OTAPP_PRINTF(TAG, " @--> update IP ADDR \n");
        break;
        
    case 2: // OAC_URI_OBS_UPDATE_URI_TOKEN_Msk
        OTAPP_PRINTF(TAG, " @--> update TOKEN uri \n");
        break;

    case 3: // OAC_URI_OBS_UPDATE_IP_ADDR_Msk OAC_URI_OBS_UPDATE_URI_TOKEN_Msk
        OTAPP_PRINTF(TAG, " @--> update IP ADDR and TOKEN uri \n");
        break;

    case 4: // OAC_URI_OBS_ADD_NEW_URI_Msk
        OTAPP_PRINTF(TAG, " @--> added new uri item \n");
        break;

    case 5: // OAC_URI_OBS_UPDATE_IP_ADDR_Msk OAC_URI_OBS_ADD_NEW_URI_Msk
        OTAPP_PRINTF(TAG, " @--> update IP ADDR and added new uri item \n");
        break;
    case OAC_URI_OBS_ADDED_NEW_DEVICE: 
        OTAPP_PRINTF(TAG, " @--> added new device to subscribe list \n");
        break;
    case OAC_URI_OBS_NO_NEED_UPDATE: 
        OTAPP_PRINTF(TAG, " @--> no need update subscriber data \n");
        break;

    default:
        break;
    }
}

void ad_light_uri_init(ot_app_devDrv_t *devDrv)
{
    if(devDrv == NULL) return;
    drv = devDrv;

    ad_light_ctr_init(drv);

}