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
#include "SipDebug.h"
#include "msg/SipHeaders.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"

extern SIP_CHAR gaszSipHdr[][SipMsgUtil::MAX_HDR_NAME_LEN];

SipHeaderBase* (*gaFactoryArray[SipHeaderBase::TYPE_END + SIP_ONE])(SIP_INT32, SipHeaderBase*) = {
        SipHeaderBase::GetNewObj,                // SipHeaderBase::ALLOW
        SipEventHeader::GetNewObj,               // SipHeaderBase::ALLOW_EVENTS
        SipAuthBase::GetNewObj,                  // SipHeaderBase::AUTHORIZATION
        SipHeaderBase::GetNewObj,                // SipHeaderBase::CALL_ID
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::CONTACT
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::CONTACT_WILD
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::CONTACT_ANY
        SipHeaderBase::GetNewObj,                // SipHeaderBase::CONTENT_DISPOSITION
        SipHeaderBase::GetNewObj,                // SipHeaderBase::CONTENT_ENCODING
        SipIntegerHeader::GetNewObj,             // SipHeaderBase::CONTENT_LENGTH
        SipContentTypeHeader::GetNewObj,         // SipHeaderBase::CONTENT_TYPE
        SipCSeqHeader::GetNewObj,                // SipHeaderBase::CSEQ
        SipEventHeader::GetNewObj,               // SipHeaderBase::EVENT
        SipIntegerHeader::GetNewObj,             // SipHeaderBase::EXPIRES_DATE
        SipIntegerHeader::GetNewObj,             // SipHeaderBase::EXPIRES_SEC
        SipIntegerHeader::GetNewObj,             // SipHeaderBase::EXPIRES_ANY
        SipContentTypeHeader::GetNewObj,         // SipHeaderBase::ACCEPT
        SipIntegerHeader::GetNewObj,             // SipHeaderBase::MIN_EXPIRES
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::FROM
        SipIntegerHeader::GetNewObj,             // SipHeaderBase::MAX_FORWARDS
        SipHeaderBase::GetNewObj,                // SipHeaderBase::MIME_VERSION
        SipPrivacyHeader::GetNewObj,             // SipHeaderBase::PRIVACY
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::P_PREFERRED_IDENTITY
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::P_ASSERTED_IDENTITY
        SipIntegerHeader::GetNewObj,             // SipHeaderBase::MIN_SE
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::PATH
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::P_ASSOCIATED_URI
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::P_CALLED_PARTY_ID
        SipPVisitedNetworkIdHeader::GetNewObj,   // SipHeaderBase::P_VISITED_NETWORK_ID
        SipPChargingVectorHeader::GetNewObj,     // SipHeaderBase::P_CHRG_FUN_ADDR
        SipHeaderBase::GetNewObj,                // SipHeaderBase::P_ACCESS_NETWORK_INFO
        SipPChargingVectorHeader::GetNewObj,     // SipHeaderBase::P_CHARGING_VECTOR
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::SERVICE_ROUTE
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::HISTORY_INFO
        SipRequestDispositionHeader::GetNewObj,  // SipHeaderBase::REQUEST_DISPOSITION
        SipHeaderBase::GetNewObj,                // SipHeaderBase::ACCEPT_CONTACT
        SipHeaderBase::GetNewObj,                // SipHeaderBase::REJECT_CONTACT
        SipHeaderBase::GetNewObj,                // SipHeaderBase::JOIN
        SipHeaderBase::GetNewObj,                // SipHeaderBase::SIP_IF_MATCH
        SipHeaderBase::GetNewObj,                // SipHeaderBase::SIP_ETAG
        SipAuthBase::GetNewObj,                  // SipHeaderBase::PROXY_AUTHENTICATE
        SipAuthBase::GetNewObj,                  // SipHeaderBase::PROXY_AUTHORIZATION
        SipRAcKHeader::GetNewObj,                // SipHeaderBase::RACK
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::RECORD_ROUTE
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::REFERRED_BY
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::REFER_TO
        SipHeaderBase::GetNewObj,                // SipHeaderBase::REPLACES
        SipHeaderBase::GetNewObj,                // SipHeaderBase::REQUIRE
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::ROUTE
        SipIntegerHeader::GetNewObj,             // SipHeaderBase::RSEQ
        SipHeaderBase::GetNewObj,                // SipHeaderBase::SECURITY_CLIENT
        SipHeaderBase::GetNewObj,                // SipHeaderBase::SECURITY_VERIFY
        SipHeaderBase::GetNewObj,                // SipHeaderBase::SECURITY_SERVER
        SipIntegerHeader::GetNewObj,             // SipHeaderBase::SESSION_EXPIRES
        SipHeaderBase::GetNewObj,                // SipHeaderBase::SUBSCRIPTION_STATE
        SipHeaderBase::GetNewObj,                // SipHeaderBase::SUPPORTED
        SipTimeStampHeader::GetNewObj,           // SipHeaderBase::TIMESTAMP
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::TO
        SipHeaderBase::GetNewObj,                // SipHeaderBase::UNSUPPORTED
        SipViaHeader::GetNewObj,                 // SipHeaderBase::VIA
        SipWarningHeader::GetNewObj,             // SipHeaderBase::WARNING
        SipAuthBase::GetNewObj,                  // SipHeaderBase::WWW_AUTHENTICATE
        SipUnknownHeader::GetNewObj,             // SipHeaderBase::UNKNOWN
        SipRetryAfterHeader::GetNewObj,          // SipHeaderBase::RETRY_AFTER_DATE
        SipRetryAfterHeader::GetNewObj,          // SipHeaderBase::RETRY_AFTER_SEC
        SipRetryAfterHeader::GetNewObj,          // SipHeaderBase::RETRY_AFTER_ANY
        SipHeaderBase::GetNewObj,                // SipHeaderBase::P_EARLY_MEDIA
        SipResourcePriorityHeader::GetNewObj,    // SipHeaderBase::RESOURCE_PRIORITY
        SipResourcePriorityHeader::GetNewObj,    // SipHeaderBase::ACCEPT_RESOURCE_PRIORITY
        SipDateHeader::GetNewObj,                // SipHeaderBase::DATE
        SipHeaderBase::GetNewObj,                // SipHeaderBase::ACCEPT_ENCODING
        SipHeaderBase::GetNewObj,                // SipHeaderBase::ACCEPT_LANGUAGE
        SipInfoBase::GetNewObj,                  // SipHeaderBase::ALERT_INFO
        SipHeaderBase::GetNewObj,                // SipHeaderBase::ANSWER_MODE
        SipAuthInfoHeader::GetNewObj,            // SipHeaderBase::AUTHENTICATION_INFO
        SipInfoBase::GetNewObj,                  // SipHeaderBase::CALL_INFO
        SipHeaderBase::GetNewObj,                // SipHeaderBase::CONTENT_LANGUAGE
        SipInfoBase::GetNewObj,                  // SipHeaderBase::ERROR_INFO
        SipIntegerHeader::GetNewObj,             // SipHeaderBase::FLOW_TIMER
        SipIdentityHeader::GetNewObj,            // SipHeaderBase::IDENTITY
        SipInfoBase::GetNewObj,                  // SipHeaderBase::IDENTITY_INFO
        SipHeaderBase::GetNewObj,                // SipHeaderBase::IN_REPLY_TO
        SipHeaderBase::GetNewObj,                // SipHeaderBase::ORGANIZATION
        SipHeaderBase::GetNewObj,                // SipHeaderBase::P_ANSWER_STATE
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::PERMISSION_MISSING
        SipHeaderBase::GetNewObj,                // SipHeaderBase::P_MEDIA_AUTHORIZATION
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::P_PROFILE_KEY
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::P_REFUSED_URI_LIST
        SipHeaderBase::GetNewObj,                // SipHeaderBase::PRIORITY,
        SipHeaderBase::GetNewObj,                // SipHeaderBase::PRIV_ANSWER_MODE
        SipHeaderBase::GetNewObj,                // SipHeaderBase::PROXY_REQUIRE
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::P_SERVED_USER
        SipInfoBase::GetNewObj,                  // SipHeaderBase::P_USER_DATABASE
        SipHeaderBase::GetNewObj,                // SipHeaderBase::REASON
        SipReferSubHeader::GetNewObj,            // SipHeaderBase::REFER_SUB
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::REPLY_TO
        SIP_NULL,                                // SipHeaderBase::RESPONSE_KEY
        SipUserAgentHeader::GetNewObj,           // SipHeaderBase::SERVER
        SipHeaderBase::GetNewObj,                // SipHeaderBase::SUBJECT
        SipHeaderBase::GetNewObj,                // SipHeaderBase::SUPPRESS_IF_MATCH
        SipHeaderBase::GetNewObj,                // SipHeaderBase::TARGET_DIALOG
        SipTriggerConsentHeader::GetNewObj,      // SipHeaderBase::TRIGGER_CONSENT
        SipUserAgentHeader::GetNewObj,           // SipHeaderBase::USER_AGENT
        SipHeaderBase::GetNewObj,                // SipHeaderBase::FEATURE_CAPS
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::GEOLOCATION
        SipIntegerHeader::GetNewObj,             // SipHeaderBase::GEOLOCATION_ERROR
        SipGeolocationRoutingHeader::GetNewObj,  // SipHeaderBase::GEOLOCATION_ROUTING
        SipHeaderBase::GetNewObj,                // SipHeaderBase::INFO_PACKAGE
        SipIntegerHeader::GetNewObj,             // SipHeaderBase::MAX_BREADTH,
        SipPPreferredServiceHeader::GetNewObj,   // SipHeaderBase::P_ASSERTED_SERVICE
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::POLICY_CONTACT
        SipNameAddrHeader::GetNewObj,            // SipHeaderBase::POLICY_ID
        SipPPreferredServiceHeader::GetNewObj,   // SipHeaderBase::P_PREFERRED_SERVICE
        SipHeaderBase::GetNewObj,                // SipHeaderBase::RECV_INFO
        SipHeaderBase::GetNewObj,                // SipHeaderBase::SESSION_ID
        SIP_NULL                                 // SipHeaderBase::TYPE_END
};

