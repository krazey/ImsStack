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

#ifndef MOCK_AOS_DNS_QUERY_H_
#define MOCK_AOS_DNS_QUERY_H_

#include <gmock/gmock.h>

#include "ImsList.h"
#include "IpAddress.h"
#include "provider/AosDnsQuery.h"

class MockAosDnsQuery : public AosDnsQuery
{
public:
    MockAosDnsQuery() {}
    ~MockAosDnsQuery() {}
};

class MockIAosDnsQueryListener : public IAosDnsQueryListener
{
public:
    MOCK_METHOD(void, DnsQuery_Ready, (), (override));
    MOCK_METHOD(
            void, DnsQuery_Done, (IN IMS_BOOL bResult, IN IMSList<IPAddress> objIps), (override));
};

#endif  // MOCK_AOS_DNS_QUERY_H_
