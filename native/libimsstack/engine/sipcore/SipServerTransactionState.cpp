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

#include "SipStackManager.h"
#include "transport/SipTransportInfo.h"
#include "txn/SipTxn.h"

#include "ISipHeader.h"
#include "SipAddress.h"
#include "SipConfigProxy.h"
#include "SipDState.h"
#include "SipDebug.h"
#include "SipDialogEx.h"
#include "SipError.h"
#include "SipFactoryProxy.h"
#include "SipFeatures.h"
#include "SipHeaderUtils.h"
#include "SipManager.h"
#include "SipMessage.h"
#include "SipMessageInfo.h"
#include "SipMessageTracker.h"
#include "SipPrivate.h"
#include "SipServerTransactionState.h"
#include "SipServerTransport.h"
#include "SipStack.h"
#include "SipStackState.h"
#include "SipTimerValuesHelper.h"
#include "SipUtils.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipServerTransactionState::SipServerTransactionState(IN IMS_SINT32 nSlotId,
        IN const SipTransportAddress& objNearEnd, IN const SipTransportAddress& objFarEnd) :
        SipTransactionState(),
        m_piTimer100Trying(IMS_NULL)
{
    m_nType = TYPE_SERVER;
    m_pTransport = new SipServerTransport(nSlotId, objNearEnd, objFarEnd);
}

PUBLIC VIRTUAL SipServerTransactionState::~SipServerTransactionState()
{
    if (m_piTimer100Trying != IMS_NULL)
    {
        m_piTimer100Trying->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer100Trying);
        m_piTimer100Trying = IMS_NULL;
    }

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("dtor: SipServerTransactionState", 0, 0, 0);
#endif
}

PUBLIC VIRTUAL IMS_SINT32 SipServerTransactionState::CheckMessageValidity()
{
    // Update the transport information from the top Via header
    // (received & rport parameter handling)
    IMS_SINT32 nValidity = m_pTransport->ValidateViaHeader(m_pSipMsg);

    if (nValidity != SipPrivate::MESSAGE_VALID)
    {
        return nValidity;
    }

    // Set "received" & "rport" parameter if it needs to be set
    if (!m_pTransport->FormViaHeader(m_pSipMsg, GetSipProfile()))
    {
        return SipPrivate::MESSAGE_FAILED;
    }

    return SipPrivate::MESSAGE_VALID;
}

