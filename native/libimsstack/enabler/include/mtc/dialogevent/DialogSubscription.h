/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef DIALOG_SUBSCRIPTION_H_
#define DIALOG_SUBSCRIPTION_H_

#include "AString.h"
#include "ISubscriptionListener.h"
#include "ImsTypeDef.h"
#include "dialogevent/IDialogSubscription.h"

class IMessage;
class IMtcContext;
class ISubscription;

/**
 * @brief Manages a SIP subscription for the 'dialog' event package.
 *
 * This class handles the lifecycle of a SIP SUBSCRIBE request for dialog events,
 * including sending the initial subscription, handling notifications, and managing
 * subscription termination. It also implements retry logic for specific failure
 * scenarios, such as when the requested subscription interval is too brief.
 */
class DialogSubscription final : public IDialogSubscription, public ISubscriptionListener
{
public:
    /**
     * @brief Constructs a new DialogSubscription object.
     *
     * @param objContext The MTC context.
     * @param objListener A listener for subscription events.
     * @param strTargetUri The target URI for the subscription.
     */
    explicit DialogSubscription(IN IMtcContext& objContext,
            IN IDialogSubscriptionListener& objListener, IN const AString& strTargetUri);
    virtual ~DialogSubscription() override;
    DialogSubscription(IN const DialogSubscription&) = delete;
    DialogSubscription& operator=(IN const DialogSubscription&) = delete;

    /** See {@link IDialogSubscription::Subscribe}. */
    IMS_RESULT Subscribe() override;

    /** See {@link IDialogSubscription::Unsubscribe}. */
    void Unsubscribe() override;

    /** This is not used. */
    inline void SubscriptionForkedNotify(IN ISubscription*, IN ISubscription*) override {}

    /** Handles incoming NOTIFY requests for the subscription. */
    void SubscriptionNotify(IN ISubscription* piSubscription, IN IMessage* piNotify) override;

    /** Called when the subscription has been successfully established. */
    void SubscriptionStarted(IN ISubscription* piSubscription) override;

    /** Called when the subscription fails to start. */
    void SubscriptionStartFailed(IN ISubscription* piSubscription) override;

    /** Called when the subscription is terminated. */
    void SubscriptionTerminated(IN ISubscription* piSubscription) override;

private:
    void SetHeaders();
    IMS_BOOL IsIntervalTooBrief(IN const IMessage* piMessage) const;
    IMS_RESULT HandleIntervalTooBrief(IN const IMessage& objMessage);

    IMtcContext& m_objContext;
    IDialogSubscriptionListener& m_objListener;
    ISubscription* m_piSubscription;
    const AString m_strTargetUri;
    IMS_UINT32 m_nExpiresInterval;
};

#endif
