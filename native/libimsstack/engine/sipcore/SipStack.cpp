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
#include "ISystemProperty.h"
#include "IZLib.h"
#include "ImsStrLib.h"
#include "IpAddress.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"

#include "ISipNetworkUtil.h"
#include "SipContextUtils.h"
#include "SipStackManager.h"
#include "msg/SipMsgUtil.h"
#include "transport/SipTransportBuffer.h"
#include "txn/SipTimeoutData.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnTimerValues.h"

#include "SipClientTransactionState.h"
#include "SipConfigProxy.h"
#include "SipError.h"
#include "SipHeader.h"
#include "SipHeaderName.h"
#include "SipParameter.h"
#include "SipPrivate.h"
#include "SipMessageBodyPart.h"
#include "SipMessageBuffer.h"
#include "SipStack.h"
#include "SipTransactionCallback.h"
#include "SipUtils.h"

__IMS_TRACE_TAG_SIP_CORE__;

class SipNetworkUtil : public ISipNetworkUtil
{
public:
    SipNetworkUtil() = default;
    ~SipNetworkUtil() override = default;

    SIP_BOOL SendToNetwork(IN SipTransportBuffer* pTransportBuffer,
            IN SipTransportParameter* pTransportParam, ISipUserData* pUserData) override;
};

