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
#ifndef SIP_SERVER_CONNECTION_IMPL_H_
#define SIP_SERVER_CONNECTION_IMPL_H_

#include "IOnSipErrorListener.h"
#include "ISipServerConnection.h"

class SipDialogImpl;
class SipServerConnection;

class SipServerConnectionImpl : public ISipServerConnection, public IOnSipErrorListener
{
public:
    explicit SipServerConnectionImpl(IN SipServerConnection* pSsc);
    virtual ~SipServerConnectionImpl();

    SipServerConnectionImpl() = delete;
    SipServerConnectionImpl(IN const SipServerConnectionImpl&) = delete;
    SipServerConnectionImpl& operator=(IN const SipServerConnectionImpl&) = delete;

private:
    // IConnection interface implementation
    void Close() override;

    // ISipConnection interface implementation
    IMS_RESULT AddHeader(IN const AString& strName, IN const AString& strValue) override;
    ISipDialog* GetDialog() const override;
    AString GetHeader(IN const AString& strName, IN IMS_SINT32 nIndex = 0) override;
    IMSList<AString> GetHeaders(IN const AString& strName) override;
    const SipMethod& GetMethod() const override;
    const AString& GetReasonPhrase() const override;
    const AString& GetRequestUri() const override;
    IMS_SINT32 GetStatusCode() const override;
    IMS_RESULT RemoveHeader(IN const AString& strName) override;
    IMS_RESULT Send() override;
    inline void SetErrorListener(IN ISipErrorListener* piListener) override
    {
        m_piErrorListener = piListener;
    }
    IMS_RESULT SetHeader(IN const AString& strName, IN const AString& strValue) override;
    const ByteArray& GetContent() const override;
    IMS_RESULT SetContent(IN const ByteArray& objContent) override;
    IMS_SINT32 GetHeaderCount(IN const AString& strName) const override;
    ISipMessage* GetMessage() const override;
    IMS_SINT32 GetSlotId() const override;
    void SetSipProfile(IN SipProfile* pProfile) override;
    void SetTransactionTimerValues(IN const SipTimerValues& objTimerValues) override;

    // ISIPServerTransaction interface
    IMS_RESULT InitResponse(IN IMS_SINT32 nStatusCode) override;
    IMS_RESULT SetReasonPhrase(IN const AString& strReasonPhrase) override;
    IMS_BOOL IsSameTransaction(IN const ISipServerConnection* piOngoingSsc) const override;

    // IOnSipErrorListener interface
    void OnError_NotifyError(
            IN SipConnection* pSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

private:
    SipServerConnection* m_pSsc;
    SipDialogImpl* m_pDialogImpl;
    ISipErrorListener* m_piErrorListener;
};

#endif
