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
#include "ServiceUtil.h"
#include "IMSStrLib.h"
#include "AStringBuffer.h"
#include "SystemConfigManager.h"
#include "stack_headers.h"
#include "SipEventContext.h"
#include "SipUtil.h"
#include "SIPPrivate.h"
#include "SipConfigProxy.h"
#include "SIPMessageBuffer.h"
#include "SipHeaderName.h"
#include "SIPHeader.h"
#include "SIPMessageBodyPart.h"
#include "txn/SipTimeoutData.h"
#include "SIPTxnContextData.h"
#include "SIPTxnKey.h"
#include "SIPUtil.h"
#include "SipFeatures.h"
#include "SIPStack.h"

__IMS_TRACE_TAG_SIP__;

#define SIP_HEADER_SIZE 1024

extern void SIPStackTxnLayer_Initialize();
extern SIP_VOID sip_cbk_onTimerExpired(IN ISipUserData* pUserData,
        IN IMS_SINT32 enTimerType);

class SipNetworkUtil : public ISipNetworkUtil
{
public:
    SipNetworkUtil() {}
    virtual ~SipNetworkUtil() {}

    /* IMpem*/
    SIP_BOOL SendToNetwork(SipTransportBuffer *pTranspSipBuffer,
            SipTransportParameter *pFinalTranspParam, ISipUserData *pUserData)
    {
        (void) pFinalTranspParam;

        if (pUserData == SIP_NULL)
        {
            IMS_TRACE_E(0, "User data is missing", 0, 0, 0);
            return SIP_FALSE;
        }

        SipTxnContext* pstTxnContext = (SipTxnContext*)pUserData->GetUserData();
        if (pstTxnContext == IMS_NULL )
        {
            IMS_TRACE_E(0, "pstTxnContext is NULL", 0, 0, 0);
            return SIP_FALSE;
        }

        SIPTxnContextData* pTxnContextData = (SIPTxnContextData*)pstTxnContext->pTxnContextData;
        if (pTxnContextData == IMS_NULL)
        {
            IMS_TRACE_E(0, "User data does not contains SIPTxnContextData", 0, 0, 0);
            return SIP_FALSE;
        }

        SIPTransactionState *pTxnState = pTxnContextData->GetTxnState();

        if (pTxnState == SIP_NULL)
        {
            IMS_TRACE_E(0, "SIPTxnContextData does not contain TxnState", 0, 0, 0);
            return SIP_FALSE;
        }

        if (!pTxnState->SendToNetwork(
                (const IMS_BYTE *)pTranspSipBuffer->GetSipBuffer(),
                (IMS_SINT32)pTranspSipBuffer->GetSipBufferLen()))
        {
            IMS_TRACE_E(0, "SendToNetwork failed", 0, 0, 0);
            return SIP_FALSE;
        }

        return SIP_TRUE;
    }

    SIP_BOOL CheckTCPConnection(SipTransportParameter *pTransportParam,
            ISipUserData *pUserData)
    {
        (void) pTransportParam;
        (void) pUserData;

        return SIP_TRUE;
    }

    SIP_BOOL AbortTransmission(SipTransportParameter *pTranspParam,
            ISipUserData *pUserData)
    {
        (void) pTranspParam;
        (void) pUserData;

        return SIP_TRUE;
    }
};

class SipTxnListenerProxy : public ISipTxnListener
{
public:
    SipTxnListenerProxy(){}
    virtual ~SipTxnListenerProxy(){}

    SIP_BOOL TxnTimeout(ISipUserData *pUserData,
            IMS_SINT32 eTimerType)
    {
        IMS_TRACE_I("TxnTimeout", 0, 0, 0);
        sip_cbk_onTimerExpired(pUserData, eTimerType);
        return SIP_TRUE;
    }

    SIP_BOOL TxnTerminated(ISipUserData *pUserData)
    {
        IMS_TRACE_I("TxnTerminated", 0, 0, 0);

        if (pUserData != SIP_NULL)
        {
            if (pUserData->GetDeleteFlag() == SIP_TRUE)
            {
                SipTxnContext* pTxnContext =
                        reinterpret_cast<SipTxnContext*>(pUserData->GetUserData());

                if (pTxnContext != SIP_NULL)
                {
                    IMS_TRACE_D("Destroy::SipTxnContext", 0, 0, 0);
                    SIPStack::DestroyTxnContext(pTxnContext);
                    pUserData->SetUserData(SIP_NULL);
                }
            }
        }

        return SIP_TRUE;
    }
};



