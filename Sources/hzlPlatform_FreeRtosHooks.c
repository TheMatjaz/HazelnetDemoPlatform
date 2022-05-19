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
 * @internal
 * Implementation of hooks called upon critical errors, idle moments or just
 * dummy empty hooks to keep the linker happy.
 *
 * Code and documentation taken from the S32K144 FreeRTOS example.
 */

#include "hzlPlatform.h"
#include "hzlPlatform_FatalError.h"

/**
 * @internal
 * Do NOPs for the given amount of cycles. This is way that works even in case the RTOS is not
 * started yet, i.e. before the clocks and vTaskDelay would be available.
 */
static void
hzlPlatform_SpinWaitCycles(volatile size_t cycles)
{
    while (cycles--)
    {
        NOP();
    }
}

void
hzlPlatform_FatalCrashAlternating(const hzlPlatform_RgbColor_t longer,
                                       const hzlPlatform_RgbColor_t shorter)
{
    taskDISABLE_INTERRUPTS();
    hzlPlatform_RgbLedTurnOff();
    while (1)
    {
        // Stuck forever. We use spin waits in order to allow this function to be called
        // even without the RTOS or even clocks initialised.
        hzlPlatform_RgbLedSetColor(longer);
        hzlPlatform_SpinWaitCycles(6000000);
        hzlPlatform_RgbLedSetColor(shorter);
        hzlPlatform_SpinWaitCycles(1500000);
    }
}

/**
 * @internal
 * Called if a call to pvPortMalloc() fails because there is insufficient
 * free memory available in the FreeRTOS heap.  pvPortMalloc() is called
 * internally by FreeRTOS API functions that create tasks, queues, software
 * timers, and semaphores.  The size of the FreeRTOS heap is set by the
 * configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
 */
void
vApplicationMallocFailedHook(void)
{
    hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_OUT_OF_MEMORY);
}

/**
 * @internal
 * Run time stack overflow checking is performed if
 * configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
 * function is called if a stack overflow is detected.
 * @param [in] pxTask unused
 * @param [in] pcTaskName unused
 */
void
vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void) pcTaskName;
    (void) pxTask;
    hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_STACK_OVERFLOW);
}

/**
 * @internal
 * This function is called on each cycle of the idle task.  In this case it
 * does nothing useful, other than report the amount of FreeRTOS heap that
 * remains unallocated.
 */
void
vApplicationIdleHook(void)
{
    // TODO [MG] do we really need to have something in this idle hook?
    volatile size_t xFreeHeapSpace = xPortGetFreeHeapSize();
    if (xFreeHeapSpace > 100)
    {
        /* By now, the kernel has allocated everything it is going to, so
         if there is a lot of heap remaining unallocated then
         the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
         reduced accordingly. */
    }
}

/**
 * @internal
 * The FreeRTOSConfig.h is a shared configuration across many example project,
 * some of them (but not this one) have a run time stats gathering function.
 * To keep the linker happy, this is a dummy stats function, just like
 * ulMainGetRunTimeCounterValue();
 */
void
vMainConfigureTimerForRunTimeStats(void)
{
}

/**
 * @internal
 * The FreeRTOSConfig.h is a shared configuration across many example project,
 * some of them (but not this one) have a run time stats gathering function.
 * To keep the linker happy, this is a dummy stats function, just like
 * vMainConfigureTimerForRunTimeStats().
 */
unsigned long
ulMainGetRunTimeCounterValue(void)
{
    return 0;
}

/**
 * @internal
 */
void
vApplicationTickHook(void)
{
}
