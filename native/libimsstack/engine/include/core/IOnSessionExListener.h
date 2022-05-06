/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100615  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_ON_SESSION_EX_LISTENER_H_
#define _INTERFACE_ON_SESSION_EX_LISTENER_H_

class SessionEx;
class VirtualSession;

/*

IOnSessionExListener interface

Example

See Also

*/
class IOnSessionExListener
{
public:
    /*
     Notifies the application that the UPDATE was successfully delivered on the early state.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSessionEx              Pointer to SessionEx object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSessionEx_EarlyMediaUpdated(IN SessionEx* pSessionEx) = 0;

    /*
     Notifies the application that the UPDATE was not successfully delivered on the early state.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSessionEx              Pointer to SessionEx object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSessionEx_EarlyMediaUpdateFailed(IN SessionEx* pSessionEx) = 0;

    /*
     Notifies the application that the acknowledgement of the provisional response is received.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSessionEx              Pointer to SessionEx object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSessionEx_EarlyMediaUpdateReceived(IN SessionEx* pSessionEx) = 0;

    /*
     Notifies the application that the acknowledgement of the provisional response
    was successfully delivered.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSessionEx              Pointer to SessionEx object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSessionEx_PRAckDelivered(IN SessionEx* pSessionEx) = 0;

    /*
     Notifies the application that the acknowledgement of the provisional response
    was not successfully delivered.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSessionEx              Pointer to SessionEx object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSessionEx_PRAckDeliveryFailed(IN SessionEx* pSessionEx) = 0;

    /*
     Notifies the application that the acknowledgement of the provisional response is received.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSessionEx              Pointer to SessionEx object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSessionEx_PRAckReceived(IN SessionEx* pSessionEx) = 0;

    /*
     Notifies the application that the RPR delivery is failed (no PRACK received).

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSession                Pointer to SessionEx object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnSessionEx_RPRDeliveryFailed(IN SessionEx* pSessionEx) = 0;

    /*
     Notifies the application that the reliable provisional response is received.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSessionEx              Pointer to SessionEx object
    pVirtualSession         Pointer to VirtualSession object
    nIndex                  Index of the current response message
                            (0xFFFFFFFF : most recent message)
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSessionEx_RPRReceived(IN SessionEx* pSessionEx,
            IN VirtualSession* pVirtualSession, IN IMS_UINT32 nIndex = 0xFFFFFFFF) = 0;
};

#endif  // _INTERFACE_ON_SESSION_EX_LISTENER_H_
