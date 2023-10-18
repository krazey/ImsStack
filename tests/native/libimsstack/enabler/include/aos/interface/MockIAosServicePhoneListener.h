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
#ifndef MOCK_I_AOS_SERVICE_PHONE_LISTENER_H_
#define MOCK_I_AOS_SERVICE_PHONE_LISTENER_H_

#include <gmock/gmock.h>

#include "interface/IAosServicePhoneListener.h"

class MockIAosServicePhoneListener : public IAosServicePhoneListener
{
public:
    MOCK_METHOD(void, ServicePhone_AosStart, (), (override));
    MOCK_METHOD(void, ServicePhone_notifyIpcanHandoverFailure,
            (IN IMS_SINT32 nTargetNetwork, IN IMS_SINT32 nCauseCode), (override));
    MOCK_METHOD(void, ServicePhone_IsimStateChanged, (IN IsimState eState), (override));
    MOCK_METHOD(void, ServicePhone_LocationInfoChanged, (IN LocationInfo eState), (override));
    MOCK_METHOD(void, ServicePhone_MobileDataLimitChanged, (IN IMS_BOOL bIsLimited), (override));
    MOCK_METHOD(void, ServicePhone_NetworkVideoCapabilityChanged, (IN IMS_BOOL bIsOn), (override));
    MOCK_METHOD(void, ServicePhone_PhoneNumberStateChanged,
            (IN IMS_BOOL bIsRefresh, IN PhoneNumberState eState), (override));
    MOCK_METHOD(void, ServicePhone_PlmnChanged, (), (override));
    MOCK_METHOD(void, ServicePhone_PowerOff, (), (override));
    MOCK_METHOD(
            void, ServicePhone_PreciseCallStateChanged, (IN PreciseCallState eState), (override));
    MOCK_METHOD(void, ServicePhone_PcoValueChanged, (IN IMS_SINT32 nValue), (override));
};

#endif  // MOCK_I_AOS_SERVICE_PHONE_LISTENER_H_