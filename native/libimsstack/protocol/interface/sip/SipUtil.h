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
#include "ISipTimerUtil.h"
#include "ISipTxnListener.h"

class SipUtil
{
public:
    SipUtil();
    virtual ~SipUtil();

    SIP_VOID RegisterNetwork(ISipNetworkUtil* pNwUtil);
    SIP_VOID RegisterTxnListener(ISipTxnListener* pTxnListener);
    ISipTimerUtil* GetTimer();
    ISipLoggerUtil* GetLogger();
    ISipNetworkUtil* GetNetwork();
    ISipTxnListener* GetTxnListener();

private:
    ISipTimerUtil* m_pTimerUtil;
    ISipLoggerUtil* m_pLoggerUtil;
    ISipNetworkUtil* m_pNetworkUtil;
    ISipTxnListener* m_pTxnListener;

    SipUtil& operator=(IN const SipUtil& objRHS);
    SipUtil(IN const SipUtil& objRHS);
};

void SipUtil_Construct();
void SipUtil_Destruct();
SipUtil* SipUtil_GetInstance();

#endif  //__SIP_UTIL_H__
