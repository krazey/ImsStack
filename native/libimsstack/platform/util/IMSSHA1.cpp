/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20120217  hwangoo.park@             Created
    </table>

    Description
     This file implements the secure hash algorithm.
*/

#include "IMSSHA1.h"

// Fxx are a basic SHA1 functions
// 0 <= t <= 19
#define F00(b, c, d) (((b) & (c)) | ((~b) & (d)))
#define K00          (0x5a827999)
// 20 <= t <= 39
#define F20(b, c, d) ((b) ^ (c) ^ (d))
#define K20          (0x6ed9eba1)
// 40 <= t <= 59
#define F40(b, c, d) (((b) & (c)) | ((b) & (d)) | ((c) & (d)))
#define K40          (0x8f1bbcdc)
// 60 <= t <= 79
#define F60(b, c, d) ((b) ^ (c) ^ (d))
#define K60          (0xca62c1d6)

// ROTATE_LEFT rotates x left n bits.
#define ROTATE_LEFT(x, n) ((((x) << (n)) & 0xFFFFFFFF) | ((x) >> (32 - (n))))

// Calculate A,B,C,D,E
#define FF(a, b, c, d, e, bcd, w, k, temp)                                               \
    {                                                                                    \
        (temp) = ROTATE_LEFT((a), 5) + (bcd) + (e) + (w) + (static_cast<IMS_UINT32>(k)); \
        (e) = (d);                                                                       \
        (d) = (c);                                                                       \
        (c) = ROTATE_LEFT((b), 30);                                                      \
        (b) = (a);                                                                       \
        (a) = (temp)&0xFFFFFFFF;                                                         \
    }

