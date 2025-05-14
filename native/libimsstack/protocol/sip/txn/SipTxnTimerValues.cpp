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
#include "SipConfiguration.h"
#include "SipDebug.h"
#include "platform/SipString.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnTimerValues.h"

SipTxnTimerValues::SipTxnTimerValues() :
        m_nT1Value(SIP_ZERO),
        m_nT2Value(SIP_ZERO),
        m_nT4Value(SIP_ZERO),
        m_nTimerA_Value(SIP_ZERO),
        m_nTimerB_Value(SIP_ZERO),
        m_nTimerD_Value(SIP_ZERO),
        m_nTimerE_Value(SIP_ZERO),
        m_nTimerF_Value(SIP_ZERO),
        m_nTimerG_Value(SIP_ZERO),
        m_nTimerH_Value(SIP_ZERO),
        m_nTimerI_Value(SIP_ZERO),
        m_nTimerJ_Value(SIP_ZERO),
        m_nTimerK_Value(SIP_ZERO),
        m_nTimerM_Value(SIP_ZERO)
{
    SipConfiguration* pConfig = SipConfiguration::GetInstance();

    m_nT1Value = pConfig->GetT1();
    m_nT2Value = pConfig->GetT2();
    m_nT4Value = pConfig->GetT4();
    m_nTimerA_Value = m_nT1Value;
    m_nTimerB_Value = pConfig->GetTimerB();
    m_nTimerD_Value = pConfig->GetTimerD();
    m_nTimerE_Value = m_nT1Value;
    m_nTimerF_Value = pConfig->GetTimerF();
    m_nTimerG_Value = m_nT1Value;
    m_nTimerH_Value = pConfig->GetTimerH();
    m_nTimerI_Value = pConfig->GetTimerI();
    m_nTimerJ_Value = pConfig->GetTimerJ();
    m_nTimerK_Value = pConfig->GetTimerK();
    m_nTimerM_Value = pConfig->GetTimerM();
}

SIP_VOID SipTxnTimerValues::SetTimerValue(SIP_UINT32 nTimerType, SIP_UINT32 nDur)
{
    switch (nTimerType)
    {
        case SipTxn::TIMER_T1:
        {
            m_nT1Value = nDur;
        }
        break;
        case SipTxn::TIMER_T2:
        {
            m_nT2Value = nDur;
        }
        break;
        case SipTxn::TIMER_T4:
        {
            m_nT4Value = nDur;
        }
        break;
        case SipTxn::TIMER_A:
        {
            m_nTimerA_Value = nDur;
        }
        break;
        case SipTxn::TIMER_B:
        {
            m_nTimerB_Value = nDur;
        }
        break;
        case SipTxn::TIMER_D:
        {
            m_nTimerD_Value = nDur;
        }
        break;
        case SipTxn::TIMER_E:
        {
            m_nTimerE_Value = nDur;
        }
        break;
        case SipTxn::TIMER_F:
        {
            m_nTimerF_Value = nDur;
        }
        break;
        case SipTxn::TIMER_G:
        {
            m_nTimerG_Value = nDur;
        }
        break;
        case SipTxn::TIMER_H:
        {
            m_nTimerH_Value = nDur;
        }
        break;
        case SipTxn::TIMER_I:
        {
            m_nTimerI_Value = nDur;
        }
        break;
        case SipTxn::TIMER_J:
        {
            m_nTimerJ_Value = nDur;
        }
        break;
        case SipTxn::TIMER_K:
        {
            m_nTimerK_Value = nDur;
        }
        break;
        case SipTxn::TIMER_M:
        {
            m_nTimerM_Value = nDur;
        }
        break;

        default:
            break;
    }
}

