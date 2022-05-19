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
 * FLEXCAN driver initialisation to be able to communicate with the CAN bus.
 */

#include <hzlPlatform_RgbLed.h>
#include "hzlPlatform.h"
#include "hzlPlatform_FatalError.h"

/**
 * @internal
 * Bitmask accepting all CAN IDs (because no bits MUST be set according to this mask).
 * 0x1FFFFFFFU means that all 29 bits MUST be set.
 * 0xFF means that only CAN IDs in the range [0, 255] are accepted.
 * 0 means all of them are.
 */
#define HZL_PLATFORM_CANID_MASK_ALL_ACCEPTED 0U

/**
 * @internal
 * Configuration for the CAN mailboxes, both TX and RX.
 * Used as a default value, copied in its entirety and then customised, for each transmission.
 */
static const flexcan_data_info_t
HZL_PLATFORM_CANFD_MAILBOX_DEFAULT_CONFIG =
    {
     .data_length = 0,  // To be CUSTOMISED before transmission
     .is_remote = false,  // CAN FD does not support Remote Transmission Requests
     .msg_id_type = FLEXCAN_MSG_ID_EXT,  // 29 bit CAN IDs
     .enable_brs = false,  // Same bitrate of data and arbitration parts
     .fd_enable = true,  // Use CAN FD for longer payloads
     .fd_padding = 0xAAU,  // This padding minimises the amount of stuff bits
    };

/**
 * @internal
 * Location where a just-received CAN FD message is written by FLEXCAN_DRV_Receive() prior to
 * calling hzlPlatform_CallbackOnCanEvent().
 */
static flexcan_msgbuff_t hzlPlatform_TempRxCanMsg;

/**
 * @internal
 * Places the just-received CAN frame into a queue (producer pattern) and starts a new reception.
 */
inline static void
hzlPlatform_EnqueueReceivedCanFrame(QueueHandle_t rxCanMsgsQueue)
{
    BaseType_t isThereATaskWaitingForQueue = pdFALSE;
    // The FLEXCAN_DRV_Receive(), called by hzlPlatform_InitFlexcan() or by this
    // callback, has placed the received message into hzlPlatform_TempRxCanMsg,
    // and then this callback was called.
    // Enqueue the temp message for the main application to dequeue when it has some time.
    xQueueSendToBackFromISR(rxCanMsgsQueue,
        &hzlPlatform_TempRxCanMsg,
        &isThereATaskWaitingForQueue);
    // Note: the error returned from  the queue is not handled. If the queue is full,
    // simply the to-be-enqueued message is discarded.
    // Start a new non-blocking reception, which will call hzlPlatform_CallbackOnCanEvent()
    // again when something new is received.
    const status_t status = FLEXCAN_DRV_Receive(INST_CANCOM1,
    HZL_PLATFORM_CANFD_RX_MAILBOX_INDEX,
        &hzlPlatform_TempRxCanMsg);
    if (status != STATUS_SUCCESS)
    {
        // This should never fail, hopefully.
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_CANFD_RX);
    }
    // xQueue tells us if there is a task waiting for something to be popped from
    // a queue. With this information we can hint the scheduler with the yield operation
    // to schedule the task waiting for the queue immediately after this callback
    // instead of scheduling the task that was just interrupted.
    portYIELD_FROM_ISR(isThereATaskWaitingForQueue);
}

/**
 * @internal
 * Function called by the FLEXCAN driver upon successful transmission, reception or
 * other events of the CAN driver.
 *
 * Upon reception it copies the received CAN FD message into the RX queue passed to it.
 * Upon any other event it simply does nothing.
 *
 * @param [in] instance unused
 * @param [in] eventType shows what triggered the call of this function
 * @param [in] buffIdx unused
 * @param [in] flexcanState used to obtain the queue handle from its callbackParam field.
 */
static void
hzlPlatform_CallbackOnCanEvent(const uint8_t instance,
                                    const flexcan_event_type_t eventType,
                                    const uint32_t buffIdx,
                                    flexcan_state_t* const flexcanState)
{
    (void) instance;
    (void) buffIdx;
    // Obtain the queue where this callback will put the messages from the callback-installation
    // FLEXCAN_DRV_InstallEventCallback() call.
    QueueHandle_t rxCanMsgsQueue = flexcanState->callbackParam;
    switch (eventType)
    {
        case FLEXCAN_EVENT_RX_COMPLETE:
            {
            hzlPlatform_EnqueueReceivedCanFrame(rxCanMsgsQueue);
            break;
        }
        default:
            {
            // Event not of interest, ignoring it.
            break;
        }
    }
}

/**
 * @internal
 * Configures the CAN FD I/O with 2 mailboxes (one for TX, one for RX) and sets the RX queue for
 * CAN frames.
 * Must be called WITHIN a task as it uses some FreeRTOS functionalities to operate the FLEXCAN
 * driver.
 */
