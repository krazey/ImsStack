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

#include "FeatureCaps.h"
#include "IImsAosInfo.h"
#include "MockIMtcService.h"
#include "call/MockIMtcCallContext.h"
#include "call/message/EmergencyMessageFormatter.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockICoreService.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MtcSupplementaryService.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/SipParameter.h"
#include "utility/MessageUtils.h"
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class EmergencyMessageFormatterTest : public ::testing::Test
{
public:
    EmergencyMessageFormatter* pFormatter;

    CallInfo objCallInfo;
    MockIMessage objMessage;
    MockISipMessage objSipMessage;
    MockIMtcCallContext objContext;
    MockIMtcService objService;
    MockISession objSession;
    MockICoreService objCoreService;
    MockIMtcConfigurationManager* pConfigurationManager;
    MockIMtcAosConnector objAosConnector;
    MtcConfigurationProxy* pConfigurationProxy;
    MtcSupplementaryService* pSupplementaryService;
    MessageUtils objMessageUtils;
    FeatureCaps* pFeatureCaps;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        pSupplementaryService = new MtcSupplementaryService(objContext, *pConfigurationProxy);
        pFeatureCaps = new FeatureCaps();

        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetAosConnector).WillByDefault(Return(&objAosConnector));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));
        ON_CALL(objCoreService, GetFeatureCaps).WillByDefault(Return(pFeatureCaps));
        ON_CALL(objCoreService, GetUserIdentities)
                .WillByDefault(ReturnRef(AStringArray::ConstNull()));
        ON_CALL(objAosConnector, GetRegistrationMode)
                .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));
        ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));
        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));

        pFormatter = new EmergencyMessageFormatter(objContext, objSession);
    }

    virtual void TearDown() override
    {
        delete pFormatter;
        delete pConfigurationProxy;
        delete pSupplementaryService;
        delete pFeatureCaps;
    }
};

TEST_F(EmergencyMessageFormatterTest, FormStartMessageNormalCase)
{
    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, FormStartMessageFailureCase)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(EmergencyMessageFormatterTest, GetAoSRegMode)
{
    ON_CALL(objContext, GetAosConnector).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(EmergencyMessageFormatterTest, SetPPreferredIdentityHeader)
{
    ON_CALL(objSipMessage, IsHeaderPresent).WillByDefault(Return(IMS_TRUE));
    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objSipMessage, IsHeaderPresent).WillByDefault(Return(IMS_FALSE));
    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, SetPPreferredIdentityHeaderByDeviceId)
{
    const AString strLocalIpv6 = "::1";
    const AString strLocalIpv4 = "127.0.0.1";
    const IMS_UINT32 nLocalPort = 5060;

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_INTERNAL));
    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(strLocalIpv6));
    ON_CALL(objAosConnector, GetLocalPort).WillByDefault(Return(nLocalPort));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(strLocalIpv4));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, GetLocalIpAddress)
{
    const AString strLocalIp = "::1";

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_INTERNAL));

    EXPECT_CALL(objContext, GetAosConnector)
            .WillOnce(Return(&objAosConnector))
            .WillOnce(Return(&objAosConnector))
            .WillOnce(Return(nullptr))
            .WillRepeatedly(Return(&objAosConnector));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(AString::ConstEmpty()));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(strLocalIp));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, GetLocalPort)
{
    const AString strLocalIp = "::1";
    const IMS_UINT32 nLocalPort = 5060;
    const IMS_UINT32 nLocalPortZero = 0;

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_INTERNAL));
    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(strLocalIp));

    EXPECT_CALL(objContext, GetAosConnector)
            .WillOnce(Return(&objAosConnector))
            .WillOnce(Return(&objAosConnector))
            .WillOnce(Return(&objAosConnector))
            .WillOnce(Return(nullptr))
            .WillRepeatedly(Return(&objAosConnector));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objAosConnector, GetLocalPort).WillByDefault(Return(nLocalPortZero));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objAosConnector, GetLocalPort).WillByDefault(Return(nLocalPort));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, SetPPreferredIdentityHeaderByUserId)
{
    const AString strUserIdentity = "sip:user@google.com";

    EXPECT_CALL(objService, GetICoreService)
            .WillOnce(Return(&objCoreService))
            .WillOnce(Return(nullptr))
            .WillRepeatedly(Return(&objCoreService));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    AStringArray objUserIdentities;
    objUserIdentities.AddElement(AString::ConstEmpty());
    objUserIdentities.AddElement(strUserIdentity);
    ON_CALL(objCoreService, GetUserIdentities).WillByDefault(ReturnRef(objUserIdentities));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, SetSipInstanceFeature)
{
    ON_CALL(objSipMessage, IsHeaderPresent).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objService, GetICoreService)
            .WillOnce(Return(&objCoreService))
            .WillOnce(Return(&objCoreService))
            .WillOnce(Return(nullptr))
            .WillRepeatedly(Return(&objCoreService));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_INTERNAL));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objCoreService, GetInstanceParameter).WillByDefault(Return(nullptr));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    SipParameter objParameter;
    ON_CALL(objCoreService, GetInstanceParameter).WillByDefault(Return(&objParameter));

    ON_CALL(objCoreService, GetFeatureCaps).WillByDefault(Return(nullptr));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objCoreService, GetFeatureCaps).WillByDefault(Return(pFeatureCaps));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, SetCurrentLocationDiscovery)
{
    ON_CALL(*pConfigurationManager, IsEmergencyCallCurrentLocationDiscoverySupported)
            .WillByDefault(Return(IMS_TRUE));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(*pConfigurationManager, IsEmergencyCallCurrentLocationDiscoverySupported)
            .WillByDefault(Return(IMS_FALSE));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);
}

}  // namespace android
