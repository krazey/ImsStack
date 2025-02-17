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
#ifndef MOCK_I_AOS_REGISTRATION_CONTROL_LISTENER_H_
#define MOCK_I_AOS_REGISTRATION_CONTROL_LISTENER_H_

#include <gmock/gmock.h>

#include "interface/IAosRegistrationControlListener.h"

class MockIAosRegistrationControlListener : public IAosRegistrationControlListener
{
public:
    MOCK_METHOD(void, RegistrationControl_UpdateSipDelegateRegistration, (), (override));
    MOCK_METHOD(void, RegistrationControl_TriggerSipDelegateDeregistration, (), (override));
    MOCK_METHOD(void, RegistrationControl_TriggerFullNetworkRegistration,
            (IN IMS_SINT32 nSipCode, IN const AString& strTarget), (override));
    MOCK_METHOD(void, RegistrationControl_NotifyCapabilitiesChanged,
            ((IN const ImsMap<IMS_UINT32, IMS_UINT32>& objCapabilities)), (override));
    MOCK_METHOD(void, RegistrationControl_ControlRegistration,
            (IN AosRegRequestType eType, IN AosPcscfOrder eOrder, IN AosControlCause eCause),
            (override));
    MOCK_METHOD(void, RegistrationControl_UpdateDataFailureReason, (IN AosReasonCode eReason),
            (override));
};

#endif  // MOCK_I_AOS_REGISTRATION_CONTROL_LISTENER_H_