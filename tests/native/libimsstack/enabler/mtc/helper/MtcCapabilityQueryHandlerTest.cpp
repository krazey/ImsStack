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

#include "ImsAosParameter.h"
#include "MockIMtcContext.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockICapabilities.h"
#include "core/MockICoreService.h"
#include "core/MockIMessage.h"
#include "helper/MtcCapabilityQueryHandler.h"
#include "sipcore/ISipHeader.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/SipAddress.h"
#include "util/IpAddress.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class MtcCapabilityQueryHandlerTest : public ::testing::Test
{
public:
    MockIMtcContext objMockContext;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MockICoreService objMockCoreService;
    MockIMessage objMockMessage;
    MockICapabilities objMockCapabilities;

    MtcCapabilityQueryHandler* pCapaQueryHandler;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objMockContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objMockContext, GetSlotId).WillByDefault(Return(1));

        pCapaQueryHandler = new MtcCapabilityQueryHandler(objMockContext);
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pCapaQueryHandler;
    }
};

TEST_F(MtcCapabilityQueryHandlerTest, HandleReturnsFailureIfICapabilitiesIsNull)
{
    EXPECT_EQ(IMS_FAILURE,
            pCapaQueryHandler->HandleIncomingCapabilityQuery(
                    &objMockCoreService, IMS_NULL, "", "", 0));
}

TEST_F(MtcCapabilityQueryHandlerTest, HandleReturnsFailureIfGetNextResponseReturnsNull)
{
    ON_CALL(objMockCapabilities, GetNextResponse).WillByDefault(Return(nullptr));

    EXPECT_EQ(IMS_FAILURE,
            pCapaQueryHandler->HandleIncomingCapabilityQuery(
                    &objMockCoreService, &objMockCapabilities, "", "", 0));
}

TEST_F(MtcCapabilityQueryHandlerTest, HandleReturnsSuccess)
{
    ON_CALL(*pConfigurationManager, IsUseCarrierSpecificContactHeaderForOptionsResponse)
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCapabilities, GetNextResponse).WillByDefault(Return(&objMockMessage));

    SipAddress objSipAddress("sip:1234@1.1.1.1");
    ON_CALL(objMockCoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objSipAddress));

    IpAddress objIpAddress(100);
    ON_CALL(objMockCoreService, GetIpAddress).WillByDefault(ReturnRef(objIpAddress));

    // TODO: because IMediaConfig is null in UT, setting SDP cannot be done by Enabler.
    EXPECT_EQ(IMS_SUCCESS,
            pCapaQueryHandler->HandleIncomingCapabilityQuery(&objMockCoreService,
                    &objMockCapabilities, "ims.app.mtc", "ims.service.mtc", 0));
}

TEST_F(MtcCapabilityQueryHandlerTest, HandleReturnsSuccessWithVzwConfig)
{
    ON_CALL(*pConfigurationManager, IsUseCarrierSpecificContactHeaderForOptionsResponse)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, IsVoiceQosPreconditionSupported)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockCapabilities, GetNextResponse).WillByDefault(Return(&objMockMessage));

    SipAddress objSipAddress("sip:1234@1.1.1.1");
    ON_CALL(objMockCoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objSipAddress));

    IpAddress objIpAddress(100);
    ON_CALL(objMockCoreService, GetIpAddress).WillByDefault(ReturnRef(objIpAddress));

    EXPECT_EQ(IMS_SUCCESS,
            pCapaQueryHandler->HandleIncomingCapabilityQuery(&objMockCoreService,
                    &objMockCapabilities, "ims.app.mtc", "ims.service.mtc", ImsAosFeature::VIDEO));
}

TEST_F(MtcCapabilityQueryHandlerTest, HandleReturnsSuccessWithVzwConfigWithoutVideo)
{
    ON_CALL(*pConfigurationManager, IsUseCarrierSpecificContactHeaderForOptionsResponse)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, IsVoiceQosPreconditionSupported)
            .WillByDefault(Return(IMS_FALSE));  // for line coverage.
    ON_CALL(objMockCapabilities, GetNextResponse).WillByDefault(Return(&objMockMessage));

    SipAddress objSipAddress("sip:1234@1.1.1.1");
    ON_CALL(objMockCoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objSipAddress));

    IpAddress objIpAddress(100);
    ON_CALL(objMockCoreService, GetIpAddress).WillByDefault(ReturnRef(objIpAddress));

    EXPECT_EQ(IMS_SUCCESS,
            pCapaQueryHandler->HandleIncomingCapabilityQuery(&objMockCoreService,
                    &objMockCapabilities, "ims.app.mtc", "ims.service.mtc", 0));
}

TEST_F(MtcCapabilityQueryHandlerTest, AdjustMessageDeleteTextFeatureTag)
{
    MockISipMessage objSipMessage;
    AString strContactHeader("<sip:1234@1.1.1.1>;text");
    AString strContactHeaderWithoutText("<sip:1234@1.1.1.1>");
    ON_CALL(objSipMessage, GetHeader(_, _, _)).WillByDefault(Return(strContactHeader));

    EXPECT_CALL(objSipMessage,
            SetHeader(
                    ISipHeader::CONTACT_NORMAL, strContactHeaderWithoutText, AString::ConstNull()))
            .Times(1);

    pCapaQueryHandler->MessageMediator_AdjustMessage(&objSipMessage, 0);
}

}  // namespace android
