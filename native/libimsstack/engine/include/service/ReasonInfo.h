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
#ifndef REASON_INFO_H_
#define REASON_INFO_H_

#include "IReasonInfo.h"
#include "SipStatusCode.h"

class ReasonInfo : public IReasonInfo
{
public:
    inline ReasonInfo() :
            m_nType(REASON_TYPE_NONE)
    {
    }
    inline explicit ReasonInfo(IN IMS_SINT32 nType) :
            m_nType(nType)
    {
    }
    inline ReasonInfo(IN IMS_SINT32 nType, IN const SipStatusCode& objStatusCode) :
            m_nType(nType),
            m_objStatusCode(objStatusCode)
    {
    }
    ~ReasonInfo() override = default;

    ReasonInfo(IN const ReasonInfo&) = delete;
    ReasonInfo& operator=(IN const ReasonInfo&) = delete;

public:
    inline const AString& GetReasonPhrase() const override
    {
        return (m_nType != REASON_TYPE_RESPONSE) ? AString::ConstNull()
                                                 : m_objStatusCode.GetReasonPhrase();
    }
    inline IMS_SINT32 GetReasonType() const override { return m_nType; }
    inline IMS_SINT32 GetStatusCode() const override
    {
        return (m_nType != REASON_TYPE_RESPONSE) ? SipStatusCode::SC_INVALID
                                                 : m_objStatusCode.ToInt();
    }

    inline void SetReasonType(IN IMS_SINT32 nType) { m_nType = nType; }
    inline void SetStatusCode(IN IMS_SINT32 nStatusCode)
    {
        m_objStatusCode = nStatusCode;
        m_objStatusCode = SipStatusCode::GetReasonPhrase(nStatusCode);
    }

private:
    IMS_SINT32 m_nType;
    SipStatusCode m_objStatusCode;
};

#endif
