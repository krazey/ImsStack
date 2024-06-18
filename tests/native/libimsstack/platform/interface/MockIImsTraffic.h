/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef MOCK_I_IMS_TRAFFIC_H_
#define MOCK_I_IMS_TRAFFIC_H_

#include <gmock/gmock.h>

#include "IImsTraffic.h"
#include "ImsTypeDef.h"

class IImsTrafficListener;

class MockIImsTraffic : public IImsTraffic
{
public:
    inline MockIImsTraffic() = default;
    inline ~MockIImsTraffic() = default;

    MOCK_METHOD(void, Disable, (IN IMS_SINT32), (override));
    MOCK_METHOD(IMS_BOOL, IsAllowed, (IN IMS_SINT32, IN IMS_UINT32), (override));
    MOCK_METHOD(void, Start, (IN IMS_SINT32, IN IMS_UINT32), (override));
    MOCK_METHOD(void, Stop, (IN IMS_SINT32, IN IMS_UINT32), (override));
    MOCK_METHOD(void, SetSimultaneousCallingSupported, (IN IMS_SINT32, IN IMS_BOOL), (override));
    MOCK_METHOD(void, SetWlan, (IN IMS_SINT32, IN IMS_BOOL), (override));
    MOCK_METHOD(void, AddListener, (IN IImsTrafficListener*), (override));
    MOCK_METHOD(void, RemoveListener, (IN IImsTrafficListener*), (override));
    MOCK_METHOD(void, DispatchServiceMessage, (IN IMS_UINTP, IN IMS_UINTP), (override));
};

#endif
