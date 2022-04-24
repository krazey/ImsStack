/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090819  YR@                       Created
    20131212  yongnam.cha@              Featuring for RCS BB
    20150720  yongnam.cha@              Add GetPhoneId for MSIM
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_PHONE_INFO_SUBSCRIBER_H_
#define _INTERFACE_IMS_PHONE_INFO_SUBSCRIBER_H_

#include "AString.h"

typedef enum
{
    PREFERENCE_VALUE_STRING = 0,
    PREFERENCE_VALUE_BOOL,
    PREFERENCE_VALUE_INT,
    PREFERENCE_VALUE_LONG,
    PREFERENCE_VALUE_FLOAT,
} PREFERENCE_VALUE_ENTYPE;

class ISubscriberInfo
{
public:
    // CDMA: MDN, WCDMA: MSISDN(?)
    virtual IMS_BOOL GetPhoneNumber(OUT AString &strPhoneNumber) const = 0;
    virtual IMS_BOOL GetMcc(OUT AString &strMCC) const = 0;
    virtual IMS_BOOL GetMnc(OUT AString &strMNC) const = 0;
    virtual IMS_BOOL GetOperator(OUT AString &strOperator) const = 0;
    virtual IMS_BOOL GetCountry(OUT AString &strCountry) const = 0;
    virtual IMS_BOOL GetNetworkCountry(OUT AString &strCountry) const = 0;
    virtual IMS_BOOL GetSubscriberId(OUT AString &strIMSI) const = 0;
    virtual IMS_BOOL GetEmergencyNumberListFromSim(OUT AString &strENLFromSIM) const = 0;
    virtual IMS_SINT32 GetEmergencyPriorityFromModem() = 0;
    virtual IMS_BOOL IsUiccGbaSupported() = 0;
    virtual IMS_BOOL GetPreference(IN const AString &strFileName,
            IN const AString &strKey, OUT AString &strValue,
            IN PREFERENCE_VALUE_ENTYPE eValueType = PREFERENCE_VALUE_STRING) = 0;
    virtual IMS_BOOL SetPreference(IN const AString &strFileName,
            IN const AString &strKey, IN const AString &strValue,
            IN PREFERENCE_VALUE_ENTYPE eValueType = PREFERENCE_VALUE_STRING) = 0;
};

#endif // _INTERFACE_IMS_PHONE_INFO_SUBSCRIBER_H_
