/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ImsHmac.h"
#include "ImsMd5.h"
#include "ImsSha1.h"
#include "ServiceMemory.h"

// The byte-length of data blocks
#define HMAC_B      64
// The byte-length of hash outputs for MD5
#define HMAC_L_MD5  16
// The byte-length of hash outputs for SHA-1
#define HMAC_L_SHA1 20

GLOBAL void ImsHmac_Md5(IN const IMS_UCHAR* pucText, IN IMS_SINT32 nTextLen,
        IN const IMS_UCHAR* pucKey, IN IMS_SINT32 nKeyLen,
        OUT IMS_UCHAR aucDigest[IMS_HMAC_MD5_SIZE])
{
    IMS_UCHAR aucKey_Temp[HMAC_L_MD5];

    // If key is longer than 64 bytes, reset it to key = MD5(key)
    if (nKeyLen > HMAC_B)
    {
        ImsMd5Context objContext;

        ImsMd5_Initialize(&objContext);
        ImsMd5_Update(pucKey, nKeyLen, &objContext);
        ImsMd5_Finalize(&objContext, aucKey_Temp);

        pucKey = aucKey_Temp;
        nKeyLen = HMAC_L_MD5;
    }

    ImsMd5Context objContext;
    // Inner padding - key XORd with ipad
    IMS_UCHAR aucKey_IPad[HMAC_B + 1];
    // Outer padding - key XORd with opad
    IMS_UCHAR aucKey_OPad[HMAC_B + 1];

    // The HMAC_MD5 transform looks like:
    //    MD5 (K XOR opad, MD5 (K XOR ipad, text))
    //
    //    where K is an n byte key
    //    ipad is the byte 0x36 repeated 64 times
    //    opad is the byte 0x5c repeated 64 times
    //    and text is the data being protected

    // Start out by storing key in pads
    IMS_MEM_Memset(aucKey_IPad, 0, sizeof(aucKey_IPad));
    IMS_MEM_Memset(aucKey_OPad, 0, sizeof(aucKey_OPad));

    IMS_MEM_Memcpy(aucKey_IPad, pucKey, nKeyLen);
    IMS_MEM_Memcpy(aucKey_OPad, pucKey, nKeyLen);

    // XOR key with ipad and opad values
    for (IMS_SINT32 i = 0; i < HMAC_B; ++i)
    {
        aucKey_IPad[i] ^= 0x36;
        aucKey_OPad[i] ^= 0x5c;
    }

    //// Performs inner MD5

    // Init context for 1st pass
    ImsMd5_Initialize(&objContext);
    // Starts with inner pad
    ImsMd5_Update(aucKey_IPad, HMAC_B, &objContext);
    // then text of datagram
    ImsMd5_Update(pucText, nTextLen, &objContext);
    // Finish up 1st pass
    ImsMd5_Finalize(&objContext, aucDigest);

    //// Performs outer MD5

    // Init context for 2nd pass
    ImsMd5_Initialize(&objContext);
    // Starts with outer pad
    ImsMd5_Update(aucKey_OPad, HMAC_B, &objContext);
    // then results of 1st hash
    ImsMd5_Update(aucDigest, HMAC_L_MD5, &objContext);
    // Finish up 2nd pass
    ImsMd5_Finalize(&objContext, aucDigest);
}

GLOBAL void ImsHmac_Sha1(IN const IMS_UCHAR* pucText, IN IMS_SINT32 nTextLen,
        IN const IMS_UCHAR* pucKey, IN IMS_SINT32 nKeyLen,
        OUT IMS_UCHAR aucHash[IMS_HMAC_SHA1_SIZE])
{
    IMS_UCHAR aucKey_Temp[HMAC_L_SHA1];

    // If key is longer than 64 bytes, reset it to key = SHA-1(key)
    if (nKeyLen > HMAC_B)
    {
        ImsSha1Context objContext;

        ImsSha1_Initialize(&objContext);
        ImsSha1_Update(pucKey, nKeyLen, &objContext);
        ImsSha1_Finalize(&objContext, aucKey_Temp);

        pucKey = aucKey_Temp;
        nKeyLen = HMAC_L_SHA1;
    }

    ImsSha1Context objContext;
    // Inner padding - key XORd with ipad
    IMS_UCHAR aucKey_IPad[HMAC_B + 1];
    // Outer padding - key XORd with opad
    IMS_UCHAR aucKey_OPad[HMAC_B + 1];

    // The HMAC-SHA-1 transform looks like:
    //    SHA-1 (K XOR opad, SHA-1 (K XOR ipad, text))
    //
    //    where K is an n byte key
    //    ipad is the byte 0x36 repeated 64 times
    //    opad is the byte 0x5c repeated 64 times
    //    and text is the data being protected

    // Start out by storing key in pads
    IMS_MEM_Memset(aucKey_IPad, 0, sizeof(aucKey_IPad));
    IMS_MEM_Memset(aucKey_OPad, 0, sizeof(aucKey_OPad));

    IMS_MEM_Memcpy(aucKey_IPad, pucKey, nKeyLen);
    IMS_MEM_Memcpy(aucKey_OPad, pucKey, nKeyLen);

    // XOR key with ipad and opad values
    for (IMS_SINT32 i = 0; i < HMAC_B; ++i)
    {
        aucKey_IPad[i] ^= 0x36;
        aucKey_OPad[i] ^= 0x5c;
    }

    //// Performs inner SHA-1

    // Init context for 1st pass
    ImsSha1_Initialize(&objContext);
    // Starts with inner pad
    ImsSha1_Update(aucKey_IPad, HMAC_B, &objContext);
    // then text of datagram
    ImsSha1_Update(pucText, nTextLen, &objContext);
    // Finish up 1st pass
    ImsSha1_Finalize(&objContext, aucHash);

    //// Performs outer SHA-1

    // Init context for 2nd pass
    ImsSha1_Initialize(&objContext);
    // Starts with outer pad
    ImsSha1_Update(aucKey_OPad, HMAC_B, &objContext);
    // then results of 1st hash
    ImsSha1_Update(aucHash, HMAC_L_SHA1, &objContext);
    // Finish up 2nd pass
    ImsSha1_Finalize(&objContext, aucHash);
}
