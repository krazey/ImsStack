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
#ifndef CAPABILITIES_IMPL_H_
#define CAPABILITIES_IMPL_H_

#include "Capabilities.h"
#include "ICapabilities.h"
#include "IOnCapabilitiesListener.h"

class CapabilitiesImpl : public ICapabilities, public IOnCapabilitiesListener
{
public:
    explicit CapabilitiesImpl(IN Capabilities* pCapabilities);
    ~CapabilitiesImpl() override;

    CapabilitiesImpl(IN const CapabilitiesImpl&) = delete;
    CapabilitiesImpl& operator=(IN const CapabilitiesImpl&) = delete;

private:
    // IMethod
    void Destroy() override;
    inline void SetMessageMediator(IN IMessageMediator* piMediator) override
    {
        m_pCapabilities->SetMessageMediator(piMediator);
    }

    // IServiceMethod
    inline IMessage* GetNextRequest() override { return m_pCapabilities->GetNextRequest(); }
    inline IMessage* GetNextResponse() override { return m_pCapabilities->GetNextResponse(); }
    inline IMessage* GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const override
    {
        return m_pCapabilities->GetPreviousRequest(nServiceMethod);
    }
    inline IMessage* GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const override
    {
        return m_pCapabilities->GetPreviousResponse(nServiceMethod);
    }
    ImsList<IMessage*> GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const override;
    inline ImsList<AString> GetRemoteUserId() const override
    {
        return m_pCapabilities->GetRemoteUserId();
    }

    // ICapabilities
    inline ImsList<AString> GetRemoteUserIdentities() const override
    {
        return m_pCapabilities->GetRemoteUserIdentities();
    }
    inline IMS_SINT32 GetState() const override { return m_pCapabilities->GetState(); }
    inline IMS_BOOL HasCapabilities(IN const AString& strConnection) const override
    {
        return m_pCapabilities->HasCapabilities(strConnection);
    }
    inline IMS_RESULT QueryCapabilities(IN IMS_SINT32 nFlags = FLAG_REQUEST_DEFAULT) override
    {
        return m_pCapabilities->QueryCapabilities(nFlags);
    }
    inline IMS_RESULT QueryCapabilitiesEx() override
    {
        return m_pCapabilities->QueryCapabilities(FLAG_NONE);
    }
    inline void SetListener(IN ICapabilitiesListener* piListener) override
    {
        m_piListener = piListener;
    }
    inline IMS_RESULT Accept(IN IMS_SINT32 nFlags = FLAG_RESPONSE_DEFAULT) override
    {
        return m_pCapabilities->Accept(nFlags);
    }
    inline IMS_RESULT AcceptEx() override { return m_pCapabilities->Accept(FLAG_NONE); }
    inline IMS_RESULT Reject(IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter = 0) override
    {
        return m_pCapabilities->Reject(nStatusCode, nRetryAfter);
    }

    // IOnCapabilitiesListener interface
    void OnCapabilities_QueryDelivered(IN Capabilities* pCapabilities) override;
    void OnCapabilities_QueryDeliveryFailed(IN Capabilities* pCapabilities) override;

private:
    Capabilities* m_pCapabilities;
    ICapabilitiesListener* m_piListener;
};

#endif
