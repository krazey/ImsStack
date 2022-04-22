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
#include "ServiceVoNR.h"
#include "INetWatcher.h"
#include "IVoNR.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosNetTracker.h"
#include "provider/AosUtil.h"
#include "provider/AosVonr.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define AOSTAG strTag.GetStr()

/*

Remarks

*/
PUBLIC
AosVonr::AosVonr(IN IAosAppContext *piContext_)
    : piContext(piContext_)
    , nSlotId(piContext_->GetSlotId())
    , nStartedModules(MODULE_NONE)
    , bIsStartUpdated(IMS_FALSE)
    , piVoNR(IMS_NULL)
    , piEndDelayTimer(IMS_NULL)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [SLOT%d] AosVonr = %" PFLS_u "/%" PFLS_x, nSlotId,
        sizeof(AosVonr), this);

    piVoNR = VoNRService::GetVoNRService()->GetVoNR(nSlotId);
    if (piVoNR != IMS_NULL)
    {
        if (!piVoNR->IsVoNRSupported())
        {
            piVoNR = IMS_NULL;
        }
    }

    strTag.Sprintf("%d", nSlotId);
}

/*

Remarks

*/
PUBLIC VIRTUAL
AosVonr::~AosVonr()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [SLOT%d] AosVonr = %" PFLS_u "/%" PFLS_x, nSlotId,
        sizeof(AosVonr), this);

    piVoNR = IMS_NULL;
    StopAllTimers();
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_BOOL AosVonr::IsSupported() const
{
    return (piVoNR != IMS_NULL);
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_BOOL AosVonr::Set(IN IMS_UINT32 nModule, IN IMS_BOOL bIsSending)
{
    A_IMS_TRACE_I(AOSTAG, "Set :: module (%d) , sending (%d)", nModule, bIsSending, 0);

    if (!IsSupported())
    {
        return IMS_FALSE;
    }

    if (bIsSending)
    {
        if (!IsAvailableCellular() || IsWLAN())
        {
            return IMS_TRUE;
        }

        if (IsStarted(nModule))
        {
            return IMS_TRUE;
        }

        Start(nModule);

        if (bIsStartUpdated)
        {
            StopTimer(TIMER_END_DELAY);
        }
        else
        {
            bIsStartUpdated = IMS_TRUE;
            SetImsSignaling(IMS_TRUE);
        }
    }
    else
    {
        if (!IsStarted(nModule))
        {
            return IMS_TRUE;
        }

        Stop(nModule);
        if (bIsStartUpdated)
        {
            if (!IsStarted())
            {
                StartTimer(TIMER_END_DELAY, TIMER_END_DELAY_INTERVAL);
            }
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosVonr::IsAvailableCellular()
{
    IMS_UINT32 nRAT = piContext->GetNetTracker()->GetNetworkType();

    if (AosUtil::GetInstance()->IsMtkChipset())
    {
        return (nRAT == NW_REPORT_RADIO_NR);
    }
    else
    {
        return (nRAT == NW_REPORT_RADIO_NR || nRAT == NW_REPORT_RADIO_LTE);
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosVonr::IsStarted()
{
    return (nStartedModules != MODULE_NONE);
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosVonr::IsStarted(IN IMS_UINT32 nModule)
{
    return (nStartedModules & nModule);
}


/*

Remarks

*/
PRIVATE
IMS_BOOL AosVonr::IsWLAN() const
{
    return (piContext->GetNetTracker()->GetNetworkType() == NW_REPORT_RADIO_WLAN);
}

/*

Remarks

*/
PRIVATE
void AosVonr::ProcessEndDelayTimerExpired()
{
    StopTimer(TIMER_END_DELAY);

    if (bIsStartUpdated)
    {
        if (!IsStarted())
        {
            A_IMS_TRACE_I(AOSTAG, "ProcessEndDelayTimerExpired :: set end for UAC", 0, 0, 0);
            bIsStartUpdated = IMS_FALSE;
            SetImsSignaling(IMS_FALSE);
        }
    }
}

/*

Remarks

*/
PRIVATE
void AosVonr::SetImsSignaling(IN IMS_BOOL bIsActive)
{
    if (AosUtil::GetInstance()->IsMtkChipset())
    {
        if (bIsActive)
        {
            piVoNR->SetImsSession(IVoNR::TYPE_REG_SIGNAL, IVoNR::MTK_CALL_START);
            piVoNR->SetUacCheck(IVoNR::TYPE_REG_SIGNAL, IVoNR::MTK_CALL_START);
        }
        else
        {
            piVoNR->SetUacCheck(IVoNR::TYPE_REG_SIGNAL, IVoNR::MTK_CALL_STOP);
            piVoNR->SetImsSession(IVoNR::TYPE_REG_SIGNAL, IVoNR::MTK_CALL_STOP);
        }
    }
    else
    {
        IMS_UINT32 nType = (bIsActive) ? IVoNR::SIGNALING_TYPE_ACTIVE : IVoNR::SIGNALING_TYPE_IDLE;
        piVoNR->SetImsSignaling(nType);
    }
}

/*

Remarks

*/
PRIVATE
void AosVonr::Start(IN IMS_UINT32 nModule)
{
    nStartedModules |= nModule;
}

/*

Remarks

*/
PRIVATE
void AosVonr::Stop(IN IMS_UINT32 nModule)
{
    nStartedModules &= (~nModule);
}

/*

Remarks

*/
PRIVATE
void AosVonr::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
{
    ITimer **ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_END_DELAY:
            ppiTimer = &piEndDelayTimer;
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
void AosVonr::StopTimer(IN IMS_UINT32 nType)
{
    ITimer **ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_END_DELAY:
            ppiTimer = &piEndDelayTimer;
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
void AosVonr::StopAllTimers()
{
    StopTimer(TIMER_END_DELAY);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void AosVonr::Timer_TimerExpired(IN ITimer *piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == piEndDelayTimer)
    {
        ProcessEndDelayTimerExpired();
        return;
    }
}

/*

Remarks

*/
PRIVATE GLOBAL
const IMS_CHAR* AosVonr::TimerToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case TIMER_END_DELAY:
            return "TIMER_END_DELAY";

        default:
            return "__INVALID__";
    }
}
