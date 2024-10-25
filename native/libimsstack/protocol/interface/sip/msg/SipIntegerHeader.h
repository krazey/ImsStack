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
#ifndef __SIP_INTEGER_HEADER_H__
#define __SIP_INTEGER_HEADER_H__

#include "msg/SipHeaderBase.h"

/**
 * @brief This class is for  Sip Headers which has header value as Integer.
 */

class SipIntegerHeader : public SipHeaderBase
{
public:
    explicit SipIntegerHeader(SIP_INT32 nHeaderType);
    SipIntegerHeader(const SipIntegerHeader& objHeader);

    SIP_BOOL SetValueInt(const SIP_UINT32 nContLen);
    SIP_UINT32 GetValueInt() const;
    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    SIP_BOOL EncodeHdr(SIP_CHAR** ppszCurrPos, SIP_BOOL bParams = SIP_TRUE) override;
    SIP_BOOL DecodeHdr(const SIP_CHAR* pszStartPt, SIP_UINT32 nDecLen) override;
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

private:
    virtual ~SipIntegerHeader();

    static constexpr SIP_INT32 MAX_GEOLOCATION_ERROR = 999;
    static constexpr SIP_UINT32 MAX_EXPIRES = 4294967295;
};
#endif  //__SIP_INTEGER_HEADER_H__
