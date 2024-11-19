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
#ifndef __SIP_AUTH_INFO_HEADER_H__
#define __SIP_AUTH_INFO_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipAuthInfoHeader : public SipHeaderBase
{
private:
    SipVector<SipNameValue*> m_pAuthInfoList;

public:
    SipAuthInfoHeader();
    SipAuthInfoHeader(const SipAuthInfoHeader& objHeader);

    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    SIP_BOOL Encode(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE) override;

    SIP_BOOL Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen) override;

    const SipNameValue* GetAiInfoVal(SIP_UINT32 nIndex = SIP_ZERO);

    inline SIP_BOOL IsValidHeader() const override
    {
        return (m_pAuthInfoList.IsEmpty() == SIP_TRUE) ? SIP_FALSE : SIP_TRUE;
    }

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

private:
    ~SipAuthInfoHeader();
};
#endif  //__SIP_AUTH_INFO_HEADER_H__
