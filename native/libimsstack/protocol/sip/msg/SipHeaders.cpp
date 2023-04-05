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
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"
#include "SipDebug.h"
#include "msg/SipHeaders.h"

#define NUM_OF_MANDATORY_HEADERS 5

extern SIP_CHAR gaszSipHdr[][SIP_MAX_HDR_LEN];

const SIP_INT16 arrSipHeadersType[SipHeaderBase::TYPE_END + 1] = {1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1,
        0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
        1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0};

SipHeaderBase* (*gaFactoryArray[SipHeaderBase::TYPE_END + SIP_ONE])(SIP_INT32, SipHeaderBase*) = {
        SipHeaderBase::GetNewObj,                        // Allow
        SipAllowEventsHeader::GetNewObj,                 // AllowEvent
        SipAuthBase::GetNewObj,                          // Authorization
        SipHeaderBase::GetNewObj,                        // CallId
        SipNameAddrHeader::GetNewObj,                    //    SipHeaderBase::CONTACT
        SipNameAddrHeader::GetNewObj,                    //    SipHeaderBase::CONTACT_WILD
        SipNameAddrHeader::GetNewObj,                    //    SipHeaderBase::CONTACT_ANY
        SipHeaderBase::GetNewObj,                        //    SipHeaderBase::CONTENT_DISPOSITION
        SipHeaderBase::GetNewObj,                        //    SipHeaderBase::CONTENT_ENCODING
        SipIntegerHeader::GetNewObj,                     //    SipHeaderBase::CONTENT_LENGTH
        SipContentTypeHeader::GetNewObj,                 //    SipHeaderBase::CONTENT_TYPE //10
        SipCSeqHeader::GetNewObj,                        //    SipHeaderBase::CSEQ
        SipEventHeader::GetNewObj,                       //    SipHeaderBase::EVENT
        SipIntegerHeader::GetNewObj,                     //    SipHeaderBase::EXPIRES_DATE
        SipIntegerHeader::GetNewObj,                     //    SipHeaderBase::EXPIRES_SEC
        SipIntegerHeader::GetNewObj,                     //    SipHeaderBase::EXPIRES_ANY
        SipAcceptHeader::GetNewObj,                      //     SipHeaderBase::ACCEPT
        SipIntegerHeader::GetNewObj,                     //    SipHeaderBase::MIN_EXPIRES //added
        SipNameAddrHeader::GetNewObj,                    //    SipHeaderBase::FROM
        SipIntegerHeader::GetNewObj,                     //    SipHeaderBase::MAX_FORWARDS
        SipHeaderBase::GetNewObj,                        //    SipHeaderBase::MIME_VERSION,//20
        SipPrivacyHeader::GetNewObj,                     //    SipHeaderBase::PRIVACY,
        SipNameAddrHeader::GetNewObj,                    //    SipHeaderBase::P_PREFERRED_IDENTITY,
        SipNameAddrHeader::GetNewObj,                    //    SipHeaderBase::P_ASSERTED_IDENTITY,
        SipIntegerHeader::GetNewObj,                     //    SipHeaderBase::MIN_SE,
        SipNameAddrHeader::GetNewObj,                    //    SipHeaderBase::PATH,
        SipNameAddrHeader::GetNewObj,                    //    SipHeaderBase::P_ASSOCIATED_URI,
        SipNameAddrHeader::GetNewObj,                    //    SipHeaderBase::P_CALLED_PARTY_ID,
        SipPVisitedNetworkIdHeader::GetNewObj,           //    SipHeaderBase::P_VISITED_NETWORK_ID,
        SipPChargingFunctionAddressesHeader::GetNewObj,  //    SipHeaderBase::P_CHRG_FUN_ADDR,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::P_ACCESS_NETWORK_INFO,//30
        SipPChargingVectorHeader::GetNewObj,         //    SipHeaderBase::P_CHARGING_VECTOR,
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::SERVICE_ROUTE,
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::HISTORY_INFO,
        SipRequestDispositionHeader::GetNewObj,      //    SipHeaderBase::REQUEST_DISPOSITION,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::ACCEPT_CONTACT,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::REJECT_CONTACT,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::JOIN,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::SIP_IF_MATCH,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::SIP_ETAG,
        SipAuthBase::GetNewObj,                      //    SipHeaderBase::PROXY_AUTHENTICATE,//40
        SipAuthBase::GetNewObj,                      //    SipHeaderBase::PROXY_AUTHORIZATION,
        SipRAcKHeader::GetNewObj,                    //    SipHeaderBase::RACK,
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::RECORD_ROUTE,
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::REFERRED_BY,
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::REFER_TO,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::REPLACES,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::REQUIRE,
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::ROUTE,
        SipIntegerHeader::GetNewObj,                 //    SipHeaderBase::RSEQ,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::SECURITY_CLIENT,//50
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::SECURITY_VERIFY,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::SECURITY_SERVER,
        SipIntegerHeader::GetNewObj,                 //    SipHeaderBase::SESSION_EXPIRES,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::SUBSCRIPTION_STATE,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::SUPPORTED,
        SipTimeStampHeader::GetNewObj,               //    SipHeaderBase::TIMESTAMP,
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::TO,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::UNSUPPORTED,
        SipViaHeader::GetNewObj,                     //    SipHeaderBase::VIA,
        SipWarningHeader::GetNewObj,                 //    SipHeaderBase::WARNING,//60
        SipAuthBase::GetNewObj,                      //    SipHeaderBase::WWW_AUTHENTICATE,
        SipUnknownHeader::GetNewObj,                 //    SipHeaderBase::UNKNOWN,
        SipRetryAfterHeader::GetNewObj,              //    SipHeaderBase::RETRY_AFTER_DATE,
        SipRetryAfterHeader::GetNewObj,              //    SipHeaderBase::RETRY_AFTER_SEC,
        SipRetryAfterHeader::GetNewObj,              //    SipHeaderBase::RETRY_AFTER_ANY,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::P_EARLY_MEDIA,
        SipResourcePriorityHeader::GetNewObj,        //    SipHeaderBase::RESOURCE_PRIORITY,
        SipAcceptResourcePriorityHeader::GetNewObj,  //    SipHeaderBase::ACCEPT_RESOURCE_PRIORITY,
        SipDateHeader::GetNewObj,                    //    SipHeaderBase::DATE,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::ACCEPT_ENCODING,//70
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::ACCEPT_LANGUAGE,
        SipInfoBase::GetNewObj,                      //    SipHeaderBase::ALERT_INFO,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::ANSWER_MODE,
        SipAuthInfoHeader::GetNewObj,                //    SipHeaderBase::AUTHENTICATION_INFO,
        SipInfoBase::GetNewObj,                      //    SipHeaderBase::CALL_INFO,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::CONTENT_LANGUAGE,
        SipInfoBase::GetNewObj,                      //    SipHeaderBase::ERROR_INFO,
        SipIntegerHeader::GetNewObj,                 //    SipHeaderBase::FLOW_TIMER,
        SipIdentityHeader::GetNewObj,                //    SipHeaderBase::IDENTITY,
        SipInfoBase::GetNewObj,                      //    SipHeaderBase::IDENTITY_INFO,//80
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::IN_REPLY_TO,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::ORGANIZATION,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::P_ANSWER_STATE,
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::PERMISSION_MISSING,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::P_MEDIA_AUTHORIZATION,
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::P_PROFILE_KEY,
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::P_REFUSED_URI_LIST,
        SipHeaderBase::GetNewObj,                    //   SipHeaderBase::PRIORITY,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::PRIV_ANSWER_MODE,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::PROXY_REQUIRE,//90
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::P_SERVED_USER,
        SipInfoBase::GetNewObj,                      //    SipHeaderBase::P_USER_DATABASE,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::REASON,
        SipReferSubHeader::GetNewObj,                //    SipHeaderBase::REFER_SUB,
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::REPLY_TO,
        SIP_NULL,                                    //    SipHeaderBase::RESPONSE_KEY,
        SipUserAgentHeader::GetNewObj,               //    SipHeaderBase::SERVER,
                                                     // Server header same as user agent syntax
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::SUBJECT,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::SUPPRESS_IF_MATCH,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::TARGET_DIALOG,//100
        SipTriggerConsentHeader::GetNewObj,          //    SipHeaderBase::TRIGGER_CONSENT,
        SipUserAgentHeader::GetNewObj,               //    SipHeaderBase::USER_AGENT,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::FEATURE_CAPS,
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::GEOLOCATION,
        SipIntegerHeader::GetNewObj,                 //    SipHeaderBase::GEOLOCATION_ERROR,
        SipGeolocationRoutingHeader::GetNewObj,      //    SipHeaderBase::GEOLOCATION_ROUTING,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::INFO_PACKAGE,//110
        SipIntegerHeader::GetNewObj,                 //    SipHeaderBase::MAX_BREADTH,
        SipPAssertedServiceHeader::GetNewObj,        //    SipHeaderBase::P_ASSERTED_SERVICE,
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::POLICY_CONTACT,
        SipNameAddrHeader::GetNewObj,                //    SipHeaderBase::POLICY_ID,
        SipPPreferredServiceHeader::GetNewObj,       //    SipHeaderBase::P_PREFERRED_SERVICE,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::RECV_INFO,
        SipHeaderBase::GetNewObj,                    //    SipHeaderBase::SESSION_ID
        SIP_NULL                                     //    SipHeaderBase::TYPE_END //120
};

