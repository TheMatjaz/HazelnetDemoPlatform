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
 * Initialisation of the FreeRTOS operating system, including the minimum hardware required for it
 * and starting of the tasks.
 *
 * In this demo the multi-memory-region scheme is used, thus heap5.c is selected behind the scenes.
 */

#include <hzlPlatform_RgbLed.h>
#include "hzlPlatform.h"
#include "hzlPlatform_FatalError.h"

// ------------- FreeRTOS multi-region RAM setting (2 physical slots) -----------------

// Tell FreeRTOS to use a specific memory scheme, in these case
// the multi-region RAM.
static const uint8_t freeRTOSMemoryScheme = 5;

// These values are provided by the linker (See the linker script in Project_Settings/Linker_Files)
// to let FreeRTOS know where the heap memory region is located. The high and low sections exists
// because we are using the multi-region RAM (2 physical slots) and the heap cannot be contiguous
// i.e. no arrays of data may "jump" over the gap between the two sections. Basically we have two
// fully independent mempools for the heap.
extern uint8_t __heap_high_start__;
extern uint8_t __heap_high_size__;
extern uint8_t __heap_low_start__;
extern uint8_t __heap_low_size__;

// Struct used by FreeRTOS heap management for multiple RAM regions.
// Has to be volatile, otherwise the linker may remove it.
volatile HeapRegion_t xHeapRegions[] =
{
    {(uint8_t *) 0UL, 0UL},
    {(uint8_t *) 0UL, 0UL},
    {NULL, 0} // Termination of the array.
};


// ------------- Hardware initialisation required for FreeRTOS -----------------

/**
 * @internal Configures the system clock.
 * Must be called BEFORE hzlPlatform_InitFreeRtosPins().
 */
static void
hzlPlatform_InitFreeRtosClock(void)
{
    const status_t status = CLOCK_DRV_Init(&clockMan1_InitConfig0);
    if (status != STATUS_SUCCESS)
    {
        while(1)
        {
            // Fatal crash. Stuck forever.
            // Cannot use the LED, as it was not initialised yet and to initialise it we
            // require the clock.
            NOP();
        }
    }
}

/**
 * @internal Configures the system pins, including the RGB LED ones.
 * Must be called AFTER hzlPlatform_InitFreeRtosClock()
 */
static void
hzlPlatform_InitFreeRtosPins(void)
{
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);
    hzlPlatform_RgbLedInit(NULL);
}

/**
 * @internal
 * Configures FreeRTOS to use multiple physically-separated memory regions.
 * Must be called BEFORE any heap usage, such as task creation.
 */
static void
hzlPlatform_InitFreeRtosMulipleRamRegions(void)
{
    // Make sure the freeRTOSMemoryScheme is used at least once to guarantee the linker
    // does not optimise it out. Apparently using "volatile" does not help in this case.
    // The value written in pucStartAddess is anyway overwritten in the next line.
    xHeapRegions[0].pucStartAddress = (uint8_t*)&freeRTOSMemoryScheme;

    // Symbols __heap_xxxxx__  are provided by linker.
    // See the linker script in Project_Settings/Linker_Files
    xHeapRegions[0].pucStartAddress = &__heap_low_start__;
    xHeapRegions[0].xSizeInBytes = (size_t) &__heap_low_size__;
    xHeapRegions[1].pucStartAddress = &__heap_high_start__;
    xHeapRegions[1].xSizeInBytes = (size_t) &__heap_high_size__;

    // Define the regions that could be used as heap. Must be called before any usage of the
    // heap, such as task creation.
    vPortDefineHeapRegions((HeapRegion_t*) &xHeapRegions[0]);
}

/**
 * @internal
 * Configures the interrupt priorities to use RTOS API functions in interrupt service routines.
 */
static void
hzlPlatform_InitFreeRtosInterrupts(void)
{
    /* WHY THIS FUNCTION EXISTS:
     *
     * Any interrupt service routine that uses an RTOS API function must have
     * its priority manually set to a value that is numerically equal to or
     * greater than the configMAX_SYSCALL_INTERRUPT_PRIORITY setting. This
     * ensures the interrupt's logical priority is equal to or less than the
     * configMAX_SYSCALL_INTERRUPT_PRIORITY setting.
     *
     * Cortex-M interrupts default to having a priority value of zero. Zero is
     * the highest possible priority value. Therefore, never leave the priority
     * of an interrupt that uses the interrupt safe RTOS API at its default value.
     * https://www.freertos.org/RTOS-Cortex-M3-M4.html
     */
    const IRQn_Type interrupts[] =
        {
         LPSPI0_IRQn,
         LPSPI1_IRQn,
         LPSPI2_IRQn,
         CAN0_ORed_IRQn,
         CAN0_Error_IRQn,
         CAN0_ORed_0_15_MB_IRQn,
         CAN0_ORed_16_31_MB_IRQn,
         CAN0_Wake_Up_IRQn,
         CAN1_ORed_IRQn,
         CAN1_Error_IRQn,
         CAN1_ORed_0_15_MB_IRQn,
         CAN2_ORed_IRQn,
         CAN2_Error_IRQn,
         CAN2_ORed_0_15_MB_IRQn,
         PORTA_IRQn,
         PORTB_IRQn,
         PORTC_IRQn,
         PORTD_IRQn,
         PORTE_IRQn,
         LPUART0_RxTx_IRQn,
         LPUART1_RxTx_IRQn,
         LPUART2_RxTx_IRQn,
         LPIT0_Ch0_IRQn,
         LPIT0_Ch1_IRQn,
         LPIT0_Ch2_IRQn,
         LPIT0_Ch3_IRQn,
         FTFC_IRQn,
        };
    for (size_t i = 0; i < sizeof(interrupts) / sizeof(interrupts[0]); i++)
    {
        // configMAX_SYSCALL_INTERRUPT_PRIORITY is a shifted version of
        // configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY.
        // The INT_SYS_SetPriority() function will also shift before writing into the register.
        INT_SYS_SetPriority(interrupts[i], configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    }
}

/**
 * @internal Creates all the tasks required for the project to run.
 */
static void
hzlPlatform_InitFreeRtosTasks(void)
{
    const configSTACK_DEPTH_TYPE stackSize = 500U; // In WORDS, not bytes
    BaseType_t created;
    // Not using the handle here, but it may be passed to other tasks so they can reference each
    // other and send signals to each other.
    TaskHandle_t hzlTaskHandle = NULL;
    created = xTaskCreate(
        hzlPlatform_TaskHzl,
        "TaskHzl",
        stackSize,
        NULL,
        HZL_PLATFORM_TASK_PRIORITY_HZL,
        &hzlTaskHandle);
    if (created != pdPASS)
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_RTOS_TASK_CREATION);
    }
}

void
hzlPlatform_InitFreeRtos(void)
{
    // The order of these function calls MATTERS. If done improperly, the RTOS may crash at startup.
    hzlPlatform_InitFreeRtosClock();
    hzlPlatform_InitFreeRtosPins();
    hzlPlatform_RgbLedSetColor(HZL_PLATFORM_RGB_COLOR_YELLOW);
    hzlPlatform_InitFreeRtosInterrupts();
    hzlPlatform_InitFreeRtosMulipleRamRegions();
    hzlPlatform_InitFreeRtosTasks();
    hzlPlatform_RgbLedSetColor(HZL_PLATFORM_RGB_COLOR_GREEN);
    // Start the scheduler, which runs the tasks.
    vTaskStartScheduler();
    // The scheduler should never exit. If it does, show an error.
    hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_RTOS_TERMINATED);
}
