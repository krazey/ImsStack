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
#include "ServicePhoneInfo.h"
#include "IIpcan.h"
#include "provider/AosUtil.h"
#include "provider/AosTrm.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define AOSTAG strTag.GetStr()

/*

Remarks

*/
PUBLIC
AosTrm::AosTrm(IN IMS_SINT32 nSlotId_)
    : nSlotId(nSlotId_)
    , nServices(TYPE_NONE)
    , nIPCANCategory(IIPCAN::CATEGORY_MOBILE)
    , bIsStartUpdated(IMS_FALSE)
    , bIsEmergencyStartUpdated(IMS_FALSE)
    , piTRM(IMS_NULL)
    , piStopTimer(IMS_NULL)
    , objListeners(IMSList<IAosTrmListener*>())
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [SLOT%d] AosTrm = %" PFLS_u "/%" PFLS_x, nSlotId,
        sizeof(AosTrm), this);

    piTRM = PhoneInfoService::GetPhoneInfoService()->GetTRM();
    if (piTRM != IMS_NULL)
    {
        if (piTRM->IsTRMSupported())
        {
            piTRM->RegisterObserver(this);
            piTRM->Enable(nSlotId);
        }
        else
        {
            piTRM = IMS_NULL;
        }
    }

    strTag.Sprintf("%d", nSlotId);
}

/*

Remarks

*/
PUBLIC VIRTUAL
AosTrm::~AosTrm()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [SLOT%d] AosTrm = %" PFLS_u "/%" PFLS_x, nSlotId,
        sizeof(AosTrm), this);

    if (piTRM != IMS_NULL)
    {
        piTRM->RemoveObserver(this);
        piTRM->Disable(nSlotId);
        piTRM = IMS_NULL;
    }

    objListeners.Clear();

    StopTimer();
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosTrm::SetListener(IN IAosTrmListener *piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IAosTrmListener *piTmpListener = objListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            A_IMS_TRACE_D(AOSTAG, "SetListener :: (%" PFLS_x ") is already set", piListener, 0, 0);
            return;
        }
    }

    objListeners.Append(piListener);

    A_IMS_TRACE_D(AOSTAG, "SetListener :: (%" PFLS_x ") is set", piListener, 0, 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosTrm::RemoveListener(IN IAosTrmListener *piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IAosTrmListener *piTmpListener = objListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            objListeners.RemoveAt(i);

            A_IMS_TRACE_D(AOSTAG, "RemoveListener :: (%" PFLS_x ") is removed", piListener, 0, 0);
            return;
        }
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL AosTrm::IsReady()
{
    if (piTRM == IMS_NULL)
    {
        return IMS_TRUE;
    }

    if (bIsStartUpdated)
    {
        return IMS_TRUE;
    }

    return piTRM->IsServiceAvailable(nSlotId, ITRM::SERVICE_REG);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL AosTrm::IsTRMSupported()
{
    return (piTRM != IMS_NULL);
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosTrm::Set(IN IMS_UINT32 nType, IN IMS_BOOL bStart)
{
    A_IMS_TRACE_I(AOSTAG, "Set :: type (%d) , start (%d)", nType, bStart, 0);

    if (piTRM == IMS_NULL)
    {
        return;
    }

    if (bStart)
    {
        if (IsStarted(nType))
        {
            return;
        }

        Start(nType);

        if (bIsStartUpdated)
        {
            StopTimer();
        }
        else
        {
            bIsStartUpdated = IMS_TRUE;
            piTRM->SetService(nSlotId, ITRM::SERVICE_REG, ITRM::MODE_START);
        }
    }
    else
    {
        if (!IsStarted(nType))
        {
            return;
        }

        Stop(nType);
        if (bIsStartUpdated)
        {
            if (!IsStarted())
            {
                StartTimer(TIME_STOP_DELAY);
            }
        }
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosTrm::SetEmegency(IN IMS_UINT32 nType, IN IMS_BOOL bStart)
{
    (void) nType;

    if (piTRM == IMS_NULL)
    {
        return;
    }

    if (bStart)
    {
        if (!bIsEmergencyStartUpdated)
        {
            bIsEmergencyStartUpdated = IMS_TRUE;
            piTRM->SetEmergencyService(nSlotId, ITRM::SERVICE_REG, ITRM::MODE_START);
        }
    }
    else
    {
        if (bIsEmergencyStartUpdated)
        {
            bIsEmergencyStartUpdated = IMS_FALSE;
            piTRM->SetEmergencyService(nSlotId, ITRM::SERVICE_REG, ITRM::MODE_END);
        }
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosTrm::SetIPCAN(IN IN IMS_UINT32 nCategory)
{
    if (piTRM == IMS_NULL)
    {
        return;
    }

    if (nIPCANCategory == nCategory)
    {
        return;
    }

    nIPCANCategory = nCategory;
    piTRM->SetIPCAN(nSlotId, nIPCANCategory);
}

/*

Remarks

*/
PRIVATE
void AosTrm::Start(IN IMS_UINT32 nType)
{
    nServices |= nType;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosTrm::IsStarted()
{
    return (nServices != TYPE_NONE);
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosTrm::IsStarted(IN IMS_UINT32 nType)
{
    return (nServices & nType);
}

/*

Remarks

*/
PRIVATE
void AosTrm::Stop(IN IMS_UINT32 nType)
{
    nServices &= (~nType);
}

/*

Remarks

*/
PRIVATE
void AosTrm::StartTimer(IN IMS_UINT32 nDuration)
{
    if (piStopTimer != IMS_NULL)
    {
        StopTimer();
    }

    piStopTimer = AosUtil::GetInstance()->StartTimer(nDuration, this, "TRM_STOP_TIMER");
}

/*

Remarks

*/
PRIVATE
void AosTrm::StopTimer()
{
    if (piStopTimer == IMS_NULL)
    {
        return;
    }

    AosUtil::GetInstance()->StopTimer(piStopTimer, "TRM_STOP_TIMER");
}

/*

Remarks

*/
PRIVATE VIRTUAL
void AosTrm::ProcessTimerExpired()
{
    if (bIsStartUpdated)
    {
        if (!IsStarted())
        {
            A_IMS_TRACE_I(AOSTAG, "ProcessTimerExpired :: set end to trm", 0, 0, 0);
            bIsStartUpdated = IMS_FALSE;
            piTRM->SetService(nSlotId, ITRM::SERVICE_REG, ITRM::MODE_END);
        }
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void AosTrm::NotifyServicePriorityChanged()
{
    if (piTRM == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IAosTrmListener *piListener = objListeners.GetAt(i);

        if (piListener == IMS_NULL)
        {
            continue;
        }

        piListener->Trm_PriorityChanged();
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void AosTrm::Timer_TimerExpired(IN ITimer *piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer != piStopTimer)
    {
        return;
    }

    StopTimer();

    ProcessTimerExpired();
}
