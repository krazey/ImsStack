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
#include <gmock/gmock.h>
#include "options/UceOptions.h"
#include "IMessage.h"
#include "MockIJniEnabler.h"
#include "MockIUceJniThread.h"
#include "JniEnablerConnector.h"
#include "MockICoreService.h"
#include "MockICapabilities.h"
#include "MockIMessage.h"
#include "MockISipMessage.h"
#include "SipAddress.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

__IMS_TRACE_TAG_UCE__;

class TestUceOptions : public UceOptions
{
public:
public:
    TestUceOptions(ICoreService* piCoreService, ICapabilities* piCapabilities) :
            UceOptions(AString("UceOptionsManager"), piCoreService, piCapabilities, 1, IMS_TRUE, 0)
    {
    }
    virtual ~TestUceOptions() override {}

    IMS_UINT32 getKey() const { return m_nKey; }

    IMS_BOOL getSendingRequest() const { return m_bIsSendingRequest; }

    void setSendingRequest(IMS_BOOL value) { m_bIsSendingRequest = value; }
    IMS_BOOL handleOptionsRequest(ICapabilities* piCapabilities, IMS_UINT32 ownCapabilities)
    {
        return HandleOptionsRequest(piCapabilities, ownCapabilities);
    }
    void capaDeliverd(ICapabilities* piCapabilities) { CapabilityQueryDelivered(piCapabilities); }
    void capaDeliveryFailed(ICapabilities* piCapabilities)
    {
        CapabilityQueryDeliveryFailed(piCapabilities);
    }
};

class UceOptionsTest : public ::testing::Test
{
public:
    TestUceOptions* pUceOptions;
    MockICoreService objMockICoreService;
    MockICapabilities objMockICapabilities;
    MockIJniEnabler objMockJniEnabler;
    MockIUceJniThread objMockIUceJniThread;
    MockIMessage objMockIMessage;

protected:
    virtual void SetUp() override
    {
        pUceOptions = new TestUceOptions(&objMockICoreService, &objMockICapabilities);
        ASSERT_TRUE(pUceOptions != nullptr);
        ON_CALL(objMockJniEnabler, GetJniThread()).WillByDefault(Return(&objMockIUceJniThread));
        ON_CALL(objMockICapabilities, GetNextRequest).WillByDefault(Return(&objMockIMessage));
        ON_CALL(objMockICapabilities, GetNextResponse).WillByDefault(Return(&objMockIMessage));
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::UCE, &objMockJniEnabler);
    }

    virtual void TearDown() override
    {
        if (pUceOptions)
        {
            delete pUceOptions;
        }
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::UCE, IMS_NULL);
    }
};

TEST_F(UceOptionsTest, SendOptionsRequestWithNoCoreService)
{
    IMS_TRACE_D("SendOptionsRequestWithNoCoreService", 0, 0, 0);
    EXPECT_CALL(objMockIUceJniThread, OptionsErrorInd(_, _)).Times(1);
    TestUceOptions* pUceOptionsNoSvc = new TestUceOptions(IMS_NULL, &objMockICapabilities);
    EXPECT_FALSE(pUceOptionsNoSvc->SendOptionsRequest("RemoteURI", 0));
    delete pUceOptionsNoSvc;
}

TEST_F(UceOptionsTest, SendOptionsRequestWithNoCapabilities)
{
    IMS_TRACE_D("SendOptionsRequestWithNoCapabilities", 0, 0, 0);

    AString strFrom = "From";
    ON_CALL(objMockICoreService, CreateCapabilities(_, _)).WillByDefault(ReturnNull());

    EXPECT_CALL(objMockICoreService, GetUserIdentity(_))
            .Times(1)
            .WillRepeatedly(ReturnRef(strFrom));
    EXPECT_CALL(objMockIUceJniThread, OptionsErrorInd(_, _)).Times(1);

    EXPECT_FALSE(pUceOptions->SendOptionsRequest("RemoteURI", 0));

    EXPECT_EQ(pUceOptions->getKey(), 0);
}

