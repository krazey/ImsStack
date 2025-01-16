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

#include "IImsRadio.h"
#include "IMtsTrafficListener.h"
#include "MtsStringDef.h"
#include "MtsTraffic.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_COM_MTS__;

MtsTraffic::MtsTraffic(
        IN IMS_UINT32 nDirection, IN IMS_UINT32 nTrafficType, IN IMtsTrafficListener& objListener) :
        m_nDirection(nDirection),
        m_nTrafficType(nTrafficType),
        m_objMtsTrafficListener(objListener),
        m_piRadioGuardTimer(IMS_NULL)
{
}

PUBLIC VIRTUAL MtsTraffic::~MtsTraffic()
{
    if (m_piRadioGuardTimer != IMS_NULL)
    {
        m_piRadioGuardTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piRadioGuardTimer);
    }
}

PUBLIC
void MtsTraffic::ImsRadio_OnConnectionFailed(
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis)
{
    IMS_TRACE_I("ImsRadio_OnConnectionFailed : TrafficType[%s], Direction[%s]",
            PS_TrafficType(m_nTrafficType), PS_TrafficDirection(m_nDirection), 0);

    if (!IsReasonToIgnore(nFailureReason))
    {
        m_objMtsTrafficListener.Traffic_OnConnectionFailed(
                m_nTrafficType, m_nDirection, nFailureReason, nCauseCode, nWaitTimeMillis);
    }
}

PUBLIC
void MtsTraffic::ImsRadio_OnConnectionSetupPrepared()
{
    IMS_TRACE_I("ImsRadio_OnConnectionSetupPrepared : TrafficType[%s], Direction[%s]",
            PS_TrafficType(m_nTrafficType), PS_TrafficDirection(m_nDirection), 0);

    m_objMtsTrafficListener.Traffic_OnConnectionSetupPrepared(m_nTrafficType, m_nDirection);
}

PUBLIC
void MtsTraffic::Timer_TimerExpired(IN ITimer* piTimer)
{
    IMS_TRACE_I("Timer_TimerExpired : TrafficType[%s], Direction[%s]",
            PS_TrafficType(m_nTrafficType), PS_TrafficDirection(m_nDirection), 0);

    if (piTimer == IMS_NULL)
    {
        return;
    }
    else if (piTimer == m_piRadioGuardTimer)
    {
        m_objMtsTrafficListener.Traffic_GuardTimerExpired(m_nTrafficType, m_nDirection);
        piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(piTimer);
        m_piRadioGuardTimer = IMS_NULL;
    }
    else
    {
        IMS_TRACE_I("Timer_TimerExpired : can't find the expired timer", 0, 0, 0);
        return;
    }
}

PUBLIC
IMS_BOOL MtsTraffic::IsRadioGuardTimerActive()
{
    IMS_BOOL bResult = (m_piRadioGuardTimer != IMS_NULL) ? IMS_TRUE : IMS_FALSE;

    IMS_TRACE_I("IsRadioGuardTimerActive : TrafficType[%s], Direction[%s], Result[%s]",
            PS_TrafficType(m_nTrafficType), PS_TrafficDirection(m_nDirection), _TRACE_B_(bResult));

    return bResult;
}

PUBLIC
void MtsTraffic::StartRadioGuardTimer()
{
    IMS_TRACE_I("StartRadioGuardTimer : TrafficType[%s], Direction[%s]",
            PS_TrafficType(m_nTrafficType), PS_TrafficDirection(m_nDirection), 0);

    if (m_piRadioGuardTimer != IMS_NULL)
    {
        m_piRadioGuardTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piRadioGuardTimer);
    }

    m_piRadioGuardTimer = TimerService::GetTimerService()->CreateTimer();
    m_piRadioGuardTimer->SetTimer(MTS_RADIO_GUARD_TIME, this);
}

PRIVATE GLOBAL IMS_BOOL MtsTraffic::IsReasonToIgnore(IN IMS_UINT32 nFailureReason)
{
    switch (nFailureReason)
    {
        case IImsRadio::REASON_ACCESS_DENIED:
        case IImsRadio::REASON_RRC_REJECT:
        case IImsRadio::REASON_INTERNAL_ERROR:
            return IMS_FALSE;

        default:
            return IMS_TRUE;
    }
}
