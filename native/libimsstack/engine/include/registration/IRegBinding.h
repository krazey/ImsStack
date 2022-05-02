/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100912  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_REG_BINDING_H_
#define _INTERFACE_REG_BINDING_H_

#include "AStringArray.h"
#include "IPAddress.h"
#include "SipAddress.h"

class CallerCapability;
class SIPProfile;
class IRegBindingListener;
class IRegInfo;



/*

IRegBinding interface

Example

See Also

*/
class IRegBinding
{
public:
    /*
     Returns the network authorized public user identities.

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
    AStringArray            List of public user identity
    </table>

    */
    virtual const AStringArray& GetAssociatedURIs() const = 0;

    /*
     Returns the network authorized & registered public user identity.

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
    SIPAddress              Registered public user identity
    </table>

    */
    virtual const SIPAddress& GetAuthorizedAOR() const = 0;

    /*
     Returns the preferred contact address with the highest 'q' value of this registration.

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
    SIPAddress              Preferred contact address
    </table>

    */
    virtual const SIPAddress& GetContactAddress() const = 0;

    /*
     Returns the preferred contact address for all the outgoing SIP message.

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
    SIPAddress              Preferred contact address
    </table>

    */
    virtual const SIPAddress* GetContactAddressForOutgoingMessage() const = 0;

    /*
     Returns the IP address of the preferred contact address.

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
    IPAddress               IP address of preferred contact address
    </table>

    */
    virtual const IPAddress& GetIPAddress() const = 0;

    /*
     Returns the Path header list.

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
    AStringArray            List of Path header
    </table>

    */
    virtual const AStringArray& GetPathHeaders() const = 0;

    /*
     Returns the port number for the flow control based on RFC5626.

    Remarks
     RFC5626_FLOW_CONTROL

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SINT32              Port number
    </table>

    */
    virtual IMS_SINT32 GetPortFlowControl() const = 0;

    /*
     Returns the protected / unprotected client port number (UE).

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
    IMS_SINT32              Port number
    </table>

    */
    virtual IMS_SINT32 GetPortUC() const = 0;

    /*
     Returns the protected / unprotected server port number (UE).

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
    IMS_SINT32              Port number
    </table>

    */
    virtual IMS_SINT32 GetPortUS() const = 0;

    /*
     Returns the reginfo when the 'reg' event package subscription is supported.

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
    const IRegInfo*         Pointer to IRegInfo
    </table>

    */
    virtual const IRegInfo* GetRegInfo() const = 0;

    /*
     Returns the list of Security-Client header if present.

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
    AStringArray            List of Security-Client header
    </table>

    */
    virtual const AStringArray& GetSecurityClients() const = 0;

    /*
     Returns the list of Security-Verify header if present.

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
    AStringArray            List of Security-Verify header
    </table>

    */
    virtual const AStringArray& GetSecurityVerifys() const = 0;

    /*
     Returns the preloaded route set for the outgoing SIP request.

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
    AStringArray            List of preloaded route
    </table>

    */
    virtual const AStringArray& GetServiceRoutes() const = 0;

    /*
     Returns the SIP profile of registration binding.

    Remarks
     MULTI_REG_SIP_PROFILE

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    SIPProfile              Instance of SIPProfile
    </table>

    */
    virtual SIPProfile* GetSIPProfile() const = 0;

    /*
     Returns the state of registration binding.

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
    IMS_SINT32              State of RegBinding
    </table>

    */
    virtual IMS_SINT32 GetState() const = 0;

    /*
     Returns the subscription identifier which can identify the subscriber.

    Remarks
     MULTI_SUBS

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Identifier of the subscriber
    </table>

    */
    virtual const AString& GetSubscriberId() const = 0;

    /*
     Returns the transport extensions.

    Remarks
     MULTI_REG_TRANSPORT

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SINT32              Transport extension (SIP::TRANSPORT_EXT_XXX in Sip.h)
    </table>

    */
    virtual IMS_SINT32 GetTransportExt() const = 0;

    /*
     Returns the instance("+sip.instance") header parameter.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    SIPParameter*           Pointer to SIP header parameter for "+sip.instance"
    </table>

    */
    virtual const SIPParameter* GetInstanceParameter() const = 0;

    /*
     Returns the public GRUU.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    SIPAddress*             Pointer to public GRUU
    </table>

    */
    virtual const SIPAddress* GetPublicGRUU() const = 0;

    /*
     Returns the valid (the latest) temporary GRUU.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    SIPAddress*             Pointer to temporary GRUU
    </table>

    */
    virtual const SIPAddress* GetTemporaryGRUU() const = 0;

    /*

     Returns the valid temporary GRUUs.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMSList<SIPAddress*>    List of temporary GRUU
    </table>

    */
    virtual const IMSList<SIPAddress*>& GetTemporaryGRUUs() const = 0;

    /*
     Checks if the UA is located behind a NAT or not.

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
    IMS_TRUE                UA is located behind a NAT / FW
    IMS_FALSE               UA is not located behind a NAT / FW
    </table>

    */
    virtual IMS_BOOL IsBehindNAT() const = 0;

    /*
     Checks if the UA is located within the trust domain.

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
    IMS_TRUE                UA is located within the trust domain
    IMS_FALSE               UA is not located within the trust domain
    </table>

    */
    virtual IMS_BOOL IsWithinTrustDomain() const = 0;

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

    /*
     Sets the listener for this registration binding.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    piListener              Listener to be set
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void SetListener(IN IRegBindingListener *piListener) = 0;

public:
    enum
    {
        STATE_CREATED = 0,
        STATE_INIT,
        STATE_INIT_PENDING,
        STATE_ACTIVE,
        STATE_ACTIVE_PENDING,
        STATE_ACTIVE_TERMINATING,
        STATE_TERMINATED
    };
};

#endif // _INTERFACE_REG_BINDING_H_
