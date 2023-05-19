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

#ifndef MOCK_I_PHONE_INFO_SUBSCRIBER_H_
#define MOCK_I_PHONE_INFO_SUBSCRIBER_H_

#include <gmock/gmock.h>
#include "AString.h"
#include "IPhoneInfoSubscriber.h"

class MockISubscriberInfo : public ISubscriberInfo
{
public:
    inline MockISubscriberInfo() {}
    inline virtual ~MockISubscriberInfo() {}

    MOCK_METHOD(IMS_BOOL, GetPhoneNumber, (OUT AString& strPhoneNumber), (const, override));
    MOCK_METHOD(IMS_BOOL, GetSimMcc, (OUT AString & strMcc), (const, override));
    MOCK_METHOD(IMS_BOOL, GetSimMnc, (OUT AString & strMnc), (const, override));
    MOCK_METHOD(IMS_BOOL, GetSimCountryIso, (OUT AString & strCountry), (const, override));
    MOCK_METHOD(IMS_BOOL, GetNetworkCountryIso, (OUT AString & strCountry), (const, override));
    MOCK_METHOD(IMS_BOOL, GetSubscriberId, (OUT AString & strImsi), (const, override));
    MOCK_METHOD(IMS_BOOL, GetPreference,
            (IN const AString& strFileName, IN const AString& strKey, OUT AString& strValue),
            (override));
    MOCK_METHOD(IMS_BOOL, SetPreference,
            (IN const AString& strFileName, IN const AString& strKey, IN const AString& strValue),
            (override));
};

#endif
