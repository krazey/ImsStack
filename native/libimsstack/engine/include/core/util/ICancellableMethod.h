/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100518  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_CANCELLABLE_METHOD_H_
#define _INTERFACE_CANCELLABLE_METHOD_H_

class ICancellableMethod
{
public:
    /*

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
    virtual IMS_BOOL Cancellable_Compare(IN ISipServerConnection* piSSC_CANCEL) const = 0;

    /*

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
    virtual IMS_BOOL Cancellable_NotifyRequest(IN ISipServerConnection* piSSC_CANCEL) = 0;
};

#endif  // _INTERFACE_CANCELLABLE_METHOD_H_
