/**
 * @file ot_app_pair.h
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
#ifndef OT_APP_PAIR_H_
#define OT_APP_PAIR_H_

#include "hro_utils.h"
#include "ot_app_coap_uri_obs.h"
#include "ot_app_coap.h"
#include "string.h"

#ifndef UNIT_TEST
    #include "ot_app.h"
#else
    #include "mock_ot_app.h"
#endif

#define OTAPP_PAIR_IS                       (1)
#define OTAPP_PAIR_IS_NOT                   (2)

#define OTAPP_PAIR_OK                       (-1)
#define OTAPP_PAIR_UPDATED                  (-2)
#define OTAPP_PAIR_NO_NEED_UPDATE           (-3)

#define OTAPP_PAIR_ERROR                    (-4)
#define OTAPP_PAIR_NO_EXIST                 (-5)
#define OTAPP_PAIR_DEVICE_NAME_EXIST        (-6)
#define OTAPP_PAIR_DEVICE_NAME_TO_LONG      (-7)
#define OTAPP_PAIR_DEVICE_NO_SPACE          (-8)

#define OTAPP_PAIR_DEVICES_MAX    OTAPP_PAIRED_DEVICES_MAX // max number of devices to save them from DNS query or URI

#define OTAPP_PAIR_URI_MAX                          OTAPP_PAIRED_URI_MAX // 5
#define OTAPP_PAIR_URI_RESOURCE_BUFFER_SIZE         (OTAPP_URI_MAX_NAME_LENGHT + sizeof(otapp_deviceType_t) + sizeof(uint8_t))
#define OTAPP_PAIR_URI_RESOURCE_BUFFER_MAX_SIZE     (OTAPP_PAIR_URI_RESOURCE_BUFFER_SIZE * OTAPP_PAIR_URI_MAX)

#define OTAPP_PAIR_URI_MAX_VAL    OTAPP_URI_END_OF_INDEX
#define OTAPP_PAIR_NAME_FULL_SIZE OTAPP_DEVICE_NAME_FULL_SIZE 
#define OTAPP_PAIR_NO_URI         OTAPP_URI_NO_URI_INDEX
#define OTAPP_PAIR_URI_INIT       OTAPP_URI_END_OF_INDEX

#define OTAPP_PAIR_QUEUE_LENGTH         10
#define OTAPP_PAIR_TASK_STACK_DEPTH     (128 * 17)
#define OTAPP_PAIR_TASK_PRIORITY        5

#define OTAPP_PAIR_RULES_ALLOWED_SIZE           10
#define OTAPP_PAIR_RULES_ALLOWED_ITEM_MAX_SIZE  OTAPP_END_OF_DEVICE_TYPE

#define OTAPP_PAIR_NO_RULES     (OTAPP_END_OF_DEVICE_TYPE + 1) 
#define OTAPP_PAIR_NO_ALLOWED   OTAPP_NO_DEVICE_TYPE 
#define OTAPP_PAIR_END_OF_RULES   OTAPP_END_OF_DEVICE_TYPE 

#define OTAPP_PAIR_OBSERVER_PAIRE_DDEVICE_CALLBACK_SIZE 10

typedef struct __attribute__((packed)){
    char uri[OTAPP_URI_MAX_NAME_LENGHT];     
    otapp_deviceType_t devTypeUriFn;    // It tell you what functions this uri has
    uint8_t obs;                        // is this uri has observer support 
}otapp_pair_resUrisParseData_t;

typedef uint8_t otapp_pair_resUrisBuffer_t[OTAPP_PAIR_URI_RESOURCE_BUFFER_SIZE];

typedef struct {
    char uri[OTAPP_URI_MAX_NAME_LENGHT];
    uint32_t uriState;
    otapp_deviceType_t devTypeUriFn;    // It tell you what functions this uri has
    oacu_token_t token[OAC_URI_OBS_TOKEN_LENGTH];   
}otapp_pair_uris_t;
typedef struct {
    char devNameFull[OTAPP_PAIR_NAME_FULL_SIZE]; // deviceNameFull
    otIp6Address ipAddr;
    otapp_pair_uris_t urisList[OTAPP_PAIR_URI_MAX]; // there will be saved data from resource Uri
}otapp_pair_Device_t;

typedef struct otapp_pair_DeviceList_t otapp_pair_DeviceList_t;

typedef struct {
    otapp_deviceType_t allowed[OTAPP_PAIR_RULES_ALLOWED_SIZE]; 
} otapp_pair_rule_t;


typedef enum {
    OTAPP_PAIR_CHECK_AND_ADD_TO_DEV_LIST     
} otapp_pair_QueueDataType_t;

typedef struct {
    otapp_pair_QueueDataType_t type;
    char deviceNameFull[OTAPP_PAIR_NAME_FULL_SIZE];
    otIp6Address ipAddress;
} otapp_pair_queueItem_t;

/**
 * @brief todo
 * 
 * @return int8_t 
 */
