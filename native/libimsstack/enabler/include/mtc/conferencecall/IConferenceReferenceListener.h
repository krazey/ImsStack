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

#ifndef INTERFACE_CONFERENCE_REFERENCE_LISTENER_H_
#define INTERFACE_CONFERENCE_REFERENCE_LISTENER_H_

#include "ImsTypeDef.h"
#include "conferencecall/IConferenceReference.h"

/**
 * @brief Listener for events from an {@link ConferenceReference}.
 *
 * This interface provides callbacks for the lifecycle of a SIP REFER transaction,
 * which is used for operations like inviting or removing participants from a conference.
 */
class IConferenceReferenceListener
{
public:
    virtual ~IConferenceReferenceListener() = default;

    /**
     * @brief Notifies that the REFER request has been successfully sent and accepted.
     *
     * This is typically triggered upon receiving a 2xx response to the REFER request.
     *
     * @param piConfRef A pointer to the {@link IConferenceReference} that was successfully started.
     */
    virtual void OnReferenceStarted(IN IConferenceReference* piConfRef) = 0;

    /**
     * @brief Notifies that the REFER request has failed to be sent.
     *
     * This can be due to network issues or other errors preventing the request from being
     * delivered.
     *
     * @param pConfRef A pointer to the {@link IConferenceReference} that failed to start.
     */
    virtual void OnReferenceStartFailed(IN IConferenceReference* pConfRef) = 0;

    /**
     * @brief Notifies that a NOTIFY message has been received for the REFER subscription.
     *
     * @param piConfRef A pointer to the corresponding {@link IConferenceReference}.
     * @param nSipFragCode The SIP status code from the sipfrag body of the NOTIFY message.
     * @param eState The state of the subscription from the 'Subscription-State' header.
     */
    virtual void OnReferenceUpdated(IN IConferenceReference* piConfRef, IN IMS_SINT32 nSipFragCode,
            IN ReferSubscriptionState eState) = 0;
};

#endif
