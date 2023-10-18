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

#include "MockISession.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/message/MtcMessageMediator.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "media/MockIMedia.h"
#include "media/MockIMediaDescriptor.h"
#include "sipcore/ISipHeader.h"
#include "sipcore/MockISipMessage.h"
#include "utility/MessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

const AString CONTACT_FULL_MEDIA_TAGS(
        "<sip:+1234@help.me>;audio;video;text;+sip.instance=\"<urn:gsma:imei:0-0-0>\"");
const IMS_SINT32 MESSAGE_ANY = 0;

// AString
MATCHER_P(NotContains, str, "Contains unexpected string")
{
    return !arg.Contains(str);
}

class MtcMessageMediatorTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockISession objISession;
    MockIMtcSession objMtcSession;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MtcMessageMediator* pMessageMediator;
    MessageUtils objMessageUtils;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);

        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objISession));

        pMessageMediator = new MtcMessageMediator(objContext);
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pMessageMediator;
    }

    MockIMedia* CreateMedia(IMS_SINT32 eSdpMediaType)
    {
        // Currently these instances are not released
        MockIMedia* pMedia = new MockIMedia();
        MockIMediaDescriptor* pMediaDescriptor = new MockIMediaDescriptor();
        SdpMedia* pSdpMedia = new SdpMedia();
        pSdpMedia->SetType(eSdpMediaType);
        pSdpMedia->SetPort(1);

        ON_CALL(*pMedia, GetMediaDescriptor).WillByDefault(Return(pMediaDescriptor));
        ON_CALL(*pMediaDescriptor, GetMediaDescriptionEx).WillByDefault(Return(pSdpMedia));

        return pMedia;
    }
};

