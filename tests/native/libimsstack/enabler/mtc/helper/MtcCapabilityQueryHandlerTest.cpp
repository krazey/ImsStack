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

#include "../../../config/interface/common/MockICoreServiceConfig.h"
#include "../../../config/interface/common/MockIMediaConfig.h"
#include "AStringArray.h"
#include "ICapabilities.h"
#include "ImsAosParameter.h"
#include "MockIMtcContext.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockICapabilities.h"
#include "core/MockICoreService.h"
#include "core/MockIMessage.h"
#include "core/MockIMessageBodyPart.h"
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

LOCAL const AString AUDIO_M_LINE = "m=audio 1234 RTP/AVP 0";
LOCAL const AString VIDEO_M_LINE = "m=video 1234 RTP/AVP 0";
LOCAL const AString VIDEO_M_LINE_INVALID = "m=video";

MATCHER_P(HasAddSdpBodyPartFlag, query, "")
{
    IMS_BOOL bHasSdpBody = arg & ICapabilities::FLAG_ADD_SDP_BODY_PART;
    return query ? bHasSdpBody : !bHasSdpBody;
}

class MtcCapabilityQueryHandlerTest : public ::testing::Test
{
public:
    MockIMtcContext objMockContext;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MockICoreService objMockCoreService;
    MockIMessage objMockMessage;
    MockICapabilities objMockCapabilities;
    MockIMessageBodyPart objMockIMessageBodyPart;
    MockICoreServiceConfig objMockICoreServiceConfig;
    MockIMediaConfig objMockIMediaConfig;
    AStringArray objAudioMediaCapability;
    AStringArray objVideoMediaCapability;

    MtcCapabilityQueryHandler* pCapaQueryHandler;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objMockContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objMockContext, GetSlotId).WillByDefault(Return(1));
        AString strAny("Any String");
        ON_CALL(objMockICoreServiceConfig, GetMediaProfile).WillByDefault(ReturnRef(strAny));
        objAudioMediaCapability.AddElement(AUDIO_M_LINE);
        objVideoMediaCapability.AddElement(VIDEO_M_LINE);
        ON_CALL(objMockIMediaConfig, GetMediaProfile(_, ImsAosFeature::MMTEL))
                .WillByDefault(ReturnRef(objAudioMediaCapability));
        ON_CALL(objMockIMediaConfig, GetMediaProfile(_, ImsAosFeature::VIDEO))
                .WillByDefault(ReturnRef(objVideoMediaCapability));

        pCapaQueryHandler = new MtcCapabilityQueryHandler(
                objMockContext, &objMockICoreServiceConfig, &objMockIMediaConfig);
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
                    &objMockCoreService, IMS_NULL, ImsAosFeature::MMTEL));
}

TEST_F(MtcCapabilityQueryHandlerTest, HandleReturnsFailureIfGetNextResponseReturnsNull)
{
    ON_CALL(objMockCapabilities, GetNextResponse).WillByDefault(Return(nullptr));

    EXPECT_EQ(IMS_FAILURE,
            pCapaQueryHandler->HandleIncomingCapabilityQuery(
                    &objMockCoreService, &objMockCapabilities, ImsAosFeature::MMTEL));
}

TEST_F(MtcCapabilityQueryHandlerTest, HandleReturnsSuccessWithSdpBodyPartFlagIfNoMandatorySdpLine)
{
    ON_CALL(*pConfigurationManager, IsUseCarrierSpecificContactHeaderForOptionsResponse)
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCapabilities, GetNextResponse).WillByDefault(Return(&objMockMessage));
    EXPECT_CALL(objMockContext, GetSlotId).Times(0);
    EXPECT_CALL(objMockMessage, CreateBodyPart).Times(0);

    SipAddress objSipAddress("sip:1234@1.1.1.1");
    ON_CALL(objMockCoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objSipAddress));

    IpAddress objIpAddress(".");
    ON_CALL(objMockCoreService, GetIpAddress).WillByDefault(ReturnRef(objIpAddress));

    EXPECT_CALL(objMockCapabilities, Accept(HasAddSdpBodyPartFlag(IMS_TRUE)));

    EXPECT_EQ(IMS_SUCCESS,
            pCapaQueryHandler->HandleIncomingCapabilityQuery(
                    &objMockCoreService, &objMockCapabilities, ImsAosFeature::MMTEL));
}

TEST_F(MtcCapabilityQueryHandlerTest, HandleReturnsSuccessWithSdpBodyPartFlagIfNoAudioCapability)
{
    objAudioMediaCapability.RemoveAllElements();

    ON_CALL(*pConfigurationManager, IsUseCarrierSpecificContactHeaderForOptionsResponse)
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCapabilities, GetNextResponse).WillByDefault(Return(&objMockMessage));
    EXPECT_CALL(objMockMessage, CreateBodyPart).Times(0);

    SipAddress objSipAddress("sip:1234@1.1.1.1");
    ON_CALL(objMockCoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objSipAddress));

    IpAddress objIpAddress(100);
    ON_CALL(objMockCoreService, GetIpAddress).WillByDefault(ReturnRef(objIpAddress));

    EXPECT_CALL(objMockCapabilities, Accept(HasAddSdpBodyPartFlag(IMS_TRUE)));

    EXPECT_EQ(IMS_SUCCESS,
            pCapaQueryHandler->HandleIncomingCapabilityQuery(
                    &objMockCoreService, &objMockCapabilities, ImsAosFeature::MMTEL));
}

