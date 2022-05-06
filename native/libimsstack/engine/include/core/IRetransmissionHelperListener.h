/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090817  toastops@                 Created
    </table>

    Description

*/

#ifndef _INTERFACE_RETRANSMISSION_HELPER_LISTENER_H_
#define _INTERFACE_RETRANSMISSION_HELPER_LISTENER_H_

/*

RetransmissionHelperListener interface

Example

See Also

*/
class IRetransmissionHelperListener
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
    virtual IMS_RESULT RetransmissionHelper_NotifyStatus(IN IMS_SINT32 nStatus) = 0;
};

#endif  // _INTERFACE_RETRANSMISSION_HELPER_LISTENER_H_
