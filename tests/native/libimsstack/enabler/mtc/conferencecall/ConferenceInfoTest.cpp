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

#include "conferencecall/ConferenceInfo.h"
#include <gtest/gtest.h>
#include <vector>

namespace android
{
// clang-format off
static const IMS_CHAR CONFERENCE_EVENT_PACKAGE_TEMPLATE[] = {
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<conference-info\n"
"xmlns=\"urn:ietf:params:xml:ns:conference-info\" \n"
"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \n"
"entity=\"sip:111111110571@tas001ltn.ims.mnc030.mcc234.3gppnetwork.org:5060\" \n"
"state=\"_CONFERENCE_STATE_\" version=\"_CONFERENCE_VERSION_\">\n"
"<conference-description>\n"
"<maximum-user-count>_DESCRIPTION_MAX_USER_COUNT_</maximum-user-count>\n"
"</conference-description>\n"
"<users>\n"
"<user entity=\"_USER_1_ENTITY_\" state=\"_USER_1_STATE_\">\n"
"<display-text>_USER_1_DISPLAY_TEXT_</display-text>\n"
"<endpoint entity=\"_USER_1_EP_1_ENTITY_\" state=\"_USER_1_EP_1_STATE\">\n"
"<status>_USER_1_EP_1_STATUS_</status>\n"
"<display-text>_USER_1_EP_1_DISPLAY_TEXT_</display-text>\n"
"</endpoint>\n"
"</user>\n"
"<user entity=\"_USER_2_ENTITY_\" state=\"_USER_2_STATE_\">\n"
"<display-text>_USER_2_DISPLAY_TEXT_</display-text>\n"
"<endpoint entity=\"_USER_2_EP_1_ENTITY_\" state=\"_USER_2_EP_1_STATE\">\n"
"<status>_USER_2_EP_1_STATUS_</status>\n"
"<display-text>_USER_2_EP_1_DISPLAY_TEXT_</display-text>\n"
"</endpoint>\n"
"</user>\n"
"</users>\n"
"</conference-info>\n"
};
// clang-format on

class ConferenceInfoTest : public ::testing::Test
{
public:
    inline ConferenceInfoTest() :
            strConferenceState("full"),
            strConferenceVersion("1"),
            strDescriptionMaxUserCount("6"),
            strUser1Entity("sip:user1"),
            strUser1State("full"),
            strUser1DisplayText("user1"),
            strUser1Ep1Entity("sip:user1ep1@192.168.0.1"),
            strUser1Ep1State("full"),
            strUser1Ep1Status("connected"),
            strUser1Ep1DisplayText("user1 ep1"),
            strUser2Entity("sip:user2"),
            strUser2State("full"),
            strUser2DisplayText("user2"),
            strUser2Ep1Entity("sip:user2ep1@192.168.0.2"),
            strUser2Ep1State("full"),
            strUser2Ep1Status("connected"),
            strUser2Ep1DisplayText("user2 ep1"),
            objConferenceInfo()
    {
    }

protected:
    AString strConferenceState;
    AString strConferenceVersion;
    AString strDescriptionMaxUserCount;

    AString strUser1Entity;
    AString strUser1State;
    AString strUser1DisplayText;
    AString strUser1Ep1Entity;
    AString strUser1Ep1State;
    AString strUser1Ep1Status;
    AString strUser1Ep1DisplayText;

    AString strUser2Entity;
    AString strUser2State;
    AString strUser2DisplayText;
    AString strUser2Ep1Entity;
    AString strUser2Ep1State;
    AString strUser2Ep1Status;
    AString strUser2Ep1DisplayText;

    ConferenceInfo objConferenceInfo;

    virtual void SetUp() override {}

    virtual void TearDown() override {}

