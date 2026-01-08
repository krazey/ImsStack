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

#include "helper/ISrvccStateListener.h"
#include "helper/MockISrvccStateListener.h"
#include "helper/SrvccStateManager.h"
#include <gtest/gtest.h>

namespace android
{

class SrvccStateManagerTest : public ::testing::Test
{
public:
    SrvccStateManager* pManager;

protected:
    virtual void SetUp() override { pManager = new SrvccStateManager(); }

    virtual void TearDown() override { delete pManager; }
};

TEST_F(SrvccStateManagerTest, GetStateReturnsIdleInitially)
{
    EXPECT_EQ(pManager->GetState(), SrvccState::IDLE);
}

TEST_F(SrvccStateManagerTest, SetAndGetStartedState)
{
    pManager->UpdateSrvccState(SrvccState::STARTED);
    EXPECT_EQ(pManager->GetState(), SrvccState::STARTED);
}

TEST_F(SrvccStateManagerTest, SetStateOtherThanStartedResetState)
{
    pManager->UpdateSrvccState(SrvccState::SUCCEEDED);
    EXPECT_EQ(pManager->GetState(), SrvccState::IDLE);

    pManager->UpdateSrvccState(SrvccState::FAILED);
    EXPECT_EQ(pManager->GetState(), SrvccState::IDLE);

    pManager->UpdateSrvccState(SrvccState::CANCELED);
    EXPECT_EQ(pManager->GetState(), SrvccState::IDLE);
}

TEST_F(SrvccStateManagerTest, SetStateNotifiesListener)
{
    SrvccState eAnyState = SrvccState::STARTED;
    MockISrvccStateListener objListener;
    EXPECT_CALL(objListener, OnSrvccStateUpdated(eAnyState)).Times(1);

    pManager->AddListener(&objListener);

    pManager->UpdateSrvccState(eAnyState);
}

TEST_F(SrvccStateManagerTest, UpdateSrvccStateTwiceWithSameStateDoesNothing)
{
    SrvccState eAnyState = SrvccState::STARTED;
    MockISrvccStateListener objListener;
    EXPECT_CALL(objListener, OnSrvccStateUpdated(eAnyState)).Times(1);

    pManager->AddListener(&objListener);

    pManager->UpdateSrvccState(eAnyState);
    pManager->UpdateSrvccState(eAnyState);
}

TEST_F(SrvccStateManagerTest, SetStateDoesNotNotifyListenerAfterRemoveListener)
{
    SrvccState eAnyState = SrvccState::STARTED;
    MockISrvccStateListener objListener;
    EXPECT_CALL(objListener, OnSrvccStateUpdated(eAnyState)).Times(0);

    pManager->AddListener(&objListener);
    pManager->RemoveListener(&objListener);

    pManager->UpdateSrvccState(eAnyState);
}

TEST_F(SrvccStateManagerTest, AddListenerTwiceWithSameListenerDoesNothing)
{
    SrvccState eAnyState = SrvccState::STARTED;
    MockISrvccStateListener objListener;
    EXPECT_CALL(objListener, OnSrvccStateUpdated(eAnyState)).Times(0);

    pManager->AddListener(&objListener);
    pManager->AddListener(&objListener);
    pManager->RemoveListener(&objListener);

    pManager->UpdateSrvccState(eAnyState);
}

}  // namespace android
