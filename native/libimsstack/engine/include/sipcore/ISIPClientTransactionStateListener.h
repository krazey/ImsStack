/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090302  toastops@                 Created
    </table>

    Description
     This class defines a SIP transport listener interface.
*/

#ifndef _INTERFACE_SIP_CLIENT_TRANSACTION_STATE_LISTENER_H_
#define _INTERFACE_SIP_CLIENT_TRANSACTION_STATE_LISTENER_H_

class SIPClientTransactionState;

/*
SIP client transaction state listener interface

Example

See Also

*/
class ISIPClientTransactionStateListener
{
public:
    /*

    Remarks

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
    virtual void ClientTransactionState_ForkedResponseReceived(
            IN SIPClientTransactionState* pCTState) = 0;

    /*

    Remarks

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
    virtual void ClientTransactionState_ResponseReceived(IN SipMessage* pstMessage) = 0;
};

#endif  // _INTERFACE_SIP_CLIENT_TRANSACTION_STATE_LISTENER_H_