TEST_F(MtcCapabilityQueryHandlerTest,
        HandleReturnsSuccessWithSdpBodyPartFlagIfnoCorrectVideoCapability)
{
    objVideoMediaCapability.RemoveAllElements();
    objVideoMediaCapability.AddElement(VIDEO_M_LINE_INVALID);

    ON_CALL(*pConfigurationManager, IsUseCarrierSpecificContactHeaderForOptionsResponse)
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCapabilities, GetNextResponse).WillByDefault(Return(&objMockMessage));
    EXPECT_CALL(objMockMessage, CreateBodyPart).Times(0);

    SipAddress objSipAddress("sip:1234@1.1.1.1");
    ON_CALL(objMockCoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objSipAddress));

    IpAddress objIpAddress(100);
    ON_CALL(objMockCoreService, GetIpAddress).WillByDefault(ReturnRef(objIpAddress));

    EXPECT_CALL(objMockCapabilities, Accept(HasAddSdpBodyPartFlag(IMS_TRUE)));

    EXPECT_EQ(IMS_SUCCESS,
            pCapaQueryHandler->HandleIncomingCapabilityQuery(
                    &objMockCoreService, &objMockCapabilities, ImsAosFeature::VIDEO));
}

TEST_F(MtcCapabilityQueryHandlerTest, HandleReturnsSuccessWithoutSdpBodyPartFlag)
{
    // To pass of Video capability decode.
    objVideoMediaCapability.RemoveAllElements();

    ON_CALL(*pConfigurationManager, IsUseCarrierSpecificContactHeaderForOptionsResponse)
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCapabilities, GetNextResponse).WillByDefault(Return(&objMockMessage));
    EXPECT_CALL(objMockMessage, CreateBodyPart).Times(1).WillOnce(Return(&objMockIMessageBodyPart));

    SipAddress objSipAddress("sip:1234@1.1.1.1");
    ON_CALL(objMockCoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objSipAddress));

    IpAddress objIpAddress(100);
    ON_CALL(objMockCoreService, GetIpAddress).WillByDefault(ReturnRef(objIpAddress));

    EXPECT_CALL(objMockCapabilities, Accept(HasAddSdpBodyPartFlag(IMS_FALSE)));

    EXPECT_EQ(IMS_SUCCESS,
            pCapaQueryHandler->HandleIncomingCapabilityQuery(
                    &objMockCoreService, &objMockCapabilities, ImsAosFeature::VIDEO));
}

TEST_F(MtcCapabilityQueryHandlerTest, HandleReturnsSuccessWithVzwConfig)
{
    ON_CALL(*pConfigurationManager, IsUseCarrierSpecificContactHeaderForOptionsResponse)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, IsVoiceQosPreconditionSupported)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockCapabilities, GetNextResponse).WillByDefault(Return(&objMockMessage));
    EXPECT_CALL(objMockCapabilities, SetMessageMediator).Times(2);
    ON_CALL(objMockMessage, CreateBodyPart).WillByDefault(Return(&objMockIMessageBodyPart));

    SipAddress objSipAddress("sip:1234@1.1.1.1");
    ON_CALL(objMockCoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objSipAddress));

    IpAddress objIpAddress(100);
    ON_CALL(objMockCoreService, GetIpAddress).WillByDefault(ReturnRef(objIpAddress));

    EXPECT_EQ(IMS_SUCCESS,
            pCapaQueryHandler->HandleIncomingCapabilityQuery(
                    &objMockCoreService, &objMockCapabilities, ImsAosFeature::VIDEO));
}

TEST_F(MtcCapabilityQueryHandlerTest, HandleReturnsSuccessWithVzwConfigWithoutVideo)
{
    ON_CALL(*pConfigurationManager, IsUseCarrierSpecificContactHeaderForOptionsResponse)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, IsVoiceQosPreconditionSupported)
            .WillByDefault(Return(IMS_FALSE));  // for line coverage.
    ON_CALL(objMockCapabilities, GetNextResponse).WillByDefault(Return(&objMockMessage));
    EXPECT_CALL(objMockCapabilities, SetMessageMediator).Times(2);
    ON_CALL(objMockMessage, CreateBodyPart).WillByDefault(Return(&objMockIMessageBodyPart));

    SipAddress objSipAddress("sip:1234@1.1.1.1");
    ON_CALL(objMockCoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objSipAddress));

    IpAddress objIpAddress(100);
    ON_CALL(objMockCoreService, GetIpAddress).WillByDefault(ReturnRef(objIpAddress));

    EXPECT_EQ(IMS_SUCCESS,
            pCapaQueryHandler->HandleIncomingCapabilityQuery(
                    &objMockCoreService, &objMockCapabilities, ImsAosFeature::MMTEL));
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
