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
#include "ImsNew.h"
#include "ImsSha1.h"

// Fxx are a basic SHA1 functions
// 0 <= t <= 19
#define F00(b, c, d)      (((b) & (c)) | ((~(b)) & (d)))
#define K00               (0x5a827999)
// 20 <= t <= 39
#define F20(b, c, d)      ((b) ^ (c) ^ (d))
#define K20               (0x6ed9eba1)
// 40 <= t <= 59
#define F40(b, c, d)      (((b) & (c)) | ((b) & (d)) | ((c) & (d)))
#define K40               (0x8f1bbcdc)
// 60 <= t <= 79
#define F60(b, c, d)      ((b) ^ (c) ^ (d))
#define K60               (0xca62c1d6)

// ROTATE_LEFT rotates x left n bits.
#define ROTATE_LEFT(x, n) ((((x) << (n)) & 0xFFFFFFFF) | ((x) >> (32 - (n))))

// Calculate A,B,C,D,E
#define FF(a, b, c, d, e, bcd, w, k, temp)                                   \
    do                                                                       \
    {                                                                        \
        __builtin_add_overflow(ROTATE_LEFT((a), 5), (bcd), &(temp));         \
        __builtin_add_overflow((temp), (e), &(temp));                        \
        __builtin_add_overflow((temp), (w), &(temp));                        \
        __builtin_add_overflow((temp), static_cast<IMS_UINT32>(k), &(temp)); \
        (e) = (d);                                                           \
        (d) = (c);                                                           \
        (c) = ROTATE_LEFT((b), 30);                                          \
        (b) = (a);                                                           \
        (a) = (temp)&0xFFFFFFFF;                                             \
    } while (0)

// clang-format off
static const IMS_UCHAR SHA1_PADDING[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
// clang-format on

static void imsSha1_Decode(
        IN const IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen, OUT IMS_UINT32* pnDest);
static void imsSha1_Encode(
        IN const IMS_UINT32* pnSrc, IN IMS_UINT32 nDestLen, OUT IMS_UCHAR* pucDest);
static void imsSha1_SetMemory(OUT IMS_UCHAR* pucDest, IN IMS_SINT32 nValue, IN IMS_SIZE_T nCount);
static void imsSha1_Transform(IN IMS_UINT32 anH[5], IN IMS_UCHAR aucMessageBlock[64]);

GLOBAL void ImsSha1_Initialize(OUT ImsSha1Context* pContext)
{
    if (pContext != IMS_NULL)
    {
        pContext->nLengthLow = 0;
        pContext->nLengthHigh = 0;

        // Load magic initialization constants
        pContext->anH[0] = 0x67452301;
        pContext->anH[1] = 0xefcdab89;
        pContext->anH[2] = 0x98badcfe;
        pContext->anH[3] = 0x10325476;
        pContext->anH[4] = 0xc3d2e1f0;
    }
}

GLOBAL void ImsSha1_Update(
        IN const IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen, IN_OUT ImsSha1Context* pContext)
{
    IMS_UINT32 nProcIndex;
    IMS_UINT32 nIndex;
    IMS_UINT32 nPartLen;

    // Computes the number of bytes mod 64
    nIndex = static_cast<IMS_UINT32>((pContext->nLengthLow >> 3) & 0x3F);

    // Updates the number of bits
    if ((pContext->nLengthLow += static_cast<IMS_UINT32>(nSrcLen << 3)) <
            (static_cast<IMS_UINT32>(nSrcLen << 3)))
    {
        (pContext->nLengthHigh)++;
    }

    pContext->nLengthHigh += static_cast<IMS_UINT32>(nSrcLen >> 29);

    nPartLen = 64 - nIndex;

    // Transforms as many times as possible.
    if (nSrcLen >= nPartLen)
    {
        IMS_MEM_Memcpy(&(pContext->aucMessageBlock[nIndex]), pucSrc, nPartLen);
        imsSha1_Transform(pContext->anH, pContext->aucMessageBlock);

        for (nProcIndex = nPartLen; nProcIndex + 63 < nSrcLen; nProcIndex += 64)
        {
            imsSha1_Transform(pContext->anH, const_cast<IMS_UCHAR*>(&pucSrc[nProcIndex]));
        }

        nIndex = 0;
    }
    else
    {
        nProcIndex = 0;
    }

    // Process the remaining input buffer
    IMS_MEM_Memcpy(&(pContext->aucMessageBlock[nIndex]), &pucSrc[nProcIndex], nSrcLen - nProcIndex);
}

GLOBAL void ImsSha1_Finalize(
        IN_OUT ImsSha1Context* pContext, OUT IMS_UCHAR aucHash[IMS_SHA1_HASH_SIZE])
{
    IMS_UCHAR aucBits[8];
    IMS_UINT32 nIndex;
    IMS_UINT32 nPadLen;

    // Save the number of bits
    imsSha1_Encode(&pContext->nLengthHigh, 4, aucBits);
    imsSha1_Encode(&pContext->nLengthLow, 4, &aucBits[4]);

    // Pad out to 56 mod 64
    nIndex = static_cast<IMS_UINT32>((pContext->nLengthLow >> 3) & 0x3F);
    nPadLen = (nIndex < 56) ? (56 - nIndex) : (120 - nIndex);

    ImsSha1_Update(SHA1_PADDING, nPadLen, pContext);

    // Append the length (before padding)
    ImsSha1_Update(aucBits, 8, pContext);

    // Store state in digest
    imsSha1_Encode(pContext->anH, IMS_SHA1_HASH_SIZE, aucHash);

    // Zeroize sensitive information
    imsSha1_SetMemory(reinterpret_cast<IMS_UCHAR*>(pContext), 0, sizeof(ImsSha1Context));
}

// Decodes the input (IMS_UCHAR) into output (IMS_UINT32). Assumes nSrcLen is a multiple of 4.
static void imsSha1_Decode(
        IN const IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen, OUT IMS_UINT32* pnDest)
{
    for (IMS_UINT32 i = 0, j = 0; j < nSrcLen; i++, j += 4)
    {
        pnDest[i] = (static_cast<IMS_UINT32>(pucSrc[j]) << 24);
        pnDest[i] |= (static_cast<IMS_UINT32>(pucSrc[j + 1]) << 16);
        pnDest[i] |= (static_cast<IMS_UINT32>(pucSrc[j + 2]) << 8);
        pnDest[i] |= (static_cast<IMS_UINT32>(pucSrc[j + 3]));
    }
}

// Encodes input (IMS_UINT32) into output (IMS_UCHAR). Assumes nSrcLen is a multiple of 4.
static void imsSha1_Encode(
        IN const IMS_UINT32* pnSrc, IN IMS_UINT32 nDestLen, OUT IMS_UCHAR* pucDest)
{
    for (IMS_UINT32 i = 0, j = 0; j < nDestLen; i++, j += 4)
    {
        pucDest[j] = static_cast<IMS_UCHAR>((pnSrc[i] >> 24) & 0xFF);
        pucDest[j + 1] = static_cast<IMS_UCHAR>((pnSrc[i] >> 16) & 0xFF);
        pucDest[j + 2] = static_cast<IMS_UCHAR>((pnSrc[i] >> 8) & 0xFF);
        pucDest[j + 3] = static_cast<IMS_UCHAR>((pnSrc[i]) & 0xFF);
    }
}

static void imsSha1_SetMemory(OUT IMS_UCHAR* pucDest, IN IMS_SINT32 nValue, IN IMS_SIZE_T nCount)
{
    for (IMS_SIZE_T i = 0; i < nCount; i++)
    {
        pucDest[i] = static_cast<IMS_CHAR>(nValue);
    }
}

static void imsSha1_Transform(IN_OUT IMS_UINT32 anH[5], IN IMS_UCHAR aucMessageBlock[64])
{
    IMS_UINT32 A = anH[0];
    IMS_UINT32 B = anH[1];
    IMS_UINT32 C = anH[2];
    IMS_UINT32 D = anH[3];
    IMS_UINT32 E = anH[4];
    IMS_UINT32 W[80];
    IMS_UINT32 TEMP;
    IMS_SINT32 t;

    imsSha1_Decode(aucMessageBlock, 64, W);

    for (t = 16; t < 80; ++t)
    {
        W[t] = ROTATE_LEFT((W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16]), 1);
    }

    // 0 <= t <= 19
    for (t = 0; t < 20; ++t)
    {
        FF(A, B, C, D, E, F00(B, C, D), W[t], K00, TEMP);
    }

    // 20 <= t <= 39
    for (t = 20; t < 40; ++t)
    {
        FF(A, B, C, D, E, F20(B, C, D), W[t], K20, TEMP);
    }

    // 40 <= t <= 59
    for (t = 40; t < 60; ++t)
    {
        FF(A, B, C, D, E, F40(B, C, D), W[t], K40, TEMP);
    }

    // 60 <= t <= 79
    for (t = 60; t < 80; ++t)
    {
        FF(A, B, C, D, E, F60(B, C, D), W[t], K60, TEMP);
    }

    __builtin_add_overflow(anH[0], A, &anH[0]);
    __builtin_add_overflow(anH[1], B, &anH[1]);
    __builtin_add_overflow(anH[2], C, &anH[2]);
    __builtin_add_overflow(anH[3], D, &anH[3]);
    __builtin_add_overflow(anH[4], E, &anH[4]);
}
