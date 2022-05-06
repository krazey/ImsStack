/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100720  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_REG_INFO_LISTENER_H_
#define _INTERFACE_REG_INFO_LISTENER_H_

class IRegInfoListener
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
    virtual void RegInfo_Updated(IN IMS_BOOL bStale = IMS_FALSE) = 0;

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
    virtual void RegInfo_UpdateFailed() = 0;
};

#endif  // _INTERFACE_REG_INFO_LISTENER_H_
