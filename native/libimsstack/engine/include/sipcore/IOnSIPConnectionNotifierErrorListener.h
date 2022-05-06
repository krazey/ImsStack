/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110528  hwangoo.park@             Created
    </table>

    Description
     This class defines a listener interface for notifications about failure of
    the transport layer operrations.
*/

#ifndef _INTERFACE_ON_SIP_CONNECTION_NOTIFIER_ERROR_LISTENER_H_
#define _INTERFACE_ON_SIP_CONNECTION_NOTIFIER_ERROR_LISTENER_H_

class ISipConnectionNotifier;

/*
SIP connection notifier error listener interface

Example

See Also
SIPConnectionNotifier
*/
class IOnSIPConnectionNotifierErrorListener
{
public:
    /*
     Called when any error occurrs in the SIPConnectionNotifier.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSCN                    Pointer to SIPConnectionNotifier object which error occurrs.
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
    virtual void OnConnectionNotifierError_NotifyError(
            IN SIPConnectionNotifier* pSCN, IN IMS_SINT32 nCode, IN CONST AString& strMessage) = 0;
};

#endif  // _INTERFACE_ON_SIP_CONNECTION_NOTIFIER_ERROR_LISTENER_H_
