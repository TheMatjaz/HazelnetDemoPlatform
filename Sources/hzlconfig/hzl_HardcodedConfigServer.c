/*
 * Copyright © 2022, Matjaž Guštin <dev@matjaz.it>
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
 * Compile-time constant configuration with static memory for the Hazelnet
 * context state for the Server.
 *
 * AUTO-GENERATED FILE by hzlconfig at 2022-05-15T17:45:55.765643+00:00
 */

#include "hzl.h"
#include "hzl_Server.h"

#define AMOUNT_OF_CLIENTS 3U
#define AMOUNT_OF_GROUPS 5U

static const hzl_ServerConfig_t serverConfig =
{
    .amountOfGroups = 5,
    .amountOfClients = 3,
    .headerType = 0,
};

static const hzl_ServerClientConfig_t clientConfigs[AMOUNT_OF_CLIENTS] =
{
{
    .sid = 1,
    .ltk =
    {
        0x01,
        0x00,
        0x02,
        0x00,
        0x03,
        0x00,
        0x04,
        0x00,
        0x05,
        0x00,
        0x06,
        0x00,
        0x07,
        0x00,
        0x08,
        0x00,
    },
},
{
    .sid = 2,
    .ltk =
    {
        0x02,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    },
},
{
    .sid = 3,
    .ltk =
    {
        0x03,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    },
}
};

static const hzl_ServerGroupConfig_t groupConfigs[AMOUNT_OF_GROUPS] =
{
{
    .maxCtrnonceDelayMsgs = 22,
    .ctrNonceUpperLimit = 0xFF0000,
    .sessionDurationMillis = 30000,
    .delayBetweenRenNotificationsMillis = 250,
    .clientSidsInGroupBitmap = 0xFFFFFFFF,
    .maxSilenceIntervalMillis = 5000,
    .gid = 0,
    .unusedPadding =
    {
        0xAA,
    },
},
{
    .maxCtrnonceDelayMsgs = 20,
    .ctrNonceUpperLimit = 0x0003E8,
    .sessionDurationMillis = 35000,
    .delayBetweenRenNotificationsMillis = 250,
    .clientSidsInGroupBitmap = 0x00000006,
    .maxSilenceIntervalMillis = 5000,
    .gid = 1,
    .unusedPadding =
    {
        0xAA,
    },
},
{
    .maxCtrnonceDelayMsgs = 33,
    .ctrNonceUpperLimit = 0xFF0000,
    .sessionDurationMillis = 40000,
    .delayBetweenRenNotificationsMillis = 250,
    .clientSidsInGroupBitmap = 0x00000001,
    .maxSilenceIntervalMillis = 5001,
    .gid = 2,
    .unusedPadding =
    {
        0xAA,
    },
},
{
    .maxCtrnonceDelayMsgs = 44,
    .ctrNonceUpperLimit = 0xFF0000,
    .sessionDurationMillis = 45000,
    .delayBetweenRenNotificationsMillis = 250,
    .clientSidsInGroupBitmap = 0x00000003,
    .maxSilenceIntervalMillis = 5002,
    .gid = 3,
    .unusedPadding =
    {
        0xAA,
    },
},
{
    .maxCtrnonceDelayMsgs = 20,
    .ctrNonceUpperLimit = 0xFEF970,
    .sessionDurationMillis = 50000,
    .delayBetweenRenNotificationsMillis = 250,
    .clientSidsInGroupBitmap = 0x00000004,
    .maxSilenceIntervalMillis = 5000,
    .gid = 4,
    .unusedPadding =
    {
        0xAA,
    },
}
};

static hzl_ServerGroupState_t groupStates[AMOUNT_OF_GROUPS];

hzl_ServerCtx_t hzlCtx0 =
{
    .serverConfig = &serverConfig,
    .clientConfigs = clientConfigs,
    .groupConfigs = groupConfigs,
    .groupStates = groupStates,
};