SipHeaders::SipHeaders()
{
    memset(m_HeaderArray, SIP_NULL, (SipHeaderBase::TYPE_END + SIP_ONE) * sizeof(SipHeaderBase*));
}

SipHeaderBase* SipHeaders::CreateCoreHdrObj(SIP_INT32 eHdrType)
{
    eHdrType = CheckAndGetHdrEnumType(eHdrType);
    if ((eHdrType >= SIP_ZERO) && eHdrType < SipHeaderBase::TYPE_END)
    {
        return gaFactoryArray[eHdrType](eHdrType, SIP_NULL);
    }
    return SIP_NULL;
}

SIP_BOOL SipHeaders::CopyHdrs(SipHeaders* pHdrs)
{
    for (SIP_UINT32 nCount = 0; nCount < SipHeaderBase::TYPE_END; nCount++)
    {
        if (pHdrs->m_HeaderArray[nCount] != SIP_NULL)
        {
            m_HeaderArray[nCount] = (IsListHdr(nCount) == SIP_TRUE)
                    ? SipHeaderList::GetNewListObj(nCount, pHdrs->m_HeaderArray[nCount])
                    : gaFactoryArray[nCount](nCount, pHdrs->m_HeaderArray[nCount]);
        }
    }
    return SIP_TRUE;
}