namespace SIPStack
{

SipParameters* GetParameters(SipHeaderBase* pstHeader, IMS_BOOL bCreateIfNotPresent);
IMS_BOOL hasParamList(SipEn_HdrType eType);

// SIP stack last error storage -- starts
LOCAL SipEn_ErrorTypes genError;

/*

Remarks

*/
/* NOT_USED
LOCAL inline SipEn_ErrorTypes* SIPStackError()
{ return &genError; }
*/

/*

Remarks

*/
LOCAL inline void SIPStackError(IN SipEn_ErrorTypes enError_)
{ genError = enError_; }

// SIP stack last error storage -- ends

/*

Remarks

*/
LOCAL void DeleteStackString(IN SIP_CHAR*& pszStr)
{
    if (pszStr != IMS_NULL)
    {
        delete[] pszStr;
        pszStr = IMS_NULL;
    }
}

/*

Remarks

*/
LOCAL IMS_BOOL FormAddrSpec(IN CONST SipAddrSpec *pstAddrSpec, IN IMS_BOOL bParams,
        OUT AStringBuffer &objStringBuffer)
{
    (void) bParams;

    //---------------------------------------------------------------------------------------------

    SIP_CHAR szAddrSpec[SIP_HEADER_SIZE];
    IMS_MEM_Memset(szAddrSpec, 0x0, sizeof(szAddrSpec));
    SIP_CHAR *pszTemp = szAddrSpec;

    // FIXME: needs to be checked "bParams"

    if (pstAddrSpec->EncodeAddrSpec(&pszTemp) == SIP_TRUE)
    {
        objStringBuffer = szAddrSpec;
        return IMS_TRUE;
    }

    return IMS_FALSE;
}


/*

Remarks

*/
LOCAL IMS_BOOL GetParameter(IN SipHeaderBase* pstHeader, IN CONST AString &strName,
        OUT SipNameValue *&pstParam)
{
    SIPStackError(EERR_NOERR);

    SipParameters* pstSCHdr = GetParameters(pstHeader, IMS_FALSE);

    if (pstSCHdr == IMS_NULL)
    {
        SIPStackError(EERR_NOEXISTS);
        return IMS_FALSE;
    }

    IMS_UINT32 nParamCount = 0;
    SipNameValue* pstTempPrm = pstSCHdr->GetParamNode((SIP_CHAR *)strName.GetStr(), &nParamCount);

    if (pstTempPrm == SIP_NULL)
    {
        SIPStackError(EERR_NOEXISTS);
        return IMS_FALSE;
    }

    pstParam = pstTempPrm;
    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL void Initialize()
{
    IMS_SINT32 nSlotId = SystemConfigManager::GetInstance()->GetActiveSlotId();
    const ISipConfigV *piSipConfigV = SIPConfigProxy::GetSipConfigV(nSlotId);

    //---------------------------------------------------------------------------------------------

    // For transaction layer handling
    // Initialize the retransmission initial timer values, timeout timer values,
    // and wait timer values.
    // Below timer values are based on 3GPP; Consider IETF timer values, later.

    /* Initialize SIP Stack and Register NetworkUtil */
    SipStackManager *pStackMngr = SipStackManager::GetInstance();
    SipConfiguration* pSipConfig = SipConfiguration::GetInstance();

    if (pSipConfig != IMS_NULL)
    {
        pSipConfig->SetTimerC(180000);
        pSipConfig->SetTimerCr(180000);

        //If Timer value in Operator config not equal to "-1"/"",Set Timer values from Config.
        //Else default value
        SetTransactionTimerValues(IMS_NULL, piSipConfigV);

        AString strSipTimers;
        strSipTimers.Sprintf("t1=%u, t2=%u, t4=%u, tb=%u, tc=%u, td=%u, tf=%u, th=%u, ti=%u,\
            tj=%u, tk=%u", pSipConfig->GetT1(), pSipConfig->GetT2(), pSipConfig->GetT4(),
            pSipConfig->GetTimerB(), pSipConfig->GetTimerC(), pSipConfig->GetTimerD(),
            pSipConfig->GetTimerF(), pSipConfig->GetTimerH(), pSipConfig->GetTimerI(),
            pSipConfig->GetTimerJ(), pSipConfig->GetTimerK());

        IMS_TRACE_D("SIP Timers :: %s", strSipTimers.GetStr(), 0, 0);

        //TODO - Verify and modify implementation for dualsim hot swap scenarios.
        //SetCompactForm Encoding
        SIP_BOOL bCompact = (SIP_BOOL)SIPConfigProxy::IsCompactFormConfigured(IMS_SLOT_0, NULL);
        pSipConfig->SetShortFormEncoding(bCompact);

        //PANI header for failure response INVITE-ACK
        if (SIPFeatures::IsPANIHeaderForAckRequired(nSlotId))
        {
            pSipConfig->EnablePANIHeaderForACK(SIP_TRUE);
        }
        else
        {
            pSipConfig->EnablePANIHeaderForACK(SIP_FALSE);
        }
    }

    if (pStackMngr != IMS_NULL)
    {
        pStackMngr->GetSipUtil()->RegisterNetwork(new SipNetworkUtil());
        pStackMngr->GetSipUtil()->RegisterTxnListener(new SipTxnListenerProxy());
    }

    // Register SIP transaction layer's callback functions
    SIPStackTxnLayer_Initialize();
}

/*

Remarks

*/
GLOBAL void SetTransactionTimerValues(
        IN CONST SIPProfile *pSIPProfile, IN CONST ISipConfigV *piSipConfigV)
{
    SipConfiguration* pSipConfig = SipConfiguration::GetInstance();
    if (pSipConfig != IMS_NULL)
    {
        pSipConfig->SetT1((IMS_UINT32)
            SIPConfigProxy::GetTimerValueT1(IMS_SLOT_0, pSIPProfile, piSipConfigV));
        pSipConfig->SetT2((IMS_UINT32)
            SIPConfigProxy::GetTimerValueT2(IMS_SLOT_0, pSIPProfile, piSipConfigV));
        pSipConfig->SetT4((IMS_UINT32)
            SIPConfigProxy::GetTimerValueT4(IMS_SLOT_0, pSIPProfile, piSipConfigV));
        pSipConfig->SetTimerB(SIPConfigProxy::GetTimerValueTB(IMS_SLOT_0, pSIPProfile,
            piSipConfigV));
        pSipConfig->SetTimerD(SIPConfigProxy::GetTimerValueTD(IMS_SLOT_0, pSIPProfile,
            piSipConfigV));
        pSipConfig->SetTimerF(SIPConfigProxy::GetTimerValueTF(IMS_SLOT_0, pSIPProfile,
            piSipConfigV));
        pSipConfig->SetTimerH(SIPConfigProxy::GetTimerValueTH(IMS_SLOT_0, pSIPProfile,
            piSipConfigV));
        pSipConfig->SetTimerI(SIPConfigProxy::GetTimerValueTI(IMS_SLOT_0, pSIPProfile,
            piSipConfigV));
        pSipConfig->SetTimerJ(SIPConfigProxy::GetTimerValueTJ(IMS_SLOT_0, pSIPProfile,
            piSipConfigV));
        pSipConfig->SetTimerK(SIPConfigProxy::GetTimerValueTK(IMS_SLOT_0, pSIPProfile,
            piSipConfigV));
    }
}

/*

Remarks

*/
GLOBAL SipEn_ErrorTypes GetLastError()
{
    //---------------------------------------------------------------------------------------------

    return genError;
}

/*

Remarks

*/
GLOBAL IMS_BOOL AppendHeader(IN SipHeaderBase* pstHeader, IN_OUT SipMessage *&pstMessage)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    /*TODO: Implementation required to append the header to the existing list*/
    if (pstMessage->AppendHeader(pstHeader) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL AppendMessageBody(IN SipMsgBody *pstMsgBody, IN_OUT SipMessage *&pstMessage)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstMessage->AppendMessageBody(pstMsgBody) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL PrependHeader(IN SipHeaderBase* pstHeader, IN_OUT SipMessage *&pstMessage)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    if (pstMessage->InsertHeader(pstHeader, SIP_ZERO) == SIP_FALSE)
    {
       return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL PrependUnknownHeader(IN CONST AString &strName, IN CONST AString &strValue,
        IN_OUT SipMessage *&pstMessage)
{
    SipHeaderBase *pstHeader = DecodeHeader(ESIPHDR_UNKNOWN, strName, strValue);

    //---------------------------------------------------------------------------------------------

    if (pstHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (SIP_FALSE == PrependHeader(pstHeader, pstMessage))
    //SetHeader Changed to PrependHeader to accommodate multiple unknown Headers
    {
        FreeHeader(pstHeader);
        return IMS_FALSE;
    }

    FreeHeader(pstHeader);

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL SipHeaderBase* CloneHeader(IN SipHeaderBase *pstHeader)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);


    if (pstHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    return SipHeaders::CloneHdrObj(pstHeader);
}

/*

Remarks

*/
GLOBAL SipMessage* CloneMessage(IN SipMessage *pstMessage)
{
    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    return new SipMessage(*pstMessage);
}

/*

Remarks

*/
GLOBAL SipMsgBody* CloneMessageBody(IN SipMsgBody *pstMsgBody)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstMsgBody == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    return new SipMsgBody(*pstMsgBody);
}

/*

Remarks

*/
GLOBAL SipHeaderBase* CopyHeader(IN SipHeaderBase *pstHeader)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    return CloneHeader(pstHeader);
}

/*

Remarks

*/
GLOBAL SipHeaderBase* CopyHeader(IN SipHeaderBase *pstDest, IN SipHeaderBase *pstSrc)
{
    //---------------------------------------------------------------------------------------------

    if (pstSrc == IMS_NULL)
    {
        return pstDest;
    }

    if (pstDest == IMS_NULL)
    {
        return CopyHeader(pstSrc);
    }

    pstDest = pstSrc;

    AddReference(pstDest);

    return pstDest;
}

/*

Remarks

*/
GLOBAL IMS_BOOL CorrectMessageBody(IN_OUT SipMessage *&pstMessage)
{
    //---------------------------------------------------------------------------------------------

    IMS_UINT32 nMBCount = pstMessage->GetMsgBodyCount();

    if (nMBCount == 1)
    {
        // If the Content-Type header does not exist,
        // then insert the Content-Type header into SIP message.
        SipMsgBody *pstMsgBody = pstMessage->GetMsgBody(SIP_ZERO);

        if (pstMsgBody == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (pstMessage->HasHeader(ESIPHDR_CONTENTTYPE) == SIP_FALSE)
        {
            SipContentTypeHeader *pContentType = pstMsgBody->GetContentType();

            if (pContentType != IMS_NULL)
            {
                pstMessage->SetHeader(pContentType);
                pContentType->SipDelete();
            }
            else
            {
                pstMsgBody->SipDelete();
                return IMS_FALSE;
            }

            // TODO:: Other Content- Headers ???
        }

        pstMsgBody->SipDelete();
        // Remove the Content MIME headers
    }
    /*In case of multiple header*/
    else
    {
        // Sets the boundary parameter if not present
        IMS_TRACE_D( "SIPStack CorrectMessageBody:: nMBCount(%d)", nMBCount, 0, 0);

        SipHeaderBase *pstHeader = pstMessage->GetHdrObj(ESIPHDR_CONTENTTYPE);

        if (pstHeader == IMS_NULL)
        {
            SipHeaderBase *pstMultiHeader
                           = DecodeHeader(ESIPHDR_CONTENTTYPE, SIP::STR_MULTIPART_MIXED);

            if (pstMultiHeader == IMS_NULL)
            {
                return IMS_NULL;
            }

            AString strName(SIP::STR_BOUNDARY);
            AString strBoundary = SIPUtil::GenerateBoundary();

            if (!SetParameter(pstMultiHeader,strName,strBoundary))
            {
                FreeHeader(pstMultiHeader);
                return IMS_FALSE;
            }

            if (pstMessage->SetHeader(pstMultiHeader) == SIP_FALSE)
            {
                FreeHeader(pstMultiHeader);
                return IMS_FALSE;
            }

            FreeHeader(pstMultiHeader);
            return IMS_TRUE;
        }

        AString strName(SIP::STR_BOUNDARY);
        SipNameValue *pstParam = IMS_NULL;

        if (!GetParameter(pstHeader, strName, pstParam))
        {
            if (GetLastError() == EERR_NOEXISTS)
            {
                // Insert a boundary parameter
                AString strBoundary = SIPUtil::GenerateBoundary();

                if (!SetParameter(pstHeader, strName, strBoundary))
                {
                    FreeHeader(pstHeader);
                    return IMS_FALSE;
                }
            }
        }

        FreeHeader(pstHeader);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL SipHeaderBase* CreateHeader(IN IMS_SINT32 nType_)
{
    //---------------------------------------------------------------------------------------------

    return SipHeaders::CreateCoreHdrObj((SipEn_HdrType)nType_);
}

/*

Remarks

*/
GLOBAL SipHeaderBase* CreateHeader(IN IMS_SINT32 nType, IN SipAddrSpec *pstAddrSpec)
{
    //---------------------------------------------------------------------------------------------

    if (!IsAddressFormatHeader(nType, IMS_NULL))
    {
        return IMS_NULL;
    }

    SipNameAddrHeader *pstNewHeader
            = DYNAMIC_CAST(SipNameAddrHeader*, CreateHeader(nType));

    if (pstNewHeader == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (pstNewHeader->SetAddrSpec(pstAddrSpec) == SIP_FALSE)
    {
        pstNewHeader->SipDelete();
        return IMS_NULL;
    }

    return pstNewHeader;
}

/*

Remarks

*/
GLOBAL SipMessage* CreateMessage(IN IMS_SINT32 nType)
{
    //---------------------------------------------------------------------------------------------

    SipMessage *pstMessage = new SipMessage();

    if (pstMessage == IMS_NULL)
    {
        return IMS_NULL;
    }

    pstMessage->SetMessageType((SipEn_MsgType) nType);

    if (nType == ESIP_REQTYPE)
    {
        SipRequestLine* pReq = new SipRequestLine();

        if (pReq == SIP_NULL)
        {
            delete pstMessage;
            return IMS_NULL;
        }

        pReq->SetSipVersion(SIP::STR_SIP_VERSION);
        pstMessage->SetRequestline(pReq);
    }
    else
    {
        SipStatusLine* pStatus = new SipStatusLine();

        if (pStatus == SIP_NULL)
        {
            delete pstMessage;
            return IMS_NULL;
        }

        pStatus->SetSipVersion(SIP::STR_SIP_VERSION);
        pstMessage->SetStatusLine(pStatus);
    }

    return pstMessage;
}

/*

Remarks

*/
GLOBAL SipMsgBody* CreateMessageBody()
{
    //---------------------------------------------------------------------------------------------

    return new SipMsgBody();
}

/*

Remarks - NOT REQUIRED

*/
GLOBAL IMS_BOOL CreateMIMEHeader(IN_OUT SipMsgBody *pstMsgBody)
{
    (void) pstMsgBody;

    //---------------------------------------------------------------------------------------------

    //This type of initialization is not required in lsip
    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL SipHeaderBase* CreateViaHeader(IN CONST AString &strSentProtocol,
        IN CONST AString &strSentBy, IN CONST AString &strBranch)
{
    SipViaHeader* pVia = new SipViaHeader();

    if (pVia == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (pVia->SetProtocolName("SIP") == SIP_FALSE)
    {
        delete pVia;
        return IMS_NULL;
    }

    if (pVia->SetProtocolVer(SIP::STR_SIP_VERSION_ONLY) == SIP_FALSE)
    {
        delete pVia;
        return IMS_NULL;
    }

    SIP_CHAR* pcTransport = (SIP_CHAR*)strSentProtocol.GetStr();

    if (strSentProtocol.StartsWith("SIP") || strSentProtocol.StartsWith("sip"))
    {
        pcTransport += 8;
    }

    if (pVia->SetTransport((SIP_CHAR*)pcTransport) == SIP_FALSE)
    {
        delete pVia;
        return IMS_NULL;
    }

    SIP_CHAR* pcHost = (SIP_CHAR*)strSentBy.GetStr();
    SIP_CHAR* pcTmp;

    if (*pcHost == LEFT_SQUARE)//For IPV6
    {
        pcTmp = IMS_StrChr(pcHost, RIGHT_SQUARE);
        if (pcTmp != SIP_NULL)
        {
            pcTmp++;
            if (*pcTmp == COLON)
            {
                *pcTmp = '\0';
                pVia->SetHost(pcHost);
                pcTmp++;
                SIP_INT32 iPort = IMS_Atoi(pcTmp);
                if (pVia->SetPortNum((SIP_UINT16)iPort) == SIP_FALSE)
                {
                    delete pVia;
                    return IMS_NULL;
                }
            }
            else
            {
                *pcTmp = '\0';
                pVia->SetHost(pcHost);
            }
        }
        else
        {
            delete pVia;
            return IMS_NULL;
        }
    }
    else if ((pcTmp = IMS_StrChr(pcHost, ':')) != SIP_NULL) //For IPV4
    {
        *pcTmp = '\0';
        pVia->SetHost(pcHost);
        *pcTmp = ':';
        pcTmp++;

        SIP_INT32 iPort = IMS_Atoi(pcTmp);
        if (pVia->SetPortNum((SIP_UINT16) iPort) == SIP_FALSE)
        {
            delete pVia;
            return IMS_NULL;
        }
    }
    else
    {
        // only host present
        if (pVia->SetHost(pcHost) == SIP_FALSE)
        {
            delete pVia;
            return IMS_NULL;
        }
    }

    if (pVia->SetBranchParam((SIP_CHAR*)strBranch.GetStr()) == SIP_FALSE)
    {
        delete pVia;
        return IMS_NULL;
    }

    return pVia;
}

/*

Remarks

*/
GLOBAL SipAddrSpec* DecodeAddrSpec(IN CONST AString &strAddress)
{
    AString strAddrSpec = strAddress;
    IMS_SINT32 nLAQUOT;

    //---------------------------------------------------------------------------------------------

    // Remove LAQUOT/RAQUOT if present
    if ((nLAQUOT = strAddrSpec.GetIndexOf(TextParser::CHAR_LAQUOT)) != AString::NPOS)
    {
        IMS_SINT32 nRAQUOT;

        strAddrSpec = strAddrSpec.GetSubStr(nLAQUOT + 1);

        nRAQUOT = strAddrSpec.GetIndexOf(TextParser::CHAR_RAQUOT, nLAQUOT + 1);

        if (nRAQUOT != AString::NPOS)
        {
            strAddrSpec.Truncate(nRAQUOT);
        }
    }

    SipAddrSpec* pAddrSpec = new SipAddrSpec();

    if (pAddrSpec->DecodeAddrSpec(strAddrSpec.GetStr(),
            strAddrSpec.GetLength()) == SIP_FALSE)
    {
        delete pAddrSpec;
        return IMS_NULL;
    }

    return pAddrSpec;
}

/*

Remarks

*/

GLOBAL SipHeaderBase* DecodeHeader(IN IMS_SINT32 nType_, IN CONST AString &strName,
        IN CONST AString &strBody)
{
    SipEn_HdrType nType = (SipEn_HdrType)nType_;

    SIPStackError(EERR_NOERR);

    if (nType == ESIPHDR_UNKNOWN && (strName.GetLength() != 0))
    {
        SipEn_HdrType nUnknownType =
            static_cast<SipEn_HdrType>(sipGetHdrType(strName.GetStr()));

        if (nUnknownType != ESIPHDR_INVALID)
        {
            nType = nUnknownType;
        }
    }

    nType = GetHdrEnumType(nType);

    if (nType != ESIPHDR_ALLOW && _IMS_LOG_DEBUG_)
    {
        IMS_TRACE_D("SIPStack::DecodeHeader() - origType=[%d], type=[%d], strBody=[%s]",
                nType_, nType, strBody.GetStr());
    }

    SipHeaderBase *pHeader = CreateHeader(nType);

    if (pHeader == IMS_NULL)
    {
        IMS_TRACE_E(0, "SipHeaderBase is null", 0 , 0, 0);
        return IMS_NULL;
    }

    if (nType == ESIPHDR_UNKNOWN)
    {
        if (strName.GetLength() == 0)
        {
            FreeHeaderEx(pHeader);
            SIPStackError(EERR_INVALIDPARAM);
            IMS_TRACE_D("Unknown header name is not specified", 0, 0, 0);
            return IMS_NULL;
        }

        SipUnknownHeader* pstUnknownHeader = reinterpret_cast<SipUnknownHeader*>(pHeader);

        pstUnknownHeader->SetHeaderName((SIP_CHAR*)strName.GetStr());
        pstUnknownHeader->SetHeaderValue((SIP_CHAR*)strBody.GetStr());
    }
    else
    {
        IMS_UINT32 nBodyLen = strBody.GetLength();
        IMS_CHAR *pszTmpBody = strBody.Duplicate();

        if (pszTmpBody == IMS_NULL)
        {
            FreeHeaderEx(pHeader);
            IMS_TRACE_D("Header body is null", 0, 0, 0);
            return IMS_NULL;
        }

        if (pHeader->DecodeHdr(pszTmpBody, nBodyLen) == SIP_FALSE)
        {
            Free(pszTmpBody);
            FreeHeaderEx(pHeader);
            IMS_TRACE_D("Decoding header is failed", 0, 0, 0);
            return IMS_NULL;

        }

        // Process the header type which has an ANY type: Contact, Expires, Retry-After
        // sip_equateTypeInSipHeader(pstHeader);

        Free(pszTmpBody);
    }

    return pHeader;
}

/*

Remarks

*/
GLOBAL IMS_BOOL DecodeMessage(IN CONST IMS_BYTE *pBuffer, IN IMS_SINT32 nBuffLen,
        IN IMS_SINT32 nOptions, OUT SipMessage *&pstMessage)
{
    (void) nOptions;

    if (pstMessage == IMS_NULL)
    {
        pstMessage = new SipMessage();
    }

    IMS_BOOL bRetStatus = pstMessage->DecCompleteMsg(
            (SIP_CHAR*)pBuffer, (IMS_UINT32)nBuffLen);

    if (bRetStatus == SIP_FALSE)
    {
        pstMessage->SipDelete();
        pstMessage = IMS_NULL;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL DecodeMessageBody(IN SipMessage *pstMessage)
{
    SIPStackError(EERR_NOERR);

    if (pstMessage == NULL)
    {
        SIPStackError(EERR_NOEXISTS);
        return IMS_FALSE;
    }

    IMS_UINT32 nBodyCount = GetMessageBodyCount(pstMessage);

    if (nBodyCount == 0)
    {
        IMS_TRACE_D("___ NO SIP MESSAGE BODY ___", 0, 0, 0);
        return IMS_TRUE;
    }

    if (IsMessageBodyCompressed(pstMessage))
    {
        SipMsgBody *pstMsgBody = GetMessageBody(pstMessage);

        if (pstMsgBody == NULL)
        {
            IMS_TRACE_E(0, "Getting SIP message body failed", 0, 0, 0);
            return IMS_FALSE;
        }

        IMS_CHAR* pszbuffer = IMS_NULL;
        pstMsgBody->GetMsgBuffer(&pszbuffer);

        if (pszbuffer == IMS_NULL)
        {
            IMS_TRACE_E(0, "Message Buffer NULL", 0, 0, 0);
            pstMsgBody->SipDelete();
            return IMS_FALSE;
        }

        ByteArray objBodyPart;
        ByteArray objCompBodyPart;

        IMS_UINT32 uiBuffLen = 0;
        pstMsgBody->GetMsgBuffLen(&uiBuffLen);
        objCompBodyPart.Attach(
                reinterpret_cast<const IMS_BYTE*>(pszbuffer),
                (IMS_SINT32)uiBuffLen);

        pstMsgBody->SipDelete();

        if (!IMS_UTIL_ZLIB_Uncompress(objCompBodyPart, objBodyPart))
        {
            IMS_TRACE_E(0, "Uncompressing a body part failed", 0, 0, 0);
            delete[] pszbuffer;
            return IMS_FALSE;
        }

        delete[] pszbuffer;

        if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
        {
            IMS_TRACE_TEXT("gzip::uncompression", objBodyPart.GetData(), objBodyPart.GetLength());
        }

        IMS_CHAR *pszCompBodyStart = reinterpret_cast<IMS_CHAR*>(objBodyPart.GetData());
        IMS_CHAR *pszCompBodyEnd = pszCompBodyStart + objBodyPart.GetLength() - 1;
        IMS_UINT32 nCompLength = objBodyPart.GetLength();

        // DecodeMultiPartBodies
        if (pstMessage->DecMultiPartBody(pszCompBodyStart, pszCompBodyEnd, nCompLength)
                != SIP_TRUE)
        {
            IMS_TRACE_E(0, "Decoding uncompressed body part failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL EncodeAddrSpec(IN CONST SipAddrSpec *pstAddrSpec, IN IMS_BOOL bParams,
        OUT AString &strAddrSpec)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstAddrSpec == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    AStringBuffer objAddrSpec(128);

    if (!FormAddrSpec(pstAddrSpec, bParams, objAddrSpec))
    {
        strAddrSpec = AString::ConstNull();
        return IMS_FALSE;
    }

    strAddrSpec = objAddrSpec.GetString();

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL EncodeHeaderBody(IN CONST SipHeaderBase *pstHeader, IN IMS_BOOL bParams,
        OUT AString &strHeaderBody)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    SipHeaderBase *pHeader = const_cast<SipHeaderBase*>(pstHeader);
    AString strTempHeaderPrm;
    // headers like via can be quite big...so play safe with 512 byte enc buffer
    IMS_CHAR szBuffer[SIP_HEADER_SIZE] = {0};
    IMS_CHAR* pBuf = szBuffer;

    pHeader->EncodeHdr((SIP_CHAR**)(&pBuf), (SIP_BOOL)bParams);

    AString strTotalHeaderBody(szBuffer);

    if (pHeader->GetHdrType() == ESIPHDR_UNKNOWN)
    {
        if (!strTotalHeaderBody.SplitF(COLON, strTempHeaderPrm, strHeaderBody))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        strHeaderBody = strTotalHeaderBody;
    }
    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL EncodeMessage(IN SipMessage *pstMessage, IN IMS_SINT32 nOptions,
        OUT IMS_BYTE *&pBuffer, OUT IMS_SINT32 &nBuffLen)
{
    SIP_UINT32 nMsgOptions = ESIPMSGOPT_NONE;

    if ((nOptions & SIPPrivate::OPT_E_SHORTFORM) != 0)
    {
        nMsgOptions |= ESIPMSGOPT_ENCSHORTFORM;
    }

    if (pstMessage->EncodeMsg((SIP_CHAR**)&pBuffer,
            (SIP_UINT32*)&nBuffLen, nMsgOptions) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "SIPStack::EncodeMessage Failed", SIP_ZERO, SIP_ZERO);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL EncodePartialMessage(IN SipMessage *pstMessage, IN IMS_SINT32 nOptions,
        OUT ByteArray &objMessage)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    if ((nOptions | OPT_ALL) == 0)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    SIP_CHAR ucCurrPos[SIPMessageBuffer::MAX_MSG_SIZE];
    SIP_CHAR* pucCurrPos = ucCurrPos;
    SIP_BOOL bStatus = SIP_FALSE;

    // Check start-line
    if ((nOptions & OPT_START_LINE) == OPT_START_LINE)
    {
        if (pstMessage->GetMsgType() == ESIP_REQTYPE)
        {
            SipRequestLine* pRequestLine = pstMessage->GetReqLine();

            if (pRequestLine != SIP_NULL)
            {
                bStatus = pRequestLine->EncodeRequestLine(&pucCurrPos);
                pRequestLine->SipDelete();
            }
        }
        else if (pstMessage->GetMsgType() == ESIP_RESPTYPE)
        {
            SipStatusLine* pStatusLine = pstMessage->GetStatusLine();

            if (pStatusLine != SIP_NULL)
            {
                bStatus = pStatusLine->EncodeStatusLine(&pucCurrPos);
                pStatusLine->SipDelete();
            }
        }

        if (bStatus == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "SipEnc_SipMsg: Start Line Encoding Failed", SIP_ZERO, SIP_ZERO);
            return IMS_FALSE;
        }

        //Put CRLF at the end of Start Line
        SIP_ENC_CRLF(pucCurrPos);
    }

    // Check header parts
    if ((nOptions & OPT_HEADER_PART) == OPT_HEADER_PART)
    {
        SipMsgBodyList* pstMsgBodyList = pstMessage->GetMsgBodyList();

        if (pstMsgBodyList != SIP_NULL)
        {
            //Content-Length Encoding
            SipUnknownHeader *pobjContentLen = pstMessage->GetUnknownHdrObj(ESIPHDR_CONTENTLENGTH);

            if (pobjContentLen == SIP_NULL)
            {
                SIP_UINT16 usLen = (pstMsgBodyList != SIP_NULL) ?
                            pstMsgBodyList->GetTotalBodyLen() : SIP_ZERO;
                pstMessage->SetContentLengthHdr(usLen, ESIPMSGOPT_NONE);
            }
            else
            {
                pobjContentLen->SipDelete();
            }

            /*check for content type header
            and set the new one if not present*/
            if (pstMessage->HasHeader(ESIPHDR_CONTENTTYPE) == SIP_FALSE)
            {
                SipContentTypeHeader* pContentType = SIP_NULL;
                IMS_UINT16 usBodyCount = pstMsgBodyList->GetMsgBodyCount();

                if (usBodyCount == SIP_ONE)
                {
                    SipMsgBody* pstMsgbody = pstMsgBodyList->GetBodyByIndex(SIP_ZERO);

                    /*Check */
                    SipContentTypeHeader* pTempContentType = pstMsgbody->GetContentType();

                    if (pTempContentType != SIP_NULL)
                    {
                        pContentType = new SipContentTypeHeader(*pTempContentType);
                        pTempContentType->SipDelete();
                        pTempContentType = SIP_NULL;
                    }
                    else {
                        pstMsgbody->SipDelete();
                        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                            "SipEnc_SipMsg: Content Type Not Present", SIP_ZERO, SIP_ZERO);
                        return IMS_FALSE;
                    }

                    /*Set the header into the SIP message*/
                    pstMessage->SetHeader(pContentType);

                    /*Delete After Setting*/
                    pContentType->SipDelete();
                    pContentType = SIP_NULL;

                    /*Delete the message body*/
                    pstMsgbody->SipDelete();
                    pstMsgbody = SIP_NULL;
                }
                /*Case of more than one bodies*/
                else
                {
                    pContentType = new SipContentTypeHeader();
                    pContentType->SetMediaType(MULTIPART);
                    pContentType->SetSubMediaType(MIXED);

                    pstMessage->SetHeader(pContentType);

                    pContentType->SipDelete();
                    pContentType = SIP_NULL;
                }
            }
        }
        else
        {
            SipIntegerHeader* pContentLen = new SipIntegerHeader(SipHeaderBase::CONTENT_LENGTH);
            pContentLen->SetValueInt(SIP_ZERO);
            pstMessage->SetHeader(pContentLen);
            pContentLen->SipDelete();
            pContentLen = SIP_NULL;
        }
        //Encode Message body if present

        //Encoding of headers
        bStatus = pstMessage->GetMsgHdrs()->EncodeHdrs(&pucCurrPos, ESIPMSGOPT_NONE);

        if (bStatus == SIP_FALSE)
        {
            //g delete pucSipLocalBuffer;
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "SipEnc_SipMsg: Headers Encoding Failed",SIP_ZERO,SIP_ZERO);
            return IMS_FALSE;
        }
    }

    SIP_ENC_CRLF(pucCurrPos);

    if ((nOptions & OPT_BODY_PART) == OPT_BODY_PART)
    {
        SipMsgBodyList* pstMsgBodyList = pstMessage->GetMsgBodyList();

        if (pstMsgBodyList != SIP_NULL)
        {
            SipContentTypeHeader* pContentType = SIP_STATIC_CAST
                    (SipContentTypeHeader*, pstMessage->GetHdrObj(ESIPHDR_CONTENTTYPE));

            if (pContentType == SIP_NULL)
            {
                pstMsgBodyList->SipDelete();
                SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                    "SipEnc_SipMsg: Content Type Not Present", SIP_ZERO, SIP_ZERO);
                return IMS_FALSE;

            }

            const SIP_CHAR* pszMediaType = pContentType->GetMediaType();

            if (pszMediaType == SIP_NULL)
            {
                pContentType->SipDelete();
                pstMsgBodyList->SipDelete();
                SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                    "SipEnc_SipMsg: Content Type Invalid", SIP_ZERO, SIP_ZERO);
                return IMS_FALSE;
            }
            /*Case of Single Body*/
            if (IMS_StrICmp(pszMediaType, MULTIPART) != SIP_ZERO)
            {
                SipMsgBody* pBody = pstMsgBodyList->GetBodyByIndex(SIP_ZERO);

                if (pBody == SIP_NULL)
                {
                    pContentType->SipDelete();
                    pstMsgBodyList->SipDelete();
                    SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                        "SipEnc_SipMsg: Msg Body NULL", SIP_ZERO, SIP_ZERO);
                    return IMS_FALSE;
                }

                bStatus = pBody->EncodeSingleMsgBody(&pucCurrPos);

                pBody->SipDelete();

                if (bStatus == SIP_FALSE)
                {
                    pContentType->SipDelete();
                    pstMsgBodyList->SipDelete();
                    SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                        "SipEnc_SipMsg: Msg Body Enc Failed", SIP_ZERO, SIP_ZERO);
                    return IMS_FALSE;
                }
            }
            /*Case of multipart body*/
            else
            {
                SIP_CHAR* pszBoundary = pContentType->GetBoundary();

                bStatus = pstMsgBodyList->EncodeBody(&pucCurrPos, pszBoundary);

                if (bStatus == SIP_FALSE)
                {
                    DeleteStackString(pszBoundary);
                    pContentType->SipDelete();
                    pstMsgBodyList->SipDelete();
                    SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                        "SipEnc_SipMsg: Msg Body Enc Failed", SIP_ZERO, SIP_ZERO);
                    return IMS_FALSE;
                }

                DeleteStackString(pszBoundary);
            }

            pstMsgBodyList->SipDelete();
            pstMsgBodyList = SIP_NULL;
        }

    }

    AString strMsgBuffer(ucCurrPos);
    objMessage.Append(reinterpret_cast<const IMS_BYTE*>(strMsgBuffer.GetStr()),
            strMsgBuffer.GetLength());

    SIPStackError(EERR_NOERR);

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL IsUnknownHeader(IN_OUT IMS_SINT32 &nType, IN CONST AString &strName)
{
    if (strName.GetLength() != 0)
    {
        nType = sipGetHdrType(strName.GetStr());
    }

    return (nType == ISIPHeader::UNKNOWN);
}

/*

Remarks

*/

IMS_BOOL hasParamList(SipEn_HdrType eType)
{
    switch (eType)
    {
        case ESIPHDR_ACCEPT:
        case ESIPHDR_ACCEPTCONTACT:
        case ESIPHDR_CONTENTDISPOSITION:
        case ESIPHDR_CONTENTTYPE:
        case ESIPHDR_EVENT:
        case ESIPHDR_MINSE:
        case ESIPHDR_PACCESSNETWORKINFO:
        case ESIPHDR_PANSWERSTATE:
        case ESIPHDR_RETRYAFTERSEC:
        case ESIPHDR_RETRYAFTERDATE:
        case ESIPHDR_RETRYAFTERANY:
        case ESIPHDR_SESSIONEXPIRES:
        case ESIPHDR_VIA:
        case ESIPHDR_PVISITEDNETWORKID:
        case ESIPHDR_ALERTINFO:
        case ESIPHDR_CONTACT:
        case ESIPHDR_CONTACTWILD:
        case ESIPHDR_CONTACTANY:
        case ESIPHDR_FROM:
        case ESIPHDR_PASSERTEDIDENTITY:
        case ESIPHDR_PASSOCIATEDURI:
        case ESIPHDR_PATH:
        case ESIPHDR_PCALLEDPARTYID:
        case ESIPHDR_PPREFERREDIDENTITY:
        case ESIPHDR_RECORDROUTE:
        case ESIPHDR_REPLYTO:
        case ESIPHDR_ROUTE:
        case ESIPHDR_SERVICEROUTE:
        case ESIPHDR_TO:
            return IMS_TRUE;
        default:
            return IMS_FALSE;
    }
}
GLOBAL IMSList<SIPParameter*> ExtractParameters(IN SipHeaderBase *pstHeader)
{
    IMSList<SIPParameter*> objParams;
    SipParameters* pstParam = GetParameters(pstHeader, IMS_FALSE);

    if (pstParam != IMS_NULL)
    {
        SipParameterList* pSipParameterList = pstParam->GetParameterList();

        if (pSipParameterList == IMS_NULL)
        {
            // IMS_TRACE_D("There are no parameters in the header.",0,0,0);
            return objParams;
        }

        IMS_UINT32 uiNumParams = pSipParameterList->GetCount();

        for (IMS_UINT32 i = 0; i < uiNumParams; i++)
        {
            //first get the name of the param
            SipNameValue* pNameVal = pSipParameterList->GetNameValNode(i);

            if (pNameVal == IMS_NULL)
            {
                continue;
            }

            SIPParameter* pParameter = new SIPParameter(pNameVal->m_pszName);

            // now get the list of params for the name
            IMS_UINT32 usValSize = pNameVal->m_valueList.GetSize();

            for (IMS_UINT32 j = 0; j < usValSize; j++)
            {
                IMS_CHAR* pcVal = pNameVal->m_valueList.GetAt(j);

                if (pcVal != IMS_NULL)
                {
                    AString strValue(pcVal);
                    pParameter->AddValues(strValue);
                }
            }

            objParams.Append(pParameter);
        }
    }

    return objParams;
}

/*

Remarks

*/
GLOBAL IMSList<SIPParameter*> ExtractParameters(IN SipAddrSpec *pstAddrSpec)
{
    IMSList<SIPParameter*> objParams;

    if (pstAddrSpec == IMS_NULL)
    {
        return objParams;
    }

    SipUri *pSipUri = pstAddrSpec->GetSipUri();

    if (pSipUri == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::PARSING_ERROR);
        return objParams;
    }

    SipParameterList *pUriParamList = pSipUri->GetUriParamList();

    if (pUriParamList != IMS_NULL)
    {
        IMS_UINT32 iListCount = pUriParamList->GetCount();

        for (IMS_UINT32 i = 0; i < iListCount; ++i)
        {
            SipNameValue *pNmVl = pUriParamList->GetNameValNode(i);

            if (pNmVl == IMS_NULL)
            {
                continue;
            }

            SIPParameter* pParameter = new SIPParameter(pNmVl->m_pszName);

            if (pParameter == IMS_NULL)
            {
                pSipUri->SipDelete();
                pUriParamList->SipDelete();
                SIPPrivate::SetLastError(SIPError::PARSING_ERROR);
                return objParams;
            }

            if (pNmVl->m_valueList.IsEmpty())
            {
                objParams.Append(pParameter);
                continue;
            }

            IMS_UINT32 iValueCount = pNmVl->m_valueList.GetSize();

            for (IMS_UINT32 j = 0; j < iValueCount; ++j)
            {
                IMS_CHAR* pszVal = pNmVl->m_valueList.GetAt(j);

                if (pszVal != IMS_NULL)
                {
                    AString strValue(pszVal);
                    pParameter->AddValues(strValue);
                }
            }

            objParams.Append(pParameter);
        }

        pUriParamList->SipDelete();
    }

    pSipUri->SipDelete();

    return objParams;
}

/*

Remarks

*/
GLOBAL IMSList<SIPParameter*> ExtractParameters(IN CONST AString &strParams, IN IMS_CHAR cSep)
{
    IMSList<SIPParameter*> objParams;
    AString strTmp = strParams.Trim();

    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    IMS_TRACE_D("ExtractParameters for = [%s] with separator [%c]", strParams.GetStr(), cSep, 0);

    if ((strTmp.GetLength() == 0) || strTmp.Equals(cSep))
    {
        return objParams;
    }

    AStringArray objTokens = strTmp.Split(cSep);

    for (IMS_SINT32 i = 0; i < objTokens.GetCount(); ++i)
    {
        const AString &strToken = objTokens.GetElementAt(i);

        IMS_SINT32 nPos = strToken.GetIndexOf(TextParser::CHAR_EQUAL);
        SIPParameter* pParameter = IMS_NULL;

        // name only parameter
        if (nPos == AString::NPOS)
        {
            pParameter = new SIPParameter(strToken);
        }
        // name - value pair parameter
        else
        {
            AString strName = strToken.GetSubStr(0, nPos);
            AString strValue = strToken.GetSubStr(nPos + 1, strToken.GetLength() - nPos);
            AStringArray objValues = strValue.Split(TextParser::CHAR_COMMA);

            pParameter = new SIPParameter(strName, objValues);
        }

        objParams.Append(pParameter);
    }

    return objParams;
}

/*

Remarks

*/
GLOBAL void FreeMemBlock(IN void *&pvMemBlock)
{
    //---------------------------------------------------------------------------------------------

    IMS_MEM_Free(pvMemBlock);
    pvMemBlock = IMS_NULL;
}

/*

Remarks

*/
GLOBAL void FreeAddrSpec(IN SipAddrSpec *&pstAddrSpec)
{
    //---------------------------------------------------------------------------------------------

    if (pstAddrSpec != IMS_NULL)
    {
        pstAddrSpec->SipDelete();
        pstAddrSpec = IMS_NULL;
    }
}

/*

Remarks

*/
GLOBAL void FreeHeader(IN SipHeaderBase *pstHeader)
{
    //---------------------------------------------------------------------------------------------

    if (pstHeader != IMS_NULL)
    {
        pstHeader->SipDelete();
    }
}

/*

Remarks

*/
GLOBAL void FreeHeaderEx(IN SipHeaderBase *&pstHeader)
{
    //---------------------------------------------------------------------------------------------

    if (pstHeader != IMS_NULL)
    {
        pstHeader->SipDelete();
    }

    pstHeader = IMS_NULL;
}

/*

Remarks

*/
GLOBAL void FreeMessage(IN SipMessage *&pstMessage)
{
    //---------------------------------------------------------------------------------------------

    if (pstMessage != IMS_NULL)
    {
        pstMessage->SipDelete();
        pstMessage = IMS_NULL;
    }
}

/*

Remarks

*/
GLOBAL void FreeMessageBody(IN SipMsgBody *&pstMsgBody)
{
    //---------------------------------------------------------------------------------------------

    if (pstMsgBody != IMS_NULL)
    {
        pstMsgBody->SipDelete();
        pstMsgBody = IMS_NULL;
    }
}

/*

Remarks

*/
GLOBAL IMS_CHAR GetCompactHeaderName(IN IMS_SINT32 nType,
        IN CONST AString &strName /* = AString::ConstNull() */)
{
    (void) strName;

    //---------------------------------------------------------------------------------------------

    switch (nType)
    {
    case ESIPHDR_CALLID:
        return SIPHeaderName::CF_CALL_ID;
    case ESIPHDR_CONTACT: // FALL-THROUGH
    case ESIPHDR_CONTACTWILD: // FALL-THROUGH
    case ESIPHDR_CONTACTANY:
        return SIPHeaderName::CF_CONTACT;
    case ESIPHDR_CONTENTENCODING:
        return SIPHeaderName::CF_CONTENT_ENCODING;
    case ESIPHDR_CONTENTLENGTH:
        return SIPHeaderName::CF_CONTENT_LENGTH;
    case ESIPHDR_CONTENTTYPE:
        return SIPHeaderName::CF_CONTENT_TYPE;
    case ESIPHDR_FROM:
        return SIPHeaderName::CF_FROM;
    case ESIPHDR_SUPPORTED:
        return SIPHeaderName::CF_SUPPORTED;
    case ESIPHDR_TO:
        return SIPHeaderName::CF_TO;
    case ESIPHDR_VIA:
        return SIPHeaderName::CF_VIA;
    case ESIPHDR_EVENT:
        return SIPHeaderName::CF_EVENT;
    case ESIPHDR_ALLOWEVENTS:
        return SIPHeaderName::CF_ALLOW_EVENTS;
     case ESIPHDR_REFERTO:
        return SIPHeaderName::CF_REFER_TO;
    case ESIPHDR_REFERREDBY:
        return SIPHeaderName::CF_REFERRED_BY;
    case ESIPHDR_REQUESTDISPOSITION:
        return SIPHeaderName::CF_REQUEST_DISPOSITION;
    case ESIPHDR_ACCEPTCONTACT:
        return SIPHeaderName::CF_ACCEPT_CONTACT;
    case ESIPHDR_REJECTCONTACT:
        return SIPHeaderName::CF_REJECT_CONTACT;
    case ESIPHDR_SESSIONEXPIRES:
        return SIPHeaderName::CF_SESSION_EXPIRES;
    case ESIPHDR_SUBJECT:
        return SIPHeaderName::CF_SUBJECT;
    case ESIPHDR_IDENTITYINFO:
        return SIPHeaderName::CF_IDENTITY_INFO;
    case ESIPHDR_IDENTITY:
        return SIPHeaderName::CF_IDENTITY;

    default:
        break;
    }

    return '\0';
}

/*

Remarks

*/
GLOBAL const IMS_CHAR* GetHeaderName(IN IMS_SINT32 nType,
        IN CONST AString &strName /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    if ((nType <= ISIPHeader::INVALID) || (nType >= ISIPHeader::ANY))
    {
        return IMS_NULL;
    }

    switch (nType)
    {
    case ESIPHDR_UNKNOWN:
        if (strName.EqualsIgnoreCase(SIPHeaderName::CF_SUBJECT))
            return SIPHeaderName::SUBJECT;
        else if (strName.EqualsIgnoreCase(SIPHeaderName::CF_IDENTITY))
            return SIPHeaderName::IDENTITY;
        else if (strName.EqualsIgnoreCase(SIPHeaderName::CF_IDENTITY_INFO))
            return SIPHeaderName::IDENTITY_INFO;
        else
            return strName.GetStr();

    default:
        break;
    }

    return SIPHeader::NAME[nType];
}

/*

Remarks

*/
GLOBAL const IMS_CHAR* GetHeaderNameFromType(IN IMS_SINT32 nType)
{
    //---------------------------------------------------------------------------------------------

    if ((nType <= ESIPHDR_INVALID) || (nType > ESIPHDR_UNKNOWN))
    {
            return "";
    }

    switch (nType)
    {
        case ESIPHDR_UNKNOWN:
            // FIXME
            return "";

        default:
            return SIPHeader::NAME[nType];
    }
}

/*

Remarks

*/
GLOBAL IMS_SINT32 GetHeaderTypeFromName(IN CONST AString &strName)
{
    IMS_SINT32 nType = ESIPHDR_UNKNOWN;

    //---------------------------------------------------------------------------------------------

    if (strName.GetLength() == 0)
    {
        return ESIPHDR_INVALID;
    }

    switch (strName[0])
    {
    case 'a':
    case 'A':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_ACCEPT_CONTACT))
            {
                nType = ESIPHDR_ACCEPTCONTACT;
            }

            else if (strName.EqualsIgnoreCase(SIPHeaderName::ALLOW))
            {
                nType = ESIPHDR_ALLOW;
            }

            else if (strName.EqualsIgnoreCase(SIPHeaderName::ALLOW_EVENTS))
            {
                nType = ESIPHDR_ALLOWEVENTS;
            }

            else if (strName.EqualsIgnoreCase(SIPHeaderName::ACCEPT_CONTACT))
            {
                nType = ESIPHDR_ACCEPTCONTACT;
            }

            else if (strName.EqualsIgnoreCase(SIPHeaderName::ACCEPT))
            {
                nType = ESIPHDR_ACCEPT;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::AUTHORIZATION))
            {
                nType = ESIPHDR_AUTHORIZATION;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::ACCEPT_RESOURCE_PRIORITY))
            {
                nType = ESIPHDR_ACCEPTRESOURCEPRIORITY;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::ACCEPT_ENCODING))
            {
                nType = ESIPHDR_ACCEPTENCODING;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::ACCEPT_LANGUAGE))
            {
                nType =  ESIPHDR_ACCEPTLANGUAGE;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::ALERT_INFO))
            {
                nType =  ESIPHDR_ALERTINFO;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::ANSWER_MODE))
            {
                nType =  ESIPHDR_ANSWERMODE;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::AUTHENTICATION_INFO))
            {
                nType =  ESIPHDR_AUTHENTICATIONINFO;
            }
        }
        break;

    case 'b':
    case 'B':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_REFERRED_BY))
            {
                nType = ESIPHDR_REFERREDBY;
            }
        }
        break;

    case 'c':
    case 'C':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_CONTENT_TYPE))
            {
                nType = ESIPHDR_CONTENTTYPE;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::CALL_ID))
            {
                nType = ESIPHDR_CALLID;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::CONTACT))
            {
                nType = ESIPHDR_CONTACT;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::CONTENT_TYPE))
            {
                nType = ESIPHDR_CONTENTTYPE;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::CONTENT_LENGTH))
            {
                nType = ESIPHDR_CONTENTLENGTH;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::CONTENT_DISPOSITION))
            {
                nType = ESIPHDR_CONTENTDISPOSITION;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::CONTENT_ENCODING))
            {
                nType = ESIPHDR_CONTENTENCODING;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::CSEQ))
            {
                nType = ESIPHDR_CSEQ;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::CALL_INFO))
            {
                nType = ESIPHDR_CALLINFO;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::CONTENT_LANGUAGE))
            {
                nType = ESIPHDR_CONTENTLANGUAGE;
            }
        }
        break;

    case 'd':
    case 'D':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_REQUEST_DISPOSITION))
            {
                nType = ESIPHDR_REQUESTDISPOSITION;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::DATE))
            {
                nType = ESIPHDR_DATE;
            }
        }
        break;

    case 'e':
    case 'E':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_CONTENT_ENCODING))
            {
                nType = ESIPHDR_CONTENTENCODING;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::EVENT))
            {
                nType = ESIPHDR_EVENT;
            }

            else if (strName.EqualsIgnoreCase(SIPHeaderName::EXPIRES))
            {
                nType = ESIPHDR_EXPIRESANY;

            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::ERROR_INFO))
            {
                nType = ESIPHDR_ERRORINFO;
            }
        }
        break;

    case 'f':
    case 'F':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_FROM))
            {
                nType = ESIPHDR_FROM;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::FROM))
            {
                nType = ESIPHDR_FROM;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::FLOW_TIMER))
            {
                nType = ESIPHDR_FLOWTIMER;
            }
        }
        break;

    case 'h':
    case 'H':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::HISTORY_INFO))
            {
                nType = ESIPHDR_HISTORYINFO;
            }
        }
        break;

    case 'i':
    case 'I':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_CALL_ID))
            {
                nType = ESIPHDR_CALLID;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::IDENTITY))
            {
                nType = ESIPHDR_IDENTITY;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::IDENTITY_INFO))
            {
                nType = ESIPHDR_IDENTITYINFO;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::IN_REPLY_TO))
            {
                nType = ESIPHDR_INREPLYTO;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::INFO_PACKAGE))
            {
                nType = ESIPHDR_INFOPACKAGE;
            }
        }
        break;

    case 'j':
    case 'J':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_REJECT_CONTACT))
            {
                nType = ESIPHDR_REJECTCONTACT;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::JOIN))
            {
                nType = ESIPHDR_JOIN;
            }
        }
        break;

    case 'k':
    case 'K':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_SUPPORTED))
            {
                nType = ESIPHDR_SUPPORTED;
            }
        }
        break;

    case 'l':
    case 'L':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_CONTENT_LENGTH))
            {
                nType = ESIPHDR_CONTENTLENGTH;
            }
        }
        break;

    case 'm':
    case 'M':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_CONTACT))
            {
                nType = ESIPHDR_CONTACT;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::MAX_FORWARDS))
            {
                nType = ESIPHDR_MAXFORWARDS;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::MIME_VERSION))
            {
                nType = ESIPHDR_MIMEVERSION;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::MIN_SE))
            {
                nType = ESIPHDR_MINSE;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::MIN_EXPIRES))
            {
                nType = ESIPHDR_MINEXPIRES;
            }
        }
        break;

    case 'o':
    case 'O':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_EVENT))
            {
                nType = ESIPHDR_EVENT;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::ORGANIZATION))
            {
                nType = ESIPHDR_ORGANIZATION;
            }
        }
        break;

    case 'p':
    case 'P':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::PATH))
            {
                nType = ESIPHDR_PATH;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_ASSOCIATED_URI))
            {
                nType = ESIPHDR_PASSOCIATEDURI;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_CALLED_PARTY_ID))
            {
                nType = ESIPHDR_PCALLEDPARTYID;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_VISITED_NETWORK_ID))
            {
                nType = ESIPHDR_PVISITEDNETWORKID;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_CHARGING_FUNCTION_ADDRESSES))
            {
                nType = ESIPHDR_PCHRGFUNADDR;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_ACCESS_NETWORK_INFO))
            {
                nType = ESIPHDR_PACCESSNETWORKINFO;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_CHARGING_VECTOR))
            {
                nType = ESIPHDR_PCHARGINGVECTOR;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::PROXY_AUTHENTICATE))
            {
                nType = ESIPHDR_PROXYAUTHENTICATE;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::PROXY_AUTHORIZATION))
            {
                nType = ESIPHDR_PROXYAUTHORIZATION;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::PRIVACY))
            {
                nType = ESIPHDR_PRIVACY;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_PREFERRED_IDENTITY))
            {
                nType = ESIPHDR_PPREFERREDIDENTITY;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_ASSERTED_IDENTITY))
            {
                nType = ESIPHDR_PASSERTEDIDENTITY;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_EARLY_MEDIA))
            {
                nType = ESIPHDR_PEARLYMEDIA;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_ANSWER_STATE))
            {
                nType = ESIPHDR_PANSWERSTATE;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_MEDIA_AUTHORIZATION))
            {
                nType = ESIPHDR_PMEDIAAUTHORIZATION;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_PROFILE_KEY))
            {
                nType = ESIPHDR_PPROFILEKEY;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_REFUSED_URI_LIST))
            {
                nType = ESIPHDR_PREFUSEDURILIST;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_SERVED_USER))
            {
                nType = ESIPHDR_PSERVEDUSER;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::P_USER_DATABASE))
            {
                nType = ESIPHDR_PUSERDATABASE;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::PERMISSION_MISSING))
            {
                nType = ESIPHDR_PERMISSIONMISSING;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::PRIORITY))
            {
                nType = ESIPHDR_PRIORITY;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::PRIV_ANSWER_MODE))
            {
                nType = ESIPHDR_PRIVANSWERMODE;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::PROXY_REQUIRE))
            {
                nType = ESIPHDR_PROXYREQUIRE;
            }
        }
        break;

    case 'r':
    case 'R':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::REQUIRE))
            {
                nType = ESIPHDR_REQUIRE;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::REFERRED_BY))
            {
                nType = ESIPHDR_REFERREDBY;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::CF_REFER_TO))
            {
                nType = ESIPHDR_REFERTO;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::REFER_TO))
            {
                nType = ESIPHDR_REFERTO;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::REQUEST_DISPOSITION))
            {
                nType = ESIPHDR_REQUESTDISPOSITION;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::REJECT_CONTACT))
            {
                nType = ESIPHDR_REJECTCONTACT;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::REPLACES))
            {
                nType = ESIPHDR_REPLACES;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::RACK))
            {
                nType = ESIPHDR_RACK;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::RECORD_ROUTE))
            {
                nType = ESIPHDR_RECORDROUTE;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::RECV_INFO))
            {
                nType = ESIPHDR_RECVINFO;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::ROUTE))
            {
                nType = ESIPHDR_ROUTE;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::RSEQ))
            {
                nType = ESIPHDR_RSEQ;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::RETRY_AFTER))
            {
                nType = ESIPHDR_RETRYAFTERSEC;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::RESOURCE_PRIORITY))
            {
                nType = ESIPHDR_RESOURCEPRIORITY;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::RESPONSE_KEY))
            {
                nType = ESIPHDR_RESPONSEKEY;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::REASON))
            {
                nType = ESIPHDR_REASON;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::REPLY_TO))
            {
                nType = ESIPHDR_REPLYTO;
            }
        }
        break;

    case 's':
    case 'S':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_SUBJECT))
            {
                nType = ESIPHDR_SUBJECT;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::SUPPORTED))
            {
                nType = ESIPHDR_SUPPORTED;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::SIP_IF_MATCH))
            {
                nType = ESIPHDR_SIPIFMATCH;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::SIP_ETAG))
            {
                nType = ESIPHDR_SIPETAG;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::SERVICE_ROUTE))
            {
                nType = ESIPHDR_SERVICEROUTE;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::SESSION_EXPIRES))
            {
                nType = ESIPHDR_SESSIONEXPIRES;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::SUBSCRIPTION_STATE))
            {
                nType = ESIPHDR_SUBSCRIPTIONSTATE;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::SUBJECT))
            {
                nType = ESIPHDR_SUBJECT;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::SECURITY_CLIENT))
            {
                nType = ESIPHDR_SECURITYCLIENT;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::SECURITY_VERIFY))
            {
                nType = ESIPHDR_SECURITYVERIFY;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::SECURITY_SERVER))
            {
                nType = ESIPHDR_SECURITYSERVER;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::SERVER))
            {
                nType = ESIPHDR_SERVER;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::SUPPRESS_IF_MATCH))
            {
                nType = ESIPHDR_SUPPRESSIFMATCH;
            }
        }
        break;

    case 't':
    case 'T':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_TO))
            {
                nType = ESIPHDR_TO;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::TO))
            {
                nType = ESIPHDR_TO;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::TIMESTAMP))
            {
                nType = ESIPHDR_TIMESTAMP;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::TRIGGER_CONSENT))
            {
                nType = ESIPHDR_TRIGGERCONSENT;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::TARGET_DIALOG))
            {
                nType = ESIPHDR_TARGETDIALOG;
            }
        }
        break;

    case 'u':
    case 'U':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_ALLOW_EVENTS))
            {
                nType = ESIPHDR_ALLOWEVENTS;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::UNSUPPORTED))
            {
                nType = ESIPHDR_UNSUPPORTED;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::USER_AGENT))
            {
                nType = ESIPHDR_USERAGENT;
            }

        }
        break;

    case 'v':
    case 'V':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_VIA))
            {
                nType = ESIPHDR_VIA;
            }
            else if (strName.EqualsIgnoreCase(SIPHeaderName::VIA))
            {
                nType = ESIPHDR_VIA;
            }
        }
        break;

    case 'w':
    case 'W':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::WARNING))
            {
                nType = ESIPHDR_WARNING;
            }
            if (strName.EqualsIgnoreCase(SIPHeaderName::WWW_AUTHENTICATE))
            {
                nType = ESIPHDR_WWWAUTHENTICATE;
            }
        }
        break;

    case 'x':
    case 'X':
        {
            if (strName.EqualsIgnoreCase(SIPHeaderName::CF_SESSION_EXPIRES))
            {
                nType = ESIPHDR_SESSIONEXPIRES;
            }
        }
        break;

    default:
        break;
    }

    return nType;
}

