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
#include "ServiceNetwork.h"
#include "SystemConfig.h"
#include "ByteArray.h"
#include "SIPPrivate.h"
#include "SipDebug.h"
#include "SipConfigProxy.h"
#include "SIPManager.h"
#include "SIPWakeLock.h"
#include "SIPFactoryProxy.h"
#include "SIPRTConfigUtils.h"
#include "SIPMessage.h"
#include "SIPTransport.h"
#include "SIPClientTransactionState.h"
#include "SIPServerTransactionState.h"
#include "ISIPServerTransactionStateListener.h"
#include "SipRoutingRejectNotifier.h"
#include "SIPPacketTracker.h"
#include "SIPMessageHandler.h"

__IMS_TRACE_TAG_SIP__;

PRIVATE
SIPMessageHandler::SIPMessageHandler() {}

PUBLIC
SIPMessageHandler::~SIPMessageHandler() {}

PUBLIC GLOBAL SIPMessageHandler* SIPMessageHandler::GetInstance()
{
    static SIPMessageHandler* pSIPMsgHandler = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (pSIPMsgHandler == IMS_NULL)
    {
        pSIPMsgHandler = new SIPMessageHandler();
    }

    return pSIPMsgHandler;
}

PRIVATE VIRTUAL void SIPMessageHandler::Transport_PacketReceived(IN IMS_SINT32 nSlotId,
        IN CONST ByteArray& objBuffer, IN CONST SIPTransportAddress& objNearEnd,
        IN CONST SIPTransportAddress& objFarEnd)
{
    SipMessage* pstMessage = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    // 3 Updates the operations in case of an exceptional operation

    // No need to call DecodeMessage() repeatedly till the entire buffer is parsed.
    // We do not take care of the situation where the multiple messages arrive
    // on a single buffer (as in the case of TCP).
    // Because, the checking of message completion will be evaluated in the SIP socket layer.

    if (!SIPStack::DecodeMessage(
                objBuffer.GetData(), objBuffer.GetLength(), SIPPrivate::OPTIONS_D, pstMessage))
    {
        SIPStack::FreeMessage(pstMessage);

        IMS_TRACE_E(SIPStack::GetLastError(),
                "DECODING FAILURE : The incoming packet MAY be a malformed SIP message", 0, 0, 0);
        return;
    }

    /// TRACE_DEBUG ....
    SIPStack::DisplayUnknownHeaders(pstMessage);

    if (SIPStack::GetBadHeaderCount(pstMessage) > 0)
    {
        SIPStack::DisplayBadHeaders(pstMessage);

        if (!SIPStack::HasMandatoryHeaders(pstMessage))
        {
            SIPStack::FreeMessage(pstMessage);

            IMS_TRACE_E(SIPStack::GetLastError(),
                    "DECODING FAILURE : Malformed SIP message - mandatory headers", 0, 0, 0);
            return;
        }
    }

    // SIP_PACKET_TRACKER
    NotifyPacketReceived(nSlotId, objBuffer, pstMessage, SIPPrivate::MESSAGE_VALID);

    IMS_SINT32 nValidity;

    if (SIPStack::IsRequestMessage(pstMessage))
    {
        if (SIPWakeLock::IsSupported())
        {
            SipMethod objMethod = SIPStack::GetMethod(pstMessage);
            SIPWakeLock::Acquire(objMethod);
        }

        nValidity = NotifyRequest(nSlotId, pstMessage, objNearEnd, objFarEnd);
    }
    else
    {
        // REG-CONTACT-VALIDATION
        if (SIPRTConfigUtils::IsRegContactAddressConfigured(nSlotId))
        {
            if (!CheckRegContactValidity(nSlotId, pstMessage))
            {
                IMS_TRACE_D("Ignore REGISTER response (reg-contact-mismatch)", 0, 0, 0);

                SIPStack::FreeMessage(pstMessage);
                return;
            }
        }

        nValidity = NotifyResponse(nSlotId, pstMessage, objNearEnd, objFarEnd);
    }

    if (nValidity == SIPPrivate::MESSAGE_FAILED)
    {
        IMS_TRACE_I("___ PROCESSING %s MESSAGE FAILED",
                SIPStack::IsRequestMessage(pstMessage) ? "REQUEST" : "RESPONSE", 0, 0);
    }

    SIPStack::FreeMessage(pstMessage);
}