SipHeaderBase* SipHeaders::CloneHdrObj(SipHeaderBase* pOld)
{
    if (pOld == SIP_NULL)
    {
        return SIP_NULL;
    }

    SIP_INT32 eHdrType = pOld->GetHdrType();

    return (eHdrType == SipHeaderBase::UNKNOWN) ? SipHeaderList::GetNewListObj(eHdrType, pOld)
                                                : gaFactoryArray[eHdrType](eHdrType, pOld);
}

SipHeaders::~SipHeaders()
{
    for (SIP_INT32 nCount = 0; nCount < SipHeaderBase::TYPE_END; nCount++)
    {
        if (m_HeaderArray[nCount] != SIP_NULL)
        {
            m_HeaderArray[nCount]->SipDelete();
        }
    }
}

SipHeaderBase* SipHeaders::GetHdrObj(SIP_INT32 eHdrType, SIP_UINT16 eIndex)
{
    if (SipHeaderBase::IsHeaderTypeValid(eHdrType) == SIP_FALSE)
    {
        return SIP_NULL;
    }

    if (m_HeaderArray[eHdrType] != SIP_NULL)
    {
        if (IsListHdr(eHdrType) == SIP_TRUE)
        {
            return (static_cast<SipHeaderList*>(m_HeaderArray[eHdrType]))->GetObj(eIndex);
        }
        m_HeaderArray[eHdrType]->Increment();
        return m_HeaderArray[eHdrType];
    }

    return SIP_NULL;
}