/*

Remarks

*/
GLOBAL SipAddrSpec* GetAddrSpec(IN SipHeaderBase* pstHeader)
{
    if (pstHeader == SIP_NULL)
    {
        return IMS_NULL;
    }

    if (!IsAddressFormatHeader(pstHeader->GetHdrType(), IMS_NULL))
    {
        return IMS_NULL;
    }

    SipNameAddrHeader* pAddrSpecHdr = DYNAMIC_CAST(SipNameAddrHeader*, pstHeader);
    SipNameAddr* pNameAddr = pAddrSpecHdr->GetNameAddr();

    if (pNameAddr == IMS_NULL)
    {
        return IMS_NULL;
    }

    SipAddrSpec* pstAddrSpec = pNameAddr->GetAddrSpec();

    pNameAddr->SipDelete();

    return pstAddrSpec;
}

/*

Remarks

*/
GLOBAL SipAddrSpec* GetAddrSpec(IN SipMessage *pstMessage, IN IMS_SINT32 nType,
        IN IMS_UINT32 nIndex /* = 0 */)
{

    SipNameAddr* pNameAddr;

    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    if (!IsAddressFormatHeader(nType, IMS_NULL))
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    SipHeaderBase* pHdr = pstMessage->GetMsgHdrs()->getHdrObj((SipEn_HdrType)nType, nIndex);

    if (pHdr == IMS_NULL)
    {
        if ((SipEn_HdrType) nType == ESIPHDR_ROUTE)
        {
            SIPStackError(EERR_NOEXISTS);
        }
        else
        {
            IMS_TRACE_D("GetAddrSpec - Header is not found", 0, 0, 0);
            SIPStackError(EERR_INVALIDPARAM);
        }

        return IMS_NULL;
    }

    SipNameAddrHeader* pAddr = DYNAMIC_CAST(SipNameAddrHeader*, pHdr);

    SipAddrSpec* pAddrSpec = IMS_NULL;

    if (pAddr != SIP_NULL)
    {
        pNameAddr = pAddr->GetNameAddr();

        if (pNameAddr != IMS_NULL)
        {
            pAddrSpec = pNameAddr->GetAddrSpec();

            pNameAddr->SipDelete();
        }
    }

    pHdr->SipDelete();

    return pAddrSpec;
}

