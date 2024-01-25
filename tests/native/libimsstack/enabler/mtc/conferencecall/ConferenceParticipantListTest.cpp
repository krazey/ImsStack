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
#include "call/MockCallConnectionIdManager.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallManager.h"
#include "conferencecall/ConferenceParticipantList.h"
#include "conferencecall/MockIConferenceReference.h"
#include "helper/MockICallStateProxy.h"
#include <gtest/gtest.h>

LOCAL const IMS_CHAR USER_ENTITY[] = "sip:testuser@ims.google.com;user=phone";

using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class ConferenceParticipantListTest : public ::testing::Test
{
public:
    ConferenceParticipantList::ConferenceParticipant* pParticipant;
    ConferenceParticipantList* pParticipantList;

protected:
    virtual void SetUp() override
    {
        // TODO: separate into different Test files?
        pParticipant = new ConferenceParticipantList::ConferenceParticipant();
        pParticipantList = new ConferenceParticipantList();
    }

    virtual void TearDown() override
    {
        delete pParticipant;
        delete pParticipantList;
    }
};

// ConferenceParticipantTests
TEST_F(ConferenceParticipantListTest, CreateParticipant)
{
    ASSERT_NE(pParticipant, nullptr);
}

TEST_F(ConferenceParticipantListTest, SetConfUserCopiesConfUser)
{
    ConfUser* pUser = new ConfUser();
    pParticipant->SetConfUser(pUser);

    EXPECT_NE(pParticipant->GetConfUser(), pUser);
    delete pUser;
}

TEST_F(ConferenceParticipantListTest, SetConfUserWithUserEntityAndGetUserEntity)
{
    ConfUser* pUser = new ConfUser();
    pUser->strUserEntity = USER_ENTITY;
    pParticipant->SetConfUser(pUser);

    EXPECT_STREQ(pParticipant->GetUserEntity().GetStr(), USER_ENTITY);
    delete pUser;
}

TEST_F(ConferenceParticipantListTest, SetAndGetReference)
{
    EXPECT_EQ(pParticipant->GetReference(), nullptr);

    MockIConferenceReference objMockConferenceReference;
    pParticipant->SetReference(&objMockConferenceReference);

    EXPECT_EQ(pParticipant->GetReference(), &objMockConferenceReference);
}

TEST_F(ConferenceParticipantListTest, SetAndGetReferInviteUri)
{
    EXPECT_STREQ(pParticipant->GetReferInviteUri().GetStr(), AString::ConstNull().GetStr());

    AString strInviteUri = "sip:tempuri@ims.google.com";
    pParticipant->SetReferInviteUri(strInviteUri);

    EXPECT_STREQ(pParticipant->GetReferInviteUri().GetStr(), strInviteUri.GetStr());
}

TEST_F(ConferenceParticipantListTest, SetAndIsInfoUpdated)
{
    EXPECT_FALSE(pParticipant->IsInfoUpdated());
    pParticipant->SetInfoUpdated(IMS_TRUE);
    EXPECT_TRUE(pParticipant->IsInfoUpdated());
}

TEST_F(ConferenceParticipantListTest, SetAndIsMatchingCompleted)
{
    EXPECT_FALSE(pParticipant->IsMatchingCompleted());
    pParticipant->SetMatchingCompleted(IMS_TRUE);
    EXPECT_TRUE(pParticipant->IsMatchingCompleted());
}

TEST_F(ConferenceParticipantListTest, SetAndIsDisconnectionNotified)
{
    EXPECT_FALSE(pParticipant->IsDisconnectionNotified());
    pParticipant->SetDisconnectionNotified(IMS_TRUE);
    EXPECT_TRUE(pParticipant->IsDisconnectionNotified());
}

TEST_F(ConferenceParticipantListTest, SetAndIsDisconnectedExplicitly)
{
    EXPECT_FALSE(pParticipant->IsDisconnectedExplicitly());
    pParticipant->SetDisconnectedExplicitly(IMS_TRUE);
    EXPECT_TRUE(pParticipant->IsDisconnectedExplicitly());
}

