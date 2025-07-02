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

#ifndef MOCK_MULTI_ENDPOINT_FACTORY_H_
#define MOCK_MULTI_ENDPOINT_FACTORY_H_

#include "dialogevent/IDialogInfoManager.h"
#include "dialogevent/IDialogSubscription.h"
#include "dialogevent/MultiEndpointFactory.h"
#include <gmock/gmock.h>
#include <memory>

class AString;
class IDialogSubscriptionListener;
class IMtcContext;

class MockMultiEndpointFactory : public MultiEndpointFactory
{
public:
    // MockMultiEndpointFactory() : MultiEndpointFactory() {}
    virtual ~MockMultiEndpointFactory() override {}

    MOCK_METHOD(std::unique_ptr<IDialogInfoManager>, CreateDialogInfoManager, (), (override));
    MOCK_METHOD(std::unique_ptr<IDialogSubscription>, CreateDialogSubscription,
            (IN IMtcContext&, IN IDialogSubscriptionListener&, IN const AString&), (override));
};

#endif
