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
#ifndef IMS_BASE64_H_
#define IMS_BASE64_H_

#include "ImsTypeDef.h"

/**
 * @brief This file implements the base64 encoding.
 *
 * It is designed to represent arbitrary sequences of octets in a form that allows the use of
 * both upper- and lowercase letters but that need not be human readable.
 * A 65-character subset of US-ASCII is used, enabling 6 bits to be represented per printable
 * character. (The extra 65th character, "=" is used to signify a special processing function.)
 */
#ifdef __cplusplus
extern "C"
{
    /**
     * @brief This function encodes an arbitrary source data as base64 encoded data.
     *
     * @param pSrcData The source data to be encoded
     * @param nSrcLen The length in bytes of source data to be encoded
     * @param pszDest The caller-allocated buffer to received the encoded data
     * @param nDestLen The length in characters of pszDest
     * @param bAddCrlf The flag specifying if the CRLF can be inserted or not
     * @return A length in characters of the encoded data if encoding was successful, 0 otherwise.
     */
    GLOBAL IMS_SINT32 ImsBase64_Encode(IN const IMS_BYTE* pSrcData, IN IMS_SIZE_T nSrcLen,
            IN_OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestLen, IN IMS_BOOL bAddCrlf = IMS_TRUE);

    /**
     * @brief This function decodes a base64 encoded data as a binary data.
     *
     * @param pszSrcData The null-terminated string containing the data to be decoded
     * @param nSrcLen The length in characters of pszSrcData
     * @param pDest The caller-allocated buffer to receive the decoded data
     * @param nDestLen The length in bytes of pDest
     * @return A length in bytes of the decoded data if decoding was successful, 0 otherwise.
     */
    GLOBAL IMS_SINT32 ImsBase64_Decode(IN const IMS_CHAR* pszSrcData, IN IMS_SIZE_T nSrcLen,
            IN_OUT IMS_BYTE* pDest, IN IMS_SIZE_T nDestLen);
}
#endif

#endif
