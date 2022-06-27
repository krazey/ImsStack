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
#include "ImsBase64.h"

#define ADD_CRLF(p, n)           \
    do                           \
    {                            \
        if ((n) == MAX_LINE_LEN) \
        {                        \
            (n) = 0;             \
            (*(p)) = CR;         \
            (p)++;               \
            (*(p)) = LF;         \
            (p)++;               \
        }                        \
    } while (0)

#define MAX_LINE_LEN 76

// Default value for non-printable characters
#define NP           0xFF
// Carriage-Return (\r)
#define CR           0x0D
// Line-Feed (\n)
#define LF           0x0A
// Padding character for Base64
#define BASE64_PAD   '='

// Constant table for Base64 value encoding / decoding
static const IMS_CHAR BASE64_ENCODING_TABLE[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const IMS_UCHAR BASE64_DECODING_TABLE[] = {
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 0 ~ 9
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 10 ~ 19
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 20 ~ 29
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 30 ~ 39
        NP, NP, NP, 62, NP, NP, NP, 63, 52, 53,  // 40 ~ 49
        54, 55, 56, 57, 58, 59, 60, 61, NP, NP,  // 50 ~ 59
        NP, NP, NP, NP, NP, 0, 1, 2, 3, 4,       // 60 ~ 69
        5, 6, 7, 8, 9, 10, 11, 12, 13, 14,       // 70 ~ 79
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24,  // 80 ~ 89
        25, NP, NP, NP, NP, NP, NP, 26, 27, 28,  // 90 ~ 99
        29, 30, 31, 32, 33, 34, 35, 36, 37, 38,  // 100 ~ 109
        39, 40, 41, 42, 43, 44, 45, 46, 47, 48,  // 110 ~ 119
        49, 50, 51, NP, NP, NP, NP, NP, NP, NP,  // 120 ~ 129
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 130 ~ 139
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 140 ~ 149
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 150 ~ 159
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 160 ~ 169
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 170 ~ 179
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 180 ~ 189
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 190 ~ 199
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 200 ~ 209
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 210 ~ 219
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 220 ~ 229
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 230 ~ 239
        NP, NP, NP, NP, NP, NP, NP, NP, NP, NP,  // 240 ~ 249
        NP, NP, NP, NP, NP, NP                   // 250 ~ 256
};

GLOBAL IMS_SINT32 ImsBase64_Encode(IN IMS_BYTE* pSrcData, IN IMS_SIZE_T nSrcLen,
        IN_OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestLen, IN IMS_BOOL bAddCrlf /*= IMS_TRUE*/)
{
    IMS_CHAR c6bit;
    IMS_CHAR* pEncBuffer = pszDest;

    (void)nDestLen;

    if (bAddCrlf == IMS_TRUE)
    {
        IMS_SINT32 nLineCount = 0;

        for (IMS_SIZE_T nPos = 0; nPos < nSrcLen; ++nPos)
        {
            c6bit = (pSrcData[nPos] >> 2) & 0x3F;
            (*pEncBuffer) = BASE64_ENCODING_TABLE[(IMS_UINT8)c6bit];
            pEncBuffer++;
            nLineCount++;
            ADD_CRLF(pEncBuffer, nLineCount);

            c6bit = (pSrcData[nPos] << 4) & 0x3F;

            if (++nPos < nSrcLen)
            {
                c6bit |= (pSrcData[nPos] >> 4) & 0x0F;
            }

            (*pEncBuffer) = BASE64_ENCODING_TABLE[(IMS_UINT8)c6bit];
            pEncBuffer++;
            nLineCount++;
            ADD_CRLF(pEncBuffer, nLineCount);

            if (nPos < nSrcLen)
            {
                c6bit = (pSrcData[nPos] << 2) & 0x3F;

                if (++nPos < nSrcLen)
                {
                    c6bit |= (pSrcData[nPos] >> 6) & 0x03;
                }

                (*pEncBuffer) = BASE64_ENCODING_TABLE[(IMS_UINT8)c6bit];
                pEncBuffer++;
                nLineCount++;
                ADD_CRLF(pEncBuffer, nLineCount);
            }
            else
            {
                ++nPos;
                (*pEncBuffer) = BASE64_PAD;
                pEncBuffer++;
                nLineCount++;
                ADD_CRLF(pEncBuffer, nLineCount);
            }

            if (nPos < nSrcLen)
            {
                c6bit = pSrcData[nPos] & 0x3F;
                (*pEncBuffer) = BASE64_ENCODING_TABLE[(IMS_UINT8)c6bit];
                pEncBuffer++;
                nLineCount++;
                ADD_CRLF(pEncBuffer, nLineCount);
            }
            else
            {
                (*pEncBuffer) = BASE64_PAD;
                pEncBuffer++;
                nLineCount++;
                ADD_CRLF(pEncBuffer, nLineCount);
            }
        }
    }
    else
    {
        for (IMS_SIZE_T nPos = 0; nPos < nSrcLen; ++nPos)
        {
            c6bit = (pSrcData[nPos] >> 2) & 0x3F;
            (*pEncBuffer) = BASE64_ENCODING_TABLE[(IMS_UINT8)c6bit];
            pEncBuffer++;

            c6bit = (pSrcData[nPos] << 4) & 0x3F;

            if (++nPos < nSrcLen)
            {
                c6bit |= (pSrcData[nPos] >> 4) & 0x0F;
            }

            (*pEncBuffer) = BASE64_ENCODING_TABLE[(IMS_UINT8)c6bit];
            pEncBuffer++;

            if (nPos < nSrcLen)
            {
                c6bit = (pSrcData[nPos] << 2) & 0x3F;

                if (++nPos < nSrcLen)
                {
                    c6bit |= (pSrcData[nPos] >> 6) & 0x03;
                }

                (*pEncBuffer) = BASE64_ENCODING_TABLE[(IMS_UINT8)c6bit];
                pEncBuffer++;
            }
            else
            {
                ++nPos;
                (*pEncBuffer) = BASE64_PAD;
                pEncBuffer++;
            }

            if (nPos < nSrcLen)
            {
                c6bit = pSrcData[nPos] & 0x3F;
                (*pEncBuffer) = BASE64_ENCODING_TABLE[(IMS_UINT8)c6bit];
                pEncBuffer++;
            }
            else
            {
                (*pEncBuffer) = BASE64_PAD;
                pEncBuffer++;
            }
        }
    }

    (*pEncBuffer) = 0x00;

    return (IMS_SINT32)LONG_TO_INT(pEncBuffer - pszDest);
}

GLOBAL IMS_SINT32 ImsBase64_Decode(
        IN IMS_CHAR* pszSrcData, IN IMS_SIZE_T nSrcLen, OUT IMS_BYTE* pDest, IN IMS_SIZE_T nDestLen)
{
    IMS_CHAR c8bit;
    IMS_CHAR c8bit1;
    IMS_BYTE* pDecBuffer = pDest;

    (void)nDestLen;

    for (IMS_SIZE_T nPos = 0; nPos < nSrcLen; ++nPos)
    {
        if (pszSrcData[nPos] == LF)
        {
            nPos += 1;
        }

        if (pszSrcData[nPos] == CR)
        {
            nPos += 2;
        }

        c8bit = (IMS_CHAR)BASE64_DECODING_TABLE[(IMS_UINT8)pszSrcData[nPos]];
        ++nPos;

        if (pszSrcData[nPos] == LF)
        {
            nPos += 1;
        }

        if (pszSrcData[nPos] == CR)
        {
            nPos += 2;
        }

        c8bit1 = (IMS_CHAR)BASE64_DECODING_TABLE[(IMS_UINT8)pszSrcData[nPos]];
        c8bit = (c8bit << 2) | ((c8bit1 >> 4) & 0x03);
        (*pDecBuffer) = c8bit;
        pDecBuffer++;

        if (++nPos < nSrcLen)
        {
            if (pszSrcData[nPos] == LF)
            {
                nPos += 1;
            }

            if (pszSrcData[nPos] == CR)
            {
                nPos += 2;
            }

            c8bit = pszSrcData[nPos];
            if (c8bit == BASE64_PAD)
            {
                break;
            }

            c8bit = (IMS_CHAR)BASE64_DECODING_TABLE[(IMS_UINT8)pszSrcData[nPos]];
            c8bit1 = ((c8bit1 << 4) & 0xF0) | ((c8bit >> 2) & 0x0F);
            (*pDecBuffer) = c8bit1;
            pDecBuffer++;
        }

        if (++nPos < nSrcLen)
        {
            if (pszSrcData[nPos] == LF)
            {
                nPos += 1;
            }

            if (pszSrcData[nPos] == CR)
            {
                nPos += 2;
            }

            c8bit1 = pszSrcData[nPos];
            if (c8bit1 == BASE64_PAD)
            {
                break;
            }

            c8bit1 = (IMS_CHAR)BASE64_DECODING_TABLE[(IMS_UINT8)pszSrcData[nPos]];
            c8bit = ((c8bit << 6) & 0xC0) | c8bit1;
            (*pDecBuffer) = c8bit;
            pDecBuffer++;
        }
    }

    return (IMS_SINT32)LONG_TO_INT(pDecBuffer - pDest);
}
