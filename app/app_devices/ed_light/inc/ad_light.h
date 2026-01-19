/**
 * @file ad_light.h
 * @author Jan Łukaszewicz
 * @brief High‑level OpenThread RGB light device implementation.
 * @version 0.1
 * @date 06-09-2025
 *
 * @defgroup device_light Light Device
 * @ingroup devices
 * @{
 *
 * @section device_light_description Overview
 *
 * The Light Device module turns the OpenThread Application Framework driver
 * (@ref ot_app_device_api) into a concrete RGB light node.
 *
 * Responsibilities:
 * - Owns the device name group used for pairing (room / zone identifier).
 * - Configures the driver instance with:
 *   - Light device type (`OTAPP_LIGHTING` family),
 *   - Pairing rules (which remote devices are allowed),
 *   - Observer callbacks for paired devices and subscribed URIs,
 *   - URI list and URI count for this device.
 * - Initializes the URI layer (@ref device_light_uri) and hardware layer
 *   (@ref device_light_control).
 * - Exposes a main task function (in `ad_light.c`) that is called from
 *   @ref ot_app_drv_task() to process device‑specific logic.
 *
 * Typical lifecycle:
 * - `ad_light_init()` is called once from `app_main()` with a device
 *   name group string (for example `"kitchen"` or `"device1"`).
 * - The function obtains the singleton driver, initializes NVS and URIs,
 *   and registers callbacks.
 * - The application main loop periodically calls `ot_app_drv_task()`,
 *   which in turn calls the light device task.
 *
 * Integration points:
 * - CoAP/URI integration: @ref device_light_uri
 * - Hardware control: @ref device_light_control
 * - Pairing API and observers: @ref ot_app_pair.h, @ref ot_app_coap_uri_obs.h
 */


#ifndef AD_LIGHT_H_
#define AD_LIGHT_H_

/**
 * @brief Initialize the RGB light device and register it in the framework.
 *
 * This function performs all one‑time initialization required to run
 * the light device as an OpenThread application node:
 *
 * - Validates and stores the @p deviceNameGroup string in an internal buffer.
 *   This string defines the **device name group** used for pairing and must
 *   match the group used by controller devices that are allowed to control
 *   this light.
 * - Obtains the singleton driver instance via `ot_app_drv_getInstance()`.
 * - Ensures that the NVS subsystem is initialized by calling
 *   `drv->api.nvs.init()`. Without this step, pairing data and other
 *   persistent state cannot be stored.
 * - Initializes the light URI table by calling @ref ad_light_uri_init(),
 *   then assigns:
 *   - `drv->uriGetList_clb`     – returns the URI list,
 *   - `drv->uriGetListSize`     – returns the URI count.
 * - Configures pairing behavior by setting `drv->pairRuleGetList_clb`
 *   to one of the light‑specific rule providers (all allowed / blocked / filtered).
 * - Registers observer callbacks:
 *   - `drv->obs_pairedDevice_clb`     – called when a new device is paired,
 *   - `drv->obs_subscribedUri_clb`    – called when a subscribed URI changes.
 * - Sets:
 *   - `drv->deviceName` to point to the internal name‑group buffer,
 *   - `drv->deviceType` to the constant light device type,
 *   - `drv->task` to the light main task function.
 *
 * @param deviceNameGroup Pointer to a null‑terminated string containing
 *        the device name group identifier (max 9 characters).
 *
 * @note Must be called exactly once before the main loop starts.
 * @note Pointer @p deviceNameGroup must not be NULL.
 * @note The same group string must be used on controller devices to allow
 *       automatic pairing with this light.
 */
void ad_light_init(char *deviceNameGroup);

/** @} */ /* end of group device_light */

#endif /* AD_LIGHT_H_ */
