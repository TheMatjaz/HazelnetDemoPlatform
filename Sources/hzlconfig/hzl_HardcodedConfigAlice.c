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
 * context state for the Client Alice.
 *
 * AUTO-GENERATED FILE by hzlconfig at 2022-05-15T17:45:55.765643+00:00
 */

#include "hzl.h"
#include "hzl_Client.h"

#define AMOUNT_OF_GROUPS 3U

static const hzl_ClientConfig_t clientConfig =
{
    .timeoutReqToResMillis = 10000,
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
    .sid = 1,
    .headerType = 0,
    .amountOfGroups = 3,
    .unusedPadding =
    {
        0xAA,
    },
};

static const hzl_ClientGroupConfig_t groupConfigs[AMOUNT_OF_GROUPS] =
{
{
    .maxCtrnonceDelayMsgs = 22,
    .maxSilenceIntervalMillis = 5000,
    .sessionRenewalDurationMillis = 2000,
    .gid = 0,
    .unusedPadding =
    {
        0xAA,
        0xAA,
        0xAA,
    },
},
{
    .maxCtrnonceDelayMsgs = 33,
    .maxSilenceIntervalMillis = 5001,
    .sessionRenewalDurationMillis = 2000,
    .gid = 2,
    .unusedPadding =
    {
        0xAA,
        0xAA,
        0xAA,
    },
},
{
    .maxCtrnonceDelayMsgs = 44,
    .maxSilenceIntervalMillis = 5002,
    .sessionRenewalDurationMillis = 2000,
    .gid = 3,
    .unusedPadding =
    {
        0xAA,
        0xAA,
        0xAA,
    },
}
};

static hzl_ClientGroupState_t groupStates[AMOUNT_OF_GROUPS];

hzl_ClientCtx_t hzlCtx0 =
{
    .clientConfig = &clientConfig,
    .groupConfigs = groupConfigs,
    .groupStates = groupStates,
};