PUBLIC VIRTUAL IMS_BOOL SipServerTransactionState::FormMessage()
{
    // Update the transport information
    if (!UpdateTransportDetails())
    {
        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FALSE;
    }

    SipMethod objMethod = SipStack::GetMethod(m_pSipMsg);

    SetFlowControlOption(objMethod);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SipServerTransactionState::Send(
        IN SipTimerValues* pTimerValues /*= IMS_NULL*/)
{
    StopTimer100Trying(this);

    if (!UpdateTxnDetails())
    {
        IMS_TRACE_E(0, "Updating transaction details failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return SipTransactionState::Send(pTimerValues);
}

PUBLIC VIRTUAL IMS_BOOL SipServerTransactionState::UpdateTransportDetails()
{
    return SipTransactionState::UpdateTransportDetails();
}

PUBLIC
IMS_BOOL SipServerTransactionState::InitResponse(IN IMS_SINT32 nStatusCode)
{
    return InitResponse(nStatusCode, m_pSipMsg);
}

PUBLIC
IMS_BOOL SipServerTransactionState::IsSameTransaction(
        IN const SipServerTransactionState* pStState) const
{
    if (pStState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if ((m_pTxnKey == IMS_NULL) || (pStState->m_pTxnKey == IMS_NULL))
    {
        IMS_TRACE_D("No transaction key", 0, 0, 0);
        return IMS_FALSE;
    }

    // Compares these values : Call-ID, CSeq number, From-Tag, Via branch parameter
    if (!SipStack::CompareTxnKeysForCancel(m_pTxnKey, pStState->m_pTxnKey))
    {
        IMS_TRACE_D("Transaction Not Matched: Cancel(%s), Ongoing(%s:%s)",
                SipStack::TxnKey_GetViaBranch(m_pTxnKey),
                SipStack::TxnKey_GetMethod(pStState->m_pTxnKey),
                SipStack::TxnKey_GetViaBranch(pStState->m_pTxnKey));

        return IMS_FALSE;
    }

    IMS_TRACE_D("Transaction Matched: Cancel(%s), Ongoing(%s:%s)",
            SipStack::TxnKey_GetViaBranch(m_pTxnKey),
            SipStack::TxnKey_GetMethod(pStState->m_pTxnKey),
            SipStack::TxnKey_GetViaBranch(pStState->m_pTxnKey));

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 SipServerTransactionState::MatchTransaction(IN ::SipMessage* pSipMsg)
{
    // Update a new incoming request
    UpdateMessage(pSipMsg);

    const SipTransportAddress& objFarEnd = m_pTransport->GetAddress(SipTransport::TA_FAR);

    SipTransportParameter objTranspParam;
    objTranspParam.SetHostAddress(objFarEnd.GetIpAddress().ToString().GetStr());
    objTranspParam.SetPort(objFarEnd.GetPort());
    objTranspParam.SetTranspProtocol(objFarEnd.GetProtocol());

    if (objFarEnd.GetIpAddress().IsIPv4Address() == IMS_TRUE)
    {
        objTranspParam.SetTanspIpType(SipTransportInfo::NETWORK_IPV4);
    }
    else
    {
        objTranspParam.SetTanspIpType(SipTransportInfo::NETWORK_IPV6);
    }

    SipMethod objMethod = SipStack::GetMethod(pSipMsg);

    SipTxnContext* pTxnContext = SipStack::CreateTxnContext();

    if (pTxnContext == IMS_NULL)
    {
        return SipPrivate::MESSAGE_DISCARDED;
    }

    SipTxnContextData* pTxnContextData = new SipTxnContextData();
    if (pTxnContextData != IMS_NULL)
    {
        pTxnContextData->SetTxnState(this);
    }

    pTxnContext->m_pTxnContextData = static_cast<SIP_VOID*>(pTxnContextData);
    ISipUserData objUserData(static_cast<SIP_VOID*>(pTxnContext));
    IMS_SINT32 eTxnStatus = SipTxn::STATUS_INVALID;
    ::SipTxnKey* pTxnKey = IMS_NULL;
    IMS_UINT16 nError = 0;
    IMS_SINT32 nEncodingOptions = SipPrivate::GetEncodingOptions();
    IMS_UINT32 nMsgOptions = SipConfiguration::MSG_OPT_ENCODE_NONE;

    if ((nEncodingOptions & SipPrivate::OPT_E_SHORTFORM) != 0)
    {
        nMsgOptions |= SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM;
    }

    objUserData.SetMsgOptions(nMsgOptions);

    SIP_BOOL bStatus = SipStackManager::GetInstance()->OnRecvMessage(pSipMsg, &objTranspParam,
            &objUserData, reinterpret_cast<SIP_INT32*>(&eTxnStatus), &pTxnKey, &nError);

    // If it's NULL, new transaction grab this object.
    if (objUserData.GetUserData() != IMS_NULL)
    {
        if (objUserData.GetDeleteFlag() == SIP_TRUE)
        {
            SipTxnContext* pOldTxnContext =
                    reinterpret_cast<SipTxnContext*>(objUserData.GetUserData());

            if (pOldTxnContext != pTxnContext)
            {
                SipStack::DestroyTxnContext(pOldTxnContext);
            }
        }

        SipStack::DestroyTxnContext(pTxnContext);
    }

    if (bStatus == SIP_FALSE)
    {
        IMS_TRACE_D("MatchTransaction: OnRecvMessage failed(%d)", nError, 0, 0);
        return SipPrivate::MESSAGE_FAILED;
    }

    switch (eTxnStatus)
    {
        case SipTxn::STATUS_NEW_REQ_RECVD:
        {
            IMS_TRACE_I("__UAS__: ___ NEW REQUEST ___", 0, 0, 0);
            break;
        }
        case SipTxn::STATUS_VALID_MESSAGE:
        {
            IMS_TRACE_I("__UAS__: ___ VALID MESSAGE ___", 0, 0, 0);
            break;
        }
        case SipTxn::STATUS_IGNORE_REQ:
        {
            IMS_TRACE_I("__UAS__: ___ IGNORE REQUEST ___", 0, 0, 0);
            return SipPrivate::MESSAGE_DISCARDED;
        }
        case SipTxn::STATUS_IGNORE_RESP:
        {
            IMS_TRACE_I("__UAS__: ___ IGNORE RESPONSE ___", 0, 0, 0);
            return SipPrivate::MESSAGE_DISCARDED;
        }
        case SipTxn::STATUS_STRAY_RESP:
        {
            IMS_TRACE_I("__UAS__: ___ STRAY RESPONSE ___", 0, 0, 0);
            return SipPrivate::MESSAGE_DISCARDED;
        }
        case SipTxn::STATUS_RETRANSMISSION:
        {
            IMS_TRACE_I("__UAS__: ___ REMOTE RETRANSMISSION ___", 0, 0, 0);
            return SipPrivate::MESSAGE_DISCARDED;
        }
        case SipTxn::STATUS_ACK_RETRANSMISSION:
        {
            IMS_TRACE_I("__UAS__: ___ ACK RETRANSMISSION ___", 0, 0, 0);
            return SipPrivate::MESSAGE_DISCARDED;
        }
        case SipTxn::STATUS_ERROR_ON_SEND:    // FALL-THROUGH
        case SipTxn::STATUS_INVALID_MESSAGE:  // FALL-THROUGH
        case SipTxn::STATUS_INVALID:
        {
            IMS_TRACE_I("__UAS__: ___ PROCESSING FAILED ___", 0, 0, 0);
            return SipPrivate::MESSAGE_FAILED;
        }
        case SipTxn::STATUS_STRAY_PRACK:
        {
            IMS_TRACE_I("__UAS__: ___ STRAY PRACK RECEIVED ___", 0, 0, 0);
            return SipPrivate::MESSAGE_INVALID_481;
        }
        default:
            IMS_TRACE_I("__UAS__: ___ PROCESSING FAILED ___", 0, 0, 0);
            return SipPrivate::MESSAGE_FAILED;
    }

    if (pTxnKey != IMS_NULL)
    {
        SipStack::FreeTxnKey(m_pTxnKey);
        m_pTxnKey = pTxnKey;
    }

    if (m_pTxnKey != IMS_NULL)
    {
        if (m_pTxnKey->GetTxnType() == SipTxn::INVITE_SERVER)
        {
            m_nClass = CLASS_INVITE;
        }
        else if (m_pTxnKey->GetTxnType() == SipTxn::NON_INVITE_SERVER)
        {
            m_nClass = CLASS_REGULAR;
        }
    }

    SipMessageInfo objMsgInfo(GetSlotId(), objMethod, pSipMsg, SipMessageInfo::DIRECTION_INCOMING);
    LogSipMessageInfo(objMsgInfo);

    SipFactoryProxy* pFactoryProxy = SipFactoryProxy::GetInstance();

    // If the message is an ACK request for non-2xx response to INVITE request,
    // then stack drops the message by returning STATUS_IGNORE_REQ.
    if ((m_pTxnKey != IMS_NULL) && (m_pTxnKey->GetTxnType() == SipTxn::INVITE_SERVER))
    {
        if (objMethod.Equals(SipMethod::ACK) &&
                (m_pTxnKey->GetResponseCode() >= SipStatusCode::SC_300))
        {
            IMS_TRACE_I("__UAS__: ___ ACK (%s) TO UNSUCCESSFUL FINAL RESPONSE ___",
                    SipDebug::GetCharA1(m_pTxnKey->GetCallId(), 8, '@'), 0, 0);

            // SIP_MESSAGE_TRACKER
            if (pFactoryProxy->IsMessageTrackerEnabled(GetSlotId()))
            {
                SipMessageTracker* pMessageTracker = pFactoryProxy->GetMessageTracker(GetSlotId());
                pMessageTracker->NotifyMessageReceived(objMethod, m_pTxnKey->GetResponseCode(),
                        AString(static_cast<const IMS_CHAR*>(m_pTxnKey->GetCallId())));
            }
            return SipPrivate::MESSAGE_DISCARDED;
        }
    }

    // SIP_MESSAGE_TRACKER
    if (pFactoryProxy->IsMessageTrackerEnabled(GetSlotId()))
    {
        SipMessageTracker* pMessageTracker = pFactoryProxy->GetMessageTracker(GetSlotId());
        if (m_pTxnKey != IMS_NULL)
        {
            pMessageTracker->NotifyMessageReceived(
                    objMethod, 0, AString(static_cast<const IMS_CHAR*>(m_pTxnKey->GetCallId())));
        }
        else
        {
            pMessageTracker->NotifyMessageReceived(objMethod, 0, AString::ConstNull());
        }
    }
    return SipPrivate::MESSAGE_VALID;
}

PUBLIC
void SipServerTransactionState::RejectRequest(
        IN IMS_SINT32 nStatusCode, IN const AString& strReason /*= AString::ConstNull()*/)
{
    SipMethod objMethod = SipStack::GetMethod(m_pSipMsg);

    if (objMethod.ToInt() == SipMethod::ACK)
    {
        return;
    }

    if (m_pSipMsg != IMS_NULL)
    {
        SipStack::FreeMessage(m_pSipMsg);
    }

    m_pSipMsg = SipStack::CreateMessage(SipStack::SIP_MESSAGE_RESPONSE);

    if (m_pSipMsg == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a reject message failed", 0, 0, 0);
        return;
    }

    // Set the status code
    if (strReason.IsNULL())
    {
        if (!SipStack::SetStatusLine(
                    nStatusCode, SipStatusCode::GetReasonPhrase(nStatusCode), m_pSipMsg))
        {
            IMS_TRACE_E(0, "Setting a status-line failed", 0, 0, 0);
            return;
        }
    }
    else
    {
        if (!SipStack::SetStatusLine(nStatusCode, strReason, m_pSipMsg))
        {
            IMS_TRACE_E(0, "Setting a status-line failed", 0, 0, 0);
            return;
        }
    }

    if (!InitResponse(nStatusCode))
    {
        IMS_TRACE_E(0, "Initializing a response failed", 0, 0, 0);
        return;
    }

    IMS_BOOL bIsRetryAfterPresent = IMS_FALSE;

    // Retry-After header field
    //  In case of a second INVITE received before it sends the final response
    //  to a first INVITE with a lower
    //  CSeq sequence number on the same dialog.
    if (objMethod.Equals(SipMethod::INVITE) && (nStatusCode == SipStatusCode::SC_500) &&
            (m_pDialogEx->GetState() != SipDState::STATE_CONFIRMED))
    {
        // Sets randomly chosen value of between 1 and 10 seconds.
        AString strRetryAfterValue;
        strRetryAfterValue.SetNumber(SipHeaderUtils::GenerateRetryAfterSeconds(10));

        SipHeaderBase* pSipHdr =
                SipStack::DecodeHeader(ISipHeader::RETRY_AFTER_SEC, strRetryAfterValue);

        if (pSipHdr != IMS_NULL)
        {
            if (!SipStack::SetHeader(pSipHdr, m_pSipMsg))
            {
                SipStack::FreeHeaderEx(pSipHdr);

                IMS_TRACE_E(0, "Setting Retry-After header field failed", 0, 0, 0);
                return;
            }

            SipStack::FreeHeaderEx(pSipHdr);

            bIsRetryAfterPresent = IMS_TRUE;
        }
    }

    //// Add the specific headers according to the reject code

    // Retry-After : 404, 413, 480, 486, 500, 503, 600, 603
    // Allow : 200, 405
    // Supported : 200
    // Accept, Accept-Encoding, Accept-Language : 415
    // Unsupported : 420
    // Require : 421

    // Retry-After
    if ((!bIsRetryAfterPresent) &&
            ((nStatusCode == SipStatusCode::SC_404) || (nStatusCode == SipStatusCode::SC_413) ||
                    (nStatusCode == SipStatusCode::SC_480) ||
                    (nStatusCode == SipStatusCode::SC_486) ||
                    (nStatusCode == SipStatusCode::SC_500) ||
                    (nStatusCode == SipStatusCode::SC_503) ||
                    (nStatusCode == SipStatusCode::SC_600) ||
                    (nStatusCode == SipStatusCode::SC_603)))
    {
        // Sets randomly chosen value of between 1 and 10 seconds.
        AString strRetryAfterValue;
        strRetryAfterValue.SetNumber(SipHeaderUtils::GenerateRetryAfterSeconds(10));

        SipHeaderBase* pSipHdr =
                SipStack::DecodeHeader(ISipHeader::RETRY_AFTER_SEC, strRetryAfterValue);

        if (pSipHdr != IMS_NULL)
        {
            if (!SipStack::SetHeader(pSipHdr, m_pSipMsg))
            {
                SipStack::FreeHeaderEx(pSipHdr);

                IMS_TRACE_E(0, "Setting Retry-After header field failed", 0, 0, 0);
                return;
            }

            SipStack::FreeHeaderEx(pSipHdr);
        }
    }

    if (nStatusCode == SipStatusCode::SC_405)
    {
        // Sets the Allow header fields with all the supported methods
        for (IMS_SINT32 i = 0; i < SipMethod::MAX; ++i)
        {
            if ((i == SipMethod::REGISTER) || (i == SipMethod::PUBLISH) ||
                    (i == SipMethod::UNKNOWN))
            {
                continue;
            }

            SipHeaderBase* pSipHdr =
                    SipStack::DecodeHeader(ISipHeader::ALLOW, SipMethod::ToName(i));

            if (pSipHdr != IMS_NULL)
            {
                if (!SipStack::AppendHeader(pSipHdr, m_pSipMsg))
                {
                    SipStack::FreeHeaderEx(pSipHdr);

                    IMS_TRACE_E(0, "Setting Allow header field failed", 0, 0, 0);
                    return;
                }

                SipStack::FreeHeaderEx(pSipHdr);
            }
        }
    }

    // Set a default User-Agent or Server header field
    if (SipConfigProxy::IsUserAgentConfigured(GetSlotId(), GetSipProfile()))
    {
        AString strUaString = SipConfigProxy::GetUaString(GetSlotId(), GetSipProfile());

        if (strUaString.GetLength() > 0)
        {
            IMS_SINT32 nHeaderType;

            if (SipConfigProxy::IsUserAgentSetByContext(GetSlotId(), GetSipProfile()))
            {
                nHeaderType = ISipHeader::SERVER;
            }
            else
            {
                nHeaderType = ISipHeader::USER_AGENT;
            }

            SipHeaderBase* pUserAgentHdr =
                    SipStack::DecodeHeader(nHeaderType, AString::ConstNull(), strUaString);

            if (pUserAgentHdr != IMS_NULL)
            {
                (void)SipStack::SetHeader(pUserAgentHdr, m_pSipMsg);
                SipStack::FreeHeaderEx(pUserAgentHdr);
            }
        }
    }

    IMS_TRACE_I("RejectRequest: Sending %d response", nStatusCode, 0, 0);

    if (!FormMessage())
    {
        IMS_TRACE_E(0, "FormMessage failed", 0, 0, 0);
        return;
    }

    // Update the transaction timer values if the timer values need to be updated on runtime
    SipStackState::GetInstance()->SetTransactionTimerValues(GetSlotId(), GetSipProfile());

    IMS_SINT32 nTxnType = SipTimerValuesHelper::NON_INVITE_SERVER;

    if (objMethod.Equals(SipMethod::INVITE))
    {
        nTxnType = SipTimerValuesHelper::INVITE_SERVER;
    }

    SipTimerValues objTimerValues =
            SipTimerValuesHelper::GetValues(GetSlotId(), GetSipProfile(), nTxnType);

    Send(&objTimerValues);
}

PUBLIC
IMS_SINT32 SipServerTransactionState::HandleRequest(OUT RcPtr<SipDialogEx>& pOrigDialogEx)
{
    // Store the last request message
    SipStack::FreeMessage(m_pLastSipMsg);
    m_pLastSipMsg = SipStack::CloneMessage(m_pSipMsg);

    // Check the address type of Contact header for the response message (TLS)

    RcPtr<SipDialogState> pTempDState = new SipDialogState(IMS_FALSE);

    if (!pTempDState->InitDialogDetails(m_pSipMsg))
    {
        return SipPrivate::MESSAGE_FAILED;
    }

    IMS_SINT32 nValidity;
    IMS_BOOL bIsForked = IMS_FALSE;
    SipManager* pSIPMngr = SipManager::GetInstance();
    SipMethod objMethod = SipStack::GetMethod(m_pSipMsg);
    SipMessageInfo objMsgInfo(
            GetSlotId(), objMethod, m_pSipMsg, SipMessageInfo::DIRECTION_INCOMING);

    // Look up to identify if the dialog-state already exists or not
    RcPtr<SipDialogState> pOrigDState = pSIPMngr->LookupDialogState(pTempDState.Get(), m_pSipMsg);

    // Checking for the case where the 2xx to SUBSCRIBE is received after NOTIFY
    // to the subscription, each of them being from different users i.e. SUBSCRIBE was forked.
    if ((pOrigDState.IsNull()) && objMethod.Equals(SipMethod::NOTIFY))
    {
        pOrigDState =
                pSIPMngr->LookupDialogState(pTempDState.Get(), m_pSipMsg, IMS_TRUE, &bIsForked);
    }

    // The request is received outside of the dialog.
    if (pOrigDState.IsNull())
    {
        // If the method can create a dialog, make the dialog state to a permanent state.
        m_pDialogEx = SipDialogEx::CreateDialog(pTempDState.Get(), objMsgInfo);

        if (m_pDialogEx.IsNull())
        {
            IMS_TRACE_E(0, "NullPointer: Instantiating a dialog usage for %s failed",
                    objMethod.ToString().GetStr(), 0, 0);
            return SipPrivate::MESSAGE_FAILED;
        }

        // STRAY message handling
        // 1) ACK request will be discarded after its INVITE dialog is destroyed.
        // 2) The below requests are received after an INVITE dialog is destroyed.
        //   BYE, INFO, UPDATE, PRACK
        // 3) NOTIFY request is received after its SUBSCRIBE dialog is destroyed.
        if (objMethod.Equals(SipMethod::ACK))
        {
            // 4) ACK request is received, but discarded.
            return SipPrivate::MESSAGE_DISCARDED;
        }
        else if (objMethod.Equals(SipMethod::NOTIFY) || objMethod.Equals(SipMethod::BYE) ||
                objMethod.Equals(SipMethod::CANCEL) || objMethod.Equals(SipMethod::INFO) ||
                objMethod.Equals(SipMethod::UPDATE) || objMethod.Equals(SipMethod::PRACK))
        {
            RejectRequest(SipStatusCode::SC_481);
            SipPrivate::SetLastError(SipError::DIALOG_NOT_EXIST);

            return SipPrivate::MESSAGE_DISCARDED;
        }
        else
        {
            const SipDialogState* pDialogState = m_pDialogEx->GetDialogState();
            AString strToTag =
                    (pDialogState != IMS_NULL) ? pDialogState->GetLocalTag() : AString::ConstNull();

            if (strToTag.GetLength() > 0)
            {
                IMS_TRACE_E(0, "To-tag is already present, but no existing dialog.", 0, 0, 0);
                RejectRequest(SipStatusCode::SC_481);
                SipPrivate::SetLastError(SipError::DIALOG_NOT_EXIST);

                return SipPrivate::MESSAGE_DISCARDED;
            }
        }

        // Checks if CSeq value is exceeded
        IMS_UINT32 nSeqNum = SipStack::GetCSeqNumber(m_pSipMsg);

        if (nSeqNum > SipPrivate::MAX_CSEQ_VALUE)
        {
            IMS_TRACE_E(0, "CSeq number is exceeded", 0, 0, 0);

            RejectRequest(SipStatusCode::SC_400);
            SipPrivate::SetLastError(SipError::CSEQ_VALUE_EXCEEDED);

            return SipPrivate::MESSAGE_DISCARDED;
        }
    }
    // The request is received within the existing dialog.
    else
    {
        // The dialog is in EARLY state and this CANCEL is not related to the dialog,
        // so, the dialog for CANCEL is created with a default information.
        if (objMethod.Equals(SipMethod::CANCEL))
        {
            // Update the dialog details from INVITE-created dialog.
            pTempDState->InitDialogDetails(SipDialogState::DIALOG_CANCELLED, pOrigDState.Get());

            m_pDialogEx = SipDialogEx::CreateDialog(pTempDState.Get(), objMsgInfo);
        }
        else if (bIsForked && objMethod.Equals(SipMethod::NOTIFY))
        {
            // Update the dialog details from NOTIFY-created dialog.
            pTempDState->InitDialogDetails(
                    SipDialogState::DIALOG_FORKED_REQUEST, pOrigDState.Get());

            m_pDialogEx = SipDialogEx::CreateDialog(pTempDState.Get(), objMsgInfo);

            pOrigDialogEx = pOrigDState->GetDialogUsage(objMsgInfo);
        }
        // For abnormal case, BYE request received before receiving 1xx response to INVITE request
        else if (objMethod.Equals(SipMethod::BYE) && pOrigDState->IsCaller() &&
                (pOrigDState->GetState() == SipDState::STATE_INIT))
        {
            // Update the dialog details from INVITE-created dialog.
            pTempDState->InitDialogDetails(SipDialogState::DIALOG_CANCELLED, pOrigDState.Get());

            m_pDialogEx = SipDialogEx::CreateDialog(pTempDState.Get(), objMsgInfo);

            IMS_TRACE_D("___ BYE received before receiving 1xx-INVITE ___", 0, 0, 0);
        }
        else
        {
            // Find an existing dialog usage from this server transaction if present.
            m_pDialogEx = pOrigDState->GetDialogUsage(objMsgInfo);

            if (m_pDialogEx.IsNull())
            {
                // If the subscribe dialog usage is not found for this NOTIFY request,
                // then reject the request with 481.
                if (objMethod.Equals(SipMethod::NOTIFY))
                {
                    // Create a new dialog usage from this server transaction.
                    m_pDialogEx = SipDialogEx::CreateDialog(pTempDState.Get(), objMsgInfo);

                    if (!m_pDialogEx.IsNull())
                    {
                        RejectRequest(SipStatusCode::SC_481);
                    }

                    SipPrivate::SetLastError(SipError::DIALOG_NOT_EXIST);

                    return SipPrivate::MESSAGE_DISCARDED;
                }

                IMS_TRACE_D("___ CREATE A NEW DIALOG USAGE (%s) ___", objMethod.ToString().GetStr(),
                        0, 0);

                // Create a new dialog usage from this server transaction.
                m_pDialogEx = SipDialogEx::CreateDialog(pOrigDState.Get(), objMsgInfo);
            }
            else
            {
                IMS_TRACE_D("___ REUSE AN EXISTING DIALOG USAGE (%s) ___",
                        objMethod.ToString().GetStr(), 0, 0);
            }
        }

        // Request-URI VALIDATION
#ifdef __IMS_SIP_REQUEST_URI_VALIDATION__
        if (!objMethod.Equals(SipMethod::CANCEL))
        {
            SipAddrSpec* pAddrSpec = SipStack::GetRequestUri(m_pSipMsg);
            SipAddrSpec* pLocalAddrSpec = SipStack::GetAddrSpec(pOrigDState->GetLocalTargetUri());

            if ((pAddrSpec != IMS_NULL) && (pLocalAddrSpec != IMS_NULL))
            {
                AString strAddrSpec = AString::ConstNull();
                AString strLocalAddrSpec = AString::ConstNull();

                SipStack::EncodeAddrSpec(pAddrSpec, IMS_TRUE, strAddrSpec);
                SipStack::EncodeAddrSpec(pLocalAddrSpec, IMS_TRUE, strLocalAddrSpec);

                SipAddress objAddress(strAddrSpec);
                SipAddress objLocalAddress(strLocalAddrSpec);

                if (!objAddress.Equals(objLocalAddress))
                {
                    SipStack::FreeAddrSpec(pAddrSpec);
                    SipStack::FreeAddrSpec(pLocalAddrSpec);

                    IMS_TRACE_D("R-URI(%s|%s) is not matched",
                            SipDebug::GetUri1(strLocalAddrSpec).GetStr(),
                            SipDebug::GetUri2(strAddrSpec).GetStr(), 0);

                    if (!objMethod.Equals(SipMethod::ACK))
                    {
                        RejectRequest(SipStatusCode::SC_404);
                    }

                    return SipPrivate::MESSAGE_INVALID;
                }
            }
            else
            {
                SipStack::FreeAddrSpec(pAddrSpec);
                SipStack::FreeAddrSpec(pLocalAddrSpec);

                IMS_TRACE_D("R-URI is not matched", 0, 0, 0);

                if (!objMethod.Equals(SipMethod::ACK))
                {
                    RejectRequest(SipStatusCode::SC_404);
                }

                return SipPrivate::MESSAGE_INVALID;
            }

            SipStack::FreeAddrSpec(pAddrSpec);
            SipStack::FreeAddrSpec(pLocalAddrSpec);
        }
#endif

        // Validate a CSeq header
        nValidity = pOrigDState->ValidateRemoteCSeq(m_pSipMsg,
                ((m_pTxnKey != IMS_NULL) ? SipStack::TxnKey_GetStatusCode(m_pTxnKey) : 0));

        if (nValidity != SipPrivate::MESSAGE_VALID)
        {
            IMS_TRACE_E(0, "CSeq number is out of sequence", 0, 0, 0);

            switch (nValidity)
            {
                case SipPrivate::MESSAGE_INVALID_400:
                    RejectRequest(SipStatusCode::SC_400);
                    break;

                case SipPrivate::MESSAGE_INVALID_500:
                    RejectRequest(SipStatusCode::SC_500);
                    break;

                default:
                    break;
            }

            return nValidity;
        }
    }

    // METHOD INSPECTION
    //  In case of REGISTER & PUBLISH, it is not allowed in our device.
    //  So, if the method is REGISTER & PUBLISH, it will be rejected by the UA core.
    if (objMethod.Equals(SipMethod::REGISTER) || objMethod.Equals(SipMethod::PUBLISH))
    {
        RejectRequest(SipStatusCode::SC_405);
        return SipPrivate::MESSAGE_INVALID;
    }

    // VIA & RECORD-ROUTE header(s) handling
    // TIMESTAMP header handling

    nValidity = m_pDialogEx->UpdateDialogDetails(objMsgInfo);

    if (nValidity != SipPrivate::MESSAGE_VALID)
    {
        IMS_TRACE_E(0, "Updating the dialog details failed (%d)", nValidity, 0, 0);

        switch (nValidity)
        {
            case SipPrivate::MESSAGE_INVALID_400:
                RejectRequest(SipStatusCode::SC_400);
                break;

            case SipPrivate::MESSAGE_INVALID_481:
                RejectRequest(SipStatusCode::SC_481);
                break;

            case SipPrivate::MESSAGE_INVALID_500:
                RejectRequest(SipStatusCode::SC_500);
                break;

            default:
                break;
        }

        return nValidity;
    }

    if (Is100TryingResponseRequired(objMethod))
    {
        IMS_SINT32 nTimerValue100Trying = SipConfigProxy::GetTimerValue100Trying(GetSlotId());

        if (nTimerValue100Trying > 0)
        {
            // Start a timer for 100 Trying response transmission.
            StartTimer100Trying(this, nTimerValue100Trying);
        }
        else
        {
            // Send 100 Trying response immediately
            SendResponse100Trying(this);
        }
    }

    if (bIsForked)
    {
        return SipPrivate::MESSAGE_VALID_FORKED;
    }

    return SipPrivate::MESSAGE_VALID;
}

PRIVATE VIRTUAL void SipServerTransactionState::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (m_piTimer100Trying == IMS_NULL)
    {
        return;
    }

    if (m_piTimer100Trying != piTimer)
    {
        return;
    }

    IMS_TRACE_I("ServerTransactionState: 100 Trying timer expired", 0, 0, 0);

    SendResponse100Trying(this);

    m_piTimer100Trying->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer100Trying);
    m_piTimer100Trying = IMS_NULL;
}

PRIVATE
IMS_BOOL SipServerTransactionState::InitResponse(
        IN IMS_SINT32 nStatusCode, OUT ::SipMessage*& pOutSipMsg)
{
    SipHeaderBase* pPrevSipHdr;
    SipHeaderBase* pSipHdr;

    SipMethod objMethod = SipStack::GetMethod(m_pLastSipMsg);

    // CSeq
    pPrevSipHdr = SipStack::GetHeader(m_pLastSipMsg, ISipHeader::CSEQ);

    pSipHdr = SipStack::CloneHeader(pPrevSipHdr);
    SipStack::FreeHeaderEx(pPrevSipHdr);

    if (pSipHdr != IMS_NULL)
    {
        if (!SipStack::SetHeader(pSipHdr, pOutSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        SipStack::FreeHeaderEx(pSipHdr);
    }

    // From
    pPrevSipHdr = SipStack::GetHeader(m_pLastSipMsg, ISipHeader::FROM);

    pSipHdr = SipStack::CloneHeader(pPrevSipHdr);
    SipStack::FreeHeaderEx(pPrevSipHdr);

    if (pSipHdr != IMS_NULL)
    {
        if (!SipStack::SetHeader(pSipHdr, pOutSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        SipStack::FreeHeaderEx(pSipHdr);
    }

    // To
    pPrevSipHdr = SipStack::GetHeader(m_pLastSipMsg, ISipHeader::TO);

    pSipHdr = SipStack::CloneHeader(pPrevSipHdr);
    SipStack::FreeHeaderEx(pPrevSipHdr);

    if (pSipHdr != IMS_NULL)
    {
        // Check if the tag is present, and if not present, insert a new tag value
        if ((nStatusCode != SipStatusCode::SC_100) &&
                !SipStack::HasParameter(pSipHdr, Sip::STR_TAG))
        {
            AString strTagVal = m_pDialogEx->GetDialogState()->GetLocalTag();

            if (strTagVal.IsNULL())
            {
                strTagVal = SipUtils::GenerateTag(
                        SipConfigProxy::GetTagPrefix(GetSlotId(), GetSipProfile()));
            }

            if (!SipStack::SetParameter(pSipHdr, Sip::STR_TAG, strTagVal))
            {
                SipStack::FreeHeaderEx(pSipHdr);
                return IMS_FALSE;
            }
        }

        if (!SipStack::SetHeader(pSipHdr, pOutSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        SipStack::FreeHeaderEx(pSipHdr);
    }

    // Call-ID
    pPrevSipHdr = SipStack::GetHeader(m_pLastSipMsg, ISipHeader::CALL_ID);

    pSipHdr = SipStack::CloneHeader(pPrevSipHdr);
    SipStack::FreeHeaderEx(pPrevSipHdr);

    if (pSipHdr != IMS_NULL)
    {
        if (!SipStack::SetHeader(pSipHdr, pOutSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        SipStack::FreeHeaderEx(pSipHdr);
    }

    // Via headers with the same order
    IMS_SINT32 nHCount = SipStack::GetHeaderCount(m_pLastSipMsg, ISipHeader::VIA);

    for (IMS_SINT32 i = 0; i < nHCount; ++i)
    {
        pPrevSipHdr = SipStack::GetHeader(m_pLastSipMsg, ISipHeader::VIA, i);

        if (SipStack::IsValidHeader(pPrevSipHdr))
        {
            if (!SipStack::InsertHeader(pPrevSipHdr, i, pOutSipMsg))
            {
                SipStack::FreeHeaderEx(pPrevSipHdr);
                return IMS_FALSE;
            }
        }

        SipStack::FreeHeaderEx(pPrevSipHdr);
    }

    // Record-Route headers with the same order
    if (SipStatusCode::IsProvisional(nStatusCode) || SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        nHCount = SipStack::GetHeaderCount(m_pLastSipMsg, ISipHeader::RECORD_ROUTE);

        for (IMS_SINT32 i = 0; i < nHCount; ++i)
        {
            pPrevSipHdr = SipStack::GetHeader(m_pLastSipMsg, ISipHeader::RECORD_ROUTE, i);

            if (SipStack::IsValidHeader(pPrevSipHdr))
            {
                if (!SipStack::InsertHeader(pPrevSipHdr, i, pOutSipMsg))
                {
                    SipStack::FreeHeaderEx(pPrevSipHdr);
                    return IMS_FALSE;
                }
            }

            SipStack::FreeHeaderEx(pPrevSipHdr);
        }
    }

    // Contact
    IMS_BOOL bContactInAll1xxRequired =
            SipConfigProxy::IsContactInAll1xxRequired(GetSlotId(), GetSipProfile());

    if (SipDialogState::IsContactMandatory(sipcore::SipMessage::TYPE_RESPONSE, objMethod,
                nStatusCode, bContactInAll1xxRequired))
    {
        IMS_BOOL bContactHeaderInserted = IMS_FALSE;
        IMS_SINT32 nState = m_pDialogEx->GetState();

        if ((nState == SipDState::STATE_EARLY) || (nState == SipDState::STATE_CONFIRMED))
        {
            pSipHdr = m_pDialogEx->GetDialogState()->GetLocalTargetUri();

            if (pSipHdr != IMS_NULL)
            {
                if (SipStack::IsValidHeader(pSipHdr))
                {
                    if (!SipStack::SetHeader(pSipHdr, pOutSipMsg))
                    {
                        return IMS_FALSE;
                    }

                    bContactHeaderInserted = IMS_TRUE;
                }
            }
        }

        // If Contact header is not inserted, sets the Contact header to a default contact.
        if (!bContactHeaderInserted && !objMethod.Equals(SipMethod::MESSAGE) &&
                (m_strDefaultContact.GetLength() > 0))
        {
            pSipHdr = SipStack::DecodeHeader(ISipHeader::CONTACT_NORMAL, m_strDefaultContact);

            if (pSipHdr != IMS_NULL)
            {
                if (!SipStack::SetHeader(pSipHdr, pOutSipMsg))
                {
                    SipStack::FreeHeaderEx(pSipHdr);
                    return IMS_FALSE;
                }

                SipStack::FreeHeaderEx(pSipHdr);
            }
        }
    }

    // Timestamp header field, RFC 3261
    if (nStatusCode == SipStatusCode::SC_100)
    {
        pPrevSipHdr = SipStack::GetHeader(m_pLastSipMsg, ISipHeader::TIMESTAMP);

        if (SipStack::IsValidHeader(pPrevSipHdr))
        {
            if (!SipStack::SetHeader(pPrevSipHdr, pOutSipMsg))
            {
                SipStack::FreeHeaderEx(pPrevSipHdr);
                return IMS_FALSE;
            }
        }

        SipStack::FreeHeaderEx(pPrevSipHdr);
    }
    // HEADER_REQ_SESSION-ID
    else if (SipFeatures::IsHeaderSessionIdRequired(GetSlotId()))
    {
        const SipDialogState* pDState = m_pDialogEx->GetDialogState();
        AString strSessionId = pDState->GetSessionId();

        if (strSessionId.GetLength() == 0 && objMethod.Equals(SipMethod::INVITE))
        {
            strSessionId = SipUtils::GenerateSessionId(GetSlotId(), pDState->GetCallId());
        }

        if (strSessionId.GetLength() > 0)
        {
            SipHeaderBase* pSessionId = SipStack::DecodeHeader(
                    ISipHeader::SESSION_ID, AString::ConstNull(), strSessionId);

            if (pSessionId != IMS_NULL)
            {
                (void)SipStack::SetHeader(pSessionId, pOutSipMsg);
                SipStack::FreeHeaderEx(pSessionId);
            }
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SipServerTransactionState::UpdateTxnDetails()
{
    SipMethod objMethod = SipStack::GetMethod(m_pSipMsg);
    SipMessageInfo objMsgInfo(
            GetSlotId(), objMethod, m_pSipMsg, SipMessageInfo::DIRECTION_OUTGOING);
    IMS_SINT32 nValidity = m_pDialogEx->UpdateDialogDetails(objMsgInfo);

    if (nValidity != SipPrivate::MESSAGE_VALID)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE GLOBAL IMS_BOOL SipServerTransactionState::Is100TryingResponseRequired(
        IN const SipMethod& objMethod)
{
    // The INVITE server transaction MUST generate a 100 (Trying) response unless it knows
    // that the TU will generate a provisional or final response within 200 ms, in which case
    // it MAY generate a 100 (Trying) response.
    return objMethod.Equals(SipMethod::INVITE);
}

PRIVATE GLOBAL IMS_RESULT SipServerTransactionState::SendResponse100Trying(
        IN SipServerTransactionState* pStState)
{
    ::SipMessage* pSipMsg100Trying = SipStack::CreateMessage(SipStack::SIP_MESSAGE_RESPONSE);

    if (pSipMsg100Trying == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a 100 Trying response failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Set the status code
    AString strPhrase = SipStatusCode::GetReasonPhrase(SipStatusCode::SC_100);

    if (!SipStack::SetStatusLine(SipStatusCode::SC_100, strPhrase, pSipMsg100Trying))
    {
        SipStack::FreeMessage(pSipMsg100Trying);

        IMS_TRACE_E(0, "Setting a status-line of 100 Trying response failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Initialize a default header field from the previous request message
    if (!pStState->InitResponse(SipStatusCode::SC_100, pSipMsg100Trying))
    {
        SipStack::FreeMessage(pSipMsg100Trying);

        IMS_TRACE_E(0, "Initializing a 100 Trying response failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    IMS_TRACE_I("___ SENDING 100 \"TRYING\"", 0, 0, 0);

    // Update the transport information to route the response to the proper network entity
    if (pStState->m_pTransport != IMS_NULL)
    {
        pStState->m_pTransport->UpdateDestinationInfo(pSipMsg100Trying, pStState->GetSipProfile());
    }

    // Send SIP message to the network
    pStState->SipTransactionState::Send(pSipMsg100Trying, IMS_NULL);

    SipStack::FreeMessage(pSipMsg100Trying);

    return IMS_SUCCESS;
}

PRIVATE GLOBAL void SipServerTransactionState::StartTimer100Trying(
        IN SipServerTransactionState* pStState, IN IMS_SINT32 nTimerInterval /*milli-seconds*/)
{
    if (nTimerInterval > 0)
    {
        pStState->m_piTimer100Trying = TimerService::GetTimerService()->CreateTimer();

        if (pStState->m_piTimer100Trying != IMS_NULL)
        {
            // 200 ms
            pStState->m_piTimer100Trying->SetTimer(nTimerInterval, pStState);
        }
    }
}

PRIVATE GLOBAL void SipServerTransactionState::StopTimer100Trying(
        IN SipServerTransactionState* pStState)
{
    if (pStState->m_piTimer100Trying != IMS_NULL)
    {
        IMS_TRACE_I("___ Stopping 100 Trying timer", 0, 0, 0);

        pStState->m_piTimer100Trying->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(pStState->m_piTimer100Trying);
        pStState->m_piTimer100Trying = IMS_NULL;
    }
}