SIP_VOID SipHeaders::OverWriteHdrObj(IN SipHeaders* pSrcHdrs, IN SIP_BOOL bIgnoreUnknownHeader)
{
    for (SIP_INT32 iCount = 0; iCount < SipHeaderBase::TYPE_END; iCount++)
    {
        if ((pSrcHdrs->m_HeaderArray[iCount] == SIP_NULL) ||
                ((iCount == SipHeaderBase::UNKNOWN) && (bIgnoreUnknownHeader == SIP_TRUE)))
        {
            continue;
        }

        SipHeaderBase* pTemp = (IsListHdr(iCount) == SIP_TRUE)
                ? SipHeaderList::GetNewListObj(iCount, pSrcHdrs->m_HeaderArray[iCount])
                : gaFactoryArray[iCount](iCount, pSrcHdrs->m_HeaderArray[iCount]);

        if (m_HeaderArray[iCount] != SIP_NULL)
        {
            m_HeaderArray[iCount]->SipDelete();
        }

        m_HeaderArray[iCount] = pTemp;
    }
}

SipHeaderBase* SipHeaders::GetHdrObj(SIP_INT32 eHdrType)
{
    if (SipHeaderBase::IsHeaderTypeValid(eHdrType) == SIP_FALSE)
    {
        return SIP_NULL;
    }
    if (m_HeaderArray[eHdrType] != SIP_NULL)
    {
        m_HeaderArray[eHdrType]->Increment();
        return m_HeaderArray[eHdrType];
    }
    return SIP_NULL;
}

SipHeaderBase* SipHeaders::GetNewHdrObj(SIP_INT32 eHdrType)
{
    eHdrType = CheckAndGetHdrEnumType(eHdrType);
    if (SipHeaderBase::IsHeaderTypeValid(eHdrType) == SIP_FALSE)
    {
        return SIP_NULL;
    }

    return (IsListHdr(eHdrType) == SIP_TRUE) ? SipHeaderList::GetNewListObj(eHdrType, SIP_NULL)
                                             : gaFactoryArray[eHdrType](eHdrType, SIP_NULL);
}

SIP_BOOL SipHeaders::RemoveHdr(SIP_INT32 eHdrType)
{
    if (SipHeaderBase::IsHeaderTypeValid(eHdrType) == SIP_FALSE)
    {
        return SIP_FALSE;
    }
    if (m_HeaderArray[eHdrType] != SIP_NULL)
    {
        (m_HeaderArray[eHdrType])->SipDelete();
    }
    m_HeaderArray[eHdrType] = SIP_NULL;
    return SIP_TRUE;
}

SIP_BOOL SipHeaders::SetHdr(SipHeaderBase* pHdr)
{
    if ((pHdr == SIP_NULL) || (pHdr->IsValidHeader() == SIP_FALSE))
    {
        return SIP_FALSE;
    }

    SIP_INT32 eHdrType = pHdr->GetHdrType();

    if (IsListHdr(eHdrType) == SIP_TRUE)
    {
        if (m_HeaderArray[eHdrType] != SIP_NULL)
        {
            RemoveHdr(eHdrType);
        }
        if (m_HeaderArray[eHdrType] == SIP_NULL)
        {
            m_HeaderArray[eHdrType] = GetNewHdrObj(eHdrType);
        }
        (static_cast<SipHeaderList*>(m_HeaderArray[eHdrType]))->AddHeader(pHdr);
    }
    else
    {
        if (m_HeaderArray[eHdrType] != SIP_NULL)
        {
            m_HeaderArray[eHdrType]->SipDelete();
        }

        m_HeaderArray[eHdrType] = pHdr;
        pHdr->Increment();
    }
    return SIP_TRUE;
}

SIP_BOOL SipHeaders::AppendHdr(SipHeaderBase* pHdr)
{
    if ((pHdr == SIP_NULL) || (pHdr->IsValidHeader() == SIP_FALSE))
    {
        return SIP_FALSE;
    }

    SIP_INT32 eHdrType = pHdr->GetHdrType();
    if (IsListHdr(eHdrType) == SIP_TRUE)
    {
        if (m_HeaderArray[eHdrType] == SIP_NULL)
        {
            m_HeaderArray[eHdrType] = GetNewHdrObj(eHdrType);
        }
        (static_cast<SipHeaderList*>(m_HeaderArray[eHdrType]))->AddHeader(pHdr);
    }
    else
    {
        if (m_HeaderArray[eHdrType] != SIP_NULL)
        {
            m_HeaderArray[eHdrType]->SipDelete();
        }
        m_HeaderArray[eHdrType] = pHdr;
        pHdr->Increment();
    }
    return SIP_TRUE;
}

