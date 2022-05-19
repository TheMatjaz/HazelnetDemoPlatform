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
 * Main application task of the Hazelnet Demo Platform, which simply
 * exchanges dummy data between the nodes in an encrypted manner.
 */

#include "hzlPlatform.h"
#include "hzlPlatform_FatalError.h"
#include "hzl.h"
#if defined(HZL_PLATFORM_ROLE_SERVER)
#include "hzl_Server.h"
#include "hzl_HardcodedConfigServer.h"
#else
#include "hzl_Client.h"
#include "hzl_HardcodedConfigClient.h"
#endif

static void
hzlPlatform_AppServerOnlyForceSessionRenewal(void);
static void
hzlPlatform_AppClientOnlyNewHandshake(void);
static void
hzlPlatform_AppProcessReceivedValid(const hzl_CbsPduMsg_t* reactionPdu,
                                         const hzl_RxSduMsg_t* receivedUserData);

static size_t gSuccessiveSecurityWarningsCounter = 0U;

/**
 * @internal
 * Writes a short (<= 61 B) ASCII string to the bus in the form of a CBS UAD message, which
 * all other parties are configured to ignore.
 * @param string a human readable message.
 */
static void
hzlPlatform_AppLog(const char* string)
{
    hzl_CbsPduMsg_t uad;
    hzl_Err_t hzlErrCode = HZL_PLATFORM_HZL_BUILD_UNSECURED(&uad, &hzlCtx0,
        (const uint8_t*) string, strlen(string),
        HZL_BROADCAST_GID);
    if (hzlErrCode != HZL_OK)
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_HZL_BUILD_UAD);
    }
    hzlPlatform_FlexcanTransmit(uad.data, uad.dataLen);
}

/**
 * @internal
 * Handles the case of a valid HZL-processed (validated, decrypted) message. This includes the
 * transmission of any automatic reaction message and the handling of data that was received
 * in either secured or unsecured format.
 *
 * Any real-world application should probably replace this function with the behaviour that
 * is expected of it.
 */
/**
 * @internal
 * Handles the case of a security problem in the received message.
 *
 * Simply converts the error code into a small ASCII string and logs it onto the bus.
 */
static void
hzlPlatform_AppProcessReceivedSecWarn(const hzl_Err_t hzlErrCode)
{
    hzlPlatform_RgbLedSetColor(HZL_PLATFORM_ERR_HZL_RX_SECURITY_WARNING);
    gSuccessiveSecurityWarningsCounter++;
    const char* msg = "";
    switch (hzlErrCode)
    {
        case HZL_ERR_SECWARN_INVALID_TAG:
            msg = "WARN: invalid tag";
            break;
        case HZL_ERR_SECWARN_MESSAGE_FROM_MYSELF:
            msg = "WARN: message from myself";
            break;
        case HZL_ERR_SECWARN_NOT_EXPECTING_A_RESPONSE:
            msg = "WARN: not expecting RES";
            break;
        case HZL_ERR_SECWARN_SERVER_ONLY_MESSAGE:
            msg = "WARN: server-only message";
            break;
        case HZL_ERR_SECWARN_RESPONSE_TIMEOUT:
            msg = "WARN: RES too late (timeout REQ-to-RES)";
            break;
        case HZL_ERR_SECWARN_OLD_MESSAGE:
            msg = "WARN: old counter nonce";
            break;
        case HZL_ERR_SECWARN_DENIAL_OF_SERVICE:
            msg = "WARN: denial of service";
            break;
        case HZL_ERR_SECWARN_NOT_IN_GROUP:
            msg = "WARN: Client not in REQ Group";
            break;
        case HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE:
            msg = "WARN: RX overflown counter nonce";
            break;
        case HZL_ERR_SECWARN_RECEIVED_ZERO_KEY:
            msg = "WARN: RX all-zero key";
            break;
        default:
            // Not a security warning error code.
            return;
    }
    hzlPlatform_AppLog(msg);
    if (gSuccessiveSecurityWarningsCounter
        > HZL_PLATFORM_HZL_MAX_SECURITY_WARNINGS_BEFORE_REQ)
    {
        hzlPlatform_AppLog("INFO: too many secwarnings");
        gSuccessiveSecurityWarningsCounter = 0U;
        hzlPlatform_AppClientOnlyNewHandshake();
        hzlPlatform_AppServerOnlyForceSessionRenewal();
    }
}

