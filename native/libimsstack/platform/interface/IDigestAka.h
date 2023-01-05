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
#ifndef INTERFACE_DIGEST_AKA_H_
#define INTERFACE_DIGEST_AKA_H_

#include "ByteArray.h"

class IDigestAkaListener;

class IDigestAka
{
protected:
    virtual ~IDigestAka() = default;

public:
    /**
     * @brief Destroys the Digest AKA.
     */
    virtual void Destroy() = 0;

    /**
     * @brief Authenticates the client & server from the Digest AKA authentication challenge.
     *
     * It extracts the RAND and AUTN from the "nonce" parameter, and accesses the AUTN token
     * provided by the server. If the client successfully authenticates the server with the AUTN,
     * and determines that the SQN used in generating the challenge is within expected range,
     * the AKA algorithms are run with the RAND challenge and shared secret K.
     *
     * @param objChallenge Authentication challenge
     *                     (Length of RAND (1) + RAND (16) + Length of AUTN (1) + AUTN (16))
     * @return IMS_SUCCESS if the operation is successfully done, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT GetAuthResponse(IN const ByteArray& objChallenge) = 0;

    /**
     * @brief Sets the listener to receive the result of authentication.
     *
     * @param piListener The listener to be set
     */
    virtual void SetListener(IN IDigestAkaListener* piListener) = 0;
};

#endif
