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
#ifndef __SIP_HEADER_LIST_H__
#define __SIP_HEADER_LIST_H__

#include "SipDatatypes.h"
#include "msg/SipHeaderBase.h"

class SipHeaderList : public SipHeaderBase
{
private:
    SipVector<SipHeaderBase*> m_headerList;

public:
    explicit SipHeaderList(SIP_INT32 eHdrType);
    SipHeaderList(const SipHeaderList& objHeaderList);
    static SipHeaderBase* GetNewListObj(SIP_INT32 eHdr, SipHeaderBase* pHeader);
    inline SIP_BOOL Encode(AStringBuffer& /*objBuffer*/, SIP_BOOL /*bParams*/) const override
    {
        return SIP_TRUE;
    }
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE) override;

    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams, SIP_UINT32 nMsgOptions) override;

    SipHeaderBase* GetObj(SIP_UINT32 nIndex);
    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen) override;

    SipHeaderBase* GetListObj(SipHeaderBase* pHeader = SIP_NULL);
    SIP_BOOL AddHeader(SipHeaderBase* pHeader);
    SIP_BOOL InsertHdrAtPos(SipHeaderBase* pHeader, SIP_UINT32 nIndex);
    void RemoveHdr(SIP_UINT32 nIndex);

    inline SIP_UINT32 GetSize() const { return m_headerList.GetSize(); }

private:
    ~SipHeaderList();
};

#endif  //__SIP_HEADER_LIST_H__