/**
 * @internal
 * Processes a received CAN FD message with the Hazelnet library.
 *
 * Automatic reaction messages are transmitted immediately, decrypted messages are transmitted
 * unencrypted in human-readable format on the bus for the sake of demonstration,
 * unencrypted messages from the bus are ignored.
 */
static void
hzlPlatform_AppProcessReceived(const flexcan_msgbuff_t* const poppedCanFdMsg)
{
    hzl_CbsPduMsg_t reactionPdu;
    hzl_RxSduMsg_t receivedUserData;
    hzl_Err_t hzlErrCode = HZL_PLATFORM_HZL_PROCESS_RECEIVED(
        &reactionPdu,
        &receivedUserData,
        &hzlCtx0,
        poppedCanFdMsg->data,
        poppedCanFdMsg->dataLen,
        poppedCanFdMsg->msgId);
    if (hzlErrCode == HZL_OK)
    {
        // Successful validation and potential decrpytion of the message.
        hzlPlatform_AppProcessReceivedValid(&reactionPdu, &receivedUserData);
    }
    else if (hzlErrCode == HZL_ERR_MSG_IGNORED)
    {
        // The message was successfully processed, only it is not addressed to this party
        // or not of interest in the current state.
        hzlPlatform_RgbLedSetColor(HZL_PLATFORM_ERR_HZL_IGNORED);
    }
    else if (hzlErrCode == HZL_ERR_SESSION_NOT_ESTABLISHED)
    {
        // Client-side error only: the session information was not obtained yet,
        // cannot process the secured message received while waiting for a Response.
        // (Re)send a Request message to obtain the session information instead.
        // Discard the received message.
        hzlPlatform_RgbLedSetColor(HZL_PLATFORM_ERR_HZL_WAITING_FOR_RES);
        hzlPlatform_AppLog("INFO: Session not established, cannot RX yet");
        hzlPlatform_AppClientOnlyNewHandshake();
    }
    else if (HZL_IS_SECURITY_WARNING(hzlErrCode))
    {
        // The message was not successfully processed, as a security problem was detected with it.
        hzlPlatform_AppProcessReceivedSecWarn(hzlErrCode);
    }
    else
    {
        // All other problems, which should all be issues in at program-time (e.g. using too small
        // buffers) but not at run-time.
        hzlPlatform_RgbLedSetColor(HZL_PLATFORM_ERR_HZL_PROCESS_RX_OTHER);
        hzlPlatform_AppLog("ERROR: unexpected problem with process RX");
    }
}

/**
 * @internal
 * Starts a new handshake between Clients and Server.
 *
 * Transmits a Request to obtain the current Session information from the Server
 * again.
 */
static void
hzlPlatform_AppClientOnlyNewHandshake(void)
{
#if !defined(HZL_PLATFORM_ROLE_SERVER)
    hzl_CbsPduMsg_t pdu;
    hzl_Err_t hzlErrCode;
    // On the Client
    hzlErrCode = hzl_ClientBuildRequest(&pdu, &hzlCtx0, HZL_BROADCAST_GID);
    if (hzlErrCode == HZL_OK)
    {
        hzlPlatform_RgbLedSetColor(HZL_PLATFORM_ERR_HZL_WAITING_FOR_RES);
        hzlPlatform_FlexcanTransmit(pdu.data, pdu.dataLen);
    }
    else if (hzlErrCode == HZL_ERR_HANDSHAKE_ONGOING)
    {
        // The previously transmitted Request did not timeout yet. Not transmitting anything.
        hzlPlatform_RgbLedSetColor(HZL_PLATFORM_ERR_HZL_WAITING_FOR_RES);
        hzlPlatform_AppLog("INFO: Not requesting yet, still waiting for RES");
    }
    else
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_HZL_BUILD_REQUEST);
    }
#endif  /* defined(HZL_PLATFORM_ROLE_SERVER) */
}

/**
 * @internal
 * Starts a new Session on the Server and transmits the Renewal notification.
 */