int8_t otapp_pair_init(ot_app_devDrv_t *driver);

/**
 * @brief observer Callback. 
 * @param   [out] newDevice ptr to data struct otapp_pair_Device_t
 */
typedef void (*otapp_pair_observerCallback_t)(otapp_pair_Device_t *newDevice);

/**
 * @brief TODO
 * 
 * @param callback 
 * @return int8_t 
 */
int8_t otapp_pair_observerPairedDeviceRegisterCallback(otapp_pair_observerCallback_t callback);

/**
 * @brief add new device to the pairing list
 * 
 * @param pairDeviceList  [in] handle ptr of otapp_pair_DeviceList_t. Use: otapp_pair_getHandle()
 * @param deviceNameFull  [in] char ptr of full device name ("device1_1_588c81fffe301ea4")
 * @param ipAddr          [in] ptr to IP address of the new device
 * @return int8_t         [out] table index of pairDeviceList->list[tableIndex] 
 *                              or  OTAPP_PAIR_ERROR, OTAPP_PAIR_DEVICE_NAME_EXIST, OTAPP_PAIR_DEVICE_NAME_TO_LONG
 */
int8_t otapp_pair_DeviceAdd(otapp_pair_DeviceList_t *pairDeviceList, const char *deviceNameFull, otIp6Address *ipAddr);

/**
 * @brief get indexDevice from device list
 * 
 * @param pairDeviceList  [in] handle ptr of otapp_pair_DeviceList_t. Use: otapp_pair_getHandle()  
 * @param deviceNameFull  [in] char ptr of full device name ("device1_1_588c81fffe301ea4") 
 * @return int8_t         [out] indexDevice 
 *                              or OTAPP_PAIR_ERROR (-1)
 */
int8_t otapp_pair_DeviceIndexGet(otapp_pair_DeviceList_t *pairDeviceList, const char *deviceNameFull);

/**
 * @brief todo
 * 
 * @param pairDeviceList 
 * @param deviceNameFull 
 * @return otapp_pair_Device_t* 
 */
otapp_pair_Device_t *otapp_pair_DeviceGet(otapp_pair_DeviceList_t *pairDeviceList, const char *deviceNameFull);

/**
 * @brief get deviceNameFull from device list
 * 
 * @param pairDeviceList  [in] handle ptr of otapp_pair_DeviceList_t. Use: otapp_pair_getHandle() 
 * @param indexDevice     [in] index of device 
 * @return char*          [out] ptr to deviceNameFull, 
 *                              or NULL if error
 */
char *otapp_pair_DeviceNameGet(otapp_pair_DeviceList_t *pairDeviceList, uint8_t indexDevice);

/**
 * @brief delete paired device
 * 
 * @param pairDeviceList  [in] handle ptr of otapp_pair_DeviceList_t. Use: otapp_pair_getHandle()
 * @param deviceNameFull  [in] char ptr of full device name ("device1_1_588c81fffe301ea4")
 * @return int8_t         [out] table index of deleted device
 *                              or  OTAPP_PAIR_ERROR (-1) 
 */
int8_t otapp_pair_DeviceDelete(otapp_pair_DeviceList_t *pairDeviceList, const char *deviceNameFull);

/**
 * @brief delete all elements in pairDeviceList
 * 
 * @param pairDeviceList  [in] handle ptr of otapp_pair_DeviceList_t. Use: otapp_pair_getHandle() 
 * @return int8_t         [out] return OTAPP_PAIR_OK or OTAPP_PAIR_ERROR
 */
int8_t otapp_pair_DeviceDeleteAll(otapp_pair_DeviceList_t *pairDeviceList);

/**
 * @brief get ptr of otapp_pair_DeviceList_t handle
 * 
 * @return otapp_pair_DeviceList_t* [out] ptr of otapp_pair_DeviceList_t handle
 */
otapp_pair_DeviceList_t *otapp_pair_getHandle(void);