TEST_F(UceOptionsTest, SendOptionsRequestWithNoMessage)
{
    IMS_TRACE_D("SendOptionsRequestWithNoMessage", 0, 0, 0);

    ON_CALL(objMockICapabilities, GetNextRequest()).WillByDefault(ReturnNull());

    EXPECT_CALL(objMockICapabilities, SetListener(_)).Times(1);
    EXPECT_CALL(objMockIUceJniThread, OptionsErrorInd(_, _)).Times(1);

    EXPECT_FALSE(pUceOptions->handleOptionsRequest(&objMockICapabilities, 0));

    EXPECT_EQ(pUceOptions->getKey(), 0);
}

TEST_F(UceOptionsTest, SendOptionsRequestWithNoSipMessage)
{
    IMS_TRACE_D("SendOptionsRequestWithNoSipMessage", 0, 0, 0);

    ON_CALL(objMockIMessage, GetMessage()).WillByDefault(ReturnNull());

    EXPECT_CALL(objMockIUceJniThread, OptionsErrorInd(_, _)).Times(1);

    EXPECT_FALSE(pUceOptions->handleOptionsRequest(&objMockICapabilities, 0));

    EXPECT_EQ(pUceOptions->getKey(), 0);
}

TEST_F(UceOptionsTest, SendOptionsRequest)
{
    IMS_TRACE_D("SendOptionsRequest", 0, 0, 0);
    MockISipMessage objMockISipMessage;

    ON_CALL(objMockICoreService, GetPublicGruu).WillByDefault(ReturnNull());
    ON_CALL(objMockIMessage, GetMessage()).WillByDefault(Return(&objMockISipMessage));

    SipAddress objSipAddress("sip:1111@1.1.1.1");
    EXPECT_CALL(objMockICoreService, GetContactAddress)
            .Times(1)
            .WillRepeatedly(ReturnRef(objSipAddress));

    EXPECT_CALL(objMockISipMessage, SetHeader).Times(1);
    EXPECT_CALL(objMockICapabilities, QueryCapabilities(ICapabilities::FLAG_ADD_CONTACT_HEADER))
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(objMockIUceJniThread, OptionsErrorInd(_, _)).Times(1);

    EXPECT_FALSE(pUceOptions->handleOptionsRequest(&objMockICapabilities, 0));
    EXPECT_EQ(pUceOptions->getKey(), 0);
}

TEST_F(UceOptionsTest, SendOptionsResponseWithNoCapa)
{
    IMS_TRACE_D("SendOptionsResponseWithNoCapa", 0, 0, 0);
    TestUceOptions* pUceOptionsNoCapa = new TestUceOptions(&objMockICoreService, IMS_NULL);

    EXPECT_FALSE(pUceOptionsNoCapa->SendOptionsResponse(404, "not found", 0));
    delete pUceOptionsNoCapa;
}

TEST_F(UceOptionsTest, SendOptionsResponseWithNoSipMessage)
{
    IMS_TRACE_D("SendOptionsResponseWithNoSipMessage", 0, 0, 0);
    ON_CALL(objMockIMessage, GetMessage()).WillByDefault(ReturnNull());

    EXPECT_FALSE(pUceOptions->SendOptionsResponse(200, "OK", 0));
}

TEST_F(UceOptionsTest, SendOptionsResponseWithAccept)
{
    IMS_TRACE_D("SendOptionsResponse", 0, 0, 0);
    MockISipMessage objMockISipMessage;

    ON_CALL(objMockIMessage, GetMessage()).WillByDefault(Return(&objMockISipMessage));

    SipAddress objSipAddress("sip:1111@1.1.1.1");
    EXPECT_CALL(objMockICoreService, GetContactAddress)
            .Times(1)
            .WillRepeatedly(ReturnRef(objSipAddress));

    EXPECT_CALL(objMockISipMessage, SetHeader).Times(1);
    EXPECT_CALL(objMockICapabilities, Accept).Times(1).WillOnce(Return(IMS_FAILURE));

    EXPECT_FALSE(pUceOptions->SendOptionsResponse(200, "OK", 0));
}

