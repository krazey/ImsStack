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

#include "ISipClientTransactionStateListener.h"
#include "ISipHeader.h"
#include "PAccessNetworkInfoHeader.h"
#include "SipAckPackage.h"
#include "SipClientTransactionState.h"
#include "SipClientTransport.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipDialogEx.h"
#include "SipFactoryProxy.h"
#include "SipFeatures.h"
#include "SipHeaderName.h"
#include "SipMessageTracker.h"
#include "SipPrivate.h"
#include "SipStack.h"
#include "SipStackState.h"
#include "SipTxnContextData.h"
#include "SipUtils.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SipClientTransactionState::SipClientTransactionState(IN IMS_SINT32 nSlotId) :
        SipTransactionState(),
        m_nRoutingType(TARGET_LR),
        m_pImplicitRoute(IMS_NULL),
        m_piCtsListener(IMS_NULL)
{
    m_nType = TYPE_CLIENT;
    m_pTransport = new SipClientTransport(nSlotId);
}

PUBLIC VIRTUAL SipClientTransactionState::~SipClientTransactionState()
{
    SipStack::FreeAddrSpec(m_pImplicitRoute);

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("Destructor :: SipClientTransactionState", 0, 0, 0);
#endif
}

PUBLIC VIRTUAL void SipClientTransactionState::Abort()
{
    // FORKED_RESPONSE
    if (RemoveForkedTransaction() > 0)
    {
        return;
    }

    SipTransactionState::Abort();
}

PUBLIC VIRTUAL IMS_SINT32 SipClientTransactionState::CheckMessageValidity()
{
    // Check the validity of transport information from the top Via header
    IMS_SINT32 nValidity = m_pTransport->ValidateViaHeader(m_pSipMsg);

    if (nValidity != SipPrivate::MESSAGE_VALID)
    {
        return nValidity;
    }

    return SipPrivate::MESSAGE_VALID;
}

