/**
 * @file ot_app_srp_client.c
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

#include "ot_app_srp_client.h"
#include "ot_app.h"
#include "ot_app_dns.h"
#include "ot_app_coap.h"
#include "ot_app_deviceName.h"

#include "openthread/srp_client.h"

#define TAG "ot_app_srp_client "

static const char *otapp_serviceName = "_coap._udp";
static const char *otapp_browseDefaultServiceName = "_coap._udp.default.service.arpa.";

static otSrpClientService otapp_otSrpClientService;
static uint32_t otapp_srpLeaseValue;

static otError otapp_srpClientRefreshService(otInstance *instance);


void otapp_srpServiceLeaseInitCount(otInstance *instance)
{
    otapp_srpLeaseValue = otSrpClientGetLeaseInterval(instance) + OTAPP_DNS_LEASE_TASK_DELAY; // because while created task, the task is immediately running without any delay 
}

uint32_t otapp_srpServiceLeaseGetCount(void)
{
    return otapp_srpLeaseValue;
}

void otapp_srpServiceLeaseCountDecrease(void)
{
    otapp_srpLeaseValue = otapp_srpLeaseValue - OTAPP_DNS_LEASE_TASK_DELAY;
}

uint8_t otapp_srpServiceLeaseCheckExpiry(uint32_t lease)
{
    return (lease <= OTAPP_DNS_LEASE_GUARD);
}

void otapp_srpServiceLeaseCheckTask(void *arg)
{
    (void)arg;    
  
    TickType_t previousWakeTime;
    otInstance *instance;

    previousWakeTime = xTaskGetTickCount();
    instance = otapp_getOpenThreadInstancePtr();

    otapp_srpServiceLeaseInitCount(instance);

    while (1)
    {
        otapp_srpServiceLeaseCountDecrease();
        OTAPP_PRINTF(TAG, "Current SRP lease interval: %lu seconds", otapp_srpServiceLeaseGetCount());

        if (otapp_srpServiceLeaseCheckExpiry(otapp_srpServiceLeaseGetCount()))
        {
           otapp_srpClientRefreshService(instance);
           otapp_srpServiceLeaseInitCount(instance);          
        }

        vTaskDelayUntil(&previousWakeTime,  pdMS_TO_TICKS(OTAPP_DNS_LEASE_TASK_DELAY * 1000)); // in ms
    }
}

void otapp_srpServiceLeaseCheckTaskInit(void)
{
    static uint8_t initFlag = 0;

    if(!initFlag)
    {
        BaseType_t result = xTaskCreate(otapp_srpServiceLeaseCheckTask, "SRP_Check_Service_Lease_Task", 4096, NULL, 5, NULL);
    
        if (result == pdPASS)
        {
            initFlag = 1;
            OTAPP_PRINTF(TAG, "SRP_Check_Service_Lease_Task created successfully \n");
        }
        else
        {
            OTAPP_PRINTF(TAG, "Failed to create SRP_Check_Service_Lease_Task \n");
        }
    }else
    {
        OTAPP_PRINTF(TAG, "SRP_Check_Lease_Task has been created  \n");
    }
}

static void otapp_srpClientSetHostName(otInstance *instance, const char *hostName)
{
    otError error;

    error = otSrpClientSetHostName(instance, hostName);
    if (error != OT_ERROR_NONE)
    {
        OTAPP_PRINTF(TAG, "Error: hostname SRP NOT set: %d\n", error);
        return;
    }
}

static void otapp_srpClientAddHostAddress(otInstance *instance)
{
    otError error;
       
    error = otSrpClientSetHostAddresses(instance, otapp_ip6AddressRefresh(), 1);
    if (error != OT_ERROR_NONE)
    {
        OTAPP_PRINTF(TAG, "Error: SRP set IPv6 host addresses: %d\n", error);
        return;
    }
}

void otapp_srpClientUpdateHostAddress(otInstance *instance)
{
    otapp_srpClientAddHostAddress(instance);
}

otError otapp_srpClientAddService(otInstance *instance, otSrpClientItemState mState)
{
    otError error;

    otapp_otSrpClientService.mName = otapp_serviceName;               
    otapp_otSrpClientService.mInstanceName = otapp_deviceNameFullGet();    
    otapp_otSrpClientService.mSubTypeLabels = NULL;              
    otapp_otSrpClientService.mTxtEntries = NULL;                 
    otapp_otSrpClientService.mPort = OTAPP_COAP_PORT;               
    otapp_otSrpClientService.mPriority = 0;                      
    otapp_otSrpClientService.mWeight = 0;                       
    otapp_otSrpClientService.mNumTxtEntries = 0;                
    otapp_otSrpClientService.mState = mState; 
    otapp_otSrpClientService.mData = 0;                         
    otapp_otSrpClientService.mNext = NULL;                     
    otapp_otSrpClientService.mLease = OTAPP_DNS_LEASE_TIME;                   
    otapp_otSrpClientService.mKeyLease = OTAPP_DNS_M_KEY_LEASE_TIME;                
    
    if(mState == OT_SRP_CLIENT_ITEM_STATE_TO_REFRESH)
    {
        error = otSrpClientClearService(instance, &otapp_otSrpClientService);
        if (error != OT_ERROR_NONE)
        {
            OTAPP_PRINTF(TAG, "Error: SRP service clear: %d\n", error);
            return error;
        }
    }

    error = otSrpClientAddService(instance, &otapp_otSrpClientService);
    if (error != OT_ERROR_NONE)
    {
        OTAPP_PRINTF(TAG, "Error: SRP service add: %d\n", error);
        return error;
    }

    return OT_ERROR_NONE; 
}

otError otapp_srpClientRefreshService(otInstance *instance)
{
    otError error;

    error = otapp_srpClientAddService(instance, OT_SRP_CLIENT_ITEM_STATE_TO_REFRESH);
    if (error != OT_ERROR_NONE)
    {
        OTAPP_PRINTF(TAG, "Error: SRP service refreshing: %d\n", error);
        return error;
    }

    OTAPP_PRINTF(TAG, "SRP SERVICES has been refreshed \n");

    return error;
}

void otapp_otSrpClientCallback(otError aError, const otSrpClientHostInfo *aHostInfo, const otSrpClientService *aServices, const otSrpClientService *aRemovedServices, void *aContext)
{
    if(aError == OT_ERROR_NONE)
    {
        if(aHostInfo->mState == OT_SRP_CLIENT_ITEM_STATE_REGISTERED)
        {
            otapp_srpServiceLeaseCheckTaskInit();   
            
            otapp_dnsClientBrowse(otapp_getOpenThreadInstancePtr(), otapp_browseDefaultServiceName);
           
        }       
    }
}

void otapp_srpClientAutoStartCallback(const otSockAddr *aServerSockAddr, void *aContext)
{
    if(NULL != aServerSockAddr)
    {
        OTAPP_PRINTF(TAG, "SRP SERVER detected on IP: ");
        otapp_ip6AddressPrint(&aServerSockAddr->mAddress);
    }else
    {
         OTAPP_PRINTF(TAG, "SRP SERVER lost\n");
    }
}

static void otapp_srpClientInit(otInstance *instance)
{
    otSrpClientStop(instance);
    otapp_srpClientSetHostName(instance, otapp_deviceNameFullGet());
    otapp_srpClientAddHostAddress(instance);
    otapp_srpClientAddService(instance, OT_SRP_CLIENT_ITEM_STATE_TO_ADD);
     
    if(otSrpClientIsAutoStartModeEnabled(instance))
    {
        OTAPP_PRINTF(TAG, "SRP client has already ran\n");
        return;
    }else
    {
       otSrpClientEnableAutoStartMode(instance, otapp_srpClientAutoStartCallback, NULL); 
    }
    
    OTAPP_PRINTF(TAG, "SRP client Auto start Enabled \n");
}

void otapp_srpInit()
{
    otapp_srpClientInit(otapp_getOpenThreadInstancePtr());
    otSrpClientSetCallback(otapp_getOpenThreadInstancePtr(), otapp_otSrpClientCallback, NULL); 
}