TEST_F(UceOptionsTest, SendOptionsResponseWithReject)
{
    IMS_TRACE_D("SendOptionsResponse", 0, 0, 0);

    EXPECT_CALL(objMockICapabilities, Reject(_, _)).Times(1).WillOnce(Return(IMS_FAILURE));

    EXPECT_FALSE(pUceOptions->SendOptionsResponse(404, "not found", 0));
}

TEST_F(UceOptionsTest, AoSDisconnected)
{
    IMS_TRACE_D("AoSDisconnected", 0, 0, 0);
    pUceOptions->setSendingRequest(IMS_TRUE);

    EXPECT_CALL(objMockIUceJniThread, OptionsErrorInd(_, _)).Times(1);

    pUceOptions->AoSDisconnected();

    EXPECT_EQ(pUceOptions->getKey(), 0);
    EXPECT_EQ(pUceOptions->getSendingRequest(), IMS_FALSE);
}

TEST_F(UceOptionsTest, GetCapability)
{
    IMS_TRACE_D("GetCapability", 0, 0, 0);
    ImsList<AString> objContactList = ImsList<AString>();

    EXPECT_EQ(pUceOptions->GetCapability(objContactList), 0);

    objContactList.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp");
    objContactList.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel");
    objContactList.Append("video");
    objContactList.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg");
    objContactList.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg");
    objContactList.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session");
    objContactList.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im");
    objContactList.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fullsfgroupchat");
    objContactList.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftthumb");
    objContactList.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftstandfw");
    objContactList.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp");
    objContactList.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush");
    objContactList.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.ft");
    objContactList.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedmap");
    objContactList.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedsketch");
    objContactList.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callcomposer");
    objContactList.Append("+g.gsma.callcomposer");
    objContactList.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callunanswered");
    objContactList.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftsms");
    objContactList.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geosms");
    objContactList.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot");
    objContactList.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot.sa");
    objContactList.Append("+g.gsma.rcs.isbot");
    objContactList.Append("+g.gsma.rcs.botversion=\"#=1\"");
    objContactList.Append("+g.gsma.rcs.botversion=\"#=1,#=2\"");
    EXPECT_EQ(pUceOptions->GetCapability(objContactList),
            UceOptions::FEATURE_TAG_DP | UceOptions::FEATURE_TAG_IPCALL_VOICE |
                    UceOptions::FEATURE_TAG_IPCALL_VIDEO | UceOptions::FEATURE_TAG_PAGER_MESSAGING |
                    UceOptions::FEATURE_TAG_LARGE_MESSAGING | UceOptions::FEATURE_TAG_CPM_CHAT |
                    UceOptions::FEATURE_TAG_SIMPLE_IM |
                    UceOptions::FEATURE_TAG_STORE_FORWARD_GROUP_CHAT |
                    UceOptions::FEATURE_TAG_FT_THUMBNAIL |
                    UceOptions::FEATURE_TAG_FT_STORE_FORWARD | UceOptions::FEATURE_TAG_FT_HTTP |
                    UceOptions::FEATURE_TAG_GEOLOCATION_PUSH | UceOptions::FEATURE_TAG_FT |
                    UceOptions::FEATURE_TAG_SHARED_MAP | UceOptions::FEATURE_TAG_SHARED_SKETCH |
                    UceOptions::FEATURE_TAG_CALL_COMPOSER |
                    UceOptions::FEATURE_TAG_CALL_COMPOSER_MMTEL |
                    UceOptions::FEATURE_TAG_POST_CALL | UceOptions::FEATURE_TAG_FT_SMS |
                    UceOptions::FEATURE_TAG_GEOLOCATION_SMS |
                    UceOptions::FEATURE_TAG_CHATBOT_SESSION | UceOptions::FEATURE_TAG_CHATBOT_SA |
                    UceOptions::FEATURE_TAG_IS_BOT | UceOptions::FEATURE_TAG_CHATBOT_VERSION_V1 |
                    UceOptions::FEATURE_TAG_CHATBOT_VERSION_V2);
}

