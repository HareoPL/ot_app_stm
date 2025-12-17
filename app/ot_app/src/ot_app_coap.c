/**
 * @file ot_app_coap.c
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief 
 * @version 0.1
 * @date 21-07-2025
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

#include "ot_app_coap.h"
#include "ot_app_deviceName.h"
#include "ot_app_drv.h"
#include "ot_app_coap_uri.h"

#include "string.h"

 #ifdef UNIT_TEST
    #include "mock_ip6.h"
    #include "mock_ot_app.h"

 #else
    #include <openthread/ip6.h>
    #include "ot_app.h"
#endif

#define TAG "ot_app_coap "
static ot_app_devDrv_t *drv;
uint8_t otapp_coap_token[OAC_URI_OBS_TOKEN_LENGTH];

static otapp_coap_uri_t otapp_coap_uriDefault[] ={
    {OTAPP_URI_WELL_KNOWN_CORE, {".well-known/core", ad_temp_uri_well_knownCoreHandle, NULL, NULL},},
    {OTAPP_URI_PARING_SERVICES, {"paring_services", otapp_coap_uri_paringServicesHandle, NULL, NULL}},
    {OTAPP_URI_SUBSCRIBED_URIS, {"subscribed_uris", otapp_coap_uri_subscribedHandle, NULL, NULL}},
    {OTAPP_URI_TEST,            {"test", otapp_coap_uri_testHandle, NULL, NULL}},                  // for test
    {OTAPP_URI_TEST_LED,        {"test/led", otapp_coap_uri_ledControlHandle, NULL, NULL}},      // for test
};
#define OTAPP_COAP_URI_DEFAULT_SIZE (sizeof(otapp_coap_uriDefault) / sizeof(otapp_coap_uriDefault[0]))

typedef struct {
    otapp_coap_messageId_t msgID;
    char *message;
}otapp_coap_message_t;

static const otapp_coap_message_t otapp_coap_messages[] = {
    {OTAPP_MESSAGE_OK, "OK"},
    {OTAPP_MESSAGE_ERROR, "ERROR"},
    {OTAPP_MESSAGE_TEST, "Hello coap !!"},
};
#define OTAPP_COAP_MESSAGE_SIZE (sizeof(otapp_coap_messages) / sizeof(otapp_coap_messages[0]))

static otMessageInfo messageInfo;

const char *otapp_coap_getMessage(otapp_coap_messageId_t msgID)
{
    for (uint16_t i = 0; i < OTAPP_COAP_MESSAGE_SIZE; i++)
    {
        if(otapp_coap_messages[i].msgID == msgID)
        {
            return otapp_coap_messages[i].message;
        }
    }
    
    return NULL;
} 

const char *otapp_coap_getUriName(const otapp_coap_uri_t *uriTable, uint8_t tableSize, otapp_coap_uriIndex_t uriIndex)
{
    if(uriTable == NULL || tableSize == 0 || uriIndex == OTAPP_URI_NO_URI_INDEX || uriIndex == OTAPP_URI_END_OF_INDEX)
    {
        return NULL;
    }

    for (uint8_t i = 0; i < tableSize; i++)
    {
        if(uriTable[i].devType == uriIndex) // if(uriTable[i].uriId == uriIndex) trzeba przemyslec czy funkcja get uri name jest przydatna ??? todo
        {            
            return uriTable[i].resource.mUriPath;
        }
    }
    
    return NULL;
} 

const char *otapp_coap_getUriNameFromDefault(otapp_coap_uriIndex_t uriIndex)
{
    return otapp_coap_getUriName(otapp_coap_uriDefault, OTAPP_COAP_URI_DEFAULT_SIZE, uriIndex);
}

void otapp_coap_responseHandler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aResult)
{
    if (aResult == OT_ERROR_NONE && aMessage)
    {
        // Obsłuż odpowiedź (przeczytaj payload, kod odpowiedzi itd.) 
       
    }
}


void otapp_coap_printSenderIP(const otMessageInfo *aMessageInfo)
{
    if (aMessageInfo != NULL)
    {
        // read sender IPv6 address
        const otIp6Address *sender_addr = &aMessageInfo->mPeerAddr;
        uint16_t sender_port = aMessageInfo->mPeerPort;
             
        OTAPP_PRINTF(TAG, "Sender mPeer address: \n");
        otapp_ip6AddressPrint(sender_addr);  
        OTAPP_PRINTF(TAG, ", port: %u\n\n", sender_port);

        // sender_addr = &aMessageInfo->mSockAddr;
        // sender_port = aMessageInfo->mSockPort;
        // OTAPP_PRINTF(TAG, "Sender mSock address: ");
        // otapp_ip6AddressPrint(sender_addr);  
        // OTAPP_PRINTF(TAG, ", port: %u\n\n", sender_port);

    }
}

void otapp_coap_sendResponse(otMessage *requestMessage, const otMessageInfo *aMessageInfo, const uint8_t *responceContent, uint16_t responceLength)
{
    otError error = OT_ERROR_NONE;
    otMessage *responseMessage = NULL;
    otCoapCode responseCode = OT_COAP_CODE_EMPTY;
   
    otCoapCode requestCode = otCoapMessageGetCode(requestMessage);

    if (requestCode == OT_COAP_CODE_GET)
    {
        responseCode = OT_COAP_CODE_CONTENT;

        responseMessage = otCoapNewMessage(otapp_getOpenThreadInstancePtr(), NULL);
        if (responseMessage == NULL)
        {
            error = OT_ERROR_NO_BUFS;
            goto exit;
        }

        // Create ACK for GET query
        error = otCoapMessageInitResponse(responseMessage, requestMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, responseCode);
        if (error != OT_ERROR_NONE) { goto exit; }

        // // Add marker payload's and payload
        error = otCoapMessageSetPayloadMarker(responseMessage);
        if (error != OT_ERROR_NONE) { goto exit; }

        if (NULL == responceContent) { goto exit; }
        error = otMessageAppend(responseMessage, (const uint8_t *)responceContent, responceLength);
        if (error != OT_ERROR_NONE) { goto exit; }
 
    }
    else if(requestCode == OT_COAP_CODE_PUT)
    {
        responseCode = OT_COAP_CODE_CHANGED;

        responseMessage = otCoapNewMessage(otapp_getOpenThreadInstancePtr(), NULL);
        if (responseMessage == NULL)
        {
            error = OT_ERROR_NO_BUFS;
            goto exit;
        }

        // Create ACK for GET query
        error = otCoapMessageInitResponse(responseMessage, requestMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, responseCode);
        if (error != OT_ERROR_NONE)  { goto exit; }
    
    }
    else
    {
        OTAPP_PRINTF(TAG, "CoAP method not supported: %d\n", requestCode);
        error = OT_ERROR_NOT_IMPLEMENTED;
        return;
    }

    // send response with default parameters
    error = otCoapSendResponseWithParameters(otapp_getOpenThreadInstancePtr(), responseMessage, aMessageInfo, NULL);
    if (error != OT_ERROR_NONE)
    {
        goto exit;
    }else
    {
        OTAPP_PRINTF(TAG, "CoAP response sent.\n");  
    }

exit:
    if (error != OT_ERROR_NONE)
    {
        OTAPP_PRINTF(TAG, "CoAP error: %d (%s)\n", error, otThreadErrorToString(error));
        if (responseMessage != NULL)
        {
            otMessageFree(responseMessage);
        }
    }
}

void otapp_coap_client_send(const otIp6Address *peer_addr, 
                            const char *aUriPath, 
                            otCoapCode code, 
                            const void *payloadMsg, 
                            const uint16_t payloadMsgSize,
                            otCoapResponseHandler responseHandler, 
                            void *aContext, 
                            uint8_t *tokenOutIn,
                            uint8_t obsState) // obsState: 0 - register, 1 - unsubscribe, 2 - update request
{
    otError error;
    otMessage *message = NULL;

    messageInfo.mPeerAddr = *peer_addr;
    messageInfo.mPeerPort = OT_DEFAULT_COAP_PORT;
    messageInfo.mHopLimit = 0; // standard limit
    messageInfo.mIsHostInterface = false; // package comes out of openthread interface

    if(NULL == peer_addr || NULL == aUriPath)
    {
        return;
    }

    // create new message CoAP
    message = otCoapNewMessage(otapp_getOpenThreadInstancePtr(), NULL);
    if (message == NULL)
    {
        error = OT_ERROR_NO_BUFS;
        goto exit;
    }

    // message initialize as Confirmable GET
    otCoapMessageInit(message, OT_COAP_TYPE_CONFIRMABLE, code);

    // add observer token and option
    if(tokenOutIn != NULL)
    {
        if(obsState == 0)
        {
            otCoapMessageGenerateToken(message, OAC_URI_OBS_TOKEN_LENGTH);
            memcpy(tokenOutIn, otCoapMessageGetToken(message), OAC_URI_OBS_TOKEN_LENGTH);            
        }else
        {
            otCoapMessageSetToken(message, tokenOutIn, OAC_URI_OBS_TOKEN_LENGTH);
        }

        otCoapMessageAppendObserveOption(message, obsState);        
    }

    // add URI
    error = otCoapMessageAppendUriPathOptions(message, aUriPath);
    if(error != OT_ERROR_NONE) { goto exit; }

    if(code == OT_COAP_CODE_PUT)
    {
        if(NULL == payloadMsg) { goto exit; }

        error = otCoapMessageSetPayloadMarker(message);
        if (error != OT_ERROR_NONE) { goto exit; }

        error = otMessageAppend(message, payloadMsg, payloadMsgSize);
        if (error != OT_ERROR_NONE) { goto exit; }
    }
    
    // send request. otapp_coap_responseHandler 
    error = otCoapSendRequest(otapp_getOpenThreadInstancePtr(), message, &messageInfo, responseHandler, aContext); // 
    if (error != OT_ERROR_NONE) { goto exit; }

exit:
    if (error != OT_ERROR_NONE)
    {
        OTAPP_PRINTF(TAG, "CoAP send() error: %d (%s)\n", error, otThreadErrorToString(error));
        if (message != NULL)
        {
            otMessageFree(message);
        }
    }   
}

void otapp_coap_clientSendPutByte(const otIp6Address *peer_addr, const char *aUriPath, const uint8_t *payloadMsg, const uint16_t payloadMsgSize, otCoapResponseHandler responseHandler, void *aContext)
{
   otapp_coap_client_send(peer_addr, aUriPath, OT_COAP_CODE_PUT, (const uint8_t *)payloadMsg, payloadMsgSize, responseHandler, aContext, NULL, 0);
   OTAPP_PRINTF(TAG, "CoAP sentPutByte to %s\n", aUriPath);
}

void otapp_coap_clientSendPutChar(const otIp6Address *peer_addr, const char *aUriPath, const char *payloadMsg, otCoapResponseHandler responseHandler)
{
   otapp_coap_client_send(peer_addr, aUriPath, OT_COAP_CODE_PUT, (const char *)payloadMsg, strlen(payloadMsg), responseHandler, NULL, NULL, 0);
}

void otapp_coap_clientSendGet(const otIp6Address *peer_addr, const char *aUriPath, otCoapResponseHandler responseHandler, void *aContext, uint8_t *outToken)
{
   otapp_coap_client_send(peer_addr, aUriPath, OT_COAP_CODE_GET, NULL, 0, responseHandler, aContext, outToken, 0);
}

void otapp_coap_clientSendGetByte(const otIp6Address *peer_addr, const char *aUriPath, otCoapResponseHandler responseHandler, void *aContext)
{
   otapp_coap_client_send(peer_addr, aUriPath, OT_COAP_CODE_GET, NULL, 0, responseHandler, aContext, NULL, 0);
   OTAPP_PRINTF(TAG, "CoAP sentGetByte to %s\n", aUriPath);
}

// char *charTest = {"test"};

void otapp_coapSendtoTestGet()
{
    otapp_coap_clientSendGetByte(otapp_multicastAddressGet(), otapp_coap_getUriNameFromDefault(OTAPP_URI_TEST), otapp_coap_responseHandler, NULL);
    OTAPP_PRINTF(TAG, "CoAP sent get to uri: test\n");
}

// char *charLed = {"device/led"};
char *charLedPayload = {"LED_ON"};
void otapp_coapSendtoTestPut()
{
    otapp_coap_uri_t *uri = drv->uriGetList_clb();
    if(uri == NULL) return;
    // otapp_coap_clientSendPutChar(otapp_multicastAddressGet(), otapp_coap_getUriNameFromDefault(OTAPP_URI_TEST_LED), charLedPayload, otapp_coap_responseHandler);
    otapp_coap_clientSendPutChar(otapp_multicastAddressGet(), uri[0].resource.mUriPath, charLedPayload, otapp_coap_responseHandler);
    OTAPP_PRINTF(TAG, "CoAP sent put to uri: device/led \n");
}
void otapp_coapSendDeviceNamePut()
{
    otapp_coap_clientSendPutChar(otapp_multicastAddressGet(), otapp_coap_getUriNameFromDefault(OTAPP_URI_PARING_SERVICES), otapp_deviceNameFullGet(), otapp_coap_responseHandler);
    OTAPP_PRINTF(TAG, "CoAP sent multicast device name \n");
}

void otapp_coapSendGetUri_Well_known(const otIp6Address *ipAddr, otCoapResponseHandler responseHandler, void *aContext)
{
   otapp_coap_clientSendGetByte(ipAddr, otapp_coap_getUriNameFromDefault(OTAPP_URI_WELL_KNOWN_CORE), responseHandler, aContext);
   OTAPP_PRINTF(TAG, "CoAP sent WELL KNOWN URI \n");
}

void otapp_coapSendPutUri_subscribed_uris(const otIp6Address *ipAddr, const uint8_t *data, uint16_t dataSize)
{
    otapp_coap_clientSendPutByte(ipAddr, otapp_coap_getUriNameFromDefault(OTAPP_URI_SUBSCRIBED_URIS), data, dataSize, otapp_coap_responseHandler, NULL);
    OTAPP_PRINTF(TAG, "CoAP sent update subscribers \n");
}

void otapp_coapSendSubscribeRequest(const otIp6Address *ipAddr, const char *aUriPath, uint8_t *tokenOut)
{
    otapp_coap_client_send(ipAddr, aUriPath, OT_COAP_CODE_PUT, (char*)otapp_deviceNameFullGet(), strlen(otapp_deviceNameFullGet()), otapp_coap_responseHandler, NULL, tokenOut, 0);
    OTAPP_PRINTF(TAG, "CoAP sent SubscribeRequest \n");
}

void otapp_coapSendSubscribeRequestUpdate(const otIp6Address *ipAddr, const char *aUriPath, uint8_t *tokenIn)
{
    otapp_coap_client_send(ipAddr, aUriPath, OT_COAP_CODE_PUT, (char*)otapp_deviceNameFullGet(), strlen(otapp_deviceNameFullGet()), otapp_coap_responseHandler, NULL, tokenIn, 2);
    OTAPP_PRINTF(TAG, "CoAP sent SubscribeRequest update \n");
}

int8_t otapp_coapReadPayload(otMessage *aMessage, uint8_t *bufferOut, uint16_t bufferSize, uint16_t *readBytesOut)
{
    uint16_t len = 0;
    uint16_t readBytes = 0;

    if(aMessage == NULL || bufferOut == NULL)
    {
        return OTAPP_COAP_ERROR;
    }

    memset(bufferOut, 0, bufferSize);

    len = otMessageGetLength(aMessage) - otMessageGetOffset(aMessage);
    if(len >= bufferSize)
    {
        OTAPP_PRINTF(TAG, "ERROR: coapReadPayload too small buf \n");
        return OTAPP_COAP_ERROR;
    }

    if(len == 0)
    {
        return OTAPP_COAP_ERROR;
    }
    readBytes = otMessageRead(aMessage, otMessageGetOffset(aMessage), bufferOut, len);
    *readBytesOut = readBytes;

    return OTAPP_COAP_OK;
}

int8_t otapp_coap_processUriRequest(otMessage *aMessage, const otMessageInfo *aMessageInfo, oacu_uriIndex_t uriId, uint8_t *bufOut, uint16_t bufSize)
{
    int8_t result = 0;
    uint16_t payloadReadBytes = 0;
    oac_uri_observer_t *obsHandle;

    if(aMessage == NULL || aMessageInfo == NULL || bufOut == NULL || bufSize == 0 || bufSize > OAC_URI_OBS_BUFFER_SIZE)
    {
        return OTAPP_COAP_ERROR;
    } 

    // handle subscribe request
    obsHandle = oac_uri_obs_getSubListHandle();
    if(obsHandle == NULL) return OTAPP_COAP_ERROR;
    
    // clear buffer and read payload to buffer
    memset(bufOut, 0, bufSize);
    result = otapp_coapReadPayload(aMessage, bufOut, bufSize, &payloadReadBytes);

    if(result != OTAPP_COAP_ERROR)
    {
        // check if this request concern adding thhis device to subscribe list
        result = oac_uri_obs_subscribeFromUri(obsHandle, aMessage, aMessageInfo, uriId, (char*)bufOut);  
        if(result == OAC_URI_OBS_ERROR) return OTAPP_COAP_ERROR;

        if(result == OAC_URI_OBS_NOT_SUB_REQUEST) 
        {            
            // send response OK
            otapp_coap_sendResponse(aMessage, aMessageInfo, (uint8_t*)otapp_coap_getMessage(OTAPP_MESSAGE_OK), strlen(otapp_coap_getMessage(OTAPP_MESSAGE_OK)) );            
           
            // notify subscribers about event
            oac_uri_obs_notify(obsHandle, &aMessageInfo->mPeerAddr, uriId, bufOut, bufSize); 

        }else // if request concerned observer. result > 0 
        {
            // send response OK
            otapp_coap_sendResponse(aMessage, aMessageInfo, (uint8_t*)otapp_coap_getMessage(OTAPP_MESSAGE_OK), strlen(otapp_coap_getMessage(OTAPP_MESSAGE_OK)) );
            return result; // OTAPP_COAP_OK_OBSERVER_REQUEST;
        }
    }else
    {
        // send response ERROR
        otapp_coap_sendResponse(aMessage, aMessageInfo, (uint8_t*)otapp_coap_getMessage(OTAPP_MESSAGE_ERROR), strlen(otapp_coap_getMessage(OTAPP_MESSAGE_ERROR)) );
        return OTAPP_COAP_ERROR;
    }
   
    
    return OTAPP_COAP_OK;
}

int8_t otapp_coap_initCoapResource(otapp_coap_uri_t *uriTable, uint8_t tableSize)
{   
    if(uriTable == NULL || tableSize == 0)
    {
        return OTAPP_COAP_ERROR;
    }

    for (uint32_t i = 0; i < tableSize; i++)
    {
        otCoapAddResource(otapp_getOpenThreadInstancePtr(), &uriTable[i].resource);
    }

    return OTAPP_COAP_URI_OK;
}


int8_t otapp_coap_init(ot_app_devDrv_t *devDriver)
{
    otError error;
    
    if (devDriver == NULL)
    {
       return OTAPP_COAP_URI_ERROR;
    }
    drv = devDriver;
    error = otCoapStart(otapp_getOpenThreadInstancePtr(), OT_DEFAULT_COAP_PORT);
    if (error != OT_ERROR_NONE)
    {
       return OTAPP_COAP_URI_ERROR;
    }
    // init coap handler
    // otCoapSetDefaultHandler(otapp_getOpenThreadInstancePtr(), otapp_coap_requestHandler, NULL);
        error = otapp_coap_initCoapResource(otapp_coap_uriDefault, OTAPP_COAP_URI_DEFAULT_SIZE);
    if (error != OTAPP_COAP_URI_OK)
    {
       return OTAPP_COAP_URI_ERROR;
    }

    
    otapp_coap_initCoapResource(devDriver->uriGetList_clb(), devDriver->uriGetListSize);
    // error = otapp_coap_initCoapResource(devDriver->uriGetList_clb(), devDriver->uriGetListSize);
    // if (error != OTAPP_COAP_URI_OK)
    // {
    //    return OTAPP_COAP_URI_ERROR;
    // }

    return OTAPP_COAP_URI_OK;
}

