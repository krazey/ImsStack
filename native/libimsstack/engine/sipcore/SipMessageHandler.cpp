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
#include "ByteArray.h"
#include "ServiceMemory.h"
#include "ServiceNetwork.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"

#include "ISipHeader.h"
#include "ISipServerTransactionStateListener.h"
#include "SipClientTransactionState.h"
#include "SipConfigProxy.h"
#include "SipConnectionNotifier.h"
#include "SipDebug.h"
#include "SipFactoryProxy.h"
#include "SipManager.h"
#include "SipMessage.h"
#include "SipMessageHandler.h"
#include "SipPacketTracker.h"
#include "SipPrivate.h"
#include "SipRoutingRejectNotifier.h"
#include "SipRtConfigHelper.h"
#include "SipRtConfigUtils.h"
#include "SipServerTransactionState.h"
#include "SipStack.h"
#include "SipTransport.h"
#include "SipWakeLock.h"

__IMS_TRACE_TAG_SIP_CORE__;

PRIVATE
SipMessageHandler::SipMessageHandler() {}

PUBLIC
SipMessageHandler::~SipMessageHandler() {}

PUBLIC GLOBAL SipMessageHandler* SipMessageHandler::GetInstance()
{
    static SipMessageHandler* s_pMessageHandler = IMS_NULL;

    if (s_pMessageHandler == IMS_NULL)
    {
        s_pMessageHandler = new SipMessageHandler();
    }

    return s_pMessageHandler;
}

PRIVATE VIRTUAL void SipMessageHandler::TransportMessage_PacketReceived(IN IMS_SINT32 nSlotId,
        IN const ByteArray& objBuffer, IN const SipTransportAddress& objNearEnd,
        IN const SipTransportAddress& objFarEnd)
{
    ::SipMessage* pSipMsg = IMS_NULL;

    // 3 Updates the operations in case of an exceptional operation

    // No need to call DecodeMessage() repeatedly till the entire buffer is parsed.
    // We do not take care of the situation where the multiple messages arrive
    // on a single buffer (as in the case of TCP).
    // Because, the checking of message completion will be evaluated in the SIP socket layer.

    if (!SipStack::DecodeMessage(
                objBuffer.GetData(), objBuffer.GetLength(), SipPrivate::OPTIONS_D, pSipMsg))
    {
        SipStack::FreeMessage(pSipMsg);

        IMS_TRACE_E(SipStack::GetLastError(),
                "DECODING FAILURE: Incoming packet MAY be a malformed SIP message", 0, 0, 0);
        return;
    }

    SipStack::DisplayUnknownHeaders(pSipMsg);

    if (SipStack::GetBadHeaderCount(pSipMsg) > 0)
    {
        SipStack::DisplayBadHeaders(pSipMsg);

        if (!SipStack::HasMandatoryHeaders(pSipMsg))
        {
            SipStack::FreeMessage(pSipMsg);

            IMS_TRACE_E(SipStack::GetLastError(),
                    "DECODING FAILURE: Malformed SIP message - mandatory headers", 0, 0, 0);
            return;
        }
    }

    // SIP_PACKET_TRACKER
    NotifyPacketReceived(nSlotId, objBuffer, pSipMsg, SipPrivate::MESSAGE_VALID);

    IMS_SINT32 nValidity;

    if (SipStack::IsRequestMessage(pSipMsg))
    {
        if (SipWakeLock::IsSupported())
        {
            SipMethod objMethod = SipStack::GetMethod(pSipMsg);
            SipWakeLock::Acquire(objMethod);
        }

        nValidity = NotifyRequest(nSlotId, pSipMsg, objNearEnd, objFarEnd);
    }
    else
    {
        // REG-CONTACT-VALIDATION
        if (SipRtConfigUtils::IsRegContactAddressConfigured(nSlotId))
        {
            if (!CheckRegContactValidity(nSlotId, pSipMsg))
            {
                IMS_TRACE_D("Ignore REGISTER response (reg-contact-mismatch)", 0, 0, 0);

                SipStack::FreeMessage(pSipMsg);
                return;
            }
        }

        nValidity = NotifyResponse(nSlotId, pSipMsg, objNearEnd, objFarEnd);
    }

    if (nValidity == SipPrivate::MESSAGE_FAILED)
    {
        IMS_TRACE_I("___ PROCESSING %s MESSAGE FAILED",
                SipStack::IsRequestMessage(pSipMsg) ? "REQUEST" : "RESPONSE", 0, 0);
    }

    SipStack::FreeMessage(pSipMsg);
}