TEST_F(UceOptionsTest, SetIARIFeatureTag)
{
    IMS_TRACE_D("SetIARIFeatureTag", 0, 0, 0);
    AString strIari = AString::ConstEmpty();
    pUceOptions->SetIARIFeatureTag(UceOptions::FEATURE_TAG_DP, strIari);
    EXPECT_STREQ(strIari.GetStr(), "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp");

    strIari = AString::ConstEmpty();
    pUceOptions->SetIARIFeatureTag(UceOptions::FEATURE_TAG_DP | UceOptions::FEATURE_TAG_SIMPLE_IM |
                    UceOptions::FEATURE_TAG_STORE_FORWARD_GROUP_CHAT,
            strIari);
    EXPECT_STREQ(strIari.GetStr(),
            "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp,"
            "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im,"
            "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fullsfgroupchat");

    strIari = AString::ConstEmpty();
    pUceOptions->SetIARIFeatureTag(UceOptions::FEATURE_TAG_FT_THUMBNAIL |
                    UceOptions::FEATURE_TAG_FT_STORE_FORWARD | UceOptions::FEATURE_TAG_FT_HTTP,
            strIari);
    EXPECT_STREQ(strIari.GetStr(),
            "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftthumb,"
            "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftstandfw,"
            "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp");

    strIari = AString::ConstEmpty();
    pUceOptions->SetIARIFeatureTag(UceOptions::FEATURE_TAG_GEOLOCATION_PUSH |
                    UceOptions::FEATURE_TAG_FT | UceOptions::FEATURE_TAG_FT_SMS |
                    UceOptions::FEATURE_TAG_GEOLOCATION_SMS |
                    UceOptions::FEATURE_TAG_CHATBOT_SESSION | UceOptions::FEATURE_TAG_CHATBOT_SA,
            strIari);
    EXPECT_STREQ(strIari.GetStr(),
            "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush,"
            "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.ft,"
            "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftsms,"
            "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geosms,"
            "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot,"
            "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot.sa");
}

TEST_F(UceOptionsTest, SetICSIFeatureTag)
{
    IMS_TRACE_D("SetIARIFeatureTag", 0, 0, 0);
    AString strIcsi = AString::ConstEmpty();
    pUceOptions->SetICSIFeatureTag(
            UceOptions::FEATURE_TAG_IPCALL_VOICE | UceOptions::FEATURE_TAG_IPCALL_VIDEO, strIcsi);
    EXPECT_STREQ(strIcsi.GetStr(), "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel");

    strIcsi = AString::ConstEmpty();
    pUceOptions->SetICSIFeatureTag(UceOptions::FEATURE_TAG_PAGER_MESSAGING |
                    UceOptions::FEATURE_TAG_LARGE_MESSAGING | UceOptions::FEATURE_TAG_CPM_CHAT |
                    UceOptions::FEATURE_TAG_SHARED_MAP | UceOptions::FEATURE_TAG_SHARED_SKETCH |
                    UceOptions::FEATURE_TAG_CALL_COMPOSER | UceOptions::FEATURE_TAG_POST_CALL,
            strIcsi);
    EXPECT_STREQ(strIcsi.GetStr(),
            "urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg,"
            "urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg,"
            "urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session,"
            "urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedmap,"
            "urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedsketch,"
            "urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callcomposer,"
            "urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callunanswered");
}

TEST_F(UceOptionsTest, SetNoTypeFeatureTag)
{
    IMS_TRACE_D("SetNoTypeFeatureTag", 0, 0, 0);
    AString strTag = AString::ConstEmpty();
    pUceOptions->SetNoTypeFeatureTag(UceOptions::FEATURE_TAG_IPCALL_VIDEO, strTag);
    EXPECT_STREQ(strTag.GetStr(), "video");

    strTag = AString::ConstEmpty();
    pUceOptions->SetNoTypeFeatureTag(UceOptions::FEATURE_TAG_CALL_COMPOSER_MMTEL |
                    UceOptions::FEATURE_TAG_IS_BOT | UceOptions::FEATURE_TAG_CHATBOT_VERSION_V1 |
                    UceOptions::FEATURE_TAG_CHATBOT_VERSION_V2,
            strTag);
    EXPECT_STREQ(strTag.GetStr(),
            "+g.gsma.callcomposer;"
            "+g.gsma.rcs.isbot;"
            "+g.gsma.rcs.botversion=\"#=1\";"
            "+g.gsma.rcs.botversion=\"#=1,#=2\"");
}

