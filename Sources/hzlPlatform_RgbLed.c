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
 * Implementation of the RBG LED for S32K144.
 */

#include <hzlPlatform_RgbLed.h>
#include "pins_driver.h"

#define RGB_LED_GPIO PTD
#define RGB_LED_PORT PORTD
#define RGB_LED_PIN_RED 15U
#define RGB_LED_PIN_GREEN 16U
#define RGB_LED_PIN_BLUE 0U

static pins_channel_type_t colorToPins(const hzlPlatform_RgbColor_t color)
{
    pins_channel_type_t pins = 0U;
    if (color & HZL_PLATFORM_RGB_COLOR_RED)
    {
        pins |= 1U << RGB_LED_PIN_RED;
    }
    if (color & HZL_PLATFORM_RGB_COLOR_GREEN)
    {
        pins |= 1U << RGB_LED_PIN_GREEN;
    }
    if (color & HZL_PLATFORM_RGB_COLOR_BLUE)
    {
        pins |= 1U << RGB_LED_PIN_BLUE;
    }
    return pins;
}

#define PIN_DIRECTION_OUTPUT 1

uint32_t hzlPlatform_RgbLedInit(void* const ctx)
{
    (void) ctx;
    PINS_DRV_SetMuxModeSel(RGB_LED_PORT, RGB_LED_PIN_RED, PORT_MUX_AS_GPIO);
    PINS_DRV_SetMuxModeSel(RGB_LED_PORT, RGB_LED_PIN_GREEN, PORT_MUX_AS_GPIO);
    PINS_DRV_SetMuxModeSel(RGB_LED_PORT, RGB_LED_PIN_BLUE, PORT_MUX_AS_GPIO);
    // Set the pins of the RGB LED to output pins.
    PINS_DRV_SetPinDirection(RGB_LED_GPIO, RGB_LED_PIN_RED, PIN_DIRECTION_OUTPUT);
    PINS_DRV_SetPinDirection(RGB_LED_GPIO, RGB_LED_PIN_GREEN, PIN_DIRECTION_OUTPUT);
    PINS_DRV_SetPinDirection(RGB_LED_GPIO, RGB_LED_PIN_BLUE, PIN_DIRECTION_OUTPUT);
    hzlPlatform_RgbLedTurnOff();
    return 0;
}

void hzlPlatform_RgbLedSetColor(const hzlPlatform_RgbColor_t color)
{
    // The logical negation is required, because setting the pin to high (true)
    // pulls the voltage on the RGB LED to ground, deactivating it.
    PINS_DRV_WritePin(RGB_LED_GPIO, RGB_LED_PIN_RED,
        !(color & HZL_PLATFORM_RGB_COLOR_RED));
    PINS_DRV_WritePin(RGB_LED_GPIO, RGB_LED_PIN_GREEN,
        !(color & HZL_PLATFORM_RGB_COLOR_GREEN));
    PINS_DRV_WritePin(RGB_LED_GPIO, RGB_LED_PIN_BLUE,
        !(color & HZL_PLATFORM_RGB_COLOR_BLUE));
}

void hzlPlatform_RgbLedAddPrimaryColors(hzlPlatform_RgbColor_t colors)
{
    // Clearing RGB LED pins activates the color, because we stop pulling the
    // RGB LED voltage to ground.
    PINS_DRV_ClearPins(RGB_LED_GPIO, colorToPins(colors));
}

void hzlPlatform_RgbLedRemovePrimaryColors(hzlPlatform_RgbColor_t colors)
{
    // Setting RGB LED pins deactivates the color, because we start pulling the
    // RGB LED voltage to ground.
    PINS_DRV_SetPins(RGB_LED_GPIO, colorToPins(colors));
}

void hzlPlatform_RgbLedToggleColor(const hzlPlatform_RgbColor_t color)
{
    PINS_DRV_TogglePins(RGB_LED_GPIO, colorToPins(color));
}
