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
#ifndef MOCK_I_SIP_CONNECTION_FACTORY_H_
#define MOCK_I_SIP_CONNECTION_FACTORY_H_

#include <gmock/gmock.h>

#include "ISipConnectionFactory.h"

class MockISipConnectionFactory : public ISipConnectionFactory
{
public:
    ~MockISipConnectionFactory() override = default;

    MOCK_METHOD(ISipClientConnection*, CreateClientConnection,
            (IN const SipMethod& objMethod, IN const SipAddress* pFrom, IN const SipAddress* pTo),
            (override));
    MOCK_METHOD(ISipClientConnection*, CreateClientConnection,
            (IN ISipDialog * piDialog, IN const SipMethod& objMethod), (override));
    MOCK_METHOD(IMS_BOOL, CreateResponse,
            (IN ISipServerConnection * piSsc, IN IMS_SINT32 nStatusCode,
                    IN const AString& strPhrase /*= AString::ConstNull()*/),
            (override));
    MOCK_METHOD(ISipServerConnection*, GetNewServerConnection, (), (override));
    MOCK_METHOD(void, SetDialog, (IN ISipDialog * piDialog), (override));
    MOCK_METHOD(void, SetListener, (IN ISipConnectionFactoryListener * piListener), (override));
    MOCK_METHOD(void, SetSscForCancel, (IN ISipServerConnection * piSsc), (override));
};

#endif
