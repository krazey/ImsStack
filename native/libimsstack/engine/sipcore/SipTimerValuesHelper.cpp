/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101120  hwangoo.park@             Created
    </table>

    Description
     This class defines the helper class to set the SIP transaction timer values.
*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "private/SipConfigV.h"
#include "SipConfigProxy.h"
#include "SipTimerValuesHelper.h"

__IMS_TRACE_TAG_IMS_CORE__;



PUBLIC GLOBAL
SIPTimerValues SIPTimerValuesHelper::GetValues(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pSIPProfile/* = IMS_NULL*/,
        IN IMS_SINT32 nTxnType/* = NON_INVITE_CLIENT*/)
{
    SIPTimerValues objTV;
    const SipConfigV *pSipConfigV = DYNAMIC_CAST(const SipConfigV*,
            SIPConfigProxy::GetSipConfigV(nSlotId));

    //---------------------------------------------------------------------------------------------

    // If the timer values are not changed on runtime, do not set the timer values.
    if ((pSIPProfile == IMS_NULL)
            && (pSipConfigV != IMS_NULL)
            && !pSipConfigV->IsTimerValueConfiguredOnRuntime())
    {
        IMS_TRACE_D("SIP timer values are not configured on runtime ...", 0, 0, 0);
        return objTV;
    }

    IMS_SINT32 nTV_T1 = 2000;
    IMS_SINT32 nTV_T2 = 16000;
    IMS_SINT32 nTV_Temp;

    //// T1 (RTT estimate)
    nTV_T1 = SIPConfigProxy::GetTimerValueT1(nSlotId, pSIPProfile, pSipConfigV);
    objTV.SetValue(SIPTimerValues::TIMER_T1, nTV_T1);

    //// Overwrite the T1 according to the SIP transaction
    if (nTxnType == INVITE_CLIENT)
    {
        nTV_Temp = SIPConfigProxy::GetTimerValueTA(nSlotId, pSIPProfile, pSipConfigV, IMS_FALSE);

        if (nTV_Temp > 0)
        {
            nTV_T1 = nTV_Temp;
            objTV.SetValue(SIPTimerValues::TIMER_T1, nTV_T1);
        }
    }
    else if (nTxnType == INVITE_SERVER)
    {
        nTV_Temp = SIPConfigProxy::GetTimerValueTG(nSlotId, pSIPProfile, pSipConfigV, IMS_FALSE);

        if (nTV_Temp > 0)
        {
            nTV_T1 = nTV_Temp;
            objTV.SetValue(SIPTimerValues::TIMER_T1, nTV_T1);
        }
    }
    else
    {
        nTV_Temp = SIPConfigProxy::GetTimerValueTE(nSlotId, pSIPProfile, pSipConfigV, IMS_FALSE);

        if (nTV_Temp > 0)
        {
            nTV_T1 = nTV_Temp;
            objTV.SetValue(SIPTimerValues::TIMER_T1, nTV_T1);
        }
    }

    //// T2 (maximum retranmit interval for non-INVITE request & INVITE responses)
    nTV_T2 = SIPConfigProxy::GetTimerValueT2(nSlotId, pSIPProfile, pSipConfigV);
    objTV.SetValue(SIPTimerValues::TIMER_T2, nTV_T2);

    //// Timer B (INVITE transaction timeout timer)
    nTV_Temp = SIPConfigProxy::GetTimerValueTB(nSlotId, pSIPProfile, pSipConfigV, IMS_FALSE);
    objTV.SetValue(SIPTimerValues::TV_TIMER_B, (nTV_Temp > 0) ? nTV_Temp : (nTV_T1 * 64));

    //// Timer D (wait time for INVITE response retransmits)
    nTV_Temp = SIPConfigProxy::GetTimerValueTD(nSlotId, pSIPProfile, pSipConfigV, IMS_FALSE);
    objTV.SetValue(SIPTimerValues::TV_TIMER_D, (nTV_Temp > 0) ? nTV_Temp : (nTV_T1 * 64));

    //// Timer F (non-INVITE transaction timeout timer)
    nTV_Temp = SIPConfigProxy::GetTimerValueTF(nSlotId, pSIPProfile, pSipConfigV, IMS_FALSE);
    objTV.SetValue(SIPTimerValues::TV_TIMER_F, (nTV_Temp > 0) ? nTV_Temp : (nTV_T1 * 64));

    //// Timer H (wait time for ACK receipt)
    nTV_Temp = SIPConfigProxy::GetTimerValueTH(nSlotId, pSIPProfile, pSipConfigV, IMS_FALSE);
    objTV.SetValue(SIPTimerValues::TV_TIMER_H, (nTV_Temp > 0) ? nTV_Temp : (nTV_T1 * 64));

    //// Timer I (wait for ACK retransmits)
    nTV_Temp = SIPConfigProxy::GetTimerValueTI(nSlotId, pSIPProfile, pSipConfigV, IMS_FALSE);

    if (nTV_Temp > 0)
    {
        objTV.SetValue(SIPTimerValues::TV_TIMER_I, nTV_Temp);
    }
    else
    {
        nTV_Temp = SIPConfigProxy::GetTimerValueT4(nSlotId, pSIPProfile, pSipConfigV, IMS_FALSE);
        objTV.SetValue(SIPTimerValues::TV_TIMER_I, (nTV_Temp > 0) ? nTV_Temp : (nTV_T2 + 1000));
    }

    //// Timer J (wait time for non-INVITE request retransmits)
    nTV_Temp = SIPConfigProxy::GetTimerValueTJ(nSlotId, pSIPProfile, pSipConfigV, IMS_FALSE);
    objTV.SetValue(SIPTimerValues::TV_TIMER_J, (nTV_Temp > 0) ? nTV_Temp : (nTV_T1 * 64));

    //// Timer K (wait time for non-INVITE response retransmits)
    nTV_Temp = SIPConfigProxy::GetTimerValueTK(nSlotId, pSIPProfile, pSipConfigV, IMS_FALSE);

    if (nTV_Temp > 0)
    {
        objTV.SetValue(SIPTimerValues::TV_TIMER_K, nTV_Temp);
    }
    else
    {
        nTV_Temp = SIPConfigProxy::GetTimerValueT4(nSlotId, pSIPProfile, pSipConfigV, IMS_FALSE);
        objTV.SetValue(SIPTimerValues::TV_TIMER_K, (nTV_Temp > 0) ? nTV_Temp : (nTV_T2 + 1000));
    }

    return objTV;
}
