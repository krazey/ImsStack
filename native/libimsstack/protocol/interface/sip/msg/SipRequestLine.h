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
#ifndef __SIP_REQUEST_LINE_H__
#define __SIP_REQUEST_LINE_H__

#include "SipRefBase.h"

class SipPercentEncoding;

class SipRequestLine : public SipRefBase
{
private:
    SIP_CHAR* m_pszMethod;
    SipAddrSpec* m_pReqUri;
    SIP_CHAR* m_pszSipVersion;

public:
    /*Constructor*/
    SipRequestLine();

    SipRequestLine(const SIP_CHAR* pszMethod, SipAddrSpec* pReqUri, const SIP_CHAR* pszSipVersion);
    SipRequestLine(const SipRequestLine& objHeader);

    /*Function for encoding*/
    SIP_BOOL EncodeRequestLine(SIP_CHAR** ppCurrPos);

    /*Function for decoding*/
    SIP_BOOL DecodeRequestLine(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    /*Set Methods*/
    SIP_BOOL SetMethod(const SIP_CHAR* pszMethod);

    SIP_BOOL SetSipVersion(const SIP_CHAR* pszVer);

    SIP_BOOL SetReqUri(SipAddrSpec* pAddrSpec);

    inline const SIP_CHAR* GetMethod() const { return m_pszMethod; }

    inline const SIP_CHAR* GetSipVersion() const { return m_pszSipVersion; }

    SipAddrSpec* GetReqUri();

private:
    ~SipRequestLine();
};

#endif  //__SIP_REQUEST_LINE_H__
