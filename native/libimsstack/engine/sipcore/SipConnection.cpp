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
#include "ServiceMemory.h"

#include "IOnSipErrorListener.h"
#include "SipConnection.h"
#include "SipFactoryProxy.h"
#include "SipIpSecState.h"
#include "SipMessageTracker.h"
#include "SipPrivate.h"
#include "SipStack.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SipConnection::SipConnection() :
        Connection(),
        m_pMessage(IMS_NULL),
        m_pDialog(IMS_NULL),
        m_pTimerValues(IMS_NULL),
        m_piErrorListener(IMS_NULL)
{
}

PUBLIC VIRTUAL SipConnection::~SipConnection()
{
    if (m_pDialog != IMS_NULL)
    {
        delete m_pDialog;
    }

    if (m_pMessage != IMS_NULL)
    {
        delete m_pMessage;
    }

    if (m_pTimerValues != IMS_NULL)
    {
        delete m_pTimerValues;
    }
}

PUBLIC VIRTUAL void SipConnection::Close()
{
    Connection::Close();
}

PUBLIC VIRTUAL IMS_RESULT SipConnection::AddHeader(
        IN const AString& strName, IN const AString& strValue)
{
    AString strHValue;

    if (strName.GetLength() == 0)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS_SINT32 nHType = SipStack::GetHeaderTypeFromName(strName);

    if (SipConnection::IsReadOnlyHeader(nHType, strName) ||
            SipConnection::IsInaccessibleHeader(nHType, strName))
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    // Check dialog or subscription specific read only headers : From / To / Route

    strHValue = strValue.Trim();

    if (strHValue.IsNULL() || strHValue.IsEmpty())
    {
        if (m_pMessage->PrependHeader(nHType, AString::ConstEmpty(), strName) != IMS_SUCCESS)
        {
            return IMS_FAILURE;
        }
    }
    else
    {
        // TODO:: IMS_SINT32 nHCount = pMessage->GetHeaderCount(nHType, strHNameFF);
        /*
         *    " abc , cde, fgh " --> abc/cde/fgh
         */
        AStringArray objTokens;

        if (SipConnection::IsCommaSeparatedListHeader(nHType, strName))
        {
            objTokens = strHValue.Split(TextParser::CHAR_COMMA);
        }
        else
        {
            objTokens.AddElement(strHValue);
        }

        for (IMS_SINT32 i = objTokens.GetCount() - 1; i >= 0; --i)
        {
            m_pMessage->PrependHeader(nHType, objTokens.GetElementAt(i), strName);

            // TODO:: If failed, remove the appended headers from the message (using nHCount)
        }
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL AString SipConnection::GetHeader(
        IN const AString& strName, IN IMS_SINT32 nIndex /*= 0*/)
{
    if (strName.GetLength() == 0)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return AString::ConstNull();
    }

    IMS_SINT32 nHType = SipStack::GetHeaderTypeFromName(strName);

    if (SipConnection::IsInaccessibleHeader(nHType, strName))
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return AString::ConstNull();
    }

    return m_pMessage->GetHeader(nHType, nIndex, strName);
}

PUBLIC VIRTUAL ImsList<AString> SipConnection::GetHeaders(IN const AString& strName)
{
    if (strName.GetLength() == 0)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return ImsList<AString>();
    }

    IMS_SINT32 nHType = SipStack::GetHeaderTypeFromName(strName);

    if (SipConnection::IsInaccessibleHeader(nHType, strName))
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return ImsList<AString>();
    }

    return m_pMessage->GetHeaders(nHType, strName);
}