SIP_BOOL SipHeaders::InsertHdr(SipHeaderBase* pHdr, SIP_UINT32 nIndex)
{
    if ((pHdr == SIP_NULL) || (pHdr->IsValidHeader() == SIP_FALSE))
    {
        return SIP_FALSE;
    }

    SIP_INT32 eHdrType = pHdr->GetHdrType();
    if (IsListHdr(eHdrType) == SIP_TRUE)
    {
        if (m_HeaderArray[eHdrType] == SIP_NULL)
        {
            m_HeaderArray[eHdrType] = GetNewHdrObj(eHdrType);
        }
        (static_cast<SipHeaderList*>(m_HeaderArray[eHdrType]))->InsertHdrAtPos(pHdr, nIndex);
    }
    else
    {
        if (m_HeaderArray[eHdrType] != SIP_NULL)
        {
            m_HeaderArray[eHdrType]->SipDelete();
        }
        m_HeaderArray[eHdrType] = pHdr;
        pHdr->Increment();
    }
    return SIP_TRUE;
}

SIP_BOOL SipHeaders::EncodeMandatoryHdrs(SIP_CHAR** ppCurrPos, SIP_UINT32 nMsgOptions)
{
    SIP_INT32 arMandatoryHeaders[NUM_OF_MANDATORY_HEADERS] = {SipHeaderBase::VIA,
            SipHeaderBase::FROM, SipHeaderBase::TO, SipHeaderBase::CALL_ID, SipHeaderBase::CSEQ};

    for (SIP_INT32 iCount = 0; iCount < NUM_OF_MANDATORY_HEADERS; iCount++)
    {
        SipHeaderBase* pHeaderObj = GetHdrObj(arMandatoryHeaders[iCount]);
        if (pHeaderObj == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Mandatory header %d unavailable",
                    arMandatoryHeaders[iCount], SIP_ZERO);
            return SIP_FALSE;
        }

        SipEncodeHdrName(arMandatoryHeaders[iCount], ppCurrPos, nMsgOptions);
        if (pHeaderObj->EncodeHdr(ppCurrPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Mandatory header %d encode fail",
                    arMandatoryHeaders[iCount], SIP_ZERO);
            pHeaderObj->SipDelete();
            return SIP_FALSE;
        }
        pHeaderObj->SipDelete();
        SIP_ENC_CRLF(*ppCurrPos);
    }

    return SIP_TRUE;
}

