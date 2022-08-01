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

#define AOSTAG strTag.GetStr()

#define PONG_WAT_TIME 10000

/*

Remarks

*/
PUBLIC
AosKeepAlive::AosKeepAlive(IN IMS_SINT32 nSlotId_) :
        piKAHelper(IMS_NULL),
        piListener(IMS_NULL),
        piKeepAliveTimer(IMS_NULL),
        piPongWATTimer(IMS_NULL),
        nKATime(0),
        bIsPongChecked(IMS_TRUE),
        nSlotId(nSlotId_)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [SLOT%d] AosKeepAlive = %" PFLS_u "/%" PFLS_x, nSlotId,
            sizeof(AosKeepAlive), this);

    piKAHelper = SipFactory::CreateKeepAliveHelper(nSlotId);

    strTag.Sprintf("%d", nSlotId);
}

/*

Remarks

*/
PUBLIC VIRTUAL AosKeepAlive::~AosKeepAlive()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [SLOT%d] AosKeepAlive = %" PFLS_u "/%" PFLS_x, nSlotId,
            sizeof(AosKeepAlive), this);

    if (piKAHelper != IMS_NULL)
    {
        piKAHelper->Destroy();
    }
}

/*

Remarks

*/
PUBLIC
void AosKeepAlive::SetListener(IN IAosKeepAliveListener* piListener)
{
    this->piListener = piListener;
}

/*

Remarks

*/
PUBLIC
void AosKeepAlive::Start(IN IMS_UINT32 nRepeatTime, IN IMS_BOOL bCheckingPong /* = IMS_TRUE */)
{
    A_IMS_TRACE_I(AOSTAG, "Start :: keep alive time(%d)", nRepeatTime, 0, 0);
    piKAHelper->SetListener(this);

    SendPING();

    nKATime = nRepeatTime;
    StartTimer(TIMER_KEEP_ALIVE, nKATime);

    if (bCheckingPong && (nRepeatTime > PONG_WAT_TIME))
    {
        StartTimer(TIMER_PONG_WAT, PONG_WAT_TIME);
        SetCheckingPong(IMS_TRUE);
    }
    else
    {
        SetCheckingPong(IMS_FALSE);
    }
}

/*

Remarks

*/
PUBLIC
void AosKeepAlive::Stop()
{
    A_IMS_TRACE_I(AOSTAG, "Stop", 0, 0, 0);

    piKAHelper->SetListener(IMS_NULL);
    nKATime = 0;
    ClearTimer();
}
/*

Remarks

*/
PUBLIC
void AosKeepAlive::SetTransport(IN const IPAddress& objSourceIPA, IN IMS_SINT32 nSourcePort,
        IN const IPAddress& objDestIPA, IN IMS_SINT32 nDestPort,
        IN IMS_SINT32 nProtocol /* = AosKeepAlive::TRANSPORT_UDP */)
{
    piKAHelper->SetTransportTupleS(objSourceIPA, nSourcePort, nProtocol);
    piKAHelper->SetTransportTupleD(objDestIPA, nDestPort);
}

/*

Remarks

*/
PRIVATE
void AosKeepAlive::SendPING()
{
    A_IMS_TRACE_I(AOSTAG, "SendPING", 0, 0, 0);

    static const ByteArray objCRLF("\r\n\r\n");
    piKAHelper->SendPacket(objCRLF);
}

/*

Remarks

*/
PRIVATE
void AosKeepAlive::SetCheckingPong(IN IMS_BOOL bCheck)
{
    bIsPongChecked = bCheck;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosKeepAlive::IsPongChecked() const
{
    return bIsPongChecked;
}

/*

Remarks

*/
PRIVATE
void AosKeepAlive::ProcessKeepAliveTimerExpired()
{
    A_IMS_TRACE_D(AOSTAG, "ProcessKeepAliveTimerExpired", 0, 0, 0);

    // stop and start timer
    StartTimer(TIMER_KEEP_ALIVE, nKATime);

    if (IsPongChecked())
        StartTimer(TIMER_PONG_WAT, PONG_WAT_TIME);

    SendPING();
}

/*

Remarks

*/
PRIVATE
void AosKeepAlive::ProcessPongWaitTimerExpired()
{
    A_IMS_TRACE_I(AOSTAG, "ProcessPongWaitTimerExpired", 0, 0, 0);

    ClearTimer();

    if (piListener != IMS_NULL)
    {
        piListener->KeepAlive_DetectedFlowFailed();
    }
}

/*

Remarks

*/
PRIVATE
void AosKeepAlive::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
{
    if (nDuration == 0)
    {
        return;
    }

    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_KEEP_ALIVE:
            ppiTimer = &piKeepAliveTimer;
            break;

        case TIMER_PONG_WAT:
            ppiTimer = &piPongWATTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer != IMS_NULL)
    {
        StopTimer(nType);
    }

    *ppiTimer = AosUtil::GetInstance()->StartTimer(nDuration, this, TimerToString(nType));
}

/*

Remarks

*/
PRIVATE
void AosKeepAlive::StopTimer(IN IMS_UINT32 nType)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_KEEP_ALIVE:
            ppiTimer = &piKeepAliveTimer;
            break;

        case TIMER_PONG_WAT:
            ppiTimer = &piPongWATTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer == IMS_NULL)
    {
        return;
    }

    AosUtil::GetInstance()->StopTimer(*ppiTimer, TimerToString(nType));
}

/*

Remarks

*/
PRIVATE
void AosKeepAlive::ClearTimer()
{
    if (piKeepAliveTimer != IMS_NULL)
    {
        StopTimer(TIMER_KEEP_ALIVE);
    }

    if (piPongWATTimer != IMS_NULL)
    {
        StopTimer(TIMER_PONG_WAT);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void AosKeepAlive::KeepAliveHelper_PongReceived()
{
    A_IMS_TRACE_I(AOSTAG, "KeepAliveHelper_PongReceived", 0, 0, 0);

    StopTimer(TIMER_PONG_WAT);
}

/*

Remarks

*/
PRIVATE VIRTUAL void AosKeepAlive::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == piKeepAliveTimer)
    {
        ProcessKeepAliveTimerExpired();
        return;
    }

    if (piTimer == piPongWATTimer)
    {
        ProcessPongWaitTimerExpired();
        return;
    }
}

/*

Remarks

*/
PRIVATE GLOBAL const IMS_CHAR* AosKeepAlive::TimerToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case TIMER_KEEP_ALIVE:
            return "TIMER_KEEP_ALIVE";

        case TIMER_PONG_WAT:
            return "TIMER_PONG_WAT";

        default:
            return "__INVALID__";
    }
}