    const AString GetConferenceEventPackage()
    {
        AString strConferenceEventPackage(CONFERENCE_EVENT_PACKAGE_TEMPLATE);
        strConferenceEventPackage =
                strConferenceEventPackage.Replace("_CONFERENCE_STATE_", strConferenceState);
        strConferenceEventPackage =
                strConferenceEventPackage.Replace("_CONFERENCE_VERSION_", strConferenceVersion);
        strConferenceEventPackage = strConferenceEventPackage.Replace(
                "_DESCRIPTION_MAX_USER_COUNT_", strDescriptionMaxUserCount);
        strConferenceEventPackage =
                strConferenceEventPackage.Replace("_USER_1_ENTITY_", strUser1Entity);
        strConferenceEventPackage =
                strConferenceEventPackage.Replace("_USER_1_STATE_", strUser1State);
        strConferenceEventPackage =
                strConferenceEventPackage.Replace("_USER_1_DISPLAY_TEXT_", strUser1DisplayText);
        strConferenceEventPackage =
                strConferenceEventPackage.Replace("_USER_1_EP_1_ENTITY_", strUser1Ep1Entity);
        strConferenceEventPackage =
                strConferenceEventPackage.Replace("_USER_1_EP_1_STATE", strUser1Ep1State);
        strConferenceEventPackage =
                strConferenceEventPackage.Replace("_USER_1_EP_1_STATUS_", strUser1Ep1Status);
        strConferenceEventPackage = strConferenceEventPackage.Replace(
                "_USER_1_EP_1_DISPLAY_TEXT_", strUser1Ep1DisplayText);
        strConferenceEventPackage =
                strConferenceEventPackage.Replace("_USER_2_ENTITY_", strUser2Entity);
        strConferenceEventPackage =
                strConferenceEventPackage.Replace("_USER_2_STATE_", strUser2State);
        strConferenceEventPackage =
                strConferenceEventPackage.Replace("_USER_2_DISPLAY_TEXT_", strUser2DisplayText);
        strConferenceEventPackage =
                strConferenceEventPackage.Replace("_USER_2_EP_1_ENTITY_", strUser2Ep1Entity);
        strConferenceEventPackage =
                strConferenceEventPackage.Replace("_USER_2_EP_1_STATE", strUser2Ep1State);
        strConferenceEventPackage =
                strConferenceEventPackage.Replace("_USER_2_EP_1_STATUS_", strUser2Ep1Status);
        strConferenceEventPackage = strConferenceEventPackage.Replace(
                "_USER_2_EP_1_DISPLAY_TEXT_", strUser2Ep1DisplayText);

        return strConferenceEventPackage;
    }

