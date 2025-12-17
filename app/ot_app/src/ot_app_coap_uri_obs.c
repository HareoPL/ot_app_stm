/**
 * @file ot_app_coap_uri_obs.c
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief observer pattern for uri 
 * @version 0.1
 * @date 16-09-2025
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

#include "ot_app_coap_uri_obs.h"
#include "string.h"

static oac_uri_observer_t oac_obsSubList[OAC_URI_OBS_SUBSCRIBERS_MAX_NUM];
static oac_uri_dataPacket_t oac_dataPacket;
static uint8_t oac_txRxBuffer[OAC_URI_OBS_TX_BUFFER_SIZE];

///////////////////////
// fn for devName
PRIVATE int8_t oac_uri_obs_spaceDevNameIsFree(oac_uri_observer_t *subListHandle)
{    
    if(subListHandle == NULL)
    {
        return OAC_URI_OBS_ERROR;
    }

    for (uint8_t i = 0; i < OAC_URI_OBS_SUBSCRIBERS_MAX_NUM; i++)
    {
        if(subListHandle[i].takenPosition_dev == 0)
        {
            return i; 
        }
    }

    return OAC_URI_OBS_LIST_FULL; 
}

PRIVATE int8_t oac_uri_obs_spaceDevNameIsTaken(oac_uri_observer_t *subListHandle, int8_t tabDevId)
{ 
    if(subListHandle == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM)
    {
        return OAC_URI_OBS_ERROR;
    }

    return (subListHandle[tabDevId].takenPosition_dev);
}

PRIVATE int8_t oac_uri_obs_spaceDevNameTake(oac_uri_observer_t *subListHandle, int8_t tabDevId)
{
    if(subListHandle == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM)
    {
        return OAC_URI_OBS_ERROR;
    }

    if(oac_uri_obs_spaceDevNameIsTaken(subListHandle, tabDevId))
    {
        return OAC_URI_OBS_ERROR;
    }else
    {
        subListHandle[tabDevId].takenPosition_dev = 1;
    }

    return OAC_URI_OBS_OK;
}

///////////////////////
// fn for uri
PRIVATE int8_t oac_uri_obs_spaceUriIsFree(oac_uri_observer_t *subListHandle, int8_t tabDevId)
{    
    if(subListHandle == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM)
    {
        return OAC_URI_OBS_ERROR;
    }

    for (uint8_t i = 0; i < OAC_URI_OBS_PAIRED_URI_MAX; i++)
    {
        if(subListHandle[tabDevId].uri[i].takenPosition_uri == 0)
        {
            return i; 
        }
    }

    return OAC_URI_OBS_LIST_FULL; 
}

PRIVATE int8_t oac_uri_obs_spaceUriIsTaken(oac_uri_observer_t *subListHandle, int8_t tabDevId, int8_t tabUriId)
{ 
    if(subListHandle == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM || tabUriId >= OAC_URI_OBS_PAIRED_URI_MAX)
    {
        return OAC_URI_OBS_ERROR;
    }

    return (subListHandle[tabDevId].uri[tabUriId].takenPosition_uri);
}

PRIVATE int8_t oac_uri_obs_spaceUriTake(oac_uri_observer_t *subListHandle, int8_t tabDevId, int8_t tabUriId)
{
    if(subListHandle == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM || tabUriId >= OAC_URI_OBS_PAIRED_URI_MAX)
    {
        return OAC_URI_OBS_ERROR;
    }

    if(oac_uri_obs_spaceUriIsTaken(subListHandle, tabDevId, tabUriId))
    {
        return OAC_URI_OBS_ERROR;
    }else
    {
        subListHandle[tabDevId].uri[tabUriId].takenPosition_uri = 1;
    }

    return OAC_URI_OBS_OK;
}

PRIVATE int8_t oac_uri_obs_uriIsExist(oac_uri_observer_t *subListHandle, int8_t tabDevId, oacu_uriIndex_t uriIndex)
{
    if(subListHandle == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM)
    {
        return OAC_URI_OBS_ERROR;
    }

    for (uint8_t i = 0; i < OAC_URI_OBS_PAIRED_URI_MAX; i++)
    {
        if(subListHandle[tabDevId].uri[i].uriIndex == uriIndex)
        {
            return i;
        }
    }
       
    return OAC_URI_OBS_IS_NOT;
}

///////////////////////
// fn for save
PRIVATE int8_t oac_uri_obs_saveDeviceNameFull(oac_uri_observer_t *subListHandle, int8_t tabDevId, const char* deviceNameFull)
{
    uint8_t devNameFull_len = 0;

    if(subListHandle == NULL || deviceNameFull == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM)
    {
        return OAC_URI_OBS_ERROR;
    }

    devNameFull_len = strlen(deviceNameFull);
    if(devNameFull_len >= OTAPP_DEVICENAME_FULL_SIZE) return OAC_URI_OBS_ERROR;
    
    strncpy(subListHandle[tabDevId].deviceNameFull, deviceNameFull, devNameFull_len);

    return OAC_URI_OBS_OK;
}

PRIVATE int8_t oac_uri_obs_saveIpAddr(oac_uri_observer_t *subListHandle, int8_t tabDevId, const otIp6Address *ipAddr)
{    
    if(subListHandle == NULL || ipAddr == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM)
    {
        return OAC_URI_OBS_ERROR;
    }

    memcpy(&subListHandle[tabDevId].ipAddr, ipAddr, OT_IP6_ADDRESS_SIZE);

    return OAC_URI_OBS_OK;
}

PRIVATE int8_t oac_uri_obs_saveUriIndex(oac_uri_observer_t *subListHandle, int8_t tabDevId, int8_t tabUriId, oacu_uriIndex_t uriIndex)
{
    if(subListHandle == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM || tabUriId >= OAC_URI_OBS_PAIRED_URI_MAX)
    {
        return OAC_URI_OBS_ERROR;
    }

    subListHandle[tabDevId].uri[tabUriId].uriIndex = uriIndex;

    return OAC_URI_OBS_OK;
}

PRIVATE int8_t oac_uri_obs_saveToken(oac_uri_observer_t *subListHandle, int8_t tabDevId, int8_t tabUriId, const oacu_token_t *token)
{
    if(subListHandle == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM || tabUriId >= OAC_URI_OBS_PAIRED_URI_MAX || token == NULL)
    {
        return OAC_URI_OBS_ERROR;
    }

    memcpy(subListHandle[tabDevId].uri[tabUriId].token, token, OAC_URI_OBS_TOKEN_LENGTH);        

    return OAC_URI_OBS_OK;
}

PRIVATE int8_t oac_uri_obs_addNewDevice(oac_uri_observer_t *subListHandle, const char* deviceNameFull, const otIp6Address *ipAddr)
{
    int8_t tabDevId_ = 0;
    int8_t result_ = 0;

    if(subListHandle == NULL || deviceNameFull == NULL || ipAddr == NULL)
    {
        return OAC_URI_OBS_ERROR;
    }

    tabDevId_ = oac_uri_obs_spaceDevNameIsFree(subListHandle);
    if(tabDevId_ == OAC_URI_OBS_ERROR) return OAC_URI_OBS_ERROR;

    if(tabDevId_ == OAC_URI_OBS_LIST_FULL)
    {
        return OAC_URI_OBS_LIST_FULL;
    }
    else
    {
        result_ = oac_uri_obs_saveDeviceNameFull(subListHandle, tabDevId_, deviceNameFull);
        if(result_ == OAC_URI_OBS_ERROR) return OAC_URI_OBS_ERROR;

        result_ = oac_uri_obs_saveIpAddr(subListHandle, tabDevId_, ipAddr);
        if(result_ == OAC_URI_OBS_ERROR) return OAC_URI_OBS_ERROR;

        oac_uri_obs_spaceDevNameTake(subListHandle, tabDevId_);
    }

    return tabDevId_;
}

///////////////////////
// fn for add
PRIVATE int8_t oac_uri_obs_addNewUri(oac_uri_observer_t *subListHandle, int8_t tabDevId, const oacu_token_t *token, oacu_uriIndex_t uriIndex)
{
    int8_t tabUriId_ = 0;
    int8_t result_ = 0;

    if(subListHandle == NULL || token == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM)
    {
        return OAC_URI_OBS_ERROR;
    }

    tabUriId_ = oac_uri_obs_spaceUriIsFree(subListHandle, tabDevId);
    if(tabUriId_ == OAC_URI_OBS_ERROR) return OAC_URI_OBS_ERROR;

    if(tabUriId_ == OAC_URI_OBS_LIST_FULL)
    {
        return OAC_URI_OBS_LIST_FULL;

    }else // tabUriId_ = uri table index
    {
        result_ = oac_uri_obs_saveUriIndex(subListHandle, tabDevId, tabUriId_, uriIndex);
        if(result_ == OAC_URI_OBS_ERROR) return OAC_URI_OBS_ERROR;

        result_ = oac_uri_obs_saveToken(subListHandle, tabDevId, tabUriId_, token);
        if(result_ == OAC_URI_OBS_ERROR) return OAC_URI_OBS_ERROR;

        oac_uri_obs_spaceUriTake(subListHandle, tabDevId, tabUriId_);        
    }

    return tabUriId_;
}

///////////////////////
// fn for token
PRIVATE int8_t oac_uri_obs_tokenIsSame(oac_uri_observer_t *subListHandle, int8_t tabDevId, int8_t tabUriId, const oacu_token_t *tokenToCheck)
{
    if(tokenToCheck == NULL || subListHandle == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM || tabUriId >= OAC_URI_OBS_PAIRED_URI_MAX)
    {
        return OAC_URI_OBS_ERROR;
    }

    for (uint8_t i = 0; i < OAC_URI_OBS_TOKEN_LENGTH; i++)
    {
       if(subListHandle[tabDevId].uri[tabUriId].token[i] != tokenToCheck[i])
       {
            return OAC_URI_OBS_IS_NOT;
       }
    }
        
    return OAC_URI_OBS_IS;
}

PRIVATE int8_t oac_uri_obs_tokenIsExist(oac_uri_observer_t *subListHandle, int8_t tabDevId, const oacu_token_t *token)
{
    if(token == NULL || subListHandle == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM)
    {
        return OAC_URI_OBS_ERROR;
    }

    for (int8_t i = 0; i < OAC_URI_OBS_SUBSCRIBERS_MAX_NUM; i++)
    {
        if(oac_uri_obs_spaceUriIsTaken(subListHandle, tabDevId, i))
        {
            if(oac_uri_obs_tokenIsSame(subListHandle,tabDevId ,i, token) == OAC_URI_OBS_IS)
            {
                return i;
            }
        }
    }        

    return OAC_URI_OBS_IS_NOT;
}

oac_uri_observer_t *oac_uri_obs_getSubListHandle()
{
    return oac_obsSubList;
}

oac_uri_dataPacket_t *oac_uri_obs_getdataPacketHandle()
{
    return &oac_dataPacket;
}

PRIVATE int8_t oac_uri_obs_checkTableIsInit(const uint8_t *tabPtr, uint16_t tabSize)
{
    uint16_t count = 0;

    if(tabPtr == NULL)
    {
        return OAC_URI_OBS_IS_NOT;
    }

    for (uint16_t i = 0; i < tabSize; i++) 
    {
        if (tabPtr[i] == 0) 
        {
            count++;           
        }
    }

    if(count == tabSize)
    {
        return OAC_URI_OBS_IS_NOT;
    }

    return OAC_URI_OBS_IS;
}

PRIVATE int8_t oac_uri_obs_ipAddrIsSame(oac_uri_observer_t *subListHandle, int8_t tabDevId, const otIp6Address *ipAddr)
{
    if(ipAddr == NULL || subListHandle == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM)
    {
        return OAC_URI_OBS_ERROR;
    }

    for (uint8_t i = 0; i < OT_IP6_ADDRESS_SIZE; i++)
    {
       if(subListHandle[tabDevId].ipAddr.mFields.m8[i] != ipAddr->mFields.m8[i])
       {
            return OAC_URI_OBS_IS_NOT;
       }
    }
    return OAC_URI_OBS_IS;
}

PRIVATE int8_t oac_uri_obs_devNameFullIsSame(oac_uri_observer_t *subListHandle, int8_t tabDevId, const char *deviceNameFull)
{
    if(subListHandle == NULL || deviceNameFull == NULL || tabDevId >= OAC_URI_OBS_SUBSCRIBERS_MAX_NUM)
    {
        return OAC_URI_OBS_ERROR;
    }

    if(strlen(deviceNameFull) >= OAC_URI_OBS_DEVICENAME_FULL_SIZE) return OAC_URI_OBS_ERROR;

    if(strcmp(subListHandle[tabDevId].deviceNameFull, deviceNameFull) == 0)
    {
        return OAC_URI_OBS_IS;
    }    

    return OAC_URI_OBS_IS_NOT;
}

PRIVATE int8_t oac_uri_obs_devNameFullIsExist(oac_uri_observer_t *subListHandle, const char *deviceNameFull)
{
    if(subListHandle == NULL || deviceNameFull == NULL)
    {
        return OAC_URI_OBS_ERROR;
    }

    if(strlen(deviceNameFull) >= OAC_URI_OBS_DEVICENAME_FULL_SIZE) return OAC_URI_OBS_ERROR;
    
    for (uint8_t i = 0; i < OAC_URI_OBS_SUBSCRIBERS_MAX_NUM; i++)
    {
        if(oac_uri_obs_spaceDevNameIsTaken(subListHandle, i))
        {
            if(oac_uri_obs_devNameFullIsSame(subListHandle, i, deviceNameFull) == OAC_URI_OBS_IS)
            {
                return i;
            }
        } 
    }
    
    return OAC_URI_OBS_IS_NOT;
}
PRIVATE int8_t oac_uri_obs_subscribeIsValidData(oac_uri_observer_t *subListHandle, const oacu_token_t *token, oacu_uriIndex_t uriIndex, const otIp6Address *ipAddr, const char* deviceNameFull)
{
    if(subListHandle == NULL || token == NULL || deviceNameFull == NULL)
    {
        return OAC_URI_OBS_ERROR; 
    }
    
    if(uriIndex == 0 )
    {
        return OAC_URI_OBS_ERROR;
    }

    if(oac_uri_obs_checkTableIsInit(ipAddr->mFields.m8, OT_IP6_ADDRESS_SIZE) == OAC_URI_OBS_IS_NOT)
    {
        return OAC_URI_OBS_ERROR;
    }

    if(oac_uri_obs_checkTableIsInit(token, OAC_URI_OBS_TOKEN_LENGTH) == OAC_URI_OBS_IS_NOT)
    {
        return OAC_URI_OBS_ERROR;
    }

    return OAC_URI_OBS_IS;
}

// updateState 
int8_t oac_uri_obs_subscribe(oac_uri_observer_t *subListHandle, const oacu_token_t *token, oacu_uriIndex_t uriIndex, const otIp6Address *ipAddr, const char* deviceNameFull)
{
    oacu_result_t result_ = 0;
    int8_t updateState = 0;
    int8_t tabDevId_ = 0;
    int8_t tabUriId_ = 0;

    if(oac_uri_obs_subscribeIsValidData(subListHandle, token, uriIndex, ipAddr, deviceNameFull) != OAC_URI_OBS_IS)
    {
        return OAC_URI_OBS_ERROR;
    }
    
    // check if devNameFull is exist
    tabDevId_ = oac_uri_obs_devNameFullIsExist(subListHandle, deviceNameFull);
    if(tabDevId_ == OAC_URI_OBS_ERROR) return OAC_URI_OBS_ERROR;
    
    if(tabDevId_ != OAC_URI_OBS_IS_NOT) // deviceNameFull is already existed, tabDevId_ = dev table index
    {
        // check if ipAddres is same. If not let's update it
        result_ = oac_uri_obs_ipAddrIsSame(subListHandle, tabDevId_, ipAddr);
        if(result_ == OAC_URI_OBS_ERROR) return OAC_URI_OBS_ERROR;

        if(result_ == OAC_URI_OBS_IS_NOT)
        {
            oac_uri_obs_saveIpAddr(subListHandle, tabDevId_, ipAddr); 

            updateState |= OAC_URI_OBS_UPDATE_IP_ADDR_Msk;           
        }

        // check if uriIndex is exist
        tabUriId_ = oac_uri_obs_uriIsExist(subListHandle, tabDevId_, uriIndex);
        if(tabUriId_ == OAC_URI_OBS_ERROR) return OAC_URI_OBS_ERROR;

        if(tabUriId_ == OAC_URI_OBS_IS_NOT) 
        {            
           // added new uriIndex and token
           result_ = oac_uri_obs_addNewUri(subListHandle, tabDevId_, token, uriIndex); 

           updateState |= OAC_URI_OBS_ADD_NEW_URI_Msk;

        }else // uriIndex is already existed. let's check if it needs to be updated. tabUriId_ = uri table index
        {
            // check if token is same
            result_ = oac_uri_obs_tokenIsSame(subListHandle, tabDevId_, tabUriId_, token);
            if (result_ == OAC_URI_OBS_ERROR) return OAC_URI_OBS_ERROR;

            if (result_ == OAC_URI_OBS_IS_NOT)
            {
                oac_uri_obs_saveToken(subListHandle, tabDevId_, tabUriId_, token); 

                updateState |= OAC_URI_OBS_UPDATE_URI_TOKEN_Msk;
            }
        }

        if(updateState == 0)
        {
            return OAC_URI_OBS_NO_NEED_UPDATE;
        }else
        {
            return updateState;
        }

    }else
    {
        // add deviceNameFull and ipAddr
        tabDevId_ = oac_uri_obs_addNewDevice(subListHandle, deviceNameFull, ipAddr);
        if(tabDevId_ == OAC_URI_OBS_ERROR) return OAC_URI_OBS_ERROR;

        if(tabDevId_ == OAC_URI_OBS_LIST_FULL) 
        {
            return OAC_URI_OBS_LIST_FULL;
        }else
        {
            // add uriIndex, token
            tabUriId_ = oac_uri_obs_addNewUri(subListHandle, tabDevId_, token, uriIndex);
            if(tabUriId_ == OAC_URI_OBS_ERROR) return OAC_URI_OBS_ERROR;
            if(tabUriId_ == OAC_URI_OBS_LIST_FULL) return OAC_URI_OBS_LIST_FULL;            
        }
    }

    return OAC_URI_OBS_ADDED_NEW_DEVICE;
}

int8_t oac_uri_obs_subscribeFromUri(oac_uri_observer_t *subListHandle, otMessage *aMessage, const otMessageInfo *aMessageInfo, oacu_uriIndex_t uriId, char* deviceNameFull)
{
    int8_t result = 0;
    uint64_t obsValue = 0;
    otCoapOptionIterator iterator;    
    const otCoapOption *coapOption;

    if(subListHandle == NULL || aMessage == NULL || aMessageInfo == NULL ||deviceNameFull == NULL)
    {
        return OAC_URI_OBS_ERROR;
    }

    otCoapOptionIteratorInit(&iterator, aMessage);
    coapOption = otCoapOptionIteratorGetFirstOptionMatching(&iterator, OT_COAP_OPTION_OBSERVE);
    
    if(coapOption == NULL) 
    {
        return OAC_URI_OBS_NOT_SUB_REQUEST;
    }else
    {
        if(strlen(deviceNameFull) >= OTAPP_DEVICENAME_FULL_SIZE)
        {
            return OAC_URI_OBS_ERROR;
        }
        
        otCoapOptionIteratorGetOptionValue(&iterator, &obsValue);

        if(obsValue != 1)
        {

            result = oac_uri_obs_subscribe(subListHandle, otCoapMessageGetToken(aMessage), uriId, &aMessageInfo->mPeerAddr, deviceNameFull);
        }else
        {
            // unSub. todo
        }
    }

    return result;
}


int8_t oac_uri_obs_unsubscribe(oac_uri_observer_t *subListHandle, char* deviceNameFull, const oacu_token_t *token)
{
    int8_t tabDevId_ = 0;
    int8_t tabUriId_ = 0;
    uint8_t takenUris = 0;

    if(subListHandle == NULL || deviceNameFull == NULL || token == NULL)
    {
        return OAC_URI_OBS_ERROR;
    }
    
    // chech if deviceNameFull is exist
    tabDevId_ = oac_uri_obs_devNameFullIsExist(subListHandle, deviceNameFull);
    if(tabDevId_ == OAC_URI_OBS_ERROR) return OAC_URI_OBS_ERROR;
    
    if(tabDevId_ == OAC_URI_OBS_IS_NOT) 
    {
        return OAC_URI_OBS_ERROR;        
    }else // deviceNameFull is already existed, tabDevId_ = dev table index
    {
        // check if uri token is exist
        tabUriId_ = oac_uri_obs_tokenIsExist(subListHandle, tabDevId_, token);
        if(tabUriId_ == OAC_URI_OBS_ERROR) return OAC_URI_OBS_ERROR;
        
        if(tabUriId_ != OAC_URI_OBS_IS_NOT)
        {
            memset(subListHandle[tabDevId_].uri[tabUriId_].token, 0, OAC_URI_OBS_TOKEN_LENGTH);
            subListHandle[tabDevId_].uri[tabUriId_].uriIndex = 0;
            subListHandle[tabDevId_].uri[tabUriId_].takenPosition_uri = 0;

            // check if there are not others saved uris - if not delete device from the subscribe list
            for (uint8_t i = 0; i < OAC_URI_OBS_PAIRED_URI_MAX; i++)
            {
                if(oac_uri_obs_spaceUriIsTaken(subListHandle, tabDevId_, i))
                {
                    takenUris++;
                }
            }

            if(takenUris == 0)
            {
                // delete device
                memset(subListHandle[tabDevId_].deviceNameFull, 0, OAC_URI_OBS_DEVICENAME_FULL_SIZE);
                memset(&subListHandle[tabDevId_].ipAddr, 0, OT_IP6_ADDRESS_SIZE);
                subListHandle[tabDevId_].takenPosition_dev = 0;
            }
            
            return OAC_URI_OBS_OK;     
        }
    }

    return OAC_URI_OBS_TOKEN_NOT_EXIST;
}

int8_t oac_uri_obs_notify(oac_uri_observer_t *subListHandle, const otIp6Address *excludedIpAddr, oacu_uriIndex_t uriIndex, const uint8_t *dataToNotify, uint16_t dataSize)
{

    uint16_t numOfnotifications = 0;
    if(subListHandle == NULL || dataToNotify == NULL || uriIndex == 0)
    {
        return OAC_URI_OBS_ERROR;
    }

    if(dataSize > OAC_URI_OBS_BUFFER_SIZE)
    {
        return OAC_URI_OBS_ERROR;
    }

    for(uint8_t i = 0; i < OAC_URI_OBS_SUBSCRIBERS_MAX_NUM; i++)
    {
       if(oac_uri_obs_spaceDevNameIsTaken(subListHandle, i))
       {
            for(uint8_t j = 0; j < OAC_URI_OBS_PAIRED_URI_MAX; j++)
            {
                if(oac_uri_obs_spaceUriIsTaken(subListHandle, i, j))
                {
                    if(subListHandle[i].uri[j].uriIndex == uriIndex)
                    {
                        if(excludedIpAddr != NULL) // there is nothing to exclude
                        {
                            // checking whether the current IP ADDR index is not same as te exclude one
                            if(oac_uri_obs_ipAddrIsSame(subListHandle, i, excludedIpAddr) == OAC_URI_OBS_IS)
                            {
                                continue;
                            }
                        }
                                                
                        // clear tx buffer
                        memset(oac_txRxBuffer, 0, sizeof(oac_txRxBuffer)); 

                        // copy token to tx buffer
                        memcpy(oac_txRxBuffer, subListHandle[i].uri[j].token, OAC_URI_OBS_TOKEN_LENGTH);
                        
                        // increase tx buffer ptr about token lenght
                        // copy dataToNotify to tx buffer 
                        memcpy(oac_txRxBuffer + OAC_URI_OBS_TOKEN_LENGTH, dataToNotify, dataSize);

                        // send data to subscriber
                        otapp_coapSendPutUri_subscribed_uris(&subListHandle[i].ipAddr, oac_txRxBuffer, sizeof(oac_txRxBuffer));
                        numOfnotifications++;
                        
                    }
                }
            }
            
       }
    }
        
    return numOfnotifications;
}

int8_t oac_uri_obs_parseMessageFromNotify(const uint8_t *inBuffer, oac_uri_dataPacket_t *out)
{
    if(inBuffer == NULL || out == NULL)
    {
        return OAC_URI_OBS_ERROR; 
    }

    const uint8_t *bufPtr = inBuffer;
    
    memset(out, 0, sizeof(oac_uri_dataPacket_t)); // clear out buffer

    memcpy(out->token, bufPtr, OAC_URI_OBS_TOKEN_LENGTH); // OAC_URI_OBS_TOKEN_LENGTH
    
    bufPtr += OAC_URI_OBS_TOKEN_LENGTH;
    memcpy(out->buffer, bufPtr, OAC_URI_OBS_BUFFER_SIZE); // OAC_URI_OBS_BUFFER_SIZE

    return OAC_URI_OBS_OK;
}

int8_t oac_uri_obs_sendSubscribeRequestUpdate(const otIp6Address *ipAddr, const char *aUriPath, uint8_t *tokenIn)
{
    otapp_coapSendSubscribeRequestUpdate(ipAddr, aUriPath, tokenIn);
    return OAC_URI_OBS_OK;
}
int8_t oac_uri_obs_sendSubscribeRequest(const otIp6Address *ipAddr, const char *aUriPath, uint8_t *tokenOut)
{
    otapp_coapSendSubscribeRequest(ipAddr, aUriPath, tokenOut);
    return OAC_URI_OBS_OK;
}

int8_t oac_uri_obs_deleteAll(oac_uri_observer_t *subListHandle)
{
    if(subListHandle == NULL)
    {
        return OAC_URI_OBS_ERROR;
    }

    for (uint8_t i = 0; i < OAC_URI_OBS_SUBSCRIBERS_MAX_NUM; i++)
    {
        memset(&subListHandle[i], 0, sizeof(subListHandle[0]));
    }
    
    return OAC_URI_OBS_OK;
}



#ifdef UNIT_TEST
    
    oac_uri_observer_t test_obs={
        .deviceNameFull = "device1_1_588c81fffe301ea4",
        .takenPosition_dev = 1,
        .ipAddr.mFields.m8 = {
                        0x20, 0x01, 0x0d, 0xb8, 0x85, 0xa3, 0x00, 0x00,
                        0x00, 0x00, 0x8a, 0x2e, 0x03, 0x70, 0x73, 0x34},
        .uri[0].takenPosition_uri = 1,
        .uri[0].token = {0xFA, 0x04, 0xB6, 0xD1},
        .uri[0].uriIndex = 1,

        .uri[1].takenPosition_uri = 1,
        .uri[1].token = {0xF1, 0x04, 0xB6, 0xD1},
        .uri[1].uriIndex = 2,

        .uri[2].takenPosition_uri = 1,
        .uri[2].token = {0xF2, 0x04, 0xB6, 0xD1},
        .uri[2].uriIndex = 3,

        .uri[3].takenPosition_uri = 1,
        .uri[3].token = {0xF3, 0x04, 0xB6, 0xD1},
        .uri[3].uriIndex = 4,

        .uri[4].takenPosition_uri = 1,
        .uri[4].token = {0xF4, 0x04, 0xB6, 0xD1},
        .uri[4].uriIndex = 5,
    };

    int8_t test_obs_fillListExampleData(oac_uri_observer_t *subListHandle)
    {
        for (uint16_t i = 0; i < OAC_URI_OBS_SUBSCRIBERS_MAX_NUM; i++)
        {
           memcpy(&subListHandle[i], &test_obs, sizeof(test_obs));
        }
        
        return OAC_URI_OBS_OK;
    }
#endif
