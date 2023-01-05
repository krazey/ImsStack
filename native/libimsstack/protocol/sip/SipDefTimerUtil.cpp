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
#include "SipDefTimerUtil.h"
#include "ServiceTimer.h"

extern SIP_BOOL Sip_Cbk_StartTimer(IN SIP_UINT32 nDuration, IN SipTimerCallback pfnTimerCallback,
        IN SIP_VOID* pvData, IN SIP_VOID** ppvHandle);
extern SIP_BOOL Sip_Cbk_StopTimer(IN SIP_VOID* pvHandle, IN SIP_VOID** ppvData);

SipDefTimerUtil::SipDefTimerUtil() {}
SipDefTimerUtil::~SipDefTimerUtil() {}

SIP_BOOL SipDefTimerUtil::StartTimer(SIP_VOID** ppvTimerId, SIP_UINT32 nDuration,
        SIP_UINT16 nResetFlag, SipTimerCallback pfnTimerCallback, SIP_VOID* pvData)
{
    if (ppvTimerId == SIP_NULL)
    {
        return SIP_FALSE;
    }

    (void)nResetFlag;

    return Sip_Cbk_StartTimer(nDuration, pfnTimerCallback, pvData, ppvTimerId);
}
SIP_VOID* SipDefTimerUtil::StopTimer(SIP_VOID* pvTimerId)
{
    if (pvTimerId == SIP_NULL)
    {
        return SIP_NULL;
    }

    SIP_VOID* pvData = SIP_NULL;
    Sip_Cbk_StopTimer(pvTimerId, &pvData);

    return pvData;
}

SIP_VOID* SipDefTimerUtil::StopTimerEx(SIP_VOID* pvTimerId)
{
    return StopTimer(pvTimerId);
}
