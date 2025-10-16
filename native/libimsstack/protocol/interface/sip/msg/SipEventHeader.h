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
#ifndef __SIP_EVENT_HEADER_H__
#define __SIP_EVENT_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipEventHeader : public SipHeaderBase
{
private:
    SipVector<SIP_CHAR*> m_objEventTemplates;

public:
    SipEventHeader(SIP_INT32 eHdrType);
    SipEventHeader(const SipEventHeader& objHeader);

    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams = SIP_TRUE) const override;
    SIP_BOOL Encode(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE) override;

    SIP_BOOL Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen) override;

    inline SIP_UINT32 GetTemplateCount() const { return m_objEventTemplates.GetSize(); }
    SIP_BOOL IsTemplatePresent(const SIP_CHAR* pTemplateName) const;
    SIP_INT32 GetTemplateIndex(const SIP_CHAR* pTemplateName) const;
    SIP_VOID AddTemplate(const SIP_CHAR* pTemplateName);
    SIP_VOID RemoveTemplate(const SIP_CHAR* pTemplateName);
    inline const SIP_CHAR* GetTemplate(SIP_UINT32 nPos) const
    {
        return (nPos < m_objEventTemplates.GetSize()) ? m_objEventTemplates.GetAt(nPos) : SIP_NULL;
    }

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

private:
    ~SipEventHeader() override;
};
#endif  //__SIP_EVENT_HEADER_H__
