/**
 * @file ot_app_pair.c
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief 
 * @version 0.1
 * @date 19-08-2025
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

 #include "ot_app_pair.h"

 #include "ot_app_drv.h"
 
 #ifdef UNIT_TEST
    #include "mock_freertos_queue.h"
    #include "mock_freertos_task.h"
    #include "mock_ot_app_deviceName.h"
    #include "mock_ip6.h"
    #include "mock_ot_message.h"
 #else
    #include "ot_app_deviceName.h"
    #include "ot_app_port_rtos.h"
#endif
    
#define TAG "ot_app_pair "

static ot_app_devDrv_t *drv;

typedef struct otapp_pair_DeviceList_t{
    otapp_pair_Device_t list[OTAPP_PAIR_DEVICES_MAX];
    uint8_t takenPosition[OTAPP_PAIR_DEVICES_MAX];
}otapp_pair_DeviceList_t;

static otapp_pair_DeviceList_t otapp_pair_DeviceList;
static QueueHandle_t otapp_pair_queueHandle;
static otapp_pair_queueItem_t otapp_pair_queueIteam;

static otapp_pair_resUrisParseData_t resUrisParseData[OTAPP_PAIR_URI_MAX];
static otapp_pair_resUrisBuffer_t resUrisBuffer[OTAPP_PAIR_URI_MAX];
//////////////////
// observer
static otapp_pair_observerCallback_t otapp_pair_observerPairedDeviceCallback[OTAPP_PAIR_OBSERVER_PAIRE_DDEVICE_CALLBACK_SIZE] = {NULL};

// Matching
int8_t otapp_pair_observerPairedDeviceRegisterCallback(otapp_pair_observerCallback_t callback)
{
    if(callback == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }

    for (uint8_t i = 0; i < OTAPP_PAIR_OBSERVER_PAIRE_DDEVICE_CALLBACK_SIZE; i++)
    {
        if(otapp_pair_observerPairedDeviceCallback[i] == NULL)
        {
            otapp_pair_observerPairedDeviceCallback[i] = callback;
            return OTAPP_PAIR_OK;
        }
    }
    
    return OTAPP_PAIR_ERROR;
}

PRIVATE int8_t otapp_pair_observerPairedDeviceNotify(otapp_pair_Device_t *newDevice)
{
    if(newDevice == NULL )
    {
        return OTAPP_PAIR_ERROR;
    }

    for (uint8_t i = 0; i < OTAPP_PAIR_OBSERVER_PAIRE_DDEVICE_CALLBACK_SIZE; i++)
    {
        if(otapp_pair_observerPairedDeviceCallback[i] != NULL)
        {
            otapp_pair_observerPairedDeviceCallback[i](newDevice);
            return OTAPP_PAIR_OK;
        }
    }

   return OTAPP_PAIR_OK;
}

// end of observer
//////////////////

int8_t otapp_pair_addToQueue(otapp_pair_queueItem_t *queueItem) 
{
    if(queueItem == NULL || otapp_pair_queueHandle == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }

    if(xQueueSend(otapp_pair_queueHandle, (void *)queueItem, (TickType_t) 0) != pdTRUE)
    {
        return OTAPP_PAIR_ERROR;
    }
    return OTAPP_PAIR_OK;
}

PRIVATE int8_t otapp_pair_DeviceIsFreeSpace(otapp_pair_DeviceList_t *pairDeviceList)
{
    if(pairDeviceList == NULL)
    {
        return OTAPP_PAIR_ERROR;
    } 

     for (uint8_t i = 0; i < OTAPP_PAIR_DEVICES_MAX; i++)
     {
        if(pairDeviceList->takenPosition[i] == 0)
        {
           return i;
        }
     }
     return OTAPP_PAIR_ERROR; 
}

PRIVATE void otapp_pair_spaceTake(otapp_pair_DeviceList_t *pairDeviceList, uint8_t indexDevice)
{
    if(pairDeviceList == NULL || indexDevice >= OTAPP_PAIR_DEVICES_MAX)
    {
        return;
    } 

    pairDeviceList->takenPosition[indexDevice] = 1;
}

PRIVATE int8_t otapp_pair_spaceIsTaken(otapp_pair_DeviceList_t *pairDeviceList, uint8_t indexDevice)
{ 
    if(pairDeviceList == NULL || indexDevice >= OTAPP_PAIR_DEVICES_MAX)
    {
        return OTAPP_PAIR_ERROR;
    }

    return (pairDeviceList->takenPosition[indexDevice]);
}

PRIVATE int8_t otapp_pair_deviceNameIsSame(otapp_pair_DeviceList_t *pairDeviceList, const char *deviceNameFull, uint8_t indexDevice)
{
    if(pairDeviceList == NULL || deviceNameFull == NULL || indexDevice >= OTAPP_PAIR_DEVICES_MAX)
    {
        return OTAPP_PAIR_ERROR;
    }

    if(strcmp(deviceNameFull, pairDeviceList->list[indexDevice].devNameFull) == 0)
    {
        return OTAPP_PAIR_IS;
    }
    return OTAPP_PAIR_IS_NOT;
}

PRIVATE int8_t otapp_pair_DeviceIsExist(otapp_pair_DeviceList_t *pairDeviceList, const char *deviceNameFull)
{
    if(pairDeviceList == NULL || deviceNameFull == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }

    for (int8_t i = 0; i < OTAPP_PAIR_DEVICES_MAX; i++)
    {
        if(otapp_pair_spaceIsTaken(pairDeviceList, i))
        {
            if(otapp_pair_deviceNameIsSame(pairDeviceList, deviceNameFull, i) == OTAPP_PAIR_IS)
            {
                return i;
            }
        }
    }        

    return OTAPP_PAIR_NO_EXIST;
}

char *otapp_pair_DeviceNameGet(otapp_pair_DeviceList_t *pairDeviceList, uint8_t indexDevice)
{
    if(pairDeviceList == NULL || indexDevice >= OTAPP_PAIR_DEVICES_MAX)
    {
        return NULL;
    }
    
    if(otapp_pair_spaceIsTaken(pairDeviceList, indexDevice) == 0)
    {
        return NULL;
    }

    return pairDeviceList->list[indexDevice].devNameFull;
}

otapp_pair_DeviceList_t *otapp_pair_getHandle(void)
{
    return &otapp_pair_DeviceList;
}

int8_t otapp_pair_DeviceDelete(otapp_pair_DeviceList_t *pairDeviceList, const char *deviceNameFull)
{
    if(pairDeviceList == NULL || deviceNameFull == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }

    int8_t tableIndex = otapp_pair_DeviceIsExist(pairDeviceList, deviceNameFull);
    if(tableIndex >= 0)
    {
        memset(pairDeviceList->list[tableIndex].devNameFull, 0, OTAPP_PAIR_NAME_FULL_SIZE);
        memset(&pairDeviceList->list[tableIndex].ipAddr, 0, sizeof(otIp6Address));
        memset(&pairDeviceList->list[tableIndex].urisList, 0, (sizeof(otapp_pair_uris_t) * OTAPP_PAIR_URI_MAX));
        pairDeviceList->takenPosition[tableIndex] = 0;
        
        return tableIndex;
    }
    return OTAPP_PAIR_NO_EXIST;
}

int8_t otapp_pair_DeviceDeleteAll(otapp_pair_DeviceList_t *pairDeviceList)
{
    if(pairDeviceList == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }

    for (uint8_t i = 0; i < OTAPP_PAIR_DEVICES_MAX; i++)
    {
        memset(pairDeviceList->list[i].devNameFull, 0, OTAPP_PAIR_NAME_FULL_SIZE);
        memset(&pairDeviceList->list[i].ipAddr, 0, sizeof(otIp6Address));
        pairDeviceList->takenPosition[i] = 0;
        memset(&pairDeviceList->list[i].urisList, 0, (sizeof(otapp_pair_uris_t) * OTAPP_PAIR_URI_MAX));
    }

    return OTAPP_PAIR_OK;
}

int8_t otapp_pair_DeviceIndexGet(otapp_pair_DeviceList_t *pairDeviceList, const char *deviceNameFull)
{
    if(pairDeviceList == NULL || deviceNameFull == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }

    int8_t deviceIndex;
    
    deviceIndex = otapp_pair_DeviceIsExist(pairDeviceList, deviceNameFull);
    if(deviceIndex == OTAPP_PAIR_NO_EXIST)
    {
        return OTAPP_PAIR_NO_EXIST;
    }

    return deviceIndex;
}

otapp_pair_Device_t *otapp_pair_DeviceGet(otapp_pair_DeviceList_t *pairDeviceList, const char *deviceNameFull)
{
    if(pairDeviceList == NULL || deviceNameFull == NULL)
    {
        return NULL;
    }
    int8_t deviceIndex;
    deviceIndex = otapp_pair_DeviceIndexGet(pairDeviceList, deviceNameFull);

    return &pairDeviceList->list[deviceIndex];
}

int8_t otapp_pair_DeviceAdd(otapp_pair_DeviceList_t *pairDeviceList, const char *deviceNameFull, otIp6Address *ipAddr)
{
    if(pairDeviceList == NULL || deviceNameFull == NULL || ipAddr == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }

    int8_t tableIndex;
    uint16_t strLen = 0;
    int8_t result = 0;

    result = otapp_pair_DeviceIsExist(pairDeviceList, deviceNameFull);
    if(result == OTAPP_PAIR_ERROR) 
    {
        return OTAPP_PAIR_ERROR;
    }
    else if(result == OTAPP_PAIR_NO_EXIST)
    {
       tableIndex = otapp_pair_DeviceIsFreeSpace(pairDeviceList);
       
       if(tableIndex != OTAPP_PAIR_ERROR)
       { 
            strLen = strlen(deviceNameFull); 
            if(strLen >= OTAPP_DNS_SRV_LABEL_SIZE)
            {
                return OTAPP_PAIR_DEVICE_NAME_TO_LONG;
            }
            strncpy(pairDeviceList->list[tableIndex].devNameFull, deviceNameFull, strLen);
            memcpy(&pairDeviceList->list[tableIndex].ipAddr, ipAddr, sizeof(otIp6Address)); 

            otapp_pair_spaceTake(pairDeviceList, tableIndex);

            return tableIndex;    
       }
       else
       {
            return OTAPP_PAIR_DEVICE_NO_SPACE;
       }
    }
    else
    {
        tableIndex = otapp_pair_DeviceIndexGet(pairDeviceList, deviceNameFull); 
        if(otapp_pair_ipAddressUpdate(pairDeviceList, tableIndex, ipAddr) == OTAPP_PAIR_UPDATED)
        {
           return OTAPP_PAIR_UPDATED; 
        }
    }
    
    return OTAPP_PAIR_NO_NEED_UPDATE ;
}

otIp6Address *otapp_pair_ipAddressGet(otapp_pair_DeviceList_t *pairDeviceList, uint8_t indexDevice)
{
    if(pairDeviceList == NULL || indexDevice >= OTAPP_PAIR_DEVICES_MAX)
    {
        return NULL;
    }

    return &pairDeviceList->list[indexDevice].ipAddr;
}

int8_t otapp_pair_ipAddressIsSame(otapp_pair_DeviceList_t *pairDeviceList, uint8_t indexDevice, otIp6Address *ipAddr)
{
    if(pairDeviceList == NULL || indexDevice >= OTAPP_PAIR_DEVICES_MAX || ipAddr  == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }

    if(otapp_pair_spaceIsTaken(pairDeviceList, indexDevice) == 0)
    {
        return OTAPP_PAIR_NO_EXIST;
    }

    uint8_t *ipAddr_old = (uint8_t *)otapp_pair_ipAddressGet(pairDeviceList, indexDevice);
    uint8_t *ipAddr_new = (uint8_t *)ipAddr;
    
    for (uint8_t i = 0; i < sizeof(otIp6Address); i++)
    {
        if(*ipAddr_new != *ipAddr_old)
        {
            return OTAPP_PAIR_IS_NOT;
        }

        ipAddr_new ++;
        ipAddr_old ++;
    }
    
    return OTAPP_PAIR_IS;
}

int8_t otapp_pair_ipAddressUpdate(otapp_pair_DeviceList_t *pairDeviceList, uint8_t indexDevice, otIp6Address *ipAddrNew)
{
    if(pairDeviceList == NULL || indexDevice >= OTAPP_PAIR_DEVICES_MAX || ipAddrNew  == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }

    if(otapp_pair_spaceIsTaken(pairDeviceList, indexDevice) == 0)
    {
        return OTAPP_PAIR_NO_EXIST;
    }

    otIp6Address *ipAddr_saved = otapp_pair_ipAddressGet(pairDeviceList, indexDevice);

    int8_t isSame =  otapp_pair_ipAddressIsSame(pairDeviceList, indexDevice, ipAddrNew);

    if(isSame == OTAPP_PAIR_IS_NOT)
    {
        memcpy(ipAddr_saved, ipAddrNew, sizeof(otIp6Address));

        return OTAPP_PAIR_UPDATED;
    }

    return OTAPP_PAIR_NO_NEED_UPDATE;
}

void otapp_pair_devicePrintData(otapp_pair_DeviceList_t *pairDeviceList, uint8_t indexDevice)
{
    if(pairDeviceList == NULL || indexDevice >= OTAPP_PAIR_DEVICES_MAX)
    {
        return  ;
    }
    
    char *deviceName;
    otIp6Address *ipAddr;
    const char *uriName;
    otapp_coap_uriIndex_t uriIndex = OTAPP_PAIR_URI_INIT;

    deviceName = otapp_pair_DeviceNameGet(pairDeviceList, indexDevice);
    ipAddr = otapp_pair_ipAddressGet(pairDeviceList, indexDevice);

    OTAPP_PRINTF(TAG, "Paired device info: \n");
    OTAPP_PRINTF(TAG, "  name: %s \n", deviceName);
    OTAPP_PRINTF(TAG, "  IP: ");
    otapp_ip6AddressPrint(ipAddr);
    OTAPP_PRINTF(TAG, "  URI: \n");

    for (uint8_t i = 0; i < OTAPP_PAIR_URI_MAX; i++)
    {
       
        if(uriIndex == OTAPP_PAIR_NO_URI)
        {
            if(i == 0)
            {
                OTAPP_PRINTF(TAG, "    no URI \n");
            }
            return;
        }

        uriName = otapp_coap_getUriNameFromDefault(uriIndex);
        OTAPP_PRINTF(TAG, "        %s\n", uriName);
    }
}

PRIVATE int8_t otapp_pair_deviceIsAllowed(ot_app_devDrv_t *deviceDrv, otapp_deviceType_t incommingDeviceID)
{    
    if(deviceDrv == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }

    otapp_pair_rule_t *rules = deviceDrv->pairRuleGetList_clb();
    
    if(rules == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }

    for(int i = 0; OTAPP_PAIR_RULES_ALLOWED_SIZE; i++) 
    {   
        if(rules->allowed[i] == OTAPP_PAIR_END_OF_RULES)  break;
        
        if(rules->allowed[i] == OTAPP_PAIR_NO_RULES || rules->allowed[i] == incommingDeviceID) 
        {
            return OTAPP_PAIR_IS;
        }
    }
    return OTAPP_PAIR_IS_NOT;
}

PRIVATE int8_t otapp_pair_deviceIsMatchingFromQueue(otapp_pair_queueItem_t *queueIteam)
{
    if(queueIteam == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }

    int16_t incomingDevID;
    const char *thisDevNameFull;
    

    if(otapp_deviceNameIsMatching(queueIteam->deviceNameFull) == OTAPP_DEVICENAME_IS)
    {
        thisDevNameFull = otapp_deviceNameFullGet();
        if(thisDevNameFull == NULL)
        {
            return OTAPP_PAIR_ERROR;
        }

        incomingDevID = otapp_deviceNameGetDevId(queueIteam->deviceNameFull, strlen(queueIteam->deviceNameFull));
        if(incomingDevID == OTAPP_DEVICENAME_ERROR || incomingDevID == OTAPP_DEVICENAME_TOO_LONG){ return OTAPP_PAIR_ERROR; }

        if(otapp_pair_deviceIsAllowed(drv, incomingDevID) == OTAPP_PAIR_IS)
        {
            return OTAPP_PAIR_IS;
        }        
    }
   
    return OTAPP_PAIR_IS_NOT;
}

static inline int8_t otapp_pair_uriCheckArgs(const void *buf, uint16_t *outDataSize) 
{      
    return (buf == NULL || outDataSize == NULL) ? OTAPP_PAIR_ERROR : OTAPP_PAIR_OK;
}

static inline int8_t otapp_pair_uriCheckString(const char* uri)
{   
    if(strlen(uri) >= OTAPP_URI_MAX_NAME_LENGHT) // the string len should be decrease by one becouse it must be at the end \0
    {           
        return OTAPP_PAIR_ERROR;
    }
    return OTAPP_PAIR_OK;
}

otapp_pair_resUrisBuffer_t *otapp_pair_uriResourcesCreate(otapp_coap_uri_t *uri, uint8_t uriSize, int8_t *result, uint16_t *outBufSize)
{   
    uint8_t *uriBuff = (uint8_t*)resUrisBuffer;

    if(result == NULL) return NULL;
    if(otapp_pair_uriCheckArgs(uri, outBufSize) == OTAPP_PAIR_ERROR || uriSize == 0 || uriSize > OTAPP_PAIR_URI_MAX)
    {
        *result = OTAPP_PAIR_ERROR;
        return NULL;
    }
    
    memset(uriBuff, 0, sizeof(resUrisBuffer)); // clear buffer

    for (uint8_t i = 0; i < uriSize; i++)
    {
        if(otapp_pair_uriCheckString(uri[i].resource.mUriPath) == OTAPP_PAIR_ERROR)
        {
            *result = OTAPP_PAIR_ERROR;
            return NULL;
        }
        // serialize from uri to bytes buffer
        strcpy((char*)uriBuff, uri[i].resource.mUriPath);   // char uri name
        
        uriBuff += OTAPP_URI_MAX_NAME_LENGHT;
        *uriBuff = uri[i].devType;                           // devTypeUriFn from otapp_deviceType_t
        
        uriBuff +=  sizeof(otapp_deviceType_t);
        *uriBuff = 1;                                        // uint8_t
        
        uriBuff += sizeof(uint8_t);
    }
    
    *outBufSize = (OTAPP_PAIR_URI_RESOURCE_BUFFER_SIZE * uriSize);
    *result = OTAPP_PAIR_OK;
    return resUrisBuffer;
}

otapp_pair_resUrisParseData_t *otapp_pair_uriParseMessage(const uint8_t *inBuffer, uint16_t inBufferSize, int8_t *result, uint16_t *outParsedDataSize)
{
    const uint8_t *inBuffer_tmp = inBuffer;
    uint16_t numOfDataToParse = 0;

    if(result == NULL) return NULL;

    if(otapp_pair_uriCheckArgs(inBuffer, outParsedDataSize) == OTAPP_PAIR_ERROR || inBufferSize == 0 || inBufferSize > (OTAPP_PAIR_URI_RESOURCE_BUFFER_MAX_SIZE))
    {
        *result = OTAPP_PAIR_ERROR;
        return NULL;
    }
  
    memset(resUrisParseData, 0, sizeof(resUrisParseData)); // clear buffer

    numOfDataToParse = inBufferSize / OTAPP_PAIR_URI_RESOURCE_BUFFER_SIZE;
    if(numOfDataToParse > OTAPP_PAIR_URI_MAX)
    {
       *result = OTAPP_PAIR_ERROR;
        return NULL; 
    }

    for (uint8_t i = 0; i < numOfDataToParse; i++)
    {
        if(otapp_pair_uriCheckString((char*)inBuffer_tmp) == OTAPP_PAIR_ERROR)
        {
            *result = OTAPP_PAIR_ERROR;
            return NULL;
        }
        // deserialize from bytes buffer to struct
        strcpy(resUrisParseData[i].uri, (char*)inBuffer_tmp);

        inBuffer_tmp += OTAPP_URI_MAX_NAME_LENGHT; 
        resUrisParseData[i].devTypeUriFn = *inBuffer_tmp;

        inBuffer_tmp += sizeof(otapp_deviceType_t);
        resUrisParseData[i].obs = *inBuffer_tmp;

        inBuffer_tmp += sizeof(uint8_t);
    }      
    
    *outParsedDataSize = numOfDataToParse;
    *result =  OTAPP_PAIR_OK;
    return resUrisParseData;
}

PRIVATE int8_t otapp_pair_uriTokenIsValid(const oacu_token_t *token)
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < OAC_URI_OBS_TOKEN_LENGTH; i++)
    {
        if(token[i] == 0)
        {
            count++;
        }
    }

    if(count == OAC_URI_OBS_TOKEN_LENGTH)
    {
        return OTAPP_PAIR_IS_NOT;
    }

    return OTAPP_PAIR_IS;
}

int8_t otapp_pair_uriAdd(otapp_pair_uris_t *deviceUriListIndex, const otapp_pair_resUrisParseData_t *uriData, const oacu_token_t *token)
{
    uint8_t uriLen = 0;
    
    if(deviceUriListIndex == NULL || uriData == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }
    
    uriLen = strlen(uriData->uri);
    if(uriLen > OTAPP_URI_MAX_NAME_LENGHT || uriLen == 0)
    {
        return OTAPP_PAIR_ERROR;
    }

    if(uriData->devTypeUriFn == 0)
    {
        return OTAPP_PAIR_ERROR;
    }

    if(token != NULL) // token is only when before addUri() uri is subscribed
    {   
        if(otapp_pair_uriTokenIsValid(token) == OTAPP_PAIR_IS) // check token is not empty
        {
            memcpy(deviceUriListIndex->token, token, OAC_URI_OBS_TOKEN_LENGTH);
        }else
        {
            return OTAPP_PAIR_ERROR;
        }
    }    
    memset(deviceUriListIndex->uri, 0, OTAPP_URI_MAX_NAME_LENGHT);
    deviceUriListIndex->devTypeUriFn = uriData->devTypeUriFn;
    strcpy(deviceUriListIndex->uri, uriData->uri);
     
    return OTAPP_PAIR_OK;

}

int8_t otapp_pair_uriGetIdList(otapp_pair_Device_t *deviceHandle, otapp_deviceType_t uriDevType)
{
    if(deviceHandle == NULL) return OTAPP_PAIR_ERROR;

    for (uint8_t i = 0; i < OTAPP_PAIR_URI_MAX; i++)
    {
       if(deviceHandle->urisList[i].devTypeUriFn == uriDevType)
       {
            return i;
       }
    }
    
    return OTAPP_PAIR_NO_EXIST;
}

PRIVATE int8_t otapp_pair_tokenIsSame(otapp_pair_DeviceList_t *pairDeviceList, int8_t devListId, int8_t uriListId, const oacu_token_t *tokenToCheck)
{
    if(tokenToCheck == NULL || pairDeviceList == NULL || devListId >= OTAPP_PAIR_DEVICES_MAX || uriListId >= OTAPP_PAIR_URI_MAX)
    {
        return OTAPP_PAIR_ERROR;
    }

    for (uint8_t i = 0; i < OAC_URI_OBS_TOKEN_LENGTH; i++)
    {
       if(pairDeviceList->list[devListId].urisList[uriListId].token[i] != tokenToCheck[i])
       {
            return OTAPP_PAIR_IS_NOT;
       }
    }
        
    return OTAPP_PAIR_IS;
}

// return uriLIstId or error
otapp_pair_uris_t *otapp_pair_tokenGetUriIteams(otapp_pair_DeviceList_t *pairDeviceList, const oacu_token_t *token)
{
    if(pairDeviceList == NULL || token == NULL) return NULL; 

    if(otapp_pair_uriTokenIsValid(token) == OTAPP_PAIR_IS)          // check if token is not empty
    {
        for (uint8_t i = 0; i < OTAPP_PAIR_DEVICES_MAX; i++)        // check if device list is taken
        {
            if(otapp_pair_spaceIsTaken(pairDeviceList, i))
            {
                for (uint8_t j = 0; j < OTAPP_PAIR_URI_MAX; j++)    // check if token form uriLIst is same
                {
                   if(otapp_pair_tokenIsSame(pairDeviceList, i, j, token) == OTAPP_PAIR_IS)
                   {
                       return &pairDeviceList->list[i].urisList[j]; // return uriList ptr
                   }
                }
                
            }
        }
    }

    return NULL;
}

int8_t otapp_pair_uriStateSet(otapp_pair_DeviceList_t *pairDeviceList, const oacu_token_t *token, const uint32_t *uriState)
{   
    otapp_pair_uris_t *uriIteams;

    if(pairDeviceList == NULL || token == NULL || uriState == NULL) return OTAPP_PAIR_ERROR; 
    
    uriIteams = otapp_pair_tokenGetUriIteams(pairDeviceList, token);
    if(uriIteams == NULL) return OTAPP_PAIR_ERROR;

    uriIteams->uriState = *uriState;

    return OTAPP_PAIR_OK;
}

// return OTAPP_PAIR_ERROR or count of updated devices
int8_t otapp_pair_subSendUpdateIP(otapp_pair_DeviceList_t *pairDeviceList)
{
    int8_t result = 0;
    int8_t countUpdatedDev = 0 ;

    otIp6Address *ipAddr = NULL;
    oacu_token_t *token = NULL;
    char *uri = NULL;

    if(pairDeviceList == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }

    for (uint8_t i = 0; i < OTAPP_PAIR_DEVICES_MAX; i++) // check saved devices in the deviceList
    {
        result = otapp_pair_spaceIsTaken(pairDeviceList, i);
        if(result)
        {
            for (uint8_t j = 0; j < OTAPP_PAIR_URI_MAX; j++) // check uri token is saved
            {
                if(otapp_pair_uriTokenIsValid(pairDeviceList->list[i].urisList[j].token) == OTAPP_PAIR_IS)
                {
                    ipAddr = &pairDeviceList->list[i].ipAddr;
                    uri     = pairDeviceList->list[i].urisList[j].uri;
                    token   = pairDeviceList->list[i].urisList[j].token;

                    oac_uri_obs_sendSubscribeRequestUpdate(ipAddr, uri, token);
                    countUpdatedDev++;
                }            
            }
            
        }
        
    }

    return countUpdatedDev;
}

void otapp_pair_responseHandlerUriWellKnown(void *pairedDevice, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aResult)
{
    UNUSED(aMessageInfo);
    UNUSED(aResult);

    if(pairedDevice == NULL) return;

    static uint8_t urisBuffer[OTAPP_PAIR_URI_RESOURCE_BUFFER_MAX_SIZE];
    static oacu_token_t token[OAC_URI_OBS_TOKEN_LENGTH];
    int8_t result = 0;
    uint16_t msgLen = 0;
    uint16_t readBytes = 0;
    otapp_pair_resUrisParseData_t *parsedData = NULL;
    uint16_t parsedDataSize = 0; // number of uri structures to add to the list

    otapp_pair_Device_t *device = (otapp_pair_Device_t*)pairedDevice;

    if (aMessage)
    {
        msgLen = otMessageGetLength(aMessage) - otMessageGetOffset(aMessage);
        readBytes = otMessageRead(aMessage, otMessageGetOffset(aMessage), urisBuffer, msgLen);
        if(readBytes != msgLen) return;

        parsedData = otapp_pair_uriParseMessage(urisBuffer, readBytes, &result, &parsedDataSize);
        if(parsedData == NULL || result == OTAPP_PAIR_ERROR) return;

        for (uint8_t i = 0; i < parsedDataSize; i++)
        {            
            if(parsedData[i].obs)
            {                
                oac_uri_obs_sendSubscribeRequest(&device->ipAddr, parsedData[i].uri, token);
                // todo w odpowiedzi powinienem otrzymac aktualny stan uri ta odpowiedz powinana pojawic sie w 
                // void otapp_coap_responseHandler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aResult)
                // trzeba zmodyfikowac fn sendsubReq aby przyjmowala callbacka response (powyzszego).
                otapp_pair_uriAdd(&device->urisList[i], &parsedData[i], token);
            }else
            {
                otapp_pair_uriAdd(&device->urisList[i], &parsedData[i], NULL);
            }
        }
        otapp_pair_observerPairedDeviceNotify(device); 
    }
}

void otapp_pair_task(void *params) 
{
    UNUSED(params);

    int8_t result, devId;
    otapp_pair_DeviceList_t *deviceListHandle;
    otapp_pair_Device_t *newDevice;
    otIp6Address *ipAddr;

    while (1)
    {
        if (xQueueReceive(otapp_pair_queueHandle, &otapp_pair_queueIteam, portMAX_DELAY) == pdTRUE) 
        {
            if (otapp_pair_queueIteam.type == OTAPP_PAIR_CHECK_AND_ADD_TO_DEV_LIST)
            {
                OTAPP_PRINTF(TAG, "Pairing new device: %s \n", otapp_pair_queueIteam.deviceNameFull);

                if(otapp_pair_deviceIsMatchingFromQueue(&otapp_pair_queueIteam) == OTAPP_PAIR_IS)
                {
                    deviceListHandle = otapp_pair_getHandle();
                    
                    result = otapp_pair_DeviceAdd(deviceListHandle, otapp_pair_queueIteam.deviceNameFull, &otapp_pair_queueIteam.ipAddress);
                    /*
                    sprawdzic czy po zresetowaniu urzadzenia typu light (ktory przyjmuje subskrybentow) button wysle ponownie subskrybcje.
                    wydaje mi se ze nie poniewaz funkcja otapp_coapSendGetUri_Well_known jest wywolywana tylko dla nowych urzadzen 
                    druga opcja to wysylanie pingow ? jesli nie odpowie x razy to usunac z listy pair ?
                    */
                    switch (result)
                    {
                    case OTAPP_PAIR_ERROR:                    
                    case OTAPP_PAIR_DEVICE_NAME_TO_LONG:                    
                    case OTAPP_PAIR_DEVICE_NO_SPACE:                    
                        OTAPP_PRINTF(TAG, "has NOT been paired. Error: %d \n", result);
                        break;

                    case OTAPP_PAIR_UPDATED:                    
                        otapp_ip6AddressPrint(&otapp_pair_queueIteam.ipAddress);

                        devId = otapp_pair_DeviceIndexGet(deviceListHandle, otapp_pair_queueIteam.deviceNameFull);
                        newDevice = &deviceListHandle->list[devId]; 

                        // tutaj powinno sie ponownie wyslac subskrybcje/odkrywanie zasowbow czyli to co po nizej 
                        // ipAddr = otapp_pair_ipAddressGet(deviceListHandle, devId);
                        // otapp_coapSendGetUri_Well_known(ipAddr, otapp_pair_responseHandlerUriWellKnown, (otapp_pair_Device_t*)newDevice); // .well-known/core

                        otapp_pair_observerPairedDeviceNotify(newDevice);

                        OTAPP_PRINTF(TAG, "has been updated index: %d (ip Addr): \n", result);
                        break;

                    case OTAPP_PAIR_NO_NEED_UPDATE:
                        otapp_pair_subSendUpdateIP(otapp_pair_getHandle());

                        devId = otapp_pair_DeviceIndexGet(deviceListHandle, otapp_pair_queueIteam.deviceNameFull);
                        newDevice = &deviceListHandle->list[devId]; 

                        // tutaj powinno sie ponownie wyslac subskrybcje/odkrywanie zasowbow czyli to co po nizej 
                        // ipAddr = otapp_pair_ipAddressGet(deviceListHandle, devId);
                        // otapp_coapSendGetUri_Well_known(ipAddr, otapp_pair_responseHandlerUriWellKnown, (otapp_pair_Device_t*)newDevice); // .well-known/core

                        otapp_pair_observerPairedDeviceNotify(newDevice);
                        
                        OTAPP_PRINTF(TAG, "no need IP update \n");
                        break;                   
                   
                    default:
                        if(result >= 0)
                        {                            
                            newDevice = &deviceListHandle->list[result];                            
                            ipAddr = otapp_pair_ipAddressGet(deviceListHandle, result);

                            otapp_coapSendGetUri_Well_known(ipAddr, otapp_pair_responseHandlerUriWellKnown, (otapp_pair_Device_t*)newDevice); // .well-known/core
                         
                            OTAPP_PRINTF(TAG, "has been success paired on index %d \n", result);
                        }else
                        {
                            OTAPP_PRINTF(TAG, " Error: %d \n", result);
                        }                        

                        break;
                    }
                }else
                {
                    OTAPP_PRINTF(TAG, "= current device, or NOT allowed \n");
                }
            }

            UTILS_RTOS_CHECK_FREE_STACK();
        }

        BREAK_U_TEST;
    }
}

