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
#ifndef REG_FLOW_H_
#define REG_FLOW_H_

#include "IPAddress.h"

#include "RegKey.h"

class RegFlow
{
public:
    explicit RegFlow(IN const RegKey& objRegKey);
    RegFlow(IN const RegFlow& other);
    ~RegFlow();

public:
    RegFlow& operator=(IN const RegFlow& other);

public:
    IMS_BOOL Capture(IN IMS_UINT32 nSubscriber = DEFAULT_SUBSCRIBER);
    inline const AString& GetCallId() const { return m_strCallId; }
    inline const RegKey& GetRegKey() const { return m_objRegKey; }
    // HEADER_REQ_SESSION-ID
    inline const AString& GetSessionId() const { return m_strSessionId; }
    IMS_SINT32 IncreaseNGetCSeqValue(IN IMS_SINT32 nIncrement = 1);
    IMS_BOOL IsReserved(OUT IMS_UINT32* pnSubscriber = IMS_NULL) const;
    inline void Release() { m_nSubscriber = NO_SUBSCRIBER; }
    void Restore();
    inline void SetCSeqValue(IN IMS_SINT32 nValue) { m_nCSeqValue = nValue; }
    void UpdateCallId(IN const IPAddress& objIpAddr);

public:
    enum
    {
        DEFAULT_SUBSCRIBER = 0,
        /// 1 ~ : Unique identifier to identify the current subscriber
        NO_SUBSCRIBER = 0x7FFFFFFF
    };

private:
    // Registration key : slot-id & flow-id
    RegKey m_objRegKey;

    // Call-ID; Key value of this object
    AString m_strCallId;
    // CSeq number
    IMS_SINT32 m_nCSeqValue;

    // Subscriber
    IMS_UINT32 m_nSubscriber;

    // HEADER_REQ_SESSION-ID
    AString m_strSessionId;
};

#endif
