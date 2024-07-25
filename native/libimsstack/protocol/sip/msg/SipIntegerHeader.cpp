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
#include "msg/SipIntegerHeader.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"

SipIntegerHeader::SipIntegerHeader(SIP_INT32 eHeaderType) :
        SipHeaderBase(eHeaderType)
{
}

SipIntegerHeader::SipIntegerHeader(const SipIntegerHeader& objHeader) :
        SipHeaderBase(objHeader)
{
}

SipIntegerHeader::~SipIntegerHeader() {}

SIP_BOOL SipIntegerHeader::SetValueInt(const SIP_UINT32 nValue)
{
    if (nValue > MAX_MAXFD)
    {
        if (GetHdrType() == SipHeaderBase::MAX_FORWARDS)
        {
            return SIP_FALSE;
        }
    }

    if (nValue > MAX_ERROR_CODE)
    {
        if (GetHdrType() == SipHeaderBase::GEOLOCATION_ERROR)
        {
            return SIP_FALSE;
        }
    }

    const SIP_UINT16 MAX_LEN = 11;
    SIP_CHAR szValue[MAX_LEN];
    SipPf_Sprintf(szValue, "%u", nValue);
    return SetValue(szValue);
}

SIP_UINT32 SipIntegerHeader::GetValueInt() const
{
    return SipPf_Atoi(GetValue());
}

SIP_BOOL SipIntegerHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    const SIP_CHAR* pszValue = GetValue();

    if ((pszValue == SIP_NULL) && (GetHdrType() == SipHeaderBase::CONTENT_LENGTH))
    {
        objBuffer += "0";
        return SIP_TRUE;
    }

    return SipHeaderBase::Encode(objBuffer, bParams);
}

SIP_BOOL SipIntegerHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams)
{
    const SIP_CHAR* pszValue = GetValue();

    if ((pszValue == SIP_NULL) && (GetHdrType() == SipHeaderBase::CONTENT_LENGTH))
    {
        SetValue("0");
    }
    return SipHeaderBase::EncodeHdr(ppCurrPos, bParams);
}

SIP_BOOL SipIntegerHeader::DecodeHdr(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (SipHeaderBase::DecodeHdr(pStartPt, nDecLen) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    const SIP_CHAR* pszValue = GetValue();

    if (pszValue != SIP_NULL)
    {
        SIP_UINT32 nValue = SipPf_Atoi(pszValue);
        SIP_INT32 eHeaderType = GetHdrType();

        if (((eHeaderType == SipHeaderBase::MAX_FORWARDS) && (nValue > MAX_MAXFD)) ||
                ((eHeaderType == SipHeaderBase::EXPIRES_SEC) && (nValue > MAX_EXPIRES)) ||
                ((eHeaderType == SipHeaderBase::GEOLOCATION_ERROR) && (nValue > MAX_ERROR_CODE)))
        {
            return SIP_FALSE;
        }
    }

    return SIP_TRUE;
}

SipHeaderBase* SipIntegerHeader::GetNewObj(SIP_INT32 eHdr, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipIntegerHeader(*reinterpret_cast<SipIntegerHeader*>(pHeader));
    }
    return new SipIntegerHeader(eHdr);
}
