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

#include "IImsAosInfo.h"
#include "ISipHeader.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "MockISipMessage.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/message/MtcMessageMediator.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include "media/MockIMedia.h"
#include "media/MockIMediaDescriptor.h"
#include "utility/MessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

const AString CONTACT_FULL_MEDIA_TAGS(
        "<sip:+1234@help.me>;audio;video;text;+sip.instance=\"<urn:gsma:imei:0-0-0>\"");
const AString CONTACT_SOS_PARAMETER(
        "<sip:+1234@help.me;sos>;audio;+sip.instance=\"<urn:gsma:imei:0-0-0>\"");
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
    MockIMtcService objService;
    MockIMtcAosConnector objAosConnector;
    MockISession objISession;
    MockIMtcSession objMtcSession;
    MockMtcConfigurationProxy objConfiguration;
    MtcMessageMediator* pMessageMediator;
    MessageUtils objMessageUtils;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfiguration));
        ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objContext, GetAosConnector).WillByDefault(Return(&objAosConnector));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objISession));

        pMessageMediator = new MtcMessageMediator(objContext);

        ON_CALL(objConfiguration,
                GetBoolean(ConfigVt::
                                KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL))
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(objConfiguration, GetBoolean(ConfigVoice::KEY_ALLOW_SOS_PARAM_IN_CONTACT_BOOL))
                .WillByDefault(Return(IMS_TRUE));

        ImsVector<AString> lstContactAddress;
        lstContactAddress.Add("");
        lstContactAddress.Add("");
        lstContactAddress.Add("");
        lstContactAddress.Add("");
        ON_CALL(objConfiguration,
                GetStringArray(ConfigEmergency::KEY_CONTACT_HEADER_ADDRESS_IN_INVITE_STRING_ARRAY))
                .WillByDefault(Return(lstContactAddress));
    }

    virtual void TearDown() override { delete pMessageMediator; }

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
    ON_CALL(objConfiguration,
            GetBoolean(ConfigVt::
                            KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfiguration, GetBoolean(ConfigVoice::KEY_ALLOW_SOS_PARAM_IN_CONTACT_BOOL))
            .WillByDefault(Return(IMS_FALSE));

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
    ON_CALL(objConfiguration,
            GetBoolean(ConfigVt::
                            KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL))
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
    ON_CALL(objConfiguration,
            GetBoolean(ConfigVt::
                            KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL))
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
    ON_CALL(objConfiguration,
            GetBoolean(ConfigVt::
                            KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL))
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
    ON_CALL(objConfiguration,
            GetBoolean(ConfigVt::
                            KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL))
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
    ON_CALL(objConfiguration,
            GetBoolean(ConfigVt::
                            KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL))
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

TEST_F(MtcMessageMediatorTest, AdjustMessageDoesNotFormatContactAddressForNormalService)
{
    ImsVector<AString> lstContactAddress;
    lstContactAddress.Add("sip:normal@#IP#:#PORT#");
    lstContactAddress.Add("sip:admin@#IP#:#PORT#");
    lstContactAddress.Add("sip:internal@#IP#:#PORT#");
    lstContactAddress.Add("sip:nouicc@#IP#:#PORT#");
    ON_CALL(objConfiguration,
            GetStringArray(ConfigEmergency::KEY_CONTACT_HEADER_ADDRESS_IN_INVITE_STRING_ARRAY))
            .WillByDefault(Return(lstContactAddress));

    ON_CALL(objService, GetServiceType).WillByDefault(Return(ServiceType::NORMAL));

    const AString strLocalIp = "::1";
    const IMS_UINT32 nLocalPort = 5060;
    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(strLocalIp));
    ON_CALL(objAosConnector, GetLocalPort).WillByDefault(Return(nLocalPort));

    MockISipMessage objMessage;
    ON_CALL(objMessage, GetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillByDefault(Return(CONTACT_SOS_PARAMETER));
    ON_CALL(objMessage, IsHeaderPresent(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));

    EXPECT_CALL(objMessage, SetHeader(ISipHeader::CONTACT_NORMAL, _, _)).Times(0);
    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);
}

TEST_F(MtcMessageMediatorTest, AdjustMessageDoesNotFormatContactAddressIfAosConnectorIsNull)
{
    ImsVector<AString> lstContactAddress;
    lstContactAddress.Add("sip:normal@#IP#:#PORT#");
    lstContactAddress.Add("sip:admin@#IP#:#PORT#");
    lstContactAddress.Add("sip:internal@#IP#:#PORT#");
    lstContactAddress.Add("sip:nouicc@#IP#:#PORT#");
    ON_CALL(objConfiguration,
            GetStringArray(ConfigEmergency::KEY_CONTACT_HEADER_ADDRESS_IN_INVITE_STRING_ARRAY))
            .WillByDefault(Return(lstContactAddress));

    ON_CALL(objService, GetServiceType).WillByDefault(Return(ServiceType::EMERGENCY));
    ON_CALL(objContext, GetAosConnector).WillByDefault(Return(IMS_NULL));

    MockISipMessage objMessage;
    ON_CALL(objMessage, GetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillByDefault(Return(CONTACT_SOS_PARAMETER));
    ON_CALL(objMessage, IsHeaderPresent(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));

    EXPECT_CALL(objMessage, SetHeader(ISipHeader::CONTACT_NORMAL, _, _)).Times(0);
    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);
}

