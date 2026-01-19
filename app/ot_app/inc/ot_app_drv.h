/**
 * 
 * @file ot_app_drv.h
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @version 0.1
 * @date 08-09-2025
 * 
 * @defgroup ot_app_device_api main API for OpenThread hardware application
 * @{
 * @brief Main API for OpenThread hardware application device driver.
 * 
 * This file is part of the OpenThread Application Framework.
 * Licensed under the MIT License; see the LICENSE file for details.
 *
 * @see @ref author_sec "Author & License"
 *
 * @section description Description
 *
 * Main API for OpenThread hardware application device driver.
 *
 * **Provides unified interface for managing hardware devices in Thread networks:**
 * - **Device Pairing** through filtering rules and observer callbacks
 * - **URI/CoAP Handling** for remote resource control
 * - **Device Grouping** via name groups (`deviceName`)
 * - **Persistent Storage** using NVS subsystem
 * - **IPv6/CoAP Network Communication** with other Thread devices
 *
 * **Key Features:**
 * - Singleton instance via `ot_app_drv_getInstance()`
 * - Application callbacks for dynamic behavior control
 * - API subsystems: `pair`, `nvs`, `coap`, `devName`
 * - Automatic task processing via `ot_app_drv_task()`
 *
 * **Example Initialization**:
 * ```c
 * ot_app_devDrv_t *drv = ot_app_drv_getInstance();
 * 
 * // CRITICAL: Initialize NVS FIRST before any other operations
 * // **MUST call NVS init before otapp_init():**
 * 
 * if(drv->api.nvs.init == NULL) return;  // Safety check
 * drv->api.nvs.init();                   // Required before other operations
 * 
 * 
 * // Register device callbacks and properties
 * drv->pairRuleGetList_clb = myPairRulesCallback;
 * drv->uriGetList_clb = myUriListCallback;
 * drv->obs_pairedDevice_clb = myPairedCallback;
 * drv->obs_subscribedUri_clb = mySubscribedCallback;
 * drv->deviceName = myDeviceNameGroup;
 * drv->deviceType = &myDeviceType;
 * drv->task = myMainTask;
 * drv->uriGetListSize = myUriCount;
 * ```
 *
 * **Pairing Requirements:**
 * - Devices must share the same `deviceName` group
 * - Target device must match `pairRuleGetList_clb` rules
 *
 * Framework automatically calls registered callbacks during:
 * - New device discovery (`obs_pairedDevice_clb`)
 * - URI state updates (`obs_subscribedUri_clb`)
 *
 * @see ot_app_devDrv_t for complete driver structure
 * @see ot_app_drv_getInstance() for singleton access
 * @see ad_button_Init() for full example implementation [file:1]
 */

#ifndef OT_APP_DRV_H_
#define OT_APP_DRV_H_

#include "ot_app_pair.h"
#include "stdint.h"

#include "ot_app_coap_uri_obs.h"

#ifndef UNIT_TEST
    #include "ot_app_coap.h"
    #include "ot_app.h"
#else
    #include "mock_ot_app.h"
    #include "mock_ot_app_coap.h"     
#endif

/**
 * @brief NVS Non-Volatile Storage functions
 */
typedef struct ot_app_drv_nvs_t{
    /**
     * @brief save data into NVS partition
     * 
     * @param inData        [in] ptr to data
     * @param keyId         [in] it is id for saved data. It will be necessary for update or read data
     * @return int8_t       OT_APP_NVS_ERROR or OT_APP_NVS_OK
     */
    int8_t (*saveString)(const char *inData, const uint8_t keyId);

    /**
     * @brief read data from NVS partition
     * 
     * @param outData        [out] ptr to data
     * @param outDataSize    [out] ptr to the size of the data that was read
     * @param keyId          [in] it is id for saved data. It will be necessary for update or read data
     * @return int8_t        OT_APP_NVS_ERROR or OT_APP_NVS_OK
     */
    int8_t (*readString)(char *outBuff, uint8_t outBuffSize, const uint8_t keyId);

    /**
     * @brief NVS init function 
     * @warning it has to be inicjalized befor otapp_init() and first using.
     */
    int8_t (*init)(void);

}ot_app_drv_nvs_t;

