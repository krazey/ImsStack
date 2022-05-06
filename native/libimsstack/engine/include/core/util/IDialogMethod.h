/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100518  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_DIALOG_METHOD_H_
#define _INTERFACE_DIALOG_METHOD_H_

class IDialogMethod
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
    virtual IMS_BOOL Dialog_Compare(IN ISipServerConnection* piSSC) const = 0;

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
    virtual IMS_BOOL Dialog_NotifyRequest(IN ISipServerConnection* piSSC) = 0;
};

#endif  // _INTERFACE_DIALOG_METHOD_H_