/*

Remarks

*/
GLOBAL AString GetChallengeScheme(IN SipHeaderBase *pstHeader)
{
    //---------------------------------------------------------------------------------------------

    if (pstHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    if ((pstHeader->GetHdrType() != ESIPHDR_WWWAUTHENTICATE)
            && (pstHeader->GetHdrType() != ESIPHDR_PROXYAUTHENTICATE))
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    SIPStackError(EERR_NOERR);

    SipAuthBase* pAuthHdr = DYNAMIC_CAST(SipAuthBase*, pstHeader);
    const IMS_CHAR* pszScheme = pAuthHdr->GetValue();
    AString strScheme(pszScheme);

    return strScheme;
}

/*

Remarks

*/
GLOBAL IMS_BOOL GetContent(IN SipMsgBody *pstMsgBody, OUT IMS_BYTE *&pContent,
        OUT IMS_SINT32 &nContentLength)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstMsgBody == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    pContent = reinterpret_cast<IMS_BYTE*>(pstMsgBody->GetBuffer());
    nContentLength = static_cast<IMS_SINT32>(pstMsgBody->GetBufferLength());

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_UINT32 GetCSeqNumber(IN SipMessage *pstMessage)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        IMS_TRACE_D("SIPStack::GetCSeqNumber Failed Message Null", 0, 0, 0);
        SIPStackError(EERR_INVALIDPARAM);
        return SIPPrivate::INVALID_SEQ_NUM;
    }

    SipCSeqHeader *pCSeq = DYNAMIC_CAST(SipCSeqHeader*, (pstMessage->GetHdrObj(ESIPHDR_CSEQ)));

    if (pCSeq == IMS_NULL)
    {
        IMS_TRACE_D( "SIPStack::GetCSeqNumber Failed csq hdr Null", 0, 0, 0);
        return SIPPrivate::INVALID_SEQ_NUM;
    }

    IMS_UINT32 nSeqNum = pCSeq->GetCSeq();

    pCSeq->SipDelete();

    return nSeqNum;
}

/*

Remarks

*/
GLOBAL IMS_BOOL GetRAckHeader(IN SipMessage *pstMessage, OUT IMS_UINT32 &nResponseNum,
        OUT IMS_UINT32 &nCSeqNum, OUT SIPMethod &objMethod)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    SipHeaderBase *pstHeader = SIPStack::GetHeader(pstMessage, ESIPHDR_RACK);

    if ((pstHeader == IMS_NULL) || (pstHeader->GetHdrType() != ESIPHDR_RACK))
    {
        FreeHeader(pstHeader);
        return IMS_FALSE;
    }

    SipRAcKHeader *pstRAckHeader = DYNAMIC_CAST(SipRAcKHeader*, pstHeader);

    // Get the response number from RAck header
    nResponseNum = pstRAckHeader->GetResponseNum();
    // Get the response number from CSeqNum from header
    nCSeqNum = pstRAckHeader->GetCSeqNum();

    objMethod = pstRAckHeader->GetMethod();

    FreeHeader(pstHeader);

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_SINT32 GetDestinationTransport(IN SipMessage *pstMessage)
{
    //---------------------------------------------------------------------------------------------

    // Check if the addr-spec which is set in the topmost Route header contains SIPS URI
    SipAddrSpec* pstAddrSpec = GetAddrSpec(pstMessage, ESIPHDR_ROUTE, 0);

    if (pstAddrSpec != IMS_NULL)
    {
        if ((pstAddrSpec->GetUriScheme() == SipUri::SCHEME_SIPS)
                && HasParameter(pstAddrSpec, AString(SIP::STR_LR)))
        {
            FreeAddrSpec(pstAddrSpec);
            return SIP::TRANSPORT_TLS;
        }

        FreeAddrSpec(pstAddrSpec);
        pstAddrSpec = IMS_NULL;
     }

    pstAddrSpec = GetRequestUri(pstMessage);

    if (pstAddrSpec != IMS_NULL)
    {
        if (pstAddrSpec->GetUriScheme() == SipUri::SCHEME_SIPS)
        {
            FreeAddrSpec(pstAddrSpec);
            return SIP::TRANSPORT_TLS;
        }

        FreeAddrSpec(pstAddrSpec);
    }

    return SIP::TRANSPORT_ANY;
}