// ConferenceParticipantListTests
TEST_F(ConferenceParticipantListTest, SetAndGetLocalUri)
{
    EXPECT_STREQ(pParticipantList->GetLocalUri().GetStr(), AString::ConstNull().GetStr());
    AString strLocaUri("sip:localuri");
    pParticipantList->SetLocalUri(strLocaUri);
    EXPECT_STREQ(pParticipantList->GetLocalUri().GetStr(), strLocaUri.GetStr());
}

TEST_F(ConferenceParticipantListTest, SetAndGetXmlVersion)
{
    EXPECT_EQ(pParticipantList->GetXmlVersion(), -1);
    IMS_SINT32 nVersion = 10;
    pParticipantList->SetXmlVersion(nVersion);
    EXPECT_EQ(pParticipantList->GetXmlVersion(), nVersion);
}

TEST_F(ConferenceParticipantListTest, SetAndGetMaxUserCount)
{
    EXPECT_EQ(pParticipantList->GetMaxUserCount(), 0);
    IMS_SINT32 nMaxUserCount = 6;
    pParticipantList->SetMaxUserCount(nMaxUserCount);
    EXPECT_EQ(pParticipantList->GetMaxUserCount(), nMaxUserCount);
}

TEST_F(ConferenceParticipantListTest, AddUserCreatesNewParticipant)
{
    EXPECT_EQ(pParticipantList->GetSize(), 0);
    ConfUser* pUser = new ConfUser();
    pParticipantList->AddUser(pUser);

    EXPECT_EQ(pParticipantList->GetSize(), 1);
    delete pUser;
}

TEST_F(ConferenceParticipantListTest, RemoveUserRemovesCorrespondingParticipant)
{
    ConfUser* pUser = new ConfUser();
    pParticipantList->AddUser(pUser);
    EXPECT_EQ(pParticipantList->GetSize(), 1);
    delete pUser;

    ConfUser* pCopiedUser = pParticipantList->GetConfUser((IMS_UINT32)0);

    pParticipantList->RemoveUser(pCopiedUser);
    EXPECT_EQ(pParticipantList->GetSize(), 0);
}

TEST_F(ConferenceParticipantListTest, RemoveUserByIndexRemovesCorrespondingParticipant)
{
    ConfUser* pUser = new ConfUser();
    pParticipantList->AddUser(pUser);
    EXPECT_EQ(pParticipantList->GetSize(), 1);

    pParticipantList->RemoveUser((IMS_UINT32)0);
    EXPECT_EQ(pParticipantList->GetSize(), 0);
    delete pUser;
}

TEST_F(ConferenceParticipantListTest, GetConfUserByConferenceReferenceReturnsCopiedUser)
{
    MockIConferenceReference objMockReference;

    ConfUser* pUser = new ConfUser();
    pParticipantList->AddUser(pUser);
    delete pUser;
    ConfUser* pCopiedUser = pParticipantList->GetConfUser((IMS_UINT32)0);

    pParticipantList->SetReference(&objMockReference, pCopiedUser);
    EXPECT_EQ(pParticipantList->GetConfUser(&objMockReference), pCopiedUser);
}

TEST_F(ConferenceParticipantListTest, GetConfUserByConferenceReferenceReturnsNullAfterReset)
{
    MockIConferenceReference objMockReference;

    ConfUser* pUser = new ConfUser();
    pParticipantList->AddUser(pUser);
    delete pUser;
    ConfUser* pCopiedUser = pParticipantList->GetConfUser((IMS_UINT32)0);

    pParticipantList->SetReference(&objMockReference, pCopiedUser);
    EXPECT_EQ(pParticipantList->GetConfUser(&objMockReference), pCopiedUser);
    pParticipantList->ResetReference(&objMockReference);
    EXPECT_EQ(pParticipantList->GetConfUser(&objMockReference), nullptr);
}