    const AString GetConferenceEventPackageWithInvalidElement(IN const AString& strName)
    {
        AString strInvalidName = "invalid-" + strName;
        AString strConferenceEventPackage(CONFERENCE_EVENT_PACKAGE_TEMPLATE);
        strConferenceEventPackage = strConferenceEventPackage.Replace(strName, strInvalidName);
        return strConferenceEventPackage;
    }
};

TEST_F(ConferenceInfoTest, ParseSucceedsIfValidPackage)
{
    AString strXml = GetConferenceEventPackage();
    EXPECT_TRUE(objConferenceInfo.Parse(strXml));
}

TEST_F(ConferenceInfoTest, ParseFailsIfInvalidPackage)
{
    // empty package
    AString strXml = "";
    EXPECT_FALSE(objConferenceInfo.Parse(strXml));

    // no root element
    // TODO: how to do this?
    strXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    strXml.Append("<not-element/>");
    EXPECT_FALSE(objConferenceInfo.Parse(strXml));

    // no conference-info element
    strXml = GetConferenceEventPackageWithInvalidElement("conference-info");
    EXPECT_FALSE(objConferenceInfo.Parse(strXml));
}

TEST_F(ConferenceInfoTest, GetStateReturnsConferenceInfoStateInXml)
{
    strConferenceState = "full";
    AString strXml = GetConferenceEventPackage();
    EXPECT_TRUE(objConferenceInfo.Parse(strXml));
    EXPECT_EQ(objConferenceInfo.GetState(), ConferenceInfo::STATE_FULL);

    strConferenceState = "partial";
    strXml = GetConferenceEventPackage();
    EXPECT_TRUE(objConferenceInfo.Parse(strXml));
    EXPECT_EQ(objConferenceInfo.GetState(), ConferenceInfo::STATE_PARTIAL);

    strConferenceState = "deleted";
    strXml = GetConferenceEventPackage();
    EXPECT_TRUE(objConferenceInfo.Parse(strXml));
    EXPECT_EQ(objConferenceInfo.GetState(), ConferenceInfo::STATE_DELETED);
}

TEST_F(ConferenceInfoTest, GetVersionReturnsConferenceInfoVersionInXml)
{
    strConferenceVersion = "10";
    AString strXml = GetConferenceEventPackage();
    EXPECT_TRUE(objConferenceInfo.Parse(strXml));
    EXPECT_EQ(objConferenceInfo.GetVersion(), 10);
}

TEST_F(ConferenceInfoTest, NoMaximumUserCountReturnsDefaultValue)
{
    AString strXml = GetConferenceEventPackageWithInvalidElement("maximum-user-count");
    EXPECT_TRUE(objConferenceInfo.Parse(strXml));
    // DEFAULT_MAX_USER_COUNT = 6
    EXPECT_EQ(objConferenceInfo.GetConferenceDescription().GetMaxUserCount(), 6);
}

TEST_F(ConferenceInfoTest, MaximumUserCountReturnsValueFromXml)
{
    strDescriptionMaxUserCount = "100";
    AString strXml = GetConferenceEventPackage();
    EXPECT_TRUE(objConferenceInfo.Parse(strXml));
    EXPECT_EQ(objConferenceInfo.GetConferenceDescription().GetMaxUserCount(), 100);
}

TEST_F(ConferenceInfoTest, GetUsersReturnsAllUseEntitiesInXml)
{
    AString strXml = GetConferenceEventPackage();
    EXPECT_TRUE(objConferenceInfo.Parse(strXml));
    const ImsList<ConferenceInfo::User*>& objUsers = objConferenceInfo.GetUsers();

    EXPECT_EQ(objUsers.GetSize(), 2);
}

TEST_F(ConferenceInfoTest, GetUserInfoFromUsersReturnsValuesInXml)
{
    strUser1Entity = "sip:AnyUserEntity";
    strUser1State = "partial";
    strUser1DisplayText = "Any User Display Name";

    AString strXml = GetConferenceEventPackage();
    EXPECT_TRUE(objConferenceInfo.Parse(strXml));
    const ImsList<ConferenceInfo::User*>& objUsers = objConferenceInfo.GetUsers();
    ConferenceInfo::User* pUser1 = objUsers.GetAt(0);
    EXPECT_STREQ(pUser1->GetEntity().GetStr(), "sip:AnyUserEntity");
    EXPECT_EQ(pUser1->GetState(), ConferenceInfo::STATE_PARTIAL);
    EXPECT_STREQ(pUser1->GetDisplayText().GetStr(), "Any User Display Name");
}

TEST_F(ConferenceInfoTest, GetEndPointInfoFromEndPointsReturnsValueInXml)
{
    strUser2Ep1Entity = "sip:AnyEpEntity@192.168.0.2";
    strUser2Ep1State = "deleted";
    strUser2Ep1Status = "disconnected";
    strUser2Ep1DisplayText = "Any EndPoint Display Name";

    AString strXml = GetConferenceEventPackage();
    EXPECT_TRUE(objConferenceInfo.Parse(strXml));
    const ImsList<ConferenceInfo::User*>& objUsers = objConferenceInfo.GetUsers();
    ConferenceInfo::User* pUser2 = objUsers.GetAt(1);
    ConferenceInfo::User::EndPoint* pEndPoint1Of2 = pUser2->GetEndPoints().GetAt(0);
    EXPECT_STREQ(pEndPoint1Of2->GetEntity().GetStr(), "sip:AnyEpEntity@192.168.0.2");
    EXPECT_EQ(pEndPoint1Of2->GetState(), ConferenceInfo::STATE_DELETED);
    EXPECT_STREQ(pEndPoint1Of2->GetDisplayText().GetStr(), "Any EndPoint Display Name");
    EXPECT_EQ(pEndPoint1Of2->GetStatus(), STATUS_DISCONNECTED);
}

TEST_F(ConferenceInfoTest, ConvertStatusForAllCases)
{
    strUser1Ep1State = "full";
    // clang-format off
    std::vector<AString> objStatusStrings{
            "connected", "disconnected", "on-hold",
            "muted-via-focus", "pending", "alerting",
            "dialing-in", "dialing-out", "disconnecting",
            "connect-fail", "idle"
    };

    std::vector<IMS_SINT32> objStatusEnum{
            STATUS_CONNECTED, STATUS_DISCONNECTED, STATUS_ON_HOLD,
            STATUS_MUTED_VIA_FOCUS, STATUS_PENDING, STATUS_ALERTING,
            STATUS_DIALING_IN, STATUS_DIALING_OUT, STATUS_DISCONNECTING,
            STATUS_FAIL, STATUS_IDLE
    };
    // clang-format on
    IMS_UINT32 nIndex = 0;
    for (AString strStatus : objStatusStrings)
    {
        ConferenceInfo objInfo;  // to avoid stacking User list
        strUser1Ep1Status = strStatus;
        AString strXml = GetConferenceEventPackage();
        EXPECT_TRUE(objInfo.Parse(strXml));
        const ImsList<ConferenceInfo::User*>& objUsers = objInfo.GetUsers();
        ConferenceInfo::User* pUser1 = objUsers.GetAt(0);
        ConferenceInfo::User::EndPoint* pEndPoint1Of1 = pUser1->GetEndPoints().GetAt(0);
        EXPECT_EQ(pEndPoint1Of1->GetStatus(), objStatusEnum.at(nIndex));
        ++nIndex;
    }
}

}  // namespace android
