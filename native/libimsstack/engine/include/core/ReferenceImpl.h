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
#ifndef REFERENCE_IMPL_H_
#define REFERENCE_IMPL_H_

#include "IOnNotificationListener.h"
#include "IOnReferenceListener.h"
#include "IReference.h"
#include "Reference.h"

class ReferenceImpl : public IReference, public IOnReferenceListener, public IOnNotificationListener
{
public:
    explicit ReferenceImpl(IN Reference* pReference);
    ~ReferenceImpl() override;

    ReferenceImpl(IN const ReferenceImpl&) = delete;
    ReferenceImpl& operator=(IN const ReferenceImpl&) = delete;

private:
    // IMethod interface
    void Destroy() override;
    inline void SetMessageMediator(IN IMessageMediator* piMediator) override
    {
        m_pReference->SetMessageMediator(piMediator);
    }

    // IServiceMethod interface
    inline IMessage* GetNextRequest() override { return m_pReference->GetNextRequest(); }
    inline IMessage* GetNextResponse() override { return m_pReference->GetNextResponse(); }
    inline IMessage* GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const override
    {
        return m_pReference->GetPreviousRequest(nServiceMethod);
    }
    inline IMessage* GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const override
    {
        return m_pReference->GetPreviousResponse(nServiceMethod);
    }
    ImsList<IMessage*> GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const override;
    inline ImsList<AString> GetRemoteUserId() const override
    {
        return m_pReference->GetRemoteUserId();
    }

    // IReference interface
    inline IMS_RESULT Accept() override { return m_pReference->Accept(); }
    IMS_RESULT ConnectReferMethod(IN IServiceMethod* piServiceMethod) override;
    inline const AString& GetReferMethod() const override
    {
        return m_pReference->GetReferMethod().ToString();
    }
    inline const AString& GetReferToUserId() const override
    {
        return m_pReference->GetReferToUserId();
    }
    inline const AString& GetReplaces() const override { return m_pReference->GetReplaces(); }
    inline IMS_SINT32 GetState() const override { return m_pReference->GetState(); }
    inline IMS_RESULT Refer(IN IMS_BOOL bImplicitSubscription) override
    {
        return m_pReference->Refer(bImplicitSubscription);
    }
    inline IMS_RESULT Reject() override { return m_pReference->Reject(); }
    inline void SetListener(IN IReferenceListener* piListener) override
    {
        m_piListener = piListener;
    }
    inline IMS_RESULT SetReplaces(IN const AString& strSessionId) override
    {
        return m_pReference->SetReplaces(strSessionId);
    }
    inline IMS_RESULT AcceptEx(IN IMS_SINT32 nStatusCode = SipStatusCode::SC_202,
            IN IMS_BOOL b100Trying = IMS_TRUE) override
    {
        return m_pReference->AcceptEx(nStatusCode, b100Trying);
    }
    inline IMS_RESULT ReferEx(IN IMS_BOOL bImplicitSubscription,
            IN const AString& strHeadersForReferTo = AString::ConstNull()) override
    {
        return m_pReference->ReferEx(bImplicitSubscription, strHeadersForReferTo);
    }
    inline IMS_RESULT RejectEx(IN IMS_SINT32 nStatusCode) override
    {
        return m_pReference->RejectEx(nStatusCode);
    }
    inline IMS_RESULT SendNotification(IN IMS_SINT32 nSubState, IN const ByteArray& objContent,
            IN IMS_SINT32 nReason = ISubscriptionState::REASON_NONE,
            IN IMS_SINT32 nExpires = (-1)) override
    {
        return m_pReference->SendNotification(nSubState, objContent, nReason, nExpires);
    }
    void SetNotificationListener(IN INotificationListener* piListener) override;
    inline void SetImplicitRoutingRequired(IN IMS_BOOL bFlag) override
    {
        return m_pReference->SetImplicitRoutingRequired(bFlag);
    }

    // IOnReferenceListener interface
    void OnReference_Delivered(IN Reference* pReference) override;
    void OnReference_DeliveryFailed(IN Reference* pReference) override;
    void OnReference_NotifyReceived(IN Reference* pReference, IN Message* pNotify) override;
    void OnReference_Terminated(IN Reference* pReference) override;

    // IOnNotificationListener interface
    void OnNotification_Delivered(IN ServiceMethod* pMethod) override;
    void OnNotification_DeliveryFailed(
            IN ServiceMethod* pMethod, IN IMS_SINT32 nStatusCode) override;

private:
    Reference* m_pReference;
    IReferenceListener* m_piListener;
    INotificationListener* m_piNotificationListener;
};

#endif
