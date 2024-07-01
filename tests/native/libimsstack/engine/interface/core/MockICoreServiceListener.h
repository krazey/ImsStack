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
#ifndef MOCK_I_CORE_SERVICE_LISTENER_H_
#define MOCK_I_CORE_SERVICE_LISTENER_H_

#include <gmock/gmock.h>

#include "ICoreServiceListener.h"

class MockICoreServiceListener : public ICoreServiceListener
{
public:
    ~MockICoreServiceListener() override = default;

    MOCK_METHOD(void, CoreService_PageMessageReceived,
            (IN ICoreService * piService, IN IPageMessage* piMessage), (override));
    MOCK_METHOD(void, CoreService_ReferenceReceived,
            (IN ICoreService * piService, IN IReference* piReference), (override));
    MOCK_METHOD(void, CoreService_ServiceClosed,
            (IN ICoreService * piService, IN IReasonInfo* piReasonInfo), (override));
    MOCK_METHOD(void, CoreService_SessionInvitationReceived,
            (IN ICoreService * piService, IN ISession* piSession), (override));
    MOCK_METHOD(void, CoreService_UnsolicitedNotifyReceived,
            (IN ICoreService * piService, IN IMessage* piNotify), (override));
    MOCK_METHOD(void, CoreService_CapabilityQueryReceived,
            (IN ICoreService * piService, IN ICapabilities* piCapabilities), (override));
};

#endif