static void
hzlPlatform_AppServerOnlyForceSessionRenewal(void)
{
#if defined(HZL_PLATFORM_ROLE_SERVER)
    hzl_CbsPduMsg_t pdu;
    hzl_Err_t hzlErrCode;
    // On the Server
    hzlErrCode = hzl_ServerForceSessionRenewal(&pdu, &hzlCtx0, HZL_BROADCAST_GID);
    if (hzlErrCode == HZL_OK)
    {
        hzlPlatform_RgbLedSetColor(HZL_PLATFORM_ERR_HZL_WAITING_FOR_REQ);
        hzlPlatform_FlexcanTransmit(pdu.data, pdu.dataLen);
    }
    else if (hzlErrCode == HZL_ERR_NO_POTENTIAL_RECEIVER)
    {
        // REN message construction called to early: no Clients can be notified yet of the renewal
        // as no Client has the previous Session information. Nothing to transmit
        hzlPlatform_RgbLedSetColor(HZL_PLATFORM_ERR_HZL_NO_CLIENTS_YET);
        hzlPlatform_AppLog("INFO: No Clients to send REN to");
    }
    else
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_HZL_BUILD_RENEWAL);
    }
#endif  /* defined(HZL_PLATFORM_ROLE_SERVER) */
}


/**
 * @internal
 * Transmission in secured format of a dummy uint8_t rolling counter, padded to 128 bits.
 *
 * The padding is done to make brute-forcing through all ciphertexts harder.
 */
static void
hzlPlatform_AppTransmitDummyMsg(uint8_t dummyTxMsgContent)
{
    hzl_CbsPduMsg_t pdu;
    uint8_t txDataBuffer[16];
    memset(txDataBuffer, 0x55U, sizeof(txDataBuffer));  // Dummy padding value: 0b01010101
    txDataBuffer[0] = dummyTxMsgContent;  // Our actual plaintext is just 1 byte
    hzl_Err_t hzlErrCode = HZL_PLATFORM_HZL_BUILD_SECURED_FD(
        &pdu,
        &hzlCtx0,
        txDataBuffer,
        sizeof(txDataBuffer),
        HZL_BROADCAST_GID);
    if (hzlErrCode == HZL_OK)
    {
        // Successful securing: just transmit the message.
        hzlPlatform_FlexcanTransmit(pdu.data, pdu.dataLen);
    }
    else if (hzlErrCode == HZL_ERR_NO_POTENTIAL_RECEIVER)
    {
        // Server-side error only: still waiting for at least ONE Client to transmit a Request (REQ)
        // message. Otherwise it does not make any sense for the Server to transmit secured messages
        // as nobody else could decrypt them.
        hzlPlatform_RgbLedSetColor(HZL_PLATFORM_ERR_HZL_NO_CLIENTS_YET);
        hzlPlatform_AppLog("INFO: Cannot TX yet, no REQ so far");
    }
    else if (hzlErrCode == HZL_ERR_SESSION_NOT_ESTABLISHED)
    {
        // Client-side error only: the transmission cannot happen, as there is no session
        // information that could be used to transmit the data securely.
        hzlPlatform_AppClientOnlyNewHandshake();
    }
    else if (hzlErrCode == HZL_ERR_HANDSHAKE_ONGOING)
    {
        // Client-side error only: still waiting for the Server to transmit a Response (RES)
        // for our Request (REQ). The transmission cannot happen, as there is no session
        // information that could be used to transmit the data securely.
        hzlPlatform_RgbLedSetColor(HZL_PLATFORM_ERR_HZL_WAITING_FOR_RES);
        hzlPlatform_AppLog("INFO: Cannot TX yet, no RES yet");
    }
    else
    {
        // All other problems, which should all be issues in at program-time (e.g. using too small
        // buffers) but not at run-time.
        hzlPlatform_RgbLedSetColor(HZL_PLATFORM_ERR_HZL_BUILD_OTHER);
        hzlPlatform_AppLog("ERRO: problem with building SADFD");
    }
}

/**
 * @internal
 * Main application task initialisation phase.
 * Enable the hardware components, OS components and libraries required for the application to run.
 * Returns the reception queue of incoming CAN FD messages.
 */
