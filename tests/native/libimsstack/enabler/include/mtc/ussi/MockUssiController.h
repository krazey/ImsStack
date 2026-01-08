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

#ifndef MOCK_USSI_CONTROLLER_H_
#define MOCK_USSI_CONTROLLER_H_

#include "ImsTypeDef.h"
#include "ussi/UssiController.h"
#include "ussi/UssiDef.h"
#include <gmock/gmock.h>

class AString;
class IMtcCallContext;
class ISession;
class UssiDataParser;

class MockUssiController : public UssiController
{
public:
    MockUssiController(IN IMtcCallContext& objContext, IN UssiDataParser* pParser) :
            UssiController(objContext, pParser)
    {
    }
    ~MockUssiController() override {}
    MOCK_METHOD(IMS_BOOL, HasValidXmlBodyForNetworkInitiatedUssi, (IN const IMessage*), (override));
    MOCK_METHOD(IMS_BOOL, IsUssiInfoReceived, (IN const ISipServerConnection*), (override));
    MOCK_METHOD(IMS_BOOL, HasXmlBodyInInfo, (IN const ISipServerConnection*), (override));

    MOCK_METHOD(UssiResult, HandleUssiBody, (IN const ISipMessage*, IN IMS_SINT32), (override));

    MOCK_METHOD(IMS_RESULT, FormStartUssiRequest, (IN const AString&), (override));
    MOCK_METHOD(IMS_RESULT, FormAcceptUssi, (), (override));
    MOCK_METHOD(IMS_RESULT, SendInfo, (IN ISession&, IN const AString&, IN UssiError), (override));

    MOCK_METHOD(void, SetNextActionByTerminateUssi, (), (override));
};

#endif
