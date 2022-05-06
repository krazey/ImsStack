/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090828  toastops@                 Created
    </table>

    Description

*/

#ifndef _INTERFACE_REFRESHABLE_H_
#define _INTERFACE_REFRESHABLE_H_

class ISipClientConnection;

/*

Refreshable interface

Example

See Also

*/
class IRefreshable
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
    virtual void Refreshable_RefreshCompleted(
            IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode = 0) = 0;

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
    virtual IMS_BOOL Refreshable_RefreshStarted() = 0;

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
    virtual void Refreshable_RefreshTerminated() = 0;
};

#endif  // _INTERFACE_REFRESHABLE_H_