/**
 * @brief get saved IP address from device list
 * 
 * @param pairDeviceList  [in] handle ptr of otapp_pair_DeviceList_t. Use: otapp_pair_getHandle() 
 * @param indexDevice     [in] index of device 
 * @return otIp6Address*  [out] ptr to otIp6Address, 
 *                              or NULL if error
 */
otIp6Address *otapp_pair_ipAddressGet(otapp_pair_DeviceList_t *pairDeviceList, uint8_t indexDevice);

/**
 * @brief check the ip address is same as saved in given device index
 * 
 * @param pairDeviceList  [in] handle ptr of otapp_pair_DeviceList_t. Use: otapp_pair_getHandle() 
 * @param indexDevice     [in] index of device  
 * @param ipAddr          [in] ptr to otIp6Address
 * @return int8_t         [out] OTAPP_PAIR_IS, OTAPP_PAIR_IS_NOT
 *                              or OTAPP_PAIR_ERROR, OTAPP_PAIR_NO_EXIST
 */
int8_t otapp_pair_ipAddressIsSame(otapp_pair_DeviceList_t *pairDeviceList, uint8_t indexDevice, otIp6Address *ipAddr);

/**
 * @brief update ip address when it is different than saved
 * 
 * @param pairDeviceList  [in] handle ptr of otapp_pair_DeviceList_t. Use: otapp_pair_getHandle()  
 * @param indexDevice     [in] index of device  
 * @param ipAddr          [in] ptr to new otIp6Address 
 * @return int8_t         [out] OTAPP_PAIR_UPDATED, OTAPP_PAIR_NO_NEED_UPDATE or if error: OTAPP_PAIR_ERROR, OTAPP_PAIR_NO_EXIST
 */
int8_t otapp_pair_ipAddressUpdate(otapp_pair_DeviceList_t *pairDeviceList, uint8_t indexDevice, otIp6Address *ipAddrNew);

/**
 * @brief print all saved data of device
 * 
 * @param pairDeviceList  [in] handle ptr of otapp_pair_DeviceList_t. Use: otapp_pair_getHandle() 
 * @param indexDevice     [in] index of device to be print
 */
void otapp_pair_devicePrintData(otapp_pair_DeviceList_t *pairDeviceList, uint8_t indexDevice);

/**
 * @brief todo
 * 
 * @param queueItem 
 * @return int8_t 
 */
int8_t otapp_pair_addToQueue(otapp_pair_queueItem_t *queueItem);

/**
 * @brief 
 * 
 * @param inBuffer 
 * @param inBufferSize 
 * @param result 
 * @return otapp_pair_resUrisParseData_t* 
 */
otapp_pair_resUrisParseData_t *otapp_pair_uriParseMessage(const uint8_t *inBuffer, uint16_t inBufferSize, int8_t *result, uint16_t *outParsedDataSize);

/**
 * @brief 
 * 
 * @param deviceUrisList 
 * @param uriData 
 * @param token 
 * @return int8_t 
 */
int8_t otapp_pair_uriAdd(otapp_pair_uris_t *deviceUrisList, const otapp_pair_resUrisParseData_t *uriData, const oacu_token_t *token);

otapp_pair_resUrisBuffer_t *otapp_pair_uriResourcesCreate(otapp_coap_uri_t *uri, uint8_t uriSize, int8_t *result, uint16_t *outBufSize);

/**
 * @brief set uri state on the pair device list. Max data per uri = uint32_t 
 * 
 * @param pairDeviceList    [in] ptr to Pair devices list. @see getHandle()
 * @param token             [in] ptr to token of uri
 * @param uriState          [in] ptr to uri state. Max uint32_t
 * @return int8_t           [out] OTAPP_PAIR_OK or OTAPP_PAIR_ERROR
 */ 
int8_t otapp_pair_uriStateSet(otapp_pair_DeviceList_t *pairDeviceList, const oacu_token_t *token, const uint32_t *uriState);

/**
 * @brief todo
 * 
 * @param pairDeviceList 
 * @return int8_t 
 */
int8_t otapp_pair_subSendUpdateIP(otapp_pair_DeviceList_t *pairDeviceList);

otapp_pair_uris_t *otapp_pair_tokenGetUriIteams(otapp_pair_DeviceList_t *pairDeviceList, const oacu_token_t *token);

/**
 * @brief looking for ID of urisList in urisList
 * 
 * @param deviceHandle  [in] ptr to otapp_pair_Device_t
 * @param uriDevType    [in] type of device, we will be looking for in the uriList
 * @return int8_t       [out] index of urisList from otapp_pair_uris_t when uriDevType exist or OTAPP_PAIR_NO_EXIST
 */