QueueHandle_t hzlPlatform_FlexcanInit(void)  // TODO rename to FlexcanInit or CanInit
{
    // Initialise and prepare 2 mailboxes: 1 for transmission, 1 for reception
    status_t status;
    status = FLEXCAN_DRV_Init(INST_CANCOM1, &canCom1_State, &canCom1_InitConfig0);
    if (status != STATUS_SUCCESS)
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_CANFD_INIT);
    }
    // Apply CAN ID masking (filtering) rules. Individual == setting per-mailbox rather than global.
    FLEXCAN_DRV_SetRxMaskType(INST_CANCOM1, FLEXCAN_RX_MASK_INDIVIDUAL);
    // TX mailbox
    const uint8_t defaultCanId = 0;
    status = FLEXCAN_DRV_ConfigTxMb(INST_CANCOM1,
    HZL_PLATFORM_CANFD_TX_MAILBOX_INDEX,
        &HZL_PLATFORM_CANFD_MAILBOX_DEFAULT_CONFIG,
        defaultCanId);
    if (status != STATUS_SUCCESS)
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_CANFD_INIT);
    }
    status = FLEXCAN_DRV_SetRxIndividualMask(INST_CANCOM1,
        FLEXCAN_MSG_ID_EXT,
        HZL_PLATFORM_CANFD_TX_MAILBOX_INDEX,
        HZL_PLATFORM_CANID_MASK_ALL_ACCEPTED);
    if (status != STATUS_SUCCESS)
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_CANFD_INIT);
    }
    // RX mailbox
    status = FLEXCAN_DRV_ConfigRxMb(
    INST_CANCOM1,
    HZL_PLATFORM_CANFD_RX_MAILBOX_INDEX,
        &HZL_PLATFORM_CANFD_MAILBOX_DEFAULT_CONFIG,
        defaultCanId);
    if (status != STATUS_SUCCESS)
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_CANFD_INIT);
    }
    status = FLEXCAN_DRV_SetRxIndividualMask(INST_CANCOM1,
        FLEXCAN_MSG_ID_EXT,
        HZL_PLATFORM_CANFD_RX_MAILBOX_INDEX,
        HZL_PLATFORM_CANID_MASK_ALL_ACCEPTED);
    if (status != STATUS_SUCCESS)
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_CANFD_INIT);
    }
    // Prepare the RX queue where the received, but unprocessed messages, accumulate
    // waiting for another task to pop and process them.
    const QueueHandle_t rxCanMsgsQueue = xQueueCreate(
        HZL_PLATFORM_CANFD_RX_QUEUE_LEN,
        sizeof(flexcan_msgbuff_t));
    if (rxCanMsgsQueue == NULL)
    {
        // malloc fails to a hook internally within xQueueCreate, so this branch should never occur.
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_OUT_OF_MEMORY);
    }
    FLEXCAN_DRV_InstallEventCallback(INST_CANCOM1,
        hzlPlatform_CallbackOnCanEvent,
        rxCanMsgsQueue);
    if (status != STATUS_SUCCESS)
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_CANFD_INIT);
    }
    // Start the non-blocking reception, which will call the callback when something is received.
    status = FLEXCAN_DRV_Receive(INST_CANCOM1,
    HZL_PLATFORM_CANFD_RX_MAILBOX_INDEX,
        &hzlPlatform_TempRxCanMsg);
    if (status != STATUS_SUCCESS)
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_CANFD_RX);
    }
    return rxCanMsgsQueue;
}

void
hzlPlatform_FlexcanDeinit(void)
{
    const status_t status = FLEXCAN_DRV_Deinit(INST_CANCOM1);
    if (status != STATUS_SUCCESS)
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_CANFD_DEINIT);
    }
}

void
hzlPlatform_FlexcanTransmit(const uint8_t* payload, const size_t payloadLen)
{
    // Copy the entier config struct by VALUE in order to change just the payload length field.
    flexcan_data_info_t msgMetadata = HZL_PLATFORM_CANFD_MAILBOX_DEFAULT_CONFIG;
    msgMetadata.data_length = payloadLen;
    status_t txStatus;
    uint32_t tries = 0;
    while (tries < HZL_PLATFORM_CANFD_TX_TRIES)
    {
        txStatus = FLEXCAN_DRV_SendBlocking(
        INST_CANCOM1,
        HZL_PLATFORM_CANFD_TX_MAILBOX_INDEX,
            &msgMetadata,
            HZL_PLATFORM_CANID_FROM_ME,
            payload,
            HZL_PLATFORM_CANFD_TX_TIMEOUT_TICKS
            );
        tries++;
        if (txStatus == STATUS_SUCCESS)
        {
            return;
        }
        else if (txStatus == STATUS_BUSY)
        {
            // Wait just a tick (1 ms) and try transmitting again. Maybe the peripheral is not
            // busy anymore in the meantime.
            // NOTE: this is definitely NOT a nice solution, but for this demo platform it works
            // fine, as here the goal is communication, not performance.
            vTaskDelay(1);
            continue;
        }
        else if (txStatus == STATUS_TIMEOUT)
        {
            continue;  // Try again
        }
        else
        {
            // An error that "should not happen" occurred.
            hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_CANFD_TX);
        }
    }
    // Tried a few times, still cannot transmit. Enter the unrecoverable error state.
    hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_CANFD_TX);
}
