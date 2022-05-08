/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ISipHeader.h"
#include "SIPPrivate.h"
#include "SipDebug.h"
#include "SipHeaderName.h"
#include "SipFeatures.h"
#include "SipConfigProxy.h"
#include "SIPStackHeaders.h"
#include "SIPStackState.h"
#include "SIPTxnContextData.h"
#include "SIPUtil.h"
#include "SIPDialogEx.h"
#include "ISIPClientTransactionStateListener.h"
#include "SIPAckPackage.h"
#include "SIPFactoryProxy.h"
// SIP_MESSAGE_TRACKER
#include "SIPMessageTracker.h"
#include "SIPClientTransport.h"
#include "PAccessNetworkInfoHeader.h"
#include "SIPClientTransactionState.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPClientTransactionState::SIPClientTransactionState(IN IMS_SINT32 nSlotId) :
        SIPTransactionState(),
        nRoutingType(TARGET_LR),
        pstImplicitRoute(IMS_NULL),
        piListener(IMS_NULL)
{
    nType = TYPE_CLIENT;

    pTransport = new SIPClientTransport(nSlotId);
}

PUBLIC VIRTUAL SIPClientTransactionState::~SIPClientTransactionState()
{
    SIPStack::FreeAddrSpec(pstImplicitRoute);

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("Destructor :: SIPClientTransactionState", 0, 0, 0);
#endif
}

