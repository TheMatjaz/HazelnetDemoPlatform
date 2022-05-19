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
 * Periodic timer expires after #HZL_PLATFORM_TX_TIMER_TICKS ticks and restarts
 * once checked.
 */

#include "hzlPlatform.h"
#include "hzlPlatform_FatalError.h"

static TaskHandle_t taskToNotifyOnExpiration = NULL;

/**
 * @internal
 * Sets the event bit for the TX timer expiration to the event bitmap of the task-to-notify.
 */
static void
hzlPlatform_CallbackOnTxTimerExpiration(TimerHandle_t whichTimerExpiredHandle)
{
    (void) whichTimerExpiredHandle;
    // eSetBits: The task's notification value is bitwise ORed with ulValue.
    // The function always returns pdPASS in this case.
    xTaskNotifyFromISR(
        taskToNotifyOnExpiration,
        HZL_PLATFORM_TASK_EVENT_TX_TIMER_EXPIRED,
        eSetBits,
        NULL // pxHigherPriorityTaskWoken
        );
}

void
hzlPlatform_PeriodicTxTimerInit(TaskHandle_t taskToNotify)
{
    taskToNotifyOnExpiration = taskToNotify;
    uint32_t txTimerIdentifier = 0x11U;
    TimerHandle_t timerHandle = xTimerCreate(
            "hzl_tx_timer",
            HZL_PLATFORM_TX_TIMER_TICKS,
            true, // Do autoreload, make it a periodic timer
            &txTimerIdentifier,
            hzlPlatform_CallbackOnTxTimerExpiration
            );
    if (timerHandle == NULL)
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_TXTIMER_CREATE);
    }
    const BaseType_t timerErr = xTimerStart(timerHandle, 0);
    if (timerErr != pdPASS)
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_TXTIMER_START);
    }
}
