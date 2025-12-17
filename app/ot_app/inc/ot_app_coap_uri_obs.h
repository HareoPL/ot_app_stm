/**
 * @file ot_app_coap_uri_obs.h
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
#ifndef OT_APP_COAP_URI_OBS_H_
#define OT_APP_COAP_URI_OBS_H_

#include "hro_utils.h"

#ifdef UNIT_TEST
    #include "mock_ot_app_coap.h"
    #include "mock_ot_app_deviceName.h"
    #include "mock_ot_app.h"
#else
    #include "ot_app_coap.h"
    #include "ot_app_deviceName.h"
#endif

#define OAC_URI_OBS_TOKEN_LENGTH            4
#define OAC_URI_OBS_SUBSCRIBERS_MAX_NUM     20
#define OAC_URI_OBS_DEVICENAME_FULL_SIZE    OTAPP_DNS_SRV_LABEL_SIZE // 32 host name: "device1_1_588c81fffe301ea4"
#define OAC_URI_OBS_PAIRED_URI_MAX          OTAPP_PAIRED_URI_MAX 

#define OAC_URI_OBS_BUFFER_SIZE             256
#define OAC_URI_OBS_TX_BUFFER_SIZE         (OAC_URI_OBS_TOKEN_LENGTH + OAC_URI_OBS_BUFFER_SIZE)

#define OAC_URI_OBS_UPDATE_IP_ADDR_Msk         (0x1UL << 0U) // 1
#define OAC_URI_OBS_UPDATE_URI_TOKEN_Msk       (0x1UL << 1U) // 2
#define OAC_URI_OBS_ADD_NEW_URI_Msk            (0x1UL << 2U) // 4


typedef int8_t oacu_result_t;
typedef uint8_t oacu_uriIndex_t;
typedef uint8_t oacu_token_t;

typedef enum{
    OAC_URI_OBS_OK              = (-1),
    OAC_URI_OBS_ERROR           = (-2),
    OAC_URI_OBS_IS              = (-3),
    OAC_URI_OBS_IS_NOT          = (-4),
    OAC_URI_OBS_TOKEN_EXIST     = (-5),
    OAC_URI_OBS_TOKEN_NOT_EXIST = (-6),
    OAC_URI_OBS_LIST_FULL       = (-7),
    OAC_URI_OBS_IP_UPDATED      = (-8),
    OAC_URI_OBS_IP_NO_NEED_UPDATE = (-9),
    OAC_URI_OBS_NOT_SUB_REQUEST = (-10),
    OAC_URI_OBS_ADDED_NEW_DEVICE = (-11),
    OAC_URI_OBS_NO_NEED_UPDATE = (-12),

}oac_obsError_t;

typedef struct {
    oacu_token_t token[OAC_URI_OBS_TOKEN_LENGTH];
    uint8_t buffer[OAC_URI_OBS_BUFFER_SIZE];
} oac_uri_dataPacket_t;

typedef struct oac_uri_obs_t{
    oacu_token_t token[OAC_URI_OBS_TOKEN_LENGTH];
    oacu_uriIndex_t uriIndex; 
    uint8_t takenPosition_uri;
}oac_uri_obs_t;

typedef struct oac_uri_observer_t{
    char deviceNameFull[OAC_URI_OBS_DEVICENAME_FULL_SIZE];
    otIp6Address ipAddr;
    oac_uri_obs_t uri[OAC_URI_OBS_PAIRED_URI_MAX];
    uint8_t takenPosition_dev;
} oac_uri_observer_t;

/**
 * @brief todo
 * 
 * @return oac_uri_observer_t* 
 */
oac_uri_observer_t *oac_uri_obs_getSubListHandle(void);

/**
 * @brief get ptr to oac_uri_dataPacket_t struct. it is like as a buffer. You can override it
 * 
 * @return oac_uri_dataPacket_t* [out] ptr to oac_uri_dataPacket_t
 */
oac_uri_dataPacket_t *oac_uri_obs_getdataPacketHandle(void);

/**
 * @brief 
 * 
 * @param subListHandle 
 * @param token 
 * @param uriIndex 
 * @param ipAddr 
 * @return int8_t 
 */
int8_t oac_uri_obs_subscribe(oac_uri_observer_t *subListHandle, const oacu_token_t *token, oacu_uriIndex_t uriIndex, const otIp6Address *ipAddr, const char* deviceNameFull);

/**
 * @brief 
 * 
 * @param subListHandle 
 * @param aMessage 
 * @param aMessageInfo 
 * @param uriId 
 * @return int8_t 
 */
int8_t oac_uri_obs_subscribeFromUri(oac_uri_observer_t *subListHandle, otMessage *aMessage, const otMessageInfo *aMessageInfo, oacu_uriIndex_t uriId, char* deviceNameFull);

/**
 * @brief todo
 * 
 * @param subListHandle 
 * @param token 
 * @return int8_t 
 */
int8_t oac_uri_obs_unsubscribe(oac_uri_observer_t *subListHandle, char* deviceNameFull, const oacu_token_t *token);

