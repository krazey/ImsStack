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
#ifndef __SIP_UTIL_H__
#define __SIP_UTIL_H__

#include "ISipLoggerUtil.h"
#include "ISipNetworkUtil.h"
#include "ISipTransactionCallback.h"

class SipUtil
{
public:
    SipUtil();
    virtual ~SipUtil();
    SipUtil(IN const SipUtil&) = delete;
    SipUtil& operator=(IN const SipUtil&) = delete;

    SIP_VOID SetNetwork(ISipNetworkUtil* pNetworkUtil);
    inline SIP_VOID SetTransactionCallback(ISipTransactionCallback* pCallback)
    {
        m_pCallback = pCallback;
    }

    inline ISipLoggerUtil* GetLogger() const { return m_pLoggerUtil; }

    inline ISipNetworkUtil* GetNetwork() const { return m_pNetworkUtil; }

    inline ISipTransactionCallback* GetTransactionCallback() const { return m_pCallback; }

    static SipUtil* GetInstance();
    static SIP_VOID DestroyInstance();

private:
    ISipLoggerUtil* m_pLoggerUtil;
    ISipNetworkUtil* m_pNetworkUtil;
    ISipTransactionCallback* m_pCallback;

    static SipUtil* s_pUtil;
};

#endif  //__SIP_UTIL_H__
