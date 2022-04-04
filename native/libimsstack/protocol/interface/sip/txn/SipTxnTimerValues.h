/*
   Author
   <table>
   date      author                description
   --------  --------------        ----------
   20170110  vijay.nair@           Created
   </table>

   Description

 */

#ifndef _SIP_TXN_TIMER_VALUES_H
#define _SIP_TXN_TIMER_VALUES_H

#include "sip_pf_datatypes.h"

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

public:
    SipTxnTimerValues();
    virtual ~SipTxnTimerValues(){}

    SIP_VOID SetTimerValue(SIP_UINT32 nTimerType, SIP_UINT32 nDur);
    SIP_UINT32 GetTimerValue(SIP_UINT32 nTimerType) const;
    SIP_BOOL UpdateSipTimers(IN SIP_UINT32 nTimerOptions,
        const IN SipTxnTimerValues* ptrTxnSipTxnTimers);

private:
    SIP_BOOL IsTimerSet(IN SIP_UINT32 nTimerOptions, IN SIP_UINT32 nType);
    SIP_VOID PrintSIPTxnTimerValues() const;

    enum
    {
        TV_T1 = 0x0001,
        TV_T2 = 0x0002,
        TV_TIMER_B = 0x0004,
        TV_TIMER_D = 0x0008,
        TV_TIMER_F = 0x0010,
        TV_TIMER_H = 0x0020,
        TV_TIMER_I = 0x0040,
        TV_TIMER_J = 0x0080,
        TV_TIMER_K = 0x0100,
        TV_ALL = 0x01FF
    };
};

#endif