TEST_F(UceOptionsTest, capaDeliverd)
{
    IMS_TRACE_D("capaDeliverd", 0, 0, 0);
    MockISipMessage objMockISipMessage;

    ON_CALL(objMockICapabilities, GetPreviousResponse(IMessage::CAPABILITIES_QUERY))
            .WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage()).WillByDefault(Return(&objMockISipMessage));
    ImsList<AString> objHeaders;
    objHeaders.Clear();
    AString reason = "OK";

    ON_CALL(objMockISipMessage, GetHeaders).WillByDefault(Return(objHeaders));
    ON_CALL(objMockIMessage, GetStatusCode).WillByDefault(Return(200));
    ON_CALL(objMockIMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));

    EXPECT_CALL(objMockIUceJniThread, OptionsResponseInd(_, _, _, _)).Times(1);

    pUceOptions->capaDeliverd(&objMockICapabilities);
}

TEST_F(UceOptionsTest, capaDeliverdWithNoCapa)
{
    IMS_TRACE_D("capaDeliverdWithNoCapa", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, OptionsErrorInd(_, _)).Times(1);
    pUceOptions->capaDeliverd(IMS_NULL);
}

TEST_F(UceOptionsTest, capaDeliverdWithNoMessage)
{
    IMS_TRACE_D("capaDeliverdWithNoMessage", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, OptionsErrorInd(_, _)).Times(1);

    ON_CALL(objMockICapabilities, GetPreviousResponse(IMessage::CAPABILITIES_QUERY))
            .WillByDefault(ReturnNull());

    pUceOptions->capaDeliverd(&objMockICapabilities);
}

TEST_F(UceOptionsTest, capaDeliverdWithNoSipMessage)
{
    IMS_TRACE_D("capaDeliverdWithNoSipMessage", 0, 0, 0);

    ON_CALL(objMockICapabilities, GetPreviousResponse(IMessage::CAPABILITIES_QUERY))
            .WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage()).WillByDefault(ReturnNull());

    EXPECT_CALL(objMockIUceJniThread, OptionsErrorInd(_, _)).Times(1);

    pUceOptions->capaDeliverd(&objMockICapabilities);
}

TEST_F(UceOptionsTest, capaDeliveryFailedWithNoCapa)
{
    IMS_TRACE_D("capaDeliveryFailedWithNoCapa", 0, 0, 0);
    EXPECT_CALL(objMockIUceJniThread, OptionsResponseInd(_, _, _, _)).Times(1);

    pUceOptions->capaDeliveryFailed(IMS_NULL);
}

TEST_F(UceOptionsTest, capaDeliveryFailedWithNoMessage)
{
    IMS_TRACE_D("capaDeliveryFailedWithNoMessage", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, OptionsResponseInd(_, _, _, _)).Times(1);

    ON_CALL(objMockICapabilities, GetPreviousResponse(IMessage::CAPABILITIES_QUERY))
            .WillByDefault(ReturnNull());
    pUceOptions->capaDeliveryFailed(&objMockICapabilities);
}

TEST_F(UceOptionsTest, capaDeliveryFailed)
{
    IMS_TRACE_D("capaDeliveryFailed", 0, 0, 0);

    ON_CALL(objMockICapabilities, GetPreviousResponse(IMessage::CAPABILITIES_QUERY))
            .WillByDefault(Return(&objMockIMessage));
    AString reason = "Not Found";
    ON_CALL(objMockIMessage, GetStatusCode).WillByDefault(Return(404));
    ON_CALL(objMockIMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));

    EXPECT_CALL(objMockIUceJniThread, OptionsResponseInd(_, _, _, _)).Times(1);

    pUceOptions->capaDeliveryFailed(&objMockICapabilities);
}