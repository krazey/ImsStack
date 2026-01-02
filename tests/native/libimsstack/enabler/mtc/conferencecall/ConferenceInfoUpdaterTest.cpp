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

#include "MockIMtcContext.h"
#include "conferencecall/ConferenceInfo.h"
#include "conferencecall/ConferenceInfoUpdater.h"
#include "conferencecall/ConferenceParticipantList.h"
#include "conferencecall/MockConferenceFactory.h"
#include "conferencecall/MockConferenceInfo.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "utility/MessageUtils.h"
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;

LOCAL AString ANY_EVENT_PACKAGE_BODY = "any conference info package";
LOCAL AString MALFORMED_EVENT_PACKAGE_BODY = "malformed";

LOCAL IMS_UINT32 MAX_USER_COUNT = 6;
LOCAL AString DISPLAY_TEXT = "anyDisplayText";
LOCAL AString LOCAL_URI = "sip:anyLocalUri@ims.google.com";

LOCAL AString USER_ENTITY1 = "sip:anyEntity1@ims.google.com";
LOCAL AString EP_ENTITY1 = "sip:anyEntity1Ep@ims.google.com";
LOCAL AString USER_ENTITY2 = "sip:anyEntity2@ims.google.com";
LOCAL AString EP_ENTITY2 = "sip:anyEntity2Ep@ims.google.com";

LOCAL AString ANONYMOUS_URI = "sip:anonymous@anonymous.invalid";
LOCAL AString ANONYMOUS1_URI = "sip:anonymous1@anonymous.invalid";
LOCAL AString ANONYMOUS2_URI = "sip:anonymous2@anonymous.invalid";

LOCAL IMS_UINT32 ANY_STATE = ConferenceInfo::STATE_FULL;
LOCAL IMS_UINT32 ANY_STATUS = STATUS_CONNECTED;
LOCAL IMS_SINT32 DEFAULT_VERSION = 1;

namespace android
{

class ConferenceInfoUpdaterTest : public ::testing::Test
{
public:
    inline ConferenceInfoUpdaterTest() :
            pInfo(IMS_NULL),
            pDescription(IMS_NULL),
            pFactory(IMS_NULL),
            pConfigurationProxy(IMS_NULL),
            pUpdater(IMS_NULL),
            objMessageUtils(objContext)
    {
    }

    MockConferenceInfo* pInfo;
    MockConferenceDescription* pDescription;
    ImsList<AString> objUris;
    ImsList<ConferenceInfo::User*> objUsers;

    MockIMtcContext objContext;
    MockConferenceFactory* pFactory;
    MockMtcConfigurationProxy* pConfigurationProxy;

    ConferenceParticipantList objParticipantList;
    ConferenceInfoUpdater* pUpdater;
    MessageUtils objMessageUtils;

protected:
    virtual void SetUp() override
    {
        pInfo = IMS_NULL;
        pDescription = IMS_NULL;

        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(*pConfigurationProxy,
                GetBoolean(ConfigVoice::KEY_CHECK_CONFERENCE_EVENT_PACKAGE_VERSION_BOOL))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(*pConfigurationProxy,
                GetBoolean(ConfigVoice::KEY_ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT_BOOL))
                .WillByDefault(Return(IMS_FALSE));

        objParticipantList.SetLocalUri(LOCAL_URI);

        pFactory = new MockConferenceFactory(objContext);
        pUpdater = new ConferenceInfoUpdater(*pFactory, *pConfigurationProxy, objMessageUtils);
    }

    virtual void TearDown() override
    {
        // pInfo is deleted in ~ConferenceInfoUpdater()
        delete pDescription;
        delete pConfigurationProxy;
        delete pUpdater;
        delete pFactory;
    }

