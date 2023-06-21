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
#ifndef IMS_DIGEST_H_
#define IMS_DIGEST_H_

#include "AString.h"

/**
 * @brief This file implements the digest authentication scheme.
 */
#define HASHHEX_SIZE 32

typedef IMS_CHAR HASHHEX[HASHHEX_SIZE + 1];

GLOBAL void ImsDigest_CalculateA1(IN const AString& strAlgorithm, IN const AString& strUsername,
        IN const AString& strRealm, IN const AString& strPassword, IN const AString& strNonce,
        IN const AString& strCNonce, OUT HASHHEX hA1);

GLOBAL void ImsDigest_CalculateEntity(IN const AString& strEntityBody, OUT HASHHEX hEntity);

GLOBAL void ImsDigest_CalculateResponse(IN const HASHHEX hA1,  // H(A1)
        IN const AString& strNonce,                            // nonce from the server
        IN const AString& strNonceCount,                       // 8 HEX digits
        IN const AString& strCNonce,                           // Client nonce
        IN const AString& strQop,                              // qop-value : "", "auth", "auth-int"
        IN const AString& strMethod,                           // Method from the Request-Line
        IN const AString& strDigestUri,                        // Requested URI
        IN const HASHHEX hEntity,                              // H(entity-body) if qop = "auth-int"
        OUT HASHHEX hResponse);                                // Request-digest or response-digest

#endif
