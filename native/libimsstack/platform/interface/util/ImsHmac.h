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
#ifndef IMS_HMAC_H_
#define IMS_HMAC_H_

#include "ImsTypeDef.h"

#define IMS_HMAC_MD5_SIZE  16
#define IMS_HMAC_SHA1_SIZE 20

/**
 * @brief This file implements HMAC, a mechanism for message authentication using cryptographic
 *        hash functions.
 *
 * HMAC can be used with any iterative cryptographic hash function, e.g., MD5, SHA-1,
 * in combination with a secret shared key.
 * The cryptographic strength of HMAC depends on the properties of the underlying hash function.
 * HMAC can be used in combination with any iterated cryptographic hash function.
 * MD5 and SHA-1 are examples of such hash functions. HMAC also uses a secret key for calculation
 * and verification of the message authentication values.
 */

/**
 * @brief This function generates HMAC-MD5.
 *
 * @param pucText The input data stream
 * @param nTextLen The length of the input data stream
 * @param pucKey The authentication key
 * @param nKeyLen The length of the authentication key
 * @param aucDigest The digest value to be filled in
 */
GLOBAL void ImsHmac_Md5(IN const IMS_UCHAR* pucText, IN IMS_SINT32 nTextLen,
        IN const IMS_UCHAR* pucKey, IN IMS_SINT32 nKeyLen,
        OUT IMS_UCHAR aucDigest[IMS_HMAC_MD5_SIZE]);

/**
 * @brief This function generates HMAC-SHA-1.
 *
 * @param pucText The input data stream
 * @param nTextLen The length of the input data stream
 * @param pucKey The authentication key
 * @param nKeyLen The length of the authentication key
 * @param aucHash The hash value to be filled in
 */
GLOBAL void ImsHmac_Sha1(IN const IMS_UCHAR* pucText, IN IMS_SINT32 nTextLen,
        IN const IMS_UCHAR* pucKey, IN IMS_SINT32 nKeyLen,
        OUT IMS_UCHAR aucHash[IMS_HMAC_SHA1_SIZE]);

#endif
