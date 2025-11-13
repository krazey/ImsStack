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

#include "ImsTypeDef.h"
#include "MockIMediaSession.h"
#include "MockISession.h"
#include "media/MtcMediaProfileManager.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

class MtcMediaProfileManagerTest : public ::testing::Test
{
public:
    inline MtcMediaProfileManagerTest() :
            objProfileManager(),
            objSession1(),
            objSession2(),
            objSession3(),
            objMediaSession1(),
            objMediaSession2(),
            objMediaSession3()
    {
    }
    inline ~MtcMediaProfileManagerTest() {}

protected:
    MtcMediaProfileManager objProfileManager;
    MockISession objSession1;
    MockISession objSession2;
    MockISession objSession3;
    MockIMediaSession objMediaSession1;
    MockIMediaSession objMediaSession2;
    MockIMediaSession objMediaSession3;

    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(MtcMediaProfileManagerTest, CreateMediaProfileWithNullSessionDoesNotCreateProfile)
{
    EXPECT_CALL(objMediaSession1, CreateProfile(_, _)).Times(0);
    objProfileManager.CreateMediaProfile(
            IMS_NULL, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);
}

TEST_F(MtcMediaProfileManagerTest, CreateMediaProfileCreatesProfileWithNegoId)
{
    IMS_UINTP nNegoId = 100;
    EXPECT_EQ(objProfileManager.GetNegoId(&objSession1), 0);

    EXPECT_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillOnce(Return(nNegoId));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);

    EXPECT_EQ(objProfileManager.GetNegoId(&objSession1), nNegoId);
}

TEST_F(MtcMediaProfileManagerTest, CreateMediaProfileWithForkedSession)
{
    IMS_UINTP nNegoId = 100;
    EXPECT_EQ(objProfileManager.GetNegoId(&objSession1), 0);

    EXPECT_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillOnce(Return(nNegoId));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_TRUE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);

    EXPECT_EQ(objProfileManager.GetNegoId(&objSession1), nNegoId);
}

TEST_F(MtcMediaProfileManagerTest, CreateMediaProfileWithOriginalProfile)
{
    IMS_UINTP nNegoId = 100;
    EXPECT_EQ(objProfileManager.GetNegoId(&objSession1), 0);

    EXPECT_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillOnce(Return(nNegoId));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_FALSE, IMS_TRUE, MEDIA_TYPE_AUDIO, &objMediaSession1);

    EXPECT_EQ(objProfileManager.GetNegoId(&objSession1), nNegoId);
}

TEST_F(MtcMediaProfileManagerTest, ForkedOriginalProfileGetsNegoIdUsingOriginalSession)
{
    IMS_UINTP nNegoId1 = 100;
    IMS_UINTP nNegoId2 = 200;
    EXPECT_EQ(objProfileManager.GetNegoId(&objSession2), 0);

    ON_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(nNegoId1));
    EXPECT_CALL(objMediaSession2, CreateProfile(nNegoId1, MEDIA_TYPE_AUDIO))
            .WillOnce(Return(nNegoId2));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);
    objProfileManager.CreateMediaProfile(
            &objSession2, IMS_TRUE, IMS_TRUE, MEDIA_TYPE_AUDIO, &objMediaSession2);

    EXPECT_EQ(objProfileManager.GetNegoId(&objSession2), nNegoId2);
}

TEST_F(MtcMediaProfileManagerTest,
        ForkedOriginalProfileGetsNegoIdUsingFirstSessionIfNoOriginalSession)
{
    IMS_UINTP nNegoId1 = 100;
    IMS_UINTP nNegoId2 = 200;
    EXPECT_EQ(objProfileManager.GetNegoId(&objSession2), 0);

    ON_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(nNegoId1));
    EXPECT_CALL(objMediaSession2, CreateProfile(nNegoId1, MEDIA_TYPE_AUDIO))
            .WillOnce(Return(nNegoId2));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_TRUE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);
    objProfileManager.CreateMediaProfile(
            &objSession2, IMS_TRUE, IMS_TRUE, MEDIA_TYPE_AUDIO, &objMediaSession2);

    EXPECT_EQ(objProfileManager.GetNegoId(&objSession2), nNegoId2);
}

TEST_F(MtcMediaProfileManagerTest, DestroyMediaProfileRemovesProfileWithSessionAndMediaSession)
{
    IMS_UINTP nNegoId = 100;
    ON_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(nNegoId));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);

    EXPECT_EQ(objProfileManager.GetNegoId(&objSession1), nNegoId);
    objProfileManager.DestroyMediaProfile(&objSession2, &objMediaSession1);
    EXPECT_EQ(objProfileManager.GetNegoId(&objSession1), nNegoId);
    objProfileManager.DestroyMediaProfile(&objSession1, &objMediaSession1);
    EXPECT_EQ(objProfileManager.GetNegoId(&objSession1), 0);
}

