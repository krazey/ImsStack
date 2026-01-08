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
#ifndef PAGE_MESSAGE_H_
#define PAGE_MESSAGE_H_

#include "ServiceMethod.h"
#include "SipStatusCode.h"

class IOnPageMessageListener;

class PageMessage : public ServiceMethod
{
public:
    explicit PageMessage(IN Service* pService);
    ~PageMessage() override = default;

    PageMessage(IN const PageMessage&) = delete;
    PageMessage& operator=(IN const PageMessage&) = delete;

public:
    // IPageMessage interface
    const ByteArray& GetContent() const;
    AString GetContentType() const;
    inline IMS_SINT32 GetState() const { return m_nState; }
    IMS_RESULT Send(IN const ByteArray& objContent, IN const AString& strContentType);
    inline void SetListener(IN IOnPageMessageListener* piListener) { m_piListener = piListener; }
    IMS_RESULT Accept(IN IMS_SINT32 nStatusCode = SipStatusCode::SC_200);
    IMS_RESULT Reject(IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter = 0);

protected:
    // Activity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    // Method class
    IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piScc) override;
    // Handlie the incoming request / outgoing response message
    IMS_BOOL NotifySipRequest(IN ISipServerConnection* piSsc) override;
    // Handle to the outgoing request / incoming response message
    void NotifySipResponse(IN ISipClientConnection* piScc) override;
    void NotifySipError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

private:
    void SetState(IN IMS_SINT32 nState);
    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    /// Refer to the state in IPageMessage
    enum
    {
        STATE_UNSENT = 1,
        STATE_SENT = 2,
        STATE_RECEIVED = 3
    };

protected:
    enum
    {
        AMSG_PAGE_MESSAGE_RECEIVED = AMSG_USER,
        AMSG_PAGE_MESSAGE_DELIVERED,
        AMSG_PAGE_MESSAGE_DELIVERY_FAILED,
        AMSG_PAGE_MESSAGE_MAX
    };

private:
    IMS_SINT32 m_nState;
    IOnPageMessageListener* m_piListener;
};

#endif