    void SetUpConferenceInfo(IN IMS_UINT32 nInfoState, IMS_SINT32 nVersion = DEFAULT_VERSION)
    {
        pDescription = new MockConferenceDescription(MAX_USER_COUNT);

        pInfo = new MockConferenceInfo(*pDescription, objUsers, nInfoState, nVersion);
        ON_CALL(*pFactory, CreateInfo).WillByDefault(Return(pInfo));

        ON_CALL(*pInfo, Parse(ANY_EVENT_PACKAGE_BODY)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(*pInfo, Parse(MALFORMED_EVENT_PACKAGE_BODY)).WillByDefault(Return(IMS_FALSE));
    }

    void AddUserToInfo(IN const AString& strEntity, IN IMS_UINT32 nStatus,
            IN IMS_UINT32 nState = ANY_STATE,
            IN const AString& strDisplayText = AString::ConstNull())
    {
        // assumption1 : User and Endpoint use same entity value
        // assumption2 : one User entity has only one Endpoint entity
        ImsList<ConferenceInfo::User::EndPoint*> objEps;
        objEps.Append(new MockEndPoint(strEntity, nState, strDisplayText, nStatus));
        ConferenceInfo::User* pUser = new MockUser(strEntity, nState, strDisplayText, objEps);
        objUsers.Append(pUser);
    }

    void SetUpDefaultUsers()
    {
        AddUserToInfo(USER_ENTITY1, ANY_STATUS, ANY_STATE);
        AddUserToInfo(USER_ENTITY2, ANY_STATUS, ANY_STATE);
    }

    ConfUser* AddParticipant(
            IN const AString& strUserEntity, IN const AString& strInvitedUri, IN IMS_UINT32 eStatus)
    {
        ConfUser* pUser = new ConfUser();
        pUser->strUserEntity = strUserEntity;
        pUser->eStatus = eStatus;

        objParticipantList.AddUser(pUser);  // pUser is copied.
        delete pUser;

        ConfUser* pStoredConfUser =
                objParticipantList.GetConfUser(objParticipantList.GetSize() - 1);
        objParticipantList.SetReferInviteUri(strInvitedUri, pStoredConfUser);

        return pStoredConfUser;
    }
};

TEST_F(ConferenceInfoUpdaterTest, UpdateFailsByMalformedBody)
{
    SetUpConferenceInfo(ConferenceInfo::STATE_FULL);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_MALFORMED_XML,
            pUpdater->Update(&objParticipantList, MALFORMED_EVENT_PACKAGE_BODY));
}

TEST_F(ConferenceInfoUpdaterTest, UpdateFailsByStateDeleted)
{
    SetUpDefaultUsers();
    SetUpConferenceInfo(ConferenceInfo::STATE_DELETED);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_INFO_DELETED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
}

TEST_F(ConferenceInfoUpdaterTest, UpdateFailsByInvalidVersionInFullState)
{
    SetUpDefaultUsers();
    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 5);
    objParticipantList.SetXmlVersion(10);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_INVALID_VERSION,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
}

TEST_F(ConferenceInfoUpdaterTest, UpdateFailsByInvalidVersionInPartialState)
{
    SetUpDefaultUsers();
    SetUpConferenceInfo(ConferenceInfo::STATE_PARTIAL, 12);
    objParticipantList.SetXmlVersion(10);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_INVALID_VERSION,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
}

TEST_F(ConferenceInfoUpdaterTest, UpdateSucceedsEvenInvalidVersionIfConfigurationOff)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_CHECK_CONFERENCE_EVENT_PACKAGE_VERSION_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    SetUpDefaultUsers();
    SetUpConferenceInfo(ConferenceInfo::STATE_PARTIAL, 12);
    objParticipantList.SetXmlVersion(10);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
}

TEST_F(ConferenceInfoUpdaterTest, UpdateSucceedsIfValidVersionWithPartialState)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_CHECK_CONFERENCE_EVENT_PACKAGE_VERSION_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    SetUpDefaultUsers();
    SetUpConferenceInfo(ConferenceInfo::STATE_PARTIAL, 11);
    objParticipantList.SetXmlVersion(10);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
}

