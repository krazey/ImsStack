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
#ifndef IMS_MD5_H_
#define IMS_MD5_H_

#include "ImsTypeDef.h"

/**
 * @brief This file implements the digest authentication scheme.
 */

#define IMS_MD5_STATE_SIZE          4
#define IMS_MD5_NUMBER_OF_BITS_SIZE 2
#define IMS_MD5_BUFFER_SIZE         64
#define IMS_MD5_DIGEST_SIZE         16

// Structure for MD5 context
struct ImsMd5Context
{
    IMS_UINT32 anState[IMS_MD5_STATE_SIZE];           // State (ABCD)
    IMS_UINT32 anCount[IMS_MD5_NUMBER_OF_BITS_SIZE];  // Number of bits, modulo 2^64 (LSB first)
    IMS_UCHAR aucBuffer[IMS_MD5_BUFFER_SIZE];         // Input buffer
};

/**
 * @brief This function initializes MD5 context structure.
 *        It begins an MD5 operation, writing a new context.
 *
 * @param pContext The MD5 context to be initialized
 */
GLOBAL void ImsMd5_Initialize(OUT ImsMd5Context* pContext);

/**
 * @brief This function updates an MD5 block. It continues an MD5 message-digest operation,
 *        processing another message block, and updating the context.
 *
 * @param pucSrc The source data to be updated
 * @param nSrcLen The length of the source data(pucStr)
 * @param pContext The MD5 context to be updated
 */
GLOBAL void ImsMd5_Update(
        IN const IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen, IN_OUT ImsMd5Context* pContext);

/**
 * @brief This function finalizes an MD5 operation. It ends an MD5 message-digest operation,
 *        writing the message digest and zeroing the context.
 *
 * @param pContext The MD5 context to be finalized
 * @param aucDigest The array of the MD5 result
 */
GLOBAL void ImsMd5_Finalize(
        IN_OUT ImsMd5Context* pContext, OUT IMS_UCHAR aucDigest[IMS_MD5_DIGEST_SIZE]);

#endif