typedef struct ot_app_drv_pair_t{
    /**
     * @brief get ptr of otapp_pair_DeviceList_t handle
     * 
     * @return otapp_pair_DeviceList_t* [out] ptr of otapp_pair_DeviceList_t handle
    */ 
    otapp_pair_DeviceList_t *(*getHandle)(void);

    /**
     * @brief set uri state on the pair device list. Max data per uri = uint32_t 
     * 
     * @param pairDeviceList    [in] ptr to Pair devices list. @see getHandle()
     * @param token             [in] ptr to token of uri
     * @param uriState          [in] ptr to uri state. Max uint32_t
     * @return int8_t           [out] OTAPP_PAIR_OK or OTAPP_PAIR_ERROR
     */ 
    int8_t (*uriStateSet)(otapp_pair_DeviceList_t *pairDeviceList, const oacu_token_t *token, const uint32_t *uriState);

    /**
     * @brief looking for ID of urisList in urisList
     * 
     * @param deviceHandle  [in] ptr to otapp_pair_Device_t
     * @param uriDevType    [in] type of device, we will be looking for in the uriList
     * @return int8_t       [out] index of urisList from otapp_pair_uris_t when uriDevType exist or OTAPP_PAIR_NO_EXIST
     */
    int8_t (*uriGetIdList)(otapp_pair_Device_t *deviceHandle, otapp_deviceType_t uriDevType);

}ot_app_drv_pair_t;

typedef struct ot_app_drv_devName_t{

    /**
     * @brief decode EUI from deviceNameFull
     * 
     * @param deviceNameFull    [in] string ptr to device name full
     * @param stringLength      [in] string lenght
     * @param outEuiChrPtr      [out] ptr to EUI from string. It looks for EUI string from deviceNameFull and return ptr to first element of EUI string.
     * @return int8_t           [out] OTAPP_DEVICENAME_OK or OTAPP_DEVICENAME_ERROR
     */
    int8_t (*devNameFullToEUI)(const char *deviceNameFull, uint8_t stringLength, char **outEuiChrPtr);    

    /**
     * @brief compare EUI from deviceNameFull with given EUI 
     * 
     * @param deviceNameFull    [in] string ptr to device name full
     * @param eui               [in] ptr to EUI
     * @return int8_t           [out] OTAPP_DEVICENAME_IS OTAPP_DEVICENAME_IS_NOT OTAPP_DEVICENAME_ERROR
     */
    int8_t (*devNameEuiIsSame)(const char *deviceNameFull, const char *eui);
}ot_app_drv_devName_t;


typedef struct ot_app_drv_coap_t{

    /**
     * @brief send coap message bytes using the PUT method
     * 
     * @param peer_addr         [in] ptr to device IPv6 address
     * @param aUriPath          [in] string ptr to uri
     * @param payloadMsg        [in] ptr to payload data
     * @param payloadMsgSize    [in] payload size
     * @param responseHandler   [in] callback to response handler
     * @param aContext          [in] content will be provided with responseHandler
     */
    void (*sendBytePut)(const otIp6Address *peer_addr, const char *aUriPath, const uint8_t *payloadMsg, const uint16_t payloadMsgSize, otCoapResponseHandler responseHandler,  void *aContext);
    
    /**
     * @brief send coap request using the GET method. Response will contain bytes
     * 
     * @param peer_addr         [in] ptr to device IPv6 address
     * @param aUriPath          [in] string ptr to uri
     * @param responseHandler   [in] callback to response handler
     * @param aContext          [in] content will be provided with responseHandler
     */
    void (*sendByteGet)(const otIp6Address *peer_addr, const char *aUriPath, otCoapResponseHandler responseHandler, void *aContext);
    
    /**
     * @brief send coap response to device
     * 
     * @param requestMessage    [in] ptr to request message
     * @param aMessageInfo      [in] ptr to message info
     * @param responceContent   [in] ptr to response data
     * @param responceLength    [in] size of response data
     */
    void (*sendResponse)(otMessage *requestMessage, const otMessageInfo *aMessageInfo, const uint8_t *responceContent, uint16_t responceLength);
    
    /**
     * @brief read incoming message and save them to in buffer
     * 
     * @param aMessage      [in] ptr to incoming message
     * @param bufferOut     [out] ptr to out buffer
     * @param bufferSize    [in] size of bufferOut
     * @param readBytesOut  [out] ptr to the uint16_t variable, where the size of the read bytes will be saved
     * @return int8_t       [out] OTAPP_COAP_OK or OTAPP_COAP_ERROR
     */
    int8_t (*readPayload)(otMessage *aMessage, uint8_t *bufferOut, uint16_t bufferSize, uint16_t *readBytesOut);
    
    /**
     * @brief main function for HARDWARE DEVICE URIS. This function processing incoming request. 
     *        Request can be concern adding device to subscribe list or general uri request to next processing.
     *        If it is general uri request, then the given bufOut contains data to processing. 
     *          
     * @param aMessage      [in] ptr to incoming message
     * @param aMessageInfo  [in] ptr to message info
     * @param uriId         [in] specific HARDWARE DEVICE URI ID that was passed in uriGetListSize callback
     * @param bufOut        [out] ptr to buffer. Incomming data will be saved there.
     * @param bufSize       [in] size of bufOut
     * @return int8_t       [out]   OTAPP_COAP_OK when same data was saved in buffer,
     *                              OAC_URI_OBS_ADDED_NEW_DEVICE or OAC_URI_OBS_NO_NEED_UPDATE when observer function processed,
     *                              OTAPP_COAP_ERROR
     */
    int8_t (*processUriRequest)(otMessage *aMessage, const otMessageInfo *aMessageInfo, oacu_uriIndex_t uriId, uint8_t *bufOut, uint16_t bufSize);
}ot_app_drv_coap_t;

