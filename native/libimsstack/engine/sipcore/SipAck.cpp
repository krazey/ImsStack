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
#include "ServiceMemory.h"
#include "ServiceTimer.h"
#include "IMSStrLib.h"

#include "SipAck.h"
#include "SipStackHeaders.h"

PUBLIC
SipAck::SipAck(IN SipClientTransactionState* pCtState, IN IMS_SINT32 nAliveInterval) :
        m_pCtState(pCtState),
        m_piTimer(IMS_NULL)
{
    if (nAliveInterval > 0)
    {
        m_piTimer = TimerService::GetTimerService()->CreateTimer();

        if (m_piTimer != IMS_NULL)
        {
            m_piTimer->SetTimer(nAliveInterval, this);
        }
    }
}

PUBLIC VIRTUAL SipAck::~SipAck()
{
    if (m_piTimer != IMS_NULL)
    {
        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);
        m_piTimer = IMS_NULL;
    }

    m_pCtState = IMS_NULL;
}

PUBLIC
IMS_BOOL SipAck::IsSameTransaction(IN ::SipTxnKey* pTxnKey) const
{
    if (m_pCtState.IsNull())
    {
        return IMS_FALSE;
    }

    return SipStack::CompareTxnKeysForAck(m_pCtState->GetTxnKey(), pTxnKey);
}

PUBLIC
void SipAck::RetransmitMessage()
{
    if (m_pCtState.IsNull())
    {
        return;
    }

    (void)m_pCtState->RetransmitMessage();
}

PRIVATE VIRTUAL void SipAck::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (m_piTimer == IMS_NULL)
    {
        return;
    }

    if (m_piTimer != piTimer)
    {
        return;
    }

    m_piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    m_piTimer = IMS_NULL;

    m_pCtState = IMS_NULL;
}
