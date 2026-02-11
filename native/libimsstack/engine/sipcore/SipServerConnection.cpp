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
#include "ServiceTrace.h"

#include "private/ConfigurationManager.h"
#include "private/SipConfig.h"

#include "SipConfigProxy.h"
#include "SipDState.h"
#include "SipDebug.h"
#include "SipDialog.h"
#include "SipError.h"
#include "SipFeatures.h"
#include "SipPrivate.h"
#include "SipServerConnection.h"
#include "SipServerTransactionState.h"
#include "SipTransport.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipServerConnection::SipServerConnection(IN SipServerTransactionState* pStState) :
        SipConnection(),
        m_nState(STATE_CREATED),
        m_pStState(pStState),
        m_bClosePending(IMS_FALSE),
        m_piClosePendingTimer(IMS_NULL)
{
    m_pStState->SetTransactionListener(this);
    m_pStState->SetTransportListener(this);
}

PUBLIC VIRTUAL SipServerConnection::~SipServerConnection()
{
    if ((m_nState == STATE_REQUEST_RECEIVED) || (m_nState == STATE_INITIALIZED) ||
            (m_nState == STATE_PROVISIONAL_RESPONDED))
    {
        m_pStState->Abort();
    }

    m_pStState->SetTransactionListener(IMS_NULL);
    m_pStState->SetTransportListener(IMS_NULL);

    if (m_piClosePendingTimer != IMS_NULL)
    {
        m_piClosePendingTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piClosePendingTimer);
    }
}

PUBLIC VIRTUAL void SipServerConnection::Close()
{
    if ((m_nState == STATE_REQUEST_RECEIVED) || (m_nState == STATE_INITIALIZED) ||
            (m_nState == STATE_PROVISIONAL_RESPONDED))
    {
        m_pStState->Abort();
    }

    // Grab the server connection until the response is not completely sent to the network,
    // the transmission failed, or the timer is expired.
    if (WaitForMessageSent())
    {
        StartClosePendingTimer();
        IMS_TRACE_I("SSC close pending: %s", m_pMessage->GetMethod().ToString().GetStr(), 0, 0);
        return;
    }

    SetState(STATE_TERMINATED);
    SipConnection::Close();
}

