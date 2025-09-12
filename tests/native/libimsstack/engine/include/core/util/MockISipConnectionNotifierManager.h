/*
 * Copyright (C) 2024 The Android Open Source Project
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
#ifndef MOCK_I_SIP_CONNECTION_NOTIFIER_MANAGER_H_
#define MOCK_I_SIP_CONNECTION_NOTIFIER_MANAGER_H_

#include <gmock/gmock.h>

#include "util/ISipConnectionNotifierManager.h"

class MockISipConnectionNotifierManager : public ISipConnectionNotifierManager
{
public:
    MockISipConnectionNotifierManager() = default;
    ~MockISipConnectionNotifierManager() override = default;

    MOCK_METHOD(ISipConnectionNotifier*, CreateConnectionNotifier,
            (IN const AString& strScheme, IN const IpAddress& objIpAddr, IN IMS_SINT32 nPortS,
                    IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl,
                    IN const AString& strParams, IN const SipAddress& objUserId),
            (override));
    MOCK_METHOD(ISipConnectionNotifier*, GetConnectionNotifier,
            (IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort), (override));
    MOCK_METHOD(void, ReleaseConnectionNotifier, (IN ISipConnectionNotifier * &piScn), (override));
    MOCK_METHOD(void, Init, (IN IMS_SINT32 nSlotId), (override));
};

#endif
