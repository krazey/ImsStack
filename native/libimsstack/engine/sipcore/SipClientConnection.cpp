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
#include "ServiceTimer.h"

#include "IOnSipClientConnectionListener.h"
#include "SipAckPackage.h"
#include "SipAuHelper.h"
#include "SipClientConnection.h"
#include "SipClientTransmissionProxy.h"
#include "SipConfigProxy.h"
#include "SipConnectionNotifier.h"
#include "SipDebug.h"
#include "SipMessage.h"
#include "SipMethod.h"
#include "SipPrivate.h"
#include "SipRtConfigUtils.h"
#include "SipStack.h"
#include "SipTransport.h"
#include "SipUnknownHeaders.h"

__IMS_TRACE_TAG_SIP_CORE__;

PRIVATE GLOBAL const AString SipClientConnection::ANONYMOUS_URI(
        "\"Anonymous\" <sip:thisis@anonymous.invalid>");

PUBLIC
SipClientConnection::SipClientConnection() :
        SipConnection(),
        m_nState(STATE_CREATED),
        m_bAckSent(IMS_FALSE),
        m_bResubmissionRequestInitialized(IMS_FALSE),
        m_strTargetUri(AString::ConstNull()),
        m_pCtState(IMS_NULL),
        m_pAuHelper(IMS_NULL),
        m_piListener(IMS_NULL),
        m_pTransmissionProxy(IMS_NULL),
        m_piTcpConnectionMonitoringTimer(IMS_NULL)
{
    m_pCtState = new SipClientTransactionState(GetSlotId());

    m_pCtState->SetListener(this);
    m_pCtState->SetTransactionListener(this);
    m_pCtState->SetTransportListener(this);

    m_pTransmissionProxy = new SipClientTransmissionProxy();
    m_pTransmissionProxy->SetListener(this);
    m_pTransmissionProxy->SetTransactionState(m_pCtState.Get());
}

PUBLIC
SipClientConnection::SipClientConnection(IN const AString& strTargetUri) :
        SipConnection(),
        m_nState(STATE_CREATED),
        m_bAckSent(IMS_FALSE),
        m_bResubmissionRequestInitialized(IMS_FALSE),
        m_strTargetUri(strTargetUri),
        m_pCtState(IMS_NULL),
        m_pAuHelper(IMS_NULL),
        m_piListener(IMS_NULL),
        m_piTcpConnectionMonitoringTimer(IMS_NULL)
{
    m_pCtState = new SipClientTransactionState(GetSlotId());

    m_pCtState->SetListener(this);
    m_pCtState->SetTransactionListener(this);
    m_pCtState->SetTransportListener(this);

    m_pTransmissionProxy = new SipClientTransmissionProxy();
    m_pTransmissionProxy->SetListener(this);
    m_pTransmissionProxy->SetTransactionState(m_pCtState.Get());
}

PUBLIC
SipClientConnection::SipClientConnection(IN SipClientTransactionState* pCtState) :
        SipConnection(),
        m_nState(STATE_CREATED),
        m_bAckSent(IMS_FALSE),
        m_bResubmissionRequestInitialized(IMS_FALSE),
        m_strTargetUri(AString::ConstNull()),
        m_pCtState(pCtState),
        m_pAuHelper(IMS_NULL),
        m_piListener(IMS_NULL),
        m_piTcpConnectionMonitoringTimer(IMS_NULL)
{
    m_pCtState->SetListener(this);
    m_pCtState->SetTransactionListener(this);
    m_pCtState->SetTransportListener(this);

    m_pTransmissionProxy = new SipClientTransmissionProxy();
    m_pTransmissionProxy->SetListener(this);
    m_pTransmissionProxy->SetTransactionState(m_pCtState.Get());
}

PUBLIC VIRTUAL SipClientConnection::~SipClientConnection()
{
    if (m_nState == STATE_PROCEEDING)
    {
        m_pCtState->Abort();
    }

    // FORKED_RESPONSE
    const SipMethod& objMethod = SipConnection::GetMethod();

    if (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::ACK))
    {
        m_pCtState->RemoveForkedTransaction();
    }

    if (m_pAuHelper != IMS_NULL)
    {
        delete m_pAuHelper;
    }

    if (!m_objResponseMessages.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objResponseMessages.GetSize(); ++i)
        {
            sipcore::SipMessage* pResponseMessage = m_objResponseMessages.GetAt(i);

            if (pResponseMessage != IMS_NULL)
            {
                delete pResponseMessage;
            }
        }

        m_objResponseMessages.Clear();
    }

    m_pCtState->SetListener(IMS_NULL);
    m_pCtState->SetTransactionListener(IMS_NULL);
    m_pCtState->SetTransportListener(IMS_NULL);

    if (m_pTransmissionProxy != IMS_NULL)
    {
        delete m_pTransmissionProxy;
    }

    StopTcpConnectionMonitoringTimer();
}

