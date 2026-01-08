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

#ifndef MOCK_I_IMS_AOS_LISTENER_H_
#define MOCK_I_IMS_AOS_LISTENER_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"

#include "IImsAosListener.h"

class MockIImsAosListener : public IImsAosListener
{
public:
    MOCK_METHOD(
            void, ImsAos_Connected, (IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan), (override));
    MOCK_METHOD(void, ImsAos_Connecting, (), (override));
    MOCK_METHOD(void, ImsAos_Disconnecting, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ImsAos_Disconnected,
            (IN IMS_UINT32 nReason, IN IMS_SINT32 nDataFailureReason), (override));
    MOCK_METHOD(void, ImsAos_Suspended, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ImsAos_Resumed, (), (override));
};

#endif  // MOCK_I_IMS_AOS_LISTENER_H_
