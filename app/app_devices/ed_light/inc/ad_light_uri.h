/**
 * @file ad_light_uri.h
 * @author Jan Łukaszewicz
 * @brief CoAP URI definitions for the light device.
 * @version 0.1
 * @date 13-11-2025
 *
 * @defgroup device_light_uri Light Device URIs
 * @ingroup device_light
 * @{
 *
 * This module defines the CoAP resources that expose the light device
 * functionality to the network. Typical endpoints:
 * - `light/state`       – on/off control (boolean or 0/1 value)
 * - `light/brightness`  – dimming level (0..255)
 * - `light/color`       – RGB color (packed value or 3‑byte payload)
 *
 * For each URI the module configures:
 * - Path string and CoAP method (GET/PUT/POST as needed),
 * - Handler callback function,
 * - Optional observe capability for state change notifications.
 *
 * The URI table is registered in the OpenThread Application Framework
 * via the driver callbacks configured in @ref ad_light_init().
 */


#ifndef AD_LIGHT_URI_H_
#define AD_LIGHT_URI_H_

#include "ot_app_drv.h"

/**
 * @brief Get the static list of CoAP URIs exposed by the light device.
 *
 * The returned pointer refers to a statically allocated array of
 * @ref otapp_coap_uri_t structures. The array describes all URIs handled
 * by this device, including their handlers and observe settings.
 *
 * @return Pointer to the first element of the URI list.
 *
 * @note The lifetime of the returned array is the entire runtime
 *       of the application; it must not be freed or modified by the caller.
 */
otapp_coap_uri_t *ad_light_uri_getList(void);


/**
 * @brief Get the number of URIs provided by the light device.
 *
 * This value must match the size of the array returned by
 * @ref ad_light_uri_getList() and is used by the framework to
 * iterate over the URI table.
 *
 * @return Number of URIs in the list.
 */
uint8_t ad_light_uri_getListSize(void);


/**
 * @brief Initialize light URIs and connect them to the driver API.
 *
 * This function should be called during device initialization
 * (from @ref ad_light_init()). It:
 * - Fills the static URI table with paths, methods and handler callbacks.
 * - Optionally configures observe support for stateful resources
 *   such as `light/state` or `light/brightness`.
 * - Stores a pointer to the driver instance @p devDrv for use
 *   inside URI handlers when sending responses or notifications.
 *
 * @param devDrv Pointer to the OpenThread application driver instance.
 *        Must be a valid pointer returned by ot_app_drv_getInstance().
 *
 * @note After calling this function, the application should assign
 *       `drv->uriGetList_clb` and `drv->uriGetListSize` to
 *       @ref ad_light_uri_getList and @ref ad_light_uri_getListSize
 *       respectively (this is done inside @ref ad_light_init()).
 */
void ad_light_uri_init(ot_app_devDrv_t *devDrv);

/** @} */ /* end of group device_light_uri */

#endif /* AD_LIGHT_URI_H_ */
