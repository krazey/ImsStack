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
#ifndef __SIP_TIME_STAMP_HEADER_H__
#define __SIP_TIME_STAMP_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipTimeStampHeader : public SipHeaderBase
{
private:
    /*Time*/
    SIP_CHAR* m_pszTimeVal;

    /*Delay*/
    SIP_CHAR* m_pszDelay;

public:
    /*constructor*/
    SipTimeStampHeader();

    /*Copy Constructor*/
    SipTimeStampHeader(const SipTimeStampHeader& objHeader);

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    /*virtual methods*/
    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE) override;

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen) override;

    /*Sets */
    SIP_BOOL SetTimeVal(const SIP_CHAR* pszTimeVal);

    /*Gets */
    inline const SIP_CHAR* GetTimeVal() const { return m_pszTimeVal; }

    /*Sets */
    SIP_BOOL SetDelay(const SIP_CHAR* pszDelay);

    /*Gets */
    inline const SIP_CHAR* GetDelay() const { return m_pszDelay; }
    inline SIP_BOOL IsValidHeader() const override
    {
        return (m_pszTimeVal == SIP_NULL) ? SIP_FALSE : SIP_TRUE;
    }

private:
    ~SipTimeStampHeader();
};
#endif  //__SIP_TIME_STAMP_HEADER_H__
