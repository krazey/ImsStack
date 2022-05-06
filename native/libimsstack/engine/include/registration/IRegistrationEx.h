/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20130302  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_REGISTRATION_EX_H_
#define _INTERFACE_REGISTRATION_EX_H_

#include "IRegistration.h"
#include "ISipConnectionNotifierErrorListener.h"

class RegInfo;
class RegObserver;
class RegStateTracker;

/*

Registration extension interface

Example

See Also

*/
class IRegistrationEx : public IRegistration, public ISipConnectionNotifierErrorListener
{
public:
    /*
     Adds the observer to get the registration state transition.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pObserver               Pointer to RegObserver
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void AddObserver(IN RegObserver* pObserver) = 0;

    /*
     Removes the observer to get the registration state transition.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pObserver               Pointer to RegObserver
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void RemoveObserver(IN RegObserver* pObserver) = 0;

    /*
     Adds the reference for SIP connection notifier's error listener.

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
    IMS_SINT32              Current reference count
    </table>

    */
    virtual IMS_SINT32 AddReferenceForSCNEL() = 0;

    /*
     Removes the reference for SIP connection notifier's error listener.

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
    IMS_SINT32              Current reference count
    </table>

    */
    virtual IMS_SINT32 RemoveReferenceForSCNEL() = 0;

    /*
     Returns the reg info of this registration.

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
    RegInfo*                Pointer to RegInfo ("reg" event package info.)
    </table>

    */
    virtual const RegInfo* GetRegInfo() const = 0;

    /*
     Returns the registration state tracker of this registration.

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
    RegStateTracker*        Pointer to RegStateTracker
    </table>

    */
    virtual const RegStateTracker* GetStateTracker() const = 0;

    /*
     Notifies the registration when the caller capability is changed to refresh
    the IMS registration if the device is already registered to the IMS network.

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
    virtual void NotifyCallerCapabilityChanged() = 0;

public:
    // Update states for registration binding
    enum
    {
        BINDING_REGISTERING,
        BINDING_DEREGISTERING,
        BINDING_RESULT_OK,
        BINDING_RESULT_NOK,
        BINDING_RESTORE,
        // REG_RESTORATION_FOR_ACTIVE_BINDING
        BINDING_RESTORE_ACTIVE_BINDINGS,
        BINDING_DESTROY_CONTACT,
        BINDING_DESTROY
    };
};

#endif  // _INTERFACE_REGISTRATION_EX_H_