SIP_BOOL SipNetworkUtil::SendToNetwork(IN SipTransportBuffer* pTransportBuffer,
        IN SipTransportParameter* /*pTransportParam*/, ISipUserData* pUserData)
{
    if (pUserData == SIP_NULL)
    {
        IMS_TRACE_E(0, "User data is missing", 0, 0, 0);
        return SIP_FALSE;
    }

    SipTransactionState* pTState = IMS_NULL;
    SipStack::GetTransactionState(*pUserData, pTState);

    if (pTState == IMS_NULL)
    {
        IMS_TRACE_E(0, "No SipTransactionState in SipTxnContextData", 0, 0, 0);
        return SIP_FALSE;
    }

    if (!pTState->SendToNetwork(reinterpret_cast<const IMS_BYTE*>(pTransportBuffer->GetSipBuffer()),
                static_cast<IMS_SINT32>(pTransportBuffer->GetSipBufferLen())))
    {
        IMS_TRACE_E(0, "SendToNetwork failed", 0, 0, 0);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

namespace SipStack
{

// SIP stack last error storage -- starts
static SipEn_ErrorTypes s_eError;
static SipTransactionCallback* s_pCallback = IMS_NULL;

static void SIPStackError(IN SipEn_ErrorTypes eError)
{
    s_eError = eError;
}
// SIP stack last error storage -- ends

static void DeleteStackString(IN SIP_CHAR*& pszStr)
{
    if (pszStr != IMS_NULL)
    {
        delete[] pszStr;
        pszStr = IMS_NULL;
    }
}

static IMS_BOOL FormAddrSpec(
        IN const SipAddrSpec* pAddrSpec, IN IMS_BOOL bParams, OUT AStringBuffer& objStringBuffer)
{
    if (pAddrSpec->Encode(objStringBuffer, bParams ? SIP_TRUE : SIP_FALSE) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

static IMS_BOOL GetParameter(
        IN const SipHeaderBase* pHeader, IN const AString& strName, OUT SipNameValue*& pParam)
{
    SIPStackError(EERR_NOERR);

    IMS_SINT32 nIndex = pHeader->GetParamIndex(strName.GetStr());

    SipNameValue* pTempParam = (nIndex == -1) ? SIP_NULL : pHeader->GetParam(nIndex);

    if (pTempParam == SIP_NULL)
    {
        SIPStackError(EERR_NOEXISTS);
        return IMS_FALSE;
    }

    pParam = pTempParam;
    return IMS_TRUE;
}

GLOBAL void Initialize()
{
    IMS_SINT32 nSlotId = IMS_SLOT_0;
    const ISipConfigV* piSipConfigV = SipConfigProxy::GetSipConfigV(nSlotId);

    // For transaction layer handling
    // Initialize the retransmission initial timer values, timeout timer values,
    // and wait timer values.
    // Below timer values are based on 3GPP; Consider IETF timer values, later.

    /* Initialize SIP Stack and Register NetworkUtil */
    SipStackManager* pStackMngr = SipStackManager::GetInstance();
    SipConfiguration* pSipConfig = SipConfiguration::GetInstance();

    if (pSipConfig != IMS_NULL)
    {
        pSipConfig->SetTimerC(180000);
        pSipConfig->SetTimerCr(180000);

        // If Timer value in Operator config not equal to "-1"/"",Set Timer values from Config.
        // Else default value
        SetTransactionTimerValues(nSlotId, IMS_NULL, piSipConfigV);

        AString strSipTimers;
        strSipTimers.Sprintf(
                "t1=%u, t2=%u, t4=%u, tb=%u, tc=%u, td=%u, tf=%u, th=%u, ti=%u, tj=%u, tk=%u",
                pSipConfig->GetT1(), pSipConfig->GetT2(), pSipConfig->GetT4(),
                pSipConfig->GetTimerB(), pSipConfig->GetTimerC(), pSipConfig->GetTimerD(),
                pSipConfig->GetTimerF(), pSipConfig->GetTimerH(), pSipConfig->GetTimerI(),
                pSipConfig->GetTimerJ(), pSipConfig->GetTimerK());

        IMS_TRACE_D("SIP Timers: %s", strSipTimers.GetStr(), 0, 0);

        // SetCompactForm Encoding
        SIP_BOOL bCompact = (SIP_BOOL)SipConfigProxy::IsCompactFormConfigured(nSlotId, NULL);
        pSipConfig->SetShortFormEncoding(bCompact);

        // PANI header for ACK message to INVITE failure response
        pSipConfig->EnablePANIHeaderForACK(SIP_TRUE);
    }

    if (pStackMngr != IMS_NULL)
    {
        pStackMngr->RegisterNetwork(new SipNetworkUtil());
        if (s_pCallback == IMS_NULL)
        {
            s_pCallback = new SipTransactionCallback();
        }
        // Register SIP transaction layer's callback functions
        pStackMngr->RegisterTransactionCallback(s_pCallback);
    }
}

GLOBAL void SetTransactionTimerValues(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pSipProfile, IN const ISipConfigV* piSipConfigV)
{
    SipConfiguration* pSipConfig = SipConfiguration::GetInstance();

    if (pSipConfig != IMS_NULL)
    {
        pSipConfig->SetT1(SipConfigProxy::GetTimerValueT1(nSlotId, pSipProfile, piSipConfigV));
        pSipConfig->SetT2(SipConfigProxy::GetTimerValueT2(nSlotId, pSipProfile, piSipConfigV));
        pSipConfig->SetT4(SipConfigProxy::GetTimerValueT4(nSlotId, pSipProfile, piSipConfigV));
        pSipConfig->SetTimerB(SipConfigProxy::GetTimerValueB(nSlotId, pSipProfile, piSipConfigV));
        pSipConfig->SetTimerD(SipConfigProxy::GetTimerValueD(nSlotId, pSipProfile, piSipConfigV));
        pSipConfig->SetTimerF(SipConfigProxy::GetTimerValueF(nSlotId, pSipProfile, piSipConfigV));
        pSipConfig->SetTimerH(SipConfigProxy::GetTimerValueH(nSlotId, pSipProfile, piSipConfigV));
        pSipConfig->SetTimerI(SipConfigProxy::GetTimerValueI(nSlotId, pSipProfile, piSipConfigV));
        pSipConfig->SetTimerJ(SipConfigProxy::GetTimerValueJ(nSlotId, pSipProfile, piSipConfigV));
        pSipConfig->SetTimerK(SipConfigProxy::GetTimerValueK(nSlotId, pSipProfile, piSipConfigV));
    }
}

GLOBAL SipEn_ErrorTypes GetLastError()
{
    return s_eError;
}

GLOBAL IMS_BOOL AppendHeader(IN SipHeaderBase* pHeader, IN_OUT ::SipMessage*& pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    if (pMessage->AppendHeader(pHeader) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

GLOBAL IMS_BOOL AppendMessageBody(IN SipMsgBody* pMsgBody, IN_OUT ::SipMessage*& pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage->AppendMessageBody(pMsgBody) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

GLOBAL IMS_BOOL PrependHeader(IN SipHeaderBase* pHeader, IN_OUT ::SipMessage*& pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    if (pMessage->InsertHeader(pHeader, SIP_ZERO) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

GLOBAL IMS_BOOL PrependUnknownHeader(
        IN const AString& strName, IN const AString& strValue, IN_OUT ::SipMessage*& pMessage)
{
    SipHeaderBase* pHeader = DecodeHeader(SipHeaderBase::UNKNOWN, strName, strValue);

    if (pHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (PrependHeader(pHeader, pMessage) == SIP_FALSE)
    {
        FreeHeader(pHeader);
        return IMS_FALSE;
    }

    FreeHeader(pHeader);

    return IMS_TRUE;
}

GLOBAL SipHeaderBase* CloneHeader(IN SipHeaderBase* pHeader)
{
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    return SipHeaders::CloneHdrObj(pHeader);
}

GLOBAL ::SipMessage* CloneMessage(IN const ::SipMessage* pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    return new ::SipMessage(*pMessage);
}

GLOBAL SipMsgBody* CloneMessageBody(IN const SipMsgBody* pMsgBody)
{
    SIPStackError(EERR_NOERR);

    if (pMsgBody == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    return new SipMsgBody(*pMsgBody);
}

GLOBAL SipHeaderBase* CopyHeader(IN SipHeaderBase* pHeader)
{
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    return CloneHeader(pHeader);
}

GLOBAL SipHeaderBase* CopyHeader(IN SipHeaderBase* pDstHeader, IN SipHeaderBase* pSrcHeader)
{
    if (pSrcHeader == IMS_NULL)
    {
        return pDstHeader;
    }

    if (pDstHeader == IMS_NULL)
    {
        return CopyHeader(pSrcHeader);
    }

    pDstHeader = pSrcHeader;

    AddReference(pDstHeader);

    return pDstHeader;
}

GLOBAL IMS_BOOL CorrectMessageBody(IN_OUT ::SipMessage*& pMessage)
{
    IMS_UINT32 nMsgBodyCount = pMessage->GetMsgBodyCount();

    if (nMsgBodyCount == 1)
    {
        // If the Content-Type header does not exist,
        // then insert the Content-Type header into SIP message.
        SipMsgBody* pMsgBody = pMessage->GetMsgBody(SIP_ZERO);

        if (pMsgBody == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (pMessage->HasHeader(SipHeaderBase::CONTENT_TYPE) == SIP_FALSE)
        {
            SipContentTypeHeader* pContentType = pMsgBody->GetContentType();

            if (pContentType != IMS_NULL)
            {
                pMessage->SetHeader(pContentType);
                pContentType->SipDelete();
            }
            else
            {
                pMsgBody->SipDelete();
                return IMS_FALSE;
            }
        }

        pMsgBody->SipDelete();
    }
    // Multiple headers
    else
    {
        // Sets the boundary parameter if not present
        IMS_TRACE_D("CorrectMessageBody: MsgBodyCount(%d)", nMsgBodyCount, 0, 0);

        SipHeaderBase* pHeader = pMessage->GetHdrObj(SipHeaderBase::CONTENT_TYPE);

        if (pHeader == IMS_NULL)
        {
            SipHeaderBase* pContentType =
                    DecodeHeader(SipHeaderBase::CONTENT_TYPE, Sip::STR_MULTIPART_MIXED);

            if (pContentType == IMS_NULL)
            {
                return IMS_FALSE;
            }

            AString strName(Sip::STR_BOUNDARY);
            AString strBoundary = SipUtils::GenerateBoundary();

            if (!SetParameter(pContentType, strName, strBoundary))
            {
                FreeHeader(pContentType);
                return IMS_FALSE;
            }

            if (pMessage->SetHeader(pContentType) == SIP_FALSE)
            {
                FreeHeader(pContentType);
                return IMS_FALSE;
            }

            FreeHeader(pContentType);
            return IMS_TRUE;
        }

        AString strName(Sip::STR_BOUNDARY);
        SipNameValue* pNameVal = IMS_NULL;

        if (!GetParameter(pHeader, strName, pNameVal))
        {
            if (GetLastError() == EERR_NOEXISTS)
            {
                // Insert a boundary parameter
                AString strBoundary = SipUtils::GenerateBoundary();

                if (!SetParameter(pHeader, strName, strBoundary))
                {
                    FreeHeader(pHeader);
                    return IMS_FALSE;
                }
            }
        }

        FreeHeader(pHeader);
    }

    return IMS_TRUE;
}

GLOBAL SipHeaderBase* CreateHeader(IN IMS_SINT32 nType)
{
    return SipHeaders::CreateCoreHdrObj(nType);
}

GLOBAL SipHeaderBase* CreateHeader(IN IMS_SINT32 nType, IN SipAddrSpec* pAddrSpec)
{
    if (!IsAddressFormatHeader(nType, IMS_NULL))
    {
        return IMS_NULL;
    }

    SipNameAddrHeader* pNewHeader = DYNAMIC_CAST(SipNameAddrHeader*, CreateHeader(nType));

    if (pNewHeader == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (pNewHeader->SetAddrSpec(pAddrSpec) == SIP_FALSE)
    {
        pNewHeader->SipDelete();
        return IMS_NULL;
    }

    return pNewHeader;
}

GLOBAL ::SipMessage* CreateMessage(IN IMS_SINT32 nType)
{
    ::SipMessage* pMessage = new ::SipMessage();

    if (pMessage == IMS_NULL)
    {
        return IMS_NULL;
    }

    pMessage->SetMessageType(nType);

    if (nType == ::SipMessage::REQ_TYPE)
    {
        SipRequestLine* pReqLine = new SipRequestLine();

        if (pReqLine == SIP_NULL)
        {
            pMessage->SipDelete();
            return IMS_NULL;
        }

        pReqLine->SetSipVersion(Sip::STR_SIP_VERSION);
        pMessage->SetRequestline(pReqLine);
    }
    else
    {
        SipStatusLine* pStatusLine = new SipStatusLine();

        if (pStatusLine == SIP_NULL)
        {
            pMessage->SipDelete();
            return IMS_NULL;
        }

        pStatusLine->SetSipVersion(Sip::STR_SIP_VERSION);
        pMessage->SetStatusLine(pStatusLine);
    }

    return pMessage;
}

GLOBAL SipMsgBody* CreateMessageBody()
{
    return new SipMsgBody();
}

GLOBAL SipHeaderBase* CreateViaHeader(
        IN const AString& strSentProtocol, IN const AString& strSentBy, IN const AString& strBranch)
{
    SipViaHeader* pVia = new SipViaHeader();

    if (pVia == IMS_NULL)
    {
        return IMS_NULL;
    }

    pVia->SetProtocolName("SIP");
    pVia->SetProtocolVer(Sip::STR_SIP_VERSION_ONLY);

    const IMS_CHAR* pszTransport = strSentProtocol.GetStr();

    if (strSentProtocol.StartsWith("SIP") || strSentProtocol.StartsWith("sip"))
    {
        pszTransport += 8;
    }

    pVia->SetTransport(pszTransport);

    const IMS_CHAR* pszHost = strSentBy.GetStr();
    IMS_CHAR* pszTmp;

    if (*pszHost == TextParser::CHAR_LSBRACKET)  // For IPV6
    {
        pszTmp = IMS_StrChr(pszHost, TextParser::CHAR_RSBRACKET);

        if (pszTmp != SIP_NULL)
        {
            pszTmp++;
            if (*pszTmp == TextParser::CHAR_COLON)
            {
                *pszTmp = '\0';
                pVia->SetHost(pszHost);
                pszTmp++;
                IMS_SINT32 nPort = IMS_Atoi(pszTmp);
                pVia->SetPortNum(static_cast<SIP_UINT16>(nPort));
            }
            else
            {
                *pszTmp = '\0';
                pVia->SetHost(pszHost);
            }
        }
        else
        {
            pVia->SipDelete();
            return IMS_NULL;
        }
    }
    else if ((pszTmp = IMS_StrChr(pszHost, TextParser::CHAR_COLON)) != SIP_NULL)  // For IPV4
    {
        *pszTmp = '\0';
        pVia->SetHost(pszHost);
        *pszTmp = TextParser::CHAR_COLON;
        pszTmp++;

        IMS_SINT32 nPort = IMS_Atoi(pszTmp);
        pVia->SetPortNum(static_cast<SIP_UINT16>(nPort));
    }
    else
    {
        // only host present
        pVia->SetHost(pszHost);
    }

    if (pVia->SetBranchParam(strBranch.GetStr()) == SIP_FALSE)
    {
        pVia->SipDelete();
        return IMS_NULL;
    }

    return pVia;
}

GLOBAL SipAddrSpec* DecodeAddrSpec(IN const AString& strAddress)
{
    AString strAddrSpec = strAddress;
    IMS_SINT32 nLaquot;

    // Remove LAQUOT/RAQUOT if present
    if ((nLaquot = strAddrSpec.GetIndexOf(TextParser::CHAR_LAQUOT)) != AString::NPOS)
    {
        IMS_SINT32 nRaquot;

        strAddrSpec = strAddrSpec.GetSubStr(nLaquot + 1);

        nRaquot = strAddrSpec.GetIndexOf(TextParser::CHAR_RAQUOT, nLaquot + 1);

        if (nRaquot != AString::NPOS)
        {
            strAddrSpec.Truncate(nRaquot);
        }
    }

    SipAddrSpec* pAddrSpec = new SipAddrSpec();

    if (pAddrSpec->Decode(strAddrSpec.GetStr(), strAddrSpec.GetLength()) == SIP_FALSE)
    {
        pAddrSpec->SipDelete();
        return IMS_NULL;
    }

    return pAddrSpec;
}

GLOBAL SipHeaderBase* DecodeHeader(
        IN IMS_SINT32 nType, IN const AString& strName, IN const AString& strBody)
{
    SIPStackError(EERR_NOERR);

    if (nType == SipHeaderBase::UNKNOWN && (strName.GetLength() != 0))
    {
        SIP_INT32 nUnknownType = SipMsgUtil::GetHeaderType(strName.GetStr());

        if (nUnknownType != SipHeaderBase::TYPE_INVALID)
        {
            nType = nUnknownType;
        }
    }

    nType = GetHdrEnumType(nType);

    if (nType != SipHeaderBase::ALLOW && _IMS_LOG_DEBUG_)
    {
        IMS_TRACE_D("DecodeHeader: type=[%d], body=[%s]", nType, strBody.GetStr(), 0);
    }

    SipHeaderBase* pHeader = CreateHeader(nType);

    if (pHeader == IMS_NULL)
    {
        IMS_TRACE_E(0, "SipHeader is null", 0, 0, 0);
        return IMS_NULL;
    }

    if (nType == SipHeaderBase::UNKNOWN)
    {
        if (strName.GetLength() == 0)
        {
            FreeHeaderEx(pHeader);
            SIPStackError(EERR_INVALIDPARAM);
            IMS_TRACE_D("Unknown header name is unspecified", 0, 0, 0);
            return IMS_NULL;
        }

        SipUnknownHeader* pUnknownHeader = reinterpret_cast<SipUnknownHeader*>(pHeader);

        pUnknownHeader->SetHeaderName(strName.GetStr());
        pUnknownHeader->SetHeaderValue(strBody.GetStr());
    }
    else
    {
        IMS_UINT32 nBodyLen = strBody.GetLength();
        IMS_CHAR* pszTmpBody = strBody.Duplicate();

        if (pszTmpBody == IMS_NULL)
        {
            FreeHeaderEx(pHeader);
            IMS_TRACE_D("Header body is null", 0, 0, 0);
            return IMS_NULL;
        }

        if (pHeader->Decode(pszTmpBody, nBodyLen) == SIP_FALSE)
        {
            IMS_MEM_Free(pszTmpBody);
            FreeHeaderEx(pHeader);
            IMS_TRACE_D("Decoding header is failed", 0, 0, 0);
            return IMS_NULL;
        }

        IMS_MEM_Free(pszTmpBody);
    }

    return pHeader;
}

GLOBAL IMS_BOOL DecodeMessage(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
        IN IMS_SINT32 nOptions, OUT ::SipMessage*& pMessage)
{
    if (pMessage == IMS_NULL)
    {
        pMessage = new ::SipMessage();
    }

    const SIP_CHAR* pszSipBuffer = reinterpret_cast<SIP_CHAR*>(const_cast<IMS_BYTE*>(pBuffer));

    if (nOptions == SipPrivate::OPTIONS_D_PARTIAL)
    {
        if (pMessage->DecodeFragmentMsg(pszSipBuffer, nBuffLen) == SIP_FALSE)
        {
            pMessage->SipDelete();
            pMessage = IMS_NULL;
            return IMS_FALSE;
        }
    }
    else if (pMessage->Decode(pszSipBuffer, nBuffLen) == SIP_FALSE)
    {
        pMessage->SipDelete();
        pMessage = IMS_NULL;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

GLOBAL IMS_BOOL DecodeMessageBody(IN ::SipMessage* pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == NULL)
    {
        SIPStackError(EERR_NOEXISTS);
        return IMS_FALSE;
    }

    IMS_UINT32 nMsgBodyCount = GetMessageBodyCount(pMessage);

    if (nMsgBodyCount == 0)
    {
        IMS_TRACE_D("___ NO SIP MESSAGE BODY ___", 0, 0, 0);
        return IMS_TRUE;
    }

    if (IsMessageBodyCompressed(pMessage))
    {
        SipMsgBody* pMsgBody = GetMessageBody(pMessage);

        if (pMsgBody == NULL)
        {
            IMS_TRACE_E(0, "Getting SIP message body failed", 0, 0, 0);
            return IMS_FALSE;
        }

        IMS_CHAR* pszBuffer = IMS_NULL;
        pMsgBody->GetMsgBuffer(&pszBuffer);

        if (pszBuffer == IMS_NULL)
        {
            IMS_TRACE_E(0, "Message Buffer NULL", 0, 0, 0);
            pMsgBody->SipDelete();
            return IMS_FALSE;
        }

        ByteArray objBodyPart;
        ByteArray objCompBodyPart;

        IMS_UINT32 nBuffLen = 0;
        pMsgBody->GetMsgBuffLen(&nBuffLen);
        objCompBodyPart.Attach(reinterpret_cast<const IMS_BYTE*>(pszBuffer), nBuffLen);

        pMsgBody->SipDelete();

        if (!IMS_UTIL_ZLIB_Uncompress(objCompBodyPart, objBodyPart))
        {
            IMS_TRACE_E(0, "Uncompressing a body part failed", 0, 0, 0);
            DeleteStackString(pszBuffer);
            return IMS_FALSE;
        }

        DeleteStackString(pszBuffer);

        if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
        {
            IMS_TRACE_TEXT("gzip::uncompression", objBodyPart.GetData(), objBodyPart.GetLength());
        }

        IMS_CHAR* pszCompBodyStart = reinterpret_cast<IMS_CHAR*>(objBodyPart.GetData());
        const IMS_CHAR* pszCompBodyEnd = pszCompBodyStart + objBodyPart.GetLength() - 1;
        IMS_UINT32 nCompLength = objBodyPart.GetLength();

        if (pMessage->DecodeMultiPartBody(pszCompBodyStart, pszCompBodyEnd, nCompLength) ==
                SIP_FALSE)
        {
            IMS_TRACE_E(0, "Decoding uncompressed body part failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

GLOBAL IMS_BOOL EncodeAddrSpec(
        IN const SipAddrSpec* pAddrSpec, IN IMS_BOOL bParams, OUT AString& strAddrSpec)
{
    SIPStackError(EERR_NOERR);

    if (pAddrSpec == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    AStringBuffer objAddrSpec(128);

    if (!FormAddrSpec(pAddrSpec, bParams, objAddrSpec))
    {
        strAddrSpec = AString::ConstNull();
        return IMS_FALSE;
    }

    strAddrSpec = objAddrSpec.GetString();

    return IMS_TRUE;
}

GLOBAL IMS_BOOL EncodeHeaderBody(
        IN const SipHeaderBase* pHeader, IN IMS_BOOL bParams, OUT AString& strHeaderBody)
{
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        strHeaderBody = AString::ConstNull();
        return IMS_FALSE;
    }

    if (pHeader->GetHdrType() == SipHeaderBase::UNKNOWN)
    {
        const SipUnknownHeader* pUnknownHeader = DYNAMIC_CAST(SipUnknownHeader*, pHeader);
        strHeaderBody = pUnknownHeader->GetHeaderValue();
        return IMS_TRUE;
    }

    AStringBuffer objBuffer(512);

    if (pHeader->Encode(objBuffer, bParams ? SIP_TRUE : SIP_FALSE) == SIP_FALSE)
    {
        strHeaderBody = AString::ConstNull();
        return SIP_FALSE;
    }

    strHeaderBody = static_cast<const AStringBuffer&>(objBuffer).GetString();

    return IMS_TRUE;
}

GLOBAL IMS_BOOL EncodeMessage(IN ::SipMessage* pMessage, IN IMS_SINT32 nOptions,
        OUT IMS_BYTE*& pBuffer, OUT IMS_SINT32& nBuffLen)
{
    IMS_UINT32 nMsgOptions = SipConfiguration::MSG_OPT_ENCODE_NONE;

    if ((nOptions & SipPrivate::OPT_E_SHORTFORM) != 0)
    {
        nMsgOptions |= SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM;
    }

    if (pMessage->Encode(reinterpret_cast<SIP_CHAR**>(&pBuffer),
                reinterpret_cast<SIP_UINT32*>(&nBuffLen), nMsgOptions) == SIP_FALSE)
    {
        IMS_TRACE_D("EncodeMessage is failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

GLOBAL IMS_BOOL EncodePartialMessage(
        IN ::SipMessage* pMessage, IN IMS_SINT32 nOptions, OUT ByteArray& objMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    if ((nOptions & OPT_ALL) == 0)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    IMS_CHAR szBuffer[SipMessageBuffer::MAX_MSG_SIZE];
    IMS_CHAR* pszBuffer = szBuffer;
    SIP_BOOL bStatus = SIP_FALSE;

    // Check start-line
    if ((nOptions & OPT_START_LINE) == OPT_START_LINE)
    {
        if (pMessage->GetMsgType() == ::SipMessage::REQ_TYPE)
        {
            SipRequestLine* pRequestLine = pMessage->GetReqLine();

            if (pRequestLine != SIP_NULL)
            {
                bStatus = pRequestLine->Encode(&pszBuffer);
                pRequestLine->SipDelete();
            }
        }
        else if (pMessage->GetMsgType() == ::SipMessage::RESP_TYPE)
        {
            SipStatusLine* pStatusLine = pMessage->GetStatusLine();

            if (pStatusLine != SIP_NULL)
            {
                bStatus = pStatusLine->Encode(&pszBuffer);
                pStatusLine->SipDelete();
            }
        }

        if (bStatus == SIP_FALSE)
        {
            IMS_TRACE_D("Encoding start-line is failed", 0, 0, 0);
            return IMS_FALSE;
        }

        // Put CRLF at the end of Start-Line
        SipMsgUtil::EncodeCrlf(pszBuffer);
    }

    SIP_CHAR aMsgBody[SipMsgUtil::MAX_MSG_SIZE] = {
            0,
    };
    SIP_CHAR* pMsgBody = &(aMsgBody[0]);
    SIP_UINT32 nMsgLen = 0;

    // Check header parts
    if ((nOptions & OPT_HEADER_PART) == OPT_HEADER_PART)
    {
        SipMsgBodyList* pMsgBodyList = pMessage->GetMsgBodyList();

        if (pMsgBodyList != SIP_NULL)
        {
            // Content-Length header
            SipUnknownHeader* pContentLength =
                    pMessage->GetUnknownHdrObj(SipHeaderBase::CONTENT_LENGTH);

            if (pContentLength == SIP_NULL)
            {
                if (pMsgBodyList != SIP_NULL)
                {
                    SipContentTypeHeader* pContentType = DYNAMIC_CAST(SipContentTypeHeader*,
                            pMessage->GetHdrObj(SipHeaderBase::CONTENT_TYPE));

                    if (pContentType == SIP_NULL)
                    {
                        pMsgBodyList->SipDelete();
                        IMS_TRACE_D("Content-Type is not present", 0, 0, 0);
                        return IMS_FALSE;
                    }

                    SIP_CHAR* pszBoundary = pContentType->GetBoundary();

                    if (pMsgBodyList->GetEncodedMessageBody(&pMsgBody, nMsgLen, pszBoundary) ==
                            SIP_FALSE)
                    {
                        DeleteStackString(pszBoundary);
                        pContentType->SipDelete();
                        pMsgBodyList->SipDelete();
                        return IMS_FALSE;
                    }

                    pContentType->SipDelete();
                    DeleteStackString(pszBoundary);
                }

                pMessage->SetContentLengthHdr(nMsgLen, SipConfiguration::MSG_OPT_ENCODE_NONE);
            }
            else
            {
                pContentLength->SipDelete();
            }

            // Check for Content-Type header and set the new one if not present
            if (pMessage->HasHeader(SipHeaderBase::CONTENT_TYPE) == SIP_FALSE)
            {
                SipContentTypeHeader* pContentType = SIP_NULL;
                IMS_UINT32 nMsgBodyCount = pMsgBodyList->GetMsgBodyCount();

                if (nMsgBodyCount == SIP_ONE)
                {
                    SipMsgBody* pMsgbody = pMsgBodyList->GetBodyByIndex(SIP_ZERO);
                    SipContentTypeHeader* pTempContentType = pMsgbody->GetContentType();

                    if (pTempContentType != SIP_NULL)
                    {
                        pContentType = new SipContentTypeHeader(*pTempContentType);
                        pTempContentType->SipDelete();
                    }
                    else
                    {
                        pMsgbody->SipDelete();
                        pMsgBodyList->SipDelete();
                        IMS_TRACE_D("Content-Type header is not present", 0, 0, 0);
                        return IMS_FALSE;
                    }

                    pMessage->SetHeader(pContentType);

                    pContentType->SipDelete();
                    pMsgbody->SipDelete();
                }
                // Multiple message bodies
                else
                {
                    pContentType = new SipContentTypeHeader(SipHeaderBase::CONTENT_TYPE);
                    pContentType->SetMediaType(Sip::STR_MULTIPART);
                    pContentType->SetSubMediaType(Sip::STR_MIXED);

                    pMessage->SetHeader(pContentType);

                    pContentType->SipDelete();
                }
            }

            pMsgBodyList->SipDelete();
        }
        else
        {
            SipIntegerHeader* pContentLength = new SipIntegerHeader(SipHeaderBase::CONTENT_LENGTH);
            pContentLength->SetValueInt(SIP_ZERO);
            pMessage->SetHeader(pContentLength);
            pContentLength->SipDelete();
        }

        if (pMessage->GetMsgHdrs()->Encode(&pszBuffer, SipConfiguration::MSG_OPT_ENCODE_NONE) ==
                SIP_FALSE)
        {
            IMS_TRACE_D("Encoding headers failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    SipMsgUtil::EncodeCrlf(pszBuffer);

    if ((nOptions & OPT_BODY_PART) == OPT_BODY_PART)
    {
        SipMsgBodyList* pMsgBodyList = pMessage->GetMsgBodyList();

        if (pMsgBodyList != SIP_NULL)
        {
            if ((nOptions & OPT_HEADER_PART) != OPT_HEADER_PART)
            {
                SipContentTypeHeader* pContentType = DYNAMIC_CAST(
                        SipContentTypeHeader*, pMessage->GetHdrObj(SipHeaderBase::CONTENT_TYPE));

                if (pContentType == SIP_NULL)
                {
                    pMsgBodyList->SipDelete();
                    IMS_TRACE_D("Content-Type is not present", 0, 0, 0);
                    return IMS_FALSE;
                }

                SIP_CHAR* pszBoundary = pContentType->GetBoundary();

                if (pMsgBodyList->GetEncodedMessageBody(&pMsgBody, nMsgLen, pszBoundary) ==
                        SIP_FALSE)
                {
                    DeleteStackString(pszBoundary);
                    pContentType->SipDelete();
                    pMsgBodyList->SipDelete();
                    IMS_TRACE_D("Encode message body fail", 0, 0, 0);
                    return IMS_FALSE;
                }
                DeleteStackString(pszBoundary);
                pContentType->SipDelete();
            }

            pMsgBody = &(aMsgBody[0]);
            IMS_StrCpy(pszBuffer, nMsgLen, pMsgBody);
            pszBuffer += nMsgLen;

            pMsgBodyList->SipDelete();
        }
    }

    AString strBuffer(szBuffer);
    objMessage.Append(reinterpret_cast<const IMS_BYTE*>(strBuffer.GetStr()), strBuffer.GetLength());

    SIPStackError(EERR_NOERR);

    return IMS_TRUE;
}

GLOBAL IMS_BOOL IsUnknownHeader(IN_OUT IMS_SINT32& nType, IN const AString& strName)
{
    if (strName.GetLength() != 0)
    {
        nType = SipMsgUtil::GetHeaderType(strName.GetStr());
    }

    return (nType == ISipHeader::UNKNOWN);
}

GLOBAL ImsList<SipParameter*> ExtractParameters(IN const SipHeaderBase* pHeader)
{
    ImsList<SipParameter*> objParams;

    IMS_UINT32 nParamCount = pHeader->GetParamCount();

    for (IMS_UINT32 i = 0; i < nParamCount; ++i)
    {
        SipNameValue* pNameVal = pHeader->GetParam(i);

        if (pNameVal == IMS_NULL)
        {
            continue;
        }

        SipParameter* pParameter = new SipParameter(pNameVal->m_pszName);

        IMS_UINT32 nValueCount = pNameVal->m_objValueList.GetSize();

        for (IMS_UINT32 j = 0; j < nValueCount; ++j)
        {
            IMS_CHAR* pszValue = pNameVal->m_objValueList.GetAt(j);

            if (pszValue != IMS_NULL)
            {
                AString strValue(pszValue);
                pParameter->AddValues(strValue);
            }
        }

        objParams.Append(pParameter);
    }

    return objParams;
}

GLOBAL ImsList<SipParameter*> ExtractParameters(IN SipAddrSpec* pAddrSpec)
{
    ImsList<SipParameter*> objParams;

    if (pAddrSpec == IMS_NULL)
    {
        return objParams;
    }

    SipUri* pSipUri = pAddrSpec->GetSipUri();

    if (pSipUri == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::PARSING_ERROR);
        return objParams;
    }

    IMS_UINT32 nParamCount = pSipUri->GetUriParamCount();

    for (IMS_UINT32 i = 0; i < nParamCount; ++i)
    {
        SipNameValue* pNameVal = pSipUri->GetUriParam(i);

        if (pNameVal == IMS_NULL)
        {
            continue;
        }

        SipParameter* pParameter = new SipParameter(pNameVal->m_pszName);

        if (pParameter == IMS_NULL)
        {
            pSipUri->SipDelete();
            SipPrivate::SetLastError(SipError::PARSING_ERROR);
            return objParams;
        }

        if (pNameVal->m_objValueList.IsEmpty())
        {
            objParams.Append(pParameter);
            continue;
        }

        IMS_UINT32 nValueCount = pNameVal->m_objValueList.GetSize();

        for (IMS_UINT32 j = 0; j < nValueCount; ++j)
        {
            IMS_CHAR* pszValue = pNameVal->m_objValueList.GetAt(j);

            if (pszValue != IMS_NULL)
            {
                AString strValue(pszValue);
                pParameter->AddValues(strValue);
            }
        }

        objParams.Append(pParameter);
    }

    pSipUri->SipDelete();

    return objParams;
}

GLOBAL ImsList<SipParameter*> ExtractParameters(IN const AString& strParams, IN IMS_CHAR cSep)
{
    ImsList<SipParameter*> objParams;
    AString strTmp = strParams.Trim();

    SIPStackError(EERR_NOERR);

    IMS_TRACE_D("ExtractParameters: [%s] with separator [%c]", strParams.GetStr(), cSep, 0);

    if ((strTmp.GetLength() == 0) || strTmp.Equals(cSep))
    {
        return objParams;
    }

    AStringArray objTokens = strTmp.Split(cSep);

    for (IMS_SINT32 i = 0; i < objTokens.GetCount(); ++i)
    {
        const AString& strToken = objTokens.GetElementAt(i);

        IMS_SINT32 nPos = strToken.GetIndexOf(TextParser::CHAR_EQUAL);
        SipParameter* pParameter = IMS_NULL;

        // name only parameter
        if (nPos == AString::NPOS)
        {
            pParameter = new SipParameter(strToken);
        }
        // name - value pair parameter
        else
        {
            AString strName = strToken.GetSubStr(0, nPos);
            AString strValue = strToken.GetSubStr(nPos + 1, strToken.GetLength() - nPos);
            AStringArray objValues = strValue.Split(TextParser::CHAR_COMMA);

            pParameter = new SipParameter(strName, objValues);
        }

        objParams.Append(pParameter);
    }

    return objParams;
}

GLOBAL void FreeAddrSpec(IN SipAddrSpec*& pAddrSpec)
{
    if (pAddrSpec != IMS_NULL)
    {
        pAddrSpec->SipDelete();
        pAddrSpec = IMS_NULL;
    }
}

GLOBAL void FreeHeader(IN SipHeaderBase* pHeader)
{
    if (pHeader != IMS_NULL)
    {
        pHeader->SipDelete();
    }
}

GLOBAL void FreeHeaderEx(IN SipHeaderBase*& pHeader)
{
    if (pHeader != IMS_NULL)
    {
        pHeader->SipDelete();
        pHeader = IMS_NULL;
    }
}

GLOBAL void FreeMessage(IN ::SipMessage*& pMessage)
{
    if (pMessage != IMS_NULL)
    {
        pMessage->SipDelete();
        pMessage = IMS_NULL;
    }
}

GLOBAL void FreeMessageBody(IN SipMsgBody*& pMsgBody)
{
    if (pMsgBody != IMS_NULL)
    {
        pMsgBody->SipDelete();
        pMsgBody = IMS_NULL;
    }
}

GLOBAL IMS_CHAR GetCompactHeaderName(
        IN IMS_SINT32 nType, IN const AString& strName /*= AString::ConstNull()*/)
{
    (void)strName;

    switch (nType)
    {
        case SipHeaderBase::CALL_ID:
            return SipHeaderName::CF_CALL_ID;
        case SipHeaderBase::CONTACT:       // FALL-THROUGH
        case SipHeaderBase::CONTACT_WILD:  // FALL-THROUGH
        case SipHeaderBase::CONTACT_ANY:
            return SipHeaderName::CF_CONTACT;
        case SipHeaderBase::CONTENT_ENCODING:
            return SipHeaderName::CF_CONTENT_ENCODING;
        case SipHeaderBase::CONTENT_LENGTH:
            return SipHeaderName::CF_CONTENT_LENGTH;
        case SipHeaderBase::CONTENT_TYPE:
            return SipHeaderName::CF_CONTENT_TYPE;
        case SipHeaderBase::FROM:
            return SipHeaderName::CF_FROM;
        case SipHeaderBase::SUPPORTED:
            return SipHeaderName::CF_SUPPORTED;
        case SipHeaderBase::TO:
            return SipHeaderName::CF_TO;
        case SipHeaderBase::VIA:
            return SipHeaderName::CF_VIA;
        case SipHeaderBase::EVENT:
            return SipHeaderName::CF_EVENT;
        case SipHeaderBase::ALLOW_EVENTS:
            return SipHeaderName::CF_ALLOW_EVENTS;
        case SipHeaderBase::REFER_TO:
            return SipHeaderName::CF_REFER_TO;
        case SipHeaderBase::REFERRED_BY:
            return SipHeaderName::CF_REFERRED_BY;
        case SipHeaderBase::REQUEST_DISPOSITION:
            return SipHeaderName::CF_REQUEST_DISPOSITION;
        case SipHeaderBase::ACCEPT_CONTACT:
            return SipHeaderName::CF_ACCEPT_CONTACT;
        case SipHeaderBase::REJECT_CONTACT:
            return SipHeaderName::CF_REJECT_CONTACT;
        case SipHeaderBase::SESSION_EXPIRES:
            return SipHeaderName::CF_SESSION_EXPIRES;
        case SipHeaderBase::SUBJECT:
            return SipHeaderName::CF_SUBJECT;
        case SipHeaderBase::IDENTITY_INFO:
            return SipHeaderName::CF_IDENTITY_INFO;
        case SipHeaderBase::IDENTITY:
            return SipHeaderName::CF_IDENTITY;
        default:
            break;
    }

    return '\0';
}

GLOBAL const IMS_CHAR* GetHeaderName(
        IN IMS_SINT32 nType, IN const AString& strName /*= AString::ConstNull()*/)
{
    if ((nType <= ISipHeader::INVALID) || (nType >= ISipHeader::ANY))
    {
        return IMS_NULL;
    }

    return (nType != ISipHeader::UNKNOWN) ? SipHeader::NAME[nType] : strName.GetStr();
}

GLOBAL IMS_SINT32 GetHeaderTypeFromName(IN const AString& strName)
{
    IMS_SINT32 nType = SipHeaderBase::UNKNOWN;

    if (strName.GetLength() == 0)
    {
        return SipHeaderBase::TYPE_INVALID;
    }

    switch (strName[0])
    {
        case 'a':
        case 'A':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_ACCEPT_CONTACT) ||
                    strName.EqualsIgnoreCase(SipHeaderName::ACCEPT_CONTACT))
            {
                nType = SipHeaderBase::ACCEPT_CONTACT;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::ALLOW))
            {
                nType = SipHeaderBase::ALLOW;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::ALLOW_EVENTS))
            {
                nType = SipHeaderBase::ALLOW_EVENTS;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::ACCEPT))
            {
                nType = SipHeaderBase::ACCEPT;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::AUTHORIZATION))
            {
                nType = SipHeaderBase::AUTHORIZATION;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::ACCEPT_RESOURCE_PRIORITY))
            {
                nType = SipHeaderBase::ACCEPT_RESOURCE_PRIORITY;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::ACCEPT_ENCODING))
            {
                nType = SipHeaderBase::ACCEPT_ENCODING;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::ACCEPT_LANGUAGE))
            {
                nType = SipHeaderBase::ACCEPT_LANGUAGE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::ALERT_INFO))
            {
                nType = SipHeaderBase::ALERT_INFO;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::ANSWER_MODE))
            {
                nType = SipHeaderBase::ANSWER_MODE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::AUTHENTICATION_INFO))
            {
                nType = SipHeaderBase::AUTHENTICATION_INFO;
            }
        }
        break;

        case 'b':
        case 'B':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_REFERRED_BY))
            {
                nType = SipHeaderBase::REFERRED_BY;
            }
        }
        break;

        case 'c':
        case 'C':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_CONTENT_TYPE) ||
                    strName.EqualsIgnoreCase(SipHeaderName::CONTENT_TYPE))
            {
                nType = SipHeaderBase::CONTENT_TYPE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::CALL_ID))
            {
                nType = SipHeaderBase::CALL_ID;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::CONTACT))
            {
                nType = SipHeaderBase::CONTACT;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_LENGTH))
            {
                nType = SipHeaderBase::CONTENT_LENGTH;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_DISPOSITION))
            {
                nType = SipHeaderBase::CONTENT_DISPOSITION;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_ENCODING))
            {
                nType = SipHeaderBase::CONTENT_ENCODING;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::CSEQ))
            {
                nType = SipHeaderBase::CSEQ;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::CALL_INFO))
            {
                nType = SipHeaderBase::CALL_INFO;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_LANGUAGE))
            {
                nType = SipHeaderBase::CONTENT_LANGUAGE;
            }
        }
        break;

        case 'd':
        case 'D':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_REQUEST_DISPOSITION))
            {
                nType = SipHeaderBase::REQUEST_DISPOSITION;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::DATE))
            {
                nType = SipHeaderBase::DATE;
            }
        }
        break;

        case 'e':
        case 'E':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_CONTENT_ENCODING))
            {
                nType = SipHeaderBase::CONTENT_ENCODING;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::EVENT))
            {
                nType = SipHeaderBase::EVENT;
            }

            else if (strName.EqualsIgnoreCase(SipHeaderName::EXPIRES))
            {
                nType = SipHeaderBase::EXPIRES_ANY;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::ERROR_INFO))
            {
                nType = SipHeaderBase::ERROR_INFO;
            }
        }
        break;

        case 'f':
        case 'F':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_FROM) ||
                    strName.EqualsIgnoreCase(SipHeaderName::FROM))
            {
                nType = SipHeaderBase::FROM;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::FLOW_TIMER))
            {
                nType = SipHeaderBase::FLOW_TIMER;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::FEATURE_CAPS))
            {
                nType = SipHeaderBase::FEATURE_CAPS;
            }
        }
        break;

        case 'h':
        case 'H':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::HISTORY_INFO))
            {
                nType = SipHeaderBase::HISTORY_INFO;
            }
        }
        break;

        case 'i':
        case 'I':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_CALL_ID))
            {
                nType = SipHeaderBase::CALL_ID;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::IDENTITY))
            {
                nType = SipHeaderBase::IDENTITY;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::IDENTITY_INFO))
            {
                nType = SipHeaderBase::IDENTITY_INFO;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::IN_REPLY_TO))
            {
                nType = SipHeaderBase::IN_REPLY_TO;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::INFO_PACKAGE))
            {
                nType = SipHeaderBase::INFO_PACKAGE;
            }
        }
        break;

        case 'j':
        case 'J':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_REJECT_CONTACT))
            {
                nType = SipHeaderBase::REJECT_CONTACT;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::JOIN))
            {
                nType = SipHeaderBase::JOIN;
            }
        }
        break;

        case 'k':
        case 'K':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_SUPPORTED))
            {
                nType = SipHeaderBase::SUPPORTED;
            }
        }
        break;

        case 'l':
        case 'L':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_CONTENT_LENGTH))
            {
                nType = SipHeaderBase::CONTENT_LENGTH;
            }
        }
        break;

        case 'm':
        case 'M':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_CONTACT))
            {
                nType = SipHeaderBase::CONTACT;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::MAX_FORWARDS))
            {
                nType = SipHeaderBase::MAX_FORWARDS;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::MIME_VERSION))
            {
                nType = SipHeaderBase::MIME_VERSION;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::MIN_SE))
            {
                nType = SipHeaderBase::MIN_SE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::MIN_EXPIRES))
            {
                nType = SipHeaderBase::MIN_EXPIRES;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::MAX_BREADTH))
            {
                nType = SipHeaderBase::MAX_BREADTH;
            }
        }
        break;

        case 'o':
        case 'O':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_EVENT))
            {
                nType = SipHeaderBase::EVENT;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::ORGANIZATION))
            {
                nType = SipHeaderBase::ORGANIZATION;
            }
        }
        break;

        case 'p':
        case 'P':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::PATH))
            {
                nType = SipHeaderBase::PATH;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_ASSOCIATED_URI))
            {
                nType = SipHeaderBase::P_ASSOCIATED_URI;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_CALLED_PARTY_ID))
            {
                nType = SipHeaderBase::P_CALLED_PARTY_ID;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_VISITED_NETWORK_ID))
            {
                nType = SipHeaderBase::P_VISITED_NETWORK_ID;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_CHARGING_FUNCTION_ADDRESSES))
            {
                nType = SipHeaderBase::P_CHRG_FUN_ADDR;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_ACCESS_NETWORK_INFO))
            {
                nType = SipHeaderBase::P_ACCESS_NETWORK_INFO;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_CHARGING_VECTOR))
            {
                nType = SipHeaderBase::P_CHARGING_VECTOR;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::PROXY_AUTHENTICATE))
            {
                nType = SipHeaderBase::PROXY_AUTHENTICATE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::PROXY_AUTHORIZATION))
            {
                nType = SipHeaderBase::PROXY_AUTHORIZATION;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::PRIVACY))
            {
                nType = SipHeaderBase::PRIVACY;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_PREFERRED_IDENTITY))
            {
                nType = SipHeaderBase::P_PREFERRED_IDENTITY;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_ASSERTED_IDENTITY))
            {
                nType = SipHeaderBase::P_ASSERTED_IDENTITY;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_EARLY_MEDIA))
            {
                nType = SipHeaderBase::P_EARLY_MEDIA;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_ANSWER_STATE))
            {
                nType = SipHeaderBase::P_ANSWER_STATE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_MEDIA_AUTHORIZATION))
            {
                nType = SipHeaderBase::P_MEDIA_AUTHORIZATION;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_PROFILE_KEY))
            {
                nType = SipHeaderBase::P_PROFILE_KEY;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_REFUSED_URI_LIST))
            {
                nType = SipHeaderBase::P_REFUSED_URI_LIST;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_SERVED_USER))
            {
                nType = SipHeaderBase::P_SERVED_USER;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_USER_DATABASE))
            {
                nType = SipHeaderBase::P_USER_DATABASE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::PERMISSION_MISSING))
            {
                nType = SipHeaderBase::PERMISSION_MISSING;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::PRIORITY))
            {
                nType = SipHeaderBase::PRIORITY;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::PRIV_ANSWER_MODE))
            {
                nType = SipHeaderBase::PRIV_ANSWER_MODE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::PROXY_REQUIRE))
            {
                nType = SipHeaderBase::PROXY_REQUIRE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_ASSERTED_SERVICE))
            {
                nType = SipHeaderBase::P_ASSERTED_SERVICE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::P_PREFERRED_SERVICE))
            {
                nType = SipHeaderBase::P_PREFERRED_SERVICE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::POLICY_CONTACT))
            {
                nType = SipHeaderBase::POLICY_CONTACT;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::POLICY_ID))
            {
                nType = SipHeaderBase::POLICY_ID;
            }
        }
        break;

        case 'r':
        case 'R':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::REQUIRE))
            {
                nType = SipHeaderBase::REQUIRE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::REFERRED_BY))
            {
                nType = SipHeaderBase::REFERRED_BY;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::CF_REFER_TO) ||
                    strName.EqualsIgnoreCase(SipHeaderName::REFER_TO))
            {
                nType = SipHeaderBase::REFER_TO;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::REQUEST_DISPOSITION))
            {
                nType = SipHeaderBase::REQUEST_DISPOSITION;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::REJECT_CONTACT))
            {
                nType = SipHeaderBase::REJECT_CONTACT;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::REPLACES))
            {
                nType = SipHeaderBase::REPLACES;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::RACK))
            {
                nType = SipHeaderBase::RACK;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::RECORD_ROUTE))
            {
                nType = SipHeaderBase::RECORD_ROUTE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::RECV_INFO))
            {
                nType = SipHeaderBase::RECV_INFO;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::ROUTE))
            {
                nType = SipHeaderBase::ROUTE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::RSEQ))
            {
                nType = SipHeaderBase::RSEQ;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::RETRY_AFTER))
            {
                nType = SipHeaderBase::RETRY_AFTER_SEC;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::RESOURCE_PRIORITY))
            {
                nType = SipHeaderBase::RESOURCE_PRIORITY;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::RESPONSE_KEY))
            {
                nType = SipHeaderBase::RESPONSE_KEY;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::REASON))
            {
                nType = SipHeaderBase::REASON;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::REPLY_TO))
            {
                nType = SipHeaderBase::REPLY_TO;
            }
        }
        break;

        case 's':
        case 'S':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_SUBJECT) ||
                    strName.EqualsIgnoreCase(SipHeaderName::SUBJECT))
            {
                nType = SipHeaderBase::SUBJECT;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::SUPPORTED))
            {
                nType = SipHeaderBase::SUPPORTED;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::SIP_IF_MATCH))
            {
                nType = SipHeaderBase::SIP_IF_MATCH;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::SIP_ETAG))
            {
                nType = SipHeaderBase::SIP_ETAG;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::SERVICE_ROUTE))
            {
                nType = SipHeaderBase::SERVICE_ROUTE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::SESSION_EXPIRES))
            {
                nType = SipHeaderBase::SESSION_EXPIRES;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::SUBSCRIPTION_STATE))
            {
                nType = SipHeaderBase::SUBSCRIPTION_STATE;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::SECURITY_CLIENT))
            {
                nType = SipHeaderBase::SECURITY_CLIENT;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::SECURITY_VERIFY))
            {
                nType = SipHeaderBase::SECURITY_VERIFY;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::SECURITY_SERVER))
            {
                nType = SipHeaderBase::SECURITY_SERVER;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::SERVER))
            {
                nType = SipHeaderBase::SERVER;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::SUPPRESS_IF_MATCH))
            {
                nType = SipHeaderBase::SUPPRESS_IF_MATCH;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::SESSION_ID))
            {
                nType = SipHeaderBase::SESSION_ID;
            }
        }
        break;

        case 't':
        case 'T':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_TO) ||
                    strName.EqualsIgnoreCase(SipHeaderName::TO))
            {
                nType = SipHeaderBase::TO;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::TIMESTAMP))
            {
                nType = SipHeaderBase::TIMESTAMP;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::TRIGGER_CONSENT))
            {
                nType = SipHeaderBase::TRIGGER_CONSENT;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::TARGET_DIALOG))
            {
                nType = SipHeaderBase::TARGET_DIALOG;
            }
        }
        break;

        case 'u':
        case 'U':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_ALLOW_EVENTS))
            {
                nType = SipHeaderBase::ALLOW_EVENTS;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::UNSUPPORTED))
            {
                nType = SipHeaderBase::UNSUPPORTED;
            }
            else if (strName.EqualsIgnoreCase(SipHeaderName::USER_AGENT))
            {
                nType = SipHeaderBase::USER_AGENT;
            }
        }
        break;

        case 'v':
        case 'V':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_VIA) ||
                    strName.EqualsIgnoreCase(SipHeaderName::VIA))
            {
                nType = SipHeaderBase::VIA;
            }
        }
        break;

        case 'w':
        case 'W':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::WARNING))
            {
                nType = SipHeaderBase::WARNING;
            }
            if (strName.EqualsIgnoreCase(SipHeaderName::WWW_AUTHENTICATE))
            {
                nType = SipHeaderBase::WWW_AUTHENTICATE;
            }
        }
        break;

        case 'x':
        case 'X':
        {
            if (strName.EqualsIgnoreCase(SipHeaderName::CF_SESSION_EXPIRES))
            {
                nType = SipHeaderBase::SESSION_EXPIRES;
            }
        }
        break;

        default:
            break;
    }

    return nType;
}

