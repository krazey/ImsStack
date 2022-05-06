/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101207  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_REG_BINDING_LISTENER_H_
#define _INTERFACE_REG_BINDING_LISTENER_H_

class SipAddress;
class CallerCapability;

class IRegBindingListener
{
public:
    /*
     Notifies the application that the registration binding (AOR - Contact) is
    in the ACTIVE state.

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
    virtual void RegBinding_OnActive() = 0;

    /*
     Notifies the application that the registration binding (AOR - Contact)
    is just removed (by de-REGISTER).

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
    virtual void RegBinding_OnDestroy() = 0;

    /*
     Notifies the application that the registration binding (AOR - Contact) is in the INIT state.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pAOR                    AOR of the registration
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void RegBinding_OnInit(IN CONST SipAddress* pAOR) = 0;

    /*
     Queries the service capability to the application. It is for the caller capabilities.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pCapability             Pointer to CallerCapability
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void RegBinding_OnQueryCapability(OUT CallerCapability*& pCapability) = 0;

    /*
     Queries the registration headers to the application.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    objHeaders              SIP headers to be set in REGISTER request
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void RegBinding_OnQueryRegistrationHeaders(OUT AStringArray& objHeaders) = 0;

    /*
     Notifies the application that the registration binding (AOR - Contact) is
    in the TERMINATED state.

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
    virtual void RegBinding_OnTerminated() = 0;
};

#endif  // _INTERFACE_REG_BINDING_LISTENER_H_