static QueueHandle_t
hzlPlatform_TaskHzlInit(void)
{
    QueueHandle_t rxCanMsgsQueue = hzlPlatform_FlexcanInit();
    CSEC_DRV_Init(&csec1_State);
    const status_t status = CSEC_DRV_InitRNG();
    if (status != STATUS_SUCCESS)
    {
        // The CSEC module cannot initialise the RNG properly. This is generally the case when
        // the EEPROM was not properly partitioned for the security module.
        // This has to be done with a SEPARATE S32 project once per board.
        // Steps:
        // 1. Open the csec_keyconfig_s32k144 example project in S32 Design Studio.
        // 2. Launch the debugger on the board with the RAM (IMPORTANT, NOT flash) version of
        //    said project.
        // 3. Let the debugger run through the end (F8). The board will turn on either the red
        //    or green LED at the end. Either are fine for the Hazelnet Demo Platform.
        // 4. Now use the Hazelnet Demo Platform project again and this error should not happen.
        // 5. If you change the content of the flash, you will need to perform steps 1-3 again.
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_CSEC_RNG_INIT);
    }
    hzlPlatform_PeriodicTxTimerInit(xTaskGetCurrentTaskHandle());
    hzlPlatform_Button1And2Init(xTaskGetCurrentTaskHandle());
    hzlCtx0.io.trng = hzlPlatform_HzlAdapterTrng;
    hzlCtx0.io.currentTime = hzlPlatform_HzlAdapterCurrentTime;
    const hzl_Err_t hzlErrCode = HZL_PLATFORM_HZL_INIT(&hzlCtx0);
    if (hzlErrCode != HZL_OK)
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_HZL_INIT);
    }
    hzlPlatform_AppLog(
        "INFO: Hazelnet Demo Platform:" HZL_PLATFORM_VERSION
        " Lib:" HZL_VERSION
        " CBS:" HZL_CBS_PROTOCOL_VERSION_SUPPORTED);
#if defined(HZL_PLATFORM_ROLE_SERVER)
    hzlPlatform_RgbLedSetColor(HZL_PLATFORM_ERR_HZL_WAITING_FOR_REQ);
#endif
    hzlPlatform_AppClientOnlyNewHandshake();
    return rxCanMsgsQueue;
}

/**
 * @internal
 * Cleanup of the contexts and peripherals used by the TaskHzl in case the Task is restarted.
 */
static void
hzlPlatform_AppProcessReceivedValid(const hzl_CbsPduMsg_t* const reactionPdu,
                                         const hzl_RxSduMsg_t* const receivedUserData)
{
    if (reactionPdu->dataLen > 0)
    {
        // The Hazelnet library generated an automatic response (e.g. a RES after received a REQ)
        // which we should transmit. Better do it immediately to avoid any delays and handle
        // anything else about the received message afterwards.
        hzlPlatform_FlexcanTransmit(reactionPdu->data, reactionPdu->dataLen);
    }
    if (!receivedUserData->isForUser)
    {
        // Ignore the message, it's an internal one within the CBS layer. It does not contain any
        // user (application) data.
        return;
    }
    if (!receivedUserData->wasSecured)
    {
        // The message was NOT encrypted and authenticated on the bus, thus its data should NOT
        // be considered as safe.
        //
        // In the case of this demo, all unsecured messages are just log messages coming from other
        // parties meant for a human operator sniffing the bus from a desktop/laptop. It can be
        // safely discarded for the sake for this demo.
        return;
    }
    hzlPlatform_RgbLedSetColor(HZL_PLATFORM_ERR_HZL_DECRYPTED);
    // At this point we know the received message contains some application data AND that it
    // was transmitted in a secure manner on the bus.
    //
    // In the case of this demo, the message contains simply an encrypted uint8_t counter padded to
    // 16 bytes to provide more noise to any attacker. For the sake of demonstration, the counter is
    // now converted to an ASCII string and transmitted again on the bus for the human operator to
    // see. This essentially proves that the receiving party has successfully received and decrypted
    // the secured message.
    // The formatted string has exactly 34 bytes of length (before the null terminator).
    const char format[] = "RX GID=%02X,SID=%02X,Secret counter=%02X";
    char buffer[48U];
    sprintf(buffer,
        format,
        receivedUserData->gid,
        receivedUserData->sid,
        receivedUserData->data[0]  // The decrypted counter
        );
    hzlPlatform_AppLog(buffer);
}

