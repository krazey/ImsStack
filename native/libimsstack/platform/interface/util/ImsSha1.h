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
#ifndef IMS_SHA1_H_
#define IMS_SHA1_H_

#include "ImsTypeDef.h"

/**
 * @brief This file implements the secure hash algorithm.
 */

#define IMS_SHA1_INITIAL_HEX_VALUE_SIZE 5
#define IMS_SHA1_MESSAGE_BLOCK_SIZE     64
#define IMS_SHA1_HASH_SIZE              20

// Structure for SHA1 context
struct ImsSha1Context
{
    // Length of the original message (64bits)
    IMS_UINT32 nLengthLow;
    IMS_UINT32 nLengthHigh;
    // Initial hex values (H)
    IMS_UINT32 anH[IMS_SHA1_INITIAL_HEX_VALUE_SIZE];
    // 512 bit message block
    IMS_UCHAR aucMessageBlock[IMS_SHA1_MESSAGE_BLOCK_SIZE];
};

/**
 * @brief This function initializes SHA1 context structure.
 *        It begins an SHA1 operation, writing a new context.
 *
 * @param pContext The SHA1 context to be initialized
 */
GLOBAL void ImsSha1_Initialize(OUT ImsSha1Context* pContext);

/**
 * @brief This function updates an SHA1 block. It continues an SHA1 message-digest operation,
 *        processing another message block, and updating the context.
 *
 * @param pucSrc The source data to be updated
 * @param nSrcLen The length of the source data (pucSrc)
 * @param pContext The SHA1 context to be updated
 */
GLOBAL void ImsSha1_Update(
        IN const IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen, IN_OUT ImsSha1Context* pContext);

/**
 * @brief This function finalizes an SHA1 operation. It ends an SHA1 message-digest operation,
 *        writing the message digest and zeroing the context.
 *
 * @param pContext The SHA1 context to be finalized
 * @param aucHash The array of the SHA1 result
 */
GLOBAL void ImsSha1_Finalize(
        IN_OUT ImsSha1Context* pContext, OUT IMS_UCHAR aucHash[IMS_SHA1_HASH_SIZE]);

#endif