/*

Remarks

*/
GLOBAL IMS_BOOL GetEventHeader(IN SipMessage *pstMessage, OUT AString &strEvent,
        OUT AString &strEventId)
{
    SipEventHeader* pstHeader = (SipEventHeader*) GetHeader(pstMessage, ESIPHDR_EVENT);

    if (!IsValidHeader(pstHeader))
    {
        return IMS_FALSE;
    }

    const IMS_CHAR *pszEvent = pstHeader->GetValue();

    strEvent = pszEvent;

    strEventId = GetParameter(pstHeader, AString("id"));

    FreeHeader(pstHeader);

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL SipHeaderBase* GetHeader(IN SipMessage *pstMessage, IN IMS_SINT32 nType_,
        IN IMS_UINT32 nIndex /* = 0 */)
{
    SipEn_HdrType nType = GetHdrEnumType((SipEn_HdrType)nType_);
    SipHeaderList* pHdrList = pstMessage->GetHdrList(nType);
    SipHeaderBase* pHdr = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (pHdrList == IMS_NULL)
    {
        pHdr = pstMessage->GetHdrObj(nType);
    }
    else
    {
        pHdr = pHdrList ->GetObj(nIndex);

        pHdrList->SipDelete();
    }

    return pHdr;
}

/*

Remarks

*/
GLOBAL AString GetHeaderAsString(IN SipMessage *pstMessage, IN IMS_SINT32 nType,
        IN IMS_BOOL bParams/* = IMS_FALSE*/, IN IMS_UINT32 nIndex/* = 0*/)
{
    SipHeaderBase *pHeader = GetHeader(pstMessage, nType, nIndex);

    //-----------------------------------------------------------------------------------------

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

/*

Remarks

*/
GLOBAL IMS_SINT32 GetHeaderCount(IN SipMessage *pstMessage, IN IMS_SINT32 nType_)
{
    SipEn_HdrType nType = GetHdrEnumType((SipEn_HdrType)nType_);
    SipHeaderBase* pHdr = pstMessage->GetHdrList(nType);

    //---------------------------------------------------------------------------------------------

    if (pHdr == SIP_NULL)
    {
        pHdr = pstMessage->GetHdrObj(nType);

        if (pHdr == SIP_NULL)
        {
            return SIP_ZERO;
        }

        pHdr->SipDelete();
        return SIP_ONE;
    }

    IMS_SINT32 nHCount = ((SipHeaderList*)pHdr)->GetSize();

    pHdr->SipDelete();

    return nHCount;
}

/*

Remarks

*/
GLOBAL IMS_BOOL GetHostAndPort(IN SipAddrSpec *pstAddrSpec, OUT AString &strHost,
        OUT IMS_UINT32 &nPort)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    nPort = SIP::PORT_UNSPECIFIED;

    if ((pstAddrSpec->GetUriScheme() == SipUri::SCHEME_SIP)
            || (pstAddrSpec->GetUriScheme() == SipUri::SCHEME_SIPS))
    {
        SipUri* pUri = pstAddrSpec->GetSipUri();

        if (pUri != SIP_NULL)
        {
            strHost = pUri->GetHost();

            nPort = pUri->GetPort();

            if (nPort == SIP_UNSPECIFIED_PORT)
            {
                SIPStackError(EERR_NOEXISTS);
            }

            pUri->SipDelete();
            return IMS_TRUE;
        }
    }

    SIPStackError(EERR_NOEXISTS);

    return IMS_FALSE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL GetHostNPortFromViaHeader(IN SipMessage *pstMessage,
        OUT AString &strHost, OUT IMS_SINT32 &nPort)
{
    //---------------------------------------------------------------------------------------------

    strHost = AString::ConstNull();
    nPort = SIP::PORT_UNSPECIFIED;

    // Get the topmost Via header from the message
    SipViaHeader *pViaHeader = DYNAMIC_CAST(SipViaHeader*, GetHeader(pstMessage, ESIPHDR_VIA));

    if (pViaHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    strHost = pViaHeader->GetHost();

    IMS_SINT32 nPos = AString::NPOS;

    if ((nPos = strHost.GetIndexOf(TextParser::CHAR_LSBRACKET)) != AString::NPOS)
    {
        // Strip the '[' , ']' before resolving the address.
        IMS_SINT32 nEndOfHost = strHost.GetIndexOf(TextParser::CHAR_RSBRACKET);

        strHost = strHost.GetSubStr(nPos + 1, nEndOfHost - nPos - 1);
    }

    nPort = pViaHeader->GetPort();

    if (nPort == 0 || nPort == SIP_UNSPECIFIED_PORT)
    {
        const IMS_CHAR *pszTransport = pViaHeader->GetTransport();

        if (IMS_StrICmp(pszTransport, SIP::STR_TLS_CAPS) == 0)
        {
            nPort = SIP::PORT_5061;
        }
        else
        {
            nPort = SIP::PORT_5060;
        }
    }

    // Free the local reference
    FreeHeader(pViaHeader);

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL SIPMethod GetMethod(IN SipMessage *pstMessage)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return SIPMethod();
    }

    SIPMethod objSIPMethod(pstMessage->GetMethod());

    return objSIPMethod;
}

/*

Remarks

*/
GLOBAL IMS_SINT32 GetMessageBodyCount(IN SipMessage *pstMessage)
{
    //---------------------------------------------------------------------------------------------

    return pstMessage->GetMsgBodyCount();
}

/*

Remarks

*/
GLOBAL SipMsgBody* GetMessageBody(IN SipMessage *pstMessage, IN IMS_SINT32 nIndex /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    return pstMessage->GetMsgBody(nIndex);
}

/*

Remarks

*/
GLOBAL AString GetMIMEHeader(IN SipMsgBody *pstMsgBody, IN IMS_SINT32 nType,
        IN IMS_SINT32 nIndex /* = 0 */)
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_D("GetMIMEHeader:: nType = [%d]", nType, 0, 0);

    SIPStackError(EERR_NOERR);

    if (pstMsgBody == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    SipHeaderBase* pstHeader = IMS_NULL;

    switch (nType)
    {
    case SIPMessageBodyPart::CONTENT_TYPE:
        IMS_TRACE_D("SIPMessageBodyPart::CONTENT_TYPE", 0, 0, 0);
        pstHeader = (SipHeaderBase*)pstMsgBody->GetContentType();
        break;

    case SIPMessageBodyPart::CONTENT_DISPOSITION:
        pstHeader = (SipHeaderBase*)pstMsgBody->GetContentDisposition();
        break;

    case SIPMessageBodyPart::CONTENT_TRANSFER_ENCODING:
        pstHeader = (SipHeaderBase*)pstMsgBody->GetContentEncoding();
        break;
    case SIPMessageBodyPart::CONTENT_UNKNOWN:
        pstHeader = pstMsgBody->GetMimeHdr(ESIPHDR_UNKNOWN, nIndex);
        break;
    case SIPMessageBodyPart::CONTENT_ID:
    {
        IMS_UINT32 nCount = pstMsgBody->GetUnknownHdrCount();

        for (IMS_UINT32 i = 0; i < nCount ; i++)
        {
            pstHeader = pstMsgBody->GetMimeHdr(ESIPHDR_UNKNOWN, i);
            AString strName = GetUnknownHeaderName(pstHeader);

            if (strName.EqualsIgnoreCase(SIPHeaderName::CONTENT_ID))
            {
                break;
            }
            else
            {
                FreeHeaderEx(pstHeader);
            }
        }
        break;
    }
    case SIPMessageBodyPart::CONTENT_DESCRIPTION:
    {
        IMS_UINT32 nCount = pstMsgBody->GetUnknownHdrCount();

        for (IMS_UINT32 i = 0; i < nCount ; i++)
        {
            pstHeader = pstMsgBody->GetMimeHdr(ESIPHDR_UNKNOWN, i);
            AString strName = GetUnknownHeaderName(pstHeader);

            if (strName.EqualsIgnoreCase(SIPHeaderName::CONTENT_DESCRIPTION))
            {
                break;
            }
            else
            {
                FreeHeaderEx(pstHeader);
            }
        }
        break;
    }
    default:
        break;
    }

    if (pstHeader != IMS_NULL)
    {
        AString strHeader;

        EncodeHeaderBody(pstHeader, IMS_TRUE, strHeader);

        FreeHeader(pstHeader);

        return strHeader;
    }

    return AString::ConstNull();
}

/*

Remarks

*/
GLOBAL IMS_SINT32 GetMIMEHeaderCount(IN SipMsgBody *pstMsgBody, IN IMS_SINT32 nType)
{
    //---------------------------------------------------------------------------------------------

    if (pstMsgBody == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return 0;
    }

    SIPStackError(EERR_NOERR);

    SipHeaderBase* pstHeader = IMS_NULL;

    switch (nType)
    {
    case SIPMessageBodyPart::CONTENT_TYPE:
        pstHeader = (SipHeaderBase*)pstMsgBody->GetContentType();
        break;
    case SIPMessageBodyPart::CONTENT_DISPOSITION:
        pstHeader = (SipHeaderBase*)pstMsgBody->GetContentDisposition();
        break;
    case SIPMessageBodyPart::CONTENT_TRANSFER_ENCODING:
        pstHeader = (SipHeaderBase*)pstMsgBody->GetContentEncoding();
        break;
    case SIPMessageBodyPart::CONTENT_UNKNOWN:
        return pstMsgBody->GetUnknownHdrCount();
    case SIPMessageBodyPart::CONTENT_ID:
    {
        IMS_UINT32 nCount = pstMsgBody->GetUnknownHdrCount();

        for (IMS_UINT32 i = 0; i < nCount ; i++)
        {
            pstHeader = pstMsgBody->GetMimeHdr(ESIPHDR_UNKNOWN, i);
            AString strName = GetUnknownHeaderName(pstHeader);

            if (strName.EqualsIgnoreCase(SIPHeaderName::CONTENT_ID))
            {
                break;
            }
            else
            {
                FreeHeaderEx(pstHeader);
            }
        }
        break;
    }
    case SIPMessageBodyPart::CONTENT_DESCRIPTION:
    {
        IMS_UINT32 nCount = pstMsgBody->GetUnknownHdrCount();

        for (IMS_UINT32 i = 0; i < nCount ; i++)
        {
            pstHeader = pstMsgBody->GetMimeHdr(ESIPHDR_UNKNOWN, i);
            AString strName = GetUnknownHeaderName(pstHeader);

            if (strName.EqualsIgnoreCase(SIPHeaderName::CONTENT_DESCRIPTION))
            {
                break;
            }
            else
            {
                FreeHeaderEx(pstHeader);
            }
        }
        break;
    }
    default:
        break;
    }

    if (pstHeader != IMS_NULL)
    {
        FreeHeader(pstHeader);
        return 1;
    }

    return 0;
}

/*

Remarks

*/
GLOBAL AString GetParameter(IN SipAddrSpec *pstAddrSpec, IN CONST AString &strName,
        IN IMS_UINT32 nIndex /* = 0 */)
{
    if ((pstAddrSpec->GetUriScheme() == SipUri::SCHEME_SIP)
            || (pstAddrSpec->GetUriScheme() == SipUri::SCHEME_SIPS))
    {
        SipUri *pUrl = pstAddrSpec->GetSipUri();

        if (pUrl == SIP_NULL)
        {
            return AString::ConstNull();
        }

        SipParameterList* pParamList = pUrl->GetUriParamList();

        if (pParamList == IMS_NULL)
        {
            pUrl->SipDelete();
            return AString::ConstNull();
        }

        IMS_CHAR* pszValue = pParamList->GetParamValue((SIP_CHAR*)strName.GetStr(), nIndex);
        AString strValue(pszValue);

        DeleteStackString(pszValue);

        pParamList->SipDelete();
        pUrl->SipDelete();

        return strValue;
    }
    else
    {
        // FIXME: Tel URL
        return AString::ConstNull();
    }

    return AString::ConstNull();
}

/*

Remarks

*/
GLOBAL AString GetParameter(IN SipHeaderBase* pstHeader, IN CONST AString &strName,
        IN IMS_UINT32 nIndex /* = 0 */)
{
    (void) nIndex;
    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    IMS_CHAR *pszValue = IMS_NULL;
    SipEn_HdrType enHdrType = static_cast<SipEn_HdrType>(pstHeader->GetHdrType());

    if ((enHdrType == ESIPHDR_WWWAUTHENTICATE)
            || (enHdrType == ESIPHDR_PROXYAUTHENTICATE)
            || (enHdrType == ESIPHDR_PROXYAUTHORIZATION)
            || (enHdrType == ESIPHDR_AUTHORIZATION)) {
        // Auth header parameters
        SipAuthBase *pstAuthHdr = DYNAMIC_CAST(SipAuthBase*, pstHeader);

        pszValue = pstAuthHdr->GetAuthValue(strName.GetStr());
    }
    else
    {
        // Normal header parameters
        SipParameters *pstSCHdr = GetParameters(pstHeader, IMS_FALSE);

        if (pstSCHdr == IMS_NULL)
        {
            SIPStackError(EERR_NOEXISTS);
            return AString::ConstNull();
        }

        pszValue = pstSCHdr->GetParamValue((SIP_CHAR*)strName.GetStr());
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

/*

Remarks

*/
GLOBAL IMS_SINT32 GetParameterCount(IN SipAddrSpec *pstAddrSpec)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    IMS_TRACE_D("SIPStack::GetParameterCount() pstAddrSpec", 0, 0, 0);

    if ((pstAddrSpec->GetUriScheme()== SipUri::SCHEME_SIP)
            || (pstAddrSpec->GetUriScheme() == SipUri::SCHEME_SIPS))
    {
        SipUri* pUrl = pstAddrSpec->GetSipUri();

        if (pUrl == IMS_NULL)
        {
            return 0;
        }

        IMS_SINT32 nPCount = pUrl->GetUriParamCount();

        pUrl->SipDelete();

        return nPCount;
    }
    else
    {
        SIPStackError(EERR_NOEXISTS);

        // FIXME: Tel URL
    }

    return 0;
}

/*

Remarks

*/
GLOBAL IMS_SINT32 GetParameterCount(IN SipHeaderBase* pstHeader)
{
    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        return 0;
    }

    SipParameters* pParamHdr = GetParameters(pstHeader, IMS_FALSE);

    if (pParamHdr == IMS_NULL)
    {
        return 0;
    }

    return pParamHdr->GetParamCount();
}

/*

Remarks

*/
GLOBAL SipAddrSpec* GetRequestUri(IN SipMessage *pstMessage)
{
    //---------------------------------------------------------------------------------------------
    SIPStackError(EERR_NOERR);

    if (pstMessage->GetMsgType() != ESIP_REQTYPE)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    SipRequestLine* pstReqLine = pstMessage->GetReqLine();

    if (pstReqLine == SIP_NULL)
    {
        return IMS_NULL;
    }

    SipAddrSpec *pstAddrSpec = pstReqLine->GetReqUri();

    pstReqLine->SipDelete();

    return pstAddrSpec;
}

/*

Remarks

*/
GLOBAL AString GetSentByFromVia(IN SipHeaderBase* pstHeader)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        IMS_TRACE_E(0,"GetSentByFromVia Null header passed", 0, 0, 0);
        return AString::ConstNull();
    }

    if (pstHeader->GetHdrType() != ESIPHDR_VIA)
    {
        IMS_TRACE_E(0,"Invalid HeaderType", 0, 0, 0);
        return AString::ConstNull();
    }

    SipViaHeader* pViaHeader = DYNAMIC_CAST(SipViaHeader*, pstHeader);

    if (pViaHeader == IMS_NULL)
    {
        IMS_TRACE_E(0, "Invalid Header", 0, 0, 0);
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    AString strSentby(pViaHeader->GetHost());

    IMS_UINT16 nPort = pViaHeader->GetPort();

    if (nPort != 0 && nPort != SIP_UNSPECIFIED_PORT)
    {
        strSentby.Append(':');

        AString strPort;
        strPort.SetNumber(nPort, 10);

        strSentby.Append(strPort);
    }

    return strSentby;

}

/*

Remarks

*/
GLOBAL AString GetSentProtocolFromVia(IN SipHeaderBase *pstHeader)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        return AString::ConstNull();
    }

    if (pstHeader->GetHdrType() != ESIPHDR_VIA)
    {
        return AString::ConstNull();
    }

    SipViaHeader* pViaHeader = DYNAMIC_CAST(SipViaHeader*, pstHeader);

    if (pViaHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    const IMS_CHAR* pszProtocolName = pViaHeader->GetProtocolName();
    const IMS_CHAR* pszProtocolVersion = pViaHeader->GetProtocolVer();
    const IMS_CHAR* pszTransport = pViaHeader->GetTransport();

    AString strSentProtocol;

    strSentProtocol.Sprintf("%s/%s/%s",
            (pszProtocolName != SIP_NULL) ? pszProtocolName : "SIP",
            (pszProtocolVersion != SIP_NULL) ? pszProtocolVersion : SIP::STR_SIP_VERSION_ONLY,
            (pszTransport != SIP_NULL) ? pszTransport : "UDP");

    return strSentProtocol;
}

/*

Remarks

*/
GLOBAL AString GetSIPVersion(IN SipMessage *pstMessage)
{
    //---------------------------------------------------------------------------------------------
    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    if (pstMessage->GetMsgType() == ESIP_REQTYPE)
    {
        SipRequestLine* pstReqLine = pstMessage->GetReqLine();

        if (pstReqLine == IMS_NULL)
        {
            return AString::ConstNull();
        }

        AString strVersion(pstReqLine->GetSipVersion());

        pstReqLine->SipDelete();

        return strVersion;
    }
    else
    {
        SipStatusLine *pstStatusLine = pstMessage->GetStatusLine();

        if (pstStatusLine == IMS_NULL)
        {
            return AString::ConstNull();
        }

        AString strVersion(pstStatusLine->GetSipVersion());

        pstStatusLine->SipDelete();

        return strVersion;
    }
}

/*

Remarks

*/
GLOBAL IMS_SINT32 GetStatusCode(IN SipMessage *pstMessage)
{
    //---------------------------------------------------------------------------------------------
    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return SIPStatusCode::SC_INVALID;
    }

    if (pstMessage->GetMsgType() != ESIP_RESPTYPE)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return SIPStatusCode::SC_INVALID;
    }

    SipStatusLine* pstStatusLine = pstMessage->GetStatusLine();

    if (pstStatusLine == IMS_NULL)
    {
        return SIPStatusCode::SC_INVALID;
    }

    IMS_SINT16 nStatusCode = SIPStatusCode::SC_INVALID;

    pstStatusLine->GetStatusCode(&nStatusCode);

    pstStatusLine->SipDelete();

    return (IMS_SINT32)(nStatusCode);
}

/*

Remarks

*/
GLOBAL SIPStatusCode GetStatusCodeEx(IN SipMessage *pstMessage)
{
    //---------------------------------------------------------------------------------------------
    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return SIPStatusCode();
    }

    SipStatusLine* pStatusLine = pstMessage->GetStatusLine();

    if (pStatusLine == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return SIPStatusCode();
    }

    IMS_SINT32 nStatusCode = SIPStatusCode::SC_INVALID;
    const IMS_CHAR* pszReasonPhrase = pStatusLine->GetRsnPhrase();
    const IMS_CHAR* pszStatusCode = pStatusLine->GetStatusCode();

    if (pszStatusCode != IMS_NULL)
    {
        nStatusCode = IMS_Atoi(pszStatusCode);
    }

    SIPStatusCode objStatusCode(nStatusCode, pszReasonPhrase);

    pStatusLine->SipDelete();
    return objStatusCode;
}

