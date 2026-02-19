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
#include "ServiceTrace.h"

#include "private/SipConfigV.h"

#include "SipConfigProxy.h"
#include "SipTimerValuesHelper.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL SipTimerValues SipTimerValuesHelper::GetValues(IN IMS_SINT32 nSlotId,
        IN const SipProfile* pProfile /*= IMS_NULL*/,
        IN IMS_SINT32 nTxnType /*= NON_INVITE_CLIENT*/)
{
    SipTimerValues objTv;
    const SipConfigV* pSipConfigV =
            DYNAMIC_CAST(const SipConfigV*, SipConfigProxy::GetSipConfigV(nSlotId));

    // If the timer values are not changed on runtime, do not set the timer values.
    if ((pProfile == IMS_NULL) && (pSipConfigV != IMS_NULL) &&
            !pSipConfigV->IsTimerValueConfiguredOnRuntime())
    {
        IMS_TRACE_D("SIP timer values not configured on runtime", 0, 0, 0);
        return objTv;
    }

    IMS_SINT32 nTvT1 = 2000;
    IMS_SINT32 nTvT2 = 16000;
    IMS_SINT32 nTvTemp;

    //// T1 (RTT estimate)
    nTvT1 = SipConfigProxy::GetTimerValueT1(nSlotId, pProfile, pSipConfigV);
    objTv.SetValue(SipTimerValues::TIMER_T1, nTvT1);

    //// Overwrite the T1 according to the SIP transaction
    if (nTxnType == INVITE_CLIENT)
    {
        nTvTemp = SipConfigProxy::GetTimerValueA(nSlotId, pProfile, pSipConfigV, IMS_FALSE);

        if (nTvTemp > 0)
        {
            nTvT1 = nTvTemp;
            objTv.SetValue(SipTimerValues::TIMER_T1, nTvT1);
        }
    }
    else if (nTxnType == INVITE_SERVER)
    {
        nTvTemp = SipConfigProxy::GetTimerValueG(nSlotId, pProfile, pSipConfigV, IMS_FALSE);

        if (nTvTemp > 0)
        {
            nTvT1 = nTvTemp;
            objTv.SetValue(SipTimerValues::TIMER_T1, nTvT1);
        }
    }
    else
    {
        nTvTemp = SipConfigProxy::GetTimerValueE(nSlotId, pProfile, pSipConfigV, IMS_FALSE);

        if (nTvTemp > 0)
        {
            nTvT1 = nTvTemp;
            objTv.SetValue(SipTimerValues::TIMER_T1, nTvT1);
        }
    }

    //// T2 (maximum retranmit interval for non-INVITE request & INVITE responses)
    nTvT2 = SipConfigProxy::GetTimerValueT2(nSlotId, pProfile, pSipConfigV);
    objTv.SetValue(SipTimerValues::TIMER_T2, nTvT2);

    //// Timer B (INVITE transaction timeout timer)
    nTvTemp = SipConfigProxy::GetTimerValueB(nSlotId, pProfile, pSipConfigV, IMS_FALSE);
    objTv.SetValue(SipTimerValues::TIMER_B, (nTvTemp > 0) ? nTvTemp : (nTvT1 * 64));

    //// Timer D (wait time for INVITE response retransmits)
    nTvTemp = SipConfigProxy::GetTimerValueD(nSlotId, pProfile, pSipConfigV, IMS_FALSE);
    objTv.SetValue(SipTimerValues::TIMER_D, (nTvTemp > 0) ? nTvTemp : (nTvT1 * 64));

    //// Timer F (non-INVITE transaction timeout timer)
    nTvTemp = SipConfigProxy::GetTimerValueF(nSlotId, pProfile, pSipConfigV, IMS_FALSE);
    objTv.SetValue(SipTimerValues::TIMER_F, (nTvTemp > 0) ? nTvTemp : (nTvT1 * 64));

    //// Timer H (wait time for ACK receipt)
    nTvTemp = SipConfigProxy::GetTimerValueH(nSlotId, pProfile, pSipConfigV, IMS_FALSE);
    objTv.SetValue(SipTimerValues::TIMER_H, (nTvTemp > 0) ? nTvTemp : (nTvT1 * 64));

    //// Timer I (wait for ACK retransmits)
    nTvTemp = SipConfigProxy::GetTimerValueI(nSlotId, pProfile, pSipConfigV, IMS_FALSE);

    if (nTvTemp > 0)
    {
        objTv.SetValue(SipTimerValues::TIMER_I, nTvTemp);
    }
    else
    {
        nTvTemp = SipConfigProxy::GetTimerValueT4(nSlotId, pProfile, pSipConfigV, IMS_FALSE);
        objTv.SetValue(SipTimerValues::TIMER_I, (nTvTemp > 0) ? nTvTemp : (nTvT2 + 1000));
    }

    //// Timer J (wait time for non-INVITE request retransmits)
    nTvTemp = SipConfigProxy::GetTimerValueJ(nSlotId, pProfile, pSipConfigV, IMS_FALSE);
    objTv.SetValue(SipTimerValues::TIMER_J, (nTvTemp > 0) ? nTvTemp : (nTvT1 * 64));

    //// Timer K (wait time for non-INVITE response retransmits)
    nTvTemp = SipConfigProxy::GetTimerValueK(nSlotId, pProfile, pSipConfigV, IMS_FALSE);

    if (nTvTemp > 0)
    {
        objTv.SetValue(SipTimerValues::TIMER_K, nTvTemp);
    }
    else
    {
        nTvTemp = SipConfigProxy::GetTimerValueT4(nSlotId, pProfile, pSipConfigV, IMS_FALSE);
        objTv.SetValue(SipTimerValues::TIMER_K, (nTvTemp > 0) ? nTvTemp : (nTvT2 + 1000));
    }

    return objTv;
}