GLOBAL SipAddrSpec* GetAddrSpec(IN SipHeaderBase* pHeader)
{
    if (pHeader == SIP_NULL)
    {
        return IMS_NULL;
    }

    if (!IsAddressFormatHeader(pHeader->GetHdrType(), IMS_NULL))
    {
        return IMS_NULL;
    }

    SipNameAddrHeader* pAddrHeader = DYNAMIC_CAST(SipNameAddrHeader*, pHeader);
    SipNameAddr* pNameAddr = pAddrHeader->GetNameAddr();

    if (pNameAddr == IMS_NULL)
    {
        return IMS_NULL;
    }

    SipAddrSpec* pAddrSpec = pNameAddr->GetAddrSpec();

    pNameAddr->SipDelete();

    return pAddrSpec;
}

GLOBAL SipAddrSpec* GetAddrSpec(
        IN ::SipMessage* pMessage, IN IMS_SINT32 nType, IN IMS_UINT32 nIndex /*= 0*/)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    if (!IsAddressFormatHeader(nType, IMS_NULL))
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    SipHeaderBase* pHeader = pMessage->GetMsgHdrs()->GetHdrObj(nType, nIndex);

    if (pHeader == IMS_NULL)
    {
        if (nType == SipHeaderBase::ROUTE)
        {
            SIPStackError(EERR_NOEXISTS);
        }
        else
        {
            IMS_TRACE_D("GetAddrSpec: Header is not found", 0, 0, 0);
            SIPStackError(EERR_INVALIDPARAM);
        }

        return IMS_NULL;
    }

    SipNameAddrHeader* pAddrHeader = DYNAMIC_CAST(SipNameAddrHeader*, pHeader);

    SipAddrSpec* pAddrSpec = IMS_NULL;

    if (pAddrHeader != SIP_NULL)
    {
        SipNameAddr* pNameAddr = pAddrHeader->GetNameAddr();

        if (pNameAddr != IMS_NULL)
        {
            pAddrSpec = pNameAddr->GetAddrSpec();

            pNameAddr->SipDelete();
        }
    }

    pHeader->SipDelete();

    return pAddrSpec;
}