PUBLIC VIRTUAL void SipClientConnection::Close()
{
    StopTcpConnectionMonitoringTimer();

    if (m_pTransmissionProxy != IMS_NULL)
    {
        m_pTransmissionProxy->Abort();
        delete m_pTransmissionProxy;
        m_pTransmissionProxy = IMS_NULL;
    }

    if (m_nState == STATE_PROCEEDING)
    {
        m_pCtState->Abort();
    }

    // FORKED_RESPONSE
    const SipMethod& objMethod = SipConnection::GetMethod();

    if (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::ACK))
    {
        m_pCtState->RemoveForkedTransaction();
    }

    SetState(STATE_TERMINATED);

    SipConnection::Close();
}

PUBLIC VIRTUAL IMS_RESULT SipClientConnection::AddHeader(
        IN const AString& strName, IN const AString& strValue)
{
    if (m_nState != STATE_INITIALIZED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    return SipConnection::AddHeader(strName, strValue);
}

PUBLIC VIRTUAL AString SipClientConnection::GetHeader(
        IN const AString& strName, IN IMS_SINT32 nIndex /* = 0 */)
{
    // Message is not initialized or the connection is closed
    if ((m_nState == STATE_CREATED) || (m_nState == STATE_TERMINATED))
    {
        SipPrivate::SetLastError(SipError::NO_ERROR);
        return AString::ConstNull();
    }

    return SipConnection::GetHeader(strName, nIndex);
}

PUBLIC VIRTUAL ImsList<AString> SipClientConnection::GetHeaders(IN const AString& strName)
{
    // Message is not initialized or the connection is closed
    if ((m_nState == STATE_CREATED) || (m_nState == STATE_TERMINATED))
    {
        SipPrivate::SetLastError(SipError::NO_ERROR);
        return ImsList<AString>();
    }

    return SipConnection::GetHeaders(strName);
}

PUBLIC VIRTUAL const SipMethod& SipClientConnection::GetMethod() const
{
    // Message is not initialized or the connection is closed
    if ((m_nState == STATE_CREATED) || (m_nState == STATE_TERMINATED))
    {
        return SipMethod::INVALID_METHOD;
    }

    return SipConnection::GetMethod();
}

PUBLIC VIRTUAL const AString& SipClientConnection::GetReasonPhrase() const
{
    // Status code is available if the state is in PROCEEDING, UNAUTHORIZED, and COMPLETED
    if ((m_nState != STATE_PROCEEDING) && (m_nState != STATE_UNAUTHORIZED) &&
            (m_nState != STATE_COMPLETED))
    {
        return AString::ConstNull();
    }

    return SipConnection::GetReasonPhrase();
}

PUBLIC VIRTUAL const AString& SipClientConnection::GetRequestUri() const
{
    // Message is not initialized or the connection is closed
    if ((m_nState == STATE_CREATED) || (m_nState == STATE_TERMINATED))
    {
        return AString::ConstNull();
    }

    return SipConnection::GetRequestUri();
}

PUBLIC VIRTUAL IMS_SINT32 SipClientConnection::GetStatusCode() const
{
    // Status code is available if the state is in PROCEEDING, UNAUTHORIZED, and COMPLETED
    if ((m_nState != STATE_PROCEEDING) && (m_nState != STATE_UNAUTHORIZED) &&
            (m_nState != STATE_COMPLETED))
    {
        return 0;
    }

    return SipConnection::GetStatusCode();
}

PUBLIC VIRTUAL IMS_RESULT SipClientConnection::RemoveHeader(IN const AString& strName)
{
    if (m_nState != STATE_INITIALIZED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    return SipConnection::RemoveHeader(strName);
}

PUBLIC VIRTUAL IMS_RESULT SipClientConnection::Send()
{
    if ((m_nState != STATE_INITIALIZED) && (m_nState != STATE_UNAUTHORIZED))
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    m_pTransmissionProxy->SetTimerValues(GetTransactionTimerValues());

    if (m_nState == STATE_UNAUTHORIZED)
    {
        if (SendWithCredentials() != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "SendWithCredentials() failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }
    else
    {
        // Throw exception - INVALID_MESSAGE if the message format was invalid
        if (!m_pMessage->FormMessage())
        {
            SipPrivate::SetLastError(SipError::INVALID_MESSAGE);
            return IMS_FAILURE;
        }

        // Update the Routing information (Determining the target destination)
        if (!m_pCtState->UpdateRouteDetails(GetMethod()))
        {
            SipPrivate::SetLastError(SipError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        // Set a Authorization / Proxy-Authorization in the initial request
        if (m_pAuHelper != IMS_NULL)
        {
            ::SipMessage* pSipMsg = m_pMessage->GetMessage();

            if (!m_pAuHelper->FormCredentials(pSipMsg))
            {
                SipPrivate::SetLastError(SipError::GENERAL_ERROR);
                return IMS_FAILURE;
            }

            // If the authentication challenge is present in the INITIALIZED,
            // then removes all the credentials after generating the response
            // to an authentication challenge.
            if (m_pAuHelper->IsChallengePresent())
            {
                IMS_TRACE_D("Authorization/Proxy-Authorization in the initial request; "
                            "The credentials will be removed...",
                        0, 0, 0);

                m_pAuHelper->RemoveAllCredentials();
            }
        }

        if (!m_pCtState->FormMessage())
        {
            IMS_TRACE_E(0, "FormMessage() failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        IMS_RESULT nResult = m_pTransmissionProxy->Send();

        if (nResult == SipClientTransmissionProxy::RESULT_NOK)
        {
            IMS_TRACE_E(0, "Send() failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        // Update the state
        if (m_pMessage->GetMethod().Equals(SipMethod::ACK))
        {
            m_bAckSent = IMS_TRUE;
        }

        if (nResult == SipClientTransmissionProxy::RESULT_OK)
        {
            // Update a dialog info...
            if (SipDialogBase::IsDialogCreatable(GetMethod()))
            {
                if (m_pDialog != IMS_NULL)
                {
                    m_pDialog->UpdateDialog(m_pCtState->GetDialog());
                }
            }

            StartTcpConnectionMonitoringTimer();
        }
    }

    // Update the state
    if (SipConnection::GetMethod().Equals(SipMethod::ACK))
    {
        SetState(STATE_COMPLETED);
    }
    else
    {
        SetState(STATE_PROCEEDING);
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);

    //// DEBUG
    if (!SipConnection::GetMethod().Equals(SipMethod::REGISTER))
    {
        SipDebug::Send(GetSlotId(), SipDebug::MSG_REQ, SipDebug::DIR_OUT,
                SipConnection::GetMethod().ToInt());
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT SipClientConnection::SetHeader(
        IN const AString& strName, IN const AString& strValue)
{
    if (m_nState != STATE_INITIALIZED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    return SipConnection::SetHeader(strName, strValue);
}

PUBLIC VIRTUAL const ByteArray& SipClientConnection::GetContent() const
{
    if ((m_nState != STATE_PROCEEDING) && (m_nState != STATE_COMPLETED))
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return ByteArray::ConstNull();
    }

    return SipConnection::GetContent();
}

PUBLIC VIRTUAL IMS_RESULT SipClientConnection::SetContent(IN const ByteArray& objContent)
{
    if (m_nState != STATE_INITIALIZED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    return SipConnection::SetContent(objContent);
}

PUBLIC VIRTUAL IMS_SINT32 SipClientConnection::GetHeaderCount(IN const AString& strName) const
{
    // Message is not initialized or the connection is closed
    if ((m_nState == STATE_CREATED) || (m_nState == STATE_TERMINATED))
    {
        return 0;
    }

    return SipConnection::GetHeaderCount(strName);
}

PUBLIC VIRTUAL void SipClientConnection::SetSipProfile(IN SipProfile* pProfile)
{
    if (!m_pCtState.IsNull())
    {
        m_pCtState->SetSipProfile(pProfile);
    }
}

PUBLIC
IMS_RESULT SipClientConnection::InitAck()
{
    if (m_nState != STATE_COMPLETED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    // If ACK request is already sent, throw INVALID_OPERATION
    if (m_bAckSent)
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    // If the method of this transaction is not INVITE, throw INVALID_OPERATION
    if (!SipConnection::GetMethod().Equals(SipMethod::INVITE))
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    IMS_SINT32 nStatusCode = GetStatusCode();

    if ((nStatusCode < SipStatusCode::SC_200) || (nStatusCode >= SipStatusCode::SC_300))
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    InitMessage();

    m_pCtState->UpdateMessage(m_pMessage->GetMessage());

    // Method
    m_pMessage->SetMethod(SipMethod(SipMethod::ACK));

    if (!m_pCtState->InitRequest(GetMethod(), IMS_NULL))
    {
        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    // Update the Request-URI if there are any changes
    // if (m_pCtState->IsTargetUpdated())
    m_pMessage->UpdateRequestUri();

    SetState(STATE_INITIALIZED);

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
SipClientConnection* SipClientConnection::InitCancel()
{
    if (m_nState != STATE_PROCEEDING)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_NULL;
    }

    if (!SipConnection::GetMethod().Equals(SipMethod::INVITE))
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_NULL;
    }

    SipClientConnection* pCancel = new SipClientConnection(m_strTargetUri);

    if (pCancel == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::CONNECTION_NOT_FOUND);
        return IMS_NULL;
    }

    pCancel->InitMessage();

    pCancel->m_pCtState->UpdateMessage(pCancel->m_pMessage->GetMessage());

    if (!pCancel->m_pCtState->InitCancel(m_pCtState.Get()))
    {
        pCancel->Close();

        SipPrivate::SetLastError(SipError::CONNECTION_NOT_FOUND);
        return IMS_NULL;
    }

    pCancel->m_pMessage->SetMethod(SipMethod(SipMethod::CANCEL));

    // Update the Request-URI if there are any changes
    // if (pCANCEL->m_pCtState->IsTargetUpdated())
    pCancel->m_pMessage->UpdateRequestUri();

    pCancel->SetState(STATE_INITIALIZED);

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return pCancel;
}

PUBLIC
IMS_RESULT SipClientConnection::InitRequest(
        IN const AString& strMethod, IN SipConnectionNotifier* pScn)
{
    if (m_nState != STATE_CREATED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    SipMethod objMethod(strMethod);

    if (objMethod.Equals(SipMethod::ACK) || objMethod.Equals(SipMethod::CANCEL))
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    if ((objMethod.Equals(SipMethod::BYE)) || (objMethod.Equals(SipMethod::NOTIFY)) ||
            (objMethod.Equals(SipMethod::PRACK)) || (objMethod.Equals(SipMethod::UPDATE)))
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    /*
     * The following headers are set by the system :
     *        To
     *        From
     *        CSeq
     *        Call-ID
     *        Max-Forwards
     *        Via
     *        Contact
     */

    InitMessage();

    m_pCtState->UpdateMessage(m_pMessage->GetMessage());

    if (!m_pCtState->InitRequest(objMethod))
    {
        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    // Method
    m_pMessage->SetMethod(objMethod);

    // Request-URI
    m_pMessage->SetRequestUri(m_strTargetUri);

    // From & Contact header
    // If SCN is not shared, From & Contact will be set by the application (J180 Engine)
    // From header
    if (pScn != IMS_NULL)
    {
        m_pMessage->SetHeader(ISipHeader::FROM, pScn->GetUserIdentity());
    }
    else
    {
        m_pMessage->SetHeader(ISipHeader::FROM, ANONYMOUS_URI);
    }

    // To header
    m_pMessage->SetHeader(ISipHeader::TO, m_strTargetUri);

    // Call-ID : after setting PD connection ??? ---> It will be set in the transport layer

    // Contact header if SCN exists
    if (objMethod.Equals(SipMethod::REGISTER) || objMethod.Equals(SipMethod::INVITE) ||
            objMethod.Equals(SipMethod::SUBSCRIBE) || objMethod.Equals(SipMethod::REFER))
    {
        if (pScn != IMS_NULL)
        {
            m_pMessage->SetHeader(ISipHeader::CONTACT_NORMAL, pScn->GetContactAddress());
        }
    }

    // Create a dialog if the dialog can be created
    if (SipDialogBase::IsDialogCreatable(objMethod))
    {
        m_pDialog = new SipDialog(m_pCtState->GetDialog());
    }

    SetState(STATE_INITIALIZED);

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipClientConnection::Receive(IN IMS_SLONG /* nTimeout = 0 */)
{
    if ((m_nState != STATE_PROCEEDING) && (m_nState != STATE_COMPLETED))
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    if (m_objResponseMessages.IsEmpty())
    {
        SipPrivate::SetLastError(SipError::NO_MESSAGE);
        return IMS_FAILURE;
    }

    sipcore::SipMessage* pMessage = m_objResponseMessages.GetAt(0);

    if (pMessage == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::NO_MESSAGE);
        return IMS_FAILURE;
    }

    // Initialize SIP connection with new response message
    InitMessage(pMessage, sipcore::SipMessage::TYPE_RESPONSE);
    m_pCtState->UpdateMessage(pMessage->GetMessage());

    m_objResponseMessages.RemoveAt(0);

    IMS_SINT32 nStatusCode = SipConnection::GetStatusCode();

    if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        SetState(STATE_UNAUTHORIZED);
    }
    else if ((nStatusCode >= SipStatusCode::SC_100) && (nStatusCode < SipStatusCode::SC_200))
    {
        SetState(STATE_PROCEEDING);
    }
    else
    {
        SetState(STATE_COMPLETED);
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipClientConnection::SetCredentials(IN ImsList<Credential>& objCredentials)
{
    if ((m_nState != STATE_INITIALIZED) && (m_nState != STATE_UNAUTHORIZED))
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    if (objCredentials.IsEmpty())
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (m_pAuHelper == IMS_NULL)
    {
        m_pAuHelper = new SipAuHelper();
    }

    if (m_pAuHelper == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < objCredentials.GetSize(); ++i)
    {
        if (!m_pAuHelper->AddCredential(objCredentials.GetAt(i)))
        {
            SipPrivate::SetLastError(SipError::GENERAL_ERROR);
            return IMS_FAILURE;
        }
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipClientConnection::SetCredentials(IN const Credential& objCredential)
{
    if ((m_nState != STATE_INITIALIZED) && (m_nState != STATE_UNAUTHORIZED))
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    IMS_BOOL bPasswordLogging = IMS_TRUE;

    if (SipRtConfigUtils::IsMessageHiddenInLog(GetSlotId()) ||
            (objCredential.GetType() == Credential::TYPE_AKAv1_MD5) ||
            (objCredential.GetType() == Credential::TYPE_AKAv2_MD5))
    {
        bPasswordLogging = IMS_FALSE;
    }

    // LOG_EXCLUDING_SERVER_INFO
    IMS_TRACE_D("Credential :: UserName (%s), Password (%s), Realm (%s)",
            SipDebug::GetCharA1(objCredential.GetUsername().GetStr(), 6),
            bPasswordLogging ? objCredential.GetPassword().GetStr() : "xxx",
            SipDebug::GetCharA2(objCredential.GetRealm().GetStr(), 4));

    if ((objCredential.GetType() == Credential::TYPE_AKAv1_MD5) ||
            (objCredential.GetType() == Credential::TYPE_AKAv2_MD5))
    {
        // In case of AKA authentication, the password can be an empty (AUTS/MAC failure)
        if ((objCredential.GetUsername().GetLength() == 0) ||
                (objCredential.GetRealm().GetLength() == 0))
        {
            SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
            return IMS_FAILURE;
        }
    }
    else
    {
        if ((objCredential.GetUsername().GetLength() == 0) ||
                (objCredential.GetPassword().GetLength() == 0) ||
                (objCredential.GetRealm().GetLength() == 0))
        {
            SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
            return IMS_FAILURE;
        }
    }

    if (m_pAuHelper == IMS_NULL)
    {
        m_pAuHelper = new SipAuHelper();
    }

    if (m_pAuHelper == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    if (!m_pAuHelper->AddCredential(objCredential))
    {
        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipClientConnection::SetRequestUri(IN const AString& strUri)
{
    if (m_nState != STATE_INITIALIZED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    m_strTargetUri = strUri;

    return m_pMessage->SetRequestUri(strUri);
}

PUBLIC
ISipGenericChallenge* SipClientConnection::GetAuthenticationChallenge(
        IN IMS_SINT32 nIndex /* = 0 */) const
{
    if (m_pAuHelper == IMS_NULL)
    {
        return IMS_NULL;
    }

    return m_pAuHelper->GetChallenge(nIndex);
}

PUBLIC
ISipAckPackage* SipClientConnection::GrabAck()
{
    if (m_pDialog == IMS_NULL)
    {
        return IMS_NULL;
    }

    SipAckPackage* pAckPackage = SipAckPackage::CreateAckPackage(m_pDialog->GetCallId());

    if (pAckPackage != IMS_NULL)
    {
        IMS_BOOL bSipConfigRequired = IMS_TRUE;
        IMS_SINT32 nAliveInterval = 2000 * 64;
        SipTimerValues* pTimerValues = GetTransactionTimerValues();

        if (pTimerValues != IMS_NULL)
        {
            IMS_SINT32 nTimerValue = pTimerValues->GetValue(SipTimerValues::TIMER_H);

            if (nTimerValue > 0)
            {
                bSipConfigRequired = IMS_FALSE;
                nAliveInterval = nTimerValue;
            }
            else
            {
                nTimerValue = pTimerValues->GetValue(SipTimerValues::TIMER_T1);

                if (nTimerValue > 0)
                {
                    bSipConfigRequired = IMS_FALSE;
                    nAliveInterval = nTimerValue * 64;
                }
            }
        }

        if (bSipConfigRequired)
        {
            IMS_SINT32 nTimerValueT1 =
                    SipConfigProxy::GetTimerValueT1(GetSlotId(), m_pCtState->GetSipProfile());

            if (nTimerValueT1 > 0)
            {
                nAliveInterval = nTimerValueT1 * 64;
            }
        }

        pAckPackage->AddAck(m_pCtState.Get(), nAliveInterval);
    }

    return pAckPackage;
}

PUBLIC
IMS_RESULT SipClientConnection::InitResubmissionRequest()
{
    if (m_bResubmissionRequestInitialized)
    {
        IMS_TRACE_E(0, "Resubmission request message is already initialized", 0, 0, 0);
        return IMS_SUCCESS;
    }

    if (m_nState != STATE_UNAUTHORIZED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    // Form a new request w/ credentials and sent it to the network.
    sipcore::SipMessage* pMessage = new sipcore::SipMessage(m_pCtState->GetLastMessage());

    InitMessage(pMessage, sipcore::SipMessage::TYPE_REQUEST);

    m_pCtState->UpdateMessage(pMessage->GetMessage());

    m_bResubmissionRequestInitialized = IMS_TRUE;

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
void SipClientConnection::RemoveAllChallenges()
{
    if (m_pAuHelper == IMS_NULL)
    {
        return;
    }

    m_pAuHelper->RemoveAllChallenges();
}

PUBLIC
void SipClientConnection::RemoveAllCredentials()
{
    if (m_pAuHelper == IMS_NULL)
    {
        return;
    }

    m_pAuHelper->RemoveAllCredentials();
}

PUBLIC
IMS_RESULT SipClientConnection::SetAuthenticationChallenge(IN ISipGenericChallenge* piChallenge)
{
    if (m_nState != STATE_INITIALIZED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    if (piChallenge == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (m_pAuHelper == IMS_NULL)
    {
        m_pAuHelper = new SipAuHelper();

        if (m_pAuHelper == IMS_NULL)
        {
            IMS_TRACE_E(0, "Instantiating SipAuHelper failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    if (!m_pAuHelper->AddChallenge(piChallenge))
    {
        IMS_TRACE_E(0, "Adding an authentication challenge failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC
void SipClientConnection::SetExtensionTokenForViaBranch(IN const AString& strToken)
{
    m_pCtState->SetExtensionTokenForViaBranch(strToken);
}

PUBLIC
void SipClientConnection::SetImplicitRouteHeader(IN const AString& strRouteHeader)
{
    m_pCtState->SetImplicitRouteHeader(strRouteHeader);
}

PUBLIC
void SipClientConnection::SetTransportTuple(IN const IpAddress& objIp, IN IMS_SINT32 nPortS,
        IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFc /*= Sip::PORT_UNSPECIFIED*/,
        IN IMS_SINT32 nTransportExt /*= Sip::TRANSPORT_EXT_ANY*/)
{
    m_pCtState->SetTransportTuple(objIp, nPortS, nPortC, nPortFc, nTransportExt);
}

PUBLIC
IMS_RESULT SipClientConnection::InitDialogRequest(
        IN const SipMethod& objMethod, IN SipDialogEx* pDialogEx)
{
    if (m_nState != STATE_CREATED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    InitMessage();

    m_pCtState->UpdateMessage(m_pMessage->GetMessage());

    if (!m_pCtState->InitRequest(objMethod, pDialogEx))
    {
        return IMS_FAILURE;
    }

    m_pMessage->SetMethod(objMethod);
    m_pMessage->UpdateRequestUri();

    // Create a dialog
    m_pDialog = new SipDialog(pDialogEx);

    if (m_pDialog == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_FAILURE;
    }

    SetState(STATE_INITIALIZED);

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipClientConnection::SendWithCredentials()
{
    if (m_pAuHelper == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    // Check later; if the credentials is not present,
    // it needs to be sent with "anonymous" & empty password.
    if (!m_pAuHelper->IsCredentialPresent())
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    if (!m_bResubmissionRequestInitialized)
    {
        if (InitResubmissionRequest() != IMS_SUCCESS)
        {
            SipPrivate::SetLastError(SipError::INVALID_OPERATION);
            return IMS_FAILURE;
        }
    }

    ::SipMessage* pSipMsg = m_pMessage->GetMessage();

    if (!m_pAuHelper->FormCredentials(pSipMsg))
    {
        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    if (!m_pMessage->FormMessageOnChallenge())
    {
        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    if (!m_pCtState->FormMessageForResubmissionRequest())
    {
        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    IMS_RESULT nResult = m_pTransmissionProxy->SendWithCredentials();

    if (nResult == SipClientTransmissionProxy::RESULT_NOK)
    {
        SipPrivate::SetLastError(SipError::TRANSPORT_ERROR);
        return IMS_FAILURE;
    }

    if (nResult == SipClientTransmissionProxy::RESULT_OK)
    {
        // Update a dialog info...
        if (SipDialogBase::IsDialogCreatable(GetMethod()))
        {
            if (m_pDialog != IMS_NULL)
            {
                m_pDialog->UpdateDialog(m_pCtState->GetDialog());
            }
        }

        StartTcpConnectionMonitoringTimer();
    }

    m_bResubmissionRequestInitialized = IMS_FALSE;

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL IMS_BOOL SipClientConnection::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_SIP_RESPONSE_RECEIVED:
        {
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnClientConnection_NotifyResponse(this);
            }
            return IMS_TRUE;
        }
        case AMSG_SIP_FORKED_RESPONSE_RECEIVED:
        {
            SipClientConnection* pScc = reinterpret_cast<SipClientConnection*>(objMsg.nLparam);

            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnClientConnection_NotifyForkedResponse(this, pScc);
            }
            else
            {
                if (pScc != IMS_NULL)
                {
                    pScc->Close();
                }
            }
            return IMS_TRUE;
        }
        default:
        {
            break;
        }
    }

    return EngineActivity::DispatchMessage(objMsg);
}

PRIVATE VIRTUAL void SipClientConnection::ClientTransactionState_ForkedResponseReceived(
        IN SipClientTransactionState* pCtState)
{
    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "No listener", 0, 0, 0);
        return;
    }

    SipClientConnection* pScc = new SipClientConnection(pCtState);

    IMS_TRACE_D("SCC :: Handling a forked response ...", 0, 0, 0);

    if (pScc == IMS_NULL)
    {
        return;
    }

    // Copy the transaction timer values
    SipTimerValues* pTimerValues = GetTransactionTimerValues();

    if (pTimerValues != IMS_NULL)
    {
        pScc->SetTransactionTimerValues(*pTimerValues);
    }

    pScc->m_strTargetUri = m_strTargetUri;

    ::SipMessage* pSipMsg = pCtState->GetMessage();
    SipMethod objMethod = SipStack::GetMethod(pSipMsg);

    if (SipDialogBase::IsDialogCreatable(objMethod))
    {
        pScc->m_pDialog = new SipDialog(pCtState->GetDialog());
    }

    // Set a SIP message from the previous request message
    sipcore::SipMessage* pMessage = new sipcore::SipMessage(pCtState->GetLastMessage());

    if (pMessage == IMS_NULL)
    {
        delete pScc;
        return;
    }

    pScc->InitMessage(pMessage);

    // Append the response message
    pMessage = new sipcore::SipMessage(pSipMsg);

    if (pMessage == IMS_NULL)
    {
        delete pScc;
        return;
    }

    if (!pScc->m_objResponseMessages.Append(pMessage))
    {
        delete pMessage;
        delete pScc;
        return;
    }

    pScc->SetState(STATE_PROCEEDING);

    //// DEBUG
    if (!SipConnection::GetMethod().Equals(SipMethod::REGISTER))
    {
        SipDebug::Send(GetSlotId(), SipDebug::MSG_RSP, SipDebug::DIR_IN,
                SipConnection::GetMethod().ToInt(), pMessage->GetStatusCode());
    }

    // Notify the forked response to the application
    PostMessage(AMSG_SIP_FORKED_RESPONSE_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(pScc));
}

PRIVATE VIRTUAL void SipClientConnection::ClientTransactionState_ResponseReceived(
        IN ::SipMessage* pSipMsg)
{
    StopTcpConnectionMonitoringTimer();

    IMS_SINT32 nStatusCode = SipStack::GetStatusCode(pSipMsg);

    //// DEBUG
    if (!SipConnection::GetMethod().Equals(SipMethod::REGISTER))
    {
        SipDebug::Send(GetSlotId(), SipDebug::MSG_RSP, SipDebug::DIR_IN,
                SipConnection::GetMethod().ToInt(), nStatusCode);
    }

    if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        if (m_pAuHelper != IMS_NULL)
        {
            m_pAuHelper->SetChallenges(pSipMsg);

            if (m_pAuHelper->IsCredentialPresent())
            {
                IMS_TRACE_D("___ AUTHENTICATION IS PROCESSED BY SIP STACK ...", 0, 0, 0);

                SetState(STATE_UNAUTHORIZED);

                if (Send() != IMS_SUCCESS)
                {
                    NotifyError(SipError::AUTHENTICATION_FAILED, AString("Authentication failed"));
                }
                return;
            }
        }
        else
        {
            IMS_TRACE_D("Authentication helper does not exist...", 0, 0, 0);

            m_pAuHelper = new SipAuHelper();

            if (m_pAuHelper == IMS_NULL)
            {
                NotifyError(SipError::AUTHENTICATION_FAILED, AString("Authentication failed"));
                return;
            }

            m_pAuHelper->SetChallenges(pSipMsg);
        }
    }

    sipcore::SipMessage* pMessage = new sipcore::SipMessage(pSipMsg);

    if (pMessage == IMS_NULL)
    {
        return;
    }

    if (!m_objResponseMessages.Append(pMessage))
    {
        delete pMessage;
        return;
    }

    // Notify the response to the application
    PostMessage(AMSG_SIP_RESPONSE_RECEIVED, 0, 0);
}

PROTECTED VIRTUAL void SipClientConnection::Transport_NotifyError(
        IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    // If the following conditions are met, the retransmission of original SIP request is requested.
    //    1) Socket is closed by peer. (SipError::TRANSPORT_E_CODE_104)
    //    2) TCP connection monitoring timer is running.
    //    3) No SIP response received.
    if (nCode == SipError::TRANSPORT_ERROR && IsTransportErrorCode104(strMessage) &&
            m_piTcpConnectionMonitoringTimer != IMS_NULL && GetStatusCode() < SipStatusCode::SC_100)
    {
        IMS_TRACE_I("SCC: TCP connection closed by peer, retransmit SIP request", 0, 0, 0);
        StopTcpConnectionMonitoringTimer();
        m_pCtState->RetransmitMessage();
        return;
    }

    SipConnection::Transport_NotifyError(nCode, strMessage);
}

// SIP_TRANSPORT_ERROR_REPORT_ON_TXN
PROTECTED VIRTUAL IMS_BOOL SipClientConnection::IsTransportErrorReportRequired(
        IN IMS_SINT32 nCode, IN const AString& strMessage) const
{
    if (nCode == SipError::TRANSPORT_ERROR)
    {
        // SipError::TRANSPORT_E_CODE_104
        if (IsTransportErrorCode104(strMessage))
        {
            if ((m_nState == STATE_PROCEEDING) &&
                    SipConfigProxy::IsTransportErrorReportOnTxnRequired(
                            GetSlotId(), m_pCtState->GetSipProfile()))
            {
                // Notifies "Socket is closed by peer"(104) error
                // if the SIP transaction is in progress
                return IMS_TRUE;
            }
        }
    }

    return SipConnection::IsTransportErrorReportRequired(nCode, strMessage);
}

PRIVATE VIRTUAL void SipClientConnection::ClientTransmission_NotifyError(
        IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    if (m_pTransmissionProxy == IMS_NULL)
    {
        return;
    }

    Transport_NotifyError(nCode, strMessage);
}

PRIVATE VIRTUAL void SipClientConnection::ClientTransmission_TransmissionCompleted()
{
    if (m_pTransmissionProxy == IMS_NULL)
    {
        return;
    }

    // Update a dialog info...
    if (SipDialogBase::IsDialogCreatable(GetMethod()))
    {
        if (m_pDialog != IMS_NULL)
        {
            m_pDialog->UpdateDialog(m_pCtState->GetDialog());
        }
    }

    StartTcpConnectionMonitoringTimer();
}

PRIVATE VIRTUAL void SipClientConnection::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (m_piTcpConnectionMonitoringTimer == piTimer)
    {
        IMS_TRACE_I("SCC: TCP connection monitoring timer expired", 0, 0, 0);
        StopTcpConnectionMonitoringTimer();
    }
}

PRIVATE
void SipClientConnection::StartTcpConnectionMonitoringTimer()
{
    if (!SipConnection::GetMethod().Equals(SipMethod::INVITE))
    {
        return;
    }

    SipTransport* pTransport = m_pCtState->GetSipTransport();

    if (pTransport == IMS_NULL ||
            pTransport->GetProtocol(SipTransport::TA_FAR) != SipTransportAddress::PROTOCOL_TCP)
    {
        return;
    }

    m_piTcpConnectionMonitoringTimer = TimerService::GetTimerService()->CreateTimer();
    SipTimerValues* pTimerValues = GetTransactionTimerValues();
    IMS_SINT32 nDuration =
            (pTimerValues != IMS_NULL) ? pTimerValues->GetValue(SipTimerValues::TIMER_T1) : 0;

    if (nDuration == 0)
    {
        nDuration = SipConfigProxy::GetTimerValueT1(GetSlotId(), m_pCtState->GetSipProfile(),
                SipConfigProxy::GetSipConfigV(GetSlotId()));
    }

    m_piTcpConnectionMonitoringTimer->SetTimer(nDuration, this);
}

PRIVATE
void SipClientConnection::StopTcpConnectionMonitoringTimer()
{
    if (m_piTcpConnectionMonitoringTimer != IMS_NULL)
    {
        m_piTcpConnectionMonitoringTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTcpConnectionMonitoringTimer);
    }
}

PRIVATE
void SipClientConnection::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("SCC :: %s to %s", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;
}

PRIVATE GLOBAL IMS_BOOL SipClientConnection::IsTransportErrorCode104(IN const AString& strMessage)
{
    AString strTransportErrorCode;
    strTransportErrorCode.Sprintf("%d", SipError::TRANSPORT_E_CODE_104);
    // SipError::TRANSPORT_E_CODE_104
    return strMessage.StartsWith(strTransportErrorCode);
}

PRIVATE GLOBAL const IMS_CHAR* SipClientConnection::StateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case STATE_CREATED:
            return "STATE_CREATED";
        case STATE_INITIALIZED:
            return "STATE_INITIALIZED";
        case STATE_PROCEEDING:
            return "STATE_PROCEEDING";
        case STATE_COMPLETED:
            return "STATE_COMPLETED";
        case STATE_UNAUTHORIZED:
            return "STATE_UNAUTHORIZED";
        case STATE_TERMINATED:
            return "STATE_TERMINATED";
        default:
            return "__INVALID__";
    }
}