/*

Remarks

*/
GLOBAL IMS_BOOL GetSubscriptionStateHeader(IN SipMessage *pstMessage,
        OUT AString &strSubsState, OUT IMS_SINT32 *pnExpires /* = IMS_NULL */)
{
    SipHeaderBase *pSubState = GetHeader(pstMessage, ESIPHDR_SUBSCRIPTIONSTATE);

    if (pSubState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    //---------------------------------------------------------------------------------------------

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

    IMS_TRACE_D("GetSubscriptionStateHeader: pszSubsState[%s]", strSubsState.GetStr(), 0, 0);


    // Set the expires parameter
    if (pnExpires != IMS_NULL)
    {
        AString strExpires = GetParameter(pSubState, SIP::STR_EXPIRES);

        if (strExpires.GetLength() == 0)
        {
            (*pnExpires) = (-1);
            IMS_TRACE_D("GetSubscriptionStateHeader: No Expires", 0, 0, 0);
        }
        else
        {
            IMS_BOOL bOK = IMS_TRUE;
            IMS_TRACE_D("GetSubscriptionStateHeader: pszExpires[%s]", strExpires.GetStr(), 0, 0);
            (*pnExpires) = strExpires.ToInt32(&bOK);

            if (!bOK)
            {
                (*pnExpires) = (-1);
            }
        }
    }

    FreeHeader(pSubState);
    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL SipHeaderBase* GetUnknownHeader(IN SipMessage *pstMessage, IN CONST AString &strName,
        IN IMS_UINT32 nIndex/* = 0*/)
{
    //---------------------------------------------------------------------------------------------

    SipEn_HdrType nType = static_cast<SipEn_HdrType>(sipGetHdrType(strName.GetStr()));

    switch (nType)
    {
        case ESIPHDR_USERAGENT:
        {
            return GetHeader(pstMessage, ESIPHDR_USERAGENT, nIndex);
        }
        case ESIPHDR_SESSIONID:
        {
            return GetHeader(pstMessage, ESIPHDR_SESSIONID, nIndex);
        }
        case ESIPHDR_SERVER:
        {
            return GetHeader(pstMessage, ESIPHDR_SERVER, nIndex);
        }
        case ESIPHDR_UNKNOWN:
        {
            return GetHeader(pstMessage, ESIPHDR_UNKNOWN, nIndex);
        }
        default:
        {
            return GetHeader(pstMessage, nType, nIndex);
        }
    }
}

/*

Remarks

*/
GLOBAL AString GetUnknownHeaderName(IN SipHeaderBase *pstHeader)
{
    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    if (pstHeader->GetHdrType() != ESIPHDR_UNKNOWN)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    SipUnknownHeader* pstUnknown = DYNAMIC_CAST(SipUnknownHeader*, pstHeader);
    AString strName(pstUnknown->GetHeaderName());

    return strName;
}

/*

Remarks

*/
GLOBAL AString GetUnknownHeaderBody(IN SipHeaderBase *pstHeader)
{
    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    if (pstHeader->GetHdrType() != ESIPHDR_UNKNOWN)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return AString::ConstNull();
    }

    SipUnknownHeader* pstUnknown = DYNAMIC_CAST(SipUnknownHeader*, pstHeader);
    AString strValue(pstUnknown->GetHeaderValue());

    return strValue;
}

/*

Remarks

*/
GLOBAL AString GetViaBranchParameter(IN SipMessage *pstMessage)
{
    SipHeaderBase *pstHeader = GetHeader(pstMessage, ESIPHDR_VIA);

    //-----------------------------------------------------------------------------------------

    if (!IsValidHeader(pstHeader))
    {
        return AString::ConstNull();
    }

    AString strViaBranch = GetParameter(pstHeader, SIP::STR_BRANCH);

    FreeHeader(pstHeader);

    return strViaBranch;
}

/*

Remarks

*/

GLOBAL SipEn_HdrType GetHdrEnumType(IN SipEn_HdrType nType)
{
    return static_cast<SipEn_HdrType>(CheckAndGetHdrEnumType(nType));
}

/*

Remarks

*/
GLOBAL IMS_BOOL HasParameter(IN SipHeaderBase* pstHeader, IN CONST AString &strName)
{
    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SipParameters* pParamHdr = GetParameters(pstHeader, IMS_FALSE);

    if (pParamHdr == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_UINT32 usPos = 0;

    return (pParamHdr->IsParamExists((SIP_CHAR*)strName.GetStr(),
            &usPos) == SIP_TRUE) ? IMS_TRUE : IMS_FALSE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL HasParameter(IN SipAddrSpec *pstAddrSpec, IN CONST AString &strName)
{
    //---------------------------------------------------------------------------------------------
    SIPStackError(EERR_NOERR);

    if (pstAddrSpec == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    if ((pstAddrSpec->GetUriScheme() == SipUri::SCHEME_SIP)
            || (pstAddrSpec->GetUriScheme() == SipUri::SCHEME_SIPS))
    {
        SipUri* pUrl = pstAddrSpec->GetSipUri();

        if (pUrl == SIP_NULL)
        {
            return IMS_FALSE;
        }

        IMS_UINT32 nCount = pUrl->GetUriParamCount();

        if (nCount > 0)
        {
            SipParameterList* pParamList = pUrl->GetUriParamList();

            for (IMS_UINT32 i = 0; i < nCount; ++i)
            {
                SipNameValue* pNameVal = pParamList->GetNameValNode(i);

                if (pNameVal == SIP_NULL)
                {
                    continue;
                }

                if (strName.EqualsIgnoreCase(pNameVal->m_pszName))
                {
                    pParamList->SipDelete();
                    pUrl->SipDelete();
                    return IMS_TRUE;
                }
            }

            pParamList->SipDelete();
        }

        nCount = pUrl->GetHdrParamCount();

        if (nCount > 0)
        {
            SipParameterList* pParamList = pUrl->GetHdrParamList();

            for (IMS_UINT32 i = 0; i < nCount; ++i)
            {
                SipNameValue* pNameVal = pParamList->GetNameValNode(i);

                if (pNameVal == SIP_NULL)
                {
                    continue;
                }

                if (strName.EqualsIgnoreCase(pNameVal->m_pszName))
                {
                    pParamList->SipDelete();
                    pUrl->SipDelete();
                    return IMS_TRUE;
                }
            }

            pParamList->SipDelete();
        }

        pUrl->SipDelete();
    }

    return IMS_FALSE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL HasMIMEMessageBody(IN SipMessage *pstMessage)
{
    //---------------------------------------------------------------------------------------------
    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    return pstMessage->HasMIMEMessageBody();
}

/*

Remarks

*/
GLOBAL IMS_BOOL HasSDPMessageBody(IN SipMessage *pstMessage)
{
    //---------------------------------------------------------------------------------------------
    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    return pstMessage->HasSDPMessageBody();
}

/*

Remarks

*/
GLOBAL IMS_BOOL InsertHeader(IN SipHeaderBase* pstHeader, IN IMS_UINT32 nIndex,
        IN_OUT SipMessage *&pstMessage)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    /*TODO: Implementation required to add the header to the existing list*/
    if (pstMessage->InsertHeader(pstHeader, nIndex) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/

GLOBAL IMS_BOOL IsCompactHeaderNameSupported(IN IMS_SINT32 nType,
        IN CONST AString &strName /* = AString::ConstNull() */)
{
    (void) strName;

    //---------------------------------------------------------------------------------------------

    switch (nType)
    {
        case ESIPHDR_CALLID: // FALL-THROUGH
        case ESIPHDR_CONTACT: // FALL-THROUGH
        case ESIPHDR_CONTACTWILD: // FALL-THROUGH
        case ESIPHDR_CONTACTANY: // FALL-THROUGH
        case ESIPHDR_CONTENTENCODING: // FALL-THROUGH
        case ESIPHDR_CONTENTLENGTH: // FALL-THROUGH
        case ESIPHDR_CONTENTTYPE: // FALL-THROUGH
        case ESIPHDR_FROM: // FALL-THROUGH
        case ESIPHDR_SUPPORTED: // FALL-THROUGH
        case ESIPHDR_TO: // FALL-THROUGH
        case ESIPHDR_VIA: // FALL-THROUGH
        case ESIPHDR_EVENT: // FALL-THROUGH
        case ESIPHDR_ALLOWEVENTS: // FALL-THROUGH
        case ESIPHDR_REFERTO: // FALL-THROUGH
        case ESIPHDR_REFERREDBY: // FALL-THROUGH
        case ESIPHDR_REQUESTDISPOSITION: // FALL-THROUGH
        case ESIPHDR_ACCEPTCONTACT: // FALL-THROUGH
        case ESIPHDR_REJECTCONTACT: // FALL-THROUGH
        case ESIPHDR_SESSIONEXPIRES: // FALL-THROUGH
        case ESIPHDR_SUBJECT: // FALL-THROUGH
        case ESIPHDR_IDENTITY: // FALL-THROUGH
        case ESIPHDR_IDENTITYINFO:
            return IMS_TRUE;

        default:
            break;
    }

    return IMS_FALSE;
}


/*

Remarks

*/
GLOBAL IMS_BOOL IsHeaderPresent(IN SipMessage *pstMessage, IN IMS_SINT32 nType_)
{
    SipHeaderBase* pstHdr = GetHeader(pstMessage, (SipEn_HdrType)nType_);

    if (pstHdr == IMS_NULL)
    {
        return IMS_FALSE;
    }

    FreeHeader(pstHdr);

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL IsMessageBodySDP(IN SipMsgBody *pstMsgBody)
{
    //---------------------------------------------------------------------------------------------

    if (pstMsgBody == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    SIPStackError(EERR_NOERR);

    return pstMsgBody->IsMessageBodySDP();
}

/*

Remarks

*/
GLOBAL IMS_BOOL IsMessageRPR(IN SipMessage *pstMessage)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // If the message is not of type response, return FALSE
    if (pstMessage->GetMsgType() != ESIP_RESPTYPE)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nStatusCode = GetStatusCode(pstMessage);

    // The message has a status code greater than 199; It is a final response.
    if ((nStatusCode <= SIPStatusCode::SC_100)
            || (nStatusCode >= SIPStatusCode::SC_200))
    {
        return IMS_FALSE;
    }

    // If the message is a provisional response,
    // but does not contain a RSeq header, then return FALSE.
    IMS_SINT32 nRSeqCount = GetHeaderCount(pstMessage, ESIPHDR_RSEQ);

    if (nRSeqCount == 0)
    {
        return IMS_FALSE;
    }

    IMS_BOOL bOptionTag100rel = IMS_FALSE;
    IMS_SINT32 nHCount = GetHeaderCount(pstMessage, ESIPHDR_REQUIRE);

    for (IMS_SINT32 i = 0; i < nHCount; ++i)
    {
        SipHeaderBase* pstReqHdr = GetHeader(pstMessage, ESIPHDR_REQUIRE, i);

        const SIP_CHAR* pszOptionTag = pstReqHdr->GetValue();

        if (pszOptionTag != IMS_NULL)
        {
            if (AString::CompareIgnoreCase(pszOptionTag, SIP::STR_100REL) == 0)
            {
                bOptionTag100rel = IMS_TRUE;
                FreeHeader(pstReqHdr);
                break;
            }
        }
        else
        {
            IMS_TRACE_I("100Rel Not Present", 0, 0, 0);
        }

        FreeHeader(pstReqHdr);
    }

    // 101 ~ 199 response, RSeq header, option-tag ("100rel") in Require header
    if ((nRSeqCount > 0) && bOptionTag100rel)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Remarks
#ifdef JSR_NEW_CODE_TO_BE_PORTED
*/
GLOBAL IMS_BOOL IsOptionRequired(IN SipMessage *pstMessage, IN CONST AString &strOption)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nCount = GetHeaderCount(pstMessage, ESIPHDR_REQUIRE);

    if (nCount == 0)
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < nCount; ++i)
    {
        SipHeaderBase *pstRequire = GetHeader(pstMessage, ESIPHDR_REQUIRE, i);

        const SIP_CHAR* pszOptionTag = pstRequire->GetValue();

        if (strOption.EqualsIgnoreCase(pszOptionTag))
        {
            FreeHeader(pstRequire);
            return IMS_TRUE;
        }

        FreeHeader(pstRequire);
    }

    return IMS_FALSE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL IsOptionSupported(IN SipMessage *pstMessage, IN CONST AString &strOption)
{
    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nCount = GetHeaderCount(pstMessage, ESIPHDR_SUPPORTED);

    if (nCount == 0)
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < nCount; ++i)
    {
        SipHeaderBase* pSupHeader = GetHeader(pstMessage, ESIPHDR_SUPPORTED, i);

        const SIP_CHAR* pszOptionTag = pSupHeader->GetValue();

        if ((AString::Compare(pszOptionTag, "*") == 0)
                || strOption.EqualsIgnoreCase(pszOptionTag))
        {
            FreeHeader(pSupHeader);
            return IMS_TRUE;
        }

        FreeHeader(pSupHeader);
    }

    return IMS_FALSE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL IsRequestMessage(IN SipMessage *pstMessage)
{
    return (pstMessage != IMS_NULL) ? (pstMessage->GetMsgType() == ESIP_REQTYPE) : IMS_FALSE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL IsAddressFormatHeader(IN IMS_SINT32 nType, IN CONST AString &strName)
{
    //---------------------------------------------------------------------------------------------

    switch (nType)
    {
    case ESIPHDR_CONTACT: // FALL-THROUGH
    case ESIPHDR_CONTACTWILD: // FALL-THROUGH
    case ESIPHDR_CONTACTANY: // FALL-THROUGH
    case ESIPHDR_FROM: // FALL-THROUGH
    case ESIPHDR_PPREFERREDIDENTITY: // FALL-THROUGH
    case ESIPHDR_PASSERTEDIDENTITY: // FALL-THROUGH
    case ESIPHDR_PATH: // FALL-THROUGH
    case ESIPHDR_PASSOCIATEDURI: // FALL-THROUGH
    case ESIPHDR_PCALLEDPARTYID: // FALL-THROUGH
    case ESIPHDR_SERVICEROUTE: // FALL-THROUGH
    case ESIPHDR_HISTORYINFO: // FALL-THROUGH
    case ESIPHDR_RECORDROUTE: // FALL-THROUGH
    case ESIPHDR_REFERREDBY: // FALL-THROUGH
    case ESIPHDR_REFERTO: // FALL-THROUGH
    case ESIPHDR_ROUTE: // FALL-THROUGH
    case ESIPHDR_TO: // FALL-THROUGH
    case ESIPHDR_IDENTITYINFO: // FALL-THROUGH
    case ESIPHDR_PSERVEDUSER: // FALL-THROUGH
    case ESIPHDR_POLICYCONTACT: // FALL-THROUGH
    case ESIPHDR_POLICYID: // FALL-THROUGH
    case ESIPHDR_REPLYTO: // FALL-THROUGH
    case ESIPHDR_TRIGGERCONSENT:
        return IMS_TRUE;

    case ESIPHDR_UNKNOWN:
        if (strName.EqualsIgnoreCase(SIPHeaderName::DIVERSION))
        {
            return IMS_TRUE;
        }
        break;

    default:
        break;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL IsAQUOTRequiredForAddressFormat(
        IN IMS_SINT32 nType, IN CONST AString &strName)
{
    //---------------------------------------------------------------------------------------------

    (void) strName;

    switch (nType)
    {
    case ESIPHDR_IDENTITYINFO:
    case ESIPHDR_POLICYCONTACT:
        return IMS_TRUE;

    default:
        break;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
#ifdef SIP_TOBEPORTED
GLOBAL IMS_BOOL IsUriSchemeAllowed(IN SipHeaderBase *pstHeader)
{
    SipAddrSpec *pstAddrSpec = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (sip_getAddrSpecFromCommonHdr(pstHeader, &pstAddrSpec, SIPStackError()) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    if (pstAddrSpec->dType == SipAddrReqUri)
    {
        IMS_BOOL bIsTelUrl = IMS_FALSE;
        IMS_BOOL bIsIMUrl = IMS_FALSE;
        IMS_BOOL bIsPresUrl = IMS_FALSE;

        if (sip_isTelUrl(pstAddrSpec, SIPStackError()) == SipSuccess)
            bIsTelUrl = IMS_TRUE;

        if (bIsTelUrl != IMS_TRUE)
        {
            if (sip_isImUrl(pstAddrSpec, SIPStackError()) == SipSuccess)
                bIsIMUrl = IMS_TRUE;
        }

        if ((bIsTelUrl != IMS_TRUE) && (bIsIMUrl != IMS_TRUE))
        {
            if (sip_isPresUrl(pstAddrSpec, SIPStackError()) == SipSuccess)
                bIsPresUrl = IMS_TRUE;
        }

        if ((bIsTelUrl == IMS_TRUE) || (bIsIMUrl == IMS_TRUE) || (bIsPresUrl == IMS_TRUE))
        {
            FreeAddrSpec(pstAddrSpec);
            // Not Allowed
            return IMS_FALSE;
        }
    }

    FreeAddrSpec(pstAddrSpec);

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL IsUriSchemeSupported(IN SipHeaderBase *pstHeader)
{
    SipAddrSpec *pstAddrSpec = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    if (sip_getAddrSpecFromCommonHdr(pstHeader, &pstAddrSpec, SIPStackError()) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    if ((pstAddrSpec->dType != SipAddrSipUri) && (pstAddrSpec->dType != SipAddrSipSUri))
    {
        IMS_BOOL bIsTelUrl = IMS_FALSE;
        IMS_BOOL bIsIMUrl = IMS_FALSE;
        IMS_BOOL bIsPresUrl = IMS_FALSE;

        if (sip_isTelUrl(pstAddrSpec, SIPStackError()) == SipSuccess)
            bIsTelUrl = IMS_TRUE;

        if (bIsTelUrl != IMS_TRUE)
        {
            if (sip_isImUrl(pstAddrSpec, SIPStackError()) == SipSuccess)
                bIsIMUrl = IMS_TRUE;
        }

        if ((bIsTelUrl != IMS_TRUE) && (bIsIMUrl != IMS_TRUE))
        {
            if (sip_isPresUrl(pstAddrSpec, SIPStackError()) == SipSuccess)
                bIsPresUrl = IMS_TRUE;
        }

        if ((bIsTelUrl != IMS_TRUE) && (bIsIMUrl != IMS_TRUE) && (bIsPresUrl != IMS_TRUE))
        {
            FreeAddrSpec(pstAddrSpec);

            SIPPrivate::SetLastError(SIPError::URI_SCHEME_NOT_SUPPORTED);
            return IMS_FALSE;
        }
    }

    FreeAddrSpec(pstAddrSpec);

    return IMS_TRUE;
}

/*

Remarks

*/
#endif
GLOBAL IMS_BOOL OverwriteHeaders(IN SipMessage *pstSrcMessage,
        IN_OUT SipMessage *&pstDestMessage)
{
    //---------------------------------------------------------------------------------------------

    SipHeaders* pDest = pstDestMessage->GetMsgHdrs();

    // Overwrite known headers only
    pDest->OverWriteHdrObj(pstSrcMessage->GetMsgHdrs(), SIP_TRUE);

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL void ParseHostNPort(IN CONST AString &strHostNPort, OUT AString &strHost,
        OUT IMS_SINT32 &nPort)
{
    IMS_SINT32 nPos;

    //---------------------------------------------------------------------------------------------

    nPort = SIP::PORT_UNSPECIFIED;

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

            IMS_BOOL bOK = IMS_FALSE;
            IMS_UINT16 nTmpPort = strPort.ToUInt16(&bOK);

            if (bOK)
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

            IMS_BOOL bOK = IMS_FALSE;
            IMS_UINT16 nTmpPort = strPort.ToUInt16(&bOK);

            if (bOK)
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

/*

Remarks

*/
GLOBAL IMS_BOOL RemoveAllMessageBodies(IN_OUT SipMessage *&pstMessage)
{
    //---------------------------------------------------------------------------------------------

    if (pstMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // FIXME: add impl. if SIP stack supports this kind of method.

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL RemoveHeader(IN IMS_SINT32 nType_, IN_OUT SipMessage *&pstMessage)
{
    SipEn_HdrType nType = GetHdrEnumType((SipEn_HdrType)nType_);

    if (pstMessage->RemoveHdr(nType) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/

GLOBAL IMS_BOOL RemoveParameter(IN CONST AString &strName, IN_OUT SipHeaderBase*& pstHeader)
{
    if (pstHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SipParameters* pParamHdrBase = GetParameters(pstHeader, IMS_FALSE);

    if (pParamHdrBase == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_UINT32 usPos = 0;

    if (pParamHdrBase->IsParamExists((SIP_CHAR*)strName.GetStr(), &usPos) == SIP_FALSE)
    {
        return IMS_TRUE;
    }

    if (pParamHdrBase->RemoveParam((SIP_CHAR*)strName.GetStr()) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL RemoveParameter(IN CONST AString &strName, IN_OUT SipAddrSpec *&pstAddrSpec)
{
    //---------------------------------------------------------------------------------------------
    SIPStackError(EERR_NOERR);

    if ((pstAddrSpec->GetUriScheme() == SipUri::SCHEME_SIP)
            || (pstAddrSpec->GetUriScheme() == SipUri::SCHEME_SIPS))
    {
        SipUri *pstUrl = pstAddrSpec->GetSipUri();

        if (pstUrl == IMS_NULL)
        {
            return IMS_FALSE;
        }

        pstUrl->RemoveHdrParam((SIP_CHAR*)strName.GetStr());
        pstUrl->SipDelete();

        return IMS_TRUE;
    }
    else
    {
        SIPStackError(EERR_NOEXISTS);
    }

    return IMS_FALSE;
}

/*

Remarks

*/
GLOBAL void RemoveUserAndPassword(IN_OUT SipAddrSpec *&pstAddrSpec)
{
    //---------------------------------------------------------------------------------------------

    if ((pstAddrSpec != IMS_NULL)
            && ((pstAddrSpec->GetUriScheme() == SipUri::SCHEME_SIP)
                || (pstAddrSpec->GetUriScheme() == SipUri::SCHEME_SIPS)))
    {
        SipUri *pUri = pstAddrSpec->GetSipUri();

        if (pUri != IMS_NULL)
        {
            pUri->SetUser(IMS_NULL);
            pUri->SetPassword(IMS_NULL);

            pUri->SipDelete();
        }
    }
}

/*

Remarks

*/
GLOBAL IMS_BOOL SetChallengeScheme(IN CONST AString &strScheme, IN_OUT SipHeaderBase *&pstHeader)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    if ((pstHeader->GetHdrType() != ESIPHDR_AUTHORIZATION)
            && (pstHeader->GetHdrType() != ESIPHDR_PROXYAUTHORIZATION))
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    SipAuthBase* pAuth = DYNAMIC_CAST(SipAuthBase*, pstHeader);

    if (pAuth->SetValue(strScheme.GetStr()) != SIP_TRUE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL SetContent(IN CONST IMS_BYTE *pContent, IN IMS_SINT32 nContentLength,
        IN_OUT SipMsgBody *&pstMsgBody)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstMsgBody == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    IMS_TRACE_D("SIPStack::SetContent pstMsgBody->GetBodyType() = [%d]",
            pstMsgBody->GetBodyType(), 0, 0);

    if (pstMsgBody->GetBodyType() == ESIPINVALIDBODY)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    if (pstMsgBody->SetMsgBuffer((const SIP_CHAR*)pContent,
            (IMS_UINT32)nContentLength) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL SetHeader(IN SipHeaderBase*pstHeader, IN_OUT SipMessage *&pstMessage)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    IMS_SINT32 nType = (IMS_SINT32)pstHeader->GetHdrType();

    if ((nType <= ESIPHDR_INVALID) || (nType >= ESIPHDR_END))
    {
        IMS_TRACE_D("SetHeader -- Invalid header type[%d]", nType, 0, 0);
        return IMS_NULL;
    }

    /*The existing value of the header will be overwritten with new value.*/
    /*Applied only for Contact header as of now*/
    if (pstMessage->SetHeader(pstHeader) == SIP_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL SetMethod(IN CONST SIPMethod &objMethod, IN_OUT SipMessage *&pstMessage)
{
    if (pstMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SipEn_MsgType eMsgType = static_cast<SipEn_MsgType>(pstMessage->GetMsgType());

    if (eMsgType == ESIP_REQTYPE)
    {
        SipRequestLine* pReq = pstMessage->GetReqLine();

        if (pReq != IMS_NULL)
        {
            pReq->SetMethod(objMethod.ToString().GetStr());
            pReq->SipDelete();
            return IMS_TRUE;
        }
    }
    else
    {
        SipCSeqHeader* pCSeq = (SipCSeqHeader *)pstMessage->GetHdrObj(ESIPHDR_CSEQ);

        if (pCSeq != IMS_NULL)
        {
            pCSeq->SetMethod(objMethod.ToString().GetStr());
            pCSeq->SipDelete();
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL SetMIMEHeader(IN IMS_SINT32 nType, IN SipHeaderBase *pstHeader,
        IN_OUT SipMsgBody *&pstMsgBody)
{
    (void) nType;

    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if ((pstMsgBody == IMS_NULL)
            || (pstHeader == IMS_NULL))
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    if (pstMsgBody->SetMimeHdr(pstHeader) == SIP_TRUE)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL SetMIMEHeader(IN IMS_SINT32 nType, IN CONST AString &strName,
        IN CONST AString &strBody, IN_OUT SipMsgBody *&pstMsgBody)
{
    SIPStackError(EERR_NOERR);

    if (pstMsgBody == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    AString strHdrName(strName);
    switch (nType)
    {
        case ISIPMessageBodyPart::CONTENT_TYPE:
        {
            nType = ESIPHDR_CONTENTTYPE;
            break;
        }
        case ISIPMessageBodyPart::CONTENT_DISPOSITION:
        {
            nType = ESIPHDR_CONTENTDISPOSITION;
            break;
        }
        case ISIPMessageBodyPart::CONTENT_TRANSFER_ENCODING:
        {
            nType = ESIPHDR_CONTENTENCODING;
            break;
        }
        case ISIPMessageBodyPart::CONTENT_ID:
        {
            strHdrName = SIPHeaderName::CONTENT_ID;
            nType = ESIPHDR_UNKNOWN;
            break;
        }
        case ISIPMessageBodyPart::CONTENT_DESCRIPTION:
        {
            strHdrName = SIPHeaderName::CONTENT_DESCRIPTION;
            nType = ESIPHDR_UNKNOWN;
            break;
        }
        default:
        {
            nType = ESIPHDR_UNKNOWN;
            break;
        }
    }

    SipHeaderBase *pstHeader = DecodeHeader(nType, strHdrName, strBody);

    if (pstHeader == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::PARSING_ERROR);
        return IMS_FALSE;
    }

    if (pstMsgBody->SetMimeHdr(pstHeader) == SIP_FALSE)
    {
        FreeHeader(pstHeader);
        SIPPrivate::SetLastError(SIPError::PARSING_ERROR);
        return IMS_FALSE;
    }

    FreeHeader(pstHeader);
    return IMS_TRUE;
}

/*

Remarks

*/
SipParameters* GetParameters(IN SipHeaderBase* pHeader, IN IMS_BOOL bCreateIfNotPresent)
{
    // TODO: this logic should be reviewed about the SipParameters variable handling.
    SipParameters* pParams = pHeader->GetParameters();

    if ((bCreateIfNotPresent == IMS_TRUE) && (pParams == IMS_NULL))
    {
        pHeader->InitParameters(IMS_NULL);

        pParams = pHeader->GetParameters();
    }

    return pParams;
}

GLOBAL IMS_BOOL SetParameter(IN SipHeaderBase* pstHeader,
        IN CONST AString &strName, IN CONST AString &strValue)
{
    SIPStackError(EERR_NOERR);

    if (pstHeader == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_FALSE;
    }

    SIP_CHAR *pszValue = (strValue.GetLength() > 0) ? (SIP_CHAR *)strValue.GetStr() : SIP_NULL;

    if ((pstHeader->GetHdrType() == ESIPHDR_AUTHORIZATION)
            || (pstHeader->GetHdrType() == ESIPHDR_PROXYAUTHORIZATION))
    {
        SipAuthBase *pstAuthHdr = (SipAuthBase *)pstHeader;
        return pstAuthHdr->SetParams((SIP_CHAR *)strName.GetStr(), pszValue, SIP_FALSE);
    }
    else
    {
        SipParameters* pstSCHdr = GetParameters(pstHeader, IMS_TRUE);

        if (pstSCHdr == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (pstSCHdr->SetParamValue((SIP_CHAR*)strName.GetStr(), pszValue) != SIP_TRUE)
        {
            SIPStackError(EERR_NOEXISTS);
            return IMS_FALSE;
        }

    }
    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL SetRequestLine(IN CONST AString &strMethod, IN CONST AString &strURI,
        IN_OUT SipMessage *&pstMessage)
{
    SipAddrSpec* pAddrSpec = new SipAddrSpec();
    pAddrSpec->DecodeAddrSpec((SIP_CHAR*)strURI.GetStr(), strURI.GetLength());

    SipRequestLine* pReqLine = new SipRequestLine(
            (SIP_CHAR*)strMethod.GetStr(), pAddrSpec, SIP_SIPVER);

    if (pstMessage->SetRequestline(pReqLine) == SIP_TRUE)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL SetRequestUri(IN SipAddrSpec *pstAddrSpec, IN_OUT SipMessage *&pstMessage)
{
    SipRequestLine* pReqLine = pstMessage->GetReqLine();

    if (pReqLine != IMS_NULL)
    {
        pReqLine->SetReqUri(pstAddrSpec);
        pReqLine->SipDelete();
    }

    //set the message type as req or resp
    pstMessage->SetMessageType(ESIP_REQTYPE);

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL SetStatusLine(IN IMS_SINT32 nStatusCode, IN CONST AString &strReasonPhrase,
        IN_OUT SipMessage *&pstMessage)
{
    AString strStatusCode;

    strStatusCode.SetNumber(nStatusCode);

    SipStatusLine* pStatusLine = new SipStatusLine(SIP_SIPVER,
            (SIP_CHAR*)strStatusCode.GetStr(), (SIP_CHAR*)strReasonPhrase.GetStr());

    if (pstMessage->SetStatusLine(pStatusLine) == SIP_TRUE)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

GLOBAL IMS_BOOL SetUnknownHeader(IN SipHeaderBase *pstHeader,
        IN CONST AString &strName, IN_OUT SipMessage *&pstMessage)
{
    if (pstHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pstHeader->SetHdrType(sipGetHdrType(strName.GetStr()));

    return SetHeader(pstHeader, pstMessage);
}

GLOBAL IMS_BOOL IsMessageBodyCompressed(IN SipMessage *pstMessage)
{
    //4 Check if the Content-Encoding contains the compression algorithm or not
    //4 As default, consider it to 'gzip'. It needs to be consider other compression algorithm.

    SipHeaderBase *pstContentEncoding = GetHeader(pstMessage, ESIPHDR_CONTENTENCODING);

    //-----------------------------------------------------------------------------------------
    if (IsValidHeader(pstContentEncoding) == IMS_TRUE)
    {
        const IMS_CHAR *pszEncoding = pstContentEncoding->GetValue();
        AString strEncoding(pszEncoding);

        FreeHeader(pstContentEncoding);

        if (strEncoding.EqualsIgnoreCase("gzip"))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

GLOBAL IMS_BOOL UncompressMessageBody(IN SipMessage *pstMessage)
{
    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        SIPStackError(EMSGERR_INVALIDHDRPARAM);
        return IMS_FALSE;
    }

    SipMsgBodyList *pstMsgBodyList = pstMessage->GetMsgBodyList();

    if (pstMsgBodyList == IMS_NULL)
    {
        IMS_TRACE_E(0, "Retrieving message body list failed.", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_UINT32 nBodyCount = pstMsgBodyList->GetMsgBodyCount();

    if (nBodyCount > 1)
    {
        SIPStackError(E_ERR_INVALIDBODY);
        IMS_TRACE_E(0, "There are many SIP message bodies", 0, 0, 0);
        pstMsgBodyList->SipDelete();
        return IMS_FALSE;
    }

    if (nBodyCount == 0)
    {
        IMS_TRACE_D("___ NO SIP MESSAGE BODY ___", 0, 0, 0);
        pstMsgBodyList->SipDelete();
        return IMS_TRUE;
    }

    SipMsgBody *pstMsgBody = pstMsgBodyList->GetBodyByIndex(0);

    if (pstMsgBody == IMS_NULL)
    {
        IMS_TRACE_E(0, "Getting SIP message body failed", 0, 0, 0);
        pstMsgBodyList->SipDelete();
        return IMS_FALSE;
    }

    /*SIP considers the compressed body as single body*/
    if (pstMsgBody->GetBodyType() != ESIPSINGLEBODY)
    {
        IMS_TRACE_E(0, "Message body type not SingleBody, unable to uncompress.", 0, 0, 0);
        pstMsgBody->SipDelete();
        pstMsgBodyList->SipDelete();
        return IMS_FALSE;
    }

    IMS_CHAR* pszBuffer = IMS_NULL;

    pstMsgBody->GetMsgBuffer(&pszBuffer);

    if (pszBuffer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Message Buffer NULL", 0, 0, 0);
        pstMsgBody->SipDelete();
        pstMsgBodyList->SipDelete();
        return IMS_FALSE;
    }

    //4 Check if the Content-Encoding contains the compression algorithm or not
    //4 As default, consider it to 'gzip'. It needs to be consider other compression algorithm.
    ByteArray objBodyPart;
    ByteArray objCompBodyPart;

    IMS_UINT32 uiBuffLen = 0;
    pstMsgBody->GetMsgBuffLen(&uiBuffLen);
    objCompBodyPart.Attach(reinterpret_cast<const IMS_BYTE*>(pszBuffer), (IMS_SINT32)uiBuffLen);

    if (!IMS_UTIL_ZLIB_Uncompress(objCompBodyPart, objBodyPart))
    {
        IMS_TRACE_E(0, "Uncompressing a body part failed", 0, 0, 0);
        pstMsgBody->SipDelete();
        pstMsgBodyList->SipDelete();
        DeleteStackString(pszBuffer);
        return IMS_FALSE;
    }

    if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
    {
        IMS_TRACE_TEXT("gzip::uncompression", objBodyPart.GetData(), objBodyPart.GetLength());
    }

    pstMsgBody->SetMsgBuffer(reinterpret_cast<const IMS_CHAR *>(objBodyPart.GetData()),
            (IMS_UINT32)objBodyPart.GetLength());

    pstMsgBody->SipDelete();
    pstMsgBodyList->SipDelete();
    DeleteStackString(pszBuffer);

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL UpdateSentProtocol(IN SipMessage *pstMessage, IN CONST AString &strSentProtocol)
{
    //---------------------------------------------------------------------------------------------
    SipViaHeader* pVia = DYNAMIC_CAST(SipViaHeader*, GetHeader(pstMessage, ESIPHDR_VIA));
    const SIP_CHAR *pszProtocol = strSentProtocol.GetStr();
    IMS_UINT32 nStartIndex = 0;

    if (strSentProtocol.StartsWith(SIP::STR_SIP_VERSION))
    {
        // "SIP/2.0/" : 8 characters
        nStartIndex = 8;
    }

    if (pVia->SetTransport(pszProtocol + nStartIndex) ==  SIP_TRUE)
    {
        FreeHeader(pVia);
        return IMS_TRUE;
    }

    FreeHeader(pVia);

    return IMS_FALSE;
}

/*

Remarks

*/
GLOBAL void DisplayBadHeaders(IN SipMessage *pstMessage)
{
#ifdef SIP_BADMESSAGE_PARSING
    SIPStackError(EERR_NOERR);
    IMS_TRACE_I("___ SIP bad headers - S ___", 0, 0, 0);

    SipHeaderList *pBadHdrList = pstMessage->GetBadHdrs();
    IMS_SINT32 nCount = pBadHdrList->GetSize();

    for (IMS_SINT32 i = 0; i < nCount; i++)
    {
        SipBadHeader *pBadHdr = DYNAMIC_CAST(SipBadHeader*, pBadHdrList->GetObj(i));

        if (pBadHdr == IMS_NULL)
        {
            continue;
        }

        const IMS_CHAR *pszHdrName = pBadHdr->GetHeaderName();
        const IMS_CHAR *pszHdrValue = pBadHdr->GetValue();

        IMS_TRACE_I("    (%d) %s: %s", i, _TRACE_S_(pszHdrName), _TRACE_S_(pszHdrValue));

        pBadHdr->SipDelete();
    }
    /*Memory leak fix: Delete Bad Header list after display, it's not freed for
      Non-Mandatory SIP Headers.*/
    pstMessage->DeleteBadHdrList();

    IMS_TRACE_I("___ SIP bad headers - E ___", 0, 0, 0);
#else
   (void)pstMessage;
#endif
}

/*

Remarks

*/
GLOBAL IMS_SINT32 GetBadHeaderCount(IN SipMessage *pstMessage)
{
#ifdef SIP_BADMESSAGE_PARSING
    return pstMessage->GetBadHeaderCount();
#else
   (void)pstMessage;
   return 0;
#endif
}

/*

Remarks

*/
GLOBAL IMS_BOOL HasMandatoryHeaders(IN SipMessage *pstMessage)
{
#ifdef SIP_BADMESSAGE_PARSING
    return pstMessage->HasMandatoryHdrs();
#else
   (void)pstMessage;
   return IMS_TRUE;
#endif
}

/*

Remarks

*/
/// APIs for SIP authentication
GLOBAL IMS_BOOL GetEntityBody(IN SipMessage *pstMessage, OUT AString &strEntityBody)
{
#ifdef SIP_TOBEPORTED
    IMS_UINT32 nMBCount = 0;
    RCPtr<SIPMessageBuffer> pMessageBuffer = SIPMessageBuffer::GetInstance();
    IMS_UINT32 nBuffLen = pMessageBuffer->GetLength();
    IMS_CHAR *pTmpBuffer = reinterpret_cast<IMS_CHAR*>(pMessageBuffer->GetBuffer());

    //---------------------------------------------------------------------------------------------
    nMBCount = pstMessage->GetMsgBodyCount();

    if (nMBCount == 0)
    {
        SIPStackError(EERR_NOEXISTS);
        return IMS_TRUE;
    }

    SipHeaderBase *pstHeader = GetHeader(pstMessage, ESIPHDR_CONTENTTYPE);

    if (GetLastError() != EERR_NOERR)
    {
        FreeHeader(stHeader);
        return IMS_FALSE;
    }

    IMS_MEM_Memset(pTmpBuffer, 0x00, nBuffLen);

    // Allocate a memory and initialize the entity-body buffer.
    // This will be filled with the entire entity-body and then passed to
    // IMSDigest_CalculateHEntity() to calculate the hash of the entity-body.
    if (sip_formMimeBody(IMS_NULL, pstMessage->slMessageBody,
            static_cast<SipContentTypeHeader*>(stHeader.pHeader),
            pTmpBuffer, &nBuffLen, SIPStackError()) == SIP_FALSE)
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
    (void) pstMessage;
    strEntityBody = AString::ConstNull();
#endif
    return IMS_TRUE;
}

/// APIs for SIP transaction layer
/*

Remarks

*/
GLOBAL SIPTxnKey* CreateTxnKey(IN SipMessage *pstMessage)
{
    //---------------------------------------------------------------------------------------------
    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    SipViaHeader *pViaHeader = DYNAMIC_CAST(SipViaHeader*, GetHeader(pstMessage, ESIPHDR_VIA));

    if (pViaHeader == IMS_NULL)
    {
        return IMS_NULL;
    }

    AString strViaBranch = GetParameter(pViaHeader, SIP::STR_BRANCH);

    FreeHeader(pViaHeader);

    if (strViaBranch.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Via branch is null", 0, 0, 0);
        return IMS_NULL;
    }

    SIPMethod objMethod = GetMethod(pstMessage);
    IMS_SINT32 nStatusCode = 0;
    IMS_UINT32 nCSeq = GetCSeqNumber(pstMessage);

    if (!IsRequestMessage(pstMessage))
    {
        nStatusCode = GetStatusCode(pstMessage);
    }

    return new SIPTxnKey(objMethod, nStatusCode, strViaBranch, nCSeq);
}

/*

Remarks

*/
GLOBAL SipTxnKey* CreateTxnKey(IN SipMessage *pstMessage, IN IMS_SINT32 /*nAPICalled*/)
{
    //---------------------------------------------------------------------------------------------

    SIPStackError(EERR_NOERR);

    if (pstMessage == IMS_NULL)
    {
        SIPStackError(EERR_INVALIDPARAM);
        return IMS_NULL;
    }

    IMS_UINT16 nError = 0;

    return new SipTxnKey(pstMessage, &nError);
}

/*

Remarks

*/
GLOBAL SIPTxnKey* CreateTxnKeyFromKey(IN SipTxnKey *pstTxnKey)
{
    //---------------------------------------------------------------------------------------------

    if (pstTxnKey == IMS_NULL)
    {
        return IMS_NULL;
    }

    SIPMethod objMethod(static_cast<const IMS_CHAR*>(pstTxnKey->GetMethod()));
    AString strViaBranch(TxnKey_GetViaBranch(pstTxnKey));

    return new SIPTxnKey(objMethod,
            pstTxnKey->GetRespCode(),
            strViaBranch,
            pstTxnKey->GetCSeqNum());
}

/*

Remarks

*/
GLOBAL IMS_BOOL CompareTxnKeys(IN SipTxnKey *pstTxnKey1, IN SipTxnKey *pstTxnKey2)
{
    //---------------------------------------------------------------------------------------------

    if (pstTxnKey1 == SIP_NULL)
    {
        return IMS_FALSE;
    }

    if (pstTxnKey1->CompareKeys(pstTxnKey2) != SIP_MATCHES)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL CompareTxnKeysForAck(IN SipTxnKey *pstTxnKey1, IN SipTxnKey *pstTxnKey2)
{
    //---------------------------------------------------------------------------------------------

    if ((pstTxnKey1 == IMS_NULL) || (pstTxnKey2 == IMS_NULL))
    {
        return IMS_FALSE;
    }

    if ((pstTxnKey1->GetCSeqNum() != pstTxnKey2->GetCSeqNum())
            || (IMS_StrCmp(pstTxnKey1->GetCallId(), pstTxnKey2->GetCallId()) != 0)
            || (IMS_StrICmp(pstTxnKey1->GetFromTag(), pstTxnKey2->GetFromTag()) != 0)
            || (IMS_StrICmp(pstTxnKey1->GetToTag(), pstTxnKey2->GetToTag()) != 0)
            || (IMS_StrICmp(pstTxnKey1->GetViaBranchParam(),
                    pstTxnKey2->GetViaBranchParam()) != 0))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL CompareTxnKeysForCancel(IN SipTxnKey *pstCancelKey, IN SipTxnKey *pstTxnKey)
{
    // Compares these values : CSeq number, Call-ID, From-Tag, Via branch parameter

    //---------------------------------------------------------------------------------------------

    if ((pstCancelKey == IMS_NULL) || (pstTxnKey == IMS_NULL))
    {
        return IMS_FALSE;
    }

    // Check Via branch parameter first.
    if (IMS_StrStr(pstCancelKey->m_pszViaBranchParam, SIP::STR_BRANCH_MAGIC_COOKIE) != IMS_NULL)
    {
        IMS_TRACE_D("Transaction Matching -----> Compliant to RFC 3261", 0, 0, 0);

        // Request was generated by a client transaction compliant to RFC 3261.
        // Therefore, the branch parameter will be unique across all transactions
        // sent by that client.
        if (IMS_StrICmp(pstCancelKey->m_pszViaBranchParam, pstTxnKey->m_pszViaBranchParam) != 0)
        {
            return IMS_FALSE;
        }

        // Check 'sent-by' info. - host
        if ((pstCancelKey->m_pszViaHost != IMS_NULL)
                && (pstTxnKey->m_pszViaHost != IMS_NULL)
                && (IMS_StrStr(pstCancelKey->m_pszViaHost, ":") != IMS_NULL)
                && (IMS_StrStr(pstTxnKey->m_pszViaHost, ":") != IMS_NULL))
        {
            // Compares IPv6 addresses
            AString strCancelHost(static_cast<const IMS_CHAR*>(pstCancelKey->m_pszViaHost));
            AString strTxnHost(static_cast<const IMS_CHAR*>(pstTxnKey->m_pszViaHost));
            IPAddress objCancelIPA(strCancelHost);
            IPAddress objTxnIPA(strTxnHost);

            if (!objCancelIPA.Equals(objTxnIPA))
            {
                return IMS_FALSE;
            }
        }
        else
        {
            if (IMS_StrICmp(pstCancelKey->m_pszViaHost, pstTxnKey->m_pszViaHost) != 0)
            {
                return IMS_FALSE;
            }
        }

        // Check 'sent-by' info. - port
        if (pstCancelKey->m_nViaHostPort != pstTxnKey->m_nViaHostPort)
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    ///// Request-URI, To-Tag, From-Tag, Call-ID, CSeq, top Via header

    // CSeq number
    if (pstCancelKey->m_nCseqNum != pstTxnKey->m_nCseqNum)
    {
        return IMS_FALSE;
    }

    // Call-ID
    if (IMS_StrCmp(pstCancelKey->m_pszCallId, pstTxnKey->m_pszCallId) != 0)
    {
        return IMS_FALSE;
    }

    // From-Tag
    if (IMS_StrICmp(pstCancelKey->m_pszFromTag, pstTxnKey->m_pszFromTag) != 0)
    {
        return IMS_FALSE;
    }

    // Via branch parameter
    if (IMS_StrICmp(pstCancelKey->m_pszViaBranchParam, pstTxnKey->m_pszViaBranchParam) != 0)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL IMS_BOOL AbortTransaction(IN SipTxnKey *pstTxnKey, IN SipTxnContext *pstTxnContext)
{
    //---------------------------------------------------------------------------------------------

    (void) pstTxnContext;

    TerminateTransaction(pstTxnKey);

    return IMS_TRUE;
}

/*

Remarks

*/
GLOBAL SipEventContext* CreateEventContext()
{
    //---------------------------------------------------------------------------------------------

    return IMS_NULL;
}

/*

Remarks

*/
GLOBAL SipTxnContext* CreateTxnContext()
{
    //---------------------------------------------------------------------------------------------
    return (SipContextUtils::GetInstance()->Sip_CreateTxnContext());
}

/*

Remarks

*/
GLOBAL void DestroyEventContext(IN SipEventContext *pstContext)
{
    //---------------------------------------------------------------------------------------------

    if (pstContext != IMS_NULL)
    {
        delete pstContext;
    }
}

/*

Remarks

*/
GLOBAL void DestroyTxnContext(IN SipTxnContext *pstContext)
{
    //---------------------------------------------------------------------------------------------
    if (pstContext != IMS_NULL)
    {
        SIPTxnContextData *pTxnContextData
                = static_cast<SIPTxnContextData*>(pstContext->pTxnContextData);

        if (pTxnContextData != IMS_NULL)
        {
            delete pTxnContextData;
            pstContext->pTxnContextData = IMS_NULL;
        }

        SipContextUtils::GetInstance()->Sip_DestroyTxnContext(pstContext);
    }
}

/*

Remarks

*/
GLOBAL void DisplayTxnKey(IN CONST SipTxnKey *pstTxnKey)
{
    if (pstTxnKey == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("___ TRANSACTION INFO. - S ___", 0, 0, 0);
    IMS_TRACE_I("\tMethod: %s, %d",
            _TRACE_S_(pstTxnKey->GetMethod()), pstTxnKey->GetCSeqNum(), 0);
    IMS_TRACE_I("\tVia Branch: %s, RSeq: %d",
            _TRACE_S_(pstTxnKey->GetViaBranchParam()), pstTxnKey->GetRSeq(), 0);

    IMS_CHAR acCallId[11+1] = { '\0', };

    IMS_TRACE_I("\tCall-ID: %s", GetLogString(pstTxnKey->GetCallId(), acCallId, 11, '@'), 0, 0);
    IMS_TRACE_D("___ TRANSACTION INFO. - E ___\r\n", 0, 0, 0);
}

/*

Remarks

*/
GLOBAL void FreeTxnKey(IN SipTxnKey *&pstTxnKey)
{
    //---------------------------------------------------------------------------------------------
    if (pstTxnKey != SIP_NULL)
    {
       pstTxnKey->SipDelete();
    }
    pstTxnKey = IMS_NULL;
}

/*

Remarks

*/
GLOBAL void FreeTxn(IN SipTxn*& pTxn)
{
    //---------------------------------------------------------------------------------------------

    if (pTxn != IMS_NULL)
    {
        pTxn->SipDelete();
    }

    pTxn = IMS_NULL;
}

/*

Remarks

*/
GLOBAL SIPTxnContextData* GetTxnContextData(IN SipEventContext *pstContext)
{
    //---------------------------------------------------------------------------------------------

    (void) pstContext;

    return IMS_NULL;
}

/*

Remarks

*/
GLOBAL void TerminateTransaction(IN SipTxnKey *pstTxnKey)
{

    //---------------------------------------------------------------------------------------------

    SipStackManager::GetInstance()->TerminateTxn(pstTxnKey);
}

/*

Remarks

*/
GLOBAL const IMS_CHAR* GetTimerTypeAsString(IN SipEn_TimerType enTimerType)
{
    static const IMS_CHAR* acTimerType[] =
        {
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
            "Timer_OTHER",
            "Timer_INVALID"
        };

    //---------------------------------------------------------------------------------------------

    return acTimerType[enTimerType];
}

/*

Remarks

*/
GLOBAL const IMS_CHAR* GetTimerTypeAsString(IN SipTimeoutData* pData)
{
    //---------------------------------------------------------------------------------------------

    if (pData == IMS_NULL)
    {
        return "__INVALID__";
    }

    return GetTimerTypeAsString(static_cast<SipEn_TimerType>(pData->GetTimerType()));
}

/*

Remarks

*/
GLOBAL void InvokeTimerCallback(IN SipTimerCallback pfnCallback,
        IN SipTimeoutData* pData, IN IMS_PVOID pvExtraParam)
{
    //---------------------------------------------------------------------------------------------

    if (pfnCallback == IMS_NULL)
    {
        return;
    }

    pfnCallback(pData, pvExtraParam);
}

/*

Remarks

*/
GLOBAL void SetTimerValues(IN SIPTimerValues *pTV, IN_OUT SipTxnContext *&pstTxnContext)
{
    if ((pTV == IMS_NULL) || (pstTxnContext->pSipTimerContext == IMS_NULL))
    {
        return;
    }

    SipTxnTimerValues *pTxnTimerValues = pstTxnContext->pSipTimerContext->pTxnSipTxnTimers;

    if (pTxnTimerValues == IMS_NULL)
    {
        return;
    }

    if (!pTV->IsSet(SIPTimerValues::TV_ALL))
    {
        return;
    }

    IMS_UINT32 nTxnTimerOptions = 0;

    if (pTV->IsSet(SIPTimerValues::TIMER_T1))
    {
        nTxnTimerOptions |= SIPTimerValues::TIMER_T1;
        pTxnTimerValues->SetTimerValue(ETXN_TIMER1, pTV->GetValue(SIPTimerValues::TIMER_T1));
    }

    if (pTV->IsSet(SIPTimerValues::TIMER_T2))
    {
        nTxnTimerOptions |= SIPTimerValues::TIMER_T2;
        pTxnTimerValues->SetTimerValue(ETXN_TIMER2, pTV->GetValue(SIPTimerValues::TIMER_T2));
    }

    if (pTV->IsSet(SIPTimerValues::TV_TIMER_B))
    {
        nTxnTimerOptions |= SIPTimerValues::TV_TIMER_B;
        pTxnTimerValues->SetTimerValue(ETXN_TIMERB, pTV->GetValue(SIPTimerValues::TV_TIMER_B));
    }

    if (pTV->IsSet(SIPTimerValues::TV_TIMER_D))
    {
        nTxnTimerOptions |= SIPTimerValues::TV_TIMER_D;
        pTxnTimerValues->SetTimerValue(ETXN_TIMERD, pTV->GetValue(SIPTimerValues::TV_TIMER_D));
    }

    if (pTV->IsSet(SIPTimerValues::TV_TIMER_F))
    {
        nTxnTimerOptions |= SIPTimerValues::TV_TIMER_F;
        pTxnTimerValues->SetTimerValue(ETXN_TIMERF, pTV->GetValue(SIPTimerValues::TV_TIMER_F));
    }

    if (pTV->IsSet(SIPTimerValues::TV_TIMER_H))
    {
        nTxnTimerOptions |= SIPTimerValues::TV_TIMER_H;
        pTxnTimerValues->SetTimerValue(ETXN_TIMERH, pTV->GetValue(SIPTimerValues::TV_TIMER_H));
    }

    if (pTV->IsSet(SIPTimerValues::TV_TIMER_I))
    {
        nTxnTimerOptions |= SIPTimerValues::TV_TIMER_I;
        pTxnTimerValues->SetTimerValue(ETXN_TIMERI, pTV->GetValue(SIPTimerValues::TV_TIMER_I));
    }

    if (pTV->IsSet(SIPTimerValues::TV_TIMER_J))
    {
        nTxnTimerOptions |= SIPTimerValues::TV_TIMER_J;
        pTxnTimerValues->SetTimerValue(ETXN_TIMERJ, pTV->GetValue(SIPTimerValues::TV_TIMER_J));
    }

    if (pTV->IsSet(SIPTimerValues::TV_TIMER_K))
    {
        nTxnTimerOptions |= SIPTimerValues::TV_TIMER_K;
        pTxnTimerValues->SetTimerValue(ETXN_TIMERK, pTV->GetValue(SIPTimerValues::TV_TIMER_K));
    }

    pstTxnContext->pSipTimerContext->nTimerOptions = nTxnTimerOptions;
    pstTxnContext->pSipTimerContext->pTxnSipTxnTimers = pTxnTimerValues;
}

/*

Remarks

*/
GLOBAL void DisplayUnknownHeaders(IN SipMessage *pstMessage)
{
    IMS_TRACE_I("___ SIP unknown headers - S ___", 0, 0, 0);

    if (pstMessage == IMS_NULL)
    {
        return;
    }

    SipHeaderList* pList = pstMessage->GetHdrList(ESIPHDR_UNKNOWN);

    if (pList != SIP_NULL)
    {
        IMS_UINT32 nSize = pList->GetSize();
        IMS_CHAR acLog[13 + 1] = { '\0', };
        IMS_BOOL bFullLog = IMS_FALSE;

        for (IMS_UINT32 i = 0; i < nSize; i++)
        {
            SipUnknownHeader* pUnknown = (SipUnknownHeader*)pList->GetObj(i);

            if (pUnknown != SIP_NULL)
            {
                const SIP_CHAR* pszHdrName = pUnknown->GetHeaderName();
                const SIP_CHAR* pszHdrValue = pUnknown->GetHeaderValue();

                acLog[0] = '\0';

                if (pszHdrName != SIP_NULL)
                {
                    if ((pszHdrName[0] == 'P') || (pszHdrName[0] == 'p'))
                    {
                        if (IMS_StrICmp(pszHdrName, "P-SKT-BYE-CAUSE") == 0)
                        {
                            bFullLog = IMS_TRUE;
                        }
                    }
                }

                if ((pszHdrName != SIP_NULL) && (pszHdrValue != SIP_NULL))
                {
                    IMS_TRACE_I("\t(U) %s: %s", pszHdrName,
                            bFullLog ? pszHdrValue : GetLogString(pszHdrValue, acLog, 13), 0);
                }

                pUnknown->SipDelete();
            }
        }
        pList->SipDelete();
    }

    pList = pstMessage->GetHdrList(ESIPHDR_REASON);

    if (pList != SIP_NULL)
    {
        IMS_UINT32 nSize = pList->GetSize();

        for (IMS_UINT32 i = 0; i < nSize; i++)
        {
            SipHeaderBase* pReason = pList->GetObj(i);

            if (pReason != SIP_NULL)
            {
                SIP_CHAR acHdrValue[SIP_HEADER_SIZE] = {0, };
                SIP_CHAR* pszHdrValue = &acHdrValue[0];

                if (pReason->EncodeHdr(&pszHdrValue) == SIP_TRUE)
                {
                    IMS_TRACE_I("\t(K) Reason: %s", acHdrValue, 0, 0);
                }

                pReason->SipDelete();
            }
        }

        pList->SipDelete();
    }

    IMS_TRACE_I("___ SIP unknown headers - E ___", 0, 0, 0);
}

// Return value: pszOutput (user mode & config-debug-off), pszInput (non-user mode)
GLOBAL const IMS_CHAR* GetLogString(IN CONST IMS_CHAR *pszInput,
        IN_OUT IMS_CHAR *pszOutput, IN IMS_SINT32 nOutSize /* > 3, excluding null char */,
        IN CONST IMS_CHAR cDelimiter /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
    {
        return pszInput;
    }

    if (nOutSize < 3)
    {
        return pszOutput;
    }

    if (pszInput == IMS_NULL)
    {
        pszOutput[0] = 'z';
        pszOutput[1] = 'z';
        pszOutput[2] = 'z';
        pszOutput[3] = '\0';
        return pszOutput;
    }
    else if (pszInput[0] == '\0')
    {
        pszOutput[0] = 'z';
        pszOutput[1] = 'z';
        pszOutput[2] = 'z';
        pszOutput[3] = '\0';
        return pszOutput;
    }

    IMS_SINT32 i = 0;
    IMS_SINT32 nMaxSize = nOutSize - 3;

    while ((pszInput[i] != '\0') && (i < nMaxSize))
    {
        if ((cDelimiter > 0) && (pszInput[i] == cDelimiter))
            break;

        pszOutput[i] = pszInput[i];
        ++i;
    }

    pszOutput[i] = 'x';
    pszOutput[i+1] = 'x';
    pszOutput[i+2] = 'x';
    pszOutput[i+3] = '\0';

    return pszOutput;
}

}// End of SIP (namespace)
