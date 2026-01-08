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
#ifndef IMPLICIT_NOTIFIER_STATE_H_
#define IMPLICIT_NOTIFIER_STATE_H_

#include "SubState.h"

/**
 * @brief This class defines a state & behavior for notifier (implicit subscription).
 */
class ImplicitNotifierState : public SubState
{
public:
    ImplicitNotifierState();
    ~ImplicitNotifierState() override = default;

    ImplicitNotifierState(IN const ImplicitNotifierState&) = delete;
    ImplicitNotifierState& operator=(IN const ImplicitNotifierState&) = delete;

public:
    // SubState class
    IMS_BOOL UpdateState(IN const ISipMessage* piSipMsg) override;

private:
    IMS_SINT32 TranslateMessage(IN const ISipMessage* piSipMsg) const;
    IMS_BOOL UpdateOnNotifyRequest(IN const ISipMessage* piSipMsg);
    void UpdateOnNotifyResponse(IN const ISipMessage* piSipMsg);
    void UpdateOnReferRequest(IN const ISipMessage* piSipMsg);
    void UpdateOnReferResponse(IN const ISipMessage* piSipMsg);

    static void InitializeStateTable();

private:
    /// Trigger events for subscription state transition
    enum
    {
        MESSAGE_INVALID = (-1),

        /// REFER
        MESSAGE_REFER = 0,
        MESSAGE_REFER_1XX,
        MESSAGE_REFER_200,
        MESSAGE_REFER_202,
        MESSAGE_REFER_481,
        /// Except for 481
        MESSAGE_REFER_NON2XX,

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

    // STATE for Notifier
    static IMS_SINT32 s_nState[STATE_MAX][MESSAGE_MAX];
};

#endif