TEST_F(MtcMessageMediatorTest, AdjustMessageDoesNothingIfNoContactHeader)
{
    ON_CALL(*pConfigurationManager, IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType)
            .WillByDefault(Return(IMS_TRUE));

    MockISipMessage objMessage;
    ON_CALL(objMessage, GetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillByDefault(Return(CONTACT_FULL_MEDIA_TAGS));
    ON_CALL(objMessage, IsHeaderPresent(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMessage, SetHeader(ISipHeader::CONTACT_NORMAL, _, _)).Times(0);

    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);
}

TEST_F(MtcMessageMediatorTest, AdjustMessageDoesNothingIfConfigIsNotSet)
{
    ON_CALL(*pConfigurationManager, IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType)
            .WillByDefault(Return(IMS_FALSE));

    MockISipMessage objMessage;
    ON_CALL(objMessage, GetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillByDefault(Return(CONTACT_FULL_MEDIA_TAGS));
    ON_CALL(objMessage, IsHeaderPresent(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMessage, SetHeader(ISipHeader::CONTACT_NORMAL, _, _)).Times(0);

    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);
}

TEST_F(MtcMessageMediatorTest, AdjustMessageRemovesTextFeatureIfVtSdp)
{
    ON_CALL(*pConfigurationManager, IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType)
            .WillByDefault(Return(IMS_TRUE));

    MockISipMessage objMessage;
    ON_CALL(objMessage, GetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillByDefault(Return(CONTACT_FULL_MEDIA_TAGS));
    ON_CALL(objMessage, IsHeaderPresent(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(IMS_TRUE));

    ImsList<IMedia*> lstMedias;
    lstMedias.Append(CreateMedia(SdpMedia::TYPE_AUDIO));
    lstMedias.Append(CreateMedia(SdpMedia::TYPE_VIDEO));
    ON_CALL(objISession, GetMedia).WillByDefault(Return(lstMedias));

    EXPECT_CALL(objMessage, SetHeader(ISipHeader::CONTACT_NORMAL, NotContains("text"), _)).Times(1);

    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);
}

TEST_F(MtcMessageMediatorTest, AdjustMessageRemovesVideoFeatureIfRttSdp)
{
    ON_CALL(*pConfigurationManager, IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType)
            .WillByDefault(Return(IMS_TRUE));

    MockISipMessage objMessage;
    ON_CALL(objMessage, GetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillByDefault(Return(CONTACT_FULL_MEDIA_TAGS));
    ON_CALL(objMessage, IsHeaderPresent(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(IMS_TRUE));

    ImsList<IMedia*> lstMedias;
    lstMedias.Append(CreateMedia(SdpMedia::TYPE_AUDIO));
    lstMedias.Append(CreateMedia(SdpMedia::TYPE_TEXT));
    ON_CALL(objISession, GetMedia).WillByDefault(Return(lstMedias));

    EXPECT_CALL(objMessage, SetHeader(ISipHeader::CONTACT_NORMAL, NotContains("video"), _))
            .Times(1);

    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);
}

TEST_F(MtcMessageMediatorTest, AdjustMessageRemovesTextFeatureIfCallTypeHasChangedToVtFromRtt)
{
    ON_CALL(*pConfigurationManager, IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType)
            .WillByDefault(Return(IMS_TRUE));

    MockISipMessage objMessage;
    ON_CALL(objMessage, GetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillByDefault(Return(CONTACT_FULL_MEDIA_TAGS));
    ON_CALL(objMessage, IsHeaderPresent(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(IMS_TRUE));

    ImsList<IMedia*> lstRttMedias;
    lstRttMedias.Append(CreateMedia(SdpMedia::TYPE_AUDIO));
    lstRttMedias.Append(CreateMedia(SdpMedia::TYPE_TEXT));
    ImsList<IMedia*> lstVtMedias;
    lstVtMedias.Append(CreateMedia(SdpMedia::TYPE_AUDIO));
    lstVtMedias.Append(CreateMedia(SdpMedia::TYPE_VIDEO));

    ON_CALL(objISession, GetMedia).WillByDefault(Return(lstRttMedias));
    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);

    EXPECT_CALL(objMessage, SetHeader(ISipHeader::CONTACT_NORMAL, NotContains("text"), _)).Times(1);

    ON_CALL(objISession, GetMedia).WillByDefault(Return(lstVtMedias));
    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);
}

TEST_F(MtcMessageMediatorTest, AdjustMessageRemovesTextFeatureIfVtCallType)
{
    ON_CALL(*pConfigurationManager, IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType)
            .WillByDefault(Return(IMS_TRUE));

    MockISipMessage objMessage;
    ON_CALL(objMessage, GetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillByDefault(Return(CONTACT_FULL_MEDIA_TAGS));
    ON_CALL(objMessage, IsHeaderPresent(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(IMS_TRUE));

    ImsList<IMedia*> lstMedias;
    ON_CALL(objISession, GetMedia).WillByDefault(Return(lstMedias));

    ON_CALL(objMtcSession, GetCallType).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objMessage, SetHeader(ISipHeader::CONTACT_NORMAL, NotContains("text"), _)).Times(1);

    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);
}

TEST_F(MtcMessageMediatorTest, AdjustMessageSetOriginalContactIfUnknownCallType)
{
    ON_CALL(*pConfigurationManager, IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType)
            .WillByDefault(Return(IMS_TRUE));

    MockISipMessage objMessage;
    ON_CALL(objMessage, GetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillByDefault(Return(CONTACT_FULL_MEDIA_TAGS));
    ON_CALL(objMessage, IsHeaderPresent(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(IMS_TRUE));

    ImsList<IMedia*> lstMedias;
    ON_CALL(objISession, GetMedia).WillByDefault(Return(lstMedias));

    ON_CALL(objMtcSession, GetCallType).WillByDefault(Return(CallType::UNKNOWN));

    EXPECT_CALL(objMessage, SetHeader(ISipHeader::CONTACT_NORMAL, CONTACT_FULL_MEDIA_TAGS, _))
            .Times(1);

    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);
}