SipHeaders::SipHeaders() :
        m_objHeaders(SipMap<SIP_INT32, SipHeaderBase*>())
{
}

SipHeaderBase* SipHeaders::CreateCoreHdrObj(SIP_INT32 eHdrType)
{
    eHdrType = SipMsgUtil::CheckAndGetHeaderType(eHdrType);
    if ((eHdrType >= SIP_ZERO) && eHdrType < SipHeaderBase::TYPE_END)
    {
        return gaFactoryArray[eHdrType](eHdrType, SIP_NULL);
    }
    return SIP_NULL;
}

SIP_BOOL SipHeaders::CopyHdrs(SipHeaders* pHdrs)
{
    for (SIP_UINT32 nIndex = 0; nIndex < SipHeaderBase::TYPE_END; nIndex++)
    {
        SipHeaderBase* pHeader = pHdrs->GetHeader(nIndex);
        if (pHeader != SIP_NULL)
        {
            SetHeader(nIndex, GetNewHdrObj(nIndex, pHeader));
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
    for (SIP_INT32 nIndex = 0; nIndex < SipHeaderBase::TYPE_END; nIndex++)
    {
        SetHeader(nIndex, SIP_NULL);
    }
}

SipHeaderBase* SipHeaders::GetHdrObj(SIP_INT32 eHdrType, SIP_UINT16 eIndex)
{
    if (SipHeaderBase::IsHeaderTypeValid(eHdrType) == SIP_FALSE)
    {
        return SIP_NULL;
    }

    SipHeaderBase* pHeader = GetHeader(eHdrType);

    if (pHeader != SIP_NULL)
    {
        if (IsListHdr(eHdrType) == SIP_TRUE)
        {
            return (static_cast<SipHeaderList*>(pHeader))->GetObj(eIndex);
        }
        pHeader->Increment();
        return pHeader;
    }

    return SIP_NULL;
}

SIP_VOID SipHeaders::OverWriteHdrObj(IN SipHeaders* pSrcHdrs, IN SIP_BOOL bIgnoreUnknownHeader)
{
    for (SIP_INT32 nIndex = 0; nIndex < SipHeaderBase::TYPE_END; nIndex++)
    {
        SipHeaderBase* pHeader = pSrcHdrs->GetHeader(nIndex);

        if ((pHeader == SIP_NULL) ||
                ((nIndex == SipHeaderBase::UNKNOWN) && (bIgnoreUnknownHeader == SIP_TRUE)))
        {
            continue;
        }

        SetHeader(nIndex, GetNewHdrObj(nIndex, pHeader));
    }
}

SipHeaderBase* SipHeaders::GetHdrObj(SIP_INT32 eHdrType)
{
    if (SipHeaderBase::IsHeaderTypeValid(eHdrType) == SIP_FALSE)
    {
        return SIP_NULL;
    }

    SipHeaderBase* pHeader = GetHeader(eHdrType);

    if (pHeader != SIP_NULL)
    {
        pHeader->Increment();
        return pHeader;
    }
    return SIP_NULL;
}

SipHeaderBase* SipHeaders::GetHeader(SIP_INT32 eHdrType)
{
    SIP_SLONG nIndex = m_objHeaders.GetIndexOfKey(eHdrType);

    return (nIndex != -1) ? m_objHeaders.GetValueAt(nIndex) : SIP_NULL;
}

SIP_VOID SipHeaders::SetHeader(SIP_INT32 eHdrType, SipHeaderBase* pHeader)
{
    SIP_SLONG nIndex = m_objHeaders.GetIndexOfKey(eHdrType);

    if (nIndex != -1)
    {
        SipHeaderBase* pHdr = m_objHeaders.GetValueAt(nIndex);
        if (pHdr != SIP_NULL)
        {
            pHdr->SipDelete();
        }
        m_objHeaders.RemoveAt(nIndex);
    }

    m_objHeaders.Add(eHdrType, pHeader);
}

SipHeaderBase* SipHeaders::GetNewHdrObj(SIP_INT32 eHdrType, SipHeaderBase* pHeader /* = SIP_NULL */)
{
    eHdrType = SipMsgUtil::CheckAndGetHeaderType(eHdrType);
    if (SipHeaderBase::IsHeaderTypeValid(eHdrType) == SIP_FALSE)
    {
        return SIP_NULL;
    }

    return (IsListHdr(eHdrType) == SIP_TRUE) ? SipHeaderList::GetNewListObj(eHdrType, pHeader)
                                             : gaFactoryArray[eHdrType](eHdrType, pHeader);
}

SIP_BOOL SipHeaders::RemoveHdr(SIP_INT32 eHdrType)
{
    if (SipHeaderBase::IsHeaderTypeValid(eHdrType) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    SetHeader(eHdrType, SIP_NULL);

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
        SipHeaderBase* pHeader = GetNewHdrObj(eHdrType);
        SetHeader(eHdrType, pHeader);

        (static_cast<SipHeaderList*>(pHeader))->AddHeader(pHdr);
    }
    else
    {
        SetHeader(eHdrType, pHdr);
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
        SipHeaderBase* pHeader = GetHeader(eHdrType);
        if (pHeader == SIP_NULL)
        {
            pHeader = GetNewHdrObj(eHdrType);
            SetHeader(eHdrType, pHeader);
        }
        (static_cast<SipHeaderList*>(pHeader))->AddHeader(pHdr);
    }
    else
    {
        SetHeader(eHdrType, pHdr);
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
        SipHeaderBase* pHeader = GetHeader(eHdrType);
        if (pHeader == SIP_NULL)
        {
            pHeader = GetNewHdrObj(eHdrType);
            SetHeader(eHdrType, pHeader);
        }
        (static_cast<SipHeaderList*>(pHeader))->InsertHdrAtPos(pHdr, nIndex);
    }
    else
    {
        SetHeader(eHdrType, pHdr);
        pHdr->Increment();
    }
    return SIP_TRUE;
}

SIP_BOOL SipHeaders::EncodeMandatoryHdrs(SIP_CHAR** ppCurrPos, SIP_UINT32 nMsgOptions)
{
    const SIP_UINT16 NUM_OF_MANDATORY_HEADERS = 5;

    SIP_INT32 arMandatoryHeaders[NUM_OF_MANDATORY_HEADERS] = {SipHeaderBase::VIA,
            SipHeaderBase::FROM, SipHeaderBase::TO, SipHeaderBase::CALL_ID, SipHeaderBase::CSEQ};

    for (SIP_INT32 nIndex = 0; nIndex < NUM_OF_MANDATORY_HEADERS; nIndex++)
    {
        SipHeaderBase* pHeaderObj = GetHdrObj(arMandatoryHeaders[nIndex]);
        if (pHeaderObj == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Mandatory header %d unavailable",
                    arMandatoryHeaders[nIndex], SIP_ZERO);
            return SIP_FALSE;
        }

        SipEncodeHdrName(arMandatoryHeaders[nIndex], ppCurrPos, nMsgOptions);
        if (pHeaderObj->EncodeHdr(ppCurrPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Mandatory header %d encode fail",
                    arMandatoryHeaders[nIndex], SIP_ZERO);
            pHeaderObj->SipDelete();
            return SIP_FALSE;
        }
        pHeaderObj->SipDelete();
        SipMsgUtil::EncodeCrlf(*ppCurrPos);
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
        SipMsgUtil::EncodeCrlf(*ppCurrPos);
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
            SipMsgUtil::EncodeCrlf(*ppCurrPos);
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
        SipMsgUtil::EncodeCrlf(*ppCurrPos);
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
    return SipHeaderBase::IsMultiValueHeader(eHdrType);
}

SIP_BOOL SipHeaders::DecodeHdrs(
        const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen, SIP_CHAR** ppHdrName, SIP_CHAR** ppHdrBody)
{
    if (pStartPt == SIP_NULL || nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid arguments", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Skip The LWS form the back*/
    /*Update the End point*/
    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    pEndPt = SipSkipRwLWS(pStartPt, pEndPt);

    const SIP_CHAR* pTempPos = SIP_NULL;

    /*Get the position previous to ":"*/
    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "colon not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pTempNext = pTempPos + SIP_TWO;
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
    SIP_INT32 eHdrType = SipMsgUtil::GetHeaderType(*ppHdrName);

    SipHeaderBase* pHeader = GetHeader(eHdrType);

    if ((pHeader != SIP_NULL) && (IsListHdr(eHdrType) == SIP_FALSE))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Single value header %d appears more than once",
                eHdrType, SIP_ZERO);
        *ppHdrBody = SipCreateString(pTempNext, pEndPt);
        return SIP_FALSE;
    }

    /*Get the header object*/
    if (pHeader == SIP_NULL)
    {
        pHeader = GetNewHdrObj(eHdrType);
        SetHeader(eHdrType, pHeader);
    }

    if (pHeader == SIP_NULL)
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

        pUnknown->SetHeaderName(*ppHdrName);
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

        pUnknown->SetHeaderValue(*ppHdrBody);
        /*Add the header into the unknown list*/
        if ((static_cast<SipHeaderList*>(pHeader))->AddHeader(pUnknown) == SIP_FALSE)
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
        if (pHeader->DecodeHdr(pStartPt, nDecLen) == SIP_FALSE)
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

    SipMsgUtil::Encode(*ppMsgBuffCurrPos, COLON);

    SipMsgUtil::Encode(*ppMsgBuffCurrPos, SPACE);

    return SIP_TRUE;
}

SIP_BOOL SipHeaders::SipEncodeShortHdrName(SIP_INT32 eHdrType, SIP_CHAR** ppMsgBuffCurrPos)
{
    if (eHdrType < SIP_ZERO || eHdrType >= SipHeaderBase::TYPE_END)
    {
        return SIP_FALSE;
    }

    SIP_CHAR cCompactName = SipMsgUtil::GetCompactHeaderName(eHdrType);

    if (cCompactName != SIP_NULL_CHAR)
    {
        *(*ppMsgBuffCurrPos) = cCompactName;
    }
    else
    {
        SipPf_Strcpy(*ppMsgBuffCurrPos, gaszSipHdr[eHdrType]);
    }

    SipEnc_UpdateCurrPos(ppMsgBuffCurrPos);
    SipMsgUtil::Encode(*ppMsgBuffCurrPos, COLON);
    SipMsgUtil::Encode(*ppMsgBuffCurrPos, SPACE);

    return SIP_TRUE;
}
