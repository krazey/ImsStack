/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090305  lovil@                    Created
    20090831  yhrhee@                   Modified
    </table>

    Description
    It's a timer interface provided by Platform interface layer.
    It can be used after allocating a timer interface from timer service.
*/

#ifndef _INTERFACE_IMS_TIMER_H_
#define _INTERFACE_IMS_TIMER_H_

#include "IMSTypeDef.h"

class ITimerListener;

class ITimer
{
public:
    /*
     Check if the specified timer is equal to the current object.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    piTimer                 Pointer to ITimer to be checked
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_BOOL
    </table>
    */
    virtual IMS_BOOL Equals(IN const ITimer *piTimer) const = 0;

    /*
     Starts the timer.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nDuration               Timer duration (milli-seconds)
    piListener              Listener interface pointer, supplier should implement its listener
                            to receive notification of the timer expiration.
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_UINTP               Timer id to identify this object
    </table>
    */
    virtual IMS_UINTP SetTimer(IN IMS_UINT32 nDuration, IN ITimerListener *piListener) = 0;

    /*
     Kills the running timer if it is not expired.

    Remarks
     The application MUST destroy the pointer of ITimer
    with TimerService::GetTimerService()->Destroy() after calling this method.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void KillTimer() = 0;
};



class ITimerListener
{
public:
    /*
    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    piTimer                 Timer object to be expired
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void Timer_TimerExpired(IN ITimer *piTimer) = 0;
};

#endif // _INTERFACE_IMS_TIMER_H_
