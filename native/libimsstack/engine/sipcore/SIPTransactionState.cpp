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
#include "SIPPrivate.h"
#include "SIPTxnContextData.h"
#include "SIPStackState.h"
#include "SIPFactoryProxy.h"
// SIP_PACKET_TRACKER
#include "SIPMessage.h"
#include "ISIPTransactionStateListener.h"
#include "SIPRTConfigUtils.h"
// SIP_MESSAGE_TRACKER
#include "SIPMessageTracker.h"
// SIP_PACKET_TRACKER
#include "SIPPacketTracker.h"
// SIP_IPSEC_STATE
#include "SIPIPSecState.h"
#include "SIPTransport.h"
#include "SIPTransactionState.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPTransactionState::SIPTransactionState() :
        RCObject(),
        nType(0),
        nClass(CLASS_NONE),
        nCSeqNumber(1)
        // MULTI_REG_SIP_PROFILE
        ,
        pSIPProfile(IMS_NULL),
        piListener(IMS_NULL),
        pDialogEx(IMS_NULL),
        pTransport(IMS_NULL),
        pstMessage(IMS_NULL),
        pstLastMessage(IMS_NULL),
        pstTxnKey(IMS_NULL),
        pstRPRTxnKey(IMS_NULL)
{
}

PUBLIC
SIPTransactionState::SIPTransactionState(IN SIPDialogEx* pDialogEx_) :
        RCObject(),
        nType(0),
        nClass(CLASS_NONE),
        nCSeqNumber(1)
        // MULTI_REG_SIP_PROFILE
        ,
        pSIPProfile(IMS_NULL),
        piListener(IMS_NULL),
        pDialogEx(pDialogEx_),
        pTransport(IMS_NULL),
        pstMessage(IMS_NULL),
        pstLastMessage(IMS_NULL),
        pstTxnKey(IMS_NULL),
        pstRPRTxnKey(IMS_NULL)
{
}

PUBLIC
SIPTransactionState::SIPTransactionState(IN const SIPTransactionState& objRHS) :
        RCObject(objRHS),
        nType(objRHS.nType),
        nClass(objRHS.nClass),
        nCSeqNumber(objRHS.nCSeqNumber)
        // MULTI_REG_SIP_PROFILE
        ,
        pSIPProfile(objRHS.pSIPProfile),
        piListener(objRHS.piListener),
        pDialogEx(objRHS.pDialogEx),
        pTransport(IMS_NULL),
        pstMessage(IMS_NULL),
        pstLastMessage(IMS_NULL),
        pstTxnKey(IMS_NULL),
        pstRPRTxnKey(IMS_NULL)
{
    // NOTE: If reference count is not used, you MUST implement this copy constructor
}

