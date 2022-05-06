/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100722  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_RETRY_TASK_HELPER_LISTENER_H_
#define _INTERFACE_RETRY_TASK_HELPER_LISTENER_H_

#include "IMSTypeDef.h"

class RetryCmd;
class RetryTaskHelper;

class IRetryTaskHelperListener
{
public:
    /*
     Notify to the user that the retry task is executed succefully or error occurred.
    When this method is invoked, the state of RetryTask is in STATE_INACTIVE.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pTaskHelper             Pointer to the RetryTaskHelper object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void RetryTaskHelper_OnCompleted(
            IN RetryTaskHelper* pTaskHelper, IN RetryCmd* pCmd, IN IMS_SINT32 nCode = 0) = 0;
};

#endif  // _INTERFACE_RETRY_TASK_HELPER_LISTENER_H_