GLOBAL AString GetChallengeScheme(IN SipHeaderBase* pHeader)
{
    if (pHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    if ((pHeader->GetHdrType() != SipHeaderBase::WWW_AUTHENTICATE) &&
            (pHeader->GetHdrType() != SipHeaderBase::PROXY_AUTHENTICATE))
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    SIPStackError(EERR_NOERR);

    const SipAuthBase* pAuthHeader = DYNAMIC_CAST(SipAuthBase*, pHeader);
    AString strScheme = pAuthHeader->GetValue();

    return strScheme;
}

GLOBAL IMS_BOOL GetContent(
        IN const SipMsgBody* pMsgBody, OUT IMS_BYTE*& pContent, OUT IMS_SINT32& nContentLength)
{
    SIPStackError(EERR_NOERR);

    if (pMsgBody == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    pContent = reinterpret_cast<IMS_BYTE*>(pMsgBody->GetBuffer());
    nContentLength = static_cast<IMS_SINT32>(pMsgBody->GetBufferLength());

    return IMS_TRUE;
}

GLOBAL IMS_UINT32 GetCSeqNumber(IN ::SipMessage* pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        IMS_TRACE_D("GetCSeqNumber: Message is null", 0, 0, 0);
        SIPStackError(EERR_INVALIDPARAM);
        return SipPrivate::INVALID_SEQ_NUM;
    }

    SipCSeqHeader* pCseqHeader =
            DYNAMIC_CAST(SipCSeqHeader*, pMessage->GetHdrObj(SipHeaderBase::CSEQ));

    if (pCseqHeader == IMS_NULL)
    {
        IMS_TRACE_D("GetCSeqNumber: CSeq header is null", 0, 0, 0);
        return SipPrivate::INVALID_SEQ_NUM;
    }

    IMS_UINT32 nSeqNum = pCseqHeader->GetCSeq();

    pCseqHeader->SipDelete();

    return nSeqNum;
}

GLOBAL IMS_BOOL GetEventHeader(
        IN ::SipMessage* pMessage, OUT AString& strEvent, OUT AString& strEventId)
{
    SipEventHeader* pHeader =
            DYNAMIC_CAST(SipEventHeader*, GetHeader(pMessage, SipHeaderBase::EVENT));

    if (!IsValidHeader(pHeader))
    {
        return IMS_FALSE;
    }

    strEvent = pHeader->GetValue();

    strEventId = GetParameter(pHeader, AString("id"));

    FreeHeader(pHeader);

    return IMS_TRUE;
}

GLOBAL SipHeaderBase* GetHeader(
        IN ::SipMessage* pMessage, IN IMS_SINT32 nType, IN IMS_UINT32 nIndex /*= 0*/)
{
    nType = GetHdrEnumType(nType);

    SipHeaderList* pHeaderList = pMessage->GetHdrList(nType);
    SipHeaderBase* pHeader = IMS_NULL;

    if (pHeaderList == IMS_NULL)
    {
        pHeader = pMessage->GetHdrObj(nType);
    }
    else
    {
        pHeader = pHeaderList->GetObj(nIndex);

        pHeaderList->SipDelete();
    }

    return pHeader;
}

GLOBAL AString GetHeaderAsString(IN ::SipMessage* pMessage, IN IMS_SINT32 nType,
        IN IMS_BOOL bParams /*= IMS_FALSE*/, IN IMS_UINT32 nIndex /*= 0*/)
{
    SipHeaderBase* pHeader = GetHeader(pMessage, nType, nIndex);

    if (pHeader == SIP_NULL)
    {
        return AString::ConstNull();
    }

    AString strHeaderBody(AString::ConstNull());

    if (!EncodeHeaderBody(pHeader, bParams, strHeaderBody))
    {
        FreeHeader(pHeader);
        return AString::ConstNull();
    }

    FreeHeader(pHeader);

    return strHeaderBody;
}

GLOBAL IMS_SINT32 GetHeaderCount(IN ::SipMessage* pMessage, IN IMS_SINT32 nType)
{
    nType = GetHdrEnumType(nType);

    SipHeaderBase* pHeader = pMessage->GetHdrList(nType);

    if (pHeader == SIP_NULL)
    {
        pHeader = pMessage->GetHdrObj(nType);

        if (pHeader == SIP_NULL)
        {
            return SIP_ZERO;
        }

        pHeader->SipDelete();
        return SIP_ONE;
    }

    const SipHeaderList* pHeaderList = DYNAMIC_CAST(SipHeaderList*, pHeader);
    IMS_SINT32 nHeaderCount = pHeaderList->GetSize();

    pHeader->SipDelete();

    return nHeaderCount;
}

GLOBAL IMS_BOOL GetHostAndPort(
        IN SipAddrSpec* pAddrSpec, OUT AString& strHost, OUT IMS_UINT32& nPort)
{
    SIPStackError(EERR_NOERR);

    nPort = Sip::PORT_UNSPECIFIED;

    if ((pAddrSpec->GetUriScheme() == SipUri::SCHEME_SIP) ||
            (pAddrSpec->GetUriScheme() == SipUri::SCHEME_SIPS))
    {
        SipUri* pSipUri = pAddrSpec->GetSipUri();

        if (pSipUri != SIP_NULL)
        {
            strHost = pSipUri->GetHost();
            nPort = pSipUri->GetPort();

            if (nPort == Sip::PORT_UNSPECIFIED)
            {
                SIPStackError(EERR_NOEXISTS);
            }

            pSipUri->SipDelete();
            return IMS_TRUE;
        }
    }

    SIPStackError(EERR_NOEXISTS);

    return IMS_FALSE;
}

GLOBAL IMS_BOOL GetHostNPortFromViaHeader(
        IN ::SipMessage* pMessage, OUT AString& strHost, OUT IMS_SINT32& nPort)
{
    strHost = AString::ConstNull();
    nPort = Sip::PORT_UNSPECIFIED;

    // Get the topmost Via header from the message
    SipViaHeader* pViaHeader = DYNAMIC_CAST(SipViaHeader*, GetHeader(pMessage, SipHeaderBase::VIA));

    if (pViaHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    strHost = pViaHeader->GetHost();

    IMS_SINT32 nPos;

    if ((nPos = strHost.GetIndexOf(TextParser::CHAR_LSBRACKET)) != AString::NPOS)
    {
        // Strip the '[' , ']' before resolving the address.
        IMS_SINT32 nEndOfHost = strHost.GetIndexOf(TextParser::CHAR_RSBRACKET);

        strHost = strHost.GetSubStr(nPos + 1, nEndOfHost - nPos - 1);
    }

    nPort = pViaHeader->GetPort();

    if (nPort == 0 || nPort == Sip::PORT_UNSPECIFIED)
    {
        const IMS_CHAR* pszTransport = pViaHeader->GetTransport();

        if (IMS_StrICmp(pszTransport, Sip::STR_TLS_CAPS) == 0)
        {
            nPort = Sip::PORT_5061;
        }
        else
        {
            nPort = Sip::PORT_5060;
        }
    }

    // Free the local reference
    FreeHeader(pViaHeader);

    return IMS_TRUE;
}

GLOBAL SipMethod GetMethod(IN ::SipMessage* pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return SipMethod();
    }

    SipMethod objSipMethod(pMessage->GetMethod());

    return objSipMethod;
}

GLOBAL IMS_SINT32 GetMessageBodyCount(IN const ::SipMessage* pMessage)
{
    return pMessage->GetMsgBodyCount();
}

GLOBAL SipMsgBody* GetMessageBody(IN ::SipMessage* pMessage, IN IMS_SINT32 nIndex /*= 0*/)
{
    return pMessage->GetMsgBody(nIndex);
}

GLOBAL AString GetMimeHeader(
        IN SipMsgBody* pMsgBody, IN IMS_SINT32 nType, IN IMS_SINT32 nIndex /*= 0*/)
{
    IMS_TRACE_D("GetMimeHeader: type=%d", nType, 0, 0);

    SIPStackError(EERR_NOERR);

    if (pMsgBody == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    SipHeaderBase* pHeader = IMS_NULL;

    switch (nType)
    {
        case SipMessageBodyPart::CONTENT_TYPE:
            pHeader = pMsgBody->GetContentType();
            break;
        case SipMessageBodyPart::CONTENT_DISPOSITION:
            pHeader = pMsgBody->GetContentDisposition();
            break;
        case SipMessageBodyPart::CONTENT_TRANSFER_ENCODING:
            pHeader = pMsgBody->GetContentEncoding();
            break;
        case SipMessageBodyPart::CONTENT_UNKNOWN:
            pHeader = pMsgBody->GetMimeHdr(SipHeaderBase::UNKNOWN, nIndex);
            break;
        case SipMessageBodyPart::CONTENT_ID:
        {
            IMS_UINT32 nCount = pMsgBody->GetUnknownHdrCount();

            for (IMS_UINT32 i = 0; i < nCount; ++i)
            {
                pHeader = pMsgBody->GetMimeHdr(SipHeaderBase::UNKNOWN, i);

                AString strName = GetUnknownHeaderName(pHeader);

                if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_ID))
                {
                    break;
                }
                else
                {
                    FreeHeaderEx(pHeader);
                }
            }
            break;
        }
        case SipMessageBodyPart::CONTENT_DESCRIPTION:
        {
            IMS_UINT32 nCount = pMsgBody->GetUnknownHdrCount();

            for (IMS_UINT32 i = 0; i < nCount; ++i)
            {
                pHeader = pMsgBody->GetMimeHdr(SipHeaderBase::UNKNOWN, i);
                AString strName = GetUnknownHeaderName(pHeader);

                if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_DESCRIPTION))
                {
                    break;
                }
                else
                {
                    FreeHeaderEx(pHeader);
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }

    if (pHeader != IMS_NULL)
    {
        AString strHeader;

        EncodeHeaderBody(pHeader, IMS_TRUE, strHeader);

        FreeHeader(pHeader);

        return strHeader;
    }

    return AString::ConstNull();
}

GLOBAL IMS_SINT32 GetMimeHeaderCount(IN SipMsgBody* pMsgBody, IN IMS_SINT32 nType)
{
    if (pMsgBody == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return 0;
    }

    SIPStackError(EERR_NOERR);

    SipHeaderBase* pHeader = IMS_NULL;

    switch (nType)
    {
        case SipMessageBodyPart::CONTENT_TYPE:
            pHeader = pMsgBody->GetContentType();
            break;
        case SipMessageBodyPart::CONTENT_DISPOSITION:
            pHeader = pMsgBody->GetContentDisposition();
            break;
        case SipMessageBodyPart::CONTENT_TRANSFER_ENCODING:
            pHeader = pMsgBody->GetContentEncoding();
            break;
        case SipMessageBodyPart::CONTENT_UNKNOWN:
            return pMsgBody->GetUnknownHdrCount();
        case SipMessageBodyPart::CONTENT_ID:
        {
            IMS_UINT32 nCount = pMsgBody->GetUnknownHdrCount();

            for (IMS_UINT32 i = 0; i < nCount; ++i)
            {
                pHeader = pMsgBody->GetMimeHdr(SipHeaderBase::UNKNOWN, i);
                AString strName = GetUnknownHeaderName(pHeader);

                if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_ID))
                {
                    break;
                }
                else
                {
                    FreeHeaderEx(pHeader);
                }
            }
            break;
        }
        case SipMessageBodyPart::CONTENT_DESCRIPTION:
        {
            IMS_UINT32 nCount = pMsgBody->GetUnknownHdrCount();

            for (IMS_UINT32 i = 0; i < nCount; ++i)
            {
                pHeader = pMsgBody->GetMimeHdr(SipHeaderBase::UNKNOWN, i);
                AString strName = GetUnknownHeaderName(pHeader);

                if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_DESCRIPTION))
                {
                    break;
                }
                else
                {
                    FreeHeaderEx(pHeader);
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }

    if (pHeader != IMS_NULL)
    {
        FreeHeader(pHeader);
        return 1;
    }

    return 0;
}

