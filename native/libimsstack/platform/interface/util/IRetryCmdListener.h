/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100722  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_RETRY_CMD_LISTENER_H_
#define _INTERFACE_RETRY_CMD_LISTENER_H_

#include "IMSTypeDef.h"

class RetryCmd;

class IRetryCmdListener
{
public:
    /*
     Notify to the user that the retry command is executed succefully.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pCmd                    Pointer to the RetryCmd object
    nResultCode             Result code; Error code, status code
    nRetryAfter             Retry after value when the command triggers the SIP request
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void RetryCmd_OnCompleted(
            IN RetryCmd* pCmd, IN IMS_SINT32 nResultCode, IN IMS_SINT32 nRetryAfter = 0) = 0;
};

#endif  // _INTERFACE_RETRY_CMD_LISTENER_H_
