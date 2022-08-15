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
#include "call/MockIMtcCallContext.h"
#include "media/MtcMediaManager.h"
#include "media/MockIMediaReportEventListener.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL CallKey DEFAULT_CALL_KEY = 1;

class MtcMediaManagerTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MtcMediaManager* pMediaManager;
    MockIMediaReportEventListener* pListener;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallKey).WillByDefault(Return(DEFAULT_CALL_KEY));
        pListener = new MockIMediaReportEventListener();

        pMediaManager = new MtcMediaManager(objContext);

        pMediaManager->SetMediaReportEventListener(pListener);
    }

    virtual void TearDown() override
    {
        delete pMediaManager;
        delete pListener;
    }
};

TEST_F(MtcMediaManagerTest, GetStateReturnsIdleInitially)
{
    EXPECT_EQ(MediaState::IDLE, pMediaManager->GetState());
    EXPECT_EQ(MediaState::IDLE, pMediaManager->GetOldState());
}

TEST_F(MtcMediaManagerTest, TerminateSetsStateToTerminating)
{
    pMediaManager->Terminate();

    EXPECT_EQ(MediaState::TERMINATING, pMediaManager->GetState());
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyWithSuccessReport)
{
    pMediaManager->MediaSession_Notify(REPORT_SUCCESS);

    EXPECT_EQ(MediaState::STARTED, pMediaManager->GetState());
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyWithCloseSessionReport)
{
    pMediaManager->MediaSession_Notify(REPORT_CLOSE_SESSION);

    EXPECT_EQ(MediaState::TERMINATED, pMediaManager->GetState());
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyWithDataReceivedFailedReport)
{
    EXPECT_CALL(*pListener, OnReceivingMediaDataFailed(_, _)).Times(2);

    pMediaManager->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED);
    EXPECT_EQ(IMS_FALSE, pMediaManager->IsAudioInactive());

    pMediaManager->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, MEDIA_TYPE_AUDIO);
    EXPECT_EQ(IMS_TRUE, pMediaManager->IsAudioInactive());
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyWithVideoLowestBitRateReport)
{
    EXPECT_CALL(*pListener, OnVideoLowestBitRate).Times(1);

    pMediaManager->MediaSession_Notify(REPORT_VIDEO_LOWEST_BIT_RATE);
}
