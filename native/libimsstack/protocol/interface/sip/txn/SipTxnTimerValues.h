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
#ifndef __SIP_TXN_TIMER_VALUES_H__
#define __SIP_TXN_TIMER_VALUES_H__

#include "SipDatatypes.h"

class SipTxnTimerValues
{
    /* T1: RTT Estimate */
    SIP_UINT32 m_nT1Value;

    /* The maximum retransmit interval for non-INVITE Req and INVITE resp*/
    SIP_UINT32 m_nT2Value;

    /* Maximum duration a message will remain in the network */
    SIP_UINT32 m_nT4Value;

    SIP_UINT32 m_nTimerA_Value;
    SIP_UINT32 m_nTimerB_Value;
    SIP_UINT32 m_nTimerD_Value;
    SIP_UINT32 m_nTimerE_Value;
    SIP_UINT32 m_nTimerF_Value;
    SIP_UINT32 m_nTimerG_Value;
    SIP_UINT32 m_nTimerH_Value;
    SIP_UINT32 m_nTimerI_Value;
    SIP_UINT32 m_nTimerJ_Value;
    SIP_UINT32 m_nTimerK_Value;
    SIP_UINT32 m_nTimerM_Value;

public:
    SipTxnTimerValues();
    virtual ~SipTxnTimerValues() {}

    SIP_VOID SetTimerValue(SIP_UINT32 nTimerType, SIP_UINT32 nDur);
    SIP_UINT32 GetTimerValue(SIP_UINT32 nTimerType) const;
    SIP_BOOL UpdateSipTimers(
            IN SIP_UINT32 nTimerOptions, const IN SipTxnTimerValues* ptrTxnSipTxnTimers);

private:
    SIP_BOOL IsTimerSet(IN SIP_UINT32 nTimerOptions, IN SIP_UINT32 nType);
    SIP_VOID PrintSIPTxnTimerValues() const;

public:
    enum
    {
        TV_T1 = 0x0001,
        TV_T2 = 0x0002,
        TV_T4 = 0x0004,
        TV_TIMER_A = 0x0008,
        TV_TIMER_B = 0x0010,
        TV_TIMER_D = 0x0020,
        TV_TIMER_E = 0x0040,
        TV_TIMER_F = 0x0080,
        TV_TIMER_G = 0x0100,
        TV_TIMER_H = 0x0200,
        TV_TIMER_I = 0x0400,
        TV_TIMER_J = 0x0800,
        TV_TIMER_K = 0x1000,
        TV_TIMER_M = 0x2000,
        TV_ALL = 0x3FFF
    };
};

#endif  //__SIP_TXN_TIMER_VALUES_H__
