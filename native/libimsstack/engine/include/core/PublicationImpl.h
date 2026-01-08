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
#ifndef PUBLICATION_IMPL_H_
#define PUBLICATION_IMPL_H_

#include "IOnPublicationListener.h"
#include "IPublication.h"
#include "Publication.h"

class PublicationImpl : public IPublication, public IOnPublicationListener
{
public:
    explicit PublicationImpl(IN Publication* pPublication);
    ~PublicationImpl() override;

    PublicationImpl(IN const PublicationImpl&) = delete;
    PublicationImpl& operator=(IN const PublicationImpl&) = delete;

private:
    // IMethod interface
    void Destroy() override;
    inline void SetMessageMediator(IN IMessageMediator* piMediator) override
    {
        m_pPublication->SetMessageMediator(piMediator);
    }

    // IServiceMethod interface
    inline IMessage* GetNextRequest() override { return m_pPublication->GetNextRequest(); }
    inline IMessage* GetNextResponse() override { return m_pPublication->GetNextResponse(); }
    inline IMessage* GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const override
    {
        return m_pPublication->GetPreviousRequest(nServiceMethod);
    }
    inline IMessage* GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const override
    {
        return m_pPublication->GetPreviousResponse(nServiceMethod);
    }
    ImsList<IMessage*> GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const override;
    inline ImsList<AString> GetRemoteUserId() const override
    {
        return m_pPublication->GetRemoteUserId();
    }

    // IPublication interface
    inline const AString& GetEvent() const override { return m_pPublication->GetEvent(); }
    inline IMS_SINT32 GetState() const override { return m_pPublication->GetState(); }
    inline IMS_RESULT Publish(
            IN const ByteArray& objState, IN const AString& strContentType) override
    {
        return m_pPublication->Publish(objState, strContentType);
    }
    inline void SetListener(IN IPublicationListener* piListener) override
    {
        m_piListener = piListener;
    }
    inline IMS_RESULT Unpublish() override { return m_pPublication->Unpublish(); }
    inline void SetRefreshListener(IN IRefreshListener* piListener) override
    {
        m_pPublication->SetRefreshListener(piListener);
    }
    inline void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt) override
    {
        m_pPublication->SetRefreshPolicy(nPolicy, nCriteriaInterval, nValueEorLt, nValueGt);
    }

    // IOnPublicationListener interface
    void OnPublication_Delivered(IN Publication* pPublication) override;
    void OnPublication_DeliveryFailed(IN Publication* pPublication) override;
    void OnPublication_Terminated(IN Publication* pPublication) override;
    void OnPublication_RefreshStarted(IN Publication* pPublication) override;
    void OnPublication_RefreshCompleted(IN Publication* pPublication) override;

private:
    Publication* m_pPublication;
    IPublicationListener* m_piListener;
};

#endif
