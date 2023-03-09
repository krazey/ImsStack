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
#ifndef SIP_CONNECTION_H_
#define SIP_CONNECTION_H_

#include "Connection.h"
#include "ISipTransactionStateListener.h"
#include "ISipTransportListener.h"
#include "SipDialog.h"
#include "SipMessage.h"
#include "SipTimerValues.h"

class IOnSipErrorListener;
class SipProfile;

class SipConnection :
        public Connection,
        public ISipTransactionStateListener,
        public ISipTransportListener
{
protected:
    SipConnection();
    virtual ~SipConnection();

    SipConnection(IN const SipConnection&) = delete;
    SipConnection& operator=(IN const SipConnection&) = delete;

public:
    // IConnection interface
    void Close() override;

    // ISipConnection interface
    virtual IMS_RESULT AddHeader(IN const AString& strName, IN const AString& strValue);
    inline virtual SipDialog* GetDialog() const { return m_pDialog; }
    virtual AString GetHeader(IN const AString& strName, IN IMS_SINT32 nIndex = 0);
    virtual ImsList<AString> GetHeaders(IN const AString& strName);
    inline virtual const SipMethod& GetMethod() const { return m_pMessage->GetMethod(); }
    inline virtual const AString& GetReasonPhrase() const { return m_pMessage->GetReasonPhrase(); }
    inline virtual const AString& GetRequestUri() const { return m_pMessage->GetRequestUri(); }
    inline virtual IMS_SINT32 GetStatusCode() const { return m_pMessage->GetStatusCode(); }
    virtual IMS_RESULT RemoveHeader(IN const AString& strName);
    inline virtual IMS_RESULT Send() { return IMS_FAILURE; }
    inline virtual void SetErrorListener(IN IOnSipErrorListener* piListener)
    {
        m_piErrorListener = piListener;
    }
    virtual IMS_RESULT SetHeader(IN const AString& strName, IN const AString& strValue);
    virtual const ByteArray& GetContent() const;
    virtual IMS_RESULT SetContent(IN const ByteArray& objContent);
    virtual IMS_SINT32 GetHeaderCount(IN const AString& strName) const;
    inline virtual ISipMessage* GetMessage() const { return m_pMessage; }
    inline virtual void SetSipProfile(IN SipProfile* /*pProfile*/) {}
    void SetTransactionTimerValues(IN const SipTimerValues& objTimerValues);

protected:
    // ISipTransactionStateListener interface
    void TransactionState_TimerExpired() override;

    // ISipTransportListener interface
    void Transport_NotifyPendingMessageSent() override;
    void Transport_NotifyError(IN IMS_SINT32 nCode, IN const AString& strMessage) override;

    // SIP_TRANSPORT_ERROR_REPORT_ON_TXN
    virtual IMS_BOOL IsTransportErrorReportRequired(
            IN IMS_SINT32 nCode, IN const AString& strMessage) const;

    inline SipTimerValues* GetTransactionTimerValues() const { return m_pTimerValues; }
    void InitMessage(IN sipcore::SipMessage* pMessage = IMS_NULL,
            IN IMS_SINT32 nType = sipcore::SipMessage::TYPE_REQUEST);
    void NotifyError(IN IMS_SINT32 nCode, IN const AString& strMessage);

private:
    static IMS_BOOL IsCommaSeparatedListHeader(IN IMS_SINT32 nHType, IN const AString& strHName);
    static IMS_BOOL IsInaccessibleHeader(IN IMS_SINT32 nHType, IN const AString& strHName);
    static IMS_BOOL IsReadOnlyHeader(IN IMS_SINT32 nHType, IN const AString& strHName);

protected:
    sipcore::SipMessage* m_pMessage;
    SipDialog* m_pDialog;
    SipTimerValues* m_pTimerValues;
    IOnSipErrorListener* m_piErrorListener;
};

#endif
