/*
 * Copyright © 2020-2022, Matjaž Guštin <dev@matjaz.it>
 * <https://matjaz.it>. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of nor the names of its contributors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS “AS IS”
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * Abstract API to control the RGB LED of the board based on the color only.
 *
 * The interface is completely independent of the hardware.
 * With this API the RGB LED can be simply configured to a
 * given color. No intensity control is available.
 */

#ifndef HZL_PLATFORM_RGB_LED_H_
#define HZL_PLATFORM_RGB_LED_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/**
 * Primary and secondary colors of the Red-Green-Blue color model.
 */
typedef enum hzlPlatform_RgbColor
{
    HZL_PLATFORM_RGB_COLOR_BLACK = 0U,
    HZL_PLATFORM_RGB_COLOR_RED = 1U,
    HZL_PLATFORM_RGB_COLOR_GREEN = 2U,
    HZL_PLATFORM_RGB_COLOR_BLUE = 4U,
    HZL_PLATFORM_RGB_COLOR_CYAN = (HZL_PLATFORM_RGB_COLOR_GREEN
            | HZL_PLATFORM_RGB_COLOR_BLUE),
    HZL_PLATFORM_RGB_COLOR_MAGENTA = (HZL_PLATFORM_RGB_COLOR_RED
            | HZL_PLATFORM_RGB_COLOR_BLUE),
    HZL_PLATFORM_RGB_COLOR_YELLOW = (HZL_PLATFORM_RGB_COLOR_RED
            | HZL_PLATFORM_RGB_COLOR_GREEN),
    HZL_PLATFORM_RGB_COLOR_WHITE = (HZL_PLATFORM_RGB_COLOR_RED
            | HZL_PLATFORM_RGB_COLOR_GREEN
            | HZL_PLATFORM_RGB_COLOR_BLUE),
} hzlPlatform_RgbColor_t;

/**
 * Initialisation of the RGB LED.
 *
 * This is completely hardware dependant, so it's left for the implementation
 * to handle.
 *
 * In general it should be called AFTER PINS_DRV_Init().
 *
 * @param [in, out] ctx any data structure that may be handy to pass
 * to the initialisation function. Can be NULL.
 * @returns 0 in case of success, non-zero in case of errors.
 */
uint32_t hzlPlatform_RgbLedInit(void* ctx);

/**
 * Switches on the RGB LED to the set color.
 *
 * If the color was already active, nothing happens to it.
 *
 * To deactivate completely the RGB LED the #HZL_PLATFORM_RGB_COLOR_BLACK value
 * should be passed - calling hzlPlatform_RgbLedTurnOff() also works.
 *
 * This function has to be called after hzlPlatform_RgbLedInit().
 *
 * @param [in] color bitmap of red, green, blue colors.
 */
void hzlPlatform_RgbLedSetColor(hzlPlatform_RgbColor_t color);

/**
 * Turns on the specified primary color components (red, green or blue).
 *
 * Not specified primary color components remain unchanged. If the component
 * was already activated, nothing happens to it.
 *
 * This function has to be called after hzlPlatform_RgbLedInit().
 *
 * @param [in] color bitmap of red, green, blue colors.
 */
void hzlPlatform_RgbLedAddPrimaryColors(hzlPlatform_RgbColor_t color);

/**
 * Turns off the specified primary color components (red, green or blue).
 *
 * Not specified primary color components remain unchanged. If the component
 * was already deactivated, nothing happens to it.
 *
 * This function has to be called after hzlPlatform_RgbLedInit().
 *
 * @param [in] color bitmap of red, green, blue colors.
 */
void hzlPlatform_RgbLedRemovePrimaryColors(hzlPlatform_RgbColor_t color);

/**
 * Turns off all colors of the RGB LED.
 */
#define hzlPlatform_RgbLedTurnOff() hzlPlatform_RgbLedSetColor(HZL_PLATFORM_RGB_COLOR_BLACK)

/**
 * Toggles the state of the specified colors of the RGB LED.
 *
 * If the specified colors were off, they are turned on and vice-versa. Not
 * specified colors remain unchanged.
 *
 * This function has to be called after hzlPlatform_RgbLedInit().
 *
 * @param [in] color bitmap of red, green, blue colors.
 */
void hzlPlatform_RgbLedToggleColor(hzlPlatform_RgbColor_t color);

#ifdef __cplusplus
}
#endif

#endif  /* HZL_PLATFORM_RGB_LED_H_ */
