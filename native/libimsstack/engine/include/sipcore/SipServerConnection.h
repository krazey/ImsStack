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
#ifndef SIP_SERVER_CONNECTION_H_
#define SIP_SERVER_CONNECTION_H_

#include "SipConnection.h"

class SipServerTransactionState;

class SipServerConnection : public SipConnection
{
public:
    explicit SipServerConnection(IN SipServerTransactionState* pStState);
    virtual ~SipServerConnection();

    SipServerConnection(IN const SipServerConnection&) = delete;
    SipServerConnection& operator=(IN const SipServerConnection&) = delete;

public:
    // IConnection interface
    void Close() override;

    // ISipConnection interface
    IMS_RESULT AddHeader(IN const AString& strName, IN const AString& strValue) override;
    AString GetHeader(IN const AString& strName, IN IMS_SINT32 nIndex = 0) override;
    IMSList<AString> GetHeaders(IN const AString& strName) override;
    const SipMethod& GetMethod() const override;
    const AString& GetReasonPhrase() const override;
    const AString& GetRequestUri() const override;
    IMS_SINT32 GetStatusCode() const override;
    IMS_RESULT RemoveHeader(IN const AString& strName) override;
    IMS_RESULT Send() override;
    IMS_RESULT SetHeader(IN const AString& strName, IN const AString& strValue) override;
    const ByteArray& GetContent() const override;
    IMS_RESULT SetContent(IN const ByteArray& objContent) override;
    IMS_SINT32 GetHeaderCount(IN const AString& strName) const override;
    void SetSipProfile(IN SipProfile* pProfile) override;

    // ISIPServerTransaction interface
    IMS_RESULT InitResponse(IN IMS_SINT32 nStatusCode);
    IMS_RESULT SetReasonPhrase(IN const AString& strReasonPhrase);
    IMS_BOOL IsSameTransaction(IN const SipServerConnection* pOngoingSsc) const;
    IMS_RESULT InitRequest();

private:
    void AdjustTimerHFor2xx();
    void SetState(IN IMS_SINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    enum
    {
        STATE_CREATED = 0,
        STATE_REQUEST_RECEIVED,
        STATE_PROVISIONAL_RESPONDED,
        STATE_INITIALIZED,
        STATE_COMPLETED,
        STATE_TERMINATED
    };

private:
    IMS_SINT32 m_nState;
    RcPtr<SipServerTransactionState> m_pStState;
};

#endif
