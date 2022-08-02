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
#include "CarrierConfig.h"
#include "INetworkWatcher.h"
#include "MockIMtcService.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcLocationObject.h"

using ::testing::Return;
using ::testing::ReturnRef;

class MtcLocationObjectTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcService objService;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    CallInfo objCallInfo;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallInfo)
                .WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetService)
                .WillByDefault(ReturnRef(objService));

        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
    }
};

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsFalseIfAosConnectorIsNull)
{
    ON_CALL(objService, GetAosConnector())
            .WillByDefault(Return(nullptr));

    EXPECT_FALSE(MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsConfigForWifiNormal)
{
    const IMS_BOOL bConfig = IMS_TRUE;

    ON_CALL(*pConfigurationManager,
            IsSupportGeolocationPidfInSipInvite(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI))
            .WillByDefault(Return(bConfig));

    objCallInfo.bEmergency = IMS_FALSE;

    ON_CALL(objService, IsWlanIpCanType)
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(bConfig, MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsConfigForWifiEmergency)
{
    const IMS_BOOL bConfig = IMS_TRUE;
    ON_CALL(*pConfigurationManager,
            IsSupportGeolocationPidfInSipInvite(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI))
            .WillByDefault(Return(bConfig));

    objCallInfo.bEmergency = IMS_TRUE;

    ON_CALL(objService, IsWlanIpCanType)
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(bConfig, MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsConfigForCellularNormal)
{
    const IMS_BOOL bConfig = IMS_TRUE;
    ON_CALL(*pConfigurationManager,
            IsSupportGeolocationPidfInSipInvite(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(bConfig));

    objCallInfo.bEmergency = IMS_FALSE;

    ON_CALL(objService, IsWlanIpCanType)
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(bConfig, MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsConfigForCellularEmergency)
{
    const IMS_BOOL bConfig = IMS_TRUE;
    ON_CALL(*pConfigurationManager,
            IsSupportGeolocationPidfInSipInvite(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(bConfig));

    objCallInfo.bEmergency = IMS_TRUE;

    ON_CALL(objService, IsWlanIpCanType)
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(bConfig, MtcLocationObject::IsGeolocationInfoRequired(objContext));
}