TEST_F(ConferenceInfoUpdaterTest, UpdateSucceedsIfValidVersionWithFullState)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_CHECK_CONFERENCE_EVENT_PACKAGE_VERSION_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    SetUpDefaultUsers();
    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 20);
    objParticipantList.SetXmlVersion(10);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
}

TEST_F(ConferenceInfoUpdaterTest, UpdateFailsByInitialNotifyWithoutUsers)
{
    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_NOTHING_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
}

TEST_F(ConferenceInfoUpdaterTest,
        UpdateDoesNotFailByInitialNotifyWithoutUsersIfPreviousVersionIsValid)
{
    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 11);
    objParticipantList.SetXmlVersion(10);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
}

TEST_F(ConferenceInfoUpdaterTest, UpdateFailsByInitialNotifyWithOnlyHostUri)
{
    AddUserToInfo(LOCAL_URI, STATUS_CONNECTED);
    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_NOTHING_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
}

TEST_F(ConferenceInfoUpdaterTest, UpdateByUserEntityWithLegid)
{
    IMS_UINT32 eStatusBefore1 = STATUS_IDLE;
    IMS_UINT32 eStatusBefore2 = STATUS_IDLE;
    IMS_UINT32 eStatusAfter1 = STATUS_CONNECTED;
    IMS_UINT32 eStatusAfter2 = STATUS_ON_HOLD;

    // to cover 'IsLocalUri is true' case
    AddUserToInfo(LOCAL_URI, STATUS_CONNECTED);

    AString strAnyEntityWithLegid1 = USER_ENTITY1 + ";legid=1";
    AddUserToInfo(strAnyEntityWithLegid1, eStatusAfter1);
    const ConfUser* pUser1 = AddParticipant(USER_ENTITY1, USER_ENTITY1, eStatusBefore1);

    AString strAnyEntityWithLegid2 = USER_ENTITY2 + ";legid=2";
    AddUserToInfo(strAnyEntityWithLegid2, eStatusAfter2);
    const ConfUser* pUser2 = AddParticipant(USER_ENTITY2, USER_ENTITY2, eStatusBefore2);

    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
    EXPECT_EQ(pUser1->eStatus, eStatusAfter1);
    EXPECT_EQ(pUser2->eStatus, eStatusAfter2);
}

TEST_F(ConferenceInfoUpdaterTest, UpdateByUserEntityMatching)
{
    IMS_UINT32 eStatusBefore1 = STATUS_IDLE;
    IMS_UINT32 eStatusBefore2 = STATUS_IDLE;
    IMS_UINT32 eStatusAfter1 = STATUS_CONNECTED;
    IMS_UINT32 eStatusAfter2 = STATUS_ON_HOLD;

    AString strAnyEntity1 = USER_ENTITY1;
    AddUserToInfo(strAnyEntity1, eStatusAfter1);
    const ConfUser* pUser1 = AddParticipant(USER_ENTITY1, USER_ENTITY1, eStatusBefore1);

    AString strAnyEntity2 = USER_ENTITY2;
    AddUserToInfo(strAnyEntity2, eStatusAfter2);
    const ConfUser* pUser2 = AddParticipant(USER_ENTITY2, USER_ENTITY2, eStatusBefore2);

    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
    EXPECT_EQ(pUser1->eStatus, eStatusAfter1);
    EXPECT_EQ(pUser2->eStatus, eStatusAfter2);
}

