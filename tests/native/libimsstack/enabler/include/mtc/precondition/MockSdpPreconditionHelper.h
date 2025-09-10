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

#ifndef MOCK_SDP_PRECONDITION_HELPER_H_
#define MOCK_SDP_PRECONDITION_HELPER_H_

#include "ImsTypeDef.h"
#include "precondition/SdpPreconditionHelper.h"
#include <gmock/gmock.h>

class ISession;
class QosStatusTable;
class SdpMedia;

class MockSdpPreconditionHelper : public SdpPreconditionHelper
{
public:
    MockSdpPreconditionHelper() {}
    ~MockSdpPreconditionHelper() override {}
    MOCK_METHOD(void, FormPreconditionSdp,
            (IN ISession * piSession, IN QosStatusTable* pStatusTable, IN IMS_BOOL bUseConf),
            (override));
    MOCK_METHOD(void, RemovePreconditionSdp, (IN ISession * piSession), (override));
    MOCK_METHOD(void, FormFailurePreconditionSdp, (IN ISession * piSession), (override));
    MOCK_METHOD(IMS_UINT32, GetMediaType, (IN const SdpMedia* pSdpMedia, IN IMS_SINT32 nMediaState),
            (override));
    MOCK_METHOD(
            IMS_BOOL, IsPreconditionIncludedInSdp, (IN ISession * piSession), (override, const));
    MOCK_METHOD(IMS_BOOL, IsLocalResourceReservedInSdp,
            (IN ISession * piSession, IN IMS_SINT32 nServiceMethod), (override, const));
};

#endif
