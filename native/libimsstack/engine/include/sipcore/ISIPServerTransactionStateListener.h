/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _INTERFACE_SIP_SERVER_TRANSACTION_STATE_LISTENER_H_
#define _INTERFACE_SIP_SERVER_TRANSACTION_STATE_LISTENER_H_

#include "SIPServerTransactionState.h"

class SIPDialogState;

/*
SIP server transaction state listener interface

Example

See Also
SIPServerTransactionState
*/
class ISIPServerTransactionStateListener
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
    virtual void ServerTransactionState_ForkedRequestReceived(
            IN SIPServerTransactionState* pSTState, IN SIPDialogEx* pOrigDialogEx) = 0;

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
    virtual void ServerTransactionState_RequestCreated(IN SIPServerTransactionState* pSTState) = 0;

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
    virtual void ServerTransactionState_RequestReceived(IN SIPServerTransactionState* pSTState) = 0;
};

#endif  // _INTERFACE_SIP_SERVER_TRANSACTION_STATE_LISTENER_H_
