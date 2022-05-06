/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100713  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_FORKED_DIALOG_METHOD_H_
#define _INTERFACE_FORKED_DIALOG_METHOD_H_

class IForkedDialogMethod
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
    virtual IMS_BOOL ForkedDialog_Compare(IN ISipDialog* piOrigDialog) const = 0;

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
    virtual IMS_BOOL ForkedDialog_NotifyRequest(IN ISipServerConnection* piSSC) = 0;
};

#endif  // _INTERFACE_FORKED_DIALOG_METHOD_H_
