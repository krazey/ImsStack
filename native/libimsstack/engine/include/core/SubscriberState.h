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
#ifndef SUBSCRIBER_STATE_H_
#define SUBSCRIBER_STATE_H_

#include "SubState.h"

/**
 * @brief This class defines a state & behavior for subscriber (outgoing subscription).
 */
class SubscriberState : public SubState
{
public:
    SubscriberState();
    ~SubscriberState() override = default;

    SubscriberState(IN const SubscriberState&) = delete;
    SubscriberState& operator=(IN const SubscriberState&) = delete;

public:
    // SubState class
    IMS_BOOL UpdateState(IN const ISipMessage* piSipMsg) override;

protected:
    // SubState class
    const SipHeaderProperty* GetRestrictedHeaders(OUT IMS_UINT32& nCount) const override;

private:
    IMS_SINT32 TranslateMessage(IN const ISipMessage* piSipMsg) const;
    IMS_BOOL UpdateOnNotifyRequest(IN const ISipMessage* piSipMsg);
    void UpdateOnNotifyResponse(IN const ISipMessage* piSipMsg);
    IMS_BOOL UpdateOnSubscribeRequest(IN const ISipMessage* piSipMsg);
    void UpdateOnSubscribeResponse(IN const ISipMessage* piSipMsg);

    static void InitializeStateTable();

private:
    /// Trigger events for subscription state transition
    enum
    {
        MESSAGE_INVALID = (-1),

        /// SUBSCRIBE
        MESSAGE_SUBSCRIBE = 0,
        MESSAGE_SUBSCRIBE_1XX,
        MESSAGE_SUBSCRIBE_200,
        MESSAGE_SUBSCRIBE_202,
        MESSAGE_SUBSCRIBE_481,
        /// Except for 481
        MESSAGE_SUBSCRIBE_NON2XX,

        /// re-SUBSCRIBE
        MESSAGE_RESUBSCRIBE,
        MESSAGE_RESUBSCRIBE_1XX,
        MESSAGE_RESUBSCRIBE_200,
        MESSAGE_RESUBSCRIBE_202,
        MESSAGE_RESUBSCRIBE_481,
        /// Except for 481
        MESSAGE_RESUBSCRIBE_NON2XX,

        /// NOTIFY
        MESSAGE_NOTIFY_ACTIVE,
        MESSAGE_NOTIFY_PENDING,
        MESSAGE_NOTIFY_TERMINATED,
        MESSAGE_NOTIFY_1XX,
        MESSAGE_NOTIFY_2XX,
        MESSAGE_NOTIFY_XXX_TERMINATED,
        MESSAGE_NOTIFY_NON2XX,

        MESSAGE_MAX
    };

    // ONLY OUTGOING SUBSCRIPTION WILL BE CONCERNED ...
    static IMS_SINT32 s_nState[STATE_MAX][MESSAGE_MAX];

    static const SipHeaderProperty RESTRICTED_HEADER_PROPERTIES[];
};

#endif
