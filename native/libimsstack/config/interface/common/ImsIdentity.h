/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100905  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _IMS_IDENTITY_H_
#define _IMS_IDENTITY_H_

#include "AString.h"

class AccessNetworkInfo;

// Static class for identities of IMS services
class ImsIdentity
{
private:
    ImsIdentity();

public:
    /*
     Returns the public user identity with SIP URI from MSISDN or MDN.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nSlotId                 Key to find out a proper configuration
    bUserPhoneParam         Flag to indicate if "user=phone" is included or not
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Public user identity (SIP URI)
    </table>
    */
    static AString CreateSIPUserId(
            IN IMS_SINT32 nSlotId = IMS_SLOT_0, IN IMS_BOOL bUserPhoneParam = IMS_FALSE);

    /*
     Returns the SIP URI from the specified phone number and phone-context parameter.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    strDialString           Phone number digits
                            If null or empty string,
                            the result of this method is same to CreateSIPUserId().
    nSlotId                 Key to find out a proper configuration
    bUserPhoneParam         Flag to indicate if "user=phone" is included or not
    strPhoneContext         Domain name for phone-context
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Public user identity (SIP URI)
    </table>
    */
    static AString CreateSIPUserId(IN const AString& strDialString,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0, IN IMS_BOOL bUserPhoneParam = IMS_FALSE,
            IN const AString& strPhoneContext = AString::ConstNull());

    /*
     Returns the SIP URI from the specified dial string and phone-context parameter.
    It includes "user=dialstring" URI parameter.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    strDialString           Dial string
    nSlotId                 Key to find out a proper configuration
    strPhoneContext         Domain name for phone-context
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Public user identity (SIP URI)
    </table>
    */
    static AString CreateSIPUserIdWithDialString(IN const AString& strDialString,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0,
            IN const AString& strPhoneContext = AString::ConstNull());

    /*
     Returns the SIP URI from the specified dial string and phone-context parameter.
    It includes the embedded tel URI format in user-info field and "user=phone" URI parameter.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    strDialString           Dial string
    nSlotId                 Key to find out a proper configuration
    strPhoneContext         Domain name for phone-context
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Public user identity (SIP URI)
    </table>
    */
    static AString CreateSIPUserIdWithPhone(IN const AString& strDialString,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0,
            IN const AString& strPhoneContext = AString::ConstNull());

    /*
     Returns the public user identity with TEL URI from MSISDN or MDN of this device.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    strPhoneContext         Phone-context info.
    nSlotId                 Key to find out a proper configuration
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Public user identity (TEL URI)
    </table>
    */
    static AString CreateTelUserId(
            IN const AString& strPhoneContext, IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    /*
     Returns the TEL URI from the specified phone number and phone-context parameter.
    It returns the TEL URI without 'phone-context' parameter if the strNumberDigits is
    a global number digits.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    strDialString           Phone number digits
    strPhoneContext         Phone-context info.
    nSlotId                 Key to find out a proper configuration
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Public user identity (TEL URI)
    </table>
    */
    static AString CreateTelUserId(IN const AString& strDialString,
            IN const AString& strPhoneContext, IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    /*
     Returns the temporary home domain name.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nSlotId                 Key to find out a proper configuration
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Temporary home domain name
    </table>
    */
    static AString CreateTemporaryHomeDomainName(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    /*
     Returns the temporary private user identity according to the 3GPP.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nSlotId                 Key to find out a proper configuration
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Temporary private user identity
    </table>
    */
    static AString CreateTemporaryPrivateUserId(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    /*
     Returns the temporary public user identity according to the 3GPP.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nSlotId                 Key to find out a proper configuration
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Temporary public user identity
    </table>
    */
    static AString CreateTemporaryPublicUserId(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    /*
     Returns the anonymous SIP URI according to RFC 3261.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Anonymous SIP URI
    </table>
    */
    static const AString& GetAnonymousUserId();

    /*
     Returns the home domain name.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nSlotId                 Key to find out a proper configuration
    strSubscriberId         Subscriber id to identify the subscriber's config
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Home domain name
    </table>
    */
    static const AString& GetHomeDomainName(IN IMS_SINT32 nSlotId = IMS_SLOT_0,
            IN const AString& strSubscriberId = AString::ConstNull());

    /*
     Returns the MCC / MNC from the specified PLMN.
    If the PLMN is not specified (length is zero), it will retrieve the valid MCC/MNC
    from the platform.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    strPLMN                 PLMN (MCC + MNC) (5 or 6 length)
    nMncDigits              Digits of MNC
    strMcc                  MCC
    strMnc                  MNC
    nSlotId                 Key to find out a proper configuration
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_TRUE                When MCC and MNC are not null or empty string
    IMS_FALSE               When MCC or MNC is not valid
    </table>
    */
    static IMS_BOOL GetMccMnc(IN const AString& strPLMN, IN IMS_SINT32 nMncDigits,
            OUT AString& strMcc, OUT AString& strMnc, IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    /*
     Returns the phone-context string for tel URI.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nDialingPolicy          Dialing policy to consist of phone-context (DIALING_POLICY_XXX)
    nSlotId                 Key to find out a proper configuration
    pANI                    Pointer to access network information
                            If the format is geo-local number, it MUST be specified.
    strSubscriberId         Subscriber id to identify the subscriber's config
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Phone context string
    </table>
    */
    static const AString GetPhoneContext(IN IMS_SINT32 nDialingPolicy,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0, IN AccessNetworkInfo* pANI = IMS_NULL,
            IN const AString& strSubscriberId = AString::ConstNull());

    /*
     Returns the unavailable SIP URI according to 3GPP TS 23.003.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Unavailable SIP URI
    </table>
    */
    static const AString& GetUnavailableUserId();

public:
    // Dialing Policy
    // For type of "phone-context" parameter in tel URI when dialing
    enum
    {
        // home-local number is a default
        DIALING_POLICY_HOME_LOCAL = 0,
        DIALING_POLICY_GEO_LOCAL,
        DIALING_POLICY_OTHER
    };
};

#endif  // _IMS_IDENTITY_H_
