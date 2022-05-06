/*
    Author
    <table>
    date      author                        description
    --------  --------------                ----------
    20111015  hwangoo.park@                 Created
    </table>

    Description
*/

#include "ServiceMemory.h"
#include "ServiceEvent.h"
#include "SIPWakeLock.h"

/*

Remarks

*/
PUBLIC GLOBAL void SIPWakeLock::Acquire(
        IN CONST SipMethod& objMethod, IN IMS_UINT32 /* nTimeout = 0 */)
{
    //---------------------------------------------------------------------------------------------

    if (objMethod.Equals(SipMethod::INVITE))
    {
        // 3 seconds
        IMS_EVENT_SendEventForSlotId(IMS_EVENT_WAKE_LOCK, 0, 3000, IMS_SLOT_0);
    }
    else if (objMethod.Equals(SipMethod::UPDATE))
    {
        // 2 seconds
        IMS_EVENT_SendEventForSlotId(IMS_EVENT_WAKE_LOCK, 0, 2000, IMS_SLOT_0);
    }
    else if (objMethod.Equals(SipMethod::BYE))
    {
        // 2 seconds
        IMS_EVENT_SendEventForSlotId(IMS_EVENT_WAKE_LOCK, 0, 2000, IMS_SLOT_0);
    }
    else if (objMethod.Equals(SipMethod::ACK) || objMethod.Equals(SipMethod::PRACK) ||
            objMethod.Equals(SipMethod::OPTIONS) || objMethod.Equals(SipMethod::MESSAGE) ||
            objMethod.Equals(SipMethod::REFER) || objMethod.Equals(SipMethod::NOTIFY) ||
            objMethod.Equals(SipMethod::INFO))
    {
        // 1 seconds
        IMS_EVENT_SendEventForSlotId(IMS_EVENT_WAKE_LOCK, 0, 1000, IMS_SLOT_0);
    }
}

/*

Remarks

*/
PUBLIC GLOBAL IMS_BOOL SIPWakeLock::IsSupported()
{
    //---------------------------------------------------------------------------------------------

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC GLOBAL void SIPWakeLock::Release()
{
    //---------------------------------------------------------------------------------------------
}