PRIVATE
void SipMessageHandler::NotifyPacketReceived(IN IMS_SINT32 nSlotId, IN const ByteArray& objBuffer,
        IN ::SipMessage* pSipMsg, IN IMS_SINT32 nProcessingResult)
{
    SipFactoryProxy* pFactoryProxy = SipFactoryProxy::GetInstance();

    if (pFactoryProxy->IsPacketTrackerEnabled(nSlotId))
    {
        SipPacketTracker* pPacketTracker = pFactoryProxy->GetPacketTracker(nSlotId);
        sipcore::SipMessage objMessage(pSipMsg);

        pPacketTracker->NotifyMessageReceived(&objMessage, objBuffer,
                ((nProcessingResult == SipPrivate::MESSAGE_DISCARDED) ? IMS_TRUE : IMS_FALSE));
    }
}

PRIVATE
IMS_SINT32 SipMessageHandler::NotifyRequest(IN IMS_SINT32 nSlotId, IN ::SipMessage* pSipMsg,
        IN const SipTransportAddress& objNearEnd, IN const SipTransportAddress& objFarEnd)
{
    ISipServerTransactionStateListener* piListener =
            SipManager::GetInstance()->LookupConnectionNotifier(objNearEnd);

    // UE_TCP_CONNECTION_REUSED -- starts
    // When the transport protocol is TCP and the transaction re-use the TCP connection
    // which is created by UE, check the Request-URI again for port number.
    if ((piListener == IMS_NULL) &&
            ((objNearEnd.GetProtocol() == SipTransportAddress::PROTOCOL_TCP) ||
                    (objNearEnd.GetProtocol() == SipTransportAddress::PROTOCOL_TLS)))
    {
        // Gets the port number from the Request-URI
        SipAddrSpec* pAddrSpec = SipStack::GetRequestUri(pSipMsg);

        if (pAddrSpec != IMS_NULL)
        {
            AString strHost;
            IMS_UINT32 nPort = Sip::PORT_UNSPECIFIED;

            if (!SipStack::GetHostAndPort(pAddrSpec, strHost, nPort))
            {
                if (SipStack::IsLastErrorNoExist())
                {
                    nPort = SipConfigProxy::GetPort(nSlotId);
                }
            }
            else
            {
                // NO_EXIST (for port only)
                if ((nPort == Sip::PORT_UNSPECIFIED) && SipStack::IsLastErrorNoExist())
                {
                    nPort = SipConfigProxy::GetPort(nSlotId);
                    IMS_TRACE_D("ConnectionNotifier: port(%d) from config.", nPort, 0, 0);
                }
            }

            if (nPort != Sip::PORT_UNSPECIFIED)
            {
                SipTransportAddress objTAddr = objNearEnd;

                objTAddr.SetPort(nPort);

                piListener = SipManager::GetInstance()->LookupConnectionNotifier(objTAddr);

                if (piListener != IMS_NULL)
                {
                    IMS_TRACE_D("ConnectionNotifier found by (%s|%d)",
                            SipDebug::GetIp(objTAddr.GetIpAddress()), objTAddr.GetPort(), 0);
                }
            }

            SipStack::FreeAddrSpec(pAddrSpec);
        }
    }
    // UE_TCP_CONNECTION_REUSED -- ends

    if (SystemConfig::IsMultiSimEnabled())
    {
        IMS_TRACE_D("Incoming SIP request on slot%d", nSlotId, 0, 0);
    }

    RcPtr<SipServerTransactionState> pStState =
            new SipServerTransactionState(nSlotId, objNearEnd, objFarEnd);

    if (pStState.IsNull())
    {
        // Discard the request
        return SipPrivate::MESSAGE_FAILED;
    }

    IMS_SINT32 nValidity = pStState->MatchTransaction(pSipMsg);
    IMS_BOOL bRejectRequest = IMS_FALSE;

    if (nValidity != SipPrivate::MESSAGE_VALID)
    {
        bRejectRequest = IMS_TRUE;
    }
    else if (SipStack::GetBadHeaderCount(pSipMsg) > 0)
    {
        bRejectRequest = IMS_TRUE;
        nValidity = SipPrivate::MESSAGE_INVALID_400;
    }

    if (bRejectRequest)
    {
        IMS_SINT32 nResult = SipPrivate::MESSAGE_DISCARDED;

        // Send failure response if needs
        if ((nValidity == SipPrivate::MESSAGE_INVALID_400) ||
                (nValidity == SipPrivate::MESSAGE_INVALID_405) ||
                (nValidity == SipPrivate::MESSAGE_INVALID_481))
        {
            // Update the transport information
            if (piListener != IMS_NULL)
            {
                piListener->ServerTransactionState_RequestCreated(pStState.Get());
            }
            else
            {
                pStState->SetTransportTuple(
                        objNearEnd.GetIpAddress(), objNearEnd.GetPort(), objNearEnd.GetPort());
            }

            // To send failure response
            pStState->CheckMessageValidity();

            RcPtr<SipDialogEx> pOrigDialogEx;
            nResult = pStState->HandleRequest(pOrigDialogEx);
        }

        if ((nResult == SipPrivate::MESSAGE_VALID) || (nResult == SipPrivate::MESSAGE_VALID_FORKED))
        {
            if (nValidity == SipPrivate::MESSAGE_INVALID_481)
            {
                pStState->RejectRequest(SipStatusCode::SC_481);
            }
            else if (nValidity == SipPrivate::MESSAGE_INVALID_400)
            {
                pStState->RejectRequest(SipStatusCode::SC_400);
            }
            else if (nValidity == SipPrivate::MESSAGE_INVALID_405)
            {
                pStState->RejectRequest(SipStatusCode::SC_405);
            }
        }

        pStState->Abort();
        return nValidity;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_D("NO LISTENER - MESSAGE DISCARDED", 0, 0, 0);

        // Update the transport information
        pStState->SetTransportTuple(
                objNearEnd.GetIpAddress(), objNearEnd.GetPort(), objNearEnd.GetPort());

        // Send failure response
        pStState->CheckMessageValidity();

        RcPtr<SipDialogEx> pOrigDialogEx;
        IMS_SINT32 nResult = pStState->HandleRequest(pOrigDialogEx);

        if ((nResult == SipPrivate::MESSAGE_VALID) || (nResult == SipPrivate::MESSAGE_VALID_FORKED))
        {
            SipStatusCode objStatusCode(SipStatusCode::SC_404);
            SipFactoryProxy* pFactoryProxy = SipFactoryProxy::GetInstance();

            if (pFactoryProxy->IsRoutingRejectNotifierEnabled(nSlotId))
            {
                SipRoutingRejectNotifier* pRoutingRejectNotifier =
                        pFactoryProxy->GetRoutingRejectNotifier(nSlotId);
                sipcore::SipMessage objMessage(pSipMsg);

                pRoutingRejectNotifier->NotifyRequestReject(&objMessage, objStatusCode);

                if (objStatusCode != SipStatusCode::SC_404)
                {
                    IMS_TRACE_D("SipRoutingReject: Status code is overwritten (404 >> %d)",
                            objStatusCode.ToInt(), 0, 0);
                }
            }

            pStState->RejectRequest(objStatusCode.ToInt(), objStatusCode.GetReasonPhrase());
        }

        pStState->Abort();

        return SipPrivate::MESSAGE_FAILED;
    }

    // After creating the transaction, we need to update some informations
    // related to this transaction.
    piListener->ServerTransactionState_RequestCreated(pStState.Get());

    nValidity = pStState->CheckMessageValidity();

    if (nValidity != SipPrivate::MESSAGE_VALID)
    {
        RcPtr<SipDialogEx> pOrigDialogEx;
        IMS_SINT32 nResult = pStState->HandleRequest(pOrigDialogEx);

        if ((nResult == SipPrivate::MESSAGE_VALID) || (nResult == SipPrivate::MESSAGE_VALID_FORKED))
        {
            SipStatusCode objStatusCode(SipStatusCode::SC_400);
            SipFactoryProxy* pFactoryProxy = SipFactoryProxy::GetInstance();

            if (pFactoryProxy->IsRoutingRejectNotifierEnabled(nSlotId))
            {
                SipRoutingRejectNotifier* pRoutingRejectNotifier =
                        pFactoryProxy->GetRoutingRejectNotifier(nSlotId);
                sipcore::SipMessage objMessage(pSipMsg);

                pRoutingRejectNotifier->NotifyRequestReject(&objMessage, objStatusCode);

                if (objStatusCode != SipStatusCode::SC_400)
                {
                    IMS_TRACE_D("SipRoutingReject: Status code is overwritten (400 >> %d)",
                            objStatusCode.ToInt(), 0, 0);
                }
            }

            pStState->RejectRequest(objStatusCode.ToInt(), objStatusCode.GetReasonPhrase());
        }

        pStState->Abort();

        return nValidity;
    }

    // If IPSec is applied, check the validity of source IP and port.
    if (!CheckIpSecValidityForRequest(nSlotId, pStState.Get(), objNearEnd, objFarEnd))
    {
        // Discard the incoming request.
        IMS_TRACE_D("IPSEC: Discarded", 0, 0, 0);
        pStState->Abort();
        return SipPrivate::MESSAGE_DISCARDED;
    }

    RcPtr<SipDialogEx> pOrigDialogEx;

    nValidity = pStState->HandleRequest(pOrigDialogEx);

    if (nValidity == SipPrivate::MESSAGE_VALID)
    {
        piListener->ServerTransactionState_RequestReceived(pStState.Get());
    }
    else if (nValidity == SipPrivate::MESSAGE_VALID_FORKED)
    {
        piListener->ServerTransactionState_ForkedRequestReceived(
                pStState.Get(), pOrigDialogEx.Get());
    }
    else
    {
        pStState->Abort();
        return nValidity;
    }

    return SipPrivate::MESSAGE_VALID;
}

