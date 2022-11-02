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

#ifndef INTERFACE_CONFERENCE_REFERENCE_H_
#define INTERFACE_CONFERENCE_REFERENCE_H_

#include "ImsTypeDef.h"

class CallConnectionIdManager;

// Subscription-State value of R-NOTIFY
enum class ReferSubscriptionState
{
    INVALID,
    ACTIVE,
    TERMINATED,
};

class IConferenceReference
{
public:
    virtual ~IConferenceReference() {}

    /**
     * @brief Sends
     *
     * @param strReferToUri
     * @param objConnectionIdManager
     * @return
     */
    virtual IMS_RESULT SendInvite(
            OUT AString& strReferToUri, IN CallConnectionIdManager& objConnectionIdManager) = 0;

    /**
     * @brief Sends
     *
     * @param strInvitedUri
     * @return
     */
    virtual IMS_RESULT SendBye(IN AString strInvitedUri = AString::ConstEmpty()) = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_UINT32 GetType() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_UINT32 GetResponseCode() const = 0;

    /**
     * @brief Sets
     *
     * @param bTerminate
     */
    virtual void SetForceToTerminateInterface(IN IMS_BOOL bTerminate) = 0;
};

#endif
