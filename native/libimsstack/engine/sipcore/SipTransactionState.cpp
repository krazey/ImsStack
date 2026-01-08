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
#include "AStringBuffer.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "SipStackManager.h"
#include "transport/SipTransportInfo.h"
#include "txn/SipTxn.h"

#include "ISipHeader.h"
#include "ISipTransactionStateListener.h"
#include "SipFactoryProxy.h"
#include "SipIpSecState.h"
#include "SipMessage.h"
#include "SipMessageInfo.h"
#include "SipMessageTracker.h"
#include "SipPacketTracker.h"
#include "SipPrivate.h"
#include "SipRtConfigUtils.h"
#include "SipStack.h"
#include "SipStackState.h"
#include "SipTransactionState.h"
#include "SipTransport.h"
#include "SipTxnContextData.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipTransactionState::SipTransactionState() :
        RcObject(),
        m_nType(0),
        m_nClass(CLASS_NONE),
        m_nCSeqNumber(1),
        m_pSipProfile(IMS_NULL),
        m_piListener(IMS_NULL),
        m_pDialogEx(IMS_NULL),
        m_pTransport(IMS_NULL),
        m_pSipMsg(IMS_NULL),
        m_pLastSipMsg(IMS_NULL),
        m_pTxnKey(IMS_NULL),
        m_pRprTxnKey(IMS_NULL)
{
}

PUBLIC
SipTransactionState::SipTransactionState(IN SipDialogEx* pDialogEx) :
        RcObject(),
        m_nType(0),
        m_nClass(CLASS_NONE),
        m_nCSeqNumber(1),
        m_pSipProfile(IMS_NULL),
        m_piListener(IMS_NULL),
        m_pDialogEx(pDialogEx),
        m_pTransport(IMS_NULL),
        m_pSipMsg(IMS_NULL),
        m_pLastSipMsg(IMS_NULL),
        m_pTxnKey(IMS_NULL),
        m_pRprTxnKey(IMS_NULL)
{
}

PUBLIC
SipTransactionState::SipTransactionState(IN const SipTransactionState& other) :
        RcObject(other),
        m_nType(other.m_nType),
        m_nClass(other.m_nClass),
        m_nCSeqNumber(other.m_nCSeqNumber),
        m_pSipProfile(other.m_pSipProfile),
        m_piListener(other.m_piListener),
        m_pDialogEx(other.m_pDialogEx),
        m_pTransport(IMS_NULL),
        m_pSipMsg(IMS_NULL),
        m_pLastSipMsg(IMS_NULL),
        m_pTxnKey(IMS_NULL),
        m_pRprTxnKey(IMS_NULL)
{
    // NOTE: If reference count is not used, you MUST implement this copy constructor
}

PUBLIC VIRTUAL SipTransactionState::~SipTransactionState()
{
    if (m_pTransport != IMS_NULL)
    {
        delete m_pTransport;
    }

    SipStack::FreeMessage(m_pLastSipMsg);
    SipStack::FreeMessage(m_pSipMsg);
    SipStack::FreeTxnKey(m_pTxnKey);
    SipStack::FreeTxnKey(m_pRprTxnKey);

    IMS_TRACE_D("Destructor :: SipTransactionState", 0, 0, 0);
}

PUBLIC VIRTUAL void SipTransactionState::Abort()
{
    ::SipTxnKey* pAbortTxnKey = m_pTxnKey;

    if ((pAbortTxnKey == IMS_NULL) && (m_nType == TYPE_CLIENT) && (m_pLastSipMsg != IMS_NULL))
    {
        pAbortTxnKey = SipStack::CreateTxnKey(m_pLastSipMsg, SipStack::SIP_TXN_MSG_SENT);
        m_pTxnKey = pAbortTxnKey;
    }

    SipFactoryProxy* pFactoryProxy = SipFactoryProxy::GetInstance();

    if (pFactoryProxy->IsIpSecStateEnabled(GetSlotId()))
    {
        SipIpSecState* pIpSecState = pFactoryProxy->GetIpSecState(GetSlotId());
        pIpSecState->NotifyTransactionAborted(m_pTxnKey);
    }

    // Invoke the stack API to abort the transaction.
    (void)SipStackState::GetInstance()->AbortTransaction(pAbortTxnKey, this);

    if (m_pRprTxnKey != IMS_NULL)
    {
        (void)SipStackState::GetInstance()->AbortTransaction(m_pRprTxnKey, this);
        SipStack::FreeTxnKey(m_pRprTxnKey);
    }

    SipStack::FreeTxnKey(m_pTxnKey);
}

