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

/**
 * @brief Represents the state of a REFER subscription as indicated in a NOTIFY message.
 */
enum class ReferSubscriptionState
{
    /** The subscription state invalid (eg. No Subscription-State header case). */
    INVALID,
    /** The subscription is active. */
    ACTIVE,
    /** The subscription has been terminated. */
    TERMINATED,
};

/**
 * @brief An interface for handling SIP REFER requests for conference call control.
 *
 * This class is responsible for sending REFER requests to invite or remove participants
 * from a conference call. It abstracts the underlying SIP reference mechanism.
 */
class IConferenceReference
{
public:
    virtual ~IConferenceReference() {}

    /**
     * @brief Sends a REFER request with a `INVITE` method in the `Refer-To` URI to invite a
     *        participant to the conference.
     *
     * The `Refer-To` URI is constructed based on the participant's information and is
     * returned via the output parameter.
     *
     * @param strReferToUri The generated `Refer-To` URI that was sent in the request.
     * @param objConnectionIdManager A reference to the {@link CallConnectionIdManager} to
     *        resolve call keys.
     * @return {@link IMS_SUCCESS} if the REFER request was sent successfully,
     *         otherwise {@link IMS_FAILURE}.
     */
    virtual IMS_RESULT SendInvite(
            OUT AString& strReferToUri, IN CallConnectionIdManager& objConnectionIdManager) = 0;

    /**
     * @brief Sends a REFER request with a `BYE` method in the `Refer-To` URI to remove a
     *        participant from the conference.
     *
     * @param strInvitedUri The URI of the participant to be removed, which was previously
     *        used to invite them. Defaults to an empty string.
     * @return {@link IMS_RESULT#IMS_SUCCESS} if the REFER request was sent successfully,
     *         otherwise {@link IMS_RESULT#IMS_FAILURE}.
     */
    virtual IMS_RESULT SendBye(IN AString strInvitedUri = AString::ConstEmpty()) = 0;

    /**
     * @brief Gets the type of the reference operation (e.g., INVITE or BYE).
     *
     * @return The type of the reference, as defined in `ConferenceDef.h` (e.g.,
     *         `REFERENCE_TYPE_INVITE`, `REFERENCE_TYPE_BYE`).
     */
    virtual IMS_UINT32 GetType() const = 0;

    /**
     * @brief Gets the SIP status code from the response to the initial REFER request.
     *
     * @return The SIP status code (e.g., 202 for Accepted).
     */
    virtual IMS_UINT32 GetResponseCode() const = 0;

    /**
     * @brief Forces the immediate termination of an existing IReference interface when sending the
     *        next REFER request.
     *
     * This is used to ensure sequential matching of REFER and NOTIFY by preventing multiple
     * REFER transactions from existing simultaneously in the SIP engine when the Event header
     * lacks an 'id' parameter.
     *
     * @param bTerminate If true, the interface will be forcefully terminated.
     */
    virtual void SetForceToTerminateInterface(IN IMS_BOOL bTerminate) = 0;
};

#endif