PUBLIC VIRTUAL SIPTransactionState::~SIPTransactionState()
{
    //---------------------------------------------------------------------------------------------

    if (pTransport != IMS_NULL)
        delete pTransport;

    SIPStack::FreeMessage(pstLastMessage);
    SIPStack::FreeMessage(pstMessage);
    SIPStack::FreeTxnKey(pstTxnKey);
    SIPStack::FreeTxnKey(pstRPRTxnKey);

    IMS_TRACE_D("Destructor :: SIPTransactionState", 0, 0, 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL void SIPTransactionState::Abort()
{
    SipTxnKey* pstAbortTxnKey = pstTxnKey;

    //---------------------------------------------------------------------------------------------

    if ((pstAbortTxnKey == IMS_NULL) && (nType == TYPE_CLIENT) && (pstLastMessage != IMS_NULL))
    {
        pstAbortTxnKey = SIPStack::CreateTxnKey(pstLastMessage, SIPStack::SIP_TXN_MSG_SENT);
        pstTxnKey = pstAbortTxnKey;
    }

    // SIP_IPSEC_STATE
    SIPFactoryProxy* pFactoryProxy = SIPFactoryProxy::GetInstance();

    if (pFactoryProxy->IsIPSecStateEnabled(GetSlotId()))
    {
        SIPIPSecState* pIPSecState = pFactoryProxy->GetIPSecState(GetSlotId());
        pIPSecState->NotifyTransactionAborted(pstTxnKey);
    }

    // Invoke the stack API to abort the transaction.
    (void)SIPStackState::GetInstance()->AbortTransaction(pstAbortTxnKey, this);

    if (pstRPRTxnKey != IMS_NULL)
    {
        (void)SIPStackState::GetInstance()->AbortTransaction(pstRPRTxnKey, this);
        SIPStack::FreeTxnKey(pstRPRTxnKey);
    }

    SIPStack::FreeTxnKey(pstTxnKey);
}

/*
 Terminates SIP transaction if it's not available anymore.

Remarks

*/
PUBLIC VIRTUAL void SIPTransactionState::Terminate()
{
    IMS_TRACE_D("STS::Terminate", 0, 0, 0);

    // If there is ongoing transaction, it will be terminated promptly.
    (void)SIPStackState::GetInstance()->AbortTransaction(pstTxnKey, this);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 SIPTransactionState::CheckMessageValidity()
{
    //---------------------------------------------------------------------------------------------

    return SIPPrivate::MESSAGE_VALID;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPTransactionState::FormMessage()
{
    // no-op
    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPTransactionState::InitTxnDetails(IN CONST SIPTransactionState* pTState)
{
    //---------------------------------------------------------------------------------------------

    if (pTState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    nClass = pTState->nClass;
    nCSeqNumber = pTState->nCSeqNumber;
    // MULTI_REG_SIP_PROFILE
    pSIPProfile = pTState->pSIPProfile;
    pDialogEx = pTState->pDialogEx;

    if (!pTransport->InitTransportDetails(pTState->pTransport))
    {
        IMS_TRACE_E(0, "Initializing SIP transport details failed", 0, 0, 0);
        return IMS_FALSE;
    }

    SIPStack::FreeMessage(pstLastMessage);
    SIPStack::FreeMessage(pstMessage);
    SIPStack::FreeTxnKey(pstTxnKey);
    SIPStack::FreeTxnKey(pstRPRTxnKey);

    pstMessage = pTState->pstMessage;
    SIPStack::AddReference(pstMessage);

    pstLastMessage = pTState->pstLastMessage;
    SIPStack::AddReference(pstLastMessage);

    pstTxnKey = pTState->pstTxnKey;
    SIPStack::AddReference(pstTxnKey);

    pstRPRTxnKey = pTState->pstRPRTxnKey;
    SIPStack::AddReference(pstRPRTxnKey);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL void SIPTransactionState::NotifyTimerExpired()
{
    //---------------------------------------------------------------------------------------------

    if (piListener != IMS_NULL)
        piListener->TransactionState_TimerExpired();
}

/*
 It's invoked by SIP stack after sending SIP message successfully.

Remarks

*/
PUBLIC VIRTUAL void SIPTransactionState::PostProcessMessageSentByStack(
        IN SipMessage* pstSipMsg, IN const ByteArray& objBuffer)
{
    SIPFactoryProxy* pFactoryProxy = SIPFactoryProxy::GetInstance();

    // SIP_PACKET_TRACKER
    if (pFactoryProxy->IsPacketTrackerEnabled(GetSlotId()))
    {
        SIPPacketTracker* pPacketTracker = pFactoryProxy->GetPacketTracker(GetSlotId());
        SIPMessage objSIPMsg(pstSipMsg);

        pPacketTracker->NotifyMessageSent(&objSIPMsg, objBuffer, IMS_FALSE);
    }

    // SIP_IPSEC_STATE
    if (pFactoryProxy->IsIPSecStateEnabled(GetSlotId()))
    {
        SIPIPSecState* pIPSecState = pFactoryProxy->GetIPSecState(GetSlotId());
        const SIPTransportAddress& objNearEnd = pTransport->GetAddress(SIPTransport::TA_NEAR);
        const SIPTransportAddress& objFarEnd = pTransport->GetAddress(SIPTransport::TA_FAR);

        pIPSecState->NotifyMessageSent(objNearEnd, objFarEnd, pstSipMsg);
    }
}

/*
 It's invoked by SIP stack before sending SIP message.

Remarks

*/
PUBLIC VIRTUAL void SIPTransactionState::PreProcessMessageSentByStack(IN SipMessage* pstSipMsg)
{
    SIPFactoryProxy* pFactoryProxy = SIPFactoryProxy::GetInstance();

    // SIP_MESSAGE_TRACKER
    if (pFactoryProxy->IsMessageTrackerEnabled(GetSlotId()))
    {
        SIPMessageTracker* pMessageTracker = pFactoryProxy->GetMessageTracker(GetSlotId());
        const SipMethod objMethod = SIPStack::GetMethod(pstSipMsg);

        if (SIPStack::IsRequestMessage(pstSipMsg))
        {
            pMessageTracker->NotifyMessageSent(
                    objMethod, 0, pDialogEx->GetDialogState()->GetCallId());
        }
        else
        {
            pMessageTracker->NotifyMessageSent(objMethod, SIPStack::GetStatusCode(pstSipMsg),
                    pDialogEx->GetDialogState()->GetCallId());
        }
    }

    // LOG_EXCLUDING_SERVER_INFO
    if (SIPStack::IsRequestMessage(pstSipMsg) &&
            SIPRTConfigUtils::IsMessageHiddenInLog(GetSlotId()))
    {
        SIPStack::DisplayUnknownHeaders(pstSipMsg);
    }

    // Reset the retransmission flag when sending INVITE response
    if ((this->nType == TYPE_SERVER) && (this->nClass == CLASS_INVITE))
    {
        pTransport->InitRetransmissionFlag();
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPTransactionState::Send(IN SipTimerValues* pTV /* = IMS_NULL */)
{
    //---------------------------------------------------------------------------------------------

    return Send(pstMessage, pTV);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT SIPTransactionState::RetransmitMessage()
{
    ByteArray objBuffer;

    //---------------------------------------------------------------------------------------------

    if (pTransport == IMS_NULL)
    {
        IMS_TRACE_E(0, "Transport is already destroyed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!pTransport->EncodeMessage(pstMessage, objBuffer))
    {
        IMS_TRACE_E(0, "Encoding ACK or 2xx to INVITE request (Retransmission) failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!pTransport->SendToNetwork(objBuffer.GetData(), objBuffer.GetLength(), IMS_FALSE))
    {
        IMS_TRACE_E(0, "Retransmitting ACK or 2xx to INVITE request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // SIP_PACKET_TRACKER
    SIPFactoryProxy* pFactoryProxy = SIPFactoryProxy::GetInstance();

    if (pFactoryProxy->IsPacketTrackerEnabled(GetSlotId()))
    {
        SIPPacketTracker* pPacketTracker = pFactoryProxy->GetPacketTracker(GetSlotId());
        SIPMessage objSIPMsg(pstMessage);

        pPacketTracker->NotifyMessageSent(&objSIPMsg, objBuffer, IMS_TRUE);
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPTransactionState::UpdateTransportDetails()
{
    //---------------------------------------------------------------------------------------------

    if (!pTransport->UpdateDestinationInfo(pstMessage))
        return IMS_FALSE;

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPTransactionState::GetSlotId() const
{
    return pTransport->GetSlotId();
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPTransactionState::IsIPSecRequired() const
{
    //---------------------------------------------------------------------------------------------

    return pTransport->IsIPSecRequired();
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PUBLIC
void SIPTransactionState::SetSIPProfile(IN SipProfile* pProfile)
{
    //---------------------------------------------------------------------------------------------

    this->pSIPProfile = pProfile;
}

/*

Remarks

*/
PUBLIC
void SIPTransactionState::SetTransactionListener(IN ISIPTransactionStateListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PUBLIC
void SIPTransactionState::SetTransportListener(IN ISIPTransportErrorListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    pTransport->SetListener(piListener);
}

/*

Remarks
 RFC5626_FLOW_CONTROL, MULTI_REG_TRANSPORT
*/
PUBLIC
void SIPTransactionState::SetTransportTuple(IN CONST IPAddress& objIPA, IN IMS_SINT32 nPortS,
        IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFC /* = 0xFFFF */,
        IN IMS_SINT32 nTransportExt /* = 0 (ANY) */)
{
    //---------------------------------------------------------------------------------------------

    pTransport->SetTransportTuple(objIPA, nPortS, nPortC, nPortFC, nTransportExt);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPTransactionState::SendToNetwork(IN CONST IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen)
{
    //---------------------------------------------------------------------------------------------

    if (pTransport == IMS_NULL)
        return IMS_FALSE;

    return pTransport->SendToNetwork(pBuffer, nBuffLen);
}

/*

Remarks

*/
PUBLIC
void SIPTransactionState::UpdateMessage(IN SipMessage* pstMessage)
{
    //---------------------------------------------------------------------------------------------

    SIPStack::FreeMessage(this->pstMessage);

    this->pstMessage = pstMessage;
    SIPStack::AddReference(this->pstMessage);
}

/*

Remarks

*/
PROTECTED VIRTUAL SIPTransactionState* SIPTransactionState::Clone()
{
    //---------------------------------------------------------------------------------------------

    // The subclass MUST implement this method to create a new SIPTransactionState (client/server)

    return IMS_NULL;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL SIPTransactionState::Send(IN SipMessage* pSipMsg, IN SipTimerValues* pTV)
{
    SipTransportParameter objTranspParam;

    IMS_TRACE_D("Send", 0, 0, 0);

    /* Fill transport details */
    IMS_SINT32 nType = pTransport->GetProtocol(SIPTransport::TA_FAR);

    if (nType == SIPTransportAddress::PROTOCOL_TCP)
        objTranspParam.setTranspProtocol(SipTransportInfo::PROTOCOL_TCP);
    else if (nType == SIPTransportAddress::PROTOCOL_TLS)
        objTranspParam.setTranspProtocol(SipTransportInfo::PROTOCOL_TLS);
    else
        objTranspParam.setTranspProtocol(SipTransportInfo::PROTOCOL_UDP);

    SIPTransportAddress objTA = pTransport->GetAddress(SIPTransport::TA_FAR);

    objTranspParam.setHostAddress(objTA.GetIPAddress().ToString().GetStr());
    objTranspParam.setPort((IMS_UINT16)objTA.GetPort());

    if (objTA.GetIPAddress().IsIPv4Address() == IMS_TRUE)
    {
        objTranspParam.setTanspIpType(SipTransportInfo::NETWORK_IPV4);
    }
    else
    {
        objTranspParam.setTanspIpType(SipTransportInfo::NETWORK_IPV6);
    }

    // Prepare User Data
    SipTxnContext* pstTxnContext = SIPStack::CreateTxnContext();

    if (pstTxnContext == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SetTimerValues(pTV, pstTxnContext);

    SIPTxnContextData* pTxnContextData = new SIPTxnContextData();

    if (pTxnContextData != IMS_NULL)
    {
        IMS_BOOL bIsTxnStateSet = IMS_FALSE;

        // FIX_TXN_HANDLING_ON_401_407_TO_INVITE
        if ((this->nType == TYPE_CLIENT) && (this->nClass == CLASS_INVITE))
        {
            IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(pSipMsg);

            if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
            {
                const SipMethod objMethod = SIPStack::GetMethod(pSipMsg);

                if (objMethod.Equals(SipMethod::ACK))
                {
                    IMS_TRACE_D("AUTH_CHALLENGE_TO_INVITE :: ACK (%d) "
                                "& transaction state is cloned",
                            nStatusCode, 0, 0);

                    bIsTxnStateSet = IMS_TRUE;
                    pTxnContextData->SetTxnState(this->Clone());
                }
            }
        }

        if (!bIsTxnStateSet)
        {
            pTxnContextData->SetTxnState(this);
        }
    }

    pstTxnContext->pTxnContextData = (SIP_VOID*)pTxnContextData;

    ISipUserData objUserData;
    objUserData.SetUserData((SIP_VOID*)pstTxnContext);

    ByteArray objBuffer;

    // Form a raw SIP message
    if (!pTransport->EncodeMessage(pSipMsg, objBuffer))
    {
        SIPStack::DestroyTxnContext(pstTxnContext);
        IMS_TRACE_E(0, "Encoding SIP message failed", 0, 0, 0);
        return IMS_FALSE;  // throw exception : syntax error
    }

    /* Reserver transport resources */
    if (!pTransport->ReserveResource(GetSIPProfile()))
    {
        SIPStack::DestroyTxnContext(pstTxnContext);
        IMS_TRACE_E(0, "Reserving the transport resource failed", 0, 0, 0);
        return IMS_FALSE;  // throw exception : network not available
    }

    // SIP_MESSAGE_TRACKER
    SIPFactoryProxy* pFactoryProxy = SIPFactoryProxy::GetInstance();

    if (pFactoryProxy->IsMessageTrackerEnabled(GetSlotId()))
    {
        SIPMessageTracker* pMessageTracker = pFactoryProxy->GetMessageTracker(GetSlotId());
        const SipMethod objMethod = SIPStack::GetMethod(pSipMsg);

        if (SIPStack::IsRequestMessage(pSipMsg))
        {
            pMessageTracker->NotifyMessageSent(
                    objMethod, 0, pDialogEx->GetDialogState()->GetCallId());
        }
        else
        {
            pMessageTracker->NotifyMessageSent(objMethod, SIPStack::GetStatusCode(pSipMsg),
                    pDialogEx->GetDialogState()->GetCallId());
        }
    }

    // LOG_EXCLUDING_SERVER_INFO
    if (SIPStack::IsRequestMessage(pSipMsg) && SIPRTConfigUtils::IsMessageHiddenInLog(GetSlotId()))
    {
        SIPStack::DisplayUnknownHeaders(pSipMsg);
    }

    // Reset the retransmission flag when sending INVITE response
    if ((this->nType == TYPE_SERVER) && (this->nClass == CLASS_INVITE))
    {
        pTransport->InitRetransmissionFlag();
    }

    SipTxnKey* pTxnKey = IMS_NULL;
    IMS_UINT16 nError = 0;
    SIP_BOOL bStatus = SipStackManager::GetInstance()->SendMsg(pSipMsg, &objTranspParam,
            &objUserData, reinterpret_cast<SIP_CHAR*>(objBuffer.GetData()), objBuffer.GetLength(),
            &pTxnKey, &nError);

    if (objUserData.GetUserData() != IMS_NULL)
    {
        if (objUserData.GetDeleteFlag() == SIP_TRUE)
        {
            SipTxnContext* pOldTxnContext =
                    reinterpret_cast<SipTxnContext*>(objUserData.GetUserData());

            if (pOldTxnContext != pstTxnContext)
            {
                SIPStack::DestroyTxnContext(pOldTxnContext);
            }
        }

        SIPStack::DestroyTxnContext(pstTxnContext);
    }

    if (bStatus == SIP_FALSE)
    {
        IMS_TRACE_E(nError, "Sending SIP message to the transaction layer failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // SIP_PACKET_TRACKER
    if (pFactoryProxy->IsPacketTrackerEnabled(GetSlotId()))
    {
        SIPPacketTracker* pPacketTracker = pFactoryProxy->GetPacketTracker(GetSlotId());
        SIPMessage objSIPMsg(pSipMsg);

        pPacketTracker->NotifyMessageSent(&objSIPMsg, objBuffer, IMS_FALSE);
    }

    // SIP_IPSEC_STATE
    if (pFactoryProxy->IsIPSecStateEnabled(GetSlotId()))
    {
        SIPIPSecState* pIPSecState = pFactoryProxy->GetIPSecState(GetSlotId());
        const SIPTransportAddress& objNearEnd = pTransport->GetAddress(SIPTransport::TA_NEAR);
        const SIPTransportAddress& objFarEnd = pTransport->GetAddress(SIPTransport::TA_FAR);

        pIPSecState->NotifyMessageSent(objNearEnd, objFarEnd, pSipMsg);
    }

    // Store the transaction key here.
    if (pTxnKey != IMS_NULL)
    {
        if (SIPStack::IsMessageRPR(pSipMsg))
        {
            SIPStack::FreeTxnKey(pstRPRTxnKey);
            pstRPRTxnKey = pTxnKey;
        }
        else
        {
            SIPStack::FreeTxnKey(pstTxnKey);
            pstTxnKey = pTxnKey;
        }

        if (pstTxnKey->GetTxnType() == ETXN_INVSERTXN)
        {
            // If the method is INVITE, then store the txn key in the InvTxnKey.
            // This will be used when the application calls AbortCall().
            IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(pSipMsg);

            if (nStatusCode != SipStatusCode::SC_INVALID)
                pstTxnKey->SetRespCode(static_cast<SIP_UINT16>(nStatusCode));
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
void SIPTransactionState::SetTimerValues(
        IN SipTimerValues* pTV, IN_OUT SipTxnContext*& pstTxnContext)
{
    //---------------------------------------------------------------------------------------------

    SIPStack::SetTimerValues(pTV, pstTxnContext);
}

/*

Remarks

*/
PROTECTED
void SIPTransactionState::SetFlowControlOption(IN CONST SipMethod& objMethod)
{
    if (objMethod.Equals(SipMethod::REGISTER))
    {
        pTransport->SetTransactionFlowControlRequired(IMS_FALSE);
    }
    else
    {
        pTransport->SetTransactionFlowControlRequired(IMS_TRUE);
    }
}