static void
hzlPlatform_TaskHzlDeinit(void)
{
    hzlPlatform_AppLog("INFO: powering down");
    const hzl_Err_t hzlErrCode = HZL_PLATFORM_HZL_DEINIT(&hzlCtx0);
    if (hzlErrCode != HZL_OK)
    {
        hzlPlatform_FatalCrashAlternating(HZL_PLATFORM_CRASH_HZL_DEINIT);
    }
    // TODO Deinit the security hardware, if using it.
    hzlPlatform_FlexcanDeinit();
    while (true)
    {
        // Stuck forever in a controlled manner, waiting for an official powerdown or reset.
        // This is just a mock of a low-power mode, where the board is just waiting for a
        // CAN event or waiting for a RTC to trigger an interrupt to wake the board.
        // Press the reset button to start over.
        hzlPlatform_RgbLedSetColor(HZL_PLATFORM_RGB_COLOR_RED);
        vTaskDelay(300);
        hzlPlatform_RgbLedSetColor(HZL_PLATFORM_RGB_COLOR_GREEN);
        vTaskDelay(300);
        hzlPlatform_RgbLedSetColor(HZL_PLATFORM_RGB_COLOR_BLUE);
        vTaskDelay(300);
        hzlPlatform_RgbLedSetColor(HZL_PLATFORM_RGB_COLOR_BLACK);
        vTaskDelay(300);
    }
}

void
hzlPlatform_TaskHzl(void* const unusedParam)
{
    (void) unusedParam;
    QueueHandle_t rxCanMsgsQueue = hzlPlatform_TaskHzlInit();
    flexcan_msgbuff_t poppedRxCanFdMsg;
    uint8_t rollingCounterDummyTxMsgContent = HZL_PLATFORM_COUNTER_START;
    bool keepRunning = true;
    // Main application loop.
    // Periodically transmit a dummy encrypted message on the bus and react on all received
    // messages from the bus.
    while (keepRunning)
    {
        // Upon reception, the FLEXCAN interrupt with place the received CAN FD message into the
        // rxCanMsgsQueue (see hzlPlatform_EnqueueReceivedCanFrame). Now we remove it from
        // the queue and feed it to the Hazelnet library to process.
        const BaseType_t isPoppedFromQueue = xQueueReceive(
            rxCanMsgsQueue,
            &poppedRxCanFdMsg,
            HZL_PLATFORM_CANFD_RX_QUEUE_POP_TIMEOUT_TICKS);
        if (isPoppedFromQueue)
        {
            hzlPlatform_AppProcessReceived(&poppedRxCanFdMsg);
        }
        // Periodic transmission of a dummy message when the timer expires.
        const uint32_t notificationEventBitmap = ulTaskNotifyTake(
        true,  // Clear notification event bitmap value on exit.
            0U  // Timeout. Zero for non-blocking, return immediately.
            );
        if (notificationEventBitmap & HZL_PLATFORM_TASK_EVENT_TX_TIMER_EXPIRED)
        {
            // The time has come for the periodic transmission of dummy data.
            hzlPlatform_AppTransmitDummyMsg(rollingCounterDummyTxMsgContent);
            rollingCounterDummyTxMsgContent++;  // It IS supposed to overflow and roll-around.
        }
        if (notificationEventBitmap & HZL_PLATFORM_TASK_EVENT_BUTTON_1_PRESSED)
        {
            // Trigger the virtual shutdown of the device.
#if defined(HZL_PLATFORM_ROLE_SERVER)
            hzlPlatform_AppLog("INFO: the Server cannot be powered down");
#else
            keepRunning = false;
#endif
        }
        if (notificationEventBitmap & HZL_PLATFORM_TASK_EVENT_BUTTON_2_PRESSED)
        {
            // Triggers the synchronisation of the Session manually: a REN from the Server,
            // a REQ from the Client.
            // On the Server
            hzlPlatform_AppServerOnlyForceSessionRenewal();
            // On the Client
            hzlPlatform_AppClientOnlyNewHandshake();
        }
    }
    hzlPlatform_TaskHzlDeinit();  // This function never returns
}