TEST_F(ConferenceInfoUpdaterTest, UpdateByUserEntityMatchingByDifferentUserPhoneParameter)
{
    IMS_UINT32 eStatusBefore1 = STATUS_IDLE;
    IMS_UINT32 eStatusBefore2 = STATUS_IDLE;
    IMS_UINT32 eStatusAfter1 = STATUS_CONNECTED;
    IMS_UINT32 eStatusAfter2 = STATUS_ON_HOLD;

    AString strAnyEntity1WithPhone = USER_ENTITY1 + ";user=phone";
    AddUserToInfo(strAnyEntity1WithPhone, eStatusAfter1);
    const ConfUser* pUser1 = AddParticipant(USER_ENTITY1, strAnyEntity1WithPhone, eStatusBefore1);

    AString strAnyEntity2WithoutPhone = USER_ENTITY2;
    AddUserToInfo(strAnyEntity2WithoutPhone, eStatusAfter2);
    const ConfUser* pUser2 = AddParticipant(
            USER_ENTITY2 + ";user=phone", USER_ENTITY2 + ";user=phone", eStatusBefore2);

    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
    EXPECT_EQ(pUser1->eStatus, eStatusBefore1);
    EXPECT_EQ(pUser2->eStatus, eStatusBefore2);
}

TEST_F(ConferenceInfoUpdaterTest, UpdateByUserEntityMatchingWithAnonymousUri)
{
    IMS_UINT32 eStatusBefore1 = STATUS_IDLE;
    IMS_UINT32 eStatusAfter1 = STATUS_CONNECTED;

    AString strAnyEntity1 = USER_ENTITY1;
    AddUserToInfo(ANONYMOUS1_URI, eStatusAfter1);
    ConfUser* pUser1 = AddParticipant(ANONYMOUS_URI, ANONYMOUS_URI, eStatusBefore1);

    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
    EXPECT_EQ(pUser1->eStatus, eStatusAfter1);
    EXPECT_STREQ(pUser1->strUserEntity.GetStr(), ANONYMOUS1_URI.GetStr());
}

TEST_F(ConferenceInfoUpdaterTest, UpdateByOrderMatching)
{
    IMS_UINT32 eStatusBefore1 = STATUS_IDLE;
    IMS_UINT32 eStatusBefore2 = STATUS_IDLE;
    IMS_UINT32 eStatusAfter1 = STATUS_CONNECTED;
    IMS_UINT32 eStatusAfter2 = STATUS_ON_HOLD;

    AddUserToInfo(LOCAL_URI, STATUS_CONNECTED);

    AddUserToInfo(ANONYMOUS1_URI, eStatusAfter1);
    ConfUser* pUser1 = AddParticipant("", USER_ENTITY1, eStatusBefore1);

    AddUserToInfo(ANONYMOUS2_URI, eStatusAfter2);
    ConfUser* pUser2 = AddParticipant("", USER_ENTITY2, eStatusBefore2);

    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
    EXPECT_EQ(pUser1->eStatus, eStatusAfter1);
    EXPECT_STREQ(pUser1->strUserEntity.GetStr(), ANONYMOUS1_URI.GetStr());
    EXPECT_EQ(pUser2->eStatus, eStatusAfter2);
    EXPECT_STREQ(pUser2->strUserEntity.GetStr(), ANONYMOUS2_URI.GetStr());
}

TEST_F(ConferenceInfoUpdaterTest, UpdateByOrderMatchingDoesNotUpdateIfAlreadyHaveUserEntity)
{
    IMS_UINT32 eStatusBefore1 = STATUS_IDLE;
    IMS_UINT32 eStatusBefore2 = STATUS_IDLE;
    IMS_UINT32 eStatusAfter1 = STATUS_CONNECTED;
    IMS_UINT32 eStatusAfter2 = STATUS_ON_HOLD;

    AddUserToInfo(LOCAL_URI, STATUS_CONNECTED);

    AddUserToInfo(ANONYMOUS1_URI, eStatusAfter1);
    ConfUser* pUser1 = AddParticipant(USER_ENTITY1, USER_ENTITY1, eStatusBefore1);

    AddUserToInfo(ANONYMOUS2_URI, eStatusAfter2);
    ConfUser* pUser2 = AddParticipant(USER_ENTITY2, USER_ENTITY2, eStatusBefore2);

    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
    EXPECT_EQ(pUser1->eStatus, eStatusBefore1);
    EXPECT_STREQ(pUser1->strUserEntity.GetStr(), USER_ENTITY1.GetStr());
    EXPECT_EQ(pUser2->eStatus, eStatusBefore2);
    EXPECT_STREQ(pUser2->strUserEntity.GetStr(), USER_ENTITY2.GetStr());
}

