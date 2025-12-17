/**
 * @file ot_app_deviceName.h
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief 
 * @version 0.1
 * @date 26-08-2025
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
#ifndef OT_APP_DEVICENAME_H_
#define OT_APP_DEVICENAME_H_

#include "hro_utils.h"

#ifndef UNIT_TEST
    #include "ot_app.h"
#else
    #include "mock_ot_app.h" 
    #include "mock_ip6.h"    
    #include "mock_mocks.h"     
#endif

#define OTAPP_DEVICENAME_IS                 (1)
#define OTAPP_DEVICENAME_IS_NOT             (2)
#define OTAPP_DEVICENAME_OK                 (-1)

#define OTAPP_DEVICENAME_ERROR              (-2)
#define OTAPP_DEVICENAME_TOO_LONG           (-3)
#define OTAPP_DEVICENAME_TOO_SHORT          (-4)
#define OTAPP_DEVICENAME_BUFFER_TOO_SMALL   (-5)
#define OTAPP_DEVICENAME_CALL_DEVICE_NAME_SET_FN   (-6)

#define OTAPP_DEVICENAME_MAX_DEVICE_TYPE    OTAPP_END_OF_DEVICE_TYPE

#ifndef OTAPP_DEVICENAME_FULL_SIZE
    #define OTAPP_DEVICENAME_FULL_SIZE OTAPP_DNS_SRV_LABEL_SIZE
#endif

#define OTAPP_DEVICENAME_SIZE               (OTAPP_DEVICENAME_FULL_SIZE - 22)
#define OTAPP_DEVICENAME_MIN_SIZE           (OTAPP_DEVICENAME_FULL_SIZE - OTAPP_DEVICENAME_SIZE + 1)
#define OTAPP_DEVICENAME_MIN_ADD_DOMAIN_BUFFER_SIZE           (2 * OTAPP_DEVICENAME_FULL_SIZE)

/**
 * @brief set device name + device type. Max length of device name: 10 bytes
 * 
 * @param deviceName    [in] char ptr to device name MAX 10 BYTES
 * @param deviceType    [in] device type of otapp_deviceType_t enum
 * @return int8_t       [out] OTAPP_DEVICENAME_OK,
 *                            or if error: OTAPP_DEVICENAME_ERROR, OTAPP_DEVICENAME_TOO_LONG
 * 
 */
int8_t otapp_deviceNameSet(const char *deviceName, const otapp_deviceType_t deviceType);

/**
 * @brief get full name of current device
 * 
 * @return const char* [out] char ptr to device name full 
 */
const char *otapp_deviceNameFullGet(void);

/**
 * @brief compare device name full with current device name full
 * 
 * @param deviceNameFull [in] char ptr to device name full which will be compared
 * @return int8_t        [out] OTAPP_DEVICENAME_IS, OTAPP_DEVICENAME_IS_NOT
 *                             or if error: OTAPP_DEVICENAME_ERROR, OTAPP_DEVICENAME_TOO_LONG
 * @note this function compare "device1_1_588c81fffe301ea4"
 */
int8_t otapp_deviceNameFullIsSame(const char *deviceNameFull);

/**
 * @brief compare device name with current device name "device1_1_588c81fffe301ea4"
 * 
 * @param deviceNameFull [in] char ptr to device name full which will be compared
 * @param stringLength   [in] lenght of the string.
 * @return int8_t        [out] OTAPP_DEVICENAME_IS, OTAPP_DEVICENAME_IS_NOT
 *                             or if error: OTAPP_DEVICENAME_ERROR, OTAPP_DEVICENAME_TOO_LONG, OTAPP_DEVICENAME_CALL_DEVICE_NAME_SET_FN
 * @note this function compare "device1", NOT "device1_1_588c81fffe301ea4"
 */
int8_t otapp_deviceNameIsSame(const char *deviceNameFull, uint8_t stringLength);

/**
 * @brief get device ID from device name full
 * 
 * @param deviceNameFull [in] char ptr to device name full 
 * @param stringLength   [in] lenght of the string.  
 * @return otapp_deviceType_t [out] number of device type from otapp_deviceType_t,
 *                                  or if error: OTAPP_DEVICENAME_ERROR, OTAPP_DEVICENAME_TOO_LONG
 */
int16_t otapp_deviceNameGetDevId(const char *deviceNameFull, uint8_t stringLength);

/**
 * @brief add domain name sting to deviceNameFull. 
 * 
 * @param labelName [in] char ptr to deviceNameFull buffer. The char buffer should be 64 bytes
 *                  [out] connected domain name to deviceNameFull in the same char buffer
 * @return          [out] devID (otapp_deviceType_t)
 *                        or if error: OTAPP_DEVICENAME_ERROR, OTAPP_DEVICENAME_TOO_LONG, OTAPP_DEVICENAME_BUFFER_TOO_SMALL, OTAPP_DEVICENAME_TOO_SHORT
 * @attention char buffer should be 64 bytes (including deviceNameFull) 
 * @note after this, the char buffer should looks like "device1_1_588c81fffe301ea4.default.service.arpa."
 */
int8_t otapp_deviceNameFullAddDomain(char *deviceFullName, uint16_t bufLength);

/**
 * @brief separate device full name from host name
 * 
 * @param hostName [in] char ptr to host name buffer.
 *                 [out] cut domain name in the same char buffer
 * @return int8_t  [out] OTAPP_DEVICENAME_OK
 *                       or if error: OTAPP_DEVICENAME_ERROR
 * @note  after this, the char buffer should looks like "device1_1_588c81fffe301ea4"
 */
int8_t otapp_hostNameToDeviceNameFull(char *hostName);

/**
 * @brief todo 
 * 
 * @param deviceFullName 
 * @return int8_t 
 */
int8_t otapp_deviceNameIsMatching(const char *deviceFullName);

/**
 * @brief todo
 * 
 */
void otapp_deviceNameDelete(void);

/**
 * @brief decode EUI from deviceNameFull
 * 
 * @param deviceNameFull    [in] string ptr to device name full
 * @param stringLength      [in] string lenght
 * @param outEuiChrPtr      [out] ptr to EUI from string. It looks for EUI string from deviceNameFull and return ptr to first element of EUI string.
 * @return int8_t           [out] OTAPP_DEVICENAME_OK or OTAPP_DEVICENAME_ERROR
 */
int8_t otapp_deviceNameFullToEUI(const char *deviceNameFull, uint8_t stringLength, char **outEuiChrPtr);

/**
 * @brief compare EUI from deviceNameFull with given EUI 
 * 
 * @param deviceNameFull    [in] string ptr to device name full
 * @param eui               [in] ptr to EUI
 * @return int8_t           [out] OTAPP_DEVICENAME_IS OTAPP_DEVICENAME_IS_NOT OTAPP_DEVICENAME_ERROR
 */
int8_t otapp_deviceNameEuiIsSame(const char *deviceNameFull, const char *eui);

#endif  /* OT_APP_DEVICENAME_H_ */
