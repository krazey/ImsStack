/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100614  hwangoo.park@             Created
    </table>

    Description
     This class defines a SIP transaction state listener interface.
*/

#ifndef _INTERFACE_SIP_TRANSACTION_STATE_LISTENER_H_
#define _INTERFACE_SIP_TRANSACTION_STATE_LISTENER_H_

class SIPClientTransactionState;

/*
SIP transaction state listener interface

Example

See Also

*/
class ISIPTransactionStateListener
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
    virtual void TransactionState_TimerExpired() = 0;
};

#endif  // _INTERFACE_SIP_TRANSACTION_STATE_LISTENER_H_
