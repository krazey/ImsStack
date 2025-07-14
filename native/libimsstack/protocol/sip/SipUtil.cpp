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
#include "SipDatatypes.h"
#include "SipDefLoggerUtil.h"
#include "SipDefNetworkUtil.h"
#include "SipDefTimerUtil.h"
#include "SipUtil.h"
#include "msg/SipMsgUtil.h"

SipUtil* SipUtil::s_pUtil = SIP_NULL;

SipUtil::SipUtil() :
        m_pTxnListener(SIP_NULL)
{
    /* Create Default In-Build Services */
    m_pLoggerUtil = new SipDefLoggerUtil();
    m_pTimerUtil = new SipDefTimerUtil();
    m_pNetworkUtil = new SipDefNetworkUtil();
    SipMsgUtil::Init();
}

SipUtil::~SipUtil()
{
    if (m_pTxnListener != SIP_NULL)
    {
        delete m_pTxnListener;
    }

    if (m_pNetworkUtil != SIP_NULL)
    {
        delete m_pNetworkUtil;
    }

    if (m_pTimerUtil != SIP_NULL)
    {
        delete m_pTimerUtil;
    }

    if (m_pLoggerUtil != SIP_NULL)
    {
        delete m_pLoggerUtil;
    }
}

SIP_VOID SipUtil::SetNetwork(ISipNetworkUtil* pNetworkUtil)
{
    if (m_pNetworkUtil != SIP_NULL)
    {
        delete m_pNetworkUtil;
    }

    m_pNetworkUtil = pNetworkUtil;
}

SIP_VOID SipUtil::SetTransactionListener(ISipTxnListener* pTxnListener)
{
    if (m_pTxnListener != SIP_NULL)
    {
        delete m_pTxnListener;
    }

    m_pTxnListener = pTxnListener;
}

SIP_VOID SipUtil::DestroyInstance()
{
    if (s_pUtil != SIP_NULL)
    {
        delete s_pUtil;
        s_pUtil = SIP_NULL;
    }
}

SipUtil* SipUtil::GetInstance()
{
    if (s_pUtil == SIP_NULL)
    {
        s_pUtil = new SipUtil();
    }

    return s_pUtil;
}
