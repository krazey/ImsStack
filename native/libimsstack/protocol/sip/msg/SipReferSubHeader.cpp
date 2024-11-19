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
#include "msg/SipReferSubHeader.h"
#include "platform/SipString.h"

SipReferSubHeader::SipReferSubHeader() :
        SipHeaderBase(SipHeaderBase::REFER_SUB)
{
}

SipReferSubHeader::SipReferSubHeader(const SipReferSubHeader& objHeader) :
        SipHeaderBase(objHeader)
{
}

SipReferSubHeader::~SipReferSubHeader() {}

SIP_BOOL SipReferSubHeader::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (SipHeaderBase::Decode(pStartPt, nDecLen) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    const SIP_CHAR* pszValue = GetValue();

    if ((pszValue != SIP_NULL) && (SipPf_Stricmp(pszValue, "true") != SIP_ZERO) &&
            (SipPf_Stricmp(pszValue, "false") != SIP_ZERO))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid value, only true or false allowed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipReferSubHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipReferSubHeader(*reinterpret_cast<SipReferSubHeader*>(pHeader));
    }
    return new SipReferSubHeader();
}