GLOBAL AString GetParameter(
        IN SipAddrSpec* pAddrSpec, IN const AString& strName, IN IMS_UINT32 nIndex /*= 0*/)
{
    if ((pAddrSpec->GetUriScheme() == SipUri::SCHEME_SIP) ||
            (pAddrSpec->GetUriScheme() == SipUri::SCHEME_SIPS))
    {
        SipUri* pSipUri = pAddrSpec->GetSipUri();

        if (pSipUri == SIP_NULL)
        {
            return AString::ConstNull();
        }

        if (pSipUri->GetUriParamCount() == 0)
        {
            pSipUri->SipDelete();
            return AString::ConstNull();
        }

        IMS_CHAR* pszValue = pSipUri->GetUriParamValue(strName.GetStr(), nIndex);
        AString strValue(pszValue);

        DeleteStackString(pszValue);
        pSipUri->SipDelete();

        return strValue;
    }
    else
    {
        // Tel URL
        return AString::ConstNull();
    }

    return AString::ConstNull();
}

GLOBAL AString GetParameter(
        IN SipHeaderBase* pHeader, IN const AString& strName, IN IMS_UINT32 nIndex /*= 0*/)
{
    (void)nIndex;
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    IMS_CHAR* pszValue = IMS_NULL;
    IMS_SINT32 nHdrType = pHeader->GetHdrType();

    if ((nHdrType == SipHeaderBase::WWW_AUTHENTICATE) ||
            (nHdrType == SipHeaderBase::PROXY_AUTHENTICATE) ||
            (nHdrType == SipHeaderBase::PROXY_AUTHORIZATION) ||
            (nHdrType == SipHeaderBase::AUTHORIZATION))
    {
        // Auth header parameters
        SipAuthBase* pAuthHeader = DYNAMIC_CAST(SipAuthBase*, pHeader);

        pszValue = pAuthHeader->GetAuthValue(strName.GetStr());
    }
    else
    {
        pszValue = pHeader->GetParamValue(strName.GetStr());
    }

    if (pszValue == SIP_NULL)
    {
        SIPStackError(EERR_NOEXISTS);
        return AString::ConstNull();
    }

    AString strValue(pszValue);

    DeleteStackString(pszValue);

    return strValue;
}

GLOBAL SipAddrSpec* GetRequestUri(IN ::SipMessage* pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage->GetMsgType() != ::SipMessage::REQ_TYPE)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    SipRequestLine* pReqLine = pMessage->GetReqLine();

    if (pReqLine == SIP_NULL)
    {
        return IMS_NULL;
    }

    SipAddrSpec* pAddrSpec = pReqLine->GetReqUri();

    pReqLine->SipDelete();

    return pAddrSpec;
}

GLOBAL AString GetSentByFromVia(IN SipHeaderBase* pHeader)
{
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        IMS_TRACE_E(0, "Header is null", 0, 0, 0);
        return AString::ConstNull();
    }

    if (pHeader->GetHdrType() != SipHeaderBase::VIA)
    {
        IMS_TRACE_E(0, "Invalid header type", 0, 0, 0);
        return AString::ConstNull();
    }

    const SipViaHeader* pViaHeader = DYNAMIC_CAST(SipViaHeader*, pHeader);

    if (pViaHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    AString strSentby(pViaHeader->GetHost());

    IMS_UINT16 nPort = pViaHeader->GetPort();

    if (nPort != 0 && nPort != Sip::PORT_UNSPECIFIED)
    {
        strSentby.Append(':');

        AString strPort;
        strPort.SetNumber(nPort, 10);

        strSentby.Append(strPort);
    }

    return strSentby;
}

GLOBAL AString GetSentProtocolFromVia(IN SipHeaderBase* pHeader)
{
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        return AString::ConstNull();
    }

    if (pHeader->GetHdrType() != SipHeaderBase::VIA)
    {
        return AString::ConstNull();
    }

    SipViaHeader* pViaHeader = DYNAMIC_CAST(SipViaHeader*, pHeader);

    if (pViaHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    const IMS_CHAR* pszProtocolName = pViaHeader->GetProtocolName();
    const IMS_CHAR* pszProtocolVersion = pViaHeader->GetProtocolVer();
    const IMS_CHAR* pszTransport = pViaHeader->GetTransport();

    AString strSentProtocol;

    strSentProtocol.Sprintf("%s/%s/%s", (pszProtocolName != SIP_NULL) ? pszProtocolName : "SIP",
            (pszProtocolVersion != SIP_NULL) ? pszProtocolVersion : Sip::STR_SIP_VERSION_ONLY,
            (pszTransport != SIP_NULL) ? pszTransport : "UDP");

    return strSentProtocol;
}

GLOBAL AString GetSipVersion(IN ::SipMessage* pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    if (pMessage->GetMsgType() == ::SipMessage::REQ_TYPE)
    {
        SipRequestLine* pReqLine = pMessage->GetReqLine();

        if (pReqLine == IMS_NULL)
        {
            return AString::ConstNull();
        }

        AString strVersion(pReqLine->GetSipVersion());

        pReqLine->SipDelete();

        return strVersion;
    }
    else
    {
        SipStatusLine* pStatusLine = pMessage->GetStatusLine();

        if (pStatusLine == IMS_NULL)
        {
            return AString::ConstNull();
        }

        AString strVersion(pStatusLine->GetSipVersion());

        pStatusLine->SipDelete();

        return strVersion;
    }
}

GLOBAL IMS_SINT32 GetStatusCode(IN ::SipMessage* pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return SipStatusCode::SC_INVALID;
    }

    if (pMessage->GetMsgType() != ::SipMessage::RESP_TYPE)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return SipStatusCode::SC_INVALID;
    }

    SipStatusLine* pStatusLine = pMessage->GetStatusLine();

    if (pStatusLine == IMS_NULL)
    {
        return SipStatusCode::SC_INVALID;
    }

    IMS_SINT16 nStatusCode = SipStatusCode::SC_INVALID;

    pStatusLine->GetStatusCode(&nStatusCode);

    pStatusLine->SipDelete();

    return nStatusCode;
}

GLOBAL SipStatusCode GetStatusCodeEx(IN ::SipMessage* pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return SipStatusCode();
    }

    SipStatusLine* pStatusLine = pMessage->GetStatusLine();

    if (pStatusLine == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return SipStatusCode();
    }

    IMS_SINT32 nStatusCode = SipStatusCode::SC_INVALID;
    const IMS_CHAR* pszReasonPhrase = pStatusLine->GetReasonPhrase();
    const IMS_CHAR* pszStatusCode = pStatusLine->GetStatusCode();

    if (pszStatusCode != IMS_NULL)
    {
        nStatusCode = IMS_Atoi(pszStatusCode);
    }

    SipStatusCode objStatusCode(nStatusCode, pszReasonPhrase);

    pStatusLine->SipDelete();

    return objStatusCode;
}

GLOBAL IMS_BOOL GetSubscriptionStateHeader(IN ::SipMessage* pMessage, OUT AString& strSubsState,
        OUT IMS_SINT32* pnExpires /*= IMS_NULL*/)
{
    SipHeaderBase* pSubState = GetHeader(pMessage, SipHeaderBase::SUBSCRIPTION_STATE);

    if (pSubState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!IsValidHeader(pSubState))
    {
        return IMS_FALSE;
    }

    // Set the subs-state
    strSubsState = pSubState->GetValue();

    if (strSubsState.IsEmpty())
    {
        FreeHeader(pSubState);
        return IMS_FALSE;
    }

    // Set the expires parameter
    if (pnExpires != IMS_NULL)
    {
        AString strExpires = GetParameter(pSubState, Sip::STR_EXPIRES);

        if (strExpires.GetLength() == 0)
        {
            (*pnExpires) = (-1);
        }
        else
        {
            IMS_BOOL bOk = IMS_TRUE;

            (*pnExpires) = strExpires.ToInt32(&bOk);

            if (!bOk)
            {
                (*pnExpires) = (-1);
            }
        }
    }

    FreeHeader(pSubState);

    return IMS_TRUE;
}

GLOBAL AString GetUnknownHeaderName(IN const SipHeaderBase* pHeader)
{
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    if (pHeader->GetHdrType() != SipHeaderBase::UNKNOWN)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    const SipUnknownHeader* pUnknown = DYNAMIC_CAST(const SipUnknownHeader*, pHeader);
    AString strName(pUnknown->GetHeaderName());

    return strName;
}

GLOBAL AString GetUnknownHeaderBody(IN const SipHeaderBase* pHeader)
{
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    if (pHeader->GetHdrType() != SipHeaderBase::UNKNOWN)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    const SipUnknownHeader* pUnknown = DYNAMIC_CAST(const SipUnknownHeader*, pHeader);
    AString strValue(pUnknown->GetHeaderValue());

    return strValue;
}

GLOBAL AString GetViaBranchParameter(IN ::SipMessage* pMessage)
{
    SipHeaderBase* pHeader = GetHeader(pMessage, SipHeaderBase::VIA);

    if (!IsValidHeader(pHeader))
    {
        return AString::ConstNull();
    }

    AString strViaBranch = GetParameter(pHeader, Sip::STR_BRANCH);

    FreeHeader(pHeader);

    return strViaBranch;
}

GLOBAL IMS_SINT32 GetHdrEnumType(IN IMS_SINT32 nType)
{
    return SipMsgUtil::CheckAndGetHeaderType(nType);
}

GLOBAL IMS_BOOL HasParameter(IN SipHeaderBase* pHeader, IN const AString& strName)
{
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (pHeader->IsParamPresent(strName.GetStr()) == SIP_TRUE) ? IMS_TRUE : IMS_FALSE;
}

