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
 * Setup and callback with event for the main task on Button 1 (SW3 on the eval board)
 * being pressed.
 */

#include "hzlPlatform.h"
#include "hzlPlatform_FatalError.h"

#include "pins_driver.h"

// Configuration of the GPIO pins related to the Button 1
// See g_pin_mux_InitConfigArr[], entry with pinPortIdx==13U
#define BUTTONS_1_2_GPIO PTC
#define BUTTONS_1_2_PORT PORTC
#define BUTTONS_1_2_PORT_PCC PCC_PORTC_CLOCK
#define BUTTONS_1_2_PORT_IRQn PORTC_IRQn
#define BUTTON1_PIN 13U
#define BUTTON2_PIN 12U

static TaskHandle_t taskToNotifyOnButtonPress = NULL;

/**
 * @internal
 * Sets the event bit for the Button 1 or 2 pressed to the event bitmap of the task-to-notify.
 *
 * Note: no debouncing is applied. As long as the button is not broken and it reports being pressed
 * when it's not, there is no need to verify the de-pressing of the button correctly, as the usage
 * of the buttons is not critical in any case.
 */
static void
hzlPlatform_CallbackOnButtonsPress(void)
{
    const pins_channel_type_t highPinsBitmap = PINS_DRV_ReadPins(BUTTONS_1_2_GPIO);
    if (highPinsBitmap & (1U << BUTTON1_PIN))
    {
        xTaskNotifyFromISR(
            taskToNotifyOnButtonPress,
            HZL_PLATFORM_TASK_EVENT_BUTTON_1_PRESSED,
            eSetBits, // The task's notification value is bitwise ORed with ulValue.
            NULL  // pxHigherPriorityTaskWoken: not time critical if someone was waiting for this event
            );
    }
    if (highPinsBitmap & (1U << BUTTON2_PIN))
    {
        xTaskNotifyFromISR(
            taskToNotifyOnButtonPress,
            HZL_PLATFORM_TASK_EVENT_BUTTON_2_PRESSED,
            eSetBits, // The task's notification value is bitwise ORed with ulValue.
            NULL  // pxHigherPriorityTaskWoken: not time critical if someone was waiting for this event
            );
    }
    PINS_DRV_ClearPortIntFlagCmd(BUTTONS_1_2_PORT);
}

void
hzlPlatform_Button1And2Init(TaskHandle_t taskToNotify)
{
    taskToNotifyOnButtonPress = taskToNotify;
    PINS_DRV_SetMuxModeSel(BUTTONS_1_2_PORT, BUTTON1_PIN, PORT_MUX_AS_GPIO);
    PINS_DRV_SetMuxModeSel(BUTTONS_1_2_PORT, BUTTON2_PIN, PORT_MUX_AS_GPIO);
    // Rising edge = when pressed down = voltage goes from low to high
    PINS_DRV_SetPinIntSel(BUTTONS_1_2_PORT, BUTTON1_PIN, PORT_INT_RISING_EDGE);
    PINS_DRV_SetPinIntSel(BUTTONS_1_2_PORT, BUTTON2_PIN, PORT_INT_RISING_EDGE);
    // Buttons direction is set to input: 0=input, 1=output, thus we have to negate the bitmap.
    const pins_channel_type_t inputPinsBitmap = (1U << BUTTON1_PIN) | (1U << BUTTON2_PIN);
    PINS_DRV_SetPinsDirection(BUTTONS_1_2_GPIO, ~inputPinsBitmap);
    // Callback on button press
    INT_SYS_InstallHandler(BUTTONS_1_2_PORT_IRQn, hzlPlatform_CallbackOnButtonsPress, NULL);
    // Enable Button interrupt handler
    INT_SYS_EnableIRQ(BUTTONS_1_2_PORT_IRQn);
    // The interrupt calls an interrupt safe API function - so its priority must
    // be equal to or lower than configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY.
    INT_SYS_SetPriority(BUTTONS_1_2_PORT_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
}