PUBLIC VIRTUAL IMS_BOOL SipClientTransactionState::FormMessage()
{
    // Update the transport information
    if (!UpdateTransportDetails())
    {
        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FALSE;
    }

    SipMethod objMethod = SipStack::GetMethod(m_pSipMsg);

    if (!SetDialogRelatedHeaders(objMethod))
    {
        IMS_TRACE_E(0, "Setting a dialog-related headers failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!m_pTransport->FormViaHeader(m_pSipMsg, GetSipProfile()))
    {
        return IMS_FALSE;
    }

    SetFlowControlOption(objMethod);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SipClientTransactionState::FormMessageForResubmissionRequest()
{
    // Update the transport information
    // 4 Checks if the sec-agree is supported or not
    if (!UpdateTransportDetails())
    {
        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FALSE;
    }

    // Remove the previous Via header from the message
    SipStack::RemoveHeader(ISipHeader::VIA, m_pSipMsg);

    // Remove the from-tag
    IMS_BOOL bFromTagRemovalRequired = IMS_TRUE;

    if (!m_pDialogEx.IsNull())
    {
        if (m_pDialogEx->GetState() == SipDState::STATE_CONFIRMED)
        {
            // Do not remove "from-tag" from From header within SIP dialog
            bFromTagRemovalRequired = IMS_FALSE;
        }
    }

    if (bFromTagRemovalRequired)
    {
        SipHeaderBase* pFromHeader = SipStack::GetHeader(m_pSipMsg, ISipHeader::FROM);
        SipStack::RemoveParameter(Sip::STR_TAG, pFromHeader);
        SipStack::FreeHeaderEx(pFromHeader);
    }

    SipMethod objMethod = SipStack::GetMethod(m_pSipMsg);
    AString strHBody;

    // Sets CSeq header field
    strHBody.SetNumber(++m_nCSeqNumber);
    strHBody.Append(TextParser::CHAR_SP);
    strHBody.Append(objMethod.ToString());

    SipHeaderBase* pSipHdr = SipStack::DecodeHeader(ISipHeader::CSEQ, strHBody);

    if (!SipStack::SetHeader(pSipHdr, m_pSipMsg))
    {
        SipStack::FreeHeaderEx(pSipHdr);
        return IMS_FALSE;
    }

    SipStack::FreeHeaderEx(pSipHdr);

    if (!SetDialogRelatedHeaders(objMethod))
    {
        IMS_TRACE_E(0, "Setting a dialog-related headers failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!m_pTransport->FormViaHeader(m_pSipMsg, GetSipProfile()))
    {
        return IMS_FALSE;
    }

    SetFlowControlOption(objMethod);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SipClientTransactionState::InitTxnDetails(
        IN const SipTransactionState* pTState)
{
    const SipClientTransactionState* pCtState =
            DYNAMIC_CAST(const SipClientTransactionState*, pTState);

    if (pCtState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!SipTransactionState::InitTxnDetails(pCtState))
    {
        return IMS_FALSE;
    }

    m_nRoutingType = pCtState->m_nRoutingType;

    // IMPLICIT_ROUTE
    SipStack::FreeAddrSpec(m_pImplicitRoute);

    if (pCtState->m_pImplicitRoute != IMS_NULL)
    {
        m_pImplicitRoute = pCtState->m_pImplicitRoute;
        SipStack::AddReference(m_pImplicitRoute);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SipClientTransactionState::Send(
        IN SipTimerValues* pTimerValues /*= IMS_NULL*/)
{
    SipMethod objMethod = SipStack::GetMethod(m_pSipMsg);

    if (!UpdateTxnDetails(objMethod))
    {
        IMS_TRACE_E(0, "Updating transaction details failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return SipTransactionState::Send(pTimerValues);
}

PUBLIC VIRTUAL IMS_BOOL SipClientTransactionState::UpdateTransportDetails()
{
    IMS_BOOL bRoutingLr = (m_nRoutingType == TARGET_LR) ? IMS_TRUE : IMS_FALSE;
    IMS_BOOL bImplicitRouteRequired = IMS_FALSE;
    IMS_BOOL bCheckImplicitRouteUsage = IMS_FALSE;
    IMS_BOOL bIgnoreLr = IMS_FALSE;

    if (m_pImplicitRoute != IMS_NULL)
    {
        if (m_nRoutingType == TARGET_NO_ROUTE)
        {
            bCheckImplicitRouteUsage = IMS_TRUE;
        }
        else if (!m_pSipProfile.IsNull() &&
                m_pSipProfile->IsConfigurationSet(
                        SipProfile::CONFIG_IGNORE_RR_ON_IMPLICIT_ROUTE_REQUIRED))
        {
            bCheckImplicitRouteUsage = IMS_TRUE;
            bIgnoreLr = IMS_TRUE;
        }
    }

    // IMPLICIT_ROUTE
    if (bCheckImplicitRouteUsage)
    {
        IMS_SINT32 nDialogState = m_pDialogEx->GetState();

        // For routing all mid-dialog requests to the first outbound proxy
        if ((nDialogState == SipDState::STATE_INIT) || (nDialogState == SipDState::STATE_EARLY) ||
                (nDialogState == SipDState::STATE_CONFIRMED))
        {
            bImplicitRouteRequired = IMS_TRUE;
        }
        // BYE_REQUEST_ON_DIALOG_TERMINATED
        else if (m_pDialogEx->IsInviteUsage() && (nDialogState == SipDState::STATE_TERMINATED))
        {
            SipMethod objMethod = SipStack::GetMethod(m_pSipMsg);

            if (objMethod.Equals(SipMethod::BYE))
            {
                bImplicitRouteRequired = IMS_TRUE;
            }
        }

        if (bImplicitRouteRequired && bIgnoreLr)
        {
            bRoutingLr = IMS_FALSE;
            IMS_TRACE_I("ImplicitRoute :: Ignore route-set", 0, 0, 0);
        }
    }

    if (!m_pTransport->UpdateDestinationInfo(m_pSipMsg, bRoutingLr,
                (bImplicitRouteRequired == IMS_TRUE) ? m_pImplicitRoute : IMS_NULL))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipClientTransactionState::AdjustTransportProtocolAsUdp()
{
    if (m_pTransport == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SipStack::RemoveHeader(ISipHeader::VIA, m_pSipMsg);

    m_pTransport->SetProtocol(SipTransportAddress::PROTOCOL_UDP, SipTransport::TA_NEAR);
    m_pTransport->SetProtocol(SipTransportAddress::PROTOCOL_UDP, SipTransport::TA_FAR);
    m_pTransport->SetExplicitTargetProtocol(IMS_TRUE);

    if (!m_pTransport->FormViaHeader(m_pSipMsg, GetSipProfile()))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
::SipMessage* SipClientTransactionState::CreateAckRequest(IN ::SipMessage* pRespSipMsg)
{
    IMS_TRACE_D("CreateAckRequest", 0, 0, 0);

    ::SipMessage* pAckSipMsg = SipStack::CreateMessage(SipStack::SIP_MESSAGE_REQUEST);

    if (pAckSipMsg == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating an ACK request message failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (pRespSipMsg == IMS_NULL)
    {
        pRespSipMsg = m_pSipMsg;
    }

    // Initializes ACK request message
    if (!InitAck(pAckSipMsg, pRespSipMsg))
    {
        IMS_TRACE_E(0, "Initializing ACK request failed", 0, 0, 0);
        SipStack::FreeMessage(pAckSipMsg);
        return IMS_NULL;
    }

    return pAckSipMsg;
}

PUBLIC
IMS_BOOL SipClientTransactionState::InitCancel(IN const SipClientTransactionState* pInviteTState)
{
    SipHeaderBase* pPrevSipHdr;
    SipHeaderBase* pSipHdr;
    SipMethod objMethod(SipMethod::CANCEL);

    m_nClass = CLASS_REGULAR;

    SetSipProfile(pInviteTState->GetSipProfile());

    // Sets a SIP method name
    SipStack::SetMethod(objMethod, m_pSipMsg);

    // Sets a Request-URI field
    SipAddrSpec* pstAddrSpec = SipStack::GetRequestUri(pInviteTState->m_pLastSipMsg);

    (void)SipStack::SetRequestUri(pstAddrSpec, m_pSipMsg);
    SipStack::FreeAddrSpec(pstAddrSpec);

    AString strHBody;

    // Sets CSeq header field
    strHBody.SetNumber(pInviteTState->m_nCSeqNumber);
    strHBody.Append(TextParser::CHAR_SP);
    strHBody.Append(SipMethod::NAME[SipMethod::CANCEL]);

    pSipHdr = SipStack::DecodeHeader(ISipHeader::CSEQ, strHBody);

    if (!SipStack::SetHeader(pSipHdr, m_pSipMsg))
    {
        SipStack::FreeHeaderEx(pSipHdr);
        return IMS_FALSE;
    }

    SipStack::FreeHeaderEx(pSipHdr);

    // Sets Max-Forwards header field
    strHBody.SetNumber(SipPrivate::MAX_HOP);
    pSipHdr = SipStack::DecodeHeader(ISipHeader::MAX_FORWARDS, strHBody);

    if (!SipStack::SetHeader(pSipHdr, m_pSipMsg))
    {
        SipStack::FreeHeaderEx(pSipHdr);
        return IMS_FALSE;
    }

    SipStack::FreeHeaderEx(pSipHdr);

    // Sets From header field
    pPrevSipHdr = SipStack::GetHeader(pInviteTState->m_pLastSipMsg, ISipHeader::FROM);

    pSipHdr = SipStack::CloneHeader(pPrevSipHdr);
    SipStack::FreeHeaderEx(pPrevSipHdr);

    if (pSipHdr != IMS_NULL)
    {
        if (!SipStack::SetHeader(pSipHdr, m_pSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        SipStack::FreeHeaderEx(pSipHdr);
    }

    // Sets To header field
    pPrevSipHdr = SipStack::GetHeader(pInviteTState->m_pLastSipMsg, ISipHeader::TO);

    pSipHdr = SipStack::CloneHeader(pPrevSipHdr);
    SipStack::FreeHeaderEx(pPrevSipHdr);

    if (pSipHdr != IMS_NULL)
    {
        // Remove To-Tag if the CANCEL transaction is for initial INVITE request
        IMS_BOOL bInitialInvite = IMS_TRUE;

        if (!pInviteTState->m_pDialogEx.IsNull())
        {
            SipDialogState* pDState = pInviteTState->m_pDialogEx->GetDialogState();

            if (pDState != IMS_NULL)
            {
                if (pDState->GetState() == SipDState::STATE_CONFIRMED)
                {
                    bInitialInvite = IMS_FALSE;
                }
            }
        }

        if (bInitialInvite)
        {
            (void)SipStack::RemoveParameter(AString(Sip::STR_TAG), pSipHdr);
        }

        if (!SipStack::SetHeader(pSipHdr, m_pSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        SipStack::FreeHeaderEx(pSipHdr);
    }

    // Sets Call-ID header field
    pPrevSipHdr = SipStack::GetHeader(pInviteTState->m_pLastSipMsg, ISipHeader::CALL_ID);

    pSipHdr = SipStack::CloneHeader(pPrevSipHdr);
    SipStack::FreeHeaderEx(pPrevSipHdr);

    if (pSipHdr != IMS_NULL)
    {
        if (!SipStack::SetHeader(pSipHdr, m_pSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        SipStack::FreeHeaderEx(pSipHdr);
    }

    // Sets Via header field
    pPrevSipHdr = SipStack::GetHeader(pInviteTState->m_pLastSipMsg, ISipHeader::VIA);

    pSipHdr = SipStack::CloneHeader(pPrevSipHdr);
    SipStack::FreeHeaderEx(pPrevSipHdr);

    if (pSipHdr != IMS_NULL)
    {
        if (!SipStack::SetHeader(pSipHdr, m_pSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        SipStack::FreeHeaderEx(pSipHdr);
    }

    // Route
    IMS_SINT32 nHCount = SipStack::GetHeaderCount(pInviteTState->m_pLastSipMsg, ISipHeader::ROUTE);

    for (IMS_SINT32 i = 0; i < nHCount; i++)
    {
        pPrevSipHdr = SipStack::GetHeader(pInviteTState->m_pLastSipMsg, ISipHeader::ROUTE, i);

        if (SipStack::IsValidHeader(pPrevSipHdr))
        {
            if (!SipStack::AppendHeader(pPrevSipHdr, m_pSipMsg))
            {
                SipStack::FreeHeaderEx(pPrevSipHdr);
                return IMS_FALSE;
            }
        }

        SipStack::FreeHeaderEx(pPrevSipHdr);
    }

    // IMPLICIT_ROUTE
    SipStack::FreeAddrSpec(m_pImplicitRoute);

    if (pInviteTState->m_pImplicitRoute != IMS_NULL)
    {
        m_pImplicitRoute = pInviteTState->m_pImplicitRoute;
        SipStack::AddReference(m_pImplicitRoute);
    }

    // Update the transport information
    const SipTransportAddress& objNearAddr =
            pInviteTState->m_pTransport->GetAddress(SipTransport::TA_NEAR);
    const SipTransportAddress& objFarAddr =
            pInviteTState->m_pTransport->GetAddress(SipTransport::TA_FAR);

    // RFC5626_FLOW_CONTROL
    m_pTransport->SetTransportTuple(objNearAddr.GetIpAddress(), objNearAddr.GetPort(),
            pInviteTState->m_pTransport->GetPortC(),
            pInviteTState->m_pTransport->GetPortFlowControl(),
            pInviteTState->m_pTransport->GetTransportExt());

    // FIX_TRANSPORT_PROTOCOL
    m_pTransport->SetProtocol(objNearAddr.GetProtocol(), SipTransport::TA_NEAR);
    m_pTransport->SetProtocol(objFarAddr.GetProtocol(), SipTransport::TA_FAR);

    // Create a dialog
    m_pDialogEx = SipDialogEx::CreateDialog(objMethod);

    if (m_pDialogEx.IsNull())
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipClientTransactionState::InitRequest(IN const SipMethod& objMethod)
{
    // Set a mandatory headers in the INITIAL state
    //    - CSeq, Max-Forwards headers
    if (!SetMandatoryHeaders(objMethod))
    {
        return IMS_FALSE;
    }

    // Create a dialog usage according to the method
    m_pDialogEx = SipDialogEx::CreateDialog(objMethod);

    if (m_pDialogEx.IsNull())
    {
        return IMS_FALSE;
    }

    // FORKED_RESPONSE
    if (objMethod.Equals(SipMethod::INVITE))
    {
        m_pForkedTxnMngr = new SipForkedTransactionManager();

        if (m_pForkedTxnMngr.IsNull())
        {
            return IMS_FALSE;
        }

        m_pForkedTxnMngr->Add(this);

        // FORKED_RESPONSE_TO_SUPPORT_EARLY_DIALOG_TERMINATION
        // Grab the forked response manager when initiating the INVITE transaction
        m_pPersistentForkedTxnMngr = m_pForkedTxnMngr;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipClientTransactionState::InitRequest(
        IN const SipMethod& objMethod, IN SipDialogEx* pDialogEx)
{
    SipDialogState* pDState;

    if (!objMethod.Equals(SipMethod::ACK))
    {
        m_pDialogEx = pDialogEx;
        pDState = m_pDialogEx->GetDialogState();

        m_nCSeqNumber = pDState->GetNextCSeqNumber();
    }
    else
    {
        pDState = m_pDialogEx->GetDialogState();

        // For ACK transaction, reset the transport protocol to a default transport protocol
        m_pTransport->SetProtocol(SipTransportAddress::PROTOCOL_UDP, SipTransport::TA_NEAR);
        m_pTransport->SetProtocol(SipTransportAddress::PROTOCOL_UDP, SipTransport::TA_FAR);
    }

    // FORKED_RESPONSE
    if (objMethod.Equals(SipMethod::INVITE))
    {
        m_pForkedTxnMngr = new SipForkedTransactionManager();

        if (m_pForkedTxnMngr.IsNull())
        {
            return IMS_FALSE;
        }

        m_pForkedTxnMngr->Add(this);

        // FORKED_RESPONSE_TO_SUPPORT_EARLY_DIALOG_TERMINATION
        // Grab the forked response manager when initiating the INVITE transaction
        m_pPersistentForkedTxnMngr = m_pForkedTxnMngr;
    }

    // Set a mandatory headers in the INITIAL state
    //    - CSeq, Max-Forwards headers
    if (!SetMandatoryHeaders(objMethod))
    {
        return IMS_FALSE;
    }

    if (!pDState->InitRequest(objMethod, m_pSipMsg))
    {
        return IMS_FALSE;
    }

    if (!CorrectRouteHeader(m_pSipMsg))
    {
        return IMS_FALSE;
    }

    // P-Access-Network-Info header if required
    if (objMethod.Equals(SipMethod::ACK) && SipFeatures::IsPaniHeaderForAckRequired(GetSlotId()))
    {
        SetPaniHeader(objMethod, m_pSipMsg);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 SipClientTransactionState::RemoveForkedTransaction()
{
    if (!m_pForkedTxnMngr.IsNull())
    {
        m_pForkedTxnMngr->Remove(this);

        if (!m_pForkedTxnMngr->IsEmpty())
        {
            m_pForkedTxnMngr = IMS_NULL;
            return 1;
        }

        // FIX_NO_ACK_RETRANSMISSION
        if (m_pForkedTxnMngr->IsTransactionCompleted())
        {
            m_pForkedTxnMngr = IMS_NULL;
            return 1;
        }

        m_pForkedTxnMngr = IMS_NULL;
    }

    return 0;
}

PUBLIC
IMS_BOOL SipClientTransactionState::SendWithCredentials(
        IN SipTimerValues* pTimerValues /*= IMS_NULL*/)
{
    SipMethod objMethod = SipStack::GetMethod(m_pSipMsg);

    if (!UpdateTxnDetails(objMethod))
    {
        IMS_TRACE_E(0, "Updating transaction details failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Updates the sequence number on the dialog state, EARLY / CONFIRMED
    if (!m_pDialogEx.IsNull())
    {
        if ((m_pDialogEx->GetState() == SipDState::STATE_EARLY) ||
                (m_pDialogEx->GetState() == SipDState::STATE_CONFIRMED) ||
                // BYE_REQUEST_ON_DIALOG_TERMINATED
                (objMethod.Equals(SipMethod::BYE) &&
                        m_pDialogEx->GetState() == SipDState::STATE_TERMINATED))
        {
            m_pDialogEx->GetDialogState()->UpdateLocalCSeq(m_nCSeqNumber);
        }
    }

    return SipTransactionState::Send(pTimerValues);
}

PUBLIC
void SipClientTransactionState::SetExtensionTokenForViaBranch(IN const AString& strToken)
{
    SipClientTransport* pClientTransport = DYNAMIC_CAST(SipClientTransport*, m_pTransport);

    if (pClientTransport == IMS_NULL)
    {
        return;
    }

    pClientTransport->SetExtensionTokenForViaBranch(strToken);
}

PUBLIC
void SipClientTransactionState::SetImplicitRouteHeader(IN const AString& strRouteHeader)
{
    SipStack::FreeAddrSpec(m_pImplicitRoute);
    m_pImplicitRoute = IMS_NULL;

    if (strRouteHeader.GetLength() > 0)
    {
        m_pImplicitRoute = SipStack::DecodeAddrSpec(strRouteHeader);
    }
}

PUBLIC
IMS_BOOL SipClientTransactionState::UpdateRouteDetails(IN const SipMethod& objMethod)
{
    if (!CorrectRouteHeader(m_pSipMsg))
    {
        return IMS_FALSE;
    }

    SipAddrSpec* pAddrSpec = SipStack::GetRequestUri(m_pSipMsg);

    if (objMethod.Equals(SipMethod::REGISTER))
    {
        // Remove userinfo & password field if present
        SipStack::RemoveUserAndPassword(pAddrSpec);
    }

    SipStack::FreeAddrSpec(pAddrSpec);

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 SipClientTransactionState::HandleResponse(IN ::SipMessage* pSipMsg)
{
    // Check the message validity - CSeq number
    IMS_UINT32 nCSeqNum = SipStack::GetCSeqNumber(pSipMsg);

    if (m_nCSeqNumber != nCSeqNum)
    {
        // notify the error ???
        IMS_TRACE_E(0, "Sequence number is not ordered - Current (%d), New (%d)", m_nCSeqNumber,
                nCSeqNum, 0);
        return SipPrivate::MESSAGE_DISCARDED;
    }

    SipMethod objMethod = SipStack::GetMethod(pSipMsg);
    SipMessageInfo objMsgInfo(GetSlotId(), objMethod, pSipMsg, SipMessageInfo::DIRECTION_INCOMING);

    IMS_SINT32 nValidity = m_pDialogEx->GetDialogState()->CheckToTagValidity(objMsgInfo);

    // FORKED_RESPONSE
    if (nValidity == SipPrivate::MESSAGE_VALID_FORKED)
    {
        // Handle forked response
        IMS_TRACE_I("__UAC__ :: _____ FORKED RESPONSE (%s) RECEIVED _____",
                objMethod.ToString().GetStr(), 0, 0);

        if (!HandleForkedResponse(objMsgInfo))
        {
            IMS_TRACE_E(0, "Handling a forked response failed", 0, 0, 0);
            return SipPrivate::MESSAGE_DISCARDED;
        }

        return SipPrivate::MESSAGE_VALID;
    }
    else
    {
        if (nValidity != SipPrivate::MESSAGE_VALID)
        {
            return nValidity;
        }
    }

    // If the message is valid, then update the response message
    UpdateMessage(pSipMsg);

    // Initialize the transport info. when the response message is received
    m_pTransport->InitTransportOnMessageReceived(pSipMsg);

    // Validate the transport information
    nValidity = CheckMessageValidity();

    if (nValidity != SipPrivate::MESSAGE_VALID)
    {
        IMS_TRACE_E(nValidity, "Checking the message validity failed", 0, 0, 0);
        return nValidity;
    }

    nValidity = m_pDialogEx->UpdateDialogDetails(objMsgInfo);

    if (nValidity != SipPrivate::MESSAGE_VALID)
    {
        IMS_TRACE_E(nValidity, "Updating a dialog details failed", 0, 0, 0);
        return nValidity;
    }

    // Check if non-2xx response to INVITE, then the client transaction needs to send ACK request.
    if (objMethod.Equals(SipMethod::INVITE))
    {
        CheckNSendAck();
    }

    // FIX_NO_ACK_RETRANSMISSION
    if (!m_pForkedTxnMngr.IsNull())
    {
        IMS_SINT32 nStatusCode = SipStack::GetStatusCode(pSipMsg);

        if (SipStatusCode::IsFinal(nStatusCode))
        {
            m_pForkedTxnMngr->SetTransactionCompleted(nStatusCode);
        }
    }

    // Notify the response to SipClientConnection
    if (m_piCtsListener != IMS_NULL)
    {
        m_piCtsListener->ClientTransactionState_ResponseReceived(pSipMsg);
    }
    else
    {
        IMS_TRACE_E(0, "No listener (%s)",
                SipDebug::GetCharA1(m_pDialogEx->GetDialogState()->GetCallId().GetStr(), 8, '@'), 0,
                0);
    }

    return SipPrivate::MESSAGE_VALID;
}

PUBLIC GLOBAL IMS_SINT32 SipClientTransactionState::MatchTransaction(IN ::SipMessage* pSipMsg,
        IN const SipTransportAddress& objFarEnd, OUT RcPtr<SipClientTransactionState>& pCtState)
{
    IMS_TRACE_I("SipClientTransactionState::MatchTransaction ... ", 0, 0, 0);

    SipMethod objMethod = SipStack::GetMethod(pSipMsg);

    if (objMethod.Equals(SipMethod::ACK) && !SipStack::IsRequestMessage(pSipMsg))
    {
        IMS_TRACE_I("__UAC__ :: __ACK RESPONSE RECEIVED__", 0, 0, 0);
        return SipPrivate::MESSAGE_DISCARDED;
    }

    SipTransportParameter objTranspParam;

    /* Fill transport details */
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

    /* Prepare User data */
    ISipUserData objUserData;
    ::SipTxnKey* pTxnKey = IMS_NULL;
    IMS_SINT32 eTxnStatus = SipTxn::STATUS_INVALID;
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

    if (bStatus == SIP_FALSE)
    {
        IMS_TRACE_E(nError, "OnRecvMessage() Failed", 0, 0, 0);
        return SipPrivate::MESSAGE_FAILED;
    }

    switch (eTxnStatus)
    {
        case SipTxn::STATUS_NEW_REQ_RECVD:
        {
            IMS_TRACE_I("__UAC__ :: _____ NEW REQUEST _____", 0, 0, 0);
            break;
        }
        case SipTxn::STATUS_VALID_MESSAGE:
        {
            IMS_TRACE_I("__UAC__ :: _____ VALID MESSAGE _____", 0, 0, 0);
            break;
        }
        case SipTxn::STATUS_2XX_STRAY_RESP:
        {
            IMS_TRACE_I("__UAC__ :: _____STRAY 2XX RESPONSE _____", 0, 0, 0);
            if (!SipAckPackage::HandleStray2xx(pSipMsg))
            {
                IMS_TRACE_E(0, "__UAC__ :: 2XX RETRANSMISSION HANDLING FAILED.", 0, 0, 0);
            }
            return SipPrivate::MESSAGE_DISCARDED;
        }
        case SipTxn::STATUS_IGNORE_REQ:
        {
            IMS_TRACE_I("__UAC__ :: _____ IGNORE REQUEST _____", 0, 0, 0);
            return SipPrivate::MESSAGE_DISCARDED;
        }
        case SipTxn::STATUS_IGNORE_RESP:
        {
            IMS_TRACE_I("__UAC__ :: _____ IGNORE RESPONSE _____", 0, 0, 0);
            return SipPrivate::MESSAGE_DISCARDED;
        }
        case SipTxn::STATUS_STRAY_RESP:
        {
            IMS_TRACE_I("__UAC__ :: _____ STRAY RESPONSE _____", 0, 0, 0);
            return SipPrivate::MESSAGE_DISCARDED;
        }
        case SipTxn::STATUS_RETRANSMISSION:
        {
            IMS_TRACE_I("__UAC__ :: _____ REMOTE RETRANSMISSION _____", 0, 0, 0);
            return SipPrivate::MESSAGE_DISCARDED;
        }
        case SipTxn::STATUS_ERROR_ON_SEND:    // FALL-THROUGH
        case SipTxn::STATUS_INVALID_MESSAGE:  // FALL-THROUGH
        case SipTxn::STATUS_INVALID:
        {
            IMS_TRACE_I("__UAC__ :: _____ PROCESSING FAILED _____", 0, 0, 0);
            return SipPrivate::MESSAGE_FAILED;
        }
        default:
            IMS_TRACE_I("__UAC__ :: _____ PROCESSING FAILED _____", 0, 0, 0);
            return SipPrivate::MESSAGE_FAILED;
    }

    SipTxnContext* pTxnContext = static_cast<SipTxnContext*>(objUserData.GetUserData());

    if (pTxnContext == IMS_NULL)
    {
        SipStack::FreeTxnKey(pTxnKey);
        IMS_TRACE_E(0, "SipTxnContext is null", 0, 0, 0);
        return SipPrivate::MESSAGE_FAILED;
    }

    SipTxnContextData* pTxnContextData =
            static_cast<SipTxnContextData*>(pTxnContext->pTxnContextData);

    if (pTxnContextData == IMS_NULL)
    {
        if (objUserData.GetDeleteFlag() == SIP_TRUE)
        {
            SipStack::DestroyTxnContext(pTxnContext);
        }

        SipStack::FreeTxnKey(pTxnKey);
        // fatal error
        IMS_TRACE_E(0, "Getting the transaction context data failed", 0, 0, 0);
        return SipPrivate::MESSAGE_FAILED;
    }

    pCtState = DYNAMIC_CAST(SipClientTransactionState*, pTxnContextData->GetTxnState());

    if (pCtState.IsNull())
    {
        if (objUserData.GetDeleteFlag() == SIP_TRUE)
        {
            SipStack::DestroyTxnContext(pTxnContext);
        }

        SipStack::FreeTxnKey(pTxnKey);

        // fatal error
        IMS_TRACE_E(
                0, "The transaction context data is missing the transaction state info.", 0, 0, 0);
        return SipPrivate::MESSAGE_FAILED;
    }

    // FORKED_RESPONSE :: Check & look up the appropriate client transaction ...
    // FORKED_RESPONSE_TO_SUPPORT_EARLY_DIALOG_TERMINATION
    RcPtr<SipForkedTransactionManager> pTmpFtm;

    if (!pCtState->m_pForkedTxnMngr.IsNull())
    {
        pTmpFtm = pCtState->m_pForkedTxnMngr;
        IMS_TRACE_D("FTM :: Association with this transaction", 0, 0, 0);
    }
    else if (!pCtState->m_pPersistentForkedTxnMngr.IsNull())
    {
        pTmpFtm = pCtState->m_pPersistentForkedTxnMngr;
        IMS_TRACE_D("FTM :: No association with this transaction, but choose it", 0, 0, 0);
    }

    if (objMethod.Equals(SipMethod::INVITE) && !pTmpFtm.IsNull())
    {
        SipClientTransactionState* pTmpCtState = pTmpFtm->Lookup(pSipMsg);

        if ((pTmpCtState != IMS_NULL) && (pTmpCtState != pCtState.Get()))
        {
            IMS_TRACE_D("FTM :: The response is received via the other transaction", 0, 0, 0);

            pCtState = pTmpCtState;
        }
    }

    // Update the transaction key if it is different
    if ((pTxnKey != IMS_NULL) && (pTxnKey != pCtState->m_pTxnKey))
    {
        SipStack::FreeTxnKey(pCtState->m_pTxnKey);
        pCtState->m_pTxnKey = pTxnKey;
    }
    else
    {
        SipStack::FreeTxnKey(pTxnKey);
    }

    // Release transaction context if transaction is terminated
    if (objUserData.GetDeleteFlag() == SIP_TRUE)
    {
        SipStack::DestroyTxnContext(pTxnContext);
    }

    // SIP_MESSAGE_TRACKER
    SipFactoryProxy* pFactoryProxy = SipFactoryProxy::GetInstance();

    if (pFactoryProxy->IsMessageTrackerEnabled(pCtState->GetSlotId()))
    {
        SipMessageTracker* pMessageTracker =
                pFactoryProxy->GetMessageTracker(pCtState->GetSlotId());

        pMessageTracker->NotifyMessageReceived(objMethod, SipStack::GetStatusCode(pSipMsg),
                AString(SipStack::TxnKey_GetCallId(pCtState->m_pTxnKey)));
    }

    return SipPrivate::MESSAGE_VALID;
}

PRIVATE
IMS_BOOL SipClientTransactionState::CorrectRouteHeader(IN_OUT ::SipMessage*& pSipMsg)
{
    // Get the topmost Route entry
    SipAddrSpec* pAddrSpec = SipStack::GetAddrSpec(pSipMsg, ISipHeader::ROUTE);

    if (pAddrSpec == IMS_NULL)
    {
        if (SipStack::IsLastErrorNoExist())
        {
            m_nRoutingType = TARGET_NO_ROUTE;
            return IMS_TRUE;
        }

        return IMS_FALSE;
    }

    // The top Route entry is a loose router : no changes
    //    --> Request-URI is already set by the application or from To header
    const AString LR(Sip::STR_LR);

    if (SipStack::HasParameter(pAddrSpec, LR))
    {
        AString strLrValue = SipStack::GetParameter(pAddrSpec, LR);

        if ((strLrValue.GetLength() == 0) || strLrValue.EqualsIgnoreCase("true"))
        {
            m_nRoutingType = TARGET_LR;

            SipStack::FreeAddrSpec(pAddrSpec);
            return IMS_TRUE;
        }
    }

    m_nRoutingType = TARGET_SR;

    // Here, the top Route entry is a strict router:
    //    --> This addr-spec SHOULD be set in the Request-Line
    SipAddrSpec* pReqLineAddrSpec = SipStack::GetRequestUri(pSipMsg);
    SipHeaderBase* pRouteHeader = SipStack::CreateHeader(ISipHeader::ROUTE, pReqLineAddrSpec);
    SipStack::FreeAddrSpec(pReqLineAddrSpec);

    if (!SipStack::AppendHeader(pRouteHeader, pSipMsg))
    {
        SipStack::FreeHeaderEx(pRouteHeader);
        SipStack::FreeAddrSpec(pAddrSpec);
        return IMS_FALSE;
    }

    SipStack::FreeHeaderEx(pRouteHeader);

    // Set the Request-URI from the addr-spec of the top Route entry
    if (!SipStack::SetRequestUri(pAddrSpec, pSipMsg))
    {
        SipStack::FreeAddrSpec(pAddrSpec);
        return IMS_FALSE;
    }

    SipStack::FreeAddrSpec(pAddrSpec);

    return IMS_TRUE;
}

PRIVATE
void SipClientTransactionState::CheckNSendAck()
{
#if defined(__SIP_ACK_TRANSMISSION_REQUIRED_FOR_NON_2XX__)
    // Check if non-2xx response to INVITE, then the client transaction needs to send ACK request.
    IMS_SINT32 nStatusCode = SipStack::GetStatusCode(m_pSipMsg);

    if (nStatusCode >= SipStatusCode::SC_300)
    {
        ::SipMessage* pAckSipMsg = SipStack::CreateMessage(SipStack::SIP_MESSAGE_REQUEST);

        if (pAckSipMsg == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating an ACK request message failed", 0, 0, 0);
            return;
        }

        // Initializes ACK request message
        if (!InitAck(pAckSipMsg, m_pSipMsg))
        {
            IMS_TRACE_E(0, "Initializing ACK request failed", 0, 0, 0);
            SipStack::FreeMessage(pAckSipMsg);
            return;
        }

        SipMethod objMethod(SipMethod::ACK);
        SipMessageInfo objMsgInfo(
                GetSlotId(), objMethod, pAckSipMsg, SipMessageInfo::DIRECTION_OUTGOING);

        // Update the Contact information
        m_pDialogEx->UpdateDialogDetails(objMsgInfo);

        // Store the last request message
        // SipStack::FreeMessage(m_pLastSipMsg);
        // m_pLastSipMsg = SipStack::CloneMessage(pAckSipMsg);

        IMS_TRACE_I("___ SENDING ACK REQUEST .....", 0, 0, 0);
        SipTransactionState::Send(pAckSipMsg, IMS_NULL);

        SipStack::FreeMessage(pAckSipMsg);
    }
#endif
}

PRIVATE VIRTUAL SipTransactionState* SipClientTransactionState::Clone()
{
    SipClientTransactionState* pTxnState = new SipClientTransactionState(GetSlotId());

    if (pTxnState == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!pTxnState->InitTxnDetails(this))
    {
        delete pTxnState;
        return IMS_NULL;
    }

    return pTxnState;
}

PRIVATE
IMS_BOOL SipClientTransactionState::HandleForkedResponse(IN const SipMessageInfo& objMsgInfo)
{
    if (m_piCtsListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "No listener (%s)",
                SipDebug::GetCharA1(m_pDialogEx->GetDialogState()->GetCallId().GetStr(), 8, '@'), 0,
                0);
        return IMS_FALSE;
    }

    RcPtr<SipClientTransactionState> pForkedCtState = new SipClientTransactionState(GetSlotId());

    if (pForkedCtState.IsNull())
    {
        return IMS_FALSE;
    }

    // Add the forked transaction to the forked transaction manager...
    pForkedCtState->m_pForkedTxnMngr = m_pForkedTxnMngr;

    if (!pForkedCtState->m_pForkedTxnMngr.IsNull())
    {
        pForkedCtState->m_pForkedTxnMngr->Add(pForkedCtState.Get());
    }

    if (!pForkedCtState->InitTxnDetails(this))
    {
        IMS_TRACE_E(0, "Initializing SIP client transaction state failed", 0, 0, 0);
        return IMS_FALSE;
    }

    RcPtr<SipDialogState> pDState = new SipDialogState();

    if (pDState.IsNull())
    {
        return IMS_FALSE;
    }

    // Initialize the dialog details from the previous request message
    if (!pDState->InitDialogDetails(GetLastMessage()))
    {
        IMS_TRACE_E(0, "Initializing a dialog details from the previous request message", 0, 0, 0);
        return IMS_FALSE;
    }

    // Initialize the dialog details for the forked message
    if (!pDState->InitDialogDetails(
                SipDialogState::DIALOG_FORKED_RESPONSE, m_pDialogEx->GetDialogState()))
    {
        IMS_TRACE_E(0, "Initializing a dialog details from the forked response message", 0, 0, 0);
        return IMS_FALSE;
    }

    pForkedCtState->m_pDialogEx = SipDialogEx::CreateDialog(pDState.Get(), objMsgInfo);

    if (pForkedCtState->m_pDialogEx.IsNull())
    {
        return IMS_FALSE;
    }

    SipMessageInfo objReqMsgInfo(GetSlotId(), objMsgInfo.GetMethod(), GetLastMessage(),
            SipMessageInfo::DIRECTION_OUTGOING);

    if (pForkedCtState->m_pDialogEx->UpdateDialogDetails(objReqMsgInfo) !=
            SipPrivate::MESSAGE_VALID)
    {
        IMS_TRACE_E(0, "Updating a dialog details from the previous request message", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pForkedCtState->m_pDialogEx->UpdateDialogDetails(objMsgInfo) != SipPrivate::MESSAGE_VALID)
    {
        IMS_TRACE_E(0, "Updating a dialog details from the forked response message", 0, 0, 0);
        return IMS_FALSE;
    }

    pForkedCtState->UpdateMessage(objMsgInfo.GetMessage());

    // Initialize the transport info. when the response message is received
    pForkedCtState->m_pTransport->InitTransportOnMessageReceived(objMsgInfo.GetMessage());

    // Validate the transport information
    IMS_SINT32 nValidity = pForkedCtState->CheckMessageValidity();

    if (nValidity != SipPrivate::MESSAGE_VALID)
    {
        IMS_TRACE_E(nValidity, "Checking the message validity failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Check if non-2xx response to INVITE, then the client transaction needs to send ACK request.
    if (objMsgInfo.GetMethod().Equals(SipMethod::INVITE))
    {
        pForkedCtState->CheckNSendAck();
    }

    // FIX_NO_ACK_RETRANSMISSION
    if (!pForkedCtState->m_pForkedTxnMngr.IsNull())
    {
        IMS_SINT32 nStatusCode = SipStack::GetStatusCode(objMsgInfo.GetMessage());

        if (SipStatusCode::IsFinal(nStatusCode))
        {
            pForkedCtState->m_pForkedTxnMngr->SetTransactionCompleted(nStatusCode);
        }
    }

    m_piCtsListener->ClientTransactionState_ForkedResponseReceived(pForkedCtState.Get());

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SipClientTransactionState::InitAck(
        IN_OUT ::SipMessage*& pAckSipMsg, IN ::SipMessage* pRespSipMsg)
{
    SipHeaderBase* pPrevSipHdr;
    SipHeaderBase* pSipHdr;
    SipMethod objMethodAck(SipMethod::ACK);

    // Sets SIP method name
    SipStack::SetMethod(objMethodAck, pAckSipMsg);

    // Sets a Request-URI field
    SipAddrSpec* pAddrSpec = SipStack::GetRequestUri(m_pLastSipMsg);
    (void)SipStack::SetRequestUri(pAddrSpec, pAckSipMsg);
    SipStack::FreeAddrSpec(pAddrSpec);

    AString strHBody;

    // Sets CSeq header field
    strHBody.SetNumber(m_nCSeqNumber);
    strHBody += TextParser::CHAR_SP;
    strHBody += SipMethod::NAME[SipMethod::ACK];
    pSipHdr = SipStack::DecodeHeader(ISipHeader::CSEQ, strHBody);

    if (!SipStack::SetHeader(pSipHdr, pAckSipMsg))
    {
        SipStack::FreeHeaderEx(pSipHdr);
        return IMS_FALSE;
    }

    SipStack::FreeHeaderEx(pSipHdr);

    // Sets Max-Forwards header field
    strHBody.SetNumber(SipPrivate::MAX_HOP);
    pSipHdr = SipStack::DecodeHeader(ISipHeader::MAX_FORWARDS, strHBody);

    if (!SipStack::SetHeader(pSipHdr, pAckSipMsg))
    {
        SipStack::FreeHeaderEx(pSipHdr);
        return IMS_FALSE;
    }

    SipStack::FreeHeaderEx(pSipHdr);

    // Sets From header field
    pPrevSipHdr = SipStack::GetHeader(m_pLastSipMsg, ISipHeader::FROM);

    pSipHdr = SipStack::CloneHeader(pPrevSipHdr);
    SipStack::FreeHeaderEx(pPrevSipHdr);

    if (pSipHdr != IMS_NULL)
    {
        if (!SipStack::SetHeader(pSipHdr, pAckSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        SipStack::FreeHeaderEx(pSipHdr);
    }

    // Sets To header field
    pPrevSipHdr = SipStack::GetHeader(m_pLastSipMsg, ISipHeader::TO);

    pSipHdr = SipStack::CloneHeader(pPrevSipHdr);
    SipStack::FreeHeaderEx(pPrevSipHdr);

    if (pSipHdr != IMS_NULL)
    {
        if (!SipStack::SetHeader(pSipHdr, pAckSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        // Update To-Tag from the last response message
        pPrevSipHdr = SipStack::GetHeader(pRespSipMsg, ISipHeader::TO);

        AString strToTag = SipStack::GetParameter(pPrevSipHdr, Sip::STR_TAG);

        if (!strToTag.IsNULL())
        {
            (void)SipStack::SetParameter(pSipHdr, Sip::STR_TAG, strToTag);
        }

        SipStack::FreeHeaderEx(pPrevSipHdr);
        SipStack::FreeHeaderEx(pSipHdr);
    }

    // Sets Call-ID header field
    pPrevSipHdr = SipStack::GetHeader(m_pLastSipMsg, ISipHeader::CALL_ID);

    pSipHdr = SipStack::CloneHeader(pPrevSipHdr);
    SipStack::FreeHeaderEx(pPrevSipHdr);

    if (pSipHdr != IMS_NULL)
    {
        if (!SipStack::SetHeader(pSipHdr, pAckSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        SipStack::FreeHeaderEx(pSipHdr);
    }

    // Sets Via header field
    pPrevSipHdr = SipStack::GetHeader(m_pLastSipMsg, ISipHeader::VIA);

    pSipHdr = SipStack::CloneHeader(pPrevSipHdr);
    SipStack::FreeHeaderEx(pPrevSipHdr);

    if (pSipHdr != IMS_NULL)
    {
        if (!SipStack::SetHeader(pSipHdr, pAckSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        SipStack::FreeHeaderEx(pSipHdr);
    }

    // Sets Route header field
    IMS_SINT32 nHCount = SipStack::GetHeaderCount(m_pLastSipMsg, ISipHeader::ROUTE);

    for (IMS_SINT32 i = 0; i < nHCount; i++)
    {
        pPrevSipHdr = SipStack::GetHeader(m_pLastSipMsg, ISipHeader::ROUTE, i);

        if (SipStack::IsValidHeader(pPrevSipHdr))
        {
            if (!SipStack::AppendHeader(pPrevSipHdr, pAckSipMsg))
            {
                SipStack::FreeHeaderEx(pPrevSipHdr);
                return IMS_FALSE;
            }
        }

        SipStack::FreeHeaderEx(pPrevSipHdr);
    }

    // Sets User-Agent header if it is present in the previous request
    pPrevSipHdr = SipStack::GetUnknownHeader(m_pLastSipMsg, SipHeaderName::USER_AGENT);

    if (SipStack::IsValidHeader(pPrevSipHdr))
    {
        pSipHdr = SipStack::CloneHeader(pPrevSipHdr);

        if (pSipHdr != IMS_NULL)
        {
            // Even if setting User-Agent is failed, it will proceed to send ACK request.
            SipStack::SetHeader(pSipHdr, pAckSipMsg);
            SipStack::FreeHeaderEx(pSipHdr);
        }
    }

    SipStack::FreeHeaderEx(pPrevSipHdr);

    // P-Access-Network-Info header if required
    if (SipFeatures::IsPaniHeaderForAckRequired(GetSlotId()))
    {
        SetPaniHeader(objMethodAck, pAckSipMsg);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SipClientTransactionState::SetDialogRelatedHeaders(IN const SipMethod& objMethod)
{
    if (m_pDialogEx->GetState() == SipDState::STATE_INIT)
    {
        // Check if the Call-ID field is present or not
        if (!SipStack::IsHeaderPresent(m_pSipMsg, ISipHeader::CALL_ID))
        {
            // Add Call-ID in here
            const IpAddress& objAddress = m_pTransport->GetIpAddress();
            AString strCallId = SipUtils::GenerateCallId(objAddress.ToString());
            SipHeaderBase* pSipHdr = SipStack::DecodeHeader(ISipHeader::CALL_ID, strCallId);

            if (pSipHdr == IMS_NULL)
            {
                return IMS_FALSE;
            }

            if (!SipStack::SetHeader(pSipHdr, m_pSipMsg))
            {
                SipStack::FreeHeaderEx(pSipHdr);
                return IMS_FALSE;
            }

            SipStack::FreeHeaderEx(pSipHdr);

            // HEADER_REQ_SESSION-ID
            if (SipFeatures::IsHeaderSessionIdRequired(GetSlotId()) &&
                    objMethod.Equals(SipMethod::INVITE))
            {
                const AString SESSION_ID(SipHeaderName::SESSION_ID);
                SipHeaderBase* pSessionId = SipStack::GetUnknownHeader(m_pSipMsg, SESSION_ID);

                if (pSessionId == IMS_NULL)
                {
                    AString strSessionId = SipUtils::GenerateSessionId(GetSlotId(), strCallId);

                    if (strSessionId.GetLength() > 0)
                    {
                        pSessionId = SipStack::DecodeHeader(
                                ISipHeader::UNKNOWN, SESSION_ID, strSessionId);

                        if (pSessionId != IMS_NULL)
                        {
                            (void)SipStack::SetUnknownHeader(pSessionId, SESSION_ID, m_pSipMsg);
                            SipStack::FreeHeaderEx(pSessionId);
                        }
                    }
                }
                else
                {
                    // Enabler or Core Engine sets the Session-ID header, so do not modify it.
                    SipStack::FreeHeaderEx(pSessionId);
                }
            }
        }

        // Set From-Tag
        if (!objMethod.Equals(SipMethod::CANCEL) && !objMethod.Equals(SipMethod::ACK))
        {
            SipHeaderBase* pSipHdr = SipStack::GetHeader(m_pSipMsg, ISipHeader::FROM);

            if (!SipStack::HasParameter(pSipHdr, Sip::STR_TAG))
            {
                AString strTagVal = SipUtils::GenerateTag(
                        SipConfigProxy::GetTagPrefix(GetSlotId(), GetSipProfile()));

                if (!SipStack::SetParameter(pSipHdr, Sip::STR_TAG, strTagVal))
                {
                    SipStack::FreeHeaderEx(pSipHdr);
                    return IMS_FALSE;
                }
            }

            SipStack::FreeHeaderEx(pSipHdr);
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SipClientTransactionState::SetMandatoryHeaders(IN const SipMethod& objMethod)
{
    if (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::ACK))
    {
        m_nClass = CLASS_INVITE;
    }
    else if (objMethod.Equals(SipMethod::PRACK))
    {
        m_nClass = CLASS_OVERLAP;
    }
    else
    {
        m_nClass = CLASS_REGULAR;
    }

    AString strHBody;

    // Sets CSeq header field
    strHBody.SetNumber(m_nCSeqNumber);
    strHBody.Append(TextParser::CHAR_SP);
    strHBody.Append(objMethod.ToString());

    SipHeaderBase* pSipHdr = SipStack::DecodeHeader(ISipHeader::CSEQ, strHBody);

    if (!SipStack::SetHeader(pSipHdr, m_pSipMsg))
    {
        SipStack::FreeHeaderEx(pSipHdr);
        return IMS_FALSE;
    }

    SipStack::FreeHeaderEx(pSipHdr);

    // Sets Max-Forwards header field
    strHBody.SetNumber(SipPrivate::MAX_HOP);
    pSipHdr = SipStack::DecodeHeader(ISipHeader::MAX_FORWARDS, strHBody);

    if (!SipStack::SetHeader(pSipHdr, m_pSipMsg))
    {
        SipStack::FreeHeaderEx(pSipHdr);
        return IMS_FALSE;
    }

    SipStack::FreeHeaderEx(pSipHdr);

    return IMS_TRUE;
}

PRIVATE
void SipClientTransactionState::SetPaniHeader(
        IN const SipMethod& objMethod, IN_OUT ::SipMessage*& pSipMsg)
{
    AString strPani;

    if (PAccessNetworkInfoHeader::FormHeader(
                GetSlotId(), m_pTransport->GetIpAddress(), objMethod, GetSipProfile(), strPani))
    {
        if (strPani.GetLength() > 0)
        {
            SipHeaderBase* pSipHdr = SipStack::DecodeHeader(
                    ISipHeader::P_ACCESS_NETWORK_INFO, AString::ConstNull(), strPani);

            if (pSipHdr != IMS_NULL)
            {
                SipStack::SetHeader(pSipHdr, pSipMsg);
                SipStack::FreeHeaderEx(pSipHdr);
            }
        }
    }
}

PRIVATE
IMS_BOOL SipClientTransactionState::UpdateTxnDetails(IN const SipMethod& objMethod)
{
    // Update the transaction information for outgoing request
    SipMessageInfo objMsgInfo(
            GetSlotId(), objMethod, m_pSipMsg, SipMessageInfo::DIRECTION_OUTGOING);

    if (m_pDialogEx->GetState() == SipDState::STATE_INIT)
    {
        if (!m_pDialogEx->GetDialogState()->InitDialogDetails(m_pSipMsg))
        {
            return IMS_FALSE;
        }

        // Update the sequence number in the CSeq header if changed.
        IMS_UINT32 nSeqNum = SipStack::GetCSeqNumber(m_pSipMsg);

        if (nSeqNum != SipPrivate::INVALID_SEQ_NUM)
        {
            m_nCSeqNumber = nSeqNum;
        }
    }

    // If the request can create a dialog usage, then update the state if changed
    if (SipDialogBase::IsDialogCreatable(objMethod))
    {
        SipDialogState* pDState = m_pDialogEx->GetDialogState();
        SipDialogEx* pTmpDialogEx = pDState->GetDialogUsage(objMsgInfo);

        // Not present; So, use a new dialog
        if (pTmpDialogEx == IMS_NULL)
        {
            m_pDialogEx->InitDialogWithDelay(objMsgInfo);
        }
        // Use an existing dialog
        else
        {
            IMS_TRACE_D("_____ UPDATE DIALOG  (%s:%s) _____", objMethod.ToString().GetStr(),
                    SipDebug::GetCharA1(pDState->GetCallId().GetStr(), 8, '@'), 0);

            m_pDialogEx = pTmpDialogEx;
        }
    }

    // Update the Contact information
    m_pDialogEx->UpdateDialogDetails(objMsgInfo);

    // Store the last request message
    if (!objMethod.Equals(SipMethod::ACK))
    {
        SipStack::FreeMessage(m_pLastSipMsg);
        m_pLastSipMsg = SipStack::CloneMessage(m_pSipMsg);
    }

    return IMS_TRUE;
}