PRIVATE
void SIPMessageHandler::NotifyPacketReceived(IN IMS_SINT32 nSlotId, IN CONST ByteArray& objBuffer,
        IN SipMessage* pstMessage, IN IMS_SINT32 nProcessingResult)
{
    SIPFactoryProxy* pFactoryProxy = SIPFactoryProxy::GetInstance();

    if (pFactoryProxy->IsPacketTrackerEnabled(nSlotId))
    {
        SIPPacketTracker* pPacketTracker = pFactoryProxy->GetPacketTracker(nSlotId);
        SIPMessage objSIPMsg(pstMessage);

        pPacketTracker->NotifyMessageReceived(&objSIPMsg, objBuffer,
                ((nProcessingResult == SIPPrivate::MESSAGE_DISCARDED) ? IMS_TRUE : IMS_FALSE));
    }
}

PRIVATE
IMS_SINT32 SIPMessageHandler::NotifyRequest(IN IMS_SINT32 nSlotId, IN SipMessage* pstMessage,
        IN CONST SIPTransportAddress& objNearEnd, IN CONST SIPTransportAddress& objFarEnd)
{
    ISIPServerTransactionStateListener* piListener =
            SIPManager::GetInstance()->LookupConnectionNotifier(objNearEnd);

    //---------------------------------------------------------------------------------------------

    // UE_TCP_CONNECTION_REUSED -- starts
    // When the transport protocol is TCP and the transaction re-use the TCP connection
    // which is created by UE, check the Request-URI again for port number.
    if ((piListener == IMS_NULL) &&
            ((objNearEnd.GetProtocol() == SIPTransportAddress::PROTOCOL_TCP) ||
                    (objNearEnd.GetProtocol() == SIPTransportAddress::PROTOCOL_TLS)))
    {
        // Gets the port number from the Request-URI
        SipAddrSpec* pstAddrSpec = SIPStack::GetRequestUri(pstMessage);

        if (pstAddrSpec != IMS_NULL)
        {
            AString strHost;
            IMS_UINT32 nPort = Sip::PORT_UNSPECIFIED;

            if (!SIPStack::GetHostAndPort(pstAddrSpec, strHost, nPort))
            {
                if (SIPStack::IsLastErrorNoExist())
                {
                    // FIXME: should we get SIP profile?
                    nPort = SipConfigProxy::GetPort(nSlotId);
                }
            }
            else
            {
                // NO_EXIST (for port only)
                if ((nPort == Sip::PORT_UNSPECIFIED) && SIPStack::IsLastErrorNoExist())
                {
                    // FIXME: should we get SIP profile?
                    nPort = SipConfigProxy::GetPort(nSlotId);
                    IMS_TRACE_D("ConnectionNotifier :: port(%d) from config.", nPort, 0, 0);
                }
            }

            if (nPort != Sip::PORT_UNSPECIFIED)
            {
                SIPTransportAddress objTA = objNearEnd;

                objTA.SetPort(nPort);

                piListener = SIPManager::GetInstance()->LookupConnectionNotifier(objTA);

                if (piListener != IMS_NULL)
                {
                    IMS_TRACE_D("ConnectionNotifier found by (%s, %d)",
                            SipDebug::GetIp(objTA.GetIPAddress()), objTA.GetPort(), 0);
                }
            }

            SIPStack::FreeAddrSpec(pstAddrSpec);
        }
    }
    // UE_TCP_CONNECTION_REUSED -- ends

    if (SystemConfig::IsMultiSimEnabled())
    {
        IMS_TRACE_D("Incoming SIP request on SIM%d ...", nSlotId, 0, 0);
    }

    RCPtr<SIPServerTransactionState> pSTState =
            new SIPServerTransactionState(nSlotId, objNearEnd, objFarEnd);

    if (pSTState.IsNull())
    {
        // Discard the request
        return SIPPrivate::MESSAGE_FAILED;
    }

    IMS_SINT32 nValidity = pSTState->MatchTransaction(pstMessage);
    IMS_BOOL bRejectRequest = IMS_FALSE;

    if (nValidity != SIPPrivate::MESSAGE_VALID)
    {
        bRejectRequest = IMS_TRUE;
    }
    else if (SIPStack::GetBadHeaderCount(pstMessage) > 0)
    {
        bRejectRequest = IMS_TRUE;
        nValidity = SIPPrivate::MESSAGE_INVALID_400;
    }

    if (bRejectRequest)
    {
        IMS_SINT32 nResult = SIPPrivate::MESSAGE_DISCARDED;

        // Send failure response if needs
        if ((nValidity == SIPPrivate::MESSAGE_INVALID_400) ||
                (nValidity == SIPPrivate::MESSAGE_INVALID_405) ||
                (nValidity == SIPPrivate::MESSAGE_INVALID_481))
        {
            // Update the transport information
            if (piListener != IMS_NULL)
            {
                piListener->ServerTransactionState_RequestCreated(pSTState.Get());
            }
            else
            {
                pSTState->SetTransportTuple(
                        objNearEnd.GetIPAddress(), objNearEnd.GetPort(), objNearEnd.GetPort());
            }

            // To send failure response
            pSTState->CheckMessageValidity();

            RCPtr<SIPDialogEx> pOrigDialogEx;
            nResult = pSTState->HandleRequest(pOrigDialogEx);
        }

        if ((nResult == SIPPrivate::MESSAGE_VALID) || (nResult == SIPPrivate::MESSAGE_VALID_FORKED))
        {
            if (nValidity == SIPPrivate::MESSAGE_INVALID_481)
            {
                pSTState->RejectRequest(SipStatusCode::SC_481);
            }
            else if (nValidity == SIPPrivate::MESSAGE_INVALID_400)
            {
                pSTState->RejectRequest(SipStatusCode::SC_400);
            }
            else if (nValidity == SIPPrivate::MESSAGE_INVALID_405)
            {
                pSTState->RejectRequest(SipStatusCode::SC_405);
            }
        }

        pSTState->Abort();
        return nValidity;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_D("NO LISTENER :: REQUEST MESSAGE IS DISCARDED", 0, 0, 0);

        // Update the transport information
        pSTState->SetTransportTuple(
                objNearEnd.GetIPAddress(), objNearEnd.GetPort(), objNearEnd.GetPort());

        // Send failure response
        pSTState->CheckMessageValidity();

        RCPtr<SIPDialogEx> pOrigDialogEx;
        IMS_SINT32 nResult = pSTState->HandleRequest(pOrigDialogEx);

        if ((nResult == SIPPrivate::MESSAGE_VALID) || (nResult == SIPPrivate::MESSAGE_VALID_FORKED))
        {
            SipStatusCode objStatusCode(SipStatusCode::SC_404);
            SIPFactoryProxy* pFactoryProxy = SIPFactoryProxy::GetInstance();

            if (pFactoryProxy->IsRoutingRejectNotifierEnabled(nSlotId))
            {
                SipRoutingRejectNotifier* pRoutingRejectNotifier =
                        pFactoryProxy->GetRoutingRejectNotifier(nSlotId);
                SIPMessage objSIPMsg(pstMessage);

                pRoutingRejectNotifier->NotifyRequestReject(&objSIPMsg, objStatusCode);

                if (objStatusCode != SipStatusCode::SC_404)
                {
                    IMS_TRACE_D("SIPRoutingReject :: Status code is overwritten (404 >> %d)",
                            objStatusCode.ToInt(), 0, 0);
                }
            }

            pSTState->RejectRequest(objStatusCode.ToInt(), objStatusCode.GetReasonPhrase());
        }

        pSTState->Abort();

        return SIPPrivate::MESSAGE_FAILED;
    }

    // After creating the transaction, we need to update some informations
    // related to this transaction.
    piListener->ServerTransactionState_RequestCreated(pSTState.Get());

    nValidity = pSTState->CheckMessageValidity();

    if (nValidity != SIPPrivate::MESSAGE_VALID)
    {
        RCPtr<SIPDialogEx> pOrigDialogEx;
        IMS_SINT32 nResult = pSTState->HandleRequest(pOrigDialogEx);

        if ((nResult == SIPPrivate::MESSAGE_VALID) || (nResult == SIPPrivate::MESSAGE_VALID_FORKED))
        {
            SipStatusCode objStatusCode(SipStatusCode::SC_400);
            SIPFactoryProxy* pFactoryProxy = SIPFactoryProxy::GetInstance();

            if (pFactoryProxy->IsRoutingRejectNotifierEnabled(nSlotId))
            {
                SipRoutingRejectNotifier* pRoutingRejectNotifier =
                        pFactoryProxy->GetRoutingRejectNotifier(nSlotId);
                SIPMessage objSIPMsg(pstMessage);

                pRoutingRejectNotifier->NotifyRequestReject(&objSIPMsg, objStatusCode);

                if (objStatusCode != SipStatusCode::SC_400)
                {
                    IMS_TRACE_D("SIPRoutingReject :: Status code is overwritten (400 >> %d)",
                            objStatusCode.ToInt(), 0, 0);
                }
            }

            pSTState->RejectRequest(objStatusCode.ToInt(), objStatusCode.GetReasonPhrase());
        }

        pSTState->Abort();

        return nValidity;
    }

    // If IPSec is applied, check the validity of source IP and port.
    if (!CheckIPSecValidityForRequest(nSlotId, pSTState.Get(), objNearEnd, objFarEnd))
    {
        // Discard the incoming request...
        IMS_TRACE_D("IPSEC :: Discarded...", 0, 0, 0);
        pSTState->Abort();
        return SIPPrivate::MESSAGE_DISCARDED;
    }

    RCPtr<SIPDialogEx> pOrigDialogEx;

    nValidity = pSTState->HandleRequest(pOrigDialogEx);

    if (nValidity == SIPPrivate::MESSAGE_VALID)
    {
        piListener->ServerTransactionState_RequestReceived(pSTState.Get());
    }
    else if (nValidity == SIPPrivate::MESSAGE_VALID_FORKED)
    {
        piListener->ServerTransactionState_ForkedRequestReceived(
                pSTState.Get(), pOrigDialogEx.Get());
    }
    else
    {
        pSTState->Abort();
        return nValidity;
    }

    return SIPPrivate::MESSAGE_VALID;
}