TEST_F(ConferenceInfoUpdaterTest, UpdateByOrderMatchingOnParticipant)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    IMS_UINT32 eStatusAfter1 = STATUS_CONNECTED;
    IMS_UINT32 eStatusAfter2 = STATUS_CONNECTED;

    AddUserToInfo(USER_ENTITY1, eStatusAfter1);
    AddUserToInfo(USER_ENTITY2, eStatusAfter2);

    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));

    // adding is made in reverse order
    EXPECT_EQ(objParticipantList.GetAt(1)->GetConfUser()->eStatus, eStatusAfter1);
    EXPECT_STREQ(objParticipantList.GetAt(1)->GetConfUser()->strUserEntity.GetStr(),
            USER_ENTITY1.GetStr());
    EXPECT_EQ(objParticipantList.GetAt(0)->GetConfUser()->eStatus, eStatusAfter2);
    EXPECT_STREQ(objParticipantList.GetAt(0)->GetConfUser()->strUserEntity.GetStr(),
            USER_ENTITY2.GetStr());
}

TEST_F(ConferenceInfoUpdaterTest, UpdateByReferToMatching)
{
    IMS_UINT32 eStatusBefore1 = STATUS_IDLE;
    IMS_UINT32 eStatusBefore2 = STATUS_IDLE;
    IMS_UINT32 eStatusAfter1 = STATUS_CONNECTED;
    IMS_UINT32 eStatusAfter2 = STATUS_ON_HOLD;

    AddUserToInfo(USER_ENTITY1, eStatusAfter1);
    ConfUser* pUser1 = AddParticipant("", USER_ENTITY1, eStatusBefore1);

    AddUserToInfo(USER_ENTITY2, eStatusAfter2);
    ConfUser* pUser2 = AddParticipant("", USER_ENTITY2, eStatusBefore2);

    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
    EXPECT_EQ(pUser1->eStatus, eStatusAfter1);
    EXPECT_STREQ(pUser1->strUserEntity.GetStr(), USER_ENTITY1.GetStr());
    EXPECT_EQ(pUser2->eStatus, eStatusAfter2);
    EXPECT_STREQ(pUser2->strUserEntity.GetStr(), USER_ENTITY2.GetStr());
}

TEST_F(ConferenceInfoUpdaterTest, UpdateByReferToMatchingDoesNotUpdateIfAlreadyHaveUserEntity)
{
    IMS_UINT32 eStatusBefore1 = STATUS_IDLE;
    IMS_UINT32 eStatusBefore2 = STATUS_IDLE;
    IMS_UINT32 eStatusAfter1 = STATUS_CONNECTED;
    IMS_UINT32 eStatusAfter2 = STATUS_ON_HOLD;

    AddUserToInfo(USER_ENTITY1, eStatusAfter1);
    ConfUser* pUser1 = AddParticipant(ANONYMOUS1_URI, USER_ENTITY1, eStatusBefore1);

    AddUserToInfo(USER_ENTITY2, eStatusAfter2);
    ConfUser* pUser2 = AddParticipant(ANONYMOUS2_URI, USER_ENTITY2, eStatusBefore2);

    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
    EXPECT_EQ(pUser1->eStatus, eStatusBefore1);
    EXPECT_STREQ(pUser1->strUserEntity.GetStr(), ANONYMOUS1_URI.GetStr());
    EXPECT_EQ(pUser2->eStatus, eStatusBefore2);
    EXPECT_STREQ(pUser2->strUserEntity.GetStr(), ANONYMOUS2_URI.GetStr());
}

