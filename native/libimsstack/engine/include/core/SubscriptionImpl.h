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
#ifndef SUBSCRIPTION_IMPL_H_
#define SUBSCRIPTION_IMPL_H_

#include "ISubscription.h"
#include "IOnSubscriptionListener.h"
#include "Subscription.h"

class SubscriptionImpl : public ISubscription, public IOnSubscriptionListener
{
public:
    explicit SubscriptionImpl(IN Subscription* pSubscription);
    ~SubscriptionImpl() override;

    SubscriptionImpl(IN const SubscriptionImpl&) = delete;
    SubscriptionImpl& operator=(IN const SubscriptionImpl&) = delete;

private:
    // IMethod interface
    void Destroy() override;
    inline void SetMessageMediator(IN IMessageMediator* piMediator) override
    {
        m_pSubscription->SetMessageMediator(piMediator);
    }

    // IServiceMethod interface
    inline IMessage* GetNextRequest() override { return m_pSubscription->GetNextRequest(); }
    inline IMessage* GetNextResponse() override { return m_pSubscription->GetNextResponse(); }
    inline IMessage* GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const override
    {
        return m_pSubscription->GetPreviousRequest(nServiceMethod);
    }
    inline IMessage* GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const override
    {
        return m_pSubscription->GetPreviousResponse(nServiceMethod);
    }
    ImsList<IMessage*> GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const override;
    inline ImsList<AString> GetRemoteUserId() const override
    {
        return m_pSubscription->GetRemoteUserId();
    }

    // ISubscription interface
    inline const AString& GetEvent() const override { return m_pSubscription->GetEvent(); }
    inline IMS_SINT32 GetState() const override { return m_pSubscription->GetState(); }
    inline IMS_RESULT Poll() override { return m_pSubscription->Poll(); }
    inline void SetListener(IN ISubscriptionListener* piListener) override
    {
        m_piListener = piListener;
    }
    inline IMS_RESULT Subscribe() override { return m_pSubscription->Subscribe(); }
    inline IMS_RESULT Unsubscribe() override { return m_pSubscription->Unsubscribe(); }

    inline void SetImplicitRoutingRequired(IN IMS_BOOL bFlag) override
    {
        m_pSubscription->SetImplicitRoutingRequired(bFlag);
    }
    inline void SetRefreshListener(IN IRefreshListener* piListener) override
    {
        m_pSubscription->SetRefreshListener(piListener);
    }
    inline void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt) override
    {
        m_pSubscription->SetRefreshPolicy(nPolicy, nCriteriaInterval, nValueEorLt, nValueGt);
    }

    // IOnSubscriptionListener interface
    IMS_BOOL OnSubscription_ForkedNotifyReceived(
            IN Subscription* pSubscription, IN Subscription* pForkedSubscription) override;
    void OnSubscription_NotifyReceived(IN Subscription* pSubscription, IN Message* pNotify,
            OUT IMS_BOOL& bDestroyNotify) override;
    void OnSubscription_Started(IN Subscription* pSubscription) override;
    void OnSubscription_StartFailed(IN Subscription* pSubscription) override;
    void OnSubscription_Terminated(IN Subscription* pSubscription) override;

private:
    Subscription* m_pSubscription;
    ISubscriptionListener* m_piListener;
};

#endif
