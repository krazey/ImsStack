/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100221  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_CONFIGURABLE_H_
#define _INTERFACE_CONFIGURABLE_H_

#include "AString.h"

class IConfigUpdateListener;

class IConfigurable
{
public:
    /*
     Adds a new configuration update listener.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nCPI                    Configurable parameter item to be notified
    piListener              Listener for configuration update notification
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_TRUE                The configuration update listener is successfully added
    IMS_FALSE               The configuration update listener is not added
    </table>
    */
    virtual IMS_BOOL AddListener(IN IMS_SINT32 nCPI, IN IConfigUpdateListener* piListener) = 0;

    /*
     Removes the registered configuration update listener.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nCPI                    Configurable parameter item to be notified
    piListener              Listener for configuration update notification
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void RemoveListener(IN IMS_SINT32 nCPI, IN IConfigUpdateListener* piListener) = 0;

    /*
     Updates the specified configurable item.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nCPI                    Configurable parameter item to be updated
    strValue                Value to be updated for a specified item
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_TRUE                The configurable item is successfully updated
    IMS_FALSE               The configurable item is not updated
    </table>
    */
    virtual IMS_BOOL Update(
            IN IMS_SINT32 nCPI, IN const AString& strValue = AString::ConstNull()) = 0;

public:
    // Configurable Parameter Item for configuration engine

    // Updates from the storage medium or the specified value to the cache
    enum
    {
        CP_I_BASE = 0,

        // Items for control message; it will be used for IConfigUpdateListener
        CP_I_START_SUBSCRIBER = 1,
        CP_I_START_SIP,
        CP_I_START_SIP_V,
        CP_I_START_PRESENCE,
        CP_I_END_SUBSCRIBER,
        CP_I_END_SIP,
        CP_I_END_SIP_V,
        CP_I_END_PRESENCE,

        // ISubscriberConfig / IImsSubscriberInfo
        CP_I_SUBSCRIPTION_ATTRIBUTE_ALL = 51,
        CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM,
        CP_I_SUBSCRIPTION_ATTRIBUTE_USIM,
        CP_I_HOME_DOMAIN_NAME,
        CP_I_IMPI,
        CP_I_IMPU_PRIMARY_REF_INDEX,
        CP_I_IMPU_0,
        CP_I_IMPU_1,
        CP_I_IMPU_2,
        CP_I_IMPU_3,
        CP_I_IMPU_4,
        CP_I_IMPU_5,
        CP_I_IMPU_6,
        CP_I_IMPU_7,
        CP_I_IMPU_8,
        CP_I_IMPU_9,
        CP_I_PHONE_CONTEXT,
        CP_I_AUTH_USERNAME,
        CP_I_AUTH_PASSWORD,
        CP_I_AUTH_REALM,
        CP_I_AUTH_ALGORITHM,
        CP_I_SERVER_SCSCF,
        // P-CSCF addresses
        CP_I_PCSCF_DISCOVERY_METHODS,
        CP_I_PCSCF_ADDRESS_0,
        CP_I_PCSCF_ADDRESS_1,
        CP_I_PCSCF_ADDRESS_2,
        CP_I_PCSCF_ADDRESS_3,
        CP_I_PCSCF_ADDRESS_4,
        CP_I_PCSCF_ADDRESS_5,
        CP_I_PCSCF_ADDRESS_6,
        CP_I_PCSCF_ADDRESS_7,
        CP_I_PCSCF_ADDRESS_8,
        CP_I_PCSCF_ADDRESS_9,
        CP_I_PCSCF_PORT_0,
        CP_I_PCSCF_PORT_1,
        CP_I_PCSCF_PORT_2,
        CP_I_PCSCF_PORT_3,
        CP_I_PCSCF_PORT_4,
        CP_I_PCSCF_PORT_5,
        CP_I_PCSCF_PORT_6,
        CP_I_PCSCF_PORT_7,
        CP_I_PCSCF_PORT_8,
        CP_I_PCSCF_PORT_9,

        CP_I_PCSCF_ALL,

        CP_I_SUBSCRIBER_ALL,

        // ISipConfig / ISipConfigV
        CP_I_TV_T1 = 101,
        CP_I_TV_T2,
        /* milli-seconds */
        CP_I_TV_100_TRYING,
        CP_I_SIP_FEATURES,
        CP_I_TCP_CRITERION_LENGTH,
        CP_I_REG_EXPIRES,
        CP_I_REG_SUB,
        CP_I_REG_SUB_EXPIRES,

        // ISipConfigV only
        CP_I_TV_T4,
        CP_I_TV_TA,
        CP_I_TV_TB,
        CP_I_TV_TC,
        CP_I_TV_TD,
        CP_I_TV_TE,
        CP_I_TV_TF,
        CP_I_TV_TG,
        CP_I_TV_TH,
        CP_I_TV_TI,
        CP_I_TV_TJ,
        CP_I_TV_TK,
        CP_I_UA_VERSION,  // For service level, not Generic config.
        CP_I_FEATURE_TAG_OPTIONS,
        CP_I_SESSION_MINSE,
        CP_I_SESSION_EXPIRES,
        CP_I_SESSION_100TRYING_NOTIFICATION,
        CP_I_SIP_ALL,

        CP_I_MAX = 1000
    };

    // Updates from the cache to the storage medium
    enum
    {
        CP_I_WRITE_PROVISIONING_BASE = 10000,

        // ISubscriberConfig
        CP_I_WRITE_PROVISIONING_SUBSCRIBER,

        CP_I_WRITE_PROVISIONING_MAX = 11000
    };
};

#endif  // _INTERFACE_CONFIGURABLE_H_