TEST_F(ConferenceInfoUpdaterTest, UpdatebyUserEntityWithPrefix)
{
    IMS_UINT32 eStatusBefore1 = STATUS_IDLE;
    IMS_UINT32 eStatusAfter1 = STATUS_CONNECTED;

    AString strUriWithPrefix("sip:+1123456");
    AString strUri("sip:123456");
    AddUserToInfo(strUriWithPrefix, eStatusAfter1);
    ConfUser* pUser1 = AddParticipant(strUri, strUri, eStatusBefore1);

    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
    EXPECT_EQ(pUser1->eStatus, eStatusAfter1);
    EXPECT_STREQ(pUser1->strUserEntity.GetStr(), strUriWithPrefix.GetStr());
}

TEST_F(ConferenceInfoUpdaterTest, UpdateDoesNotSetDisconnectedBeforeConnected)
{
    IMS_UINT32 eStatusBefore1 = STATUS_IDLE;
    IMS_UINT32 eStatusBefore2 = STATUS_IDLE;
    IMS_UINT32 eStatusAfter2 = STATUS_DISCONNECTED;

    // no User entity in the first NOTIFY case
    const ConfUser* pUser1 = AddParticipant(USER_ENTITY1, USER_ENTITY1, eStatusBefore1);

    // disconnected status before connected case
    AString strAnyEntityWithLegid2 = USER_ENTITY2;
    AddUserToInfo(strAnyEntityWithLegid2, eStatusAfter2);
    const ConfUser* pUser2 = AddParticipant(USER_ENTITY2, USER_ENTITY2, eStatusBefore2);

    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
    EXPECT_EQ(pUser1->eStatus, eStatusBefore1);
    EXPECT_EQ(pUser2->eStatus, eStatusBefore2);
}

TEST_F(ConferenceInfoUpdaterTest, UpdateSetsDisconnectedAfterConnected)
{
    IMS_UINT32 eStatusBefore1 = STATUS_CONNECTED;
    IMS_UINT32 eStatusBefore2 = STATUS_CONNECTED;
    IMS_UINT32 eStatusAfter1 = STATUS_DISCONNECTED;
    IMS_UINT32 eStatusAfter2 = STATUS_DISCONNECTED;

    // no User entity in the first NOTIFY case
    const ConfUser* pUser1 = AddParticipant(USER_ENTITY1, USER_ENTITY1, eStatusBefore1);
    objParticipantList.GetAt(0)->SetInfoUpdated(IMS_TRUE);

    // disconnected status before connected case
    AString strAnyEntityWithLegid2 = USER_ENTITY2;
    AddUserToInfo(strAnyEntityWithLegid2, eStatusAfter2);
    const ConfUser* pUser2 = AddParticipant(USER_ENTITY2, USER_ENTITY2, eStatusBefore2);
    objParticipantList.GetAt(1)->SetInfoUpdated(IMS_TRUE);

    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
    EXPECT_EQ(pUser1->eStatus, eStatusAfter1);
    EXPECT_EQ(pUser2->eStatus, eStatusAfter2);
}

