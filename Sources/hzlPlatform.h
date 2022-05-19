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
 */

#ifndef HZL_PLATFORM_H_
#define HZL_PLATFORM_H_

#ifdef __cplusplus
extern "C"
{
#endif

// ISO C stdlib
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// FreeRTOS kernel
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

// S32 SDK
#include "interrupt_manager.h"
#include "clock_manager.h"
#include "clockMan1.h"
#include "pin_mux.h"

// Application headers
#include "hzlPlatform_RgbLed.h"
#include "hzl.h"

#define HZL_PLATFORM_VERSION "v1.1.0"

// Macro in the main.c used to alias the actual RTOS main function written by the user.
#define PEX_RTOS_START hzlPlatform_InitFreeRtos

// Recoverable application errors and warning states
#define HZL_PLATFORM_ERR_HZL_IGNORED             HZL_PLATFORM_RGB_COLOR_BLACK
#define HZL_PLATFORM_ERR_HZL_DECRYPTED           HZL_PLATFORM_RGB_COLOR_GREEN
#define HZL_PLATFORM_ERR_HZL_WAITING_FOR_RES     HZL_PLATFORM_RGB_COLOR_CYAN
#define HZL_PLATFORM_ERR_HZL_RX_SECURITY_WARNING HZL_PLATFORM_RGB_COLOR_RED
#define HZL_PLATFORM_ERR_HZL_PROCESS_RX_OTHER    HZL_PLATFORM_RGB_COLOR_YELLOW
#define HZL_PLATFORM_ERR_HZL_WAITING_FOR_REQ     HZL_PLATFORM_RGB_COLOR_MAGENTA
#define HZL_PLATFORM_ERR_HZL_BUILD_OTHER         HZL_PLATFORM_RGB_COLOR_BLUE
#define HZL_PLATFORM_ERR_HZL_NO_CLIENTS_YET      HZL_PLATFORM_RGB_COLOR_WHITE

// FreeRTOS task priorities
#define HZL_PLATFORM_TASK_PRIORITY_HZL (tskIDLE_PRIORITY + 2)

// CAN transmission configuration
#define HZL_PLATFORM_CANFD_TX_MAILBOX_INDEX 0U
#define HZL_PLATFORM_CANFD_TX_TRIES 10U
#define HZL_PLATFORM_CANFD_TX_TIMEOUT_TICKS 30U

// CAN reception configuration
#define HZL_PLATFORM_CANFD_RX_MAILBOX_INDEX 1U
#define HZL_PLATFORM_CANFD_RX_QUEUE_LEN 8U
#define HZL_PLATFORM_CANFD_RX_QUEUE_POP_TIMEOUT_TICKS 50U
#define HZL_PLATFORM_HZL_MAX_SECURITY_WARNINGS_BEFORE_REQ 5U


typedef enum hzlPlatform_TaskEventBitmap
{
    HZL_PLATFORM_TASK_EVENT_NONE = 0x00U,
    HZL_PLATFORM_TASK_EVENT_TX_TIMER_EXPIRED = 0x01U,
    HZL_PLATFORM_TASK_EVENT_BUTTON_1_PRESSED = 0x02U,
    HZL_PLATFORM_TASK_EVENT_BUTTON_2_PRESSED = 0x04U,
} hzlPlatform_TaskEventBitmap_t;

typedef enum hzlPlatform_CanId
{
    HZL_PLATFORM_CANID_FROM_SERVER = 0x700U,
    HZL_PLATFORM_CANID_FROM_ALICE = 0x70AU,
    HZL_PLATFORM_CANID_FROM_BOB = 0x70BU,
    HZL_PLATFORM_CANID_FROM_CHARLIE = 0x70CU,
} hzlPlatform_CanId_t;

#if defined(HZL_PLATFORM_ROLE_SERVER)
#define HZL_PLATFORM_CANID_FROM_ME HZL_PLATFORM_CANID_FROM_SERVER
#define HZL_PLATFORM_COUNTER_START 0xF0U
#define HZL_PLATFORM_TX_TIMER_TICKS 2000U
#elif defined(HZL_PLATFORM_ROLE_ALICE)
#define HZL_PLATFORM_CANID_FROM_ME HZL_PLATFORM_CANID_FROM_ALICE
#define HZL_PLATFORM_COUNTER_START 0xA0U
#define HZL_PLATFORM_TX_TIMER_TICKS 3000U
#elif defined(HZL_PLATFORM_ROLE_BOB)
#define HZL_PLATFORM_CANID_FROM_ME HZL_PLATFORM_CANID_FROM_BOB
#define HZL_PLATFORM_COUNTER_START 0xB0U
#define HZL_PLATFORM_TX_TIMER_TICKS 4000U
#elif defined(HZL_PLATFORM_ROLE_CHARLIE)
#define HZL_PLATFORM_CANID_FROM_ME HZL_PLATFORM_CANID_FROM_CHARLIE
#define HZL_PLATFORM_COUNTER_START 0xC0U
#define HZL_PLATFORM_TX_TIMER_TICKS 5000U
#else
#error "Define one of the following macros at compile time: HZL_PLATFORM_ROLE_{SERVER|ALICE|BOB|CHARLIE}"
#endif

#if defined(HZL_PLATFORM_ROLE_SERVER)
#define HZL_PLATFORM_HZL_INIT hzl_ServerInit
#define HZL_PLATFORM_HZL_BUILD_UNSECURED hzl_ServerBuildUnsecured
#define HZL_PLATFORM_HZL_PROCESS_RECEIVED hzl_ServerProcessReceived
#define HZL_PLATFORM_HZL_BUILD_SECURED_FD hzl_ServerBuildSecuredFd
#define HZL_PLATFORM_HZL_DEINIT hzl_ServerDeInit
#else
#define HZL_PLATFORM_HZL_INIT hzl_ClientInit
#define HZL_PLATFORM_HZL_BUILD_UNSECURED hzl_ClientBuildUnsecured
#define HZL_PLATFORM_HZL_PROCESS_RECEIVED hzl_ClientProcessReceived
#define HZL_PLATFORM_HZL_BUILD_SECURED_FD hzl_ClientBuildSecuredFd
#define HZL_PLATFORM_HZL_DEINIT hzl_ClientDeInit
#endif

/**
 * Main function with FreeRTOS initialisation and starting of the tasks.
 * Works specifically for the S32K144, but should be easy to reconfigure for different boards and
 * different tasks.
 */
void
hzlPlatform_InitFreeRtos(void);

/**
 * Initialised the FLEXCAN driver for a CAN FD bus, accpeting all CAN IDs (no filtering)
 * and automatically pushing received messages into the queue returned by the function for the
 * main application/task to pop when it has time.
 * @return queue of received messages.
 */
QueueHandle_t
hzlPlatform_FlexcanInit(void);

/**
 * Deinitialises the FLEXCAN driver for the CAN FD bus.
 */
void
hzlPlatform_FlexcanDeinit(void);

/**
 * Blocking transmission of a CAN FD message with automatic retries when busy.
 *
 * Tries to transmit a few times in case the CAN driver is busy or the internal blocking timeouts
 * are reached. In case no transmission could succeed, a fatal error state is entered, as it's
 * probably a bus connector issue in the context of this demo platform.
 */
void
hzlPlatform_FlexcanTransmit(const uint8_t* payload, const size_t payloadLen);

/**
 * Creates a periodic timer a flag every #HZL_PLATFORM_TX_TIMER_TICKS ticks
 * that notifies the given task on expiration.
 *
 * The notification is read with ulTaskNotifyTake().
 * The set notification bitflag is #HZL_PLATFORM_TASK_EVENT_TX_TIMER_EXPIRED.
 */
void
hzlPlatform_PeriodicTxTimerInit(TaskHandle_t taskToNotify);

/**
 * Sets the Button 1 (SW3 on the eval-board) and 2 (Sw2) to notify the given task on button press.
 *
 * **No** debouncing is performed.
 *
 * The notification is read with ulTaskNotifyTake().
 * The set notification bitflags are #HZL_PLATFORM_TASK_EVENT_BUTTON_1_PRESSED
 * and #HZL_PLATFORM_TASK_EVENT_BUTTON_2_PRESSED.
 */
void
hzlPlatform_Button1And2Init(TaskHandle_t taskToNotify);

/**
 * Wrapper/adapter of a True Random Number Generator into the function signature the Hazelnet
 * library (both Client and Server) can use. See #hzl_TrngFunc.
 */
hzl_Err_t
hzlPlatform_HzlAdapterTrng(uint8_t* bytes, size_t amount);

/**
 * Wrapper/adapter of a current-timestamp function into the function signature the Hazelnet
 * library (both Client and Server) can use. See #hzl_TimestampFunc.
 */
hzl_Err_t
hzlPlatform_HzlAdapterCurrentTime(hzl_Timestamp_t* timestamp);

/**
 * Main application as a FreeRTOS task.
 *
 * @param rxCanMsgsQueue queue where the FLEXCAN driver places the received CAN FD messages.
 */
void hzlPlatform_TaskHzl(QueueHandle_t rxCanMsgsQueue);

#ifdef __cplusplus
}
#endif

#endif  /* HZL_PLATFORM_H_ */