/**
 * @brief Terminates SIP transaction if it's not available anymore.
 */
PUBLIC VIRTUAL void SipTransactionState::Terminate()
{
    IMS_TRACE_D("STS::Terminate", 0, 0, 0);

    // If there is ongoing transaction, it will be terminated promptly.
    (void)SipStackState::GetInstance()->AbortTransaction(m_pTxnKey, this);
}

PUBLIC VIRTUAL IMS_SINT32 SipTransactionState::CheckMessageValidity()
{
    return SipPrivate::MESSAGE_VALID;
}

PUBLIC VIRTUAL IMS_BOOL SipTransactionState::InitTxnDetails(IN const SipTransactionState* pTState)
{
    if (pTState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    m_nClass = pTState->m_nClass;
    m_nCSeqNumber = pTState->m_nCSeqNumber;
    m_pSipProfile = pTState->m_pSipProfile;
    m_pDialogEx = pTState->m_pDialogEx;

    if (!m_pTransport->InitTransportDetails(pTState->m_pTransport))
    {
        IMS_TRACE_E(0, "Initializing SIP transport details failed", 0, 0, 0);
        return IMS_FALSE;
    }

    SipStack::FreeMessage(m_pLastSipMsg);
    SipStack::FreeMessage(m_pSipMsg);
    SipStack::FreeTxnKey(m_pTxnKey);
    SipStack::FreeTxnKey(m_pRprTxnKey);

    m_pSipMsg = pTState->m_pSipMsg;
    SipStack::AddReference(m_pSipMsg);

    m_pLastSipMsg = pTState->m_pLastSipMsg;
    SipStack::AddReference(m_pLastSipMsg);

    m_pTxnKey = pTState->m_pTxnKey;
    SipStack::AddReference(m_pTxnKey);

    m_pRprTxnKey = pTState->m_pRprTxnKey;
    SipStack::AddReference(m_pRprTxnKey);

    return IMS_TRUE;
}

PUBLIC VIRTUAL void SipTransactionState::NotifyTimerExpired()
{
    if (m_piListener != IMS_NULL)
    {
        m_piListener->TransactionState_TimerExpired();
    }
}

/**
 * @brief It's invoked by SIP stack after sending SIP message successfully.
 */
PUBLIC VIRTUAL void SipTransactionState::PostProcessMessageSentByStack(
        IN ::SipMessage* pSipMsg, IN const ByteArray& objBuffer)
{
    SipFactoryProxy* pFactoryProxy = SipFactoryProxy::GetInstance();

    // SIP_PACKET_TRACKER
    if (pFactoryProxy->IsPacketTrackerEnabled(GetSlotId()))
    {
        SipPacketTracker* pPacketTracker = pFactoryProxy->GetPacketTracker(GetSlotId());
        sipcore::SipMessage objMessage(pSipMsg);

        pPacketTracker->NotifyMessageSent(&objMessage, objBuffer, IMS_FALSE);
    }

    // SIP_IPSEC_STATE
    if (pFactoryProxy->IsIpSecStateEnabled(GetSlotId()))
    {
        SipIpSecState* pIpSecState = pFactoryProxy->GetIpSecState(GetSlotId());
        const SipTransportAddress& objNearEnd = m_pTransport->GetAddress(SipTransport::TA_NEAR);
        const SipTransportAddress& objFarEnd = m_pTransport->GetAddress(SipTransport::TA_FAR);

        pIpSecState->NotifyMessageSent(objNearEnd, objFarEnd, pSipMsg);
    }
}

/**
 * @brief It's invoked by SIP stack before sending SIP message.
 */
PUBLIC VIRTUAL void SipTransactionState::PreProcessMessageSentByStack(IN ::SipMessage* pSipMsg)
{
    SipFactoryProxy* pFactoryProxy = SipFactoryProxy::GetInstance();

    if (pFactoryProxy->IsMessageTrackerEnabled(GetSlotId()))
    {
        SipMessageTracker* pMessageTracker = pFactoryProxy->GetMessageTracker(GetSlotId());
        const SipMethod objMethod = SipStack::GetMethod(pSipMsg);

        if (SipStack::IsRequestMessage(pSipMsg))
        {
            pMessageTracker->NotifyMessageSent(
                    objMethod, 0, m_pDialogEx->GetDialogState()->GetCallId());
        }
        else
        {
            pMessageTracker->NotifyMessageSent(objMethod, SipStack::GetStatusCode(pSipMsg),
                    m_pDialogEx->GetDialogState()->GetCallId());
        }
    }

    // LOG_EXCLUDING_SERVER_INFO
    if (SipStack::IsRequestMessage(pSipMsg) && SipRtConfigUtils::IsMessageHiddenInLog(GetSlotId()))
    {
        SipStack::DisplayUnknownHeaders(pSipMsg);
    }

    // Reset the retransmission flag when sending INVITE response
    if ((m_nType == TYPE_SERVER) && (m_nClass == CLASS_INVITE))
    {
        m_pTransport->InitRetransmissionFlag();
    }
}

PUBLIC VIRTUAL IMS_BOOL SipTransactionState::Send(IN SipTimerValues* pTimerValues /*= IMS_NULL*/)
{
    return Send(m_pSipMsg, pTimerValues);
}

PUBLIC VIRTUAL IMS_RESULT SipTransactionState::RetransmitMessage()
{
    ByteArray objBuffer;

    if (m_pTransport == IMS_NULL)
    {
        IMS_TRACE_E(0, "Transport is already destroyed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!m_pTransport->EncodeMessage(m_pSipMsg, objBuffer))
    {
        IMS_TRACE_E(0, "Encoding ACK or 2xx to INVITE request (Retransmission) failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!m_pTransport->SendToNetwork(
                objBuffer.GetData(), objBuffer.GetLength(), GetSipProfile(), IMS_FALSE))
    {
        IMS_TRACE_E(0, "Retransmitting ACK or 2xx to INVITE request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    SipFactoryProxy* pFactoryProxy = SipFactoryProxy::GetInstance();

    if (pFactoryProxy->IsPacketTrackerEnabled(GetSlotId()))
    {
        SipPacketTracker* pPacketTracker = pFactoryProxy->GetPacketTracker(GetSlotId());
        sipcore::SipMessage objMessage(m_pSipMsg);

        pPacketTracker->NotifyMessageSent(&objMessage, objBuffer, IMS_TRUE);
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_BOOL SipTransactionState::UpdateTransportDetails()
{
    if (!m_pTransport->UpdateDestinationInfo(m_pSipMsg, GetSipProfile()))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 SipTransactionState::GetSlotId() const
{
    return m_pTransport->GetSlotId();
}

PUBLIC
IMS_BOOL SipTransactionState::IsIpSecRequired() const
{
    return m_pTransport->IsIpSecRequired();
}

PUBLIC
void SipTransactionState::SetTransportListener(IN ISipTransportListener* piListener)
{
    m_pTransport->SetListener(piListener);
}

// RFC5626_FLOW_CONTROL, MULTI_REG_TRANSPORT
PUBLIC
void SipTransactionState::SetTransportTuple(IN const IpAddress& objIp, IN IMS_SINT32 nPortS,
        IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFc /*= Sip::PORT_UNSPECIFIED*/,
        IN IMS_SINT32 nTransportExt /*= Sip::TRANSPORT_EXT_ANY*/)
{
    m_pTransport->SetTransportTuple(objIp, nPortS, nPortC, nPortFc, nTransportExt);
}

PUBLIC
IMS_BOOL SipTransactionState::SendToNetwork(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen)
{
    if (m_pTransport == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pTransport->SendToNetwork(pBuffer, nBuffLen, GetSipProfile());
}

PUBLIC
void SipTransactionState::UpdateMessage(IN ::SipMessage* pSipMsg)
{
    SipStack::FreeMessage(m_pSipMsg);

    m_pSipMsg = pSipMsg;
    SipStack::AddReference(m_pSipMsg);
}

PROTECTED VIRTUAL SipTransactionState* SipTransactionState::Clone()
{
    // The subclass MUST implement this method to create a new SipTransactionState (client/server)
    return IMS_NULL;
}

PROTECTED
IMS_BOOL SipTransactionState::Send(IN ::SipMessage* pSipMsg, IN const SipTimerValues* pTimerValues)
{
    IMS_TRACE_D("Send", 0, 0, 0);

    IMS_SINT32 nType = m_pTransport->GetProtocol(SipTransport::TA_FAR);
    SipTransportParameter objTranspParam;

    if (nType == SipTransportAddress::PROTOCOL_TCP)
    {
        objTranspParam.SetTranspProtocol(SipTransportInfo::PROTOCOL_TCP);
    }
    else if (nType == SipTransportAddress::PROTOCOL_TLS)
    {
        objTranspParam.SetTranspProtocol(SipTransportInfo::PROTOCOL_TLS);
    }
    else
    {
        objTranspParam.SetTranspProtocol(SipTransportInfo::PROTOCOL_UDP);
    }

    const SipTransportAddress& objTAddr = m_pTransport->GetAddress(SipTransport::TA_FAR);

    objTranspParam.SetHostAddress(objTAddr.GetIpAddress().ToString().GetStr());
    objTranspParam.SetPort((IMS_UINT16)objTAddr.GetPort());

    if (objTAddr.GetIpAddress().IsIPv4Address() == IMS_TRUE)
    {
        objTranspParam.SetTanspIpType(SipTransportInfo::NETWORK_IPV4);
    }
    else
    {
        objTranspParam.SetTanspIpType(SipTransportInfo::NETWORK_IPV6);
    }

    // Prepare User Data
    SipTxnContext* pTxnContext = SipStack::CreateTxnContext();

    if (pTxnContext == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SetTimerValues(pTimerValues, pTxnContext);

    SipTxnContextData* pTxnContextData = new SipTxnContextData();

    if (pTxnContextData != IMS_NULL)
    {
        IMS_BOOL bIsTxnStateSet = IMS_FALSE;

        // FIX_TXN_HANDLING_ON_401_407_TO_INVITE
        if ((m_nType == TYPE_CLIENT) && (m_nClass == CLASS_INVITE))
        {
            IMS_SINT32 nStatusCode = SipStack::GetStatusCode(pSipMsg);

            if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
            {
                const SipMethod objMethod = SipStack::GetMethod(pSipMsg);

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

    pTxnContext->m_pTxnContextData = static_cast<SIP_VOID*>(pTxnContextData);

    ISipUserData objUserData;
    objUserData.SetUserData(static_cast<SIP_VOID*>(pTxnContext));

    ByteArray objBuffer;

    // Form a raw SIP message
    if (!m_pTransport->EncodeMessage(pSipMsg, objBuffer))
    {
        SipStack::DestroyTxnContext(pTxnContext);
        IMS_TRACE_E(0, "Encoding SIP message failed", 0, 0, 0);
        return IMS_FALSE;  // throw exception : syntax error
    }

    // Reserver transport resources
    if (!m_pTransport->ReserveResource(GetSipProfile()))
    {
        SipStack::DestroyTxnContext(pTxnContext);
        IMS_TRACE_E(0, "Reserving the transport resource failed", 0, 0, 0);
        return IMS_FALSE;  // throw exception : network not available
    }

    const SipMethod objMethod = SipStack::GetMethod(pSipMsg);
    SipMessageInfo objMsgInfo(GetSlotId(), objMethod, pSipMsg, SipMessageInfo::DIRECTION_OUTGOING);
    LogSipMessageInfo(objMsgInfo);

    SipFactoryProxy* pFactoryProxy = SipFactoryProxy::GetInstance();

    if (pFactoryProxy->IsMessageTrackerEnabled(GetSlotId()))
    {
        SipMessageTracker* pMessageTracker = pFactoryProxy->GetMessageTracker(GetSlotId());

        if (SipStack::IsRequestMessage(pSipMsg))
        {
            pMessageTracker->NotifyMessageSent(
                    objMethod, 0, m_pDialogEx->GetDialogState()->GetCallId());
        }
        else
        {
            pMessageTracker->NotifyMessageSent(objMethod, SipStack::GetStatusCode(pSipMsg),
                    m_pDialogEx->GetDialogState()->GetCallId());
        }
    }

    // LOG_EXCLUDING_SERVER_INFO
    if (SipStack::IsRequestMessage(pSipMsg) && SipRtConfigUtils::IsMessageHiddenInLog(GetSlotId()))
    {
        SipStack::DisplayUnknownHeaders(pSipMsg);
    }

    // Reset the retransmission flag when sending INVITE response
    if ((m_nType == TYPE_SERVER) && (m_nClass == CLASS_INVITE))
    {
        m_pTransport->InitRetransmissionFlag();
    }

    ::SipTxnKey* pTxnKey = IMS_NULL;
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

            if (pOldTxnContext != pTxnContext)
            {
                SipStack::DestroyTxnContext(pOldTxnContext);
            }
        }

        SipStack::DestroyTxnContext(pTxnContext);
    }

    if (bStatus == SIP_FALSE)
    {
        IMS_TRACE_E(nError, "Sending SIP message to the transaction layer failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pFactoryProxy->IsPacketTrackerEnabled(GetSlotId()))
    {
        SipPacketTracker* pPacketTracker = pFactoryProxy->GetPacketTracker(GetSlotId());
        sipcore::SipMessage objMessage(pSipMsg);

        pPacketTracker->NotifyMessageSent(&objMessage, objBuffer, IMS_FALSE);
    }

    if (pFactoryProxy->IsIpSecStateEnabled(GetSlotId()))
    {
        SipIpSecState* pIpSecState = pFactoryProxy->GetIpSecState(GetSlotId());
        const SipTransportAddress& objNearEnd = m_pTransport->GetAddress(SipTransport::TA_NEAR);
        const SipTransportAddress& objFarEnd = m_pTransport->GetAddress(SipTransport::TA_FAR);

        pIpSecState->NotifyMessageSent(objNearEnd, objFarEnd, pSipMsg);
    }

    // Store the transaction key here.
    if (pTxnKey != IMS_NULL)
    {
        if (SipStack::IsMessageRpr(pSipMsg))
        {
            SipStack::FreeTxnKey(m_pRprTxnKey);
            m_pRprTxnKey = pTxnKey;
        }
        else
        {
            SipStack::FreeTxnKey(m_pTxnKey);
            m_pTxnKey = pTxnKey;
        }

        if (m_pTxnKey->GetTxnType() == SipTxn::INVITE_SERVER)
        {
            // If the method is INVITE, then store the txn key in the InvTxnKey.
            // This will be used when the application calls AbortCall().
            IMS_SINT32 nStatusCode = SipStack::GetStatusCode(pSipMsg);

            if (nStatusCode != SipStatusCode::SC_INVALID)
            {
                m_pTxnKey->SetResponseCode(static_cast<SIP_UINT16>(nStatusCode));
            }
        }
    }

    return IMS_TRUE;
}

PROTECTED
void SipTransactionState::SetTimerValues(
        IN const SipTimerValues* pTimerValues, IN_OUT SipTxnContext*& pTxnContext)
{
    SipStack::SetTimerValues(pTimerValues, pTxnContext);
}

PROTECTED
void SipTransactionState::SetFlowControlOption(IN const SipMethod& objMethod)
{
    if (objMethod.Equals(SipMethod::REGISTER))
    {
        m_pTransport->SetTransactionFlowControlRequired(IMS_FALSE);
    }
    else
    {
        m_pTransport->SetTransactionFlowControlRequired(IMS_TRUE);
    }
}

PROTECTED GLOBAL void SipTransactionState::LogSipMessageInfo(IN const SipMessageInfo& objMsgInfo)
{
    AStringBuffer objBuffer(256);

    objBuffer.Append("{ ");
    objBuffer.Append(objMsgInfo.IsOutgoingMessage() ? "OUT, " : "IN, ");

    AString strTemp;
    ::SipMessage* pSipMsg = objMsgInfo.GetMessage();
    // SIP request
    if (pSipMsg->GetMsgType() == ::SipMessage::REQ_TYPE)
    {
        objBuffer.Append(objMsgInfo.GetMethod().ToString());
        objBuffer.Append(", ");

        strTemp = AString::ConstNull();
        SipAddrSpec* pAddrSpec = SipStack::GetRequestUri(pSipMsg);
        SipStack::EncodeAddrSpec(pAddrSpec, IMS_FALSE, strTemp);
        SipStack::FreeAddrSpec(pAddrSpec);
        // URI scheme + 3 characters only
        strTemp.Replace(7, strTemp.GetLength(), "***");
        objBuffer.Append(strTemp);
    }
    // SIP response
    else
    {
        strTemp = AString::ConstNull();
        SipStatusCode objStatusCode = SipStack::GetStatusCodeEx(pSipMsg);
        strTemp.Sprintf("%d %s", objStatusCode.ToInt(), objStatusCode.GetReasonPhrase().GetStr());
        objBuffer.Append(strTemp);
    }

    objBuffer.Append(", ");

    // Include a general and mandatory SIP header information.
    strTemp = SipStack::GetHeaderAsString(pSipMsg, ISipHeader::CSEQ);
    objBuffer.Append("cseq: ");
    objBuffer.Append(strTemp);
    objBuffer.Append(", ");

    strTemp = SipStack::GetHeaderAsString(pSipMsg, ISipHeader::CALL_ID);
    strTemp = strTemp.GetSubStr(0, strTemp.GetIndexOf('@'));

    // Strip the IPv6 address or port number if present.
    IMS_SINT32 nIndex = strTemp.GetIndexOf(':');

    if (nIndex != AString::NPOS)
    {
        strTemp = strTemp.GetSubStr(0, nIndex);
    }

    // Strip the IPv4 address if present.
    nIndex = strTemp.GetIndexOf('.');

    if (nIndex != AString::NPOS)
    {
        strTemp = strTemp.GetSubStr(0, nIndex);
    }

    objBuffer.Append("call-id: ");
    objBuffer.Append(strTemp);
    objBuffer.Append(", ");

    strTemp = SipStack::GetViaBranchParameter(pSipMsg);
    objBuffer.Append("via-branch: ");
    objBuffer.Append(strTemp.GetSubStr(0, 32));
    objBuffer.Append(", ");

    // Include an extra information for the specific SIP messages
    if (pSipMsg->GetMsgType() == ::SipMessage::REQ_TYPE &&
            objMsgInfo.GetMethod().Equals(SipMethod::PRACK))
    {
        strTemp = SipStack::GetHeaderAsString(pSipMsg, ISipHeader::RACK);
        objBuffer.Append("rack: ");
        objBuffer.Append(strTemp);
        objBuffer.Append(", ");
    }
    else if (SipStack::IsMessageRpr(pSipMsg))
    {
        strTemp = SipStack::GetHeaderAsString(pSipMsg, ISipHeader::RSEQ);
        objBuffer.Append("rseq: ");
        objBuffer.Append(strTemp);
        objBuffer.Append(", ");
    }

    objBuffer.Append("}");

    IMS_TRACE_I("SIPMSG[%d]=%s", objMsgInfo.GetSlotId(),
            static_cast<const AStringBuffer&>(objBuffer).GetString().GetStr(), 0);
}