/**
 * @brief todo
 * 
 * @param subListHandle 
 * @param serverUri 
 * @param dataToNotify 
 * @param dataSize 
 * @return int8_t 
 */
int8_t oac_uri_obs_notify(oac_uri_observer_t *subListHandle, const otIp6Address *excludedIpAddr, oacu_uriIndex_t uriIndex, const uint8_t *dataToNotify, uint16_t dataSize);

/**
 * @brief parse incomming message from notify
 * 
 * @param inBuffer 
 * @param out 
 * @return uint8_t 
 */
int8_t oac_uri_obs_parseMessageFromNotify(const uint8_t *inBuffer, oac_uri_dataPacket_t *out);

/**
 * @brief 
 * 
 * @param subListHandle 
 * @return int8_t 
 */
int8_t oac_uri_obs_deleteAll(oac_uri_observer_t *subListHandle);

/**
 * @brief todo
 * 
 * @param ipAddr 
 * @param aUriPath 
 * @param outToken 
 * @return int8_t 
 */
int8_t oac_uri_obs_sendSubscribeRequest(const otIp6Address *ipAddr, const char *aUriPath, uint8_t *tokenOut);

/**
 * @brief  todo
 * 
 * @param ipAddr 
 * @param aUriPath 
 * @param tokenIn 
 * @return int8_t 
 */
int8_t oac_uri_obs_sendSubscribeRequestUpdate(const otIp6Address *ipAddr, const char *aUriPath, uint8_t *tokenIn);

#ifdef UNIT_TEST

///////////////////////
// fn for devName
PRIVATE int8_t oac_uri_obs_spaceDevNameIsFree(oac_uri_observer_t *subListHandle);

PRIVATE int8_t oac_uri_obs_spaceDevNameTake(oac_uri_observer_t *subListHandle, int8_t tabDevId);

PRIVATE int8_t oac_uri_obs_spaceDevNameIsTaken(oac_uri_observer_t *subListHandle, int8_t tabDevId);

///////////////////////
// fn for uri
PRIVATE int8_t oac_uri_obs_spaceUriIsFree(oac_uri_observer_t *subListHandle, int8_t tabDevId);

PRIVATE int8_t oac_uri_obs_spaceUriTake(oac_uri_observer_t *subListHandle, int8_t tabDevId, int8_t tabUriId);

PRIVATE int8_t oac_uri_obs_spaceUriIsTaken(oac_uri_observer_t *subListHandle, int8_t tabDevId, int8_t tabUriId);

PRIVATE int8_t oac_uri_obs_uriIsExist(oac_uri_observer_t *subListHandle, int8_t tabDevId, oacu_uriIndex_t uriIndex);

PRIVATE int8_t oac_uri_obs_saveDeviceNameFull(oac_uri_observer_t *subListHandle, int8_t tabDevId, const char* deviceNameFull);

PRIVATE int8_t oac_uri_obs_saveIpAddr(oac_uri_observer_t *subListHandle, int8_t tabDevId, const otIp6Address *ipAddr);

PRIVATE int8_t oac_uri_obs_saveUriIndex(oac_uri_observer_t *subListHandle, int8_t tabDevId, int8_t tabUriId, oacu_uriIndex_t uriIndex);

PRIVATE int8_t oac_uri_obs_saveToken(oac_uri_observer_t *subListHandle, int8_t tabDevId, int8_t tabUriId, const oacu_token_t *token);

PRIVATE int8_t oac_uri_obs_addNewDevice(oac_uri_observer_t *subListHandle, const char* deviceNameFull, const otIp6Address *ipAddr);

PRIVATE int8_t oac_uri_obs_addNewUri(oac_uri_observer_t *subListHandle, int8_t tabDevId, const oacu_token_t *token, oacu_uriIndex_t uriIndex);

PRIVATE int8_t oac_uri_obs_ipAddrIsSame(oac_uri_observer_t *subListHandle, int8_t tabDevId, const otIp6Address *ipAddr);

PRIVATE int8_t oac_uri_obs_devNameFullIsSame(oac_uri_observer_t *subListHandle, int8_t tabDevId, const char *deviceNameFull);

PRIVATE int8_t oac_uri_obs_devNameFullIsExist(oac_uri_observer_t *subListHandle, const char *deviceNameFull);

/**
 * @brief todo
 * 
 * @param subListHandle 
 * @param subListIndex 
 * @param tokenToCheck 
 * @return PRIVATE 
 */
PRIVATE int8_t oac_uri_obs_tokenIsSame(oac_uri_observer_t *subListHandle, int8_t tabDevId, int8_t tabUriId, const oacu_token_t *tokenToCheck);

/**
 * @brief todo
 * 
 * @param subListHandle 
 * @param token 
 * @return PRIVATE 
 */
PRIVATE int8_t oac_uri_obs_tokenIsExist(oac_uri_observer_t *subListHandle, int8_t tabDevId, const oacu_token_t *token);

int8_t test_obs_fillListExampleData(oac_uri_observer_t *subListHandle);
#endif /* UNIT_TEST */

#endif  /* OT_APP_COAP_URI_OBS_H_ */