GLOBAL IMS_BOOL HasParameter(IN SipAddrSpec* pAddrSpec, IN const AString& strName)
{
    SIPStackError(EERR_NOERR);

    if (pAddrSpec == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    if ((pAddrSpec->GetUriScheme() == SipUri::SCHEME_SIP) ||
            (pAddrSpec->GetUriScheme() == SipUri::SCHEME_SIPS))
    {
        SipUri* pSipUri = pAddrSpec->GetSipUri();

        if (pSipUri == SIP_NULL)
        {
            return IMS_FALSE;
        }

        IMS_UINT32 nCount = pSipUri->GetUriParamCount();

        for (IMS_UINT32 i = 0; i < nCount; ++i)
        {
            const SipNameValue* pNameVal = pSipUri->GetUriParam(i);

            if (pNameVal == SIP_NULL)
            {
                continue;
            }

            if (strName.EqualsIgnoreCase(pNameVal->m_pszName))
            {
                pSipUri->SipDelete();
                return IMS_TRUE;
            }
        }

        nCount = pSipUri->GetHdrParamCount();

        for (IMS_UINT32 i = 0; i < nCount; ++i)
        {
            const SipNameValue* pNameVal = pSipUri->GetHdrParam(i);

            if (pNameVal == SIP_NULL)
            {
                continue;
            }

            if (strName.EqualsIgnoreCase(pNameVal->m_pszName))
            {
                pSipUri->SipDelete();
                return IMS_TRUE;
            }
        }

        pSipUri->SipDelete();
    }

    return IMS_FALSE;
}

GLOBAL IMS_BOOL HasMimeMessageBody(IN ::SipMessage* pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    return pMessage->HasMIMEMessageBody();
}

GLOBAL IMS_BOOL HasSdpMessageBody(IN ::SipMessage* pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    return pMessage->HasSDPMessageBody();
}

GLOBAL IMS_BOOL InsertHeader(
        IN SipHeaderBase* pHeader, IN IMS_UINT32 nIndex, IN_OUT ::SipMessage*& pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    if (pMessage->InsertHeader(pHeader, nIndex) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

GLOBAL IMS_BOOL IsHeaderPresent(IN ::SipMessage* pMessage, IN IMS_SINT32 nType)
{
    SipHeaderBase* pHeader = GetHeader(pMessage, nType);

    if (pHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    FreeHeader(pHeader);

    return IMS_TRUE;
}

GLOBAL IMS_BOOL IsMessageBodySdp(IN SipMsgBody* pMsgBody)
{
    if (pMsgBody == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    SIPStackError(EERR_NOERR);

    return pMsgBody->IsMessageBodySDP();
}

GLOBAL IMS_BOOL IsMessageRpr(IN ::SipMessage* pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // If the message is not of type response, return FALSE
    if (pMessage->GetMsgType() != ::SipMessage::RESP_TYPE)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nStatusCode = GetStatusCode(pMessage);

    // The message has a status code greater than 199; It is a final response.
    if (!SipStatusCode::IsProvisional(nStatusCode))
    {
        return IMS_FALSE;
    }

    // If the message is a provisional response,
    // but does not contain a RSeq header, then return FALSE.
    IMS_SINT32 nRseqCount = GetHeaderCount(pMessage, SipHeaderBase::RSEQ);

    if (nRseqCount == 0)
    {
        return IMS_FALSE;
    }

    IMS_BOOL bOptionTag100Rel = IMS_FALSE;
    IMS_SINT32 nCount = GetHeaderCount(pMessage, SipHeaderBase::REQUIRE);

    for (IMS_SINT32 i = 0; i < nCount; ++i)
    {
        SipHeaderBase* pRequireHeader = GetHeader(pMessage, SipHeaderBase::REQUIRE, i);

        const IMS_CHAR* pszOptionTag = pRequireHeader->GetValue();

        if (pszOptionTag != IMS_NULL)
        {
            if (AString::CompareIgnoreCase(pszOptionTag, Sip::STR_100REL) == 0)
            {
                bOptionTag100Rel = IMS_TRUE;
                FreeHeader(pRequireHeader);
                break;
            }
        }
        else
        {
            IMS_TRACE_I("100Rel Not Present", 0, 0, 0);
        }

        FreeHeader(pRequireHeader);
    }

    // 101 ~ 199 response, RSeq header, option-tag ("100rel") in Require header
    if ((nRseqCount > 0) && bOptionTag100Rel)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

GLOBAL IMS_BOOL IsOptionRequired(IN ::SipMessage* pMessage, IN const AString& strOption)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nCount = GetHeaderCount(pMessage, SipHeaderBase::REQUIRE);

    if (nCount == 0)
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < nCount; ++i)
    {
        SipHeaderBase* pRequireHeader = GetHeader(pMessage, SipHeaderBase::REQUIRE, i);
        const IMS_CHAR* pszOptionTag = pRequireHeader->GetValue();

        if (strOption.EqualsIgnoreCase(pszOptionTag))
        {
            FreeHeader(pRequireHeader);
            return IMS_TRUE;
        }

        FreeHeader(pRequireHeader);
    }

    return IMS_FALSE;
}

GLOBAL IMS_BOOL IsOptionSupported(IN ::SipMessage* pMessage, IN const AString& strOption)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nCount = GetHeaderCount(pMessage, SipHeaderBase::SUPPORTED);

    if (nCount == 0)
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < nCount; ++i)
    {
        SipHeaderBase* pSupportedHeader = GetHeader(pMessage, SipHeaderBase::SUPPORTED, i);
        const IMS_CHAR* pszOptionTag = pSupportedHeader->GetValue();

        if ((AString::Compare(pszOptionTag, "*") == 0) || strOption.EqualsIgnoreCase(pszOptionTag))
        {
            FreeHeader(pSupportedHeader);
            return IMS_TRUE;
        }

        FreeHeader(pSupportedHeader);
    }

    return IMS_FALSE;
}

GLOBAL IMS_BOOL IsRequestMessage(IN const ::SipMessage* pMessage)
{
    return (pMessage != IMS_NULL) ? (pMessage->GetMsgType() == ::SipMessage::REQ_TYPE) : IMS_FALSE;
}

GLOBAL IMS_BOOL IsAddressFormatHeader(IN IMS_SINT32 nType, IN const AString& strName)
{
    switch (nType)
    {
        case SipHeaderBase::CONTACT:               // FALL-THROUGH
        case SipHeaderBase::CONTACT_WILD:          // FALL-THROUGH
        case SipHeaderBase::CONTACT_ANY:           // FALL-THROUGH
        case SipHeaderBase::FROM:                  // FALL-THROUGH
        case SipHeaderBase::P_PREFERRED_IDENTITY:  // FALL-THROUGH
        case SipHeaderBase::P_ASSERTED_IDENTITY:   // FALL-THROUGH
        case SipHeaderBase::PATH:                  // FALL-THROUGH
        case SipHeaderBase::P_ASSOCIATED_URI:      // FALL-THROUGH
        case SipHeaderBase::P_CALLED_PARTY_ID:     // FALL-THROUGH
        case SipHeaderBase::SERVICE_ROUTE:         // FALL-THROUGH
        case SipHeaderBase::HISTORY_INFO:          // FALL-THROUGH
        case SipHeaderBase::RECORD_ROUTE:          // FALL-THROUGH
        case SipHeaderBase::REFERRED_BY:           // FALL-THROUGH
        case SipHeaderBase::REFER_TO:              // FALL-THROUGH
        case SipHeaderBase::ROUTE:                 // FALL-THROUGH
        case SipHeaderBase::TO:                    // FALL-THROUGH
        case SipHeaderBase::IDENTITY_INFO:         // FALL-THROUGH
        case SipHeaderBase::P_SERVED_USER:         // FALL-THROUGH
        case SipHeaderBase::POLICY_CONTACT:        // FALL-THROUGH
        case SipHeaderBase::POLICY_ID:             // FALL-THROUGH
        case SipHeaderBase::REPLY_TO:              // FALL-THROUGH
        case SipHeaderBase::TRIGGER_CONSENT:
            return IMS_TRUE;
        case SipHeaderBase::UNKNOWN:
            if (strName.EqualsIgnoreCase(SipHeaderName::DIVERSION))
            {
                return IMS_TRUE;
            }
            break;
        default:
            break;
    }

    return IMS_FALSE;
}

GLOBAL IMS_BOOL IsAquotRequiredForAddressFormat(IN IMS_SINT32 nType, IN const AString& strName)
{
    (void)strName;

    switch (nType)
    {
        case SipHeaderBase::IDENTITY_INFO:
        case SipHeaderBase::POLICY_CONTACT:
            return IMS_TRUE;
        default:
            break;
    }

    return IMS_FALSE;
}

GLOBAL IMS_BOOL OverwriteHeaders(IN ::SipMessage* pSrcMessage, IN_OUT ::SipMessage*& pDstMessage)
{
    SipHeaders* pDstHeaders = pDstMessage->GetMsgHdrs();

    // Overwrite known headers only
    pDstHeaders->OverWriteHdrObj(pSrcMessage->GetMsgHdrs(), SIP_TRUE);

    return IMS_TRUE;
}

GLOBAL void ParseHostNPort(
        IN const AString& strHostNPort, OUT AString& strHost, OUT IMS_SINT32& nPort)
{
    IMS_SINT32 nPos;

    nPort = Sip::PORT_UNSPECIFIED;

    // We need to take care of '[', ']' while extracting the host from IPv6 reference
    if ((nPos = strHostNPort.GetIndexOf(TextParser::CHAR_LSBRACKET)) != AString::NPOS)
    {
        // Strip the '[' , ']' before resolving the address.
        IMS_SINT32 nEndOfHost = strHostNPort.GetIndexOf(TextParser::CHAR_RSBRACKET);

        strHost = strHostNPort.GetSubStr(nPos + 1, nEndOfHost - nPos - 1);

        nPos = strHostNPort.GetIndexOf(TextParser::CHAR_COLON, nEndOfHost + 1);

        if (nPos != AString::NPOS)
        {
            AString strPort = strHostNPort.GetSubStr(nPos + 1, strHostNPort.GetLength() - nPos);

            IMS_BOOL bOk = IMS_FALSE;
            IMS_UINT16 nTmpPort = strPort.ToUInt16(&bOk);

            if (bOk)
            {
                nPort = nTmpPort;
            }
        }
    }
    else
    {
        nPos = strHostNPort.GetIndexOf(TextParser::CHAR_COLON);

        if (nPos != AString::NPOS)
        {
            strHost = strHostNPort.GetSubStr(0, nPos);
            AString strPort = strHostNPort.GetSubStr(nPos + 1, strHostNPort.GetLength() - nPos);

            IMS_BOOL bOk = IMS_FALSE;
            IMS_UINT16 nTmpPort = strPort.ToUInt16(&bOk);

            if (bOk)
            {
                nPort = nTmpPort;
            }
        }
        else
        {
            strHost = strHostNPort;
        }
    }
}

GLOBAL IMS_BOOL RemoveAllMessageBodies(IN_OUT ::SipMessage*& pMessage)
{
    if (pMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pMessage->RemoveAllMessageBodies();

    return IMS_TRUE;
}

GLOBAL IMS_BOOL RemoveHeader(IN IMS_SINT32 nType, IN_OUT ::SipMessage*& pMessage)
{
    nType = GetHdrEnumType(nType);

    if (pMessage->RemoveHdr(nType) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

GLOBAL void RemoveParameter(IN const AString& strName, IN_OUT SipHeaderBase*& pHeader)
{
    if (pHeader == IMS_NULL)
    {
        return;
    }

    pHeader->RemoveParam(strName.GetStr());
}

GLOBAL IMS_BOOL RemoveParameter(IN const AString& strName, IN_OUT SipAddrSpec*& pAddrSpec)
{
    SIPStackError(EERR_NOERR);

    if ((pAddrSpec->GetUriScheme() == SipUri::SCHEME_SIP) ||
            (pAddrSpec->GetUriScheme() == SipUri::SCHEME_SIPS))
    {
        SipUri* pSipUri = pAddrSpec->GetSipUri();

        if (pSipUri == IMS_NULL)
        {
            return IMS_FALSE;
        }

        pSipUri->RemoveHdrParam(strName.GetStr());
        pSipUri->SipDelete();

        return IMS_TRUE;
    }
    else
    {
        SIPStackError(EERR_NOEXISTS);
    }

    return IMS_FALSE;
}

GLOBAL void RemoveUserAndPassword(IN_OUT SipAddrSpec*& pAddrSpec)
{
    if ((pAddrSpec != IMS_NULL) &&
            ((pAddrSpec->GetUriScheme() == SipUri::SCHEME_SIP) ||
                    (pAddrSpec->GetUriScheme() == SipUri::SCHEME_SIPS)))
    {
        SipUri* pSipUri = pAddrSpec->GetSipUri();

        if (pSipUri != IMS_NULL)
        {
            pSipUri->SetUser(IMS_NULL);
            pSipUri->SetPassword(IMS_NULL);

            pSipUri->SipDelete();
        }
    }
}

GLOBAL IMS_BOOL SetChallengeScheme(IN const AString& strScheme, IN_OUT SipHeaderBase*& pHeader)
{
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    if ((pHeader->GetHdrType() != SipHeaderBase::AUTHORIZATION) &&
            (pHeader->GetHdrType() != SipHeaderBase::PROXY_AUTHORIZATION))
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    SipAuthBase* pAuthHeader = DYNAMIC_CAST(SipAuthBase*, pHeader);

    if (pAuthHeader->SetValue(strScheme.GetStr()) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

GLOBAL IMS_BOOL SetContent(
        IN const IMS_BYTE* pContent, IN IMS_SINT32 nContentLength, IN_OUT SipMsgBody*& pMsgBody)
{
    SIPStackError(EERR_NOERR);

    if (pMsgBody == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    IMS_TRACE_D("SetContent: msgBodyType=%d", pMsgBody->GetBodyType(), 0, 0);

    if (pMsgBody->GetBodyType() == SipMsgBody::INVALID_BODY)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    if (pMsgBody->SetMsgBuffer(reinterpret_cast<const SIP_CHAR*>(pContent), nContentLength) ==
            SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

GLOBAL IMS_BOOL SetHeader(IN SipHeaderBase* pHeader, IN_OUT ::SipMessage*& pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    IMS_SINT32 nType = pHeader->GetHdrType();

    if ((nType <= SipHeaderBase::TYPE_INVALID) || (nType >= SipHeaderBase::TYPE_END))
    {
        IMS_TRACE_D("SetHeader: Invalid header type(%d)", nType, 0, 0);
        return IMS_FALSE;
    }

    // The existing value of the header will be overwritten with new value.
    // Applied only for Contact header as of now.
    if (pMessage->SetHeader(pHeader) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

GLOBAL IMS_BOOL SetMethod(IN const SipMethod& objMethod, IN_OUT ::SipMessage*& pMessage)
{
    if (pMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (pMessage->GetMsgType() == ::SipMessage::REQ_TYPE)
    {
        SipRequestLine* pReqLine = pMessage->GetReqLine();

        if (pReqLine != IMS_NULL)
        {
            pReqLine->SetMethod(objMethod.ToString().GetStr());
            pReqLine->SipDelete();
            return IMS_TRUE;
        }
    }
    else
    {
        SipCSeqHeader* pCseqHeader =
                DYNAMIC_CAST(SipCSeqHeader*, pMessage->GetHdrObj(SipHeaderBase::CSEQ));

        if (pCseqHeader != IMS_NULL)
        {
            pCseqHeader->SetMethod(objMethod.ToString().GetStr());
            pCseqHeader->SipDelete();
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

GLOBAL IMS_BOOL SetMimeHeader(
        IN IMS_SINT32 nType, IN SipHeaderBase* pHeader, IN_OUT SipMsgBody*& pMsgBody)
{
    (void)nType;

    SIPStackError(EERR_NOERR);

    if ((pMsgBody == IMS_NULL) || (pHeader == IMS_NULL))
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    if (pMsgBody->SetMimeHdr(pHeader) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

GLOBAL IMS_BOOL SetMimeHeader(IN IMS_SINT32 nType, IN const AString& strName,
        IN const AString& strBody, IN_OUT SipMsgBody*& pMsgBody)
{
    SIPStackError(EERR_NOERR);

    if (pMsgBody == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    AString strHdrName(strName);

    switch (nType)
    {
        case ISipMessageBodyPart::CONTENT_TYPE:
        {
            nType = SipHeaderBase::CONTENT_TYPE;
            break;
        }
        case ISipMessageBodyPart::CONTENT_DISPOSITION:
        {
            nType = SipHeaderBase::CONTENT_DISPOSITION;
            break;
        }
        case ISipMessageBodyPart::CONTENT_TRANSFER_ENCODING:
        {
            nType = SipHeaderBase::CONTENT_ENCODING;
            break;
        }
        case ISipMessageBodyPart::CONTENT_ID:
        {
            strHdrName = SipHeaderName::CONTENT_ID;
            nType = SipHeaderBase::UNKNOWN;
            break;
        }
        case ISipMessageBodyPart::CONTENT_DESCRIPTION:
        {
            strHdrName = SipHeaderName::CONTENT_DESCRIPTION;
            nType = SipHeaderBase::UNKNOWN;
            break;
        }
        default:
        {
            nType = SipHeaderBase::UNKNOWN;
            break;
        }
    }

    SipHeaderBase* pHeader = DecodeHeader(nType, strHdrName, strBody);

    if (pHeader == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::PARSING_ERROR);
        return IMS_FALSE;
    }

    if (pMsgBody->SetMimeHdr(pHeader) == SIP_FALSE)
    {
        FreeHeader(pHeader);
        SipPrivate::SetLastError(SipError::PARSING_ERROR);
        return IMS_FALSE;
    }

    FreeHeader(pHeader);
    return IMS_TRUE;
}

GLOBAL IMS_BOOL SetParameter(
        IN SipHeaderBase* pHeader, IN const AString& strName, IN const AString& strValue)
{
    SIPStackError(EERR_NOERR);

    if (pHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    const IMS_CHAR* pszValue = (strValue.GetLength() > 0) ? strValue.GetStr() : IMS_NULL;

    if ((pHeader->GetHdrType() == SipHeaderBase::AUTHORIZATION) ||
            (pHeader->GetHdrType() == SipHeaderBase::PROXY_AUTHORIZATION))
    {
        SipAuthBase* pAuthHeader = DYNAMIC_CAST(SipAuthBase*, pHeader);
        return pAuthHeader->SetParams(strName.GetStr(), pszValue, SIP_FALSE);
    }
    else
    {
        if (pHeader->SetParam(strName.GetStr(), pszValue) == SIP_FALSE)
        {
            SIPStackError(EERR_NOEXISTS);
            return IMS_FALSE;
        }
    }
    return IMS_TRUE;
}

GLOBAL IMS_BOOL SetRequestLine(
        IN const AString& strMethod, IN const AString& strUri, IN_OUT ::SipMessage*& pMessage)
{
    SipAddrSpec* pAddrSpec = new SipAddrSpec();

    pAddrSpec->Decode(strUri.GetStr(), strUri.GetLength());

    SipRequestLine* pReqLine = new SipRequestLine(strMethod.GetStr(), pAddrSpec, SIP_SIPVER);

    pMessage->SetRequestline(pReqLine);

    return IMS_TRUE;
}

GLOBAL IMS_BOOL SetRequestUri(IN SipAddrSpec* pAddrSpec, IN_OUT ::SipMessage*& pMessage)
{
    SipRequestLine* pReqLine = pMessage->GetReqLine();

    if (pReqLine != IMS_NULL)
    {
        pReqLine->SetReqUri(pAddrSpec);
        pReqLine->SipDelete();
    }

    pMessage->SetMessageType(::SipMessage::REQ_TYPE);

    return IMS_TRUE;
}

GLOBAL IMS_BOOL SetStatusLine(IN IMS_SINT32 nStatusCode, IN const AString& strReasonPhrase,
        IN_OUT ::SipMessage*& pMessage)
{
    AString strStatusCode;

    strStatusCode.SetNumber(nStatusCode);

    SipStatusLine* pStatusLine =
            new SipStatusLine(SIP_SIPVER, strStatusCode.GetStr(), strReasonPhrase.GetStr());

    pMessage->SetStatusLine(pStatusLine);

    return IMS_TRUE;
}

GLOBAL IMS_BOOL IsMessageBodyCompressed(IN ::SipMessage* pMessage)
{
    // 4 Check if the Content-Encoding contains the compression algorithm or not
    // 4 As default, consider it to 'gzip'. It needs to be consider other compression algorithm.

    SipHeaderBase* pContentEncoding = GetHeader(pMessage, SipHeaderBase::CONTENT_ENCODING);

    if (IsValidHeader(pContentEncoding) == IMS_TRUE)
    {
        const IMS_CHAR* pszEncoding = pContentEncoding->GetValue();
        AString strEncoding(pszEncoding);

        FreeHeader(pContentEncoding);

        if (strEncoding.EqualsIgnoreCase("gzip"))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

GLOBAL IMS_BOOL UncompressMessageBody(IN ::SipMessage* pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        SIPStackError(EMSGERR_INVALIDHDRPARAM);
        return IMS_FALSE;
    }

    SipMsgBodyList* pMsgBodyList = pMessage->GetMsgBodyList();

    if (pMsgBodyList == IMS_NULL)
    {
        IMS_TRACE_E(0, "Retrieving message body list failed.", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_UINT32 nMsgBodyCount = pMsgBodyList->GetMsgBodyCount();

    if (nMsgBodyCount > 1)
    {
        SIPStackError(E_ERR_INVALIDBODY);
        IMS_TRACE_E(0, "There are many SIP message bodies", 0, 0, 0);
        pMsgBodyList->SipDelete();
        return IMS_FALSE;
    }

    if (nMsgBodyCount == 0)
    {
        IMS_TRACE_D("___ NO SIP MESSAGE BODY ___", 0, 0, 0);
        pMsgBodyList->SipDelete();
        return IMS_TRUE;
    }

    SipMsgBody* pMsgBody = pMsgBodyList->GetBodyByIndex(0);

    if (pMsgBody == IMS_NULL)
    {
        IMS_TRACE_E(0, "Getting SIP message body failed", 0, 0, 0);
        pMsgBodyList->SipDelete();
        return IMS_FALSE;
    }

    /*SIP considers the compressed body as single body*/
    if (pMsgBody->GetBodyType() != SipMsgBody::SINGLE_BODY)
    {
        IMS_TRACE_E(0, "Message body is not a single body, unable to uncompress.", 0, 0, 0);
        pMsgBody->SipDelete();
        pMsgBodyList->SipDelete();
        return IMS_FALSE;
    }

    IMS_CHAR* pszBuffer = IMS_NULL;

    pMsgBody->GetMsgBuffer(&pszBuffer);

    if (pszBuffer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Message buffer is null", 0, 0, 0);
        pMsgBody->SipDelete();
        pMsgBodyList->SipDelete();
        return IMS_FALSE;
    }

    // 4 Check if the Content-Encoding contains the compression algorithm or not
    // 4 As default, consider it to 'gzip'. It needs to be consider other compression algorithm.
    ByteArray objBodyPart;
    ByteArray objCompBodyPart;

    IMS_UINT32 nBuffLen = 0;
    pMsgBody->GetMsgBuffLen(&nBuffLen);
    objCompBodyPart.Attach(
            reinterpret_cast<const IMS_BYTE*>(pszBuffer), static_cast<IMS_SINT32>(nBuffLen));

    if (!IMS_UTIL_ZLIB_Uncompress(objCompBodyPart, objBodyPart))
    {
        IMS_TRACE_E(0, "Uncompressing a body part failed", 0, 0, 0);
        pMsgBody->SipDelete();
        pMsgBodyList->SipDelete();
        DeleteStackString(pszBuffer);
        return IMS_FALSE;
    }

    if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
    {
        IMS_TRACE_TEXT("gzip::uncompression", objBodyPart.GetData(), objBodyPart.GetLength());
    }

    pMsgBody->SetMsgBuffer(reinterpret_cast<const IMS_CHAR*>(objBodyPart.GetData()),
            static_cast<IMS_UINT32>(objBodyPart.GetLength()));

    pMsgBody->SipDelete();
    pMsgBodyList->SipDelete();
    DeleteStackString(pszBuffer);

    return IMS_TRUE;
}

GLOBAL IMS_BOOL UpdateSentProtocol(IN ::SipMessage* pMessage, IN const AString& strSentProtocol)
{
    SipViaHeader* pViaHeader = DYNAMIC_CAST(SipViaHeader*, GetHeader(pMessage, SipHeaderBase::VIA));
    const IMS_CHAR* pszProtocol = strSentProtocol.GetStr();
    IMS_UINT32 nStartIndex = 0;

    if (strSentProtocol.StartsWith(Sip::STR_SIP_VERSION))
    {
        // "SIP/2.0/" : 8 characters
        nStartIndex = 8;
    }

    pViaHeader->SetTransport(pszProtocol + nStartIndex);
    FreeHeader(pViaHeader);
    return IMS_TRUE;
}

GLOBAL void DisplayBadHeaders(IN ::SipMessage* pMessage)
{
#ifdef SIP_BADMESSAGE_PARSING
    SIPStackError(EERR_NOERR);

    IMS_TRACE_I("___ SIP bad headers ___", 0, 0, 0);

    SipHeaderList* pBadHdrList = pMessage->GetBadHdrs();

    if (pBadHdrList != IMS_NULL)
    {
        IMS_SINT32 nCount = pBadHdrList->GetSize();

        for (IMS_SINT32 i = 0; i < nCount; ++i)
        {
            SipBadHeader* pBadHeader = DYNAMIC_CAST(SipBadHeader*, pBadHdrList->GetObj(i));

            if (pBadHeader == IMS_NULL)
            {
                continue;
            }

            const IMS_CHAR* pszHdrName = pBadHeader->GetHeaderName();
            const IMS_CHAR* pszHdrValue = pBadHeader->GetValue();

            IMS_TRACE_I("    (%d) %s: %s", i, _TRACE_S_(pszHdrName), _TRACE_S_(pszHdrValue));

            pBadHeader->SipDelete();
        }
    }
#else
    (void)pMessage;
#endif
}

GLOBAL IMS_SINT32 GetBadHeaderCount(IN const ::SipMessage* pMessage)
{
#ifdef SIP_BADMESSAGE_PARSING
    return pMessage->GetBadHeaderCount();
#else
    (void)pMessage;
    return 0;
#endif
}

GLOBAL IMS_BOOL HasMandatoryHeaders(IN ::SipMessage* pMessage)
{
#ifdef SIP_BADMESSAGE_PARSING
    return pMessage->HasMandatoryHdrs();
#else
    (void)pMessage;
    return IMS_TRUE;
#endif
}

/// APIs for SIP authentication
GLOBAL IMS_BOOL GetEntityBody(IN ::SipMessage* pMessage, OUT AString& strEntityBody)
{
#ifdef SIP_TOBEPORTED
    IMS_UINT32 nMsgBodyCount = 0;
    RcPtr<SipMessageBuffer> pMessageBuffer = SipMessageBuffer::GetInstance();
    IMS_UINT32 nBuffLen = pMessageBuffer->GetLength();
    IMS_CHAR* pTmpBuffer = reinterpret_cast<IMS_CHAR*>(pMessageBuffer->GetBuffer());

    nMsgBodyCount = pMessage->GetMsgBodyCount();

    if (nMsgBodyCount == 0)
    {
        SIPStackError(EERR_NOEXISTS);
        return IMS_TRUE;
    }

    SipHeaderBase* pHeader = GetHeader(pMessage, SipHeaderBase::CONTENT_TYPE);

    if (GetLastError() != EERR_NOERR)
    {
        FreeHeader(pHeader);
        return IMS_FALSE;
    }

    IMS_MEM_Memset(pTmpBuffer, 0x00, nBuffLen);

    // Allocate a memory and initialize the entity-body buffer.
    // This will be filled with the entire entity-body and then passed to
    // IMSDigest_CalculateHEntity() to calculate the hash of the entity-body.
    if (sip_formMimeBody(IMS_NULL, pMessage->slMessageBody,
                static_cast<SipContentTypeHeader*>(stHeader.pHeader), pTmpBuffer, &nBuffLen,
                SIPStackError()) == SIP_FALSE)
    {
        FreeHeader(stHeader);
        return IMS_FALSE;
    }

    FreeHeader(stHeader);

    // sip_formMimeBody() returns the initial CRLF(\r\n) in the entity-body.
    // But, this initial CRLF should not be used in computing the hash (Entity).
    AString strTmp(&(pTmpBuffer[2]), nBuffLen - 2);

    strEntityBody = strTmp;
#else
    (void)pMessage;
    strEntityBody = AString::ConstNull();
#endif
    return IMS_TRUE;
}

/// APIs for SIP transaction layer
GLOBAL sipcore::SipTxnKey* CreateTxnKey(IN ::SipMessage* pMessage)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    SipViaHeader* pViaHeader = DYNAMIC_CAST(SipViaHeader*, GetHeader(pMessage, SipHeaderBase::VIA));

    if (pViaHeader == IMS_NULL)
    {
        return IMS_NULL;
    }

    AString strViaBranch = GetParameter(pViaHeader, Sip::STR_BRANCH);

    FreeHeader(pViaHeader);

    if (strViaBranch.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Via branch is null", 0, 0, 0);
        return IMS_NULL;
    }

    SipMethod objMethod = GetMethod(pMessage);
    IMS_SINT32 nStatusCode = 0;
    IMS_UINT32 nCseqNum = GetCSeqNumber(pMessage);

    if (!IsRequestMessage(pMessage))
    {
        nStatusCode = GetStatusCode(pMessage);
    }

    return new sipcore::SipTxnKey(objMethod, nStatusCode, strViaBranch, nCseqNum);
}

GLOBAL ::SipTxnKey* CreateTxnKey(IN ::SipMessage* pMessage, IN IMS_SINT32 /*nApiCalled*/)
{
    SIPStackError(EERR_NOERR);

    if (pMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    IMS_UINT16 nError = 0;

    return new ::SipTxnKey(pMessage, &nError);
}

GLOBAL sipcore::SipTxnKey* CreateTxnKeyFromKey(IN const ::SipTxnKey* pTxnKey)
{
    if (pTxnKey == IMS_NULL)
    {
        return IMS_NULL;
    }

    SipMethod objMethod(static_cast<const IMS_CHAR*>(pTxnKey->GetMethod()));
    AString strViaBranch(TxnKey_GetViaBranch(pTxnKey));

    return new sipcore::SipTxnKey(
            objMethod, pTxnKey->GetResponseCode(), strViaBranch, pTxnKey->GetCSeqNum());
}

GLOBAL IMS_BOOL CompareTxnKeys(IN ::SipTxnKey* pTxnKey1, IN ::SipTxnKey* pTxnKey2)
{
    if (pTxnKey1 == SIP_NULL)
    {
        return IMS_FALSE;
    }

    if (pTxnKey1->CompareKeys(pTxnKey2) != SIP_MATCHES)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

GLOBAL IMS_BOOL CompareTxnKeysForAck(IN const ::SipTxnKey* pTxnKey1, IN const ::SipTxnKey* pTxnKey2)
{
    if ((pTxnKey1 == IMS_NULL) || (pTxnKey2 == IMS_NULL))
    {
        return IMS_FALSE;
    }

    if ((pTxnKey1->GetCSeqNum() != pTxnKey2->GetCSeqNum()) ||
            (IMS_StrCmp(pTxnKey1->GetCallId(), pTxnKey2->GetCallId()) != 0) ||
            (IMS_StrICmp(pTxnKey1->GetFromTag(), pTxnKey2->GetFromTag()) != 0) ||
            (IMS_StrICmp(pTxnKey1->GetToTag(), pTxnKey2->GetToTag()) != 0) ||
            (IMS_StrICmp(pTxnKey1->GetViaBranchParam(), pTxnKey2->GetViaBranchParam()) != 0))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

GLOBAL IMS_BOOL CompareTxnKeysForCancel(IN ::SipTxnKey* pCancelKey, IN ::SipTxnKey* pTxnKey)
{
    // Compares these values : CSeq number, Call-ID, From-Tag, Via branch parameter

    if ((pCancelKey == IMS_NULL) || (pTxnKey == IMS_NULL))
    {
        return IMS_FALSE;
    }

    // Check Via branch parameter first.
    if (IMS_StrStr(pCancelKey->m_pszViaBranchParam, Sip::STR_BRANCH_MAGIC_COOKIE) != IMS_NULL)
    {
        IMS_TRACE_D("Transaction Matching -----> Compliant to RFC 3261", 0, 0, 0);

        // Request was generated by a client transaction compliant to RFC 3261.
        // Therefore, the branch parameter will be unique across all transactions
        // sent by that client.
        if (IMS_StrICmp(pCancelKey->m_pszViaBranchParam, pTxnKey->m_pszViaBranchParam) != 0)
        {
            return IMS_FALSE;
        }

        // Check 'sent-by' info. - host
        if ((pCancelKey->m_pszViaHost != IMS_NULL) && (pTxnKey->m_pszViaHost != IMS_NULL) &&
                (IMS_StrStr(pCancelKey->m_pszViaHost, ":") != IMS_NULL) &&
                (IMS_StrStr(pTxnKey->m_pszViaHost, ":") != IMS_NULL))
        {
            // Compares IPv6 addresses
            AString strCancelHost(static_cast<const IMS_CHAR*>(pCancelKey->m_pszViaHost));
            AString strTxnHost(static_cast<const IMS_CHAR*>(pTxnKey->m_pszViaHost));
            IpAddress objCancelIPA(strCancelHost);
            IpAddress objTxnIPA(strTxnHost);

            if (!objCancelIPA.Equals(objTxnIPA))
            {
                return IMS_FALSE;
            }
        }
        else
        {
            if (IMS_StrICmp(pCancelKey->m_pszViaHost, pTxnKey->m_pszViaHost) != 0)
            {
                return IMS_FALSE;
            }
        }

        // Check 'sent-by' info. - port
        if (pCancelKey->m_nViaHostPort != pTxnKey->m_nViaHostPort)
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    ///// Request-URI, To-Tag, From-Tag, Call-ID, CSeq, top Via header

    // CSeq number
    if (pCancelKey->m_nCseqNum != pTxnKey->m_nCseqNum)
    {
        return IMS_FALSE;
    }

    // Call-ID
    if (IMS_StrCmp(pCancelKey->m_pszCallId, pTxnKey->m_pszCallId) != 0)
    {
        return IMS_FALSE;
    }

    // From-Tag
    if (IMS_StrICmp(pCancelKey->m_pszFromTag, pTxnKey->m_pszFromTag) != 0)
    {
        return IMS_FALSE;
    }

    // Via branch parameter
    if (IMS_StrICmp(pCancelKey->m_pszViaBranchParam, pTxnKey->m_pszViaBranchParam) != 0)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

GLOBAL IMS_BOOL AbortTransaction(IN ::SipTxnKey* pTxnKey, IN SipTxnContext* pTxnContext)
{
    (void)pTxnContext;

    TerminateTransaction(pTxnKey);

    return IMS_TRUE;
}

GLOBAL SipTxnContext* CreateTxnContext()
{
    return SipContextUtils::GetInstance()->Sip_CreateTxnContext();
}

GLOBAL void DestroyTxnContext(IN SipTxnContext* pContext)
{
    if (pContext != IMS_NULL)
    {
        SipTxnContextData* pTxnContextData =
                static_cast<SipTxnContextData*>(pContext->m_pTxnContextData);

        if (pTxnContextData != IMS_NULL)
        {
            delete pTxnContextData;
            pContext->m_pTxnContextData = IMS_NULL;
        }

        SipContextUtils::GetInstance()->Sip_DestroyTxnContext(pContext);
    }
}

GLOBAL void DisplayTxnKey(IN const ::SipTxnKey* pTxnKey)
{
    if (pTxnKey == IMS_NULL)
    {
        return;
    }

    IMS_CHAR acCallId[11 + 1] = {
            '\0',
    };
    AString strLog;
    strLog.Sprintf("{ TxnKey: method=%s-%d, via-branch=%s, rseq=%d, call-id=%s }",
            _TRACE_S_(pTxnKey->GetMethod()), pTxnKey->GetCSeqNum(),
            _TRACE_S_(pTxnKey->GetViaBranchParam()), pTxnKey->GetRSeq(),
            GetLogString(pTxnKey->GetCallId(), acCallId, 11, '@'));

    IMS_TRACE_I("%s", strLog.GetStr(), 0, 0);
}

GLOBAL void FreeTxnKey(IN ::SipTxnKey*& pTxnKey)
{
    if (pTxnKey != SIP_NULL)
    {
        pTxnKey->SipDelete();
        pTxnKey = IMS_NULL;
    }
}

GLOBAL void FreeTxn(IN SipTxn*& pTxn)
{
    if (pTxn != IMS_NULL)
    {
        pTxn->SipDelete();
        pTxn = IMS_NULL;
    }
}

GLOBAL void TerminateTransaction(IN ::SipTxnKey* pTxnKey)
{
    SipStackManager::GetInstance()->TerminateTxn(pTxnKey);
}

GLOBAL const IMS_CHAR* GetTimerTypeAsString(IN IMS_SINT32 eTimerType)
{
    // clang-format off
    static const IMS_CHAR* acTimerType[] = {
            "Timer_1 >> RTT Estimate",
            "Timer_2 >> non-INVITE req & INVITE res",
            "Timer_4 >> Max duration Message in n/w",
            "Timer_A_B >> INVITE client",
            "Timer_B >> INVITE client",
            "Timer_C >> INVITE client",
            "Timer_D >> INVITE client",
            "Timer_E_F >> non-INVITE client",
            "Timer_F >> non-INVITE client",
            "Timer_G_H >> INVITE server & ACK receipt",
            "Timer_H >> ACK receipt",
            "Timer_I >> ACK retransmit",
            "Timer_J >> non-INVITE server",
            "Timer_K >> non-INVITE client",
            "Timer_L >> absorb retransmit INVITE & ACK receipt",
            "Timer_M >> 2XX-INVITE retransmission or forked",
            "Timer_OTHER",
            "Timer_INVALID"
    };
    // clang-format on

    return acTimerType[eTimerType];
}

GLOBAL const IMS_CHAR* GetTimerTypeAsString(IN const SipTimeoutData* pData)
{
    if (pData == IMS_NULL)
    {
        return "__INVALID__";
    }

    return GetTimerTypeAsString(pData->GetTimerType());
}

GLOBAL void InvokeTimerCallback(
        IN SipTimerCallback pfnCallback, IN SipTimeoutData* pData, IN IMS_PVOID pvExtraParam)
{
    if (pfnCallback == IMS_NULL)
    {
        return;
    }

    pfnCallback(pData, pvExtraParam);
}

GLOBAL void SetTimerValues(IN const SipTimerValues* pTv, IN_OUT SipTxnContext*& pTxnContext)
{
    if ((pTv == IMS_NULL) || (pTxnContext->m_pSipTimerContext == IMS_NULL))
    {
        return;
    }

    SipTxnTimerValues* pTxnTimerValues = pTxnContext->m_pSipTimerContext->m_pTxnSipTxnTimers;

    if (pTxnTimerValues == IMS_NULL)
    {
        return;
    }

    if (!pTv->IsSet(SipTimerValues::TIMER_ALL))
    {
        return;
    }

    IMS_UINT32 nTxnTimerOptions = 0;

    if (pTv->IsSet(SipTimerValues::TIMER_T1))
    {
        IMS_SINT32 nT1 = pTv->GetValue(SipTimerValues::TIMER_T1);
        nTxnTimerOptions |= SipTxnTimerValues::TV_T1;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_T1, nT1);

        // Timer-A, Timer-E, Timer-G is based on T1.
        nTxnTimerOptions |= SipTxnTimerValues::TV_TIMER_A;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_A, nT1);

        nTxnTimerOptions |= SipTxnTimerValues::TV_TIMER_E;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_E, nT1);

        nTxnTimerOptions |= SipTxnTimerValues::TV_TIMER_G;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_G, nT1);

        // Timer-L is set based on T1 due to no configuration definition.
        nTxnTimerOptions |= SipTxnTimerValues::TV_TIMER_L;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_L, nT1 * 64);

        // Timer-M is set based on T1 due to no configuration definition.
        nTxnTimerOptions |= SipTxnTimerValues::TV_TIMER_M;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_M, nT1 * 64);
    }

    if (pTv->IsSet(SipTimerValues::TIMER_T2))
    {
        IMS_SINT32 nT2 = pTv->GetValue(SipTimerValues::TIMER_T2);
        nTxnTimerOptions |= SipTxnTimerValues::TV_T2;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_T2, nT2);

        // Actually, T4 is used for Timer-I/Timer-K, but those are already set based on T4.
        // Even if T4 value is not actually used, it's set to the default value of T4 based on T2.
        nTxnTimerOptions |= SipTxnTimerValues::TV_T4;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_T4, nT2 + 1000);
    }

    if (pTv->IsSet(SipTimerValues::TIMER_B))
    {
        nTxnTimerOptions |= SipTxnTimerValues::TV_TIMER_B;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_B, pTv->GetValue(SipTimerValues::TIMER_B));
    }

    if (pTv->IsSet(SipTimerValues::TIMER_D))
    {
        nTxnTimerOptions |= SipTxnTimerValues::TV_TIMER_D;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_D, pTv->GetValue(SipTimerValues::TIMER_D));
    }

    if (pTv->IsSet(SipTimerValues::TIMER_F))
    {
        nTxnTimerOptions |= SipTxnTimerValues::TV_TIMER_F;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_F, pTv->GetValue(SipTimerValues::TIMER_F));
    }

    if (pTv->IsSet(SipTimerValues::TIMER_H))
    {
        nTxnTimerOptions |= SipTxnTimerValues::TV_TIMER_H;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_H, pTv->GetValue(SipTimerValues::TIMER_H));
    }

    if (pTv->IsSet(SipTimerValues::TIMER_I))
    {
        nTxnTimerOptions |= SipTxnTimerValues::TV_TIMER_I;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_I, pTv->GetValue(SipTimerValues::TIMER_I));
    }

    if (pTv->IsSet(SipTimerValues::TIMER_J))
    {
        nTxnTimerOptions |= SipTxnTimerValues::TV_TIMER_J;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_J, pTv->GetValue(SipTimerValues::TIMER_J));
    }

    if (pTv->IsSet(SipTimerValues::TIMER_K))
    {
        nTxnTimerOptions |= SipTxnTimerValues::TV_TIMER_K;
        pTxnTimerValues->SetTimerValue(SipTxn::TIMER_K, pTv->GetValue(SipTimerValues::TIMER_K));
    }

    pTxnContext->m_pSipTimerContext->m_nTimerOptions = nTxnTimerOptions;
    pTxnContext->m_pSipTimerContext->m_pTxnSipTxnTimers = pTxnTimerValues;
}

GLOBAL void DisplayUnknownHeaders(IN ::SipMessage* pMessage)
{
    IMS_TRACE_I("___ SIP unknown headers ___", 0, 0, 0);

    if (pMessage == IMS_NULL)
    {
        return;
    }

    SipHeaderList* pHeaderList = pMessage->GetHdrList(SipHeaderBase::UNKNOWN);

    if (pHeaderList != SIP_NULL)
    {
        IMS_UINT32 nSize = pHeaderList->GetSize();
        IMS_CHAR acLog[13 + 1] = {
                '\0',
        };

        for (IMS_UINT32 i = 0; i < nSize; ++i)
        {
            SipUnknownHeader* pUnknown = DYNAMIC_CAST(SipUnknownHeader*, pHeaderList->GetObj(i));

            if (pUnknown != SIP_NULL)
            {
                const SIP_CHAR* pszHdrName = pUnknown->GetHeaderName();
                const SIP_CHAR* pszHdrValue = pUnknown->GetHeaderValue();

                acLog[0] = '\0';

                if (pszHdrName != SIP_NULL && pszHdrValue != SIP_NULL &&
                        GetHeaderTypeFromName(pszHdrName) != SipHeaderBase::CONTENT_LENGTH)
                {
                    IMS_TRACE_I(
                            "\t(U) %s: %s", pszHdrName, GetLogString(pszHdrValue, acLog, 13), 0);
                }

                pUnknown->SipDelete();
            }
        }
        pHeaderList->SipDelete();
    }

    pHeaderList = pMessage->GetHdrList(SipHeaderBase::REASON);

    if (pHeaderList != SIP_NULL)
    {
        IMS_UINT32 nSize = pHeaderList->GetSize();

        for (IMS_UINT32 i = 0; i < nSize; ++i)
        {
            SipHeaderBase* pReason = pHeaderList->GetObj(i);

            if (pReason != SIP_NULL)
            {
                AStringBuffer objBuffer(128);

                if (pReason->Encode(objBuffer, SIP_TRUE) == SIP_TRUE)
                {
                    IMS_TRACE_I("\t(K) Reason: %s", objBuffer.GetCharString(), 0, 0);
                }

                pReason->SipDelete();
            }
        }

        pHeaderList->SipDelete();
    }
}

// Return value: pszOutput (user mode & config-debug-off), pszInput (non-user mode)
GLOBAL const IMS_CHAR* GetLogString(IN const IMS_CHAR* pszInput, IN_OUT IMS_CHAR* pszOutput,
        IN IMS_SINT32 nOutSize /* > 3, excluding null char */,
        IN const IMS_CHAR cDelimiter /* = 0 */)
{
    if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
    {
        return pszInput;
    }

    if (nOutSize < 3)
    {
        return pszOutput;
    }

    if ((pszInput == IMS_NULL) || (pszInput[0] == '\0'))
    {
        pszOutput[0] = 'z';
        pszOutput[1] = 'z';
        pszOutput[2] = 'z';
        pszOutput[3] = '\0';
        return pszOutput;
    }

    IMS_SINT32 i = 0;
    IMS_SINT32 nMaxSize = nOutSize - 3;

    while ((i < nMaxSize) && (pszInput[i] != '\0'))
    {
        if ((cDelimiter > 0) && (pszInput[i] == cDelimiter))
            break;

        pszOutput[i] = pszInput[i];
        ++i;
    }

    pszOutput[i] = 'x';
    pszOutput[i + 1] = 'x';
    pszOutput[i + 2] = 'x';
    pszOutput[i + 3] = '\0';

    return pszOutput;
}

GLOBAL IMS_BOOL DecodeHeaderComponent(
        IN const SipAddrSpec* pAddrSpec, OUT ImsList<ISipHeader*>& objHeaders)
{
    if (pAddrSpec == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SipUri* pSipUri = const_cast<SipAddrSpec*>(pAddrSpec)->GetSipUri();

    if (pSipUri == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_UINT32 nListCount = pSipUri->GetHdrParamCount();

    for (IMS_UINT32 i = 0; i < nListCount; ++i)
    {
        SipNameValue* pNameValue = pSipUri->GetHdrParam(i);

        if (pNameValue == IMS_NULL)
        {
            continue;
        }

        if (pNameValue->m_objValueList.IsEmpty())
        {
            pSipUri->SipDelete();
            return IMS_FALSE;
        }

        SipHeader* pHeader = new SipHeader();

        if (pHeader != IMS_NULL)
        {
            pHeader->SetName(pNameValue->m_pszName);

            IMS_CHAR* pszHdrVal = pNameValue->m_objValueList.GetAt(0);

            AString strHdrValue(pszHdrVal);

            pHeader->SetHeaderValue(TextParser::DoPercentDecoding(strHdrValue));

            if (!objHeaders.Append(pHeader))
            {
                delete pHeader;
                pSipUri->SipDelete();
                return IMS_FALSE;
            }
        }
    }

    pSipUri->SipDelete();

    return IMS_TRUE;
}

GLOBAL IMS_BOOL DecodeHeaderComponent(
        IN const AString& strHeaders, OUT ImsList<ISipHeader*>& objHeaders)
{
    ImsList<AString> objTokens = strHeaders.Split(TextParser::CHAR_AMPERSAND);

    if (objTokens.IsEmpty())
    {
        return IMS_TRUE;
    }

    for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
    {
        const AString& strHeader = objTokens.GetAt(i);
        IMS_SINT32 nPosOfEqual = strHeader.GetIndexOf(TextParser::CHAR_EQUAL);

        // Invalid header format
        if (nPosOfEqual == AString::NPOS)
        {
            return IMS_FALSE;
        }

        // No header value field
        if (nPosOfEqual == (strHeader.GetLength() - 1))
        {
            return IMS_FALSE;
        }

        SipHeader* pHeader = new SipHeader();

        if (pHeader != IMS_NULL)
        {
            pHeader->SetName(strHeader.GetSubStr(0, nPosOfEqual));
            pHeader->SetHeaderValue(
                    TextParser::DoPercentDecoding(strHeader.GetSubStr(nPosOfEqual + 1)));

            if (!objHeaders.Append(pHeader))
            {
                delete pHeader;
                return IMS_FALSE;
            }
        }
    }

    return IMS_TRUE;
}

}  // namespace SipStack
