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
#ifndef __SIP_STATUS_LINE_H__
#define __SIP_STATUS_LINE_H__

#include "SipRefBase.h"

class SipStatusLine : public SipRefBase
{
private:
    SIP_CHAR* m_pszSipVersion;
    SIP_CHAR* m_pszStatusCode;
    SIP_CHAR* m_pszRsnPhrase;

public:
    /*constructor*/
    SipStatusLine() :
            m_pszSipVersion(SIP_NULL),
            m_pszStatusCode(SIP_NULL),
            m_pszRsnPhrase(SIP_NULL)
    {
    }

    SipStatusLine(const SIP_CHAR* pszStatusCode, const SIP_CHAR* pszRsnPhrase);

    SipStatusLine(const SIP_CHAR* pszSipVersion, const SIP_CHAR* pszStatusCode,
            const SIP_CHAR* pszRsnPhrase);

    SipStatusLine(const SipStatusLine& objHeader);

    /*Function for encoding*/
    SIP_BOOL EncodeStatusLine(SIP_CHAR** ppCurrPos);

    /*Function for decoding*/
    SIP_BOOL DecodeStatusLine(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    SIP_VOID SetStatusCode(const SIP_CHAR* pszStatusCode);
    SIP_VOID SetSipVersion(const SIP_CHAR* pszVer);
    SIP_VOID SetRsnPhrase(const SIP_CHAR* pszRsnPhrase);

    /*Get methods*/
    inline const SIP_CHAR* GetStatusCode() const { return m_pszStatusCode; }

    SIP_BOOL GetStatusCode(SIP_INT16* pnStatusCode) const;

    SIP_UINT16 GetStatusCodeAsInt() const;

    inline const SIP_CHAR* GetSipVersion() const { return m_pszSipVersion; }

    inline const SIP_CHAR* GetRsnPhrase() const { return m_pszRsnPhrase; }

private:
    ~SipStatusLine();
};
#endif  //__SIP_STATUS_LINE_H__
