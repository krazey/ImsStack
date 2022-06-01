/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090302  toastops@                 Created
    </table>

    Description
     This class defines a listener interface for notifications about failure of asynchronous send
    operations. The application implementing this interface has to register it with
    a ISipConnection instance using the ISIPTransaction::SetErrorListener method.
*/

#ifndef _INTERFACE_ON_SIP_ERROR_LISTENER_H_
#define _INTERFACE_ON_SIP_ERROR_LISTENER_H_

#include "SipConnection.h"

/*
SIP error listener interface

Example

See Also
SIPConnection

*/
class IOnSIPErrorListener
{
public:
    /*
     Called when an asynchronous send operation fails or any error occurrs.

    Remarks
     J180 -> void Error_NotifyError(IN CONST AString &strMessage);

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSC                     Pointer to SIPConnection object which error occurrs.
    nCode                   Reason code of error
    strMessage              Reason phrase of error; Implementation dependent non-localized
                            information about the error.
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void OnError_NotifyError(
            IN SIPConnection* pSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage) = 0;
};

#endif  // _INTERFACE_ON_SIP_ERROR_LISTENER_H_
