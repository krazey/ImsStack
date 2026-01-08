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
#ifndef MOCK_I_IPCAN_H_
#define MOCK_I_IPCAN_H_

#include <gmock/gmock.h>

#include "IIpcan.h"

class MockIIpcan : public IIpcan
{
public:
    MockIIpcan() = default;
    ~MockIIpcan() override = default;

    MOCK_METHOD(void, GetAccessInfo, (IN IMS_SINT32 nSlotId, IN_OUT AccessNetworkInfo& objAni),
            (override));
    MOCK_METHOD(void, GetAccessInfoForWiFi, (OUT AccessNetworkInfo & objAni), (override));
    MOCK_METHOD(void, GetLastAccessInfo,
            (IN IMS_SINT32 nSlotId, OUT AccessNetworkInfo& objAni, OUT AString& strTimestamp,
                    OUT AString& strCellInfoAge),
            (override));
    MOCK_METHOD(void, GetLastAccessInfoForWiFi,
            (OUT AccessNetworkInfo & objAni, OUT AString& strTimestamp,
                    OUT AString& strCellInfoAge),
            (override));
    MOCK_METHOD(IMS_SINT32, GetNetworkType, (IN IMS_SINT32 nSlotId), (override));
};

#endif
