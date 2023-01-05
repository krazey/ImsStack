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
#ifndef INTERFACE_DIGEST_AKA_LISTENER_H_
#define INTERFACE_DIGEST_AKA_LISTENER_H_

#include "ByteArray.h"

class IDigestAkaListener
{
protected:
    virtual ~IDigestAkaListener() = default;

public:
    /**
     * @brief Notifies the application that the authentication is successfully done with the RES,
     *        IK and CK.
     *
     * AKA RES parameter will be used as "password" when calculating the response.
     *
     * @param objRes The response to authentication
     * @param objIk IK (Integrity Key) value
     * @param objCk CK (Ciphering Key) value
     */
    virtual void DigestAka_OnResponse(IN const ByteArray& objRes,
            IN const ByteArray& objIk = ByteArray::ConstNull(),
            IN const ByteArray& objCk = ByteArray::ConstNull()) = 0;

    /**
     * @brief Notifies the application that the AKA sequence number synchronization is failed
     *        and re-synchronize the SQN in the AuC using the AUTS token.
     *
     * @param objAuts The AUTS token
     */
    virtual void DigestAka_OnAutsFailed(IN const ByteArray& objAuts) = 0;

    /**
     * @brief Notifies the application that the MAC (Message Authentication Code) is failed.
     */
    virtual void DigestAka_OnMacFailed() = 0;
};

#endif
