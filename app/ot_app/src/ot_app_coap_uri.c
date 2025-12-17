/**
 * @file ot_app_coap_uri.c
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief 
 * @version 0.1
 * @date 24-07-2025
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

// #include "main.h"
#include "ot_app_coap_uri.h"
#include "string.h"
#include "ot_app.h"
#include "ot_app_deviceName.h"
#include "ot_app_pair.h"
#include "ot_app_drv.h"

#include <openthread/coap.h>
#include <openthread/instance.h>
#include <openthread/message.h>
#include <string.h>

#define TAG "ot_app_coap_uri "

#define OTAPP_COA_URI_BUFFER 264
void otapp_coap_uri_testHandle(void *aContext, otMessage *request, const otMessageInfo *aMessageInfo)
{
    otapp_coap_printSenderIP(aMessageInfo);

    if (request)
    {

        otapp_coap_sendResponse(request, aMessageInfo, (uint8_t*) otapp_coap_getMessage(OTAPP_MESSAGE_TEST), strlen(otapp_coap_getMessage(OTAPP_MESSAGE_TEST)));
    }
}

static char charBuffer[1024];

void otapp_coap_uri_ledControlHandle(void *aContext, otMessage *request, const otMessageInfo *aMessageInfo)
{
    uint16_t readBytes = 0;
    otapp_coap_printSenderIP(aMessageInfo);

    if (request)
    {
        if(otapp_coapReadPayload(request, (uint8_t*)charBuffer, 1024, &readBytes) == OTAPP_COAP_ERROR) return; 
        
        otapp_coap_sendResponse(request, aMessageInfo, NULL, 0);        
        OTAPP_PRINTF(TAG, "Sender data: %s bytes: %d \n", charBuffer, readBytes);
    }
}

void otapp_coap_uri_paringServicesHandle(void *aContext, otMessage *request, const otMessageInfo *aMessageInfo)
{    
    otapp_pair_queueItem_t queueItem; 
    uint16_t readBytes = 0;

    if (request)
    {
        OTAPP_PRINTF(TAG, "from uri: paring_services \n");
        if(otapp_coapReadPayload(request, (uint8_t*)queueItem.deviceNameFull, OTAPP_PAIR_NAME_FULL_SIZE, &readBytes) == OTAPP_COAP_ERROR) return; 
        
        otapp_coap_sendResponse(request, aMessageInfo, NULL, 0);

        queueItem.type = OTAPP_PAIR_CHECK_AND_ADD_TO_DEV_LIST;
        memcpy(&queueItem.ipAddress, &aMessageInfo->mPeerAddr, sizeof(otIp6Address));

        OTAPP_PRINTF(TAG, "Sender data: %s \n", queueItem.deviceNameFull );
        otapp_coap_printSenderIP(aMessageInfo);

        OTAPP_PRINTF(TAG, "URI: Add item to queue\n");
        otapp_pair_addToQueue(&queueItem);    
    }
}
void ad_temp_uri_well_knownCoreHandle(void *aContext, otMessage *request, const otMessageInfo *aMessageInfo)
{
    ot_app_devDrv_t *devDrv_ = otapp_getDevDrvInstance();
    ot_app_size_t uriSize = 0;
    otapp_coap_uri_t *urisList = NULL;
    otapp_pair_resUrisBuffer_t *uriBuf = NULL;
    uint16_t uriBufSize = 0;

    int8_t result = 0;

    if (request)
    {
        if(devDrv_ == NULL) return;

        uriSize = devDrv_->uriGetListSize;
        urisList = devDrv_->uriGetList_clb();
        if(urisList == NULL) return;

        uriBuf = otapp_pair_uriResourcesCreate(urisList, uriSize, &result, &uriBufSize);
        if(result == OTAPP_PAIR_ERROR || uriBuf == NULL) return;

        otapp_coap_sendResponse(request, aMessageInfo, (uint8_t*)uriBuf, uriBufSize);
    }
}

void otapp_coap_uri_subscribedHandle(void *aContext, otMessage *request, const otMessageInfo *aMessageInfo)
{
    ot_app_devDrv_t *drv;
    oac_uri_dataPacket_t *dataPacket;
    uint16_t readBytes = 0;
   
    uint8_t buffer[OTAPP_COA_URI_BUFFER];
    int8_t result;

    if (request)
    {
        if(otapp_coapReadPayload(request, buffer, OTAPP_COA_URI_BUFFER, &readBytes) != OTAPP_COAP_OK) return;         
        
        otapp_coap_sendResponse(request, aMessageInfo, NULL, 0);

        drv = otapp_getDevDrvInstance();

        dataPacket = oac_uri_obs_getdataPacketHandle();
        result = oac_uri_obs_parseMessageFromNotify(buffer, dataPacket); 
        if(result == OAC_URI_OBS_ERROR)
        {
            OTAPP_PRINTF(TAG, "ERROR: otapp_coap_uri_subscribedHandle\n");
            return;
        }

        drv->obs_subscribedUri_clb(dataPacket); // inform app device about new subscribed event.         
    }
}