TEST_F(ConferenceInfoUpdaterTest, UpdateDoesNotSetDisconnectedIfPartial)
{
    IMS_UINT32 eStatusBefore1 = STATUS_CONNECTED;
    IMS_UINT32 eStatusBefore2 = STATUS_IDLE;
    IMS_UINT32 eStatusAfter2 = STATUS_CONNECTED;

    // no User entity in the first NOTIFY case
    const ConfUser* pUser1 = AddParticipant(USER_ENTITY1, USER_ENTITY1, eStatusBefore1);
    objParticipantList.GetAt(0)->SetInfoUpdated(IMS_TRUE);

    // disconnected status before connected case
    AString strAnyEntityWithLegid2 = USER_ENTITY2;
    AddUserToInfo(strAnyEntityWithLegid2, eStatusAfter2);
    const ConfUser* pUser2 = AddParticipant(USER_ENTITY2, USER_ENTITY2, eStatusBefore2);
    objParticipantList.GetAt(1)->SetInfoUpdated(IMS_TRUE);

    SetUpConferenceInfo(ConferenceInfo::STATE_PARTIAL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
    EXPECT_EQ(pUser1->eStatus, eStatusBefore1);
    EXPECT_EQ(pUser2->eStatus, eStatusAfter2);
}

TEST_F(ConferenceInfoUpdaterTest, UpdateDisconnectingStatusAsDisconnected)
{
    IMS_UINT32 eStatusBefore1 = STATUS_CONNECTED;
    IMS_UINT32 eStatusAfter1 = STATUS_DISCONNECTING;
    IMS_UINT32 eStatusAfterModified1 = STATUS_DISCONNECTED;

    AddUserToInfo(USER_ENTITY1, eStatusAfter1);
    const ConfUser* pUser1 = AddParticipant(USER_ENTITY1, USER_ENTITY1, eStatusBefore1);
    objParticipantList.GetAt(0)->SetInfoUpdated(IMS_TRUE);

    SetUpConferenceInfo(ConferenceInfo::STATE_FULL, 1);

    EXPECT_EQ(ConferenceInfoUpdater::RESULT_UPDATED,
            pUpdater->Update(&objParticipantList, ANY_EVENT_PACKAGE_BODY));
    EXPECT_EQ(pUser1->eStatus, eStatusAfterModified1);
}

TEST_F(ConferenceInfoUpdaterTest, ConvertPolicyToStringReturnsCorrespondingString)
{
    EXPECT_STREQ(ConferenceInfoUpdater::ConvertPolicyToString(MatchingPolicy::ORDER_LEG_ID),
            "ORDER_LEG_ID");
    EXPECT_STREQ(ConferenceInfoUpdater::ConvertPolicyToString(MatchingPolicy::ORDER), "ORDER");
    EXPECT_STREQ(ConferenceInfoUpdater::ConvertPolicyToString(MatchingPolicy::REFER_TO_URI),
            "REFER_TO_URI");
    EXPECT_STREQ(
            ConferenceInfoUpdater::ConvertPolicyToString(MatchingPolicy::USERENTITY), "USERENTITY");
}

TEST_F(ConferenceInfoUpdaterTest, ConvertStatusToStringReturnsCorrespondingString)
{
    EXPECT_STREQ(ConferenceInfoUpdater::ConvertStatusToString(STATUS_CONNECTED), "connected");
    EXPECT_STREQ(ConferenceInfoUpdater::ConvertStatusToString(STATUS_DISCONNECTED), "disconnected");
    EXPECT_STREQ(ConferenceInfoUpdater::ConvertStatusToString(STATUS_ON_HOLD), "on-hold");
    EXPECT_STREQ(ConferenceInfoUpdater::ConvertStatusToString(STATUS_MUTED_VIA_FOCUS),
            "muted-via-focus");
    EXPECT_STREQ(ConferenceInfoUpdater::ConvertStatusToString(STATUS_PENDING), "pending");
    EXPECT_STREQ(ConferenceInfoUpdater::ConvertStatusToString(STATUS_ALERTING), "alerting");
    EXPECT_STREQ(ConferenceInfoUpdater::ConvertStatusToString(STATUS_DIALING_IN), "dialing-in");
    EXPECT_STREQ(ConferenceInfoUpdater::ConvertStatusToString(STATUS_DIALING_OUT), "dialing-out");
    EXPECT_STREQ(
            ConferenceInfoUpdater::ConvertStatusToString(STATUS_DISCONNECTING), "disconnecting");
    EXPECT_STREQ(ConferenceInfoUpdater::ConvertStatusToString(STATUS_FAIL), "connect-fail");
    EXPECT_STREQ(ConferenceInfoUpdater::ConvertStatusToString(-1), "__STATUS_IDLE__");
}

}  // namespace android