PUBLIC VIRTUAL IMS_RESULT SipServerConnection::AddHeader(
        IN const AString& strName, IN const AString& strValue)
{
    if (m_nState != STATE_INITIALIZED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    return SipConnection::AddHeader(strName, strValue);
}

PUBLIC VIRTUAL AString SipServerConnection::GetHeader(
        IN const AString& strName, IN IMS_SINT32 nIndex /*= 0*/)
{
    // Message is not initialized or the connection is closed
    if (m_nState == STATE_TERMINATED)
    {
        SipPrivate::SetLastError(SipError::NO_ERROR);
        return AString::ConstNull();
    }

    return SipConnection::GetHeader(strName, nIndex);
}

PUBLIC VIRTUAL ImsList<AString> SipServerConnection::GetHeaders(IN const AString& strName)
{
    // Message is not initialized or the connection is closed
    if (m_nState == STATE_TERMINATED)
    {
        SipPrivate::SetLastError(SipError::NO_ERROR);
        return ImsList<AString>();
    }

    return SipConnection::GetHeaders(strName);
}

PUBLIC VIRTUAL const SipMethod& SipServerConnection::GetMethod() const
{
    // Message is not initialized or the connection is closed
    if (m_nState == STATE_TERMINATED)
    {
        return SipMethod::INVALID_METHOD;
    }

    return SipConnection::GetMethod();
}

PUBLIC VIRTUAL const AString& SipServerConnection::GetReasonPhrase() const
{
    // Status code is available if the state is in PROCEEDING, UNAUTHORIZED, and COMPLETED
    // Sync. problem : add the state checking (PROVISIONAL_RESPONDED, COMPLETED)
    if ((m_nState != STATE_INITIALIZED) && (m_nState != STATE_PROVISIONAL_RESPONDED) &&
            (m_nState != STATE_COMPLETED))
    {
        return AString::ConstNull();
    }

    return SipConnection::GetReasonPhrase();
}

PUBLIC VIRTUAL const AString& SipServerConnection::GetRequestUri() const
{
    // Message is not initialized or the connection is closed
    if (m_nState == STATE_TERMINATED)
    {
        return AString::ConstNull();
    }

    return SipConnection::GetRequestUri();
}

PUBLIC VIRTUAL IMS_SINT32 SipServerConnection::GetStatusCode() const
{
    // Status code is available if the state is in PROCEEDING, UNAUTHORIZED, and COMPLETED
    // Sync. problem : add the state checking (PROVISIONAL_RESPONDED, COMPLETED)
    if ((m_nState != STATE_INITIALIZED) && (m_nState != STATE_PROVISIONAL_RESPONDED) &&
            (m_nState != STATE_COMPLETED))
    {
        return 0;
    }

    return SipConnection::GetStatusCode();
}

PUBLIC VIRTUAL IMS_RESULT SipServerConnection::RemoveHeader(IN const AString& strName)
{
    if (m_nState != STATE_INITIALIZED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    return SipConnection::RemoveHeader(strName);
}

PUBLIC VIRTUAL IMS_RESULT SipServerConnection::Send()
{
    if ((m_nState != STATE_INITIALIZED) && (m_nState != STATE_PROVISIONAL_RESPONDED) &&
            (m_nState != STATE_COMPLETED))
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    IMS_SINT32 nStatusCode = SipConnection::GetStatusCode();

    if ((m_nState != STATE_INITIALIZED) && (SipConnection::GetMethod().Equals(SipMethod::INVITE)))
    {
        if (!SipStatusCode::IsFinalSuccess(nStatusCode) &&
                !(SipStatusCode::IsProvisional(nStatusCode) && m_pMessage->IsMessageRpr()))
        {
            SipPrivate::SetLastError(SipError::INVALID_OPERATION);
            return IMS_FAILURE;
        }

        IMS_TRACE_I("Retransmission of %d to INVITE request is requested", nStatusCode, 0, 0);

        if (!m_pMessage->FormMessageOnRetransmission())
        {
            SipPrivate::SetLastError(SipError::INVALID_MESSAGE);
            IMS_TRACE_E(0, "Forming SIP message for retransmission failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        // Retransmission of a provisional (reliable) or final response to INVITE request
        return m_pStState->RetransmitMessage();
    }
    else if (m_nState == STATE_COMPLETED)
    {
        IMS_TRACE_E(0, "Sending a SIP response in STATE_COMPLETED is not allowed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Throw exception - INVALID_MESSAGE if the message format was invalid
    if (!m_pMessage->FormMessage())
    {
        SipPrivate::SetLastError(SipError::INVALID_MESSAGE);
        return IMS_FAILURE;
    }

    if (nStatusCode == SipStatusCode::SC_100)
    {
        // Remove to-tag parameter
    }

    if (!m_pStState->FormMessage())
    {
        IMS_TRACE_E(0, "FormMessage() failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (SipFeatures::IsStandard2XXRetransmissionIntervalRequired(GetSlotId()))
    {
        AdjustTimerHFor2xx();
    }

    if (!m_pStState->Send(GetTransactionTimerValues()))
    {
        IMS_TRACE_E(0, "Send() failed", 0, 0, 0);

        if (SipStatusCode::IsProvisional(nStatusCode))
        {
            // To send a final response, transit the state.
            SetState(STATE_PROVISIONAL_RESPONDED);
        }

        return IMS_FAILURE;
    }

    // Update the state
    if (SipStatusCode::IsProvisional(nStatusCode))
    {
        SetState(STATE_PROVISIONAL_RESPONDED);
    }
    else
    {
        SetState(STATE_COMPLETED);
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);

    //// DEBUG
    SipDebug::Send(GetSlotId(), SipDebug::MSG_RSP, SipDebug::DIR_OUT,
            SipConnection::GetMethod().ToInt(), nStatusCode);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT SipServerConnection::SetHeader(
        IN const AString& strName, IN const AString& strValue)
{
    if (m_nState != STATE_INITIALIZED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    return SipConnection::SetHeader(strName, strValue);
}

PUBLIC VIRTUAL const ByteArray& SipServerConnection::GetContent() const
{
    if (m_nState != STATE_REQUEST_RECEIVED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return ByteArray::ConstNull();
    }

    return SipConnection::GetContent();
}

PUBLIC VIRTUAL IMS_RESULT SipServerConnection::SetContent(IN const ByteArray& objContent)
{
    if (m_nState != STATE_INITIALIZED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    return SipConnection::SetContent(objContent);
}

PUBLIC VIRTUAL IMS_SINT32 SipServerConnection::GetHeaderCount(IN const AString& strName) const
{
    // Message is not initialized or the connection is closed
    if (m_nState == STATE_TERMINATED)
    {
        return 0;
    }

    return SipConnection::GetHeaderCount(strName);
}

PUBLIC VIRTUAL SipProfile* SipServerConnection::GetSipProfile() const
{
    return m_pStState->GetSipProfile();
}

PUBLIC VIRTUAL void SipServerConnection::SetSipProfile(IN SipProfile* pProfile)
{
    m_pStState->SetSipProfile(pProfile);
}

PUBLIC
IMS_RESULT SipServerConnection::InitResponse(IN IMS_SINT32 nStatusCode)
{
    if ((m_nState != STATE_REQUEST_RECEIVED) && (m_nState != STATE_PROVISIONAL_RESPONDED))
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    if ((nStatusCode < SipStatusCode::SC_100) || (nStatusCode > SipStatusCode::SC_699))
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    SipMethod objMethod = SipConnection::GetMethod();

    // If MESSAGE & 200 OK is sent by the SIP core, then throw ALREADY_RESPONDED

    InitMessage(IMS_NULL, sipcore::SipMessage::TYPE_RESPONSE);

    m_pMessage->SetMethod(objMethod);
    m_pMessage->SetStatusCode(nStatusCode);
    m_pMessage->SetReasonPhrase(SipStatusCode::GetReasonPhrase(nStatusCode));

    m_pStState->UpdateMessage(m_pMessage->GetMessage());

    if (!m_pStState->InitResponse(nStatusCode))
    {
        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    SetState(STATE_INITIALIZED);

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipServerConnection::SetReasonPhrase(IN const AString& strReasonPhrase)
{
    if (m_nState != STATE_INITIALIZED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FAILURE;
    }

    if (strReasonPhrase.GetLength() == 0)
    {
        m_pMessage->SetReasonPhrase(AString::ConstEmpty());
    }
    else
    {
        m_pMessage->SetReasonPhrase(strReasonPhrase);
    }

    return IMS_SUCCESS;
}

PUBLIC
IMS_BOOL SipServerConnection::IsSameTransaction(IN const SipServerConnection* pOngoingSsc) const
{
    if (m_nState == STATE_TERMINATED)
    {
        SipPrivate::SetLastError(SipError::INVALID_STATE);
        return IMS_FALSE;
    }

    const SipMethod& objMethod = SipConnection::GetMethod();

    // Check if the method is CANCEL
    if (!objMethod.Equals(SipMethod::CANCEL))
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_FALSE;
    }

    if (pOngoingSsc == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FALSE;
    }

    if ((pOngoingSsc->m_nState == STATE_COMPLETED) || (pOngoingSsc->m_nState == STATE_TERMINATED))
    {
        // Ignore the CANCEL request because the ongoing transaction is already completed or
        // terminated.
        IMS_TRACE_D("Server transaction is in %s state",
                (pOngoingSsc->m_nState == STATE_COMPLETED) ? "COMPLETED" : "TERMINATED", 0, 0);

        // CANCEL_HANDLING_AFTER_200_OK_TO_INVITE
        // When INVITE_TXN_HANDLING_CORRECTION is disabled,
        // returns IMS_FALSE with SipError#NO_ERROR.
    }

    return m_pStState->IsSameTransaction(pOngoingSsc->m_pStState.Get());
}

PUBLIC
IMS_RESULT SipServerConnection::InitRequest()
{
    m_pMessage = new sipcore::SipMessage(m_pStState->GetMessage());

    if (m_pMessage == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_FAILURE;
    }

    SipDialogEx* pDialogEx = m_pStState->GetDialog();
    IMS_SINT32 nState = pDialogEx->GetState();

    // Case 1) If the request is received inside of an existing dialog (it is not in INIT state)
    // Case 2) If the dialog is in INIT state & the request can create a dialog
    // Case 3) If the dialog (created by SUBSCRIBE)  is in INIT state & NOTIFY request is received
    //       or NOTIFY request to forked SUBSCRIBE request
    if ((nState != SipDState::STATE_INIT) ||
            SipDialogBase::IsDialogCreatable(m_pMessage->GetMethod()) ||
            m_pMessage->GetMethod().Equals(SipMethod::NOTIFY))
    {
        m_pDialog = new SipDialog(pDialogEx);

        if (m_pDialog == IMS_NULL)
        {
            SipPrivate::SetLastError(SipError::NO_MEMORY);
            return IMS_FAILURE;
        }
    }

    if (SipConnection::GetMethod().Equals(SipMethod::ACK))
    {
        SetState(STATE_COMPLETED);
    }
    else
    {
        SetState(STATE_REQUEST_RECEIVED);
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);

    //// DEBUG
    SipDebug::Send(
            GetSlotId(), SipDebug::MSG_REQ, SipDebug::DIR_IN, SipConnection::GetMethod().ToInt());

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL void SipServerConnection::Transport_NotifyPendingMessageSent()
{
    SipConnection::Transport_NotifyPendingMessageSent();

    if (m_bClosePending)
    {
        ClosePendingConnection();
    }
}

PROTECTED VIRTUAL void SipServerConnection::Transport_NotifyError(
        IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    SipConnection::Transport_NotifyError(nCode, strMessage);

    if (m_bClosePending)
    {
        ClosePendingConnection();
    }
}

PROTECTED VIRTUAL void SipServerConnection::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (m_piClosePendingTimer != piTimer)
    {
        return;
    }

    ClosePendingConnection();
}

PRIVATE
void SipServerConnection::AdjustTimerHFor2xx()
{
    const SipMethod& objMethod = SipConnection::GetMethod();
    IMS_SINT32 nStatusCode = SipConnection::GetStatusCode();

    if (objMethod.Equals(SipMethod::INVITE) && SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        SipTimerValues* pTimerValues = GetTransactionTimerValues();

        if (pTimerValues != IMS_NULL)
        {
            IMS_SINT32 nT1 = pTimerValues->GetValue(SipTimerValues::TIMER_T1);

            if (nT1 == 0)
            {
                const SipConfig* pSipConfig =
                        ConfigurationManager::GetInstance()->GetSipConfig(GetSlotId());

                nT1 = SipConfigProxy::GetTimerValueT1(
                        GetSlotId(), m_pStState->GetSipProfile(), pSipConfig->GetSipConfigV());

                if (nT1 == 0)
                {
                    // 2s
                    nT1 = 2000;
                }
            }

            IMS_TRACE_D("TimerH(2XX) is adjusted: %d >> %d",
                    pTimerValues->GetValue(SipTimerValues::TIMER_H), (nT1 * 64), 0);

            pTimerValues->SetValue(SipTimerValues::TIMER_H, (nT1 * 64));
        }
    }
}

PRIVATE
void SipServerConnection::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("SSC: %s to %s", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;
}

PRIVATE
void SipServerConnection::ClosePendingConnection()
{
    IMS_TRACE_I("SSC: ClosePendingConnection", 0, 0, 0);

    if (m_piClosePendingTimer != IMS_NULL)
    {
        m_piClosePendingTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piClosePendingTimer);
        m_piClosePendingTimer = IMS_NULL;
    }

    if (m_nState != STATE_TERMINATED)
    {
        SetState(STATE_TERMINATED);
        SipConnection::Close();
    }
}

PRIVATE
void SipServerConnection::StartClosePendingTimer()
{
    if (m_piClosePendingTimer != IMS_NULL)
    {
        m_piClosePendingTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piClosePendingTimer);
    }

    IMS_UINT32 nTimerDuration = 2 * 64 * 1000;  // 128s
    const SipTimerValues* pTimerValues = GetTransactionTimerValues();

    if (pTimerValues != IMS_NULL)
    {
        // This case can be happened in non-INVITE transaction.
        IMS_SINT32 nTimerF = pTimerValues->GetValue(SipTimerValues::TIMER_F);

        if (nTimerF <= 0)
        {
            IMS_SINT32 nT1 = pTimerValues->GetValue(SipTimerValues::TIMER_T1);

            if (nT1 > 0)
            {
                nTimerF = nT1 * 64;
            }
        }

        if (nTimerF > 0)
        {
            nTimerDuration = nTimerF;
        }
    }

    m_piClosePendingTimer = TimerService::GetTimerService()->CreateTimer();

    if (m_piClosePendingTimer != IMS_NULL)
    {
        IMS_TRACE_D("SSC: starts a close pending timer", 0, 0, 0);
        m_piClosePendingTimer->SetTimer(nTimerDuration, this);
    }
    else
    {
        ClosePendingConnection();
    }
}

PRIVATE
IMS_BOOL SipServerConnection::WaitForMessageSent()
{
    if (m_nState == STATE_COMPLETED)
    {
        const SipTransport* pTransport = m_pStState->GetSipTransport();

        if ((pTransport != IMS_NULL) && pTransport->HasPendingMessage())
        {
            m_bClosePending = IMS_TRUE;
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL const IMS_CHAR* SipServerConnection::StateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case STATE_CREATED:
            return "STATE_CREATED";
        case STATE_REQUEST_RECEIVED:
            return "STATE_REQUEST_RECEIVED";
        case STATE_PROVISIONAL_RESPONDED:
            return "STATE_PROVISIONAL_RESPONDED";
        case STATE_INITIALIZED:
            return "STATE_INITIALIZED";
        case STATE_COMPLETED:
            return "STATE_COMPLETED";
        case STATE_TERMINATED:
            return "STATE_TERMINATED";
        default:
            return "__INVALID__";
    }
}
