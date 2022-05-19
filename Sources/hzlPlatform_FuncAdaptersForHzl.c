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
 * Wrappers for the lower level IO operations required by the Hazelnet library.
 *
 * This includes the TRNG and the current time functions adapted into functions with the proper
 * signatures that the Hazelnet library (both Client and Server) expects.
 */

#include "hzlPlatform.h"
#include "hzl.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/**
 * @internal
 * The RNG generates 16 bytes (128 bits) at the time, but HZL requires an arbitrary amount, so we
 * pass one block at the time to the output buffer and part of the last block to reach any amount.
 */
hzl_Err_t
hzlPlatform_HzlAdapterTrng(uint8_t* bytes, size_t amount)
{
    size_t remainingAmountInThisBlock;
    uint8_t block[16];
    while (amount)
    {
        status_t status = CSEC_DRV_GenerateRND(block);
        if (status != STATUS_SUCCESS)
        {
            return HZL_ERR_CANNOT_GENERATE_RANDOM;
        }
        remainingAmountInThisBlock = MIN(amount, sizeof(block));
        memcpy(bytes, block, remainingAmountInThisBlock);
        bytes += remainingAmountInThisBlock;
        amount -= remainingAmountInThisBlock;
    }
    return HZL_OK;
}

/**
 * @internal
 * As FreeRTOS's tick has the resolution of 1 ms and it's a rolling counter, that is just
 * enough for the Hazelnet library. We are simply using said counter directly.
 *
 * MUST be used from WITHIN a task.
 */
hzl_Err_t
hzlPlatform_HzlAdapterCurrentTime(hzl_Timestamp_t* const timestamp)
{
    _Static_assert(sizeof(TickType_t) == sizeof(hzl_Timestamp_t),
        "FreeRTOS should use proper tick sizes for the timestamps of this demo.");
    *timestamp = xTaskGetTickCount();
    return HZL_OK;
}