TEST_F(ConferenceParticipantListTest, SetAndGetReferInviteUriUsingParticipantList)
{
    ConfUser* pUser = new ConfUser();
    pParticipantList->AddUser(pUser);
    delete pUser;
    ConfUser* pCopiedUser = pParticipantList->GetConfUser((IMS_UINT32)0);

    AString strUri("sip:testuri@ims.google.com");
    pParticipantList->SetReferInviteUri(strUri, pCopiedUser);

    EXPECT_STREQ(pParticipantList->GetReferInviteUri(pCopiedUser).GetStr(), strUri.GetStr());
}

TEST_F(ConferenceParticipantListTest,
        GetReferInviteUriWithNullUserUsingParticipantListReturnsEmptyString)
{
    EXPECT_STREQ(pParticipantList->GetReferInviteUri(IMS_NULL).GetStr(), "");
}

TEST_F(ConferenceParticipantListTest, FindParticipantUsingConnectionIdReturnsCorrespondingUesr)
{
    const IMS_UINT32 CONNECTION_ID_1 = 1;
    const IMS_UINT32 CONNECTION_ID_2 = 2;
    ConfUser* pUser = new ConfUser();
    pUser->nConnectionId = CONNECTION_ID_1;
    pParticipantList->AddUser(pUser);
    delete pUser;

    pUser = new ConfUser();
    pUser->nConnectionId = CONNECTION_ID_2;
    pParticipantList->AddUser(pUser);
    delete pUser;

    EXPECT_EQ(pParticipantList->FindParticipant(CONNECTION_ID_1), 0);
    EXPECT_EQ(pParticipantList->FindParticipant(CONNECTION_ID_2), 1);
}

TEST_F(ConferenceParticipantListTest, FindParticipantUsingConnectionIdReturnsInvalidValueIfNotFound)
{
    EXPECT_EQ(pParticipantList->FindParticipant(0), -1);
}

TEST_F(ConferenceParticipantListTest, ReOrderRearragnesListByCallCreationOrder)
{
    static const IMS_UINT32 CONNECTION_ID_1 = 1;
    static const IMS_UINT32 CONNECTION_ID_2 = 2;
    static const CallKey CALL_KEY_1 = 1;
    static const CallKey CALL_KEY_2 = 2;

    ConfUser objUser1;
    ConfUser objUser2;
    objUser1.nConnectionId = CONNECTION_ID_2;  // Reverse Order.
    objUser2.nConnectionId = CONNECTION_ID_1;  // Reverse Order.
    pParticipantList->AddUser(&objUser1);
    pParticipantList->AddUser(&objUser2);

    MockIMtcCall objCall1;
    MockIMtcCall objCall2;
    ON_CALL(objCall1, GetKey).WillByDefault(Return(CALL_KEY_1));
    ON_CALL(objCall2, GetKey).WillByDefault(Return(CALL_KEY_2));
    ImsList<IMtcCall*> objCallList;
    objCallList.Append(&objCall1);
    objCallList.Append(&objCall2);

    MockIMtcCallManager objCallManager;
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objCallList));

    MockIMtcContext objContext;
    MockICallStateProxy objCallStateProxy;
    ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
    MockCallConnectionIdManager objIdManager(objContext);

    ON_CALL(objIdManager, GetCallKey(CONNECTION_ID_1)).WillByDefault(Return(CALL_KEY_1));
    ON_CALL(objIdManager, GetCallKey(CONNECTION_ID_2)).WillByDefault(Return(CALL_KEY_2));

    pParticipantList->ReOrder(objCallManager, objIdManager);
    EXPECT_EQ(pParticipantList->GetConfUser((IMS_UINT32)0)->nConnectionId, CONNECTION_ID_1);
    EXPECT_EQ(pParticipantList->GetConfUser(1)->nConnectionId, CONNECTION_ID_2);
}

TEST_F(ConferenceParticipantListTest, ParticipantObtainedUsingIndexHasCorrespondingUser)
{
    ConfUser* pUser = new ConfUser();
    pParticipantList->AddUser(pUser);
    delete pUser;

    ConfUser* pCopiedUser = pParticipantList->GetConfUser((IMS_UINT32)0);

    ASSERT_NE(pParticipantList->GetAt(0), nullptr);
    EXPECT_EQ(pParticipantList->GetAt(0)->GetConfUser(), pCopiedUser);
}

