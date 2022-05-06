/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090303  toastops@                 Porting (From Aricent SIP stack)
    </table>

    Description
     This file implements an MD5 operations.
*/

#include "IMSMD5.h"

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

// F, G, H and I are a basic MD5 functions
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

// ROTATE_LEFT rotates x left n bits.
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.
#define FF(a, b, c, d, x, s, ac)                                       \
    do                                                                 \
    {                                                                  \
        (a) += F((b), (c), (d)) + (x) + (static_cast<IMS_UINT32>(ac)); \
        (a) = ROTATE_LEFT((a), (s));                                   \
        (a) += (b);                                                    \
    } while (0)

#define GG(a, b, c, d, x, s, ac)                                       \
    do                                                                 \
    {                                                                  \
        (a) += G((b), (c), (d)) + (x) + (static_cast<IMS_UINT32>(ac)); \
        (a) = ROTATE_LEFT((a), (s));                                   \
        (a) += (b);                                                    \
    } while (0)

#define HH(a, b, c, d, x, s, ac)                                       \
    do                                                                 \
    {                                                                  \
        (a) += H((b), (c), (d)) + (x) + (static_cast<IMS_UINT32>(ac)); \
        (a) = ROTATE_LEFT((a), (s));                                   \
        (a) += (b);                                                    \
    } while (0)

#define II(a, b, c, d, x, s, ac)                                       \
    do                                                                 \
    {                                                                  \
        (a) += I((b), (c), (d)) + (x) + (static_cast<IMS_UINT32>(ac)); \
        (a) = ROTATE_LEFT((a), (s));                                   \
        (a) += (b);                                                    \
    } while (0)