PRIVATE int8_t otapp_pair_initQueue(void)
{
    otapp_pair_queueHandle = xQueueCreate(OTAPP_PAIR_QUEUE_LENGTH, sizeof(otapp_pair_queueItem_t));
    if(otapp_pair_queueHandle == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }
    return OTAPP_PAIR_OK;
}

PRIVATE int8_t otapp_pair_initTask(void)
{    
    if(xTaskCreate(otapp_pair_task, "otapp pair task", OTAPP_PAIR_TASK_STACK_DEPTH, xTaskGetCurrentTaskHandle(), OTAPP_PAIR_TASK_PRIORITY, NULL) != pdPASS)
    {
        return OTAPP_PAIR_ERROR;
    }
     
    return OTAPP_PAIR_OK;
}

int8_t otapp_pair_init(ot_app_devDrv_t *devDriver)
{
    if(devDriver == NULL)
    {
        return OTAPP_PAIR_ERROR;
    }

    int8_t result;
    
    drv = devDriver;

    otapp_pair_observerPairedDeviceRegisterCallback(drv->obs_pairedDevice_clb);

    result = otapp_pair_initQueue();
    if(result != OTAPP_PAIR_OK)
    {
        return OTAPP_PAIR_ERROR;
    }

    result =otapp_pair_initTask();
    if(result != OTAPP_PAIR_OK)
    {
        return OTAPP_PAIR_ERROR;
    }
    
    UNUSED(TAG);
    return OTAPP_PAIR_OK;
}