LOCAL const IMS_UCHAR SHA1_PADDING[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

LOCAL void imsSHA1_CopyMemory(
        OUT IMS_UCHAR* pucDest, IN CONST IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen);
LOCAL void imsSHA1_Decode(
        IN CONST IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen, OUT IMS_UINT32* pnDest);
LOCAL void imsSHA1_Encode(
        IN CONST IMS_UINT32* pnSrc, IN IMS_UINT32 nDestLen, OUT IMS_UCHAR* pucDest);
LOCAL void imsSHA1_SetMemory(OUT IMS_UCHAR* pucDest, IN IMS_SINT32 nValue, IN IMS_SIZE_T nCount);
LOCAL void imsSHA1_Transform(IN IMS_UINT32 anH[5], IN IMS_UCHAR aucMessageBlock[64]);

/*
This function initializes MD5 context structure.
It begins an MD5 operation, writing a new context.

Remarks

Parameters
<table>
parameter               description
----------              ----------
pstContext              Pointer to MD5 context to be initialized
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
GLOBAL void IMSSHA1_Initialize(OUT SHA1Context* pstContext)
{
    if (pstContext != IMS_NULL)
    {
        pstContext->nLengthLow = 0;
        pstContext->nLengthHigh = 0;

        // Load magic initialization constants
        pstContext->anH[0] = 0x67452301;
        pstContext->anH[1] = 0xefcdab89;
        pstContext->anH[2] = 0x98badcfe;
        pstContext->anH[3] = 0x10325476;
        pstContext->anH[4] = 0xc3d2e1f0;
    }
}

/*
This function updates an MD5 block. It continues an MD5 message-digest operation,
processing another message block, and updating the context.

Remarks

Parameters
<table>
parameter               description
----------              ----------
pucSrc                  Pointer to the source data to be updated
nSrcLen                 Length of the source data (pucSrc)
pstContext              Pointer to MD5 context to be updated
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
GLOBAL void IMSSHA1_Update(
        IN CONST IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen, IN_OUT SHA1Context* pstContext)
{
    IMS_UINT32 nProcIndex;
    IMS_UINT32 nIndex;
    IMS_UINT32 nPartLen;

    // Computes the number of bytes mod 64
    nIndex = static_cast<IMS_UINT32>((pstContext->nLengthLow >> 3) & 0x3F);

    // Updates the number of bits
    if ((pstContext->nLengthLow += static_cast<IMS_UINT32>(nSrcLen << 3)) <
            (static_cast<IMS_UINT32>(nSrcLen << 3)))
    {
        (pstContext->nLengthHigh)++;
    }

    pstContext->nLengthHigh += static_cast<IMS_UINT32>(nSrcLen >> 29);

    nPartLen = 64 - nIndex;

    // Transforms as many times as possible.
    if (nSrcLen >= nPartLen)
    {
        imsSHA1_CopyMemory(&(pstContext->aucMessageBlock[nIndex]), pucSrc, nPartLen);
        imsSHA1_Transform(pstContext->anH, pstContext->aucMessageBlock);

        for (nProcIndex = nPartLen; nProcIndex + 63 < nSrcLen; nProcIndex += 64)
        {
            imsSHA1_Transform(pstContext->anH, const_cast<IMS_UCHAR*>(&pucSrc[nProcIndex]));
        }

        nIndex = 0;
    }
    else
    {
        nProcIndex = 0;
    }

    // Process the remaining input buffer
    imsSHA1_CopyMemory(
            &(pstContext->aucMessageBlock[nIndex]), &pucSrc[nProcIndex], nSrcLen - nProcIndex);
}

/*
This function finalizes an MD5 operation. It ends an MD5 message-digest operation,
writing the message digest and zeroing the context.

Remarks

Parameters
<table>
parameter               description
----------              ----------
pstContext              Pointer to MD5 context to be finalized
aucDigest               Array of the MD5 result
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
GLOBAL void IMSSHA1_Finalize(IN_OUT SHA1Context* pstContext, OUT IMS_UCHAR aucHash[20])
{
    IMS_UCHAR aucBits[8];
    IMS_UINT32 nIndex;
    IMS_UINT32 nPadLen;

    // Save the number of bits
    imsSHA1_Encode(&pstContext->nLengthHigh, 4, aucBits);
    imsSHA1_Encode(&pstContext->nLengthLow, 4, &aucBits[4]);

    // Pad out to 56 mod 64
    nIndex = static_cast<IMS_UINT32>((pstContext->nLengthLow >> 3) & 0x3F);
    nPadLen = (nIndex < 56) ? (56 - nIndex) : (120 - nIndex);

    IMSSHA1_Update(SHA1_PADDING, nPadLen, pstContext);

    // Append the length (before padding)
    IMSSHA1_Update(aucBits, 8, pstContext);

    // Store state in digest
    imsSHA1_Encode(pstContext->anH, 20, aucHash);

    // Zeroize sensitive information
    imsSHA1_SetMemory(reinterpret_cast<IMS_UCHAR*>(pstContext), 0, sizeof(SHA1Context));
}

LOCAL void imsSHA1_CopyMemory(
        OUT IMS_UCHAR* pucDest, IN CONST IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen)
{
    for (IMS_UINT32 i = 0; i < nSrcLen; i++)
    {
        pucDest[i] = pucSrc[i];
    }
}

// Decodes the input (IMS_UCHAR) into output (IMS_UINT32). Assumes nSrcLen is a multiple of 4.
LOCAL void imsSHA1_Decode(IN CONST IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen, OUT IMS_UINT32* pnDest)
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
LOCAL void imsSHA1_Encode(
        IN CONST IMS_UINT32* pnSrc, IN IMS_UINT32 nDestLen, OUT IMS_UCHAR* pucDest)
{
    for (IMS_UINT32 i = 0, j = 0; j < nDestLen; i++, j += 4)
    {
        pucDest[j] = static_cast<IMS_UCHAR>((pnSrc[i] >> 24) & 0xFF);
        pucDest[j + 1] = static_cast<IMS_UCHAR>((pnSrc[i] >> 16) & 0xFF);
        pucDest[j + 2] = static_cast<IMS_UCHAR>((pnSrc[i] >> 8) & 0xFF);
        pucDest[j + 3] = static_cast<IMS_UCHAR>((pnSrc[i]) & 0xFF);
    }
}

LOCAL void imsSHA1_SetMemory(OUT IMS_UCHAR* pucDest, IN IMS_SINT32 nValue, IN IMS_SIZE_T nCount)
{
    for (IMS_SIZE_T i = 0; i < nCount; i++)
    {
        pucDest[i] = static_cast<IMS_CHAR>(nValue);
    }
}

LOCAL void imsSHA1_Transform(IN_OUT IMS_UINT32 anH[5], IN IMS_UCHAR aucMessageBlock[64])
{
    IMS_UINT32 A = anH[0];
    IMS_UINT32 B = anH[1];
    IMS_UINT32 C = anH[2];
    IMS_UINT32 D = anH[3];
    IMS_UINT32 E = anH[4];
    IMS_UINT32 W[80];
    IMS_UINT32 TEMP;
    IMS_SINT32 t;

    imsSHA1_Decode(aucMessageBlock, 64, W);

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

    anH[0] = (anH[0] + A) & 0xFFFFFFFF;
    anH[1] = (anH[1] + B) & 0xFFFFFFFF;
    anH[2] = (anH[2] + C) & 0xFFFFFFFF;
    anH[3] = (anH[3] + D) & 0xFFFFFFFF;
    anH[4] = (anH[4] + E) & 0xFFFFFFFF;
}
