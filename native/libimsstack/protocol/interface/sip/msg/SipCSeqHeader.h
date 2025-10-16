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
#ifndef __SIP_CSEQ_HEADER_H__
#define __SIP_CSEQ_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipCSeqHeader : public SipHeaderBase
{
private:
    SIP_CHAR* m_pszMethod;
    SIP_UINT32 m_nSeq;

public:
    SipCSeqHeader();
    SipCSeqHeader(const SipCSeqHeader& objHeader);

    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    SIP_BOOL Encode(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE) override;

    SIP_BOOL Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen) override;

    SIP_VOID SetMethod(const SIP_CHAR* pszMethod);

    /*set Seq*/
    inline SIP_VOID SetSeq(SIP_UINT32 nSeq) { m_nSeq = nSeq; }
    /*Get methods*/
    inline const SIP_CHAR* GetMethod() const { return m_pszMethod; }

    inline SIP_UINT32 GetCSeq() const { return m_nSeq; }

    SIP_BOOL IsValidHeader() const override;

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

private:
    ~SipCSeqHeader() override;
};
#endif  //__SIP_CSEQ_HEADER_H__
