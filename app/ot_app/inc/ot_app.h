/**
 * @file ot_app.h
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief 
 * @version 0.1
 * @date 14-07-2025
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
#ifndef THREAD_OT_APP_H_
#define THREAD_OT_APP_H_

#include "hro_utils.h"

#ifdef UNIT_TEST
    #include "mock_ot_app_coap.h"
    #include "mock_ip6.h"
#else
    #include "ot_app_coap.h"
    #include "openthread/dns_client.h"
#endif

#define OTAPP_LOG_ENABLE

#ifdef OTAPP_LOG_ENABLE
    #define OTAPP_PRINTF HRO_PRINTF 
#else
    // logs disable
    #define OTAPP_PRINTF(fmt, ...) ((void)0)
#endif

#define OTAPP_CCA_THRESHOLD         (-70)

#define OTAPP_OK        (-1)
#define OTAPP_ERROR     (-2)


#define OTAPP_UDP_PORT 12345
#define OTAPP_CHAR_BUFFER_SIZE 1024 

#define OTAPP_PAIRED_DEVICES_MAX    10  // max number of devices to save them from DNS query
#define OTAPP_PAIRED_URI_MAX        3   // max number of uris to save 
#define OTAPP_URI_MAX_NAME_LENGHT   24  // max lenght of uri string name

#define OTAPP_DNS_SRV_NAME_SIZE     64 // OT_DNS_MAX_NAME_SIZE full service name: "_coap._udp.default.service.arpa." 
#define OTAPP_DNS_SRV_LABEL_SIZE    32 // OT_DNS_MAX_LABEL_SIZE host name: "device1_1_588c81fffe301ea4"
#define OTAPP_DNS_SRV_TXT_SIZE      512

#define OTAPP_DEVICE_NAME_FULL_SIZE      OTAPP_DNS_SRV_LABEL_SIZE
typedef struct ot_app_devDrv_t ot_app_devDrv_t; // forward declaration

typedef enum {
    OTAPP_NO_DEVICE_TYPE = 0,
    
    OTAPP_CONTROL_PANEL = 1 ,
    OTAPP_SWITCH,

    OTAPP_LIGHTING,
    OTAPP_LIGHTING_ON_OFF,
    OTAPP_LIGHTING_DIMM,
    OTAPP_LIGHTING_RGB,
    
    OTAPP_THERMOSTAT,
    OTAPP_THERMOSTAT_SET_TEMP,
    OTAPP_THERMOSTAT_READ_SET_TEMP,
    OTAPP_THERMOSTAT_READ_CURRENT_TEMP,
    
    OTAPP_SENSOR,               // only out data
    OTAPP_DOOR_LOCK,            // only out data
    OTAPP_MOTION_DETECTOR,      // only out data
    OTAPP_REMOTE_CONTROL,
    OTAPP_ENERGY_METER,
    OTAPP_SMART_PLUG,
    OTAPP_ENVIRONMENT_SENSOR,
    OTAPP_DOOR_SENSOR,
    OTAPP_ALARM,

    OTAPP_END_OF_DEVICE_TYPE
}otapp_deviceType_t;

/**
 * @brief todo
 * 
 * @return ot_app_devDrv_t* 
 */
ot_app_devDrv_t *otapp_getDevDrvInstance(void);

/**
 * @brief todo
 * 
 * @return otInstance* 
 */
otInstance *otapp_getOpenThreadInstancePtr(void);

/**
 * @brief todo
 * 
 * @return const otIp6Address* 
 */
const otIp6Address *otapp_multicastAddressGet(void);

/**
 * @brief todo
 * 
 * @return const otIp6Address* 
 */
const otIp6Address *otapp_ip6AddressGet(void);

/**
 * @brief get PTR char buffer address and take MUTEX * 
 * @return char* PTR to char buffer * 
 * @note remember to release mutex by otapp_charBufRelease()
 * 
 */
char *otapp_charBufGet_withMutex(void);

/**
 * @brief todo
 * 
 */
void otapp_charBufRelease(void);

/**
 * @brief todo
 * 
 * @param aAddress 
 */
void otapp_ip6AddressPrint(const otIp6Address *aAddress);

/**
 * @brief todo
 * 
 * @return const otIp6Address* 
 */
const otIp6Address *otapp_ip6AddressRefresh(void);

/**
 * @brief todo
 * 
 * @return const otExtAddress* 
 */
const otExtAddress *otapp_macAddrGet(void);

/**
 * @brief todo
 * 
 */
int8_t otapp_init(void);

/**
 * @brief todo
 * 
 */
void otapp_network_init();
void otapp_setDataset_tlv(void);

#endif  /* THREAD_OT_APP_H_ */