/*

Remarks

*/
PUBLIC VIRTUAL void SIPClientTransactionState::Abort()
{
    //---------------------------------------------------------------------------------------------

    // FORKED_RESPONSE
    if (RemoveForkedTransaction() > 0)
        return;

    SIPTransactionState::Abort();
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 SIPClientTransactionState::CheckMessageValidity()
{
    IMS_SINT32 nValidity;

    //---------------------------------------------------------------------------------------------

    // Check the validity of transport information from the top Via header
    nValidity = pTransport->ValidateViaHeader(pstMessage);

    if (nValidity != SIPPrivate::MESSAGE_VALID)
        return nValidity;

    return SIPPrivate::MESSAGE_VALID;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPClientTransactionState::FormMessage()
{
    // Update the transport information
    if (!UpdateTransportDetails())
    {
        SIPPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FALSE;
    }

    SipMethod objMethod = SIPStack::GetMethod(pstMessage);

    if (!SetDialogRelatedHeaders(objMethod))
    {
        IMS_TRACE_E(0, "Setting a dialog-related headers failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!pTransport->FormViaHeader(pstMessage, GetSIPProfile()))
    {
        return IMS_FALSE;
    }

    SetFlowControlOption(objMethod);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPClientTransactionState::FormMessageForResubmissionRequest()
{
    //---------------------------------------------------------------------------------------------

    // Update the transport information
    // 4 Checks if the sec-agree is supported or not
    if (!UpdateTransportDetails())
    {
        SIPPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FALSE;
    }

    // Remove the previous Via header from the message
    SIPStack::RemoveHeader(ISipHeader::VIA, pstMessage);

    // Remove the from-tag
    IMS_BOOL bFromTagRemovalRequired = IMS_TRUE;

    if (!pDialogEx.IsNull())
    {
        if (pDialogEx->GetState() == SIPDState::STATE_CONFIRMED)
        {
            // Do not remove "from-tag" from From header within SIP dialog
            bFromTagRemovalRequired = IMS_FALSE;
        }
    }

    if (bFromTagRemovalRequired)
    {
        SipHeaderBase* pstFromHeader = SIPStack::GetHeader(pstMessage, ISipHeader::FROM);
        SIPStack::RemoveParameter(Sip::STR_TAG, pstFromHeader);
        SIPStack::FreeHeaderEx(pstFromHeader);
    }

    SipMethod objMethod = SIPStack::GetMethod(pstMessage);
    AString strHBody;

    // Sets CSeq header field
    strHBody.SetNumber(++nCSeqNumber);
    strHBody.Append(TextParser::CHAR_SP);
    strHBody.Append(objMethod.ToString());

    SipHeaderBase* pstHeader = SIPStack::DecodeHeader(ISipHeader::CSEQ, strHBody);

    if (!SIPStack::SetHeader(pstHeader, pstMessage))
    {
        SIPStack::FreeHeaderEx(pstHeader);
        return IMS_FALSE;
    }

    SIPStack::FreeHeaderEx(pstHeader);

    if (!SetDialogRelatedHeaders(objMethod))
    {
        IMS_TRACE_E(0, "Setting a dialog-related headers failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!pTransport->FormViaHeader(pstMessage, GetSIPProfile()))
    {
        return IMS_FALSE;
    }

    SetFlowControlOption(objMethod);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPClientTransactionState::InitTxnDetails(
        IN CONST SIPTransactionState* pTState)
{
    const SIPClientTransactionState* pCTState =
            DYNAMIC_CAST(const SIPClientTransactionState*, pTState);

    //---------------------------------------------------------------------------------------------

    if (pCTState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!SIPTransactionState::InitTxnDetails(pCTState))
    {
        return IMS_FALSE;
    }

    nRoutingType = pCTState->nRoutingType;

    // IMPLICIT_ROUTE
    SIPStack::FreeAddrSpec(pstImplicitRoute);

    if (pCTState->pstImplicitRoute != IMS_NULL)
    {
        pstImplicitRoute = pCTState->pstImplicitRoute;
        SIPStack::AddReference(pstImplicitRoute);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPClientTransactionState::Send(IN SipTimerValues* pTV /* = IMS_NULL */)
{
    SipMethod objMethod = SIPStack::GetMethod(pstMessage);

    //---------------------------------------------------------------------------------------------

    if (!UpdateTxnDetails(objMethod))
    {
        IMS_TRACE_E(0, "Updating transaction details failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return SIPTransactionState::Send(pTV);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPClientTransactionState::UpdateTransportDetails()
{
    IMS_BOOL bRoutingLR = (nRoutingType == TARGET_LR) ? IMS_TRUE : IMS_FALSE;
    IMS_BOOL bImplicitRouteRequired = IMS_FALSE;
    IMS_BOOL bCheckImplicitRouteUsage = IMS_FALSE;
    IMS_BOOL bIgnoreLR = IMS_FALSE;

    //---------------------------------------------------------------------------------------------

    if (pstImplicitRoute != IMS_NULL)
    {
        if (nRoutingType == TARGET_NO_ROUTE)
        {
            bCheckImplicitRouteUsage = IMS_TRUE;
        }
        else if (!pSIPProfile.IsNull() &&
                pSIPProfile->IsConfigurationSet(
                        SipProfile::CONFIG_IGNORE_RR_ON_IMPLICIT_ROUTE_REQUIRED))
        {
            bCheckImplicitRouteUsage = IMS_TRUE;
            bIgnoreLR = IMS_TRUE;
        }
    }

    // IMPLICIT_ROUTE
    if (bCheckImplicitRouteUsage)
    {
        IMS_SINT32 nDialogState = pDialogEx->GetState();

        if (nDialogState == SIPDState::STATE_INIT)
        {
            bImplicitRouteRequired = IMS_TRUE;
        }
        // IMPLICIT_ROUTING_FOR_MID_DIALOG
        else if (pDialogEx->IsInviteUsage() && (nDialogState == SIPDState::STATE_CONFIRMED))
        {
            bImplicitRouteRequired = IMS_TRUE;
        }
        // For routing all mid-dialog requests to the first outbound proxy
        else if ((nDialogState == SIPDState::STATE_EARLY) ||
                (nDialogState == SIPDState::STATE_CONFIRMED))
        {
            bImplicitRouteRequired = IMS_TRUE;
        }
        // BYE_REQUEST_ON_DIALOG_TERMINATED
        else if (pDialogEx->IsInviteUsage() && (nDialogState == SIPDState::STATE_TERMINATED))
        {
            SipMethod objMethod = SIPStack::GetMethod(pstMessage);

            if (objMethod.Equals(SipMethod::BYE))
            {
                bImplicitRouteRequired = IMS_TRUE;
            }
        }

        if (bImplicitRouteRequired && bIgnoreLR)
        {
            bRoutingLR = IMS_FALSE;
            IMS_TRACE_I("ImplicitRoute :: Ignore route-set", 0, 0, 0);
        }
    }

    if (!pTransport->UpdateDestinationInfo(pstMessage, bRoutingLR,
                (bImplicitRouteRequired == IMS_TRUE) ? pstImplicitRoute : IMS_NULL))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPClientTransactionState::AdjustTransportProtocolAsUDP()
{
    if (pTransport == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SIPStack::RemoveHeader(ISipHeader::VIA, pstMessage);

    pTransport->SetProtocol(SIPTransportAddress::PROTOCOL_UDP, SIPTransport::TA_NEAR);
    pTransport->SetProtocol(SIPTransportAddress::PROTOCOL_UDP, SIPTransport::TA_FAR);
    pTransport->SetExplicitTargetProtocol(IMS_TRUE);

    if (!pTransport->FormViaHeader(pstMessage, GetSIPProfile()))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
SipMessage* SIPClientTransactionState::CreateAckRequest(IN SipMessage* pstRespMessage)
{
    IMS_TRACE_D("CreateAckRequest", 0, 0, 0);

    SipMessage* pstAckMessage = SIPStack::CreateMessage(SIPStack::SIP_MESSAGE_REQUEST);

    if (pstAckMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating an ACK request message failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (pstRespMessage == IMS_NULL)
    {
        pstRespMessage = pstMessage;
    }

    // Initializes ACK request message
    if (!InitAck(pstAckMessage, pstRespMessage))
    {
        IMS_TRACE_E(0, "Initializing ACK request failed", 0, 0, 0);
        SIPStack::FreeMessage(pstAckMessage);
        return IMS_NULL;
    }

    return pstAckMessage;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPClientTransactionState::InitCancel(IN CONST SIPClientTransactionState* pInviteTState)
{
    SipHeaderBase* pstGetHdr;
    SipHeaderBase* pstHeader;
    SipMethod objMethod(SipMethod::CANCEL);

    //---------------------------------------------------------------------------------------------

    nClass = CLASS_REGULAR;

    SetSIPProfile(pInviteTState->GetSIPProfile());

    // Sets a SIP method name
    SIPStack::SetMethod(objMethod, pstMessage);

    // Sets a Request-URI field
    SipAddrSpec* pstAddrSpec = SIPStack::GetRequestUri(pInviteTState->pstLastMessage);

    (void)SIPStack::SetRequestUri(pstAddrSpec, pstMessage);
    SIPStack::FreeAddrSpec(pstAddrSpec);

    AString strHBody;

    // Sets CSeq header field
    strHBody.SetNumber(pInviteTState->nCSeqNumber);
    strHBody.Append(TextParser::CHAR_SP);
    strHBody.Append(SipMethod::NAME[SipMethod::CANCEL]);

    pstHeader = SIPStack::DecodeHeader(ISipHeader::CSEQ, strHBody);

    if (!SIPStack::SetHeader(pstHeader, pstMessage))
    {
        SIPStack::FreeHeaderEx(pstHeader);
        return IMS_FALSE;
    }

    SIPStack::FreeHeaderEx(pstHeader);

    // Sets Max-Forwards header field
    strHBody.SetNumber(SIPPrivate::MAX_HOP);
    pstHeader = SIPStack::DecodeHeader(ISipHeader::MAX_FORWARDS, strHBody);

    if (!SIPStack::SetHeader(pstHeader, pstMessage))
    {
        SIPStack::FreeHeaderEx(pstHeader);
        return IMS_FALSE;
    }

    SIPStack::FreeHeaderEx(pstHeader);

    // Sets From header field
    pstGetHdr = SIPStack::GetHeader(pInviteTState->pstLastMessage, ISipHeader::FROM);

    pstHeader = SIPStack::CloneHeader(pstGetHdr);
    SIPStack::FreeHeaderEx(pstGetHdr);

    if (pstHeader != IMS_NULL)
    {
        if (!SIPStack::SetHeader(pstHeader, pstMessage))
        {
            SIPStack::FreeHeaderEx(pstHeader);
            return IMS_FALSE;
        }

        SIPStack::FreeHeaderEx(pstHeader);
    }

    // Sets To header field
    pstGetHdr = SIPStack::GetHeader(pInviteTState->pstLastMessage, ISipHeader::TO);

    pstHeader = SIPStack::CloneHeader(pstGetHdr);
    SIPStack::FreeHeaderEx(pstGetHdr);

    if (pstHeader != IMS_NULL)
    {
        // Remove To-Tag if the CANCEL transaction is for initial INVITE request
        IMS_BOOL bInitialINVITE = IMS_TRUE;

        if (!pInviteTState->pDialogEx.IsNull())
        {
            SIPDialogState* pDState = pInviteTState->pDialogEx->GetDialogState();

            if (pDState != IMS_NULL)
            {
                if (pDState->GetState() == SIPDState::STATE_CONFIRMED)
                {
                    bInitialINVITE = IMS_FALSE;
                }
            }
        }

        if (bInitialINVITE)
        {
            (void)SIPStack::RemoveParameter(AString(Sip::STR_TAG), pstHeader);
        }

        if (!SIPStack::SetHeader(pstHeader, pstMessage))
        {
            SIPStack::FreeHeaderEx(pstHeader);
            return IMS_FALSE;
        }

        SIPStack::FreeHeaderEx(pstHeader);
    }

    // Sets Call-ID header field
    pstGetHdr = SIPStack::GetHeader(pInviteTState->pstLastMessage, ISipHeader::CALL_ID);

    pstHeader = SIPStack::CloneHeader(pstGetHdr);
    SIPStack::FreeHeaderEx(pstGetHdr);

    if (pstHeader != IMS_NULL)
    {
        if (!SIPStack::SetHeader(pstHeader, pstMessage))
        {
            SIPStack::FreeHeaderEx(pstHeader);
            return IMS_FALSE;
        }

        SIPStack::FreeHeaderEx(pstHeader);
    }

    // Sets Via header field
    pstGetHdr = SIPStack::GetHeader(pInviteTState->pstLastMessage, ISipHeader::VIA);

    pstHeader = SIPStack::CloneHeader(pstGetHdr);
    SIPStack::FreeHeaderEx(pstGetHdr);

    if (pstHeader != IMS_NULL)
    {
        if (!SIPStack::SetHeader(pstHeader, pstMessage))
        {
            SIPStack::FreeHeaderEx(pstHeader);
            return IMS_FALSE;
        }

        SIPStack::FreeHeaderEx(pstHeader);
    }

    // Route
    IMS_SINT32 nHCount = SIPStack::GetHeaderCount(pInviteTState->pstLastMessage, ISipHeader::ROUTE);

    for (IMS_SINT32 i = 0; i < nHCount; i++)
    {
        pstGetHdr = SIPStack::GetHeader(pInviteTState->pstLastMessage, ISipHeader::ROUTE, i);

        if (SIPStack::IsValidHeader(pstGetHdr))
        {
            if (!SIPStack::AppendHeader(pstGetHdr, pstMessage))
            {
                SIPStack::FreeHeaderEx(pstGetHdr);
                return IMS_FALSE;
            }
        }

        SIPStack::FreeHeaderEx(pstGetHdr);
    }

    // IMPLICIT_ROUTE
    SIPStack::FreeAddrSpec(pstImplicitRoute);

    if (pInviteTState->pstImplicitRoute != IMS_NULL)
    {
        pstImplicitRoute = pInviteTState->pstImplicitRoute;
        SIPStack::AddReference(pstImplicitRoute);
    }

    // Update the transport information
    const SIPTransportAddress& objNearTA =
            pInviteTState->pTransport->GetAddress(SIPTransport::TA_NEAR);
    const SIPTransportAddress& objFarTA =
            pInviteTState->pTransport->GetAddress(SIPTransport::TA_FAR);

    // RFC5626_FLOW_CONTROL
    pTransport->SetTransportTuple(objNearTA.GetIPAddress(), objNearTA.GetPort(),
            pInviteTState->pTransport->GetPortC(), pInviteTState->pTransport->GetPortFlowControl(),
            pInviteTState->pTransport->GetTransportExt());

    // FIX_TRANSPORT_PROTOCOL
    pTransport->SetProtocol(objNearTA.GetProtocol(), SIPTransport::TA_NEAR);
    pTransport->SetProtocol(objFarTA.GetProtocol(), SIPTransport::TA_FAR);

    // Create a dialog
    pDialogEx = SIPDialogEx::CreateDialog(objMethod);

    if (pDialogEx.IsNull())
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPClientTransactionState::InitRequest(IN CONST SipMethod& objMethod)
{
    //---------------------------------------------------------------------------------------------

    // Set a mandatory headers in the INITIAL state
    //    - CSeq, Max-Forwards headers
    if (!SetMandatoryHeaders(objMethod))
    {
        return IMS_FALSE;
    }

    // Create a dialog usage according to the method
    pDialogEx = SIPDialogEx::CreateDialog(objMethod);

    if (pDialogEx.IsNull())
    {
        return IMS_FALSE;
    }

    // FORKED_RESPONSE
    if (objMethod.Equals(SipMethod::INVITE))
    {
        pForkedTxnMngr = new SIPForkedTransactionManager();

        if (pForkedTxnMngr.IsNull())
        {
            return IMS_FALSE;
        }

        pForkedTxnMngr->Add(this);

        // FORKED_RESPONSE_TO_SUPPORT_EARLY_DIALOG_TERMINATION
        // Grab the forked response manager when initiating the INVITE transaction
        pPersistentForkedTxnMngr = pForkedTxnMngr;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPClientTransactionState::InitRequest(
        IN CONST SipMethod& objMethod, IN SIPDialogEx* pDialogEx)
{
    SIPDialogState* pDState;

    //---------------------------------------------------------------------------------------------

    if (!objMethod.Equals(SipMethod::ACK))
    {
        this->pDialogEx = pDialogEx;
        pDState = this->pDialogEx->GetDialogState();

        nCSeqNumber = pDState->GetNextCSeqNumber();
    }
    else
    {
        pDState = this->pDialogEx->GetDialogState();

        // For ACK transaction, reset the transport protocol to a default transport protocol
        pTransport->SetProtocol(SIPTransportAddress::PROTOCOL_UDP, SIPTransport::TA_NEAR);
        pTransport->SetProtocol(SIPTransportAddress::PROTOCOL_UDP, SIPTransport::TA_FAR);
    }

    // FORKED_RESPONSE
    if (objMethod.Equals(SipMethod::INVITE))
    {
        pForkedTxnMngr = new SIPForkedTransactionManager();

        if (pForkedTxnMngr.IsNull())
        {
            return IMS_FALSE;
        }

        pForkedTxnMngr->Add(this);

        // FORKED_RESPONSE_TO_SUPPORT_EARLY_DIALOG_TERMINATION
        // Grab the forked response manager when initiating the INVITE transaction
        pPersistentForkedTxnMngr = pForkedTxnMngr;
    }

    // Set a mandatory headers in the INITIAL state
    //    - CSeq, Max-Forwards headers
    if (!SetMandatoryHeaders(objMethod))
    {
        return IMS_FALSE;
    }

    if (!pDState->InitRequest(objMethod, pstMessage))
    {
        return IMS_FALSE;
    }

    if (!CorrectRouteHeader(pstMessage))
    {
        return IMS_FALSE;
    }

    // P-Access-Network-Info header if required
    if (objMethod.Equals(SipMethod::ACK) && SipFeatures::IsPaniHeaderForAckRequired(GetSlotId()))
    {
        SetPANIHeader(objMethod, pstMessage);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPClientTransactionState::RemoveForkedTransaction()
{
    //---------------------------------------------------------------------------------------------

    if (!pForkedTxnMngr.IsNull())
    {
        pForkedTxnMngr->Remove(this);

        if (!pForkedTxnMngr->IsEmpty())
        {
            pForkedTxnMngr = IMS_NULL;
            return 1;
        }

        // FIX_NO_ACK_RETRANSMISSION
        if (pForkedTxnMngr->IsTransactionCompleted())
        {
            pForkedTxnMngr = IMS_NULL;
            return 1;
        }

        pForkedTxnMngr = IMS_NULL;
    }

    return 0;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPClientTransactionState::SendWithCredentials(IN SipTimerValues* pTV /* = IMS_NULL */)
{
    SipMethod objMethod = SIPStack::GetMethod(pstMessage);

    //---------------------------------------------------------------------------------------------

    if (!UpdateTxnDetails(objMethod))
    {
        IMS_TRACE_E(0, "Updating transaction details failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Updates the sequence number on the dialog state, EARLY / CONFIRMED
    if (!pDialogEx.IsNull())
    {
        if ((pDialogEx->GetState() == SIPDState::STATE_EARLY) ||
                (pDialogEx->GetState() == SIPDState::STATE_CONFIRMED))
        {
            pDialogEx->GetDialogState()->UpdateLocalCSeq(nCSeqNumber);
        }
        // BYE_REQUEST_ON_DIALOG_TERMINATED
        else if (objMethod.Equals(SipMethod::BYE) &&
                (pDialogEx->GetState() == SIPDState::STATE_TERMINATED))
        {
            pDialogEx->GetDialogState()->UpdateLocalCSeq(nCSeqNumber);
        }
    }

    return SIPTransactionState::Send(pTV);
}

/*

Remarks

*/
PUBLIC
void SIPClientTransactionState::SetExtensionTokenForViaBranch(IN CONST AString& strToken)
{
    SIPClientTransport* pClientTransport = DYNAMIC_CAST(SIPClientTransport*, pTransport);

    //---------------------------------------------------------------------------------------------

    if (pClientTransport == IMS_NULL)
    {
        return;
    }

    pClientTransport->SetExtensionTokenForViaBranch(strToken);
}

/*

Remarks
 IMPLICIT_ROUTE
*/
PUBLIC
void SIPClientTransactionState::SetImplicitRouteHeader(IN CONST AString& strRouteHeader)
{
    //---------------------------------------------------------------------------------------------

    SIPStack::FreeAddrSpec(pstImplicitRoute);
    pstImplicitRoute = IMS_NULL;

    if (strRouteHeader.GetLength() > 0)
    {
        pstImplicitRoute = SIPStack::DecodeAddrSpec(strRouteHeader);
    }
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPClientTransactionState::UpdateRouteDetails(IN CONST SipMethod& objMethod)
{
    //---------------------------------------------------------------------------------------------

    if (!CorrectRouteHeader(pstMessage))
    {
        return IMS_FALSE;
    }

    SipAddrSpec* pstAddrSpec = SIPStack::GetRequestUri(pstMessage);

    if (objMethod.Equals(SipMethod::REGISTER))
    {
        // Remove userinfo & password field if present
        SIPStack::RemoveUserAndPassword(pstAddrSpec);
    }

    SIPStack::FreeAddrSpec(pstAddrSpec);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPClientTransactionState::HandleResponse(IN SipMessage* pstMessage)
{
    // Update the transaction information for incoming response
    IMS_SINT32 nValidity;

    //---------------------------------------------------------------------------------------------

    // Check the message validity - CSeq number
    IMS_UINT32 nCSeqNum = SIPStack::GetCSeqNumber(pstMessage);

    if (nCSeqNumber != nCSeqNum)
    {
        // notify the error ???
        IMS_TRACE_E(0, "Sequence number is not ordered - Current (%d), New (%d)", nCSeqNumber,
                nCSeqNum, 0);
        return SIPPrivate::MESSAGE_DISCARDED;
    }

    SipMethod objMethod = SIPStack::GetMethod(pstMessage);
    SIPMessageInfo objMInfo(GetSlotId(), objMethod, pstMessage, SIPMessageInfo::DIRECTION_INCOMING);

    nValidity = pDialogEx->GetDialogState()->CheckToTagValidity(objMInfo);

    // FORKED_RESPONSE
    if (nValidity == SIPPrivate::MESSAGE_VALID_FORKED)
    {
        // Handle forked response
        IMS_TRACE_I("__UAC__ :: _____ FORKED RESPONSE (%s) RECEIVED _____",
                objMethod.ToString().GetStr(), 0, 0);

        if (!HandleForkedResponse(objMInfo))
        {
            IMS_TRACE_E(0, "Handling a forked response failed", 0, 0, 0);
            return SIPPrivate::MESSAGE_DISCARDED;
        }

        return SIPPrivate::MESSAGE_VALID;
    }
    else
    {
        if (nValidity != SIPPrivate::MESSAGE_VALID)
        {
            return nValidity;
        }
    }

    // If the message is valid, then update the response message
    UpdateMessage(pstMessage);

    // Initialize the transport info. when the response message is received
    pTransport->InitTransportOnMessageReceived(pstMessage);

    // Validate the transport information
    nValidity = CheckMessageValidity();

    if (nValidity != SIPPrivate::MESSAGE_VALID)
    {
        IMS_TRACE_E(nValidity, "Checking the message validity failed", 0, 0, 0);
        return nValidity;
    }

    nValidity = pDialogEx->UpdateDialogDetails(objMInfo);

    if (nValidity != SIPPrivate::MESSAGE_VALID)
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
    if (!pForkedTxnMngr.IsNull())
    {
        IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(pstMessage);

        if (SipStatusCode::IsFinal(nStatusCode))
        {
            pForkedTxnMngr->SetTransactionCompleted(nStatusCode);
        }
    }

    // Notify the response to SIPClientConnection
    if (piListener != IMS_NULL)
    {
        piListener->ClientTransactionState_ResponseReceived(pstMessage);
    }
    else
    {
        IMS_TRACE_E(0, "No listener (%s)",
                SipDebug::GetCharA1(pDialogEx->GetDialogState()->GetCallId().GetStr(), 8, '@'), 0,
                0);
    }

    return SIPPrivate::MESSAGE_VALID;
}

/*

Remarks

*/
PUBLIC GLOBAL IMS_SINT32 SIPClientTransactionState::MatchTransaction(IN SipMessage* pSipMsg,
        IN CONST SIPTransportAddress& objFarEnd, OUT RCPtr<SIPClientTransactionState>& pCTState)
{
    IMS_TRACE_I("SIPClientTransactionState::MatchTransaction ... ", 0, 0, 0);

    SipMethod objMethod = SIPStack::GetMethod(pSipMsg);

    if (objMethod.Equals(SipMethod::ACK) && !SIPStack::IsRequestMessage(pSipMsg))
    {
        IMS_TRACE_I("__UAC__ :: __ACK RESPONSE RECEIVED__", 0, 0, 0);
        return SIPPrivate::MESSAGE_DISCARDED;
    }

    SipTransportParameter objTranspParam;

    /* Fill transport details */
    objTranspParam.setHostAddress(objFarEnd.GetIPAddress().ToString().GetStr());
    objTranspParam.setPort(objFarEnd.GetPort());
    objTranspParam.setTranspProtocol(objFarEnd.GetProtocol());

    if (objFarEnd.GetIPAddress().IsIPv4Address() == IMS_TRUE)
    {
        objTranspParam.setTanspIpType(SipTransportInfo::NETWORK_IPV4);
    }
    else
    {
        objTranspParam.setTanspIpType(SipTransportInfo::NETWORK_IPV6);
    }

    /* Prepare User data */
    ISipUserData objUserData;
    SipTxnKey* pTxnKey = IMS_NULL;
    SipEn_TxnStatus eTxnStatus = ETXNSTATUS_INVALID;
    IMS_UINT16 nError = 0;
    IMS_SINT32 nEncodingOptions = SIPPrivate::GetEncodingOptions();
    IMS_UINT32 nMsgOptions = ESIPMSGOPT_NONE;

    if ((nEncodingOptions & SIPPrivate::OPT_E_SHORTFORM) != 0)
    {
        nMsgOptions |= ESIPMSGOPT_ENCSHORTFORM;
    }

    objUserData.SetMsgOptions(nMsgOptions);

    SIP_BOOL bStatus = SipStackManager::GetInstance()->OnRecvMessage(pSipMsg, &objTranspParam,
            &objUserData, reinterpret_cast<SIP_INT32*>(&eTxnStatus), &pTxnKey, &nError);
    if (bStatus == SIP_FALSE)
    {
        IMS_TRACE_E(nError, "OnRecvMessage() Failed", 0, 0, 0);
        return SIPPrivate::MESSAGE_FAILED;
    }

    switch (eTxnStatus)
    {
        case ETXNSTATUS_NEWREQRECVD:
        {
            IMS_TRACE_I("__UAC__ :: _____ NEW REQUEST _____", 0, 0, 0);
            break;
        }
        case ETXNSTATUS_VALIDMESSAGE:
        {
            IMS_TRACE_I("__UAC__ :: _____ VALID MESSAGE _____", 0, 0, 0);
            break;
        }
        case ETXNSTATUS_2XX_STRAYRESP:
        {
            IMS_TRACE_I("__UAC__ :: _____STRAY 2XX RESPONSE _____", 0, 0, 0);
            if (!SIPAckPackage::HandleStray2xx(pSipMsg))
            {
                IMS_TRACE_E(0, "__UAC__ :: 2XX RETRANSMISSION HANDLING FAILED.", 0, 0, 0);
            }
            return SIPPrivate::MESSAGE_DISCARDED;
        }
        case ETXNSTATUS_IGNOREREQ:
        {
            IMS_TRACE_I("__UAC__ :: _____ IGNORE REQUEST _____", 0, 0, 0);
            return SIPPrivate::MESSAGE_DISCARDED;
        }
        case ETXNSTATUS_IGNORERESP:
        {
            IMS_TRACE_I("__UAC__ :: _____ IGNORE RESPONSE _____", 0, 0, 0);
            return SIPPrivate::MESSAGE_DISCARDED;
        }
        case ETXNSTATUS_STRAYRESP:
        {
            IMS_TRACE_I("__UAC__ :: _____ STRAY RESPONSE _____", 0, 0, 0);
            return SIPPrivate::MESSAGE_DISCARDED;
        }
        case ETXNSTATUS_RETRANSMISSION:
        {
            IMS_TRACE_I("__UAC__ :: _____ REMOTE RETRANSMISSION _____", 0, 0, 0);
            return SIPPrivate::MESSAGE_DISCARDED;
        }
        case ETXNSTATUS_ERRORONSEND:
        case ETXNSTATUS_INVALIDMESSAGE:
        case ETXNSTATUS_INVALID:
        {
            IMS_TRACE_I("__UAC__ :: _____ PROCESSING FAILED _____", 0, 0, 0);
            return SIPPrivate::MESSAGE_FAILED;
        }
        default:
            IMS_TRACE_I("__UAC__ :: _____ PROCESSING FAILED _____", 0, 0, 0);
            return SIPPrivate::MESSAGE_FAILED;
    }

    SipTxnContext* pTxnContext = (SipTxnContext*)objUserData.GetUserData();

    if (pTxnContext == IMS_NULL)
    {
        IMS_TRACE_E(0, "pTxnContext is NULL", 0, 0, 0);
        return SIPPrivate::MESSAGE_FAILED;
    }

    SIPTxnContextData* pTxnContextData = (SIPTxnContextData*)pTxnContext->pTxnContextData;

    if (pTxnContextData == IMS_NULL)
    {
        if (objUserData.GetDeleteFlag() == SIP_TRUE)
        {
            SIPStack::DestroyTxnContext(pTxnContext);
        }

        SIPStack::FreeTxnKey(pTxnKey);
        // fatal error
        IMS_TRACE_E(0, "Getting the transaction context data failed", 0, 0, 0);
        return SIPPrivate::MESSAGE_FAILED;
    }

    pCTState = DYNAMIC_CAST(SIPClientTransactionState*, pTxnContextData->GetTxnState());

    if (pCTState.IsNull())
    {
        if (objUserData.GetDeleteFlag() == SIP_TRUE)
        {
            SIPStack::DestroyTxnContext(pTxnContext);
        }

        SIPStack::FreeTxnKey(pTxnKey);

        // fatal error
        IMS_TRACE_E(
                0, "The transaction context data is missing the transaction state info.", 0, 0, 0);
        return SIPPrivate::MESSAGE_FAILED;
    }

    // FORKED_RESPONSE :: Check & look up the appropriate client transaction ...
    // FORKED_RESPONSE_TO_SUPPORT_EARLY_DIALOG_TERMINATION
    RCPtr<SIPForkedTransactionManager> pTmpFTM;

    if (!pCTState->pForkedTxnMngr.IsNull())
    {
        pTmpFTM = pCTState->pForkedTxnMngr;
        IMS_TRACE_D("FTM :: Association with this transaction", 0, 0, 0);
    }
    else if (!pCTState->pPersistentForkedTxnMngr.IsNull())
    {
        pTmpFTM = pCTState->pPersistentForkedTxnMngr;
        IMS_TRACE_D("FTM :: No association with this transaction, but choose it", 0, 0, 0);
    }

    if (objMethod.Equals(SipMethod::INVITE) && !pTmpFTM.IsNull())
    {
        SIPClientTransactionState* pTmpCTState = pTmpFTM->Lookup(pSipMsg);

        if ((pTmpCTState != IMS_NULL) && (pTmpCTState != pCTState.Get()))
        {
            IMS_TRACE_D("FTM :: The response is received via the other transaction", 0, 0, 0);

            pCTState = pTmpCTState;
        }
    }

    // Update the transaction key if it is different
    if ((pTxnKey != IMS_NULL) && (pTxnKey != pCTState->pstTxnKey))
    {
        SIPStack::FreeTxnKey(pCTState->pstTxnKey);

        pCTState->pstTxnKey = pTxnKey;
        SIPStack::AddReference(pCTState->pstTxnKey);
    }
    else
    {
        SIPStack::FreeTxnKey(pTxnKey);
    }

    // Release transaction context if transaction is terminated
    if (objUserData.GetDeleteFlag() == SIP_TRUE)
    {
        SIPStack::DestroyTxnContext(pTxnContext);
    }

    // SIP_MESSAGE_TRACKER
    SIPFactoryProxy* pFactoryProxy = SIPFactoryProxy::GetInstance();

    if (pFactoryProxy->IsMessageTrackerEnabled(pCTState->GetSlotId()))
    {
        SIPMessageTracker* pMessageTracker =
                pFactoryProxy->GetMessageTracker(pCTState->GetSlotId());

        pMessageTracker->NotifyMessageReceived(objMethod, SIPStack::GetStatusCode(pSipMsg),
                AString(SIPStack::TxnKey_GetCallId(pCTState->pstTxnKey)));
    }

    return SIPPrivate::MESSAGE_VALID;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPClientTransactionState::CorrectRouteHeader(IN_OUT SipMessage*& pstMessage)
{
    // Get the topmost Route entry
    SipAddrSpec* pstAddrSpec = SIPStack::GetAddrSpec(pstMessage, ISipHeader::ROUTE);

    //---------------------------------------------------------------------------------------------

    if (pstAddrSpec == IMS_NULL)
    {
        if (SIPStack::IsLastErrorNoExist())
        {
            nRoutingType = TARGET_NO_ROUTE;
            return IMS_TRUE;
        }

        return IMS_FALSE;
    }

    // The top Route entry is a loose router : no changes
    //    --> Request-URI is already set by the application or from To header
    const AString LR(Sip::STR_LR);

    if (SIPStack::HasParameter(pstAddrSpec, LR))
    {
        AString strLRValue = SIPStack::GetParameter(pstAddrSpec, LR);

        if ((strLRValue.GetLength() == 0) || strLRValue.EqualsIgnoreCase("true"))
        {
            nRoutingType = TARGET_LR;

            SIPStack::FreeAddrSpec(pstAddrSpec);
            return IMS_TRUE;
        }
    }

    nRoutingType = TARGET_SR;

    // Here, the top Route entry is a strict router:
    //    --> This addr-spec SHOULD be set in the Request-Line
    SipAddrSpec* pstReqLineAddrSpec = SIPStack::GetRequestUri(pstMessage);

    SipHeaderBase* pstRouteHeader = SIPStack::CreateHeader(ISipHeader::ROUTE, pstReqLineAddrSpec);
    SIPStack::FreeAddrSpec(pstReqLineAddrSpec);

    if (!SIPStack::AppendHeader(pstRouteHeader, pstMessage))
    {
        SIPStack::FreeHeaderEx(pstRouteHeader);
        SIPStack::FreeAddrSpec(pstAddrSpec);
        return IMS_FALSE;
    }

    SIPStack::FreeHeaderEx(pstRouteHeader);

    // Set the Request-URI from the addr-spec of the top Route entry
    if (!SIPStack::SetRequestUri(pstAddrSpec, pstMessage))
    {
        SIPStack::FreeAddrSpec(pstAddrSpec);
        return IMS_FALSE;
    }

    SIPStack::FreeAddrSpec(pstAddrSpec);

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void SIPClientTransactionState::CheckNSendAck()
{
    //---------------------------------------------------------------------------------------------

    if (!SIPStack::IsAckTransmissionRequiredForNon2XX())
    {
        // SIP stack takes care of the failure response for INVITE and sends ACK
        return;
    }

    // Check if non-2xx response to INVITE, then the client transaction needs to send ACK request.
    IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(pstMessage);

    if (nStatusCode >= SipStatusCode::SC_300)
    {
        SipMessage* pstAckMessage = SIPStack::CreateMessage(SIPStack::SIP_MESSAGE_REQUEST);

        if (pstAckMessage == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating an ACK request message failed", 0, 0, 0);
            return;
        }

        // Initializes ACK request message
        if (!InitAck(pstAckMessage, pstMessage))
        {
            IMS_TRACE_E(0, "Initializing ACK request failed", 0, 0, 0);
            SIPStack::FreeMessage(pstAckMessage);
            return;
        }

        SipMethod objMethod(SipMethod::ACK);
        SIPMessageInfo objMInfo(
                GetSlotId(), objMethod, pstAckMessage, SIPMessageInfo::DIRECTION_OUTGOING);

        // Update the Contact information
        pDialogEx->UpdateDialogDetails(objMInfo);

        // Store the last request message
        // SIPStack::FreeMessage(pstLastMessage);
        // pstLastMessage = SIPStack::CloneMessage(pstAckMessage);

        IMS_TRACE_I("___ SENDING ACK REQUEST .....", 0, 0, 0);
        SIPTransactionState::Send(pstAckMessage, IMS_NULL);

        SIPStack::FreeMessage(pstAckMessage);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL SIPTransactionState* SIPClientTransactionState::Clone()
{
    SIPClientTransactionState* pTxnState = new SIPClientTransactionState(GetSlotId());

    //---------------------------------------------------------------------------------------------

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

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPClientTransactionState::HandleForkedResponse(IN CONST SIPMessageInfo& objMInfo)
{
    //---------------------------------------------------------------------------------------------

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "No listener (%s)",
                SipDebug::GetCharA1(pDialogEx->GetDialogState()->GetCallId().GetStr(), 8, '@'), 0,
                0);
        return IMS_FALSE;
    }

    RCPtr<SIPClientTransactionState> pForkedCTState = new SIPClientTransactionState(GetSlotId());

    if (pForkedCTState.IsNull())
    {
        return IMS_FALSE;
    }

    // Add the forked transaction to the forked transaction manager...
    pForkedCTState->pForkedTxnMngr = pForkedTxnMngr;

    if (!pForkedCTState->pForkedTxnMngr.IsNull())
    {
        pForkedCTState->pForkedTxnMngr->Add(pForkedCTState.Get());
    }

    if (!pForkedCTState->InitTxnDetails(this))
    {
        IMS_TRACE_E(0, "Initializing SIP client transaction state failed", 0, 0, 0);
        return IMS_FALSE;
    }

    RCPtr<SIPDialogState> pDState = new SIPDialogState();

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
                SIPDialogState::DIALOG_FORKED_RESPONSE, pDialogEx->GetDialogState()))
    {
        IMS_TRACE_E(0, "Initializing a dialog details from the forked response message", 0, 0, 0);
        return IMS_FALSE;
    }

    pForkedCTState->pDialogEx = SIPDialogEx::CreateDialog(pDState.Get(), objMInfo);

    if (pForkedCTState->pDialogEx.IsNull())
    {
        return IMS_FALSE;
    }

    SIPMessageInfo objPrevMInfo(GetSlotId(), objMInfo.GetMethod(), GetLastMessage(),
            SIPMessageInfo::DIRECTION_OUTGOING);

    if (pForkedCTState->pDialogEx->UpdateDialogDetails(objPrevMInfo) != SIPPrivate::MESSAGE_VALID)
    {
        IMS_TRACE_E(0, "Updating a dialog details from the previous request message", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pForkedCTState->pDialogEx->UpdateDialogDetails(objMInfo) != SIPPrivate::MESSAGE_VALID)
    {
        IMS_TRACE_E(0, "Updating a dialog details from the forked response message", 0, 0, 0);
        return IMS_FALSE;
    }

    pForkedCTState->UpdateMessage(objMInfo.GetMessage());

    // Initialize the transport info. when the response message is received
    pForkedCTState->pTransport->InitTransportOnMessageReceived(objMInfo.GetMessage());

    // Validate the transport information
    IMS_SINT32 nValidity = pForkedCTState->CheckMessageValidity();

    if (nValidity != SIPPrivate::MESSAGE_VALID)
    {
        IMS_TRACE_E(nValidity, "Checking the message validity failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Check if non-2xx response to INVITE, then the client transaction needs to send ACK request.
    if (objMInfo.GetMethod().Equals(SipMethod::INVITE))
    {
        pForkedCTState->CheckNSendAck();
    }

    // FIX_NO_ACK_RETRANSMISSION
    if (!pForkedCTState->pForkedTxnMngr.IsNull())
    {
        IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(objMInfo.GetMessage());

        if (SipStatusCode::IsFinal(nStatusCode))
        {
            pForkedCTState->pForkedTxnMngr->SetTransactionCompleted(nStatusCode);
        }
    }

    piListener->ClientTransactionState_ForkedResponseReceived(pForkedCTState.Get());

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPClientTransactionState::InitAck(
        IN_OUT SipMessage*& pstAckMessage, IN SipMessage* pstRespMessage)
{
    SipHeaderBase* pstGetHdr;
    SipHeaderBase* pstHeader;
    SipMethod objMethodACK(SipMethod::ACK);

    //---------------------------------------------------------------------------------------------

    // Sets SIP method name
    SIPStack::SetMethod(objMethodACK, pstAckMessage);

    // Sets a Request-URI field
    SipAddrSpec* pstAddrSpec = SIPStack::GetRequestUri(pstLastMessage);
    (void)SIPStack::SetRequestUri(pstAddrSpec, pstAckMessage);
    SIPStack::FreeAddrSpec(pstAddrSpec);

    AString strHBody;

    // Sets CSeq header field
    strHBody.SetNumber(nCSeqNumber);
    strHBody += TextParser::CHAR_SP;
    strHBody += SipMethod::NAME[SipMethod::ACK];
    pstHeader = SIPStack::DecodeHeader(ISipHeader::CSEQ, strHBody);

    if (!SIPStack::SetHeader(pstHeader, pstAckMessage))
    {
        SIPStack::FreeHeaderEx(pstHeader);
        return IMS_FALSE;
    }

    SIPStack::FreeHeaderEx(pstHeader);

    // Sets Max-Forwards header field
    strHBody.SetNumber(SIPPrivate::MAX_HOP);
    pstHeader = SIPStack::DecodeHeader(ISipHeader::MAX_FORWARDS, strHBody);

    if (!SIPStack::SetHeader(pstHeader, pstAckMessage))
    {
        SIPStack::FreeHeaderEx(pstHeader);
        return IMS_FALSE;
    }

    SIPStack::FreeHeaderEx(pstHeader);

    // Sets From header field
    pstGetHdr = SIPStack::GetHeader(pstLastMessage, ISipHeader::FROM);

    pstHeader = SIPStack::CloneHeader(pstGetHdr);
    SIPStack::FreeHeaderEx(pstGetHdr);

    if (pstHeader != IMS_NULL)
    {
        if (!SIPStack::SetHeader(pstHeader, pstAckMessage))
        {
            SIPStack::FreeHeaderEx(pstHeader);
            return IMS_FALSE;
        }

        SIPStack::FreeHeaderEx(pstHeader);
    }

    // Sets To header field
    pstGetHdr = SIPStack::GetHeader(pstLastMessage, ISipHeader::TO);

    pstHeader = SIPStack::CloneHeader(pstGetHdr);
    SIPStack::FreeHeaderEx(pstGetHdr);

    if (pstHeader != IMS_NULL)
    {
        if (!SIPStack::SetHeader(pstHeader, pstAckMessage))
        {
            SIPStack::FreeHeaderEx(pstHeader);
            return IMS_FALSE;
        }

        // Update To-Tag from the last response message
        pstGetHdr = SIPStack::GetHeader(pstRespMessage, ISipHeader::TO);

        AString strToTag = SIPStack::GetParameter(pstGetHdr, Sip::STR_TAG);

        if (!strToTag.IsNULL())
        {
            (void)SIPStack::SetParameter(pstHeader, Sip::STR_TAG, strToTag);
        }

        SIPStack::FreeHeaderEx(pstGetHdr);

        SIPStack::FreeHeaderEx(pstHeader);
    }

    // Sets Call-ID header field
    pstGetHdr = SIPStack::GetHeader(pstLastMessage, ISipHeader::CALL_ID);

    pstHeader = SIPStack::CloneHeader(pstGetHdr);
    SIPStack::FreeHeaderEx(pstGetHdr);

    if (pstHeader != IMS_NULL)
    {
        if (!SIPStack::SetHeader(pstHeader, pstAckMessage))
        {
            SIPStack::FreeHeaderEx(pstHeader);
            return IMS_FALSE;
        }

        SIPStack::FreeHeaderEx(pstHeader);
    }

    // Sets Via header field
    pstGetHdr = SIPStack::GetHeader(pstLastMessage, ISipHeader::VIA);

    pstHeader = SIPStack::CloneHeader(pstGetHdr);
    SIPStack::FreeHeaderEx(pstGetHdr);

    if (pstHeader != IMS_NULL)
    {
        if (!SIPStack::SetHeader(pstHeader, pstAckMessage))
        {
            SIPStack::FreeHeaderEx(pstHeader);
            return IMS_FALSE;
        }

        SIPStack::FreeHeaderEx(pstHeader);
    }

    // Sets Route header field
    IMS_SINT32 nHCount = SIPStack::GetHeaderCount(pstLastMessage, ISipHeader::ROUTE);

    for (IMS_SINT32 i = 0; i < nHCount; i++)
    {
        pstGetHdr = SIPStack::GetHeader(pstLastMessage, ISipHeader::ROUTE, i);

        if (SIPStack::IsValidHeader(pstGetHdr))
        {
            if (!SIPStack::AppendHeader(pstGetHdr, pstAckMessage))
            {
                SIPStack::FreeHeaderEx(pstGetHdr);
                return IMS_FALSE;
            }
        }

        SIPStack::FreeHeaderEx(pstGetHdr);
    }

    // Sets User-Agent header if it is present in the previous request
    pstGetHdr = SIPStack::GetUnknownHeader(pstLastMessage, SipHeaderName::USER_AGENT);

    if (SIPStack::IsValidHeader(pstGetHdr))
    {
        pstHeader = SIPStack::CloneHeader(pstGetHdr);

        if (pstHeader != IMS_NULL)
        {
            // Even if setting User-Agent is failed, it will proceed to send ACK request.
            SIPStack::SetHeader(pstHeader, pstAckMessage);
            SIPStack::FreeHeaderEx(pstHeader);
        }
    }

    SIPStack::FreeHeaderEx(pstGetHdr);

    // P-Access-Network-Info header if required
    if (SipFeatures::IsPaniHeaderForAckRequired(GetSlotId()))
    {
        SetPANIHeader(objMethodACK, pstAckMessage);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPClientTransactionState::SetDialogRelatedHeaders(IN CONST SipMethod& objMethod)
{
    //---------------------------------------------------------------------------------------------

    if (pDialogEx->GetState() == SIPDState::STATE_INIT)
    {
        // Check if the Call-ID field is present or not
        if (!SIPStack::IsHeaderPresent(pstMessage, ISipHeader::CALL_ID))
        {
            // Add Call-ID in here
            const IPAddress& objAddress = pTransport->GetIPAddress();
            AString strCallId = SIPUtil::GenerateCallId(objAddress.ToString());
            SipHeaderBase* pstHeader = SIPStack::DecodeHeader(ISipHeader::CALL_ID, strCallId);

            if (pstHeader == IMS_NULL)
                return IMS_FALSE;

            if (!SIPStack::SetHeader(pstHeader, pstMessage))
            {
                SIPStack::FreeHeaderEx(pstHeader);
                return IMS_FALSE;
            }

            SIPStack::FreeHeaderEx(pstHeader);

            // HEADER_REQ_SESSION-ID
            if (SipFeatures::IsHeaderSessionIdRequired(GetSlotId()))
            {
                AString strSessionId = SIPUtil::GenerateSessionId(GetSlotId(), strCallId);

                if (strSessionId.GetLength() > 0)
                {
                    const AString SESSION_ID(SipHeaderName::SESSION_ID);
                    SipHeaderBase* pstSessionId =
                            SIPStack::DecodeHeader(ISipHeader::UNKNOWN, SESSION_ID, strSessionId);

                    if (pstSessionId != IMS_NULL)
                    {
                        (void)SIPStack::SetUnknownHeader(pstSessionId, SESSION_ID, pstMessage);
                        SIPStack::FreeHeaderEx(pstSessionId);
                    }
                }
            }
        }

        // Set From-Tag
        if (!objMethod.Equals(SipMethod::CANCEL) && !objMethod.Equals(SipMethod::ACK))
        {
            SipHeaderBase* pstHeader = SIPStack::GetHeader(pstMessage, ISipHeader::FROM);

            if (!SIPStack::HasParameter(pstHeader, Sip::STR_TAG))
            {
                AString strTagVal = SIPUtil::GenerateTag(
                        SipConfigProxy::GetTagPrefix(GetSlotId(), GetSIPProfile()));

                if (!SIPStack::SetParameter(pstHeader, Sip::STR_TAG, strTagVal))
                {
                    SIPStack::FreeHeaderEx(pstHeader);
                    return IMS_FALSE;
                }
            }

            SIPStack::FreeHeaderEx(pstHeader);
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPClientTransactionState::SetMandatoryHeaders(IN CONST SipMethod& objMethod)
{
    SipHeaderBase* pstHeader;

    //---------------------------------------------------------------------------------------------

    if (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::ACK))
        nClass = CLASS_INVITE;
    else if (objMethod.Equals(SipMethod::PRACK))
        nClass = CLASS_OVERLAP;
    else
        nClass = CLASS_REGULAR;

    AString strHBody;

    // Sets CSeq header field
    strHBody.SetNumber(nCSeqNumber);
    strHBody.Append(TextParser::CHAR_SP);
    strHBody.Append(objMethod.ToString());

    pstHeader = SIPStack::DecodeHeader(ISipHeader::CSEQ, strHBody);

    if (!SIPStack::SetHeader(pstHeader, pstMessage))
    {
        SIPStack::FreeHeaderEx(pstHeader);
        return IMS_FALSE;
    }

    SIPStack::FreeHeaderEx(pstHeader);

    // Sets Max-Forwards header field
    strHBody.SetNumber(SIPPrivate::MAX_HOP);
    pstHeader = SIPStack::DecodeHeader(ISipHeader::MAX_FORWARDS, strHBody);

    if (!SIPStack::SetHeader(pstHeader, pstMessage))
    {
        SIPStack::FreeHeaderEx(pstHeader);
        return IMS_FALSE;
    }

    SIPStack::FreeHeaderEx(pstHeader);

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void SIPClientTransactionState::SetPANIHeader(
        IN CONST SipMethod& objMethod, IN_OUT SipMessage*& pstMessage)
{
    AString strPANI;

    if (PAccessNetworkInfoHeader::FormHeaderForOperatorSpecific(
                GetSlotId(), pTransport->GetIPAddress(), objMethod, GetSIPProfile(), strPANI))
    {
        if (strPANI.GetLength() > 0)
        {
            SipHeaderBase* pstHeader = SIPStack::DecodeHeader(
                    ISipHeader::P_ACCESS_NETWORK_INFO, AString::ConstNull(), strPANI);

            if (pstHeader != IMS_NULL)
            {
                SIPStack::SetHeader(pstHeader, pstMessage);
                SIPStack::FreeHeaderEx(pstHeader);
            }
        }
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPClientTransactionState::UpdateTxnDetails(IN CONST SipMethod& objMethod)
{
    // Update the transaction information for outgoing request
    SIPMessageInfo objMInfo(GetSlotId(), objMethod, pstMessage, SIPMessageInfo::DIRECTION_OUTGOING);

    //---------------------------------------------------------------------------------------------

    if (pDialogEx->GetState() == SIPDState::STATE_INIT)
    {
        if (!pDialogEx->GetDialogState()->InitDialogDetails(pstMessage))
        {
            return IMS_FALSE;
        }

        // Update the sequence number in the CSeq header if changed.
        IMS_UINT32 nSeqNum = SIPStack::GetCSeqNumber(pstMessage);

        if (nSeqNum != SIPPrivate::INVALID_SEQ_NUM)
            nCSeqNumber = nSeqNum;
    }

    // If the request can create a dialog usage, then update the state if changed
    if (SIPDialogBase::IsDialogCreatable(objMethod))
    {
        SIPDialogState* pDState = pDialogEx->GetDialogState();
        SIPDialogEx* pTmpDialogEx = pDState->GetDialogUsage(objMInfo);

        // Not present; So, use a new dialog
        if (pTmpDialogEx == IMS_NULL)
        {
            pDialogEx->InitDialogWithDelay(objMInfo);
        }
        // Use an existing dialog
        else
        {
            IMS_TRACE_D("_____ UPDATE DIALOG  (%s:%s) _____", objMethod.ToString().GetStr(),
                    SipDebug::GetCharA1(pDState->GetCallId().GetStr(), 8, '@'), 0);

            pDialogEx = pTmpDialogEx;
        }
    }

    // Update the Contact information
    pDialogEx->UpdateDialogDetails(objMInfo);

    // Store the last request message
    if (!objMethod.Equals(SipMethod::ACK))
    {
        SIPStack::FreeMessage(pstLastMessage);
        pstLastMessage = SIPStack::CloneMessage(pstMessage);
    }

    return IMS_TRUE;
}
