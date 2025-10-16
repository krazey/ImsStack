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
#ifndef __SIP_WARNING_HEADER_H__
#define __SIP_WARNING_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipWarningHeader : public SipHeaderBase
{
private:
    /*warn-code*/
    SIP_UINT32 m_nWarnCode;

    /*Protocol Version*/
    SIP_CHAR* m_pszWarnAgent;

    /*Transport*/
    SIP_CHAR* m_pszWarnText;

    static constexpr SIP_UINT16 MIN_WARNCODE = 100;
    static constexpr SIP_UINT16 MAX_WARNCODE = 999;

public:
    SipWarningHeader();
    SipWarningHeader(const SipWarningHeader& objHeader);

    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    SIP_BOOL Encode(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE) override;

    SIP_BOOL Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen) override;

    inline SIP_VOID SetWarnCode(SIP_UINT32 nWarnCode) { m_nWarnCode = nWarnCode; }
    inline SIP_UINT32 GetWarnCode() const { return m_nWarnCode; }

    SIP_VOID SetWarnAgent(const SIP_CHAR* pszWarnAgent);
    SIP_VOID SetWarnText(const SIP_CHAR* pszWarnText);

    inline SIP_CHAR* GetWarnAgent() const { return m_pszWarnAgent; }
    inline const SIP_CHAR* GetWarnText() const { return m_pszWarnText; }

    SIP_BOOL IsValidHeader() const override;

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

private:
    ~SipWarningHeader() override;
};
#endif  //__SIP_WARNING_HEADER_H__