PUBLIC VIRTUAL IMS_RESULT SipConnection::RemoveHeader(IN const AString& strName)
{
    if (strName.GetLength() == 0)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS_SINT32 nHType = SipStack::GetHeaderTypeFromName(strName);

    if (SipConnection::IsReadOnlyHeader(nHType, strName) ||
            SipConnection::IsInaccessibleHeader(nHType, strName))
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    m_pMessage->RemoveHeader(nHType, strName);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT SipConnection::SetHeader(
        IN const AString& strName, IN const AString& strValue)
{
    AString strHValue;

    if (strName.GetLength() == 0)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS_SINT32 nHType = SipStack::GetHeaderTypeFromName(strName);

    if (SipConnection::IsReadOnlyHeader(nHType, strName) ||
            SipConnection::IsInaccessibleHeader(nHType, strName))
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    // Check dialog or subscription specific read only headers : From / To / Route

    strHValue = strValue.Trim();

    if (strHValue.IsNULL() || strHValue.IsEmpty())
    {
        if (m_pMessage->SetHeader(nHType, AString::ConstEmpty(), strName) != IMS_SUCCESS)
            return IMS_FAILURE;
    }
    else
    {
        // TODO:: IMS_SINT32 nHCount = pMessage->GetHeaderCount(nHType, strHNameFF);
        /*
         *    " abc , cde, fgh " --> abc/cde/fgh
         */
        AStringArray objTokens;

        if (SipConnection::IsCommaSeparatedListHeader(nHType, strName))
        {
            objTokens = strHValue.Split(TextParser::CHAR_COMMA);
        }
        else
        {
            objTokens.AddElement(strHValue);
        }

        m_pMessage->RemoveHeader(nHType, strName);

        // Overwritable header
        if (m_pMessage->PrependHeader(nHType, objTokens.GetElementAt(objTokens.GetCount() - 1),
                    strName) != IMS_SUCCESS)
        {
            return IMS_FAILURE;
        }

        for (IMS_SINT32 i = objTokens.GetCount() - 2; i >= 0; --i)
        {
            m_pMessage->PrependHeader(nHType, objTokens.GetElementAt(i), strName);
            // TODO:: If failed, remove the appended headers from the message (using nHCount)
        }
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL const ByteArray& SipConnection::GetContent() const
{
    SipMessageBodyPart* pBodyPart = m_pMessage->GetBodyPart();

    if (pBodyPart == IMS_NULL)
    {
        return ByteArray::ConstNull();
    }

    return pBodyPart->GetContent();
}

PUBLIC VIRTUAL IMS_RESULT SipConnection::SetContent(IN const ByteArray& objContent)
{
    SipMessageBodyPart* pBodyPart = m_pMessage->GetBodyPart();

    // If the message body already exists, then throw exception ???

    if (pBodyPart == IMS_NULL)
    {
        pBodyPart = DYNAMIC_CAST(SipMessageBodyPart*, m_pMessage->CreateBodyPart());

        if (pBodyPart == IMS_NULL)
        {
            SipPrivate::SetLastError(SipError::NO_MEMORY);
            return IMS_FAILURE;
        }
    }

    pBodyPart->SetContent(objContent);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_SINT32 SipConnection::GetHeaderCount(IN const AString& strName) const
{
    if (strName.GetLength() == 0)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return 0;
    }

    IMS_SINT32 nHType = SipStack::GetHeaderTypeFromName(strName);

    if (SipConnection::IsInaccessibleHeader(nHType, strName))
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return 0;
    }

    return m_pMessage->GetHeaderCount(nHType, strName);
}

PUBLIC
void SipConnection::SetTransactionTimerValues(IN const SipTimerValues& objTimerValues)
{
    if (m_pTimerValues != IMS_NULL)
    {
        delete m_pTimerValues;
    }

    m_pTimerValues = new SipTimerValues(objTimerValues);

    if (m_pTimerValues == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating SIP timer values failed", 0, 0, 0);
    }
}

PROTECTED VIRTUAL void SipConnection::TransactionState_TimerExpired()
{
    SipFactoryProxy* pFactoryProxy = SipFactoryProxy::GetInstance();

    if (pFactoryProxy->IsMessageTrackerEnabled(GetSlotId()))
    {
        SipMessageTracker* pMessageTracker = pFactoryProxy->GetMessageTracker(GetSlotId());
        const SipMethod& objMethod = SipConnection::GetMethod();

        if (m_pDialog != IMS_NULL)
        {
            pMessageTracker->NotifyMessageSent(objMethod, GetStatusCode(), m_pDialog->GetCallId(),
                    SipError::TRANSACTION_TIMER_EXPIRED);
        }
        else
        {
            ISipMessage* piMessage = GetMessage();

            pMessageTracker->NotifyMessageSent(objMethod, GetStatusCode(),
                    (piMessage != IMS_NULL) ? piMessage->GetHeader(ISipHeader::CALL_ID)
                                            : AString::ConstNull(),
                    SipError::TRANSACTION_TIMER_EXPIRED);
        }
    }

    if (pFactoryProxy->IsIpSecStateEnabled(GetSlotId()) && (m_pMessage != IMS_NULL))
    {
        SipIpSecState* pIpSecState = pFactoryProxy->GetIpSecState(GetSlotId());
        pIpSecState->NotifyMessageSentFailed(m_pMessage->GetMessage());
    }

    NotifyError(SipError::TRANSACTION_TIMER_EXPIRED, AString("Transaction Timer Expired"));
}

PROTECTED VIRTUAL void SipConnection::Transport_NotifyPendingMessageSent()
{
    IMS_TRACE_D("SC: pending message is completely sent.", 0, 0, 0);
}

PROTECTED VIRTUAL void SipConnection::Transport_NotifyError(
        IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    IMS_TRACE_I("SC: SIP transport error - [%d, %s]", nCode, strMessage.GetStr(), 0);

    // SIP_TRANSPORT_ERROR_REPORT_ON_TXN
    if (!IsTransportErrorReportRequired(nCode, strMessage))
    {
        // Ignore the transport error...
        IMS_TRACE_D("SC: transport error report is off.", 0, 0, 0);
        return;
    }

    SipFactoryProxy* pFactoryProxy = SipFactoryProxy::GetInstance();

    if (pFactoryProxy->IsMessageTrackerEnabled(GetSlotId()))
    {
        SipMessageTracker* pMessageTracker = pFactoryProxy->GetMessageTracker(GetSlotId());

        if (m_pDialog != IMS_NULL)
        {
            pMessageTracker->NotifyMessageSent(
                    SipConnection::GetMethod(), GetStatusCode(), m_pDialog->GetCallId(), nCode);
        }
        else
        {
            ISipMessage* piMessage = GetMessage();

            pMessageTracker->NotifyMessageSent(SipConnection::GetMethod(), GetStatusCode(),
                    (piMessage != IMS_NULL) ? piMessage->GetHeader(ISipHeader::CALL_ID)
                                            : AString::ConstNull(),
                    nCode);
        }
    }

    if (pFactoryProxy->IsIpSecStateEnabled(GetSlotId()) && (m_pMessage != IMS_NULL))
    {
        SipIpSecState* pIpSecState = pFactoryProxy->GetIpSecState(GetSlotId());
        pIpSecState->NotifyMessageSentFailed(m_pMessage->GetMessage());
    }

    if (m_piErrorListener != IMS_NULL)
    {
        m_piErrorListener->OnError_NotifyError(this, nCode, strMessage);
    }
}

PROTECTED VIRTUAL IMS_BOOL SipConnection::IsTransportErrorReportRequired(
        IN IMS_SINT32 nCode, IN const AString& strMessage) const
{
    if (nCode == SipError::TRANSPORT_ERROR)
    {
        AString strTransportErrorCode;

        strTransportErrorCode.Sprintf("%d", SipError::TRANSPORT_E_CODE_104);

        // SipError::TRANSPORT_E_CODE_104
        if (strMessage.StartsWith(strTransportErrorCode))
        {
            // Ignore "Socket is closed by peer"(104) event notification
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PROTECTED
void SipConnection::InitMessage(IN sipcore::SipMessage* pMessage /*= IMS_NULL*/,
        IN IMS_SINT32 nType /*= sipcore::SipMessage::TYPE_REQUEST*/)
{
    if (m_pMessage != IMS_NULL)
    {
        delete m_pMessage;
        m_pMessage = IMS_NULL;
    }

    if (pMessage != IMS_NULL)
    {
        m_pMessage = pMessage;
    }
    else
    {
        m_pMessage = new sipcore::SipMessage(nType);
    }
}

PROTECTED
void SipConnection::NotifyError(IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    if (m_piErrorListener == IMS_NULL)
    {
        return;
    }

    m_piErrorListener->OnError_NotifyError(this, nCode, strMessage);
}

PRIVATE
IMS_BOOL SipConnection::IsCommaSeparatedListHeader(IN IMS_SINT32 nHType, IN const AString& strHName)
{
    (void)strHName;

    switch (nHType)
    {
        case ISipHeader::AUTHENTICATION_INFO:  // FALL-THROUGH
        case ISipHeader::AUTHORIZATION:        // FALL-THROUGH
        case ISipHeader::PROXY_AUTHORIZATION:  // FALL-THROUGH
        case ISipHeader::WWW_AUTHENTICATE:     // FALL-THROUGH
        case ISipHeader::PROXY_AUTHENTICATE:   // FALL-THROUGH
        case ISipHeader::DATE:
            return IMS_FALSE;
        default:
            return IMS_TRUE;
    }
}

PRIVATE
IMS_BOOL SipConnection::IsInaccessibleHeader(IN IMS_SINT32 nHType, IN const AString& /*strHName*/)
{
    switch (nHType)
    {
        case ISipHeader::AUTHENTICATION_INFO:  // FALL-THROUGH
        case ISipHeader::AUTHORIZATION:        // FALL-THROUGH
        case ISipHeader::CALL_ID:              // FALL-THROUGH
        case ISipHeader::CSEQ:                 // FALL-THROUGH
        case ISipHeader::MIN_EXPIRES:          // FALL-THROUGH
        case ISipHeader::MAX_FORWARDS:         // FALL-THROUGH
        case ISipHeader::SERVICE_ROUTE:        // FALL-THROUGH
        case ISipHeader::PROXY_AUTHENTICATE:   // FALL-THROUGH
        case ISipHeader::PROXY_AUTHORIZATION:  // FALL-THROUGH
        case ISipHeader::RECORD_ROUTE:         // FALL-THROUGH
        case ISipHeader::SECURITY_SERVER:      // FALL-THROUGH
        case ISipHeader::SECURITY_VERIFY:      // FALL-THROUGH
        case ISipHeader::VIA:                  // FALL-THROUGH
        case ISipHeader::WWW_AUTHENTICATE:
            return IMS_TRUE;
        default:
            return IMS_FALSE;
    }
}

PRIVATE
IMS_BOOL SipConnection::IsReadOnlyHeader(IN IMS_SINT32 nHType, IN const AString& /*strHName*/)
{
    switch (nHType)
    {
        case ISipHeader::AUTHORIZATION:        // FALL-THROUGH
        case ISipHeader::MIN_EXPIRES:          // FALL-THROUGH
        case ISipHeader::MAX_FORWARDS:         // FALL-THROUGH
        case ISipHeader::SERVICE_ROUTE:        // FALL-THROUGH
        case ISipHeader::PROXY_AUTHORIZATION:  // FALL-THROUGH
        case ISipHeader::RECORD_ROUTE:         // FALL-THROUGH
        case ISipHeader::SECURITY_SERVER:      // FALL-THROUGH
        case ISipHeader::SECURITY_VERIFY:      // FALL-THROUGH
        case ISipHeader::VIA:
            return IMS_TRUE;

        default:
            return IMS_FALSE;
    }
}
