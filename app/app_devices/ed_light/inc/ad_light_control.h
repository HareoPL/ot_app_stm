/**
 * @file ad_light_control.h
 * @author Jan Łukaszewicz
 * @brief Low‑level RGB light control for WS2812B LEDs.
 * @version 0.1
 * @date 13-11-2025
 *
 * @defgroup device_light_control Light Device Control
 * @ingroup device_light
 * @{
 *
 * This module encapsulates the **local light state** and the functions
 * that translate abstract light commands (on/off, dim, color) into calls
 * to the WS2812B effect library.
 *
 * Internal state:
 * - Base RGB value stored as three 8‑bit channels.
 * - Global dimming value stored as 32‑bit scalar (0..255).
 * - Pre‑computed dimmed RGB values used when light is on and dim > 0.
 *
 * High‑level behavior:
 * - Color can be changed independently from brightness.
 * - Dimming scales the current color without changing its base value.
 * - On/Off toggles between applying the current (possibly dimmed) color
 *   and turning all LEDs off.
 *
 * This module does **not** know anything about CoAP or pairing – it is
 * purely hardware‑facing and is used by URI handlers in @ref device_light_uri.
 */


#ifndef AD_LIGHT_CONTROL_H_
#define AD_LIGHT_CONTROL_H_

#include "ot_app_drv.h"

/**
 * @brief Turn the RGB light on or off.
 *
 * Logic:
 * - If @p ledState is non‑zero:
 *   - When the stored dim level is 0, the raw base RGB color is sent
 *     to WS2812B.
 *   - When dim > 0, the pre‑computed dimmed RGB is sent (scaled by dim).
 * - If @p ledState is 0:
 *   - All LEDs are forced to black (off) regardless of dim or color.
 *
 * This function does not change the stored color or dim value – it only
 * applies the current state to the LEDs.
 *
 * @param ledState 0 to turn the light off, non‑zero to turn it on.
 */
void ad_light_ctr_onOff(uint32_t ledState);


/**
 * @brief Set global brightness (dimming) level for the RGB light.
 *
 * Steps:
 * - Stores @p dimValue in the internal state.
 * - Recalculates dimmed RGB components based on the current base color.
 * - Immediately applies the scaled color to WS2812B, keeping the on/off
 *   state consistent with the last call to @ref ad_light_ctr_onOff().
 *
 * Typical usage:
 * - Called when a CoAP `light/brightness` URI is updated.
 *
 * @param dimValue Dimming value in the range 0..255.
 *        0 is treated as „no brightness” and is handled specially
 *        in @ref ad_light_ctr_onOff().
 */
void ad_light_ctr_dimSet(uint32_t dimValue);


/**
 * @brief Set base RGB color for the light.
 *
 * Steps:
 * - Updates the stored base RGB components with @p r, @p g and @p b.
 * - If a non‑zero dim value is currently active:
 *   - Recalculates and applies dimmed RGB values (color * dim / 255).
 * - If dim is 0:
 *   - Immediately applies the raw RGB color to the LEDs.
 *
 * This allows the application to:
 * - Change color at constant brightness (dim != 0),
 * - Or change color without dimming (dim == 0).
 *
 * @param r Red component (0..255).
 * @param g Green component (0..255).
 * @param b Blue component (0..255).
 */
void ad_light_ctr_colorSet(uint8_t r, uint8_t g, uint8_t b);


/**
 * @brief Initialize the light control module and bind it to a driver instance.
 *
 * Responsibilities:
 * - Validates @p devDrv pointer (returns immediately if NULL).
 * - Stores the driver pointer for later use (e.g. future NVS load/save
 *   of brightness and color, if implemented).
 * - Initializes the internal light state with default values
 *   (e.g. low‑intensity blue as startup color).
 *
 * This function should be called from the light device initialization path
 * before any other control function is used.
 *
 * @param devDrv Pointer to the OpenThread application driver instance.
 *        Must be the same instance as used in @ref ad_light_init().
 */
void ad_light_ctr_init(ot_app_devDrv_t *devDrv);

/** @} */ /* end of group  */
#endif /* AD_LIGHT_CONTROL_H_ */
