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
 * Macros for the unrecoverable error case RGB LED colors and the function that uses them.
 *
 * Hooks used by FreeRTOS are also declared here in order to make them available at least once
 * in the overall codebase to make the linker happy.
 */

#ifndef HZL_PLATFORM_FATALERROR_H_
#define HZL_PLATFORM_FATALERROR_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <hzlPlatform_RgbLed.h>

// RGB LED colors used by the hzlPlatform_FatalCrashAlternating() function
// Initialisation phase
#define HZL_PLATFORM_CRASH_CLOCK              HZL_PLATFORM_RGB_COLOR_RED, HZL_PLATFORM_RGB_COLOR_BLACK
#define HZL_PLATFORM_CRASH_CSEC               HZL_PLATFORM_RGB_COLOR_RED, HZL_PLATFORM_RGB_COLOR_GREEN
#define HZL_PLATFORM_CRASH_CANFD_INIT         HZL_PLATFORM_RGB_COLOR_RED, HZL_PLATFORM_RGB_COLOR_BLUE
#define HZL_PLATFORM_CRASH_OUT_OF_MEMORY      HZL_PLATFORM_RGB_COLOR_RED, HZL_PLATFORM_RGB_COLOR_CYAN
#define HZL_PLATFORM_CRASH_STACK_OVERFLOW     HZL_PLATFORM_RGB_COLOR_RED, HZL_PLATFORM_RGB_COLOR_MAGENTA
#define HZL_PLATFORM_CRASH_RTOS_TASK_CREATION HZL_PLATFORM_RGB_COLOR_RED, HZL_PLATFORM_RGB_COLOR_YELLOW
#define HZL_PLATFORM_CRASH_RTOS_TERMINATED    HZL_PLATFORM_RGB_COLOR_RED, HZL_PLATFORM_RGB_COLOR_WHITE
#define HZL_PLATFORM_CRASH_TXTIMER_CREATE     HZL_PLATFORM_RGB_COLOR_MAGENTA, HZL_PLATFORM_RGB_COLOR_RED
#define HZL_PLATFORM_CRASH_TXTIMER_START      HZL_PLATFORM_RGB_COLOR_MAGENTA, HZL_PLATFORM_RGB_COLOR_GREEN
#define HZL_PLATFORM_CRASH_CANFD_DEINIT       HZL_PLATFORM_RGB_COLOR_MAGENTA, HZL_PLATFORM_RGB_COLOR_BLUE

// IO critical failures operations
#define HZL_PLATFORM_CRASH_CANFD_TX           HZL_PLATFORM_RGB_COLOR_YELLOW, HZL_PLATFORM_RGB_COLOR_RED
#define HZL_PLATFORM_CRASH_CANFD_RX           HZL_PLATFORM_RGB_COLOR_YELLOW, HZL_PLATFORM_RGB_COLOR_BLUE
#define HZL_PLATFORM_CRASH_CSEC_RNG_INIT      HZL_PLATFORM_RGB_COLOR_YELLOW, HZL_PLATFORM_RGB_COLOR_GREEN

// Hazelnet library critical failures
#define HZL_PLATFORM_CRASH_HZL_INIT           HZL_PLATFORM_RGB_COLOR_BLUE, HZL_PLATFORM_RGB_COLOR_RED
#define HZL_PLATFORM_CRASH_HZL_BUILD_REQUEST  HZL_PLATFORM_RGB_COLOR_BLUE, HZL_PLATFORM_RGB_COLOR_GREEN
#define HZL_PLATFORM_CRASH_HZL_DEINIT         HZL_PLATFORM_RGB_COLOR_BLUE, HZL_PLATFORM_RGB_COLOR_MAGENTA
#define HZL_PLATFORM_CRASH_HZL_RX             HZL_PLATFORM_RGB_COLOR_BLUE, HZL_PLATFORM_RGB_COLOR_YELLOW
#define HZL_PLATFORM_CRASH_HZL_BUILD_UAD      HZL_PLATFORM_RGB_COLOR_BLUE, HZL_PLATFORM_RGB_COLOR_WHITE
#define HZL_PLATFORM_CRASH_HZL_BUILD_RENEWAL  HZL_PLATFORM_RGB_COLOR_BLUE, HZL_PLATFORM_RGB_COLOR_CYAN

/**
 * Reports an unrecoverable error with the RGB LED alternating flashes between 2 colors
 * looping forever. The first color stays active longer than the second. This function may
 * be used even before the RTOS or the clock are initialised, as it uses busy-wait loops instead
 * of RTOS or clock waits.
 *
 * It must be used AFTER the RGB LED pins have been initialised.
 */
void hzlPlatform_FatalCrashAlternating(hzlPlatform_RgbColor_t longer,
                                            hzlPlatform_RgbColor_t shorter);


// FreeRTOS hooks used in case of errors.
void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);
void vApplicationIdleHook(void);
void vApplicationTickHook(void);

#ifdef __cplusplus
}
#endif

#endif  /* HZL_PLATFORM_FATALERROR_H_ */
