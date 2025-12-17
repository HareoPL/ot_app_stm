/**
 * @file ot_app_drv.c
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief 
 * @version 0.1
 * @date 17-09-2025
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

#include "ot_app_drv.h"
#include "ot_app_port_nvs.h"

static ot_app_devDrv_t ot_app_devDrv = {
    .obs_subscribedUri_clb = NULL,      
    .obs_pairedDevice_clb = NULL,      

    .pairRuleGetList_clb = NULL,        
    .uriGetList_clb = NULL,  
    
    .deviceName = NULL,
    .deviceType = NULL,             

    .task = NULL,

    .uriGetListSize = 0,

    .api.pair = {
        .getHandle = otapp_pair_getHandle,
        .uriStateSet = otapp_pair_uriStateSet,
        .uriGetIdList = otapp_pair_uriGetIdList,
    },

    .api.devName = {
        .devNameFullToEUI = otapp_deviceNameFullToEUI,
        .devNameEuiIsSame = otapp_deviceNameEuiIsSame,
    },

    .api.nvs = {
        .init = ot_app_nvs_init,
        .readString = ot_app_nvs_readString,
        .saveString = ot_app_nvs_saveString,
    },
    
    .api.coap = {
        .sendBytePut = otapp_coap_clientSendPutByte,
        .sendByteGet = otapp_coap_clientSendGetByte,
        .sendResponse = otapp_coap_sendResponse,
        .readPayload = otapp_coapReadPayload,
        .processUriRequest = otapp_coap_processUriRequest,
    },
};


void ot_app_drv_task(void)
{
    if(ot_app_devDrv.task != NULL) 
    ot_app_devDrv.task();
}

ot_app_devDrv_t *ot_app_drv_getInstance()
{
    return &ot_app_devDrv;
    
}