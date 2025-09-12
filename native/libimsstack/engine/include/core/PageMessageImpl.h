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
#ifndef PAGE_MESSAGE_IMPL_H_
#define PAGE_MESSAGE_IMPL_H_

#include "IOnPageMessageListener.h"
#include "IPageMessage.h"
#include "PageMessage.h"

class PageMessageImpl : public IPageMessage, public IOnPageMessageListener
{
public:
    explicit PageMessageImpl(IN PageMessage* pPageMessage);
    ~PageMessageImpl() override;

    PageMessageImpl(IN const PageMessageImpl&) = delete;
    PageMessageImpl& operator=(IN const PageMessageImpl&) = delete;

private:
    // IMethod interface
    void Destroy() override;
    inline void SetMessageMediator(IN IMessageMediator* piMediator) override
    {
        m_pPageMessage->SetMessageMediator(piMediator);
    }

    // IServiceMethod interface
    inline IMessage* GetNextRequest() override { return m_pPageMessage->GetNextRequest(); }
    inline IMessage* GetNextResponse() override { return m_pPageMessage->GetNextResponse(); }
    inline IMessage* GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const override
    {
        return m_pPageMessage->GetPreviousRequest(nServiceMethod);
    }
    inline IMessage* GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const override
    {
        return m_pPageMessage->GetPreviousResponse(nServiceMethod);
    }
    ImsList<IMessage*> GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const override;
    inline ImsList<AString> GetRemoteUserId() const override
    {
        return m_pPageMessage->GetRemoteUserId();
    }

    // IPageMessage interface
    inline const ByteArray& GetContent() const override { return m_pPageMessage->GetContent(); }
    inline AString GetContentType() const override { return m_pPageMessage->GetContentType(); }
    inline IMS_SINT32 GetState() const override { return m_pPageMessage->GetState(); }
    inline IMS_RESULT Send(
            IN const ByteArray& objContent, IN const AString& strContentType) override
    {
        return m_pPageMessage->Send(objContent, strContentType);
    }
    inline void SetListener(IN IPageMessageListener* piListener) override
    {
        m_piListener = piListener;
    }
    inline IMS_RESULT Accept(IN IMS_SINT32 nStatusCode = SipStatusCode::SC_200) override
    {
        return m_pPageMessage->Accept(nStatusCode);
    }
    inline IMS_RESULT Reject(IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter = 0) override
    {
        return m_pPageMessage->Reject(nStatusCode, nRetryAfter);
    }

    // IOnPageMessageListener interface
    void OnPageMessage_Delivered(IN PageMessage* pPageMessage) override;
    void OnPageMessage_DeliveryFailed(IN PageMessage* pPageMessage) override;

private:
    PageMessage* m_pPageMessage;
    IPageMessageListener* m_piListener;
};

#endif