typedef struct ot_app_devDrvAPI_t{
    ot_app_drv_pair_t   pair;
    ot_app_drv_devName_t devName;
    ot_app_drv_nvs_t    nvs;
    ot_app_drv_coap_t   coap;   // coap functions  
}ot_app_devDrvAPI_t;

typedef uint8_t ot_app_size_t;
typedef otapp_pair_rule_t *(*pairRuleGet_callback_t)(void);
typedef otapp_coap_uri_t *(*uriGet_callback_t)(void);
typedef void (*subscribedUris_callback_t)(oac_uri_dataPacket_t *dataPacket);
typedef void (*mainTask_callback_t)(void);

typedef struct ot_app_devDrv_t{
    subscribedUris_callback_t     obs_subscribedUri_clb; // it will be called from subscribed_uris uri

    /**
     * @brief it will be called when new device has been properly paired. All data are saved in otapp_pair_DeviceList_t.
     * @param   [out] newDevice ptr to data struct otapp_pair_Device_t
     */
    otapp_pair_observerCallback_t obs_pairedDevice_clb;  

    /**
     * @brief Callback function to retrieve the list of pairing rules.     *
     * @details
     * This callback is used by the OpenThread device driver to obtain the current
     * set of pairing rules that determine which devices are allowed to pair.
     *
     * When invoked, the callback returns a pointer to an array of pairing rules
     * (otapp_pair_rule_t) specifying device types allowed or disallowed for pairing.
     *
     * The application should assign this callback to provide dynamic or static
     * pairing rules depending on the system's pairing policy.
     *
     * **Configuration:**
     * - OTAPP_PAIR_NO_RULES: Special flag indicating no filtering
     * - OTAPP_PAIR_NO_ALLOWED: Special flag blocking all pairing attempts
     * - OTAPP_PAIR_END_OF_RULES: Terminator for rule array
     *
     * @note OTAPP_PAIR_RULES_ALLOWED_SIZE = 10 (maximum rules per set)
     * @return Pointer to the pairing rule list (array of otapp_pair_rule_t).
     */
    pairRuleGet_callback_t      pairRuleGetList_clb;

    /**
     * @brief Callback function to retrieve the list of URIs.
     * @details
     * This callback is used to obtain the list of supported URIs exposed by the device.
     * The URIs typically represent accessible CoAP resource endpoints for network interaction.
     *
     * When called, the callback returns a pointer to a list or array holding the URIs
     * that the device registers or supports for remote access or commands.
     *
     * This allows the framework or application to discover and interact with device capabilities dynamically.
     *
     * @note Maximum URIs: OTAPP_PAIRED_URI_MAX
     * @note Maximum URI name length: OTAPP_URI_MAX_NAME_LENGHT
     * @return Pointer to the URI list.
     */
    uriGet_callback_t           uriGetList_clb;


    /**
     * @brief Device name group identifier for network pairing.
     *
     * @details
     * Pointer to the device name group string used for network pairing.
     *
     * This string identifies the device on the OpenThread network and filters pairing –
     * only devices with a matching name group can pair together.
     *
     * Enables logical grouping of devices (e.g., by room or function).
     *
     * **Buffer Properties:**
     * - Size: OTAPP_DEVICENAME_SIZE (10 bytes)
     * - Maximum string length: 9 characters + null terminator
     *
     * The buffer for this pointer is allocated and initialized by the application.
     * In the reference implementation, it is set in the ad_button_Init() function,
     * which copies the provided group name into an internal static buffer and assigns
     * its address to this field (example initialization).
     *
     * @note Content should be set once during initialization.
     * @note Length must not exceed OTAPP_DEVICENAME_SIZE - 1 characters.
     *
     * @see ad_button_Init() for example initialization.
     * @see OTAPP_DEVICENAME_SIZE for buffer size definition.
     */
    char *deviceName;

    /**
     * @brief Pointer to the device type identifier.
     *
     * @details
     * Points to a constant device type identifier used to classify the device.
     * This type typically indicates the device category or role within the OpenThread network,
     * such as switch, light, sensor, etc.
     *
     * The device type helps the pairing rules and other network management logic
     * determine the kind of device and apply appropriate filters or actions.
     *
     * The pointer refers to a static, application-defined constant,
     * for example, OTAPP_SWITCH for a button or switch device.
     *
     * The type otapp_deviceType_t represents the full supported list of device types
     * recognized by the framework, allowing classification across all supported device categories.
     *
     * @note The pointed-to data should remain constant and valid
     *       for the entire lifetime of the device.
     *
     * @see OTAPP_SWITCH device type example in reference implementation.
     * @see otapp_deviceType_t for full supported list of device types.
     */
    const otapp_deviceType_t *deviceType;

    /**
     * @brief Number of URIs in the URI list.
     *
     * @details
     * Returns the count of URIs supported by the device, corresponding to the
     * size of the array returned by uriGetList_clb callback.
     *
     * This value informs the framework how many CoAP resource endpoints
     * the device exposes for network interaction and remote control.
     *
     * Used during device registration and pairing to properly allocate
     * resources for URI handling and subscription management.
     *
     * @note Maximum value limited by OTAPP_PAIRED_URI_MAX
     * @note Must match the actual size of URI list returned by uriGetList_clb
     *
     * @see uriGetList_clb for the URI list callback
     * @see OTAPP_PAIRED_URI_MAX for maximum URI count limit
     */
    ot_app_size_t               uriGetListSize;

    /**
     * @brief Main task callback for periodic device processing.
     *
     * @details
     * Pointer to the main task function that should be called periodically
     * from the main loop or RTOS task to process device events and state updates.
     *
     * This callback handles non-blocking processing of:
     * - Button events and user input
     * - Internal state updates
     * - Communication with paired devices
     * - Any device-specific periodic maintenance
     *
     * The framework calls this function regularly (e.g., every main loop iteration)
     * to keep the device responsive and process pending work.
     *
     * **Example usage in reference implementation:**
     * ```
     * void ad_button_task(void)
     * {
     *     ad_btn_task();  // Processes button events
     * }
     * ```
     * Set during initialization: `drv->task = ad_button_task;` [file:1]
     *
     * @note Function must be non-blocking and return immediately
     * @note Should contain minimal processing to avoid delaying other tasks
     *
     * @see ad_button_task() for reference implementation example
     */
    mainTask_callback_t     task;
    
    /**
     * @brief Application device driver API structure.
     *
     * @details
     * This structure contains function pointers that define the full API for interacting
     * with the OpenThread device driver. It provides organized access to key subsystems:
     *
     * - **Pairing API (`api.pair`)**
     * - **Device Name API (`api.devName`)**
     * - **Non-Volatile Storage API (`api.nvs`)**
     * - **CoAP API (`api.coap`)**
     * 
     * This centralized API structure aids modular and flexible device driver design,
     * allowing easy interaction with all key OpenThread application features.
     *
     * Internal callbacks such as `obs_pairedDevice_clb`, `obs_subscribedUri_clb`,
     * and pairing rules callback `pairRuleGetList_clb` are maintained here alongside
     * device properties (`deviceName`, `deviceType`), URI management, and main task pointer.
     *
     * The instance is statically allocated and accessed via `ot_app_drv_getInstance()`.
     *
     * @note Functions must be properly initialized to avoid NULL pointer dereferences.
     * @note This API encapsulates both synchronous and asynchronous device behaviors.
     *
     * @see ot_app_drv_getInstance() for singleton access.
     */
    ot_app_devDrvAPI_t api;
}ot_app_devDrv_t;

/*
* @}
*/
ot_app_devDrv_t *ot_app_drv_getInstance(void);
void ot_app_drv_task(void);

#endif  /* OT_APP_DRV_H_ */