int8_t otapp_pair_uriGetIdList(otapp_pair_Device_t *deviceHandle, otapp_deviceType_t uriDevType);

#ifdef UNIT_TEST

/**
 * @brief check free space of pairDeviceList->list[]
 * 
 * @param pairDeviceList  [in] handle ptr of otapp_pair_DeviceList_t. Use: otapp_pair_getHandle()
 * @return int8_t         [out] free table index of pairDeviceList->list[tableIndex] 
 *                              or  OTAPP_PAIR_ERROR (-1)
 */
PRIVATE int8_t otapp_pair_DeviceIsFreeSpace(otapp_pair_DeviceList_t *pairDeviceList);

/**
 * @brief reserve space in pairing device list 
 * 
 * @param pairDeviceList  [in] handle ptr of otapp_pair_DeviceList_t. Use: otapp_pair_getHandle()
 * @param indexDevice     [in] index of free position of pairDeviceList->list[tableIndex] 
 * @note to get free position of device list you should use otapp_pair_DeviceIsFreeSpace()
 */
PRIVATE void otapp_pair_spaceTake(otapp_pair_DeviceList_t *pairDeviceList, uint8_t indexDevice);

/**
 * @brief check if space in paring device list is taken
 * 
 * @param pairDeviceList [in] handle ptr of otapp_pair_DeviceList_t. Use: otapp_pair_getHandle()
 * @param indexDevice    [in] index of device name to check
 * @return int8_t        [out] TRUE = 1, FALSE = 0
 *                             or OTAPP_PAIR_ERROR
 */
PRIVATE int8_t otapp_pair_spaceIsTaken(otapp_pair_DeviceList_t *pairDeviceList, uint8_t indexDevice);

/**
 * @brief check if deviceNameFull is existing on pairing device list
 * 
 * @param pairDeviceList  [in] handle ptr of otapp_pair_DeviceList_t. Use: otapp_pair_getHandle()
 * @param deviceNameFull  [in] char ptr of full device name ("device1_1_588c81fffe301ea4")
 * @return int8_t         [out] index of device name found in table list
 *                              or  OTAPP_PAIR_ERROR (-1)
 */
PRIVATE int8_t otapp_pair_DeviceIsExist(otapp_pair_DeviceList_t *pairDeviceList, const char *deviceNameFull);

/**
 * @brief compare two deviceNameFull
 * 
 * @param pairDeviceList  [in] handle ptr of otapp_pair_DeviceList_t. Use: otapp_pair_getHandle() 
 * @param deviceNameFull  [in] char ptr of full device name ("device1_1_588c81fffe301ea4") 
 * @param indexDevice     [in] device index to comparison 
 * @return int8_t         [out] TRUE = OTAPP_PAIR_IS, FALSE = OTAPP_PAIR_IS_NOT, 
 *                              or OTAPP_PAIR_ERROR (-1) if error
 */
PRIVATE int8_t otapp_pair_deviceNameIsSame(otapp_pair_DeviceList_t *pairDeviceList, const char *deviceNameFull, uint8_t indexDevice);

/**
 * @brief todo
 * 
 * @param pairDeviceList 
 * @param queueIteam 
 * @return PRIVATE 
 */
PRIVATE int8_t otapp_pair_deviceIsMatchingFromQueue(otapp_pair_queueItem_t *queueIteam);

/**
 * @brief todo
 * 
 * @return PRIVATE 
 */
PRIVATE int8_t otapp_pair_initQueue(void);

/**
 * @brief todo
 * 
 * @return PRIVATE 
 */
PRIVATE int8_t otapp_pair_initTask(void);

/**
 * @brief todo
 * 
 * @param errorState 
 * @param deviceNameFull 
 * @return PRIVATE 
 */
PRIVATE int8_t otapp_pair_observerPairedDeviceNotify(otapp_pair_Device_t *newDevice);

/**
 * @brief todo
 * 
 * @param deviceDrv 
 * @param mainDeviceID 
 * @param incommingDeviceID 
 * @return int8_t 
 */
PRIVATE int8_t otapp_pair_deviceIsAllowed(ot_app_devDrv_t *deviceDrv, otapp_deviceType_t incommingDeviceID);

PRIVATE int8_t otapp_pair_tokenIsSame(otapp_pair_DeviceList_t *pairDeviceList, int8_t devListId, int8_t uriListId, const oacu_token_t *tokenToCheck);


#endif  /* UNIT_TEST */

#endif  /* OT_APP_PAIR_H_ */
