/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef INTERFACE_PHONE_INFO_SUBSCRIBER_H_
#define INTERFACE_PHONE_INFO_SUBSCRIBER_H_

#include "AString.h"

enum PREFERENCE_VALUE_ENTYPE
{
    PREFERENCE_VALUE_STRING = 0,
    PREFERENCE_VALUE_BOOL,
    PREFERENCE_VALUE_INT,
    PREFERENCE_VALUE_LONG,
    PREFERENCE_VALUE_FLOAT,
};

class ISubscriberInfo
{
public:
    // CDMA: MDN, WCDMA: MSISDN(?)
    virtual IMS_BOOL GetPhoneNumber(OUT AString& strPhoneNumber) const = 0;
    virtual IMS_BOOL GetMcc(OUT AString& strMcc) const = 0;
    virtual IMS_BOOL GetMnc(OUT AString& strMnc) const = 0;
    virtual IMS_BOOL GetOperator(OUT AString& strOperator) const = 0;
    virtual IMS_BOOL GetCountry(OUT AString& strCountry) const = 0;
    virtual IMS_BOOL GetNetworkCountry(OUT AString& strCountry) const = 0;
    virtual IMS_BOOL GetSubscriberId(OUT AString& strImsi) const = 0;
    virtual IMS_BOOL GetEmergencyNumberListFromSim(OUT AString& strEnlFromSim) const = 0;
    virtual IMS_SINT32 GetEmergencyPriorityFromModem() = 0;
    virtual IMS_BOOL IsUiccGbaSupported() = 0;
    virtual IMS_BOOL GetPreference(IN const AString& strFileName, IN const AString& strKey,
            OUT AString& strValue,
            IN PREFERENCE_VALUE_ENTYPE enValueType = PREFERENCE_VALUE_STRING) = 0;
    virtual IMS_BOOL SetPreference(IN const AString& strFileName, IN const AString& strKey,
            IN const AString& strValue,
            IN PREFERENCE_VALUE_ENTYPE enValueType = PREFERENCE_VALUE_STRING) = 0;
};

#endif
