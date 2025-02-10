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

static SipUtil* gpUtil = SIP_NULL;

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

SIP_VOID SipUtil::RegisterNetwork(ISipNetworkUtil* pNwUtil)
{
    if (m_pNetworkUtil != SIP_NULL)
    {
        delete m_pNetworkUtil;
    }

    m_pNetworkUtil = pNwUtil;
}

SIP_VOID SipUtil::RegisterTxnListener(ISipTxnListener* pTxnListener)
{
    if (m_pTxnListener != SIP_NULL)
    {
        delete m_pTxnListener;
    }

    m_pTxnListener = pTxnListener;
}

ISipTimerUtil* SipUtil::GetTimer()
{
    return m_pTimerUtil;
}

ISipLoggerUtil* SipUtil::GetLogger()
{
    return m_pLoggerUtil;
}
ISipNetworkUtil* SipUtil::GetNetwork()
{
    return m_pNetworkUtil;
}

ISipTxnListener* SipUtil::GetTxnListener()
{
    return m_pTxnListener;
}

SIP_VOID SipUtil_Construct()
{
    SipUtil* pUtil = gpUtil;

    if (pUtil)
    {
        return;
    }

    pUtil = new SipUtil();
    gpUtil = pUtil;
}

SIP_VOID SipUtil_Destruct()
{
    SipUtil* pUtil = gpUtil;

    if (pUtil == SIP_NULL)
    {
        return;
    }

    delete pUtil;
    gpUtil = SIP_NULL;
}

SipUtil* SipUtil_GetInstance()
{
    SipUtil* pUtil = gpUtil;
    return pUtil;
}
