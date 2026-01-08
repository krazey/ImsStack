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
#ifndef TEST_CONNECTOR_H_
#define TEST_CONNECTOR_H_

#include "MockICoreService.h"
#include "MockISipClientConnection.h"

class TestConnectorPrivate;

class TestConnector
{
public:
    TestConnector();
    ~TestConnector();

    TestConnector(IN const TestConnector&) = delete;
    TestConnector& operator=(IN const TestConnector&) = delete;

public:
    MockICoreService& GetMockCoreService();
    MockISipClientConnection& GetMockSipClientConnection();

    void SetCoreService(IN ICoreService* piCoreService);
    void SetCoreService(IN const AString& strServiceId, IN ICoreService* piCoreService);
    void ClearCoreService(IN const AString& strServiceId);
    void SetSipClientConnection(IN ISipClientConnection* piScc);

private:
    TestConnectorPrivate* m_pPrivate;
};

#endif