LOCAL const IMS_UCHAR MD5_PADDING[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

LOCAL void imsMD5_CopyMemory(
        OUT IMS_UCHAR* pucDest, IN CONST IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen);
LOCAL void imsMD5_Decode(IN CONST IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen, OUT IMS_UINT32* pnDest);
LOCAL void imsMD5_Encode(
        IN CONST IMS_UINT32* pnSrc, IN IMS_UINT32 nDestLen, OUT IMS_UCHAR* pucDest);
LOCAL void imsMD5_SetMemory(OUT IMS_UCHAR* pucDest, IN IMS_SINT32 nValue, IN IMS_SIZE_T nCount);
LOCAL void imsMD5_Transform(IN IMS_UINT32 anState[4], OUT IMS_UCHAR aucBlock[64]);

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
GLOBAL void IMSMD5_Initialize(OUT MD5Context* pstContext)
{
    if (pstContext != IMS_NULL)
    {
        pstContext->anCount[0] = pstContext->anCount[1] = 0;

        // Load magic initialization constants
        pstContext->anState[0] = 0x67452301;
        pstContext->anState[1] = 0xefcdab89;
        pstContext->anState[2] = 0x98badcfe;
        pstContext->anState[3] = 0x10325476;
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
GLOBAL void IMSMD5_Update(
        IN CONST IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen, IN_OUT MD5Context* pstContext)
{
    IMS_UINT32 nProcIndex;
    IMS_UINT32 nIndex;
    IMS_UINT32 nPartLen;

    // Computes the number of bytes mod 64
    nIndex = static_cast<IMS_UINT32>((pstContext->anCount[0] >> 3) & 0x3F);

    // Updates the number of bits
    if ((pstContext->anCount[0] += static_cast<IMS_UINT32>(nSrcLen << 3)) <
            (static_cast<IMS_UINT32>(nSrcLen << 3)))
    {
        (pstContext->anCount[1])++;
    }

    pstContext->anCount[1] += static_cast<IMS_UINT32>(nSrcLen >> 29);

    nPartLen = 64 - nIndex;

    // Transforms as many times as possible.
    if (nSrcLen >= nPartLen)
    {
        imsMD5_CopyMemory(&(pstContext->aucBuffer[nIndex]), pucSrc, nPartLen);
        imsMD5_Transform(pstContext->anState, pstContext->aucBuffer);

        for (nProcIndex = nPartLen; nProcIndex + 63 < nSrcLen; nProcIndex += 64)
        {
            imsMD5_Transform(pstContext->anState, const_cast<IMS_UCHAR*>(&pucSrc[nProcIndex]));
        }

        nIndex = 0;
    }
    else
    {
        nProcIndex = 0;
    }

    // Process the remaining input buffer
    imsMD5_CopyMemory(&(pstContext->aucBuffer[nIndex]), &pucSrc[nProcIndex], nSrcLen - nProcIndex);
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
GLOBAL void IMSMD5_Finalize(IN_OUT MD5Context* pstContext, OUT IMS_UCHAR aucDigest[16])
{
    IMS_UCHAR aucBits[8];
    IMS_UINT32 nIndex;
    IMS_UINT32 nPadLen;

    // Save the number of bits
    imsMD5_Encode(pstContext->anCount, 8, aucBits);

    // Pad out to 56 mod 64
    nIndex = static_cast<IMS_UINT32>((pstContext->anCount[0] >> 3) & 0x3F);
    nPadLen = (nIndex < 56) ? (56 - nIndex) : (120 - nIndex);

    IMSMD5_Update(MD5_PADDING, nPadLen, pstContext);

    // Append the length (before padding)
    IMSMD5_Update(aucBits, 8, pstContext);

    // Store state in digest
    imsMD5_Encode(pstContext->anState, 16, aucDigest);

    // Zeroize sensitive information
    imsMD5_SetMemory(reinterpret_cast<IMS_UCHAR*>(pstContext), 0, sizeof(MD5Context));
}

LOCAL void imsMD5_CopyMemory(
        OUT IMS_UCHAR* pucDest, IN CONST IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen)
{
    for (IMS_UINT32 i = 0; i < nSrcLen; i++)
    {
        pucDest[i] = pucSrc[i];
    }
}

// Decodes the input (IMS_UCHAR) into output (IMS_UINT32). Assumes nSrcLen is a multiple of 4.
LOCAL void imsMD5_Decode(IN CONST IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen, OUT IMS_UINT32* pnDest)
{
    for (IMS_UINT32 i = 0, j = 0; j < nSrcLen; i++, j += 4)
    {
        pnDest[i] = static_cast<IMS_UINT32>(pucSrc[j]);
        pnDest[i] |= static_cast<IMS_UINT32>(pucSrc[j + 1] << 8);
        pnDest[i] |= static_cast<IMS_UINT32>(pucSrc[j + 2] << 16);
        pnDest[i] |= static_cast<IMS_UINT32>(pucSrc[j + 3] << 24);
    }
}

// Encodes input (IMS_UINT32) into output (IMS_UCHAR). Assumes nSrcLen is a multiple of 4.
LOCAL void imsMD5_Encode(IN CONST IMS_UINT32* pnSrc, IN IMS_UINT32 nDestLen, OUT IMS_UCHAR* pucDest)
{
    for (IMS_UINT32 i = 0, j = 0; j < nDestLen; i++, j += 4)
    {
        pucDest[j] = static_cast<IMS_UCHAR>(pnSrc[i] & 0xFF);
        pucDest[j + 1] = static_cast<IMS_UCHAR>((pnSrc[i] >> 8) & 0xFF);
        pucDest[j + 2] = static_cast<IMS_UCHAR>((pnSrc[i] >> 16) & 0xFF);
        pucDest[j + 3] = static_cast<IMS_UCHAR>((pnSrc[i] >> 24) & 0xFF);
    }
}

LOCAL void imsMD5_SetMemory(OUT IMS_UCHAR* pucDest, IN IMS_SINT32 nValue, IN IMS_SIZE_T nCount)
{
    for (IMS_SIZE_T i = 0; i < nCount; i++)
    {
        pucDest[i] = static_cast<IMS_CHAR>(nValue);
    }
}

LOCAL void imsMD5_Transform(IN IMS_UINT32 anState[4], OUT IMS_UCHAR aucBlock[64])
{
    IMS_UINT32 a = anState[0];
    IMS_UINT32 b = anState[1];
    IMS_UINT32 c = anState[2];
    IMS_UINT32 d = anState[3];
    IMS_UINT32 x[16];

    imsMD5_Decode(aucBlock, 64, x);

    // Round 1
    FF(a, b, c, d, x[0], S11, 0xd76aa478);  /* 1 */
    FF(d, a, b, c, x[1], S12, 0xe8c7b756);  /* 2 */
    FF(c, d, a, b, x[2], S13, 0x242070db);  /* 3 */
    FF(b, c, d, a, x[3], S14, 0xc1bdceee);  /* 4 */
    FF(a, b, c, d, x[4], S11, 0xf57c0faf);  /* 5 */
    FF(d, a, b, c, x[5], S12, 0x4787c62a);  /* 6 */
    FF(c, d, a, b, x[6], S13, 0xa8304613);  /* 7 */
    FF(b, c, d, a, x[7], S14, 0xfd469501);  /* 8 */
    FF(a, b, c, d, x[8], S11, 0x698098d8);  /* 9 */
    FF(d, a, b, c, x[9], S12, 0x8b44f7af);  /* 10 */
    FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
    FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
    FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
    FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
    FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
    FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

    // Round 2
    GG(a, b, c, d, x[1], S21, 0xf61e2562);  /* 17 */
    GG(d, a, b, c, x[6], S22, 0xc040b340);  /* 18 */
    GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
    GG(b, c, d, a, x[0], S24, 0xe9b6c7aa);  /* 20 */
    GG(a, b, c, d, x[5], S21, 0xd62f105d);  /* 21 */
    GG(d, a, b, c, x[10], S22, 0x2441453);  /* 22 */
    GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
    GG(b, c, d, a, x[4], S24, 0xe7d3fbc8);  /* 24 */
    GG(a, b, c, d, x[9], S21, 0x21e1cde6);  /* 25 */
    GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
    GG(c, d, a, b, x[3], S23, 0xf4d50d87);  /* 27 */
    GG(b, c, d, a, x[8], S24, 0x455a14ed);  /* 28 */
    GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
    GG(d, a, b, c, x[2], S22, 0xfcefa3f8);  /* 30 */
    GG(c, d, a, b, x[7], S23, 0x676f02d9);  /* 31 */
    GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    // Round 3
    HH(a, b, c, d, x[5], S31, 0xfffa3942);  /* 33 */
    HH(d, a, b, c, x[8], S32, 0x8771f681);  /* 34 */
    HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
    HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
    HH(a, b, c, d, x[1], S31, 0xa4beea44);  /* 37 */
    HH(d, a, b, c, x[4], S32, 0x4bdecfa9);  /* 38 */
    HH(c, d, a, b, x[7], S33, 0xf6bb4b60);  /* 39 */
    HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
    HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
    HH(d, a, b, c, x[0], S32, 0xeaa127fa);  /* 42 */
    HH(c, d, a, b, x[3], S33, 0xd4ef3085);  /* 43 */
    HH(b, c, d, a, x[6], S34, 0x4881d05);   /* 44 */
    HH(a, b, c, d, x[9], S31, 0xd9d4d039);  /* 45 */
    HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
    HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
    HH(b, c, d, a, x[2], S34, 0xc4ac5665);  /* 48 */

    // Round 4
    II(a, b, c, d, x[0], S41, 0xf4292244);  /* 49 */
    II(d, a, b, c, x[7], S42, 0x432aff97);  /* 50 */
    II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
    II(b, c, d, a, x[5], S44, 0xfc93a039);  /* 52 */
    II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
    II(d, a, b, c, x[3], S42, 0x8f0ccc92);  /* 54 */
    II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
    II(b, c, d, a, x[1], S44, 0x85845dd1);  /* 56 */
    II(a, b, c, d, x[8], S41, 0x6fa87e4f);  /* 57 */
    II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
    II(c, d, a, b, x[6], S43, 0xa3014314);  /* 59 */
    II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
    II(a, b, c, d, x[4], S41, 0xf7537e82);  /* 61 */
    II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
    II(c, d, a, b, x[2], S43, 0x2ad7d2bb);  /* 63 */
    II(b, c, d, a, x[9], S44, 0xeb86d391);  /* 64 */

    anState[0] += a;
    anState[1] += b;
    anState[2] += c;
    anState[3] += d;

    // Zeroize sensitive information
    imsMD5_SetMemory(reinterpret_cast<IMS_UCHAR*>(x), 0, sizeof(x));
}