SIP_BOOL SipHeaders::EncodeContentHdrs(SIP_CHAR** ppCurrPos, SIP_UINT32 nMsgOptions)
{
    SipHeaderBase* pTemp = GetHdrObj(SipHeaderBase::CONTENT_TYPE);
    if (pTemp != SIP_NULL)
    {
        SipEncodeHdrName(SipHeaderBase::CONTENT_TYPE, ppCurrPos, nMsgOptions);
        if (pTemp->EncodeHdr(ppCurrPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Content type encode fail", SIP_ZERO, SIP_ZERO);
            pTemp->SipDelete();
            return SIP_FALSE;
        }
        pTemp->SipDelete();
        SIP_ENC_CRLF(*ppCurrPos);
    }
    return SIP_TRUE;
}

SIP_BOOL SipHeaders::EncodeHdrs(SIP_CHAR** ppCurrPos, SIP_UINT32 nMsgOptions)
{
    if (EncodeMandatoryHdrs(ppCurrPos, nMsgOptions) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    SIP_INT32 nHdr = SipHeaderBase::ALLOW;
    while (nHdr < SipHeaderBase::TYPE_END)
    {
        /* Ignore mandatory headers & content headers which are separately encoded*/
        if ((nHdr == SipHeaderBase::VIA) || (nHdr == SipHeaderBase::FROM) ||
                (nHdr == SipHeaderBase::TO) || (nHdr == SipHeaderBase::CALL_ID) ||
                (nHdr == SipHeaderBase::CSEQ) || (nHdr == SipHeaderBase::CONTENT_TYPE) ||
                (nHdr == SipHeaderBase::CONTENT_LENGTH) || (nHdr == SipHeaderBase::UNKNOWN))
        {
            nHdr++;
            continue;
        }

        SipHeaderBase* pHeaderObj = GetHdrObj(nHdr);
        if (pHeaderObj != SIP_NULL)
        {
            SipEncodeHdrName(nHdr, ppCurrPos, nMsgOptions);

            if (pHeaderObj->EncodeHdr(ppCurrPos, SIP_TRUE, nMsgOptions) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode %d header Fail", nHdr, SIP_ZERO);
                pHeaderObj->SipDelete();
                return SIP_FALSE;
            }
            pHeaderObj->SipDelete();
            SIP_ENC_CRLF(*ppCurrPos);
        }
        nHdr++;
    }

    if (EncodeContentHdrs(ppCurrPos, nMsgOptions) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode content header Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipHeaderBase* pHeaderObj = GetHdrObj(SipHeaderBase::UNKNOWN);

    if (pHeaderObj != SIP_NULL)
    {
        if (pHeaderObj->EncodeHdr(ppCurrPos, SIP_TRUE, nMsgOptions) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODENCODER, "Encode Unknown header Fail", SIP_ZERO, SIP_ZERO);
            pHeaderObj->SipDelete();
            return SIP_FALSE;
        }

        pHeaderObj->SipDelete();
        SIP_ENC_CRLF(*ppCurrPos);
    }

    return SIP_TRUE;
}

SIP_BOOL SipHeaders::IsListHdr(SIP_INT32 eHdrType)
{
    if ((eHdrType < SIP_ZERO) || (eHdrType >= SipHeaderBase::TYPE_END))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Wrong Header Value", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return (arrSipHeadersType[eHdrType] == 1) ? SIP_TRUE : SIP_FALSE;
}

SIP_BOOL SipHeaders::DecodeHdrs(
        SIP_CHAR* pStartPt, SIP_UINT32 nDecLen, SIP_CHAR** ppHdrName, SIP_CHAR** ppHdrBody)
{
    if (pStartPt == SIP_NULL || nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid arguments", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Skip The LWS form the back*/
    /*Update the End point*/
    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    pEndPt = SipSkipRwLWS(pStartPt, pEndPt);

    SIP_CHAR* pTempPos = SIP_NULL;

    /*Get the position previous to ":"*/
    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "colon not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pTempNext = pTempPos + SIP_TWO;
    pTempNext = SipSkipFwLWS(pTempNext, pEndPt);

    /*skip the WSP form back*/
    pTempPos = SipSkipRwWSP(pStartPt, pTempPos);

    /*Create  the header name*/
    *ppHdrName = SipCreateString(pStartPt, pTempPos);
    if (*ppHdrName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*this will return the type of header on passing name*/
    SIP_INT32 eHdrType = SipGetHdrType(*ppHdrName);

    if ((m_HeaderArray[eHdrType] != SIP_NULL) && (IsListHdr(eHdrType) == SIP_FALSE))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Single value header %d appears more than once",
                eHdrType, SIP_ZERO);
        *ppHdrBody = SipCreateString(pTempNext, pEndPt);
        return SIP_FALSE;
    }

    /*Get the header object*/
    if (m_HeaderArray[eHdrType] == SIP_NULL)
    {
        m_HeaderArray[eHdrType] = GetNewHdrObj(eHdrType);
    }

    if (m_HeaderArray[eHdrType] == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "Getting object of header %d fail", eHdrType, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Case of Unknown Header*/
    if (eHdrType == SipHeaderBase::UNKNOWN)
    {
        SipUnknownHeader* pUnknown = new SipUnknownHeader();
        if (pUnknown == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pUnknown->SetHeaderName(*ppHdrName) == SIP_FALSE)
        {
            pUnknown->SipDelete();
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Set header name fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        *ppHdrBody = SipCreateString(pTempNext, pEndPt);
        if (*ppHdrBody == SIP_NULL)
        {
            pUnknown->SipDelete();
            if (pTempNext > pEndPt)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Unknown Hdr Contain Invalid Value",
                        SIP_ZERO, SIP_ZERO);
                return SIP_TRUE;
            }
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pUnknown->SetHeaderValue(*ppHdrBody) == SIP_FALSE)
        {
            pUnknown->SipDelete();
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Set header value fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        /*Add the header into the unknown list*/
        if ((static_cast<SipHeaderList*>(m_HeaderArray[SipHeaderBase::UNKNOWN]))
                        ->AddHeader(pUnknown) == SIP_FALSE)
        {
            pUnknown->SipDelete();
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Add to list Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pUnknown->SipDelete();
    }
    else
    {
        /*Update the Start Point to the start of hdr value*/
        pStartPt = pTempNext;

        /*Update the length for decoding*/
        nDecLen = pEndPt - pStartPt + SIP_ONE;

        *ppHdrBody = SipCreateString(pTempNext, pEndPt);

        /*Call the Decoder function*/
        if (m_HeaderArray[eHdrType]->DecodeHdr(pStartPt, nDecLen) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Decode header %d fail", eHdrType, SIP_ZERO);
            return SIP_FALSE;
        }
    }

    return SIP_TRUE;
}

SIP_BOOL SipHeaders::SipEncodeHdrName(
        SIP_INT32 eHdrType, SIP_CHAR** ppMsgBuffCurrPos, SIP_UINT32 nMsgOptions)
{
    if (eHdrType < SIP_ZERO || eHdrType >= SipHeaderBase::TYPE_END)
    {
        return SIP_FALSE;
    }

    if ((nMsgOptions & SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM) ==
            SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM)
    {
        SipEncodeShortHdrName(eHdrType, ppMsgBuffCurrPos);
        return SIP_TRUE;
    }

    SipPf_Strcpy(*ppMsgBuffCurrPos, gaszSipHdr[eHdrType]);
    SipEnc_UpdateCurrPos(ppMsgBuffCurrPos);

    SIP_ENC_COLON(*ppMsgBuffCurrPos);

    SIP_ENC_SP(*ppMsgBuffCurrPos);

    return SIP_TRUE;
}

SIP_BOOL SipHeaders::SipEncodeShortHdrName(SIP_INT32 eHdrType, SIP_CHAR** ppMsgBuffCurrPos)
{
    if (eHdrType < SIP_ZERO || eHdrType >= SipHeaderBase::TYPE_END)
    {
        return SIP_FALSE;
    }

    switch (eHdrType)
    {
        case SipHeaderBase::VIA:
            SIP_ENC_SHORT_VIA(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::TO:
            SIP_ENC_SHORT_TO(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::FROM:
            SIP_ENC_SHORT_FROM(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::CALL_ID:
            SIP_ENC_SHORT_CALLID(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::CONTACT:
        case SipHeaderBase::CONTACT_WILD:
        case SipHeaderBase::CONTACT_ANY:
            SIP_ENC_SHORT_CONTACT(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::CONTENT_TYPE:
            SIP_ENC_SHORT_CONTENT_TYPE(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::CONTENT_LENGTH:
            SIP_ENC_SHORT_CONTENT_LENGTH(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::ACCEPT_CONTACT:
            SIP_ENC_SHORT_ACCEPT_CONTACT(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::SESSION_EXPIRES:
            SIP_ENC_SHORT_SESSION_EXPIRES(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::SUPPORTED:
            SIP_ENC_SHORT_SUPPORTED(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::REQUEST_DISPOSITION:
            SIP_ENC_SHORT_REQUEST_DISPOSITION(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::REFERRED_BY:
            SIP_ENC_SHORT_REFERRED_BY(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::REFER_TO:
            SIP_ENC_SHORT_REFER_TO(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::CONTENT_ENCODING:
            SIP_ENC_SHORT_CONTENT_ENCODING(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::SUBJECT:
            SIP_ENC_SHORT_SUBJECT(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::REJECT_CONTACT:
            SIP_ENC_SHORT_REJECT_CONTACT(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::EVENT:
            SIP_ENC_SHORT_EVENT(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::ALLOW_EVENTS:
            SIP_ENC_SHORT_ALLOW_EVENTS(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::IDENTITY:
            SIP_ENC_SHORT_IDENTITY(*ppMsgBuffCurrPos);
            break;
        case SipHeaderBase::IDENTITY_INFO:
            SIP_ENC_SHORT_IDENTITY_INFO(*ppMsgBuffCurrPos);
            break;
        default:
            SipPf_Strcpy(*ppMsgBuffCurrPos, gaszSipHdr[eHdrType]);
            SipEnc_UpdateCurrPos(ppMsgBuffCurrPos);
            break;
    }

    SIP_ENC_COLON(*ppMsgBuffCurrPos);

    SIP_ENC_SP(*ppMsgBuffCurrPos);

    return SIP_TRUE;
}
