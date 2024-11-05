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
#ifndef __SIP_CONTENT_TYPE_HEADER_H__
#define __SIP_CONTENT_TYPE_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipContentTypeHeader : public SipHeaderBase
{
private:
    /*Media type*/
    SIP_CHAR* m_pszMType;
    SIP_CHAR* m_pszMSubType;

public:
    explicit SipContentTypeHeader(SIP_INT32 eHdrType);
    SipContentTypeHeader(const SipContentTypeHeader& objHeader);

    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    SIP_BOOL Encode(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE) override;

    SIP_BOOL Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen) override;

    SIP_VOID SetMediaType(const SIP_CHAR* pszMType);
    SIP_VOID SetSubMediaType(const SIP_CHAR* pszMSubType);

    SIP_CHAR* GetBoundary();

    /*Get Display Name*/
    inline const SIP_CHAR* GetMediaType() const { return m_pszMType; }

    inline const SIP_CHAR* GetSubMediaType() const { return m_pszMSubType; }

    SIP_CHAR* StripDQUOTE(const SIP_CHAR* pszStr);
    SIP_BOOL IsValidHeader() const override;

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

private:
    ~SipContentTypeHeader();
};
#endif  //__SIP_CONTENT_TYPE_HEADER_H__
