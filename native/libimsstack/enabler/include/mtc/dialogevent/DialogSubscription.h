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

#include "ISubscriptionListener.h"
#include "ImsTypeDef.h"
#include "dialogevent/IDialogSubscription.h"

class AString;
class IMessage;
class IMtcContext;
class ISubscription;

class DialogSubscription final : public IDialogSubscription, public ISubscriptionListener
{
public:
    explicit DialogSubscription(IN IMtcContext& objContext,
            IN IDialogSubscriptionListener& objListener, IN const AString& strTargetUri);
    virtual ~DialogSubscription();
    DialogSubscription(IN const DialogSubscription&) = delete;
    DialogSubscription& operator=(IN const DialogSubscription&) = delete;

    IMS_RESULT Subscribe() override;
    void Unsubscribe() override;

    inline void SubscriptionForkedNotify(IN ISubscription*, IN ISubscription*) {}
    void SubscriptionNotify(IN ISubscription* piSubscription, IN IMessage* piNotify) override;
    void SubscriptionStarted(IN ISubscription* piSubscription) override;
    void SubscriptionStartFailed(IN ISubscription* piSubscription) override;
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
