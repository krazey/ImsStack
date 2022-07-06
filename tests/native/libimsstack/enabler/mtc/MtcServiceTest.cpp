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

#include <gtest/gtest.h>
#include "IMtcService.h"
#include "MtcService.h"
#include "MockIMtcContext.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "MtcEmergencyServiceManager.h"

LOCAL IMS_SINT32 SLOT_ID = 0;

using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class MtcServiceTest : public ::testing::Test
{
public:
    MockIMtcContext objMockContext;
    MockIMtcConfigurationManager objMockConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;

protected:
    virtual void SetUp() override
    {
        pConfigurationProxy = new MtcConfigurationProxy(&objMockConfigurationManager);
        ON_CALL(objMockContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objMockContext, GetSlotId).WillByDefault(Return(SLOT_ID));

        MtcEmergencyServiceManager* pEmergencyManager =
                new MtcEmergencyServiceManager(objMockContext);
        ON_CALL(objMockContext, GetEmergencyServiceManager)
                .WillByDefault(Return(pEmergencyManager));
    }

    virtual void TearDown() override {}
};

TEST_F(MtcServiceTest, NoCrashOnSetJniServiceWithNull)
{
    MtcService* pMtcService = new MtcService(objMockContext, ServiceType::NORMAL);
    pMtcService->SetJniService(IMS_NULL);
    delete pMtcService;
}

}  // namespace android