TEST_F(ConferenceParticipantListTest, GetConnectedParticipantSizeReturnsUserCountBeingConnected)
{
    ConfUser* pUser = new ConfUser();
    pUser->eStatus = STATUS_CONNECTED;
    pParticipantList->AddUser(pUser);
    delete pUser;

    pUser = new ConfUser();
    pUser->eStatus = STATUS_ON_HOLD;
    pParticipantList->AddUser(pUser);
    delete pUser;

    pUser = new ConfUser();
    pUser->eStatus = STATUS_IDLE;
    pParticipantList->AddUser(pUser);
    delete pUser;

    pUser = new ConfUser();
    pUser->eStatus = STATUS_ALERTING;
    pParticipantList->AddUser(pUser);
    delete pUser;

    pUser = new ConfUser();
    pUser->eStatus = STATUS_DISCONNECTING;
    pParticipantList->AddUser(pUser);
    delete pUser;

    pUser = new ConfUser();
    pUser->eStatus = STATUS_DISCONNECTED;
    pParticipantList->AddUser(pUser);
    delete pUser;

    EXPECT_EQ(pParticipantList->GetConnectedParticipantSize(IMS_TRUE), 4);
    EXPECT_EQ(pParticipantList->GetConnectedParticipantSize(IMS_FALSE), 2);
}

TEST_F(ConferenceParticipantListTest, LoginDoesNothing)
{
    ConfUser objUser1;
    objUser1.eStatus = STATUS_CONNECTED;
    ConfUser objUser2;
    objUser2.eStatus = STATUS_IDLE;
    pParticipantList->AddUser(&objUser1);
    pParticipantList->AddUser(&objUser2);
    pParticipantList->Login();
    EXPECT_EQ(pParticipantList->GetConnectedParticipantSize(IMS_TRUE), 2);
    EXPECT_EQ(pParticipantList->GetConnectedParticipantSize(IMS_FALSE), 1);
}

TEST_F(ConferenceParticipantListTest, IsConnectedUserReturnsFalseIfUserIsNull)
{
    EXPECT_FALSE(pParticipantList->IsConnectedUser(IMS_NULL));
}

TEST_F(ConferenceParticipantListTest, GetConfUserReturnsCopiedUser)
{
    ConfUser objUser;
    pParticipantList->AddUser(&objUser);  // Copy User

    ASSERT_NE(pParticipantList->GetConfUser((IMS_UINT32)0),
            pParticipantList->GetConfUsers(IMS_TRUE).GetAt(0));
}

TEST_F(ConferenceParticipantListTest, GetConfUserReturnsOriginalUser)
{
    ConfUser objUser;
    pParticipantList->AddUser(&objUser);  // Copy User

    EXPECT_EQ(pParticipantList->GetConfUser((IMS_UINT32)0),
            pParticipantList->GetConfUsers(IMS_FALSE).GetAt(0));
}

TEST_F(ConferenceParticipantListTest, GetConfUserReturnsNull)
{
    EXPECT_EQ(pParticipantList->GetConfUser((IMS_UINT32)0), nullptr);
}

TEST_F(ConferenceParticipantListTest, GetReferenceReturnsNullIfNotFound)
{
    ConfUser objUser;
    EXPECT_EQ(pParticipantList->GetReference(&objUser), nullptr);
}

TEST_F(ConferenceParticipantListTest, GetReferenceReturnsStoredReference)
{
    ConfUser objUser;
    pParticipantList->AddUser(&objUser);
    MockIConferenceReference objReference;
    pParticipantList->SetReference(&objReference, pParticipantList->GetConfUser((IMS_UINT32)0));

    EXPECT_EQ(pParticipantList->GetReference(pParticipantList->GetConfUser((IMS_UINT32)0)),
            &objReference);
}

}  // namespace android
