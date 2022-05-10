#include "sip_error.h"
#include "SipTrace.h"
#include "sip_debug.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "txn/SipTxn.h"
#include "txn/sip_txn_common.h"
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
        m_nTimerK_Value(SIP_ZERO)
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
}

SIP_VOID SipTxnTimerValues::SetTimerValue(SIP_UINT32 nTimerType, SIP_UINT32 nDur)
{
    switch (nTimerType)
    {
        case SipTxn::TIMER1:
        {
            m_nT1Value = nDur;
        }
        break;
        case SipTxn::TIMER2:
        {
            m_nT2Value = nDur;
        }
        break;
        case SipTxn::TIMER4:
        {
            m_nT4Value = nDur;
        }
        break;
        case SipTxn::TIMERA:
        {
            m_nTimerA_Value = nDur;
        }
        break;
        case SipTxn::TIMERB:
        {
            m_nTimerB_Value = nDur;
        }
        break;
        case SipTxn::TIMERD:
        {
            m_nTimerD_Value = nDur;
        }
        break;
        case SipTxn::TIMERE:
        {
            m_nTimerE_Value = nDur;
        }
        break;
        case SipTxn::TIMERF:
        {
            m_nTimerF_Value = nDur;
        }
        break;
        case SipTxn::TIMERG:
        {
            m_nTimerG_Value = nDur;
        }
        break;
        case SipTxn::TIMERH:
        {
            m_nTimerH_Value = nDur;
        }
        break;
        case SipTxn::TIMERI:
        {
            m_nTimerI_Value = nDur;
        }
        break;
        case SipTxn::TIMERJ:
        {
            m_nTimerJ_Value = nDur;
        }
        break;
        case SipTxn::TIMERK:
        {
            m_nTimerK_Value = nDur;
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
        case SipTxn::TIMER1:
        {
            return m_nT1Value;
        }
        break;
        case SipTxn::TIMER2:
        {
            return m_nT2Value;
        }
        break;
        case SipTxn::TIMER4:
        {
            return m_nT4Value;
        }
        break;
        case SipTxn::TIMERA:
        {
            return m_nTimerA_Value;
        }
        break;
        case SipTxn::TIMERB:
        {
            return m_nTimerB_Value;
        }
        case SipTxn::TIMERD:
        {
            return m_nTimerD_Value;
        }
        break;
        case SipTxn::TIMERE:
        {
            return m_nTimerE_Value;
        }
        break;
        case SipTxn::TIMERF:
        {
            return m_nTimerF_Value;
        }
        break;
        case SipTxn::TIMERG:
        {
            return m_nTimerG_Value;
        }
        break;
        case SipTxn::TIMERH:
        {
            return m_nTimerH_Value;
        }
        break;
        case SipTxn::TIMERI:
        {
            return m_nTimerI_Value;
        }
        break;
        case SipTxn::TIMERJ:
        {
            return m_nTimerJ_Value;
        }
        break;
        case SipTxn::TIMERK:
        {
            return m_nTimerK_Value;
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
        SetTimerValue(SipTxn::TIMER1, ptrTxnSipTxnTimers->m_nT1Value);
    }

    if (IsTimerSet(nTimerOptions, TV_T2) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMER2, ptrTxnSipTxnTimers->m_nT2Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_B) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMERB, ptrTxnSipTxnTimers->m_nTimerB_Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_D) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMERD, ptrTxnSipTxnTimers->m_nTimerD_Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_F) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMERF, ptrTxnSipTxnTimers->m_nTimerF_Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_H) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMERH, ptrTxnSipTxnTimers->m_nTimerH_Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_I) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMERI, ptrTxnSipTxnTimers->m_nTimerI_Value);
    }

    if (IsTimerSet(nTimerOptions, TV_TIMER_J) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMERJ, ptrTxnSipTxnTimers->m_nTimerJ_Value);
    }

    if (IsTimerSet(nTimerOptions, SipTxn::TIMERK) == SIP_TRUE)
    {
        SetTimerValue(SipTxn::TIMERK, ptrTxnSipTxnTimers->m_nTimerK_Value);
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
            "(T1|T2|T4|A|B|D|E|F|G|H|I|J|K)=%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d";

    SipPf_Sprintf(szTimerLog, (SIP_CHAR*)pTimerVal, m_nT1Value, m_nT2Value, m_nT4Value,
            m_nTimerA_Value, m_nTimerB_Value, m_nTimerD_Value, m_nTimerE_Value, m_nTimerF_Value,
            m_nTimerG_Value, m_nTimerH_Value, m_nTimerI_Value, m_nTimerJ_Value, m_nTimerK_Value);

    SIP_DEBUG_WARNING(ESIPTRACE_MODTIMER, "SIPTimer%s", szTimerLog, SIP_ZERO);
}