PRIVATE
IMS_SINT32 SIPMessageHandler::NotifyResponse(IN IMS_SINT32 nSlotId, IN SipMessage* pstMessage,
        IN CONST SIPTransportAddress& objNearEnd, IN CONST SIPTransportAddress& objFarEnd)
{
    RCPtr<SIPClientTransactionState> pCTState;

    //---------------------------------------------------------------------------------------------

    // If IPSec is applied, check the validity of source IP and port.
    if (!CheckIPSecValidityForResponse(nSlotId, pstMessage, objNearEnd, objFarEnd))
    {
        // Discard the incoming response...
        IMS_TRACE_D("IPSEC :: Discarded...", 0, 0, 0);
        return SIPPrivate::MESSAGE_DISCARDED;
    }

    IMS_SINT32 nValidity =
            SIPClientTransactionState::MatchTransaction(pstMessage, objFarEnd, pCTState);

    if (nValidity != SIPPrivate::MESSAGE_VALID)
    {
        return nValidity;
    }

    nValidity = pCTState->HandleResponse(pstMessage);

    if (nValidity != SIPPrivate::MESSAGE_VALID)
    {
        return nValidity;
    }

    return SIPPrivate::MESSAGE_VALID;
}

PRIVATE
IMS_BOOL SIPMessageHandler::CheckIPSecValidityForRequest(IN IMS_SINT32 nSlotId,
        IN SIPTransactionState* pTState, IN CONST SIPTransportAddress& objNearEnd,
        IN CONST SIPTransportAddress& objFarEnd)
{
    if (pTState->IsIPSecRequired() && SIPRTConfigUtils::IsIPSecSAConfigured(nSlotId))
    {
        if (!IsIPSecSAMatched(nSlotId, objNearEnd, objFarEnd))
        {
            IMS_TRACE_D("IPSEC :: SIP request is dropped by non-SA", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SIPMessageHandler::CheckIPSecValidityForResponse(IN IMS_SINT32 nSlotId,
        IN SipMessage* pstMessage, IN CONST SIPTransportAddress& objNearEnd,
        IN CONST SIPTransportAddress& objFarEnd)
{
    if (SIPRTConfigUtils::IsIPSecSAConfigured(nSlotId) && IsSecuredMessage(nSlotId, pstMessage))
    {
        if (!IsIPSecSAMatched(nSlotId, objNearEnd, objFarEnd))
        {
            IMS_TRACE_D("IPSEC :: SIP response is dropped by non-SA", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SIPMessageHandler::IsIPSecSAMatched(IN IMS_SINT32 nSlotId,
        IN CONST SIPTransportAddress& objNearEnd, IN CONST SIPTransportAddress& objFarEnd)
{
    SIPRTConfigHelper* pConfigHelper = SIPRTConfigUtils::GetConfigHelper(nSlotId);
    IMS_BOOL bAtLeastOneSAMatched = IMS_FALSE;
    const IMSList<SipRtConfig::IpSecSa>& objIPSecSAs = pConfigHelper->GetIpSecSas();

    for (IMS_UINT32 i = 0; i < objIPSecSAs.GetSize(); ++i)
    {
        const SipRtConfig::IpSecSa& objIPSecSA = objIPSecSAs.GetAt(i);

        if (objIPSecSA.IsEmpty())
        {
            continue;
        }

        if ((objIPSecSA.GetPortPc() != objFarEnd.GetPort()) &&
                (objIPSecSA.GetPortPs() != objFarEnd.GetPort()))
        {
            IMS_TRACE_I("IPSec :: PortP is mismatched; pc(%d), ps(%d), far(%d)",
                    objIPSecSA.GetPortPc(), objIPSecSA.GetPortPs(), objFarEnd.GetPort());
            continue;
        }

        if (!objIPSecSA.GetIpAddrP().Equals(objFarEnd.GetIPAddress()))
        {
            if (pConfigHelper->IsRoutingInfoHiddenInLog())
            {
                IMS_TRACE_I("IPSec :: IPP is mismatched", 0, 0, 0);
            }
            else
            {
                IMS_TRACE_I("IPSec :: IPP is mismatched; ipp(%s), far(%s)",
                        SipDebug::GetStr1(objIPSecSA.GetIpAddrP().ToString(), 5).GetStr(),
                        SipDebug::GetIp(objFarEnd.GetIPAddress()), 0);
            }
            continue;
        }

        if ((objIPSecSA.GetPortUc() != objNearEnd.GetPort()) &&
                (objIPSecSA.GetPortUs() != objNearEnd.GetPort()))
        {
            IMS_TRACE_I("IPSec :: PortU is mismatched; uc(%d), us(%d), near(%d)",
                    objIPSecSA.GetPortPc(), objIPSecSA.GetPortPs(), objNearEnd.GetPort());
            continue;
        }

        if (!bAtLeastOneSAMatched)
        {
            bAtLeastOneSAMatched = IMS_TRUE;
            break;
        }
    }

    return bAtLeastOneSAMatched;
}

PRIVATE
IMS_BOOL SIPMessageHandler::IsIPSecSAMatchedForUS(
        IN IMS_SINT32 nSlotId, IN CONST IPAddress& objIP, IN IMS_SINT32 nPort)
{
    SIPRTConfigHelper* pConfigHelper = SIPRTConfigUtils::GetConfigHelper(nSlotId);
    const IMSList<SipRtConfig::IpSecSa>& objIPSecSAs = pConfigHelper->GetIpSecSas();

    for (IMS_UINT32 i = 0; i < objIPSecSAs.GetSize(); ++i)
    {
        const SipRtConfig::IpSecSa& objIPSecSA = objIPSecSAs.GetAt(i);

        if (objIPSecSA.IsEmpty())
        {
            continue;
        }

        if ((objIPSecSA.GetPortUs() == nPort) && objIPSecSA.GetIpAddrU().Equals(objIP))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL SIPMessageHandler::IsSecuredMessage(IN IMS_SINT32 nSlotId, IN SipMessage* pstMessage)
{
    IMS_SINT32 nPort = Sip::PORT_UNSPECIFIED;
    AString strHost;

    if (!SIPTransport::GetHostNPortFromViaHeader(pstMessage, strHost, nPort))
    {
        return IMS_FALSE;
    }

    if (SIPStack::IsRequestMessage(pstMessage))
    {
        // It can be identified by SIP transaction state.
    }
    else
    {
        IPAddress objIP(strHost);

        if (IsIPSecSAMatchedForUS(nSlotId, objIP, nPort))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL SIPMessageHandler::CheckRegContactValidity(
        IN IMS_SINT32 nSlotId, IN SipMessage* pstMessage)
{
    SipMethod objMethod = SIPStack::GetMethod(pstMessage);

    if (!objMethod.Equals(SipMethod::REGISTER))
    {
        // Pass REGISTER response to the application.
        return IMS_TRUE;
    }

    IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(pstMessage);

    if (!SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        // Pass REGISTER response to the application.
        return IMS_TRUE;
    }

    AString strCallId = SIPStack::GetHeaderAsString(pstMessage, ISipHeader::CALL_ID);
    SIPRTConfigHelper* pConfigHelper = SIPRTConfigUtils::GetConfigHelper(nSlotId);
    const SipAddress* pContact = pConfigHelper->GetRegContactUri(strCallId);

    if (pContact == IMS_NULL)
    {
        // No Reg-Contact provisioned; Pass REGISTER response to the application.
        return IMS_TRUE;
    }

    IMS_SINT32 nHeaderCount = SIPStack::GetHeaderCount(pstMessage, ISipHeader::CONTACT_ANY);

    if (nHeaderCount == 0)
    {
        // Drop REGISTER response.
        return IMS_FALSE;
    }

    AString strContact;
    SipAddress objContact;

    for (IMS_SINT32 i = 0; i < nHeaderCount; ++i)
    {
        strContact = SIPStack::GetHeaderAsString(pstMessage, ISipHeader::CONTACT_ANY, IMS_FALSE, i);

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