TEST_F(MtcMediaProfileManagerTest, DestroyAllMediaProfilesRemovesAllProfileWithMediaSession)
{
    IMS_UINTP nNegoId1 = 100;
    ON_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(nNegoId1));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);
    objProfileManager.CreateMediaProfile(
            &objSession2, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);

    EXPECT_EQ(objProfileManager.GetNegoId(&objSession1), nNegoId1);
    EXPECT_EQ(objProfileManager.GetNegoId(&objSession2), nNegoId1);

    objProfileManager.DestroyAllMediaProfiles(&objMediaSession1);

    EXPECT_EQ(objProfileManager.GetNegoId(&objSession1), 0);
    EXPECT_EQ(objProfileManager.GetNegoId(&objSession2), 0);
}

TEST_F(MtcMediaProfileManagerTest, GetPemTypeReturnsValueSet)
{
    objProfileManager.SetPemType(IMS_NULL, PemType::SENDRECV);
    EXPECT_EQ(objProfileManager.GetPemType(IMS_NULL), PemType::NONE);

    ON_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(1));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);

    EXPECT_EQ(objProfileManager.GetPemType(&objSession1), PemType::NONE);

    objProfileManager.SetPemType(&objSession1, PemType::SENDRECV);
    EXPECT_EQ(objProfileManager.GetPemType(&objSession1), PemType::SENDRECV);
}

TEST_F(MtcMediaProfileManagerTest, IsActiveReturnsValueSet)
{
    objProfileManager.SetActive(IMS_NULL, IMS_TRUE);
    EXPECT_FALSE(objProfileManager.IsActive(IMS_NULL));

    ON_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(1));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);

    EXPECT_FALSE(objProfileManager.IsActive(&objSession1));

    objProfileManager.SetActive(&objSession1, IMS_TRUE);
    EXPECT_TRUE(objProfileManager.IsActive(&objSession1));
}

TEST_F(MtcMediaProfileManagerTest, IsConfirmedReturnsValueSet)
{
    objProfileManager.SetConfirmed(IMS_NULL, IMS_TRUE);
    EXPECT_FALSE(objProfileManager.IsConfirmed(IMS_NULL));

    ON_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(1));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);

    EXPECT_FALSE(objProfileManager.IsConfirmed(&objSession1));

    objProfileManager.SetConfirmed(&objSession1, IMS_TRUE);
    EXPECT_TRUE(objProfileManager.IsConfirmed(&objSession1));
}

TEST_F(MtcMediaProfileManagerTest, IsForkedReturnsValueSet)
{
    EXPECT_FALSE(objProfileManager.IsForked(IMS_NULL));
    EXPECT_FALSE(objProfileManager.IsForked(&objSession1));

    ON_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(1));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);

    EXPECT_FALSE(objProfileManager.IsForked(&objSession1));

    ON_CALL(objMediaSession2, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(2));
    objProfileManager.CreateMediaProfile(
            &objSession2, IMS_TRUE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession2);
    EXPECT_TRUE(objProfileManager.IsForked(&objSession2));
}

TEST_F(MtcMediaProfileManagerTest, IsPemSendInOtherEarlySessionReturnsTrueIfExists)
{
    ON_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(1));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);
    objProfileManager.SetConfirmed(&objSession1, IMS_TRUE);

    ON_CALL(objMediaSession2, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(2));
    objProfileManager.CreateMediaProfile(
            &objSession2, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession2);
    objProfileManager.SetConfirmed(&objSession2, IMS_FALSE);
    objProfileManager.SetPemType(&objSession2, PemType::SENDONLY);
    objProfileManager.SetActive(&objSession2, IMS_FALSE);

    ON_CALL(objMediaSession3, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(3));
    objProfileManager.CreateMediaProfile(
            &objSession3, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession3);
    objProfileManager.SetConfirmed(&objSession3, IMS_FALSE);
    objProfileManager.SetPemType(&objSession3, PemType::SENDRECV);
    objProfileManager.SetActive(&objSession3, IMS_TRUE);

    MockISession objSession4;
    EXPECT_TRUE(objProfileManager.IsPemSendInOtherEarlySession(&objSession4));
}