PRIVATE
IMS_SINT32 SipMessageHandler::NotifyResponse(IN IMS_SINT32 nSlotId, IN ::SipMessage* pSipMsg,
        IN const SipTransportAddress& objNearEnd, IN const SipTransportAddress& objFarEnd)
{
    RcPtr<SipClientTransactionState> pCtState;

    // If IPSec is applied, check the validity of source IP and port.
    if (!CheckIpSecValidityForResponse(nSlotId, pSipMsg, objNearEnd, objFarEnd))
    {
        // Discard the incoming response.
        IMS_TRACE_D("IPSEC: Discarded", 0, 0, 0);
        return SipPrivate::MESSAGE_DISCARDED;
    }

    IMS_SINT32 nValidity =
            SipClientTransactionState::MatchTransaction(pSipMsg, objFarEnd, pCtState);

    if (nValidity != SipPrivate::MESSAGE_VALID)
    {
        return nValidity;
    }

    nValidity = pCtState->HandleResponse(pSipMsg);

    if (nValidity != SipPrivate::MESSAGE_VALID)
    {
        return nValidity;
    }

    return SipPrivate::MESSAGE_VALID;
}

PRIVATE
IMS_BOOL SipMessageHandler::CheckIpSecValidityForRequest(IN IMS_SINT32 nSlotId,
        IN const SipTransactionState* pTState, IN const SipTransportAddress& objNearEnd,
        IN const SipTransportAddress& objFarEnd)
{
    if (pTState->IsIpSecRequired() && SipRtConfigUtils::IsIpSecSaConfigured(nSlotId))
    {
        if (!IsIpSecSaMatched(nSlotId, objNearEnd, objFarEnd))
        {
            IMS_TRACE_D("IPSEC: SIP request is dropped by non-SA", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SipMessageHandler::CheckIpSecValidityForResponse(IN IMS_SINT32 nSlotId,
        IN ::SipMessage* pSipMsg, IN const SipTransportAddress& objNearEnd,
        IN const SipTransportAddress& objFarEnd)
{
    if (SipRtConfigUtils::IsIpSecSaConfigured(nSlotId) && IsSecuredMessage(nSlotId, pSipMsg))
    {
        if (!IsIpSecSaMatched(nSlotId, objNearEnd, objFarEnd))
        {
            IMS_TRACE_D("IPSEC: SIP response is dropped by non-SA", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SipMessageHandler::IsIpSecSaMatched(IN IMS_SINT32 nSlotId,
        IN const SipTransportAddress& objNearEnd, IN const SipTransportAddress& objFarEnd)
{
    const SipRtConfigHelper* pConfigHelper = SipRtConfigUtils::GetConfigHelper(nSlotId);
    IMS_BOOL bAtLeastOneSaMatched = IMS_FALSE;
    const ImsList<SipRtConfig::IpSecSa>& objIpSecSas = pConfigHelper->GetIpSecSas();

    for (IMS_UINT32 i = 0; i < objIpSecSas.GetSize(); ++i)
    {
        const SipRtConfig::IpSecSa& objIpSecSa = objIpSecSas.GetAt(i);

        if (objIpSecSa.IsEmpty())
        {
            continue;
        }

        if ((objIpSecSa.GetPortPc() != objFarEnd.GetPort()) &&
                (objIpSecSa.GetPortPs() != objFarEnd.GetPort()))
        {
            IMS_TRACE_I("IPSEC: PortP mismatched: pc(%d), ps(%d), far(%d)", objIpSecSa.GetPortPc(),
                    objIpSecSa.GetPortPs(), objFarEnd.GetPort());
            continue;
        }

        if (!objIpSecSa.GetIpAddrP().Equals(objFarEnd.GetIpAddress()))
        {
            if (pConfigHelper->IsRoutingInfoHiddenInLog())
            {
                IMS_TRACE_I("IPSEC: IPP mismatched", 0, 0, 0);
            }
            else
            {
                IMS_TRACE_I("IPSEC: IPP mismatched: ipp(%s), far(%s)",
                        SipDebug::GetStr1(objIpSecSa.GetIpAddrP().ToString(), 5).GetStr(),
                        SipDebug::GetIp(objFarEnd.GetIpAddress()), 0);
            }
            continue;
        }

        if ((objIpSecSa.GetPortUc() != objNearEnd.GetPort()) &&
                (objIpSecSa.GetPortUs() != objNearEnd.GetPort()))
        {
            IMS_TRACE_I("IPSEC: PortU mismatched: uc(%d), us(%d), near(%d)", objIpSecSa.GetPortPc(),
                    objIpSecSa.GetPortPs(), objNearEnd.GetPort());
            continue;
        }

        if (!bAtLeastOneSaMatched)
        {
            bAtLeastOneSaMatched = IMS_TRUE;
            break;
        }
    }

    return bAtLeastOneSaMatched;
}

PRIVATE
IMS_BOOL SipMessageHandler::IsIpSecSaMatchedForUs(
        IN IMS_SINT32 nSlotId, IN const IpAddress& objIp, IN IMS_SINT32 nPort)
{
    const SipRtConfigHelper* pConfigHelper = SipRtConfigUtils::GetConfigHelper(nSlotId);
    const ImsList<SipRtConfig::IpSecSa>& objIpSecSas = pConfigHelper->GetIpSecSas();

    for (IMS_UINT32 i = 0; i < objIpSecSas.GetSize(); ++i)
    {
        const SipRtConfig::IpSecSa& objIpSecSa = objIpSecSas.GetAt(i);

        if (objIpSecSa.IsEmpty())
        {
            continue;
        }

        if ((objIpSecSa.GetPortUs() == nPort) && objIpSecSa.GetIpAddrU().Equals(objIp))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL SipMessageHandler::IsSecuredMessage(IN IMS_SINT32 nSlotId, IN ::SipMessage* pSipMsg)
{
    IMS_SINT32 nPort = Sip::PORT_UNSPECIFIED;
    AString strHost;

    if (!SipTransport::GetHostNPortFromViaHeader(pSipMsg, strHost, nPort))
    {
        return IMS_FALSE;
    }

    if (SipStack::IsRequestMessage(pSipMsg))
    {
        // It can be identified by SIP transaction state.
    }
    else
    {
        IpAddress objIp(strHost);

        if (IsIpSecSaMatchedForUs(nSlotId, objIp, nPort))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL SipMessageHandler::CheckRegContactValidity(IN IMS_SINT32 nSlotId, IN ::SipMessage* pSipMsg)
{
    SipMethod objMethod = SipStack::GetMethod(pSipMsg);

    if (!objMethod.Equals(SipMethod::REGISTER))
    {
        // Pass REGISTER response to the application.
        return IMS_TRUE;
    }

    IMS_SINT32 nStatusCode = SipStack::GetStatusCode(pSipMsg);

    if (!SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        // Pass REGISTER response to the application.
        return IMS_TRUE;
    }

    AString strCallId = SipStack::GetHeaderAsString(pSipMsg, ISipHeader::CALL_ID);
    const SipRtConfigHelper* pConfigHelper = SipRtConfigUtils::GetConfigHelper(nSlotId);
    const SipAddress* pContact = pConfigHelper->GetRegContactUri(strCallId);

    if (pContact == IMS_NULL)
    {
        // No Reg-Contact provisioned; Pass REGISTER response to the application.
        return IMS_TRUE;
    }

    IMS_SINT32 nHeaderCount = SipStack::GetHeaderCount(pSipMsg, ISipHeader::CONTACT_ANY);

    if (nHeaderCount == 0)
    {
        // Drop REGISTER response.
        return IMS_FALSE;
    }

    AString strContact;
    SipAddress objContact;

    for (IMS_SINT32 i = 0; i < nHeaderCount; ++i)
    {
        strContact = SipStack::GetHeaderAsString(pSipMsg, ISipHeader::CONTACT_ANY, IMS_FALSE, i);

        objContact.Create(strContact);

        if (pContact->Equals(objContact))
        {
            // Pass REGISTER response to the application.
            return IMS_TRUE;
        }
    }

    IMS_TRACE_D("Reg-Contact(%s) is not matched", SipDebug::GetUri1(pContact->ToString()).GetStr(),
            0, 0);

    // Drop REGISTER response.
    return IMS_FALSE;
}