TEST_F(MtcMessageMediatorTest, AdjustMessageFormatsContactAddressWithIpv4Address)
{
    ImsVector<AString> lstContactAddress;
    lstContactAddress.Add("sip:normal@#IP#:#PORT#");
    lstContactAddress.Add("sip:admin@#IP#:#PORT#");
    lstContactAddress.Add("sip:internal@#IP#:#PORT#");
    lstContactAddress.Add("sip:nouicc@#IP#:#PORT#");
    ON_CALL(objConfiguration,
            GetStringArray(ConfigEmergency::KEY_CONTACT_HEADER_ADDRESS_IN_INVITE_STRING_ARRAY))
            .WillByDefault(Return(lstContactAddress));

    ON_CALL(objService, GetServiceType).WillByDefault(Return(ServiceType::EMERGENCY));

    const AString strLocalIp = "127.0.0.1";
    const IMS_UINT32 nLocalPort = 5060;
    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(strLocalIp));
    ON_CALL(objAosConnector, GetLocalPort).WillByDefault(Return(nLocalPort));

    MockISipMessage objMessage;
    ON_CALL(objMessage, GetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillByDefault(Return(CONTACT_SOS_PARAMETER));
    ON_CALL(objMessage, IsHeaderPresent(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));
    EXPECT_CALL(objMessage,
            SetHeader(ISipHeader::CONTACT_NORMAL,
                    AString("<sip:normal@127.0.0.1:5060;sos>;audio;+sip.instance=\"<urn:gsma:imei:"
                            "0-0-0>\""),
                    _));
    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_ADMIN));
    EXPECT_CALL(objMessage,
            SetHeader(ISipHeader::CONTACT_NORMAL,
                    AString("<sip:admin@127.0.0.1:5060;sos>;audio;+sip.instance=\"<urn:gsma:imei:0-"
                            "0-0>\""),
                    _));
    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_INTERNAL));
    EXPECT_CALL(objMessage,
            SetHeader(ISipHeader::CONTACT_NORMAL,
                    AString("<sip:internal@127.0.0.1:5060;sos>;audio;+sip.instance=\"<urn:gsma:"
                            "imei:0-0-0>\""),
                    _));
    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NOUICC));
    EXPECT_CALL(objMessage,
            SetHeader(ISipHeader::CONTACT_NORMAL,
                    AString("<sip:nouicc@127.0.0.1:5060;sos>;audio;+sip.instance=\"<urn:gsma:imei:"
                            "0-0-0>\""),
                    _));
    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);
}

TEST_F(MtcMessageMediatorTest, AdjustMessageFormatsContactAddressWithIpv6Address)
{
    ImsVector<AString> lstContactAddress;
    lstContactAddress.Add("sip:normal@#IP#:#PORT#");
    lstContactAddress.Add("sip:admin@#IP#:#PORT#");
    lstContactAddress.Add("sip:internal@#IP#:#PORT#");
    lstContactAddress.Add("sip:nouicc@#IP#:#PORT#");
    ON_CALL(objConfiguration,
            GetStringArray(ConfigEmergency::KEY_CONTACT_HEADER_ADDRESS_IN_INVITE_STRING_ARRAY))
            .WillByDefault(Return(lstContactAddress));

    ON_CALL(objService, GetServiceType).WillByDefault(Return(ServiceType::EMERGENCY));

    const AString strLocalIp = "::1";
    const IMS_UINT32 nLocalPort = 5060;
    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(strLocalIp));
    ON_CALL(objAosConnector, GetLocalPort).WillByDefault(Return(nLocalPort));

    MockISipMessage objMessage;
    ON_CALL(objMessage, GetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillByDefault(Return(CONTACT_SOS_PARAMETER));
    ON_CALL(objMessage, IsHeaderPresent(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));
    EXPECT_CALL(objMessage,
            SetHeader(ISipHeader::CONTACT_NORMAL,
                    AString("<sip:normal@[::1]:5060;sos>;audio;+sip.instance=\"<urn:gsma:imei:0-0-"
                            "0>\""),
                    _));
    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_ADMIN));
    EXPECT_CALL(objMessage,
            SetHeader(ISipHeader::CONTACT_NORMAL,
                    AString("<sip:admin@[::1]:5060;sos>;audio;+sip.instance=\"<urn:gsma:imei:0-0-0>"
                            "\""),
                    _));
    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_INTERNAL));
    EXPECT_CALL(objMessage,
            SetHeader(ISipHeader::CONTACT_NORMAL,
                    AString("<sip:internal@[::1]:5060;sos>;audio;+sip.instance=\"<urn:gsma:imei:0-"
                            "0-0>\""),
                    _));
    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NOUICC));
    EXPECT_CALL(objMessage,
            SetHeader(ISipHeader::CONTACT_NORMAL,
                    AString("<sip:nouicc@[::1]:5060;sos>;audio;+sip.instance=\"<urn:gsma:imei:0-0-"
                            "0>\""),
                    _));
    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);
}

TEST_F(MtcMessageMediatorTest, AdjustMessageRemovesSosParameter)
{
    ON_CALL(objConfiguration, GetBoolean(ConfigVoice::KEY_ALLOW_SOS_PARAM_IN_CONTACT_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    MockISipMessage objMessage;
    ON_CALL(objMessage, GetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillByDefault(Return(CONTACT_SOS_PARAMETER));
    ON_CALL(objMessage, IsHeaderPresent(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(objMtcSession, GetCallType).WillByDefault(Return(CallType::UNKNOWN));

    EXPECT_CALL(objMessage,
            SetHeader(ISipHeader::CONTACT_NORMAL,
                    AString("<sip:+1234@help.me>;audio;+sip.instance=\"<urn:gsma:imei:0-0-0>\""),
                    _))
            .Times(1);

    pMessageMediator->MessageMediator_AdjustMessage(&objMessage, MESSAGE_ANY);
}