TEST_F(MtcMediaProfileManagerTest, IsPemSendInOtherEarlySessionReturnsFalseByCondition)
{
    ON_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(1));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);
    objProfileManager.SetConfirmed(&objSession1, IMS_TRUE);
    EXPECT_FALSE(objProfileManager.IsPemSendInOtherEarlySession(&objSession1));
    EXPECT_FALSE(objProfileManager.IsPemSendInOtherEarlySession(&objSession2));

    ON_CALL(objMediaSession2, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(2));
    objProfileManager.CreateMediaProfile(
            &objSession2, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession2);
    objProfileManager.SetConfirmed(&objSession2, IMS_FALSE);
    objProfileManager.SetPemType(&objSession2, PemType::SENDONLY);
    objProfileManager.SetActive(&objSession2, IMS_FALSE);
    EXPECT_FALSE(objProfileManager.IsPemSendInOtherEarlySession(&objSession1));
    EXPECT_FALSE(objProfileManager.IsPemSendInOtherEarlySession(&objSession2));
    EXPECT_FALSE(objProfileManager.IsPemSendInOtherEarlySession(&objSession3));

    ON_CALL(objMediaSession3, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(3));
    objProfileManager.CreateMediaProfile(
            &objSession3, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession3);
    objProfileManager.SetConfirmed(&objSession3, IMS_FALSE);
    objProfileManager.SetPemType(&objSession3, PemType::SENDRECV);
    objProfileManager.SetActive(&objSession3, IMS_FALSE);

    MockISession objSession4;
    EXPECT_FALSE(objProfileManager.IsPemSendInOtherEarlySession(&objSession4));
}

TEST_F(MtcMediaProfileManagerTest, UpdateProfileForMediaActivationSetsInactiveForAllOtherSessions)
{
    ON_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(1));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);

    ON_CALL(objMediaSession2, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(2));
    objProfileManager.CreateMediaProfile(
            &objSession2, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession2);

    ON_CALL(objMediaSession3, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(3));
    objProfileManager.CreateMediaProfile(
            &objSession3, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession3);

    objProfileManager.SetActive(&objSession1, IMS_TRUE);
    objProfileManager.SetActive(&objSession2, IMS_TRUE);
    objProfileManager.SetActive(&objSession3, IMS_FALSE);

    objProfileManager.UpdateProfileForMediaActivation(&objSession3);
    EXPECT_FALSE(objProfileManager.IsActive(&objSession1));
    EXPECT_FALSE(objProfileManager.IsActive(&objSession2));
    EXPECT_TRUE(objProfileManager.IsActive(&objSession3));

    objProfileManager.UpdateProfileForMediaActivation(&objSession2);
    EXPECT_FALSE(objProfileManager.IsActive(&objSession1));
    EXPECT_TRUE(objProfileManager.IsActive(&objSession2));
    EXPECT_FALSE(objProfileManager.IsActive(&objSession3));
}

TEST_F(MtcMediaProfileManagerTest, GetSessionWithNegoIdReturnsMatchingSession)
{
    IMS_UINTP nNegoId1 = 100;
    IMS_UINTP nNegoId2 = 200;
    IMS_UINTP nNegoId3 = 300;
    IMS_UINTP nInvalidNegoId = 0;

    ON_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(nNegoId1));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);

    ON_CALL(objMediaSession2, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(nNegoId2));
    objProfileManager.CreateMediaProfile(
            &objSession2, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession2);

    ON_CALL(objMediaSession3, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(nNegoId3));
    objProfileManager.CreateMediaProfile(
            &objSession3, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession3);

    EXPECT_EQ(objProfileManager.GetSessionWithNegoId(nNegoId1), &objSession1);
    EXPECT_EQ(objProfileManager.GetSessionWithNegoId(nNegoId2), &objSession2);
    EXPECT_EQ(objProfileManager.GetSessionWithNegoId(nNegoId3), &objSession3);
    EXPECT_EQ(objProfileManager.GetSessionWithNegoId(nInvalidNegoId), nullptr);
}

TEST_F(MtcMediaProfileManagerTest, GetActiveSession)
{
    EXPECT_EQ(nullptr, objProfileManager.GetActiveSession());

    ON_CALL(objMediaSession1, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(1));
    objProfileManager.CreateMediaProfile(
            &objSession1, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession1);

    ON_CALL(objMediaSession2, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(2));
    objProfileManager.CreateMediaProfile(
            &objSession2, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession2);

    ON_CALL(objMediaSession3, CreateProfile(0, MEDIA_TYPE_AUDIO)).WillByDefault(Return(3));
    objProfileManager.CreateMediaProfile(
            &objSession3, IMS_FALSE, IMS_FALSE, MEDIA_TYPE_AUDIO, &objMediaSession3);

    objProfileManager.SetActive(&objSession1, IMS_FALSE);
    objProfileManager.SetActive(&objSession2, IMS_FALSE);
    objProfileManager.SetActive(&objSession3, IMS_TRUE);
    EXPECT_EQ(&objSession3, objProfileManager.GetActiveSession());
}

}  // namespace android
