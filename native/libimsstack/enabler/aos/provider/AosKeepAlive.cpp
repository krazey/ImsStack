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

#include "ServiceTrace.h"
#include "ServiceTimer.h"

#include "ISipHeader.h"
#include "ISipKeepAliveHelper.h"

#include "SipFactory.h"

#include "provider/AosUtil.h"
#include "provider/AosKeepAlive.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define AOSTAG m_strTag.GetStr()

PUBLIC
AosKeepAlive::AosKeepAlive(IN IMS_SINT32 nSlotId) :
        m_piKeepAliveHelper(IMS_NULL),
        m_piListener(IMS_NULL),
        m_piKeepAliveTimer(IMS_NULL),
        m_piPongWaitTimer(IMS_NULL),
        m_nKeepAliveTime(0),
        m_bIsPongChecked(IMS_TRUE),
        m_nSlotId(nSlotId)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [SLOT%d] AosKeepAlive = %" PFLS_u "/%" PFLS_x, m_nSlotId,
            sizeof(AosKeepAlive), this);

    m_piKeepAliveHelper = SipFactory::CreateKeepAliveHelper(m_nSlotId);

    m_strTag.Sprintf("%d", m_nSlotId);
}

PUBLIC VIRTUAL AosKeepAlive::~AosKeepAlive()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [SLOT%d] AosKeepAlive = %" PFLS_u "/%" PFLS_x, m_nSlotId,
            sizeof(AosKeepAlive), this);

    m_piKeepAliveHelper->Destroy();
}

PUBLIC
void AosKeepAlive::SetListener(IN IAosKeepAliveListener* piListener)
{
    m_piListener = piListener;
}

PUBLIC
void AosKeepAlive::Start(IN IMS_UINT32 nRepeatTime, IN IMS_BOOL bCheckingPong /* = IMS_TRUE */)
{
    A_IMS_TRACE_I(AOSTAG, "Start :: keep alive time(%d)", nRepeatTime, 0, 0);

    m_piKeepAliveHelper->SetListener(this);

    SendPing();

    m_nKeepAliveTime = nRepeatTime;
    StartTimer(TIMER_KEEP_ALIVE, m_nKeepAliveTime);

    if (bCheckingPong && (nRepeatTime > PONG_WAIT_TIME_MILLIS))
    {
        StartTimer(TIMER_PONG_WAIT, PONG_WAIT_TIME_MILLIS);
        SetCheckingPong(IMS_TRUE);
    }
    else
    {
        SetCheckingPong(IMS_FALSE);
    }
}

PUBLIC
void AosKeepAlive::Stop()
{
    A_IMS_TRACE_I(AOSTAG, "Stop", 0, 0, 0);

    m_piKeepAliveHelper->SetListener(IMS_NULL);

    m_nKeepAliveTime = 0;
    ClearTimer();
}

PUBLIC
void AosKeepAlive::SetTransport(IN const IpAddress& objSourceIpAddress, IN IMS_SINT32 nSourcePort,
        IN const IpAddress& objDestIpAddress, IN IMS_SINT32 nDestPort,
        IN IMS_SINT32 nProtocol /* = AosKeepAlive::TRANSPORT_UDP */)
{
    m_piKeepAliveHelper->SetTransportTupleS(objSourceIpAddress, nSourcePort, nProtocol);
    m_piKeepAliveHelper->SetTransportTupleD(objDestIpAddress, nDestPort);
}

PROTECTED
void AosKeepAlive::SendPing()
{
    A_IMS_TRACE_I(AOSTAG, "SendPing", 0, 0, 0);

    static const ByteArray objCrlf("\r\n\r\n");
    m_piKeepAliveHelper->SendPacket(objCrlf);
}

PROTECTED
void AosKeepAlive::SetCheckingPong(IN IMS_BOOL bCheck)
{
    m_bIsPongChecked = bCheck;
}

PROTECTED
IMS_BOOL AosKeepAlive::IsPongChecked() const
{
    return m_bIsPongChecked;
}

PROTECTED
void AosKeepAlive::ProcessKeepAliveTimerExpired()
{
    A_IMS_TRACE_D(AOSTAG, "ProcessKeepAliveTimerExpired", 0, 0, 0);

    // stop and start timer
    StartTimer(TIMER_KEEP_ALIVE, m_nKeepAliveTime);

    if (IsPongChecked())
    {
        StartTimer(TIMER_PONG_WAIT, PONG_WAIT_TIME_MILLIS);
    }

    SendPing();
}

PROTECTED
void AosKeepAlive::ProcessPongWaitTimerExpired()
{
    A_IMS_TRACE_I(AOSTAG, "ProcessPongWaitTimerExpired", 0, 0, 0);

    ClearTimer();

    if (m_piListener != IMS_NULL)
    {
        m_piListener->KeepAlive_DetectedFlowFailed();
    }
}

PROTECTED
IMS_BOOL AosKeepAlive::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
{
    if (nDuration == 0)
    {
        return IMS_FALSE;
    }

    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_KEEP_ALIVE:
            ppiTimer = &m_piKeepAliveTimer;
            break;

        case TIMER_PONG_WAIT:
            ppiTimer = &m_piPongWaitTimer;
            break;

        default:
            return IMS_FALSE;
    }

    if (*ppiTimer != IMS_NULL)
    {
        StopTimer(nType);
    }

    *ppiTimer = AosUtil::GetInstance()->StartTimer(nDuration, this, TimerToString(nType));
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AosKeepAlive::StopTimer(IN IMS_UINT32 nType)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_KEEP_ALIVE:
            ppiTimer = &m_piKeepAliveTimer;
            break;

        case TIMER_PONG_WAIT:
            ppiTimer = &m_piPongWaitTimer;
            break;

        default:
            return IMS_FALSE;
    }

    if (*ppiTimer == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AosUtil::GetInstance()->StopTimer(*ppiTimer, TimerToString(nType));
    return IMS_TRUE;
}

PROTECTED
void AosKeepAlive::ClearTimer()
{
    if (m_piKeepAliveTimer != IMS_NULL)
    {
        StopTimer(TIMER_KEEP_ALIVE);
    }

    if (m_piPongWaitTimer != IMS_NULL)
    {
        StopTimer(TIMER_PONG_WAIT);
    }
}

PROTECTED VIRTUAL void AosKeepAlive::KeepAliveHelper_PongReceived()
{
    A_IMS_TRACE_I(AOSTAG, "KeepAliveHelper_PongReceived", 0, 0, 0);

    StopTimer(TIMER_PONG_WAIT);
}

PROTECTED VIRTUAL void AosKeepAlive::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == m_piKeepAliveTimer)
    {
        ProcessKeepAliveTimerExpired();
        return;
    }

    if (piTimer == m_piPongWaitTimer)
    {
        ProcessPongWaitTimerExpired();
        return;
    }
}

PROTECTED GLOBAL const IMS_CHAR* AosKeepAlive::TimerToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case TIMER_KEEP_ALIVE:
            return "TIMER_KEEP_ALIVE";

        case TIMER_PONG_WAIT:
            return "TIMER_PONG_WAIT";

        default:
            return "__INVALID__";
    }
}