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

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

__IMS_TRACE_TAG_USER_DECL__("UCE");

class TestUceOptions : public UceOptions
{
public:
public:
    TestUceOptions(ICoreService* piCoreService, ICapabilities* piCapabilities) :
            UceOptions(AString("UceOptionsManager"), piCoreService, piCapabilities, 1, IMS_TRUE, 0)
    {
    }
    virtual ~TestUceOptions() {}

    IMS_UINT32 GetKey() const { return m_nKey; }

    IMS_BOOL GetSendingRequest() const { return m_bIsSendingRequest; }

    void SetSendingRequest(IMS_BOOL value) { m_bIsSendingRequest = value; }

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

TEST_F(UceOptionsTest, SendOptionsRequest)
{
    IMS_TRACE_D("SendOptionsRequest", 0, 0, 0);

    AString strFrom = "From";
    ON_CALL(objMockICoreService, CreateCapabilities(_, _)).WillByDefault(ReturnNull());

    EXPECT_CALL(objMockICoreService, GetUserIdentity(_))
            .Times(1)
            .WillRepeatedly(ReturnRef(strFrom));
    EXPECT_CALL(objMockIUceJniThread, OptionsErrorInd(_, _)).Times(1);

    pUceOptions->SendOptionsRequest("RemoteURI", 0);

    EXPECT_EQ(pUceOptions->GetKey(), 0);
}
/*
TEST_F(UceOptionsTest, SendOptionsRequest1)
{
    IMS_TRACE_D("SendOptionsRequest1", 0, 0, 0);
    MockICapabilities objMoMockICapabilities;
    MockIMessage objMockIMessage;
    MockISipMessage objMockISipMessage;
    AString strFrom("From");
    SipAddress objUserId("sip:123456@test.ims.com");

    EXPECT_CALL(objMockICoreService, GetPublicGruu()).Times(AnyNumber())
            .WillRepeatedly(Return(&objUserId));
    EXPECT_CALL(objMoMockICapabilities, SetListener(_)).Times(1);
    EXPECT_CALL(objMockICoreService, GetUserIdentity(_)).Times(1)
            .WillRepeatedly(ReturnRef(strFrom));
    EXPECT_CALL(objMockISipMessage, AddHeader).Times(AnyNumber())
            .WillRepeatedly(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockISipMessage, SetHeader).Times(1)
            .WillRepeatedly(Return(IMS_SUCCESS));
    EXPECT_CALL(objMoMockICapabilities, QueryCapabilities(_)).Times(1)
            .WillRepeatedly(Return(IMS_SUCCESS));;

    ON_CALL(objMockICoreService, CreateCapabilities(_, _))
            .WillByDefault(Return(&objMoMockICapabilities));
    ON_CALL(objMoMockICapabilities, GetNextRequest).WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    pUceOptions->SendOptionsRequest("RemoteURI", 0);
}
*/

TEST_F(UceOptionsTest, SendOptionsResponse)
{
    IMS_TRACE_D("SendOptionsResponse", 0, 0, 0);

    EXPECT_CALL(objMockICapabilities, Reject(_, _)).Times(1);

    pUceOptions->SendOptionsResponse(404, "not found", 0);
}

TEST_F(UceOptionsTest, AoSDisconnected)
{
    IMS_TRACE_D("AoSDisconnected", 0, 0, 0);
    pUceOptions->SetSendingRequest(IMS_TRUE);

    EXPECT_CALL(objMockIUceJniThread, OptionsErrorInd(_, _)).Times(1);

    pUceOptions->AoSDisconnected();

    EXPECT_EQ(pUceOptions->GetKey(), 0);
    EXPECT_EQ(pUceOptions->GetSendingRequest(), IMS_FALSE);
}

TEST_F(UceOptionsTest, GetCapability)
{
    IMS_TRACE_D("GetCapability", 0, 0, 0);
    IMSList<AString> objContactList = IMSList<AString>();

    EXPECT_EQ(pUceOptions->GetCapability(objContactList), 0);

    objContactList.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp");
    EXPECT_EQ(pUceOptions->GetCapability(objContactList), UceOptions::FEATURE_TAG_DP);

    objContactList.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel");
    objContactList.Append("video");
    EXPECT_EQ(pUceOptions->GetCapability(objContactList),
            UceOptions::FEATURE_TAG_DP | UceOptions::FEATURE_TAG_IPCALL_VOICE |
                    UceOptions::FEATURE_TAG_IPCALL_VIDEO);

    objContactList.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg");
    objContactList.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg");
    EXPECT_EQ(pUceOptions->GetCapability(objContactList),
            UceOptions::FEATURE_TAG_DP | UceOptions::FEATURE_TAG_IPCALL_VOICE |
                    UceOptions::FEATURE_TAG_IPCALL_VIDEO | UceOptions::FEATURE_TAG_PAGER_MESSAGING |
                    UceOptions::FEATURE_TAG_LARGE_MESSAGING);
}

TEST_F(UceOptionsTest, SetIARIFeatureTag)
{
    IMS_TRACE_D("SetIARIFeatureTag", 0, 0, 0);
    AString strIari = AString::ConstEmpty();
    pUceOptions->SetIARIFeatureTag(UceOptions::FEATURE_TAG_DP, strIari);
    EXPECT_STREQ(strIari.GetStr(), "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp");

    strIari = AString::ConstEmpty();
    pUceOptions->SetIARIFeatureTag(UceOptions::FEATURE_TAG_DP |
                    UceOptions::FEATURE_TAG_IPCALL_VOICE | UceOptions::FEATURE_TAG_IPCALL_VIDEO,
            strIari);
    EXPECT_STREQ(strIari.GetStr(), "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp");

    strIari = AString::ConstEmpty();
    pUceOptions->SetIARIFeatureTag(UceOptions::FEATURE_TAG_DP | UceOptions::FEATURE_TAG_SIMPLE_IM |
                    UceOptions::FEATURE_TAG_FT_HTTP,
            strIari);
    EXPECT_STREQ(strIari.GetStr(),
            "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp,urn%3Aurn-7%3A3gpp-application.ims."
            "iari.rcse.im,urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp");
}

TEST_F(UceOptionsTest, SetICSIFeatureTag)
{
    IMS_TRACE_D("SetIARIFeatureTag", 0, 0, 0);
    AString strIcsi = AString::ConstEmpty();
    pUceOptions->SetICSIFeatureTag(UceOptions::FEATURE_TAG_IPCALL_VOICE, strIcsi);
    EXPECT_STREQ(strIcsi.GetStr(), "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel");

    strIcsi = AString::ConstEmpty();
    pUceOptions->SetICSIFeatureTag(UceOptions::FEATURE_TAG_DP |
                    UceOptions::FEATURE_TAG_IPCALL_VOICE | UceOptions::FEATURE_TAG_IPCALL_VIDEO,
            strIcsi);
    EXPECT_STREQ(strIcsi.GetStr(), "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel");

    strIcsi = AString::ConstEmpty();
    pUceOptions->SetICSIFeatureTag(UceOptions::FEATURE_TAG_CPM_CHAT |
                    UceOptions::FEATURE_TAG_IPCALL_VOICE | UceOptions::FEATURE_TAG_IPCALL_VIDEO,
            strIcsi);
    EXPECT_STREQ(strIcsi.GetStr(),
            "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel,urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm."
            "session");
}

TEST_F(UceOptionsTest, SetNoTypeFeatureTag)
{
    IMS_TRACE_D("SetNoTypeFeatureTag", 0, 0, 0);
    AString strTag = AString::ConstEmpty();
    pUceOptions->SetNoTypeFeatureTag(UceOptions::FEATURE_TAG_IPCALL_VIDEO, strTag);
    EXPECT_STREQ(strTag.GetStr(), "video");

    strTag = AString::ConstEmpty();
    pUceOptions->SetNoTypeFeatureTag(
            UceOptions::FEATURE_TAG_DP | UceOptions::FEATURE_TAG_IPCALL_VIDEO, strTag);
    EXPECT_STREQ(strTag.GetStr(), "video");

    strTag = AString::ConstEmpty();
    pUceOptions->SetNoTypeFeatureTag(
            UceOptions::FEATURE_TAG_IPCALL_VIDEO | UceOptions::FEATURE_TAG_IS_BOT, strTag);
    EXPECT_STREQ(strTag.GetStr(), "video;+g.gsma.rcs.isbot");
}

TEST_F(UceOptionsTest, capaDeliverd)
{
    IMS_TRACE_D("capaDeliverd", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, OptionsErrorInd(_, _)).Times(1);

    ON_CALL(objMockICapabilities, GetPreviousResponse(IMessage::CAPABILITIES_QUERY))
            .WillByDefault(ReturnNull());

    pUceOptions->capaDeliverd(&objMockICapabilities);
}

TEST_F(UceOptionsTest, capaDeliveryFailed)
{
    IMS_TRACE_D("capaDeliveryFailed", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, OptionsResponseInd(_, _, _, _)).Times(1);

    ON_CALL(objMockICapabilities, GetPreviousResponse(IMessage::CAPABILITIES_QUERY))
            .WillByDefault(ReturnNull());
    /*
        MockISipMessage objMockISipMessage;

        ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
        ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(404));

        AString reason("not found");
        ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));
    */
    pUceOptions->capaDeliveryFailed(&objMockICapabilities);
}