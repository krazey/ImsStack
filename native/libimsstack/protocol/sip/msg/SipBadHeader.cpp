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
#include "msg/SipBadHeader.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"

SipBadHeader::SipBadHeader() :
        SipHeaderBase(TYPE_INVALID),
        m_pszHdrName(SIP_NULL)
{
}

SipBadHeader::SipBadHeader(const SipBadHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pszHdrName(SipPf_Strdup(objHeader.m_pszHdrName))
{
}

SipBadHeader::~SipBadHeader()
{
    if (m_pszHdrName != SIP_NULL)
    {
        delete[] m_pszHdrName;
    }
}

SIP_VOID SipBadHeader::SetHeaderName(const SIP_CHAR* pszHdrName)
{
    SipMsgUtil::SetValue(pszHdrName, m_pszHdrName);
}

SIP_BOOL SipBadHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    (void)ppCurrPos;
    (void)bParams;
    return SIP_TRUE;
}

SIP_BOOL SipBadHeader::DecodeHdr(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    (void)pStartPt;
    (void)nDecLen;

    return SIP_TRUE;
}
