/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100722  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_RETRY_TIMER_LISTENER_H_
#define _INTERFACE_RETRY_TIMER_LISTENER_H_

#include "IMSTypeDef.h"

class RetryTimer;

class IRetryTimerListener
{
public:
    /*
     Notify to the user that the interim retry timer is expired.

    Remarks
     After this method returns, the retry timer will determine to start or not to start a timer
    according to the return value (RetryTimer.h).

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pTimer                  Pointer to the RetryTimer object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SINT32              RESULT_CONTINUE (RetryTimer.h)
    RESULT_PENDING
    RESULT_STOP
    </table>
    */
    virtual IMS_SINT32 RetryTimer_OnInterimExpired(IN RetryTimer* pTimer) = 0;

    /*
     Notify to the user that the final retry timer is expired.

    Remarks
     After this method returns, the retry timer goes to the initial state.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pTimer                  Pointer to the RetryTimer object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void RetryTimer_OnFinalExpired(IN RetryTimer* pTimer) = 0;
};

#endif  // _INTERFACE_RETRY_TIMER_LISTENER_H_
