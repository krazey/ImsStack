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
#include "msg/SipGeolocationRoutingHeader.h"
#include "platform/SipString.h"

SipGeolocationRoutingHeader::SipGeolocationRoutingHeader() :
        SipHeaderBase(SipHeaderBase::GEOLOCATION_ROUTING),
        m_pGeoLocationRoutingList(SIP_NULL)
{
}

SipGeolocationRoutingHeader::SipGeolocationRoutingHeader(
        const SipGeolocationRoutingHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pGeoLocationRoutingList(SIP_NULL)
{
    if (objHeader.m_pGeoLocationRoutingList != SIP_NULL)
    {
        m_pGeoLocationRoutingList = new SipNameValue(*(objHeader.m_pGeoLocationRoutingList));
    }
}

SipGeolocationRoutingHeader::~SipGeolocationRoutingHeader()
{
    if (m_pGeoLocationRoutingList != SIP_NULL)
    {
        m_pGeoLocationRoutingList->SipDelete();
    }
}

SIP_BOOL SipGeolocationRoutingHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing geolocation-route", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return m_pGeoLocationRoutingList->Encode(objBuffer);
}

SIP_BOOL SipGeolocationRoutingHeader::Encode(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing geolocation-route", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return m_pGeoLocationRoutingList->Encode(ppCurrPos);
}

SIP_BOOL SipGeolocationRoutingHeader::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pGeoLocationRoutingList = new SipNameValue();
    if (m_pGeoLocationRoutingList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    return m_pGeoLocationRoutingList->Decode(pStartPt, pEndPt);
}

SipHeaderBase* SipGeolocationRoutingHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipGeolocationRoutingHeader(
                *reinterpret_cast<SipGeolocationRoutingHeader*>(pHeader));
    }
    return new SipGeolocationRoutingHeader();
}
