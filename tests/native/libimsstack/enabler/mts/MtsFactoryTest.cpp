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

#include "CarrierConfig.h"
#include "ImsIdentity.h"
#include "ImsTypeDef.h"
#include "MockICarrierConfig.h"
#include "MtsDef.h"
#include "MtsFactory.h"
#include "PlatformContext.h"
#include "TestConfigService.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

class TestMtsFactory : public MtsFactory
{
public:
    IMS_BOOL Has(IN IMS_SINT32 nSlotId) { return m_objMtsApps.GetIndexOfKey(nSlotId) >= 0; }
};

class MtsFactoryTest : public ::testing::Test
{
public:
    TestConfigService objConfigService;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &objConfigService);

        ON_CALL(objConfigService.GetMockCarrierConfig(),
                GetInt(CarrierConfig::ImsSms::KEY_SMS_PREFERRED_PSI_URI_TYPE_INT, _))
                .WillByDefault(Return(URI_SCHEME_SIP));
        ON_CALL(objConfigService.GetMockCarrierConfig(),
                GetInt(CarrierConfig::ImsVoice::KEY_POLICY_OF_LOCAL_NUMBERS_INT, _))
                .WillByDefault(Return(ImsIdentity::DIALING_POLICY_HOME_LOCAL));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
    }
};

TEST_F(MtsFactoryTest, GetInstanceReturnsSameInstance)
{
    EXPECT_EQ(MtsFactory::GetInstance(), MtsFactory::GetInstance());
}

TEST_F(MtsFactoryTest, StartCreatesNewApp)
{
    TestMtsFactory objFactory;

    EXPECT_FALSE(objFactory.Has(IMS_SLOT_0));
    EXPECT_FALSE(objFactory.Has(IMS_SLOT_1));

    objFactory.Start(IMS_SLOT_1);

    EXPECT_FALSE(objFactory.Has(IMS_SLOT_0));
    EXPECT_TRUE(objFactory.Has(IMS_SLOT_1));
}

TEST_F(MtsFactoryTest, StartTwiceDoesNothing)
{
    TestMtsFactory objFactory;

    objFactory.Start(IMS_SLOT_0);
    objFactory.Start(IMS_SLOT_0);

    EXPECT_TRUE(objFactory.Has(IMS_SLOT_0));
}

TEST_F(MtsFactoryTest, StopDestroysApp)
{
    TestMtsFactory objFactory;
    objFactory.Start(IMS_SLOT_0);
    objFactory.Start(IMS_SLOT_1);

    objFactory.Stop(IMS_SLOT_1);

    EXPECT_TRUE(objFactory.Has(IMS_SLOT_0));
    EXPECT_FALSE(objFactory.Has(IMS_SLOT_1));
}

TEST_F(MtsFactoryTest, StopTwiceDoesNothing)
{
    TestMtsFactory objFactory;
    objFactory.Start(IMS_SLOT_0);

    objFactory.Stop(IMS_SLOT_0);
    objFactory.Stop(IMS_SLOT_0);

    EXPECT_FALSE(objFactory.Has(IMS_SLOT_0));
}

}  // namespace android