SIP_UINT32 SipTxnTimerValues::GetTimerValue(SIP_UINT32 nTimerType) const
{
    switch (nTimerType)
    {
        case SipTxn::TIMER_T1:
        {
            return m_nT1Value;
        }
        break;
        case SipTxn::TIMER_T2:
        {
            return m_nT2Value;
        }
        break;
        case SipTxn::TIMER_T4:
        {
            return m_nT4Value;
        }
        break;
        case SipTxn::TIMER_A:
        {
            return m_nTimerA_Value;
        }
        break;
        case SipTxn::TIMER_B:
        {
            return m_nTimerB_Value;
        }
        case SipTxn::TIMER_D:
        {
            return m_nTimerD_Value;
        }
        break;
        case SipTxn::TIMER_E:
        {
            return m_nTimerE_Value;
        }
        break;
        case SipTxn::TIMER_F:
        {
            return m_nTimerF_Value;
        }
        break;
        case SipTxn::TIMER_G:
        {
            return m_nTimerG_Value;
        }
        break;
        case SipTxn::TIMER_H:
        {
            return m_nTimerH_Value;
        }
        break;
        case SipTxn::TIMER_I:
        {
            return m_nTimerI_Value;
        }
        break;
        case SipTxn::TIMER_J:
        {
            return m_nTimerJ_Value;
        }
        break;
        case SipTxn::TIMER_K:
        {
            return m_nTimerK_Value;
        }
        break;
        case SipTxn::TIMER_M:
        {
            return m_nTimerM_Value;
        }
        break;

        default:
            break;
    }
    return SIP_ZERO;
}

SIP_BOOL SipTxnTimerValues::UpdateSipTimers(
        IN SIP_UINT32 nTimerOptions, const IN SipTxnTimerValues* ptrTxnSipTxnTimers)
{
    if (ptrTxnSipTxnTimers == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTIMER, "ptrTxnSipTxnTimers is NULL", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (IsTimerSet(nTimerOptions, TV_ALL) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTIMER, "nTimerOptions is NOT set", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_DEBUG_WARNING(
            ESIPTRACE_MODTIMER, "UpdateSipTimers nTimerOptions(%X)", nTimerOptions, SIP_ZERO);

    if (IsTimerSet(nTimerOptions, TV_T1) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER_T1, ptrTxnSipTxnTimers->m_nT1Value);
    }

    if (IsTimerSet(nTimerOptions, TV_T2) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER_T2, ptrTxnSipTxnTimers->m_nT2Value);
    }

    if (IsTimerSet(nTimerOptions, TV_T4) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER_T4, ptrTxnSipTxnTimers->m_nT4Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_A) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER_A, ptrTxnSipTxnTimers->m_nTimerA_Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_B) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER_B, ptrTxnSipTxnTimers->m_nTimerB_Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_D) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER_D, ptrTxnSipTxnTimers->m_nTimerD_Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_E) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER_E, ptrTxnSipTxnTimers->m_nTimerE_Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_F) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER_F, ptrTxnSipTxnTimers->m_nTimerF_Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_G) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER_G, ptrTxnSipTxnTimers->m_nTimerG_Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_H) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER_H, ptrTxnSipTxnTimers->m_nTimerH_Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_I) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER_I, ptrTxnSipTxnTimers->m_nTimerI_Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_J) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER_J, ptrTxnSipTxnTimers->m_nTimerJ_Value);
    }

    if (IsTimerSet(nTimerOptions, SipTxn::TIMER_K) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER_K, ptrTxnSipTxnTimers->m_nTimerK_Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_M) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER_M, ptrTxnSipTxnTimers->m_nTimerM_Value);
    }

    PrintSIPTxnTimerValues();
    return SIP_TRUE;
}

SIP_BOOL SipTxnTimerValues::IsTimerSet(IN SIP_UINT32 nTimerOptions, IN SIP_UINT32 nType)
{
    return (((nTimerOptions & nType) != 0) ? SIP_TRUE : SIP_FALSE);
}

SIP_VOID SipTxnTimerValues::PrintSIPTxnTimerValues() const
{
    SIP_CHAR szTimerLog[SIP_TRACE_MAX_SIZE] = {SIP_ZERO};
    SIP_CHAR const* pTimerVal =
            "(T1|T2|A|B|D|E|F|G|H|I|J|K|M)=%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d";

    SipPf_Sprintf(szTimerLog, pTimerVal, m_nT1Value, m_nT2Value, m_nTimerA_Value, m_nTimerB_Value,
            m_nTimerD_Value, m_nTimerE_Value, m_nTimerF_Value, m_nTimerG_Value, m_nTimerH_Value,
            m_nTimerI_Value, m_nTimerJ_Value, m_nTimerK_Value, m_nTimerM_Value);

    SIP_DEBUG_WARNING(ESIPTRACE_MODTIMER, "SIPTimer%s", szTimerLog, SIP_ZERO);
}
