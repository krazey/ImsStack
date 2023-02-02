
/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include <unistd.h>

#include "network/OsPollFdSet.h"

namespace android
{

class OsPollFdSetTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(OsPollFdSetTest, CopyConstructor)
{
    OsPollFdSet objOsPollFdSet;

    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    OsPollFdSet objCopiedOsPollFdSet(objOsPollFdSet);
}

TEST_F(OsPollFdSetTest, CopyFrom)
{
    OsPollFdSet objOsPollFdSet;
    OsPollFdSet objCopiedOsPollFdSet;

    objCopiedOsPollFdSet.CopyFrom(nullptr);
    objCopiedOsPollFdSet.CopyFrom(&objOsPollFdSet);
    objCopiedOsPollFdSet.CopyFrom(&objCopiedOsPollFdSet);
}

TEST_F(OsPollFdSetTest, SetEvent)
{
    OsPollFdSet objOsPollFdSet;

    IMS_SINT32 nEvent = 0;
    IMS_SINT32 nFd = -1;

    EXPECT_EQ(0, objOsPollFdSet.SetEvent(nFd, nEvent));

    nEvent = ImsFdSet::EVENT_TCP | ImsFdSet::EVENT_EXCEPT;
    nFd = 1;

    EXPECT_EQ(0, objOsPollFdSet.SetEvent(nFd, nEvent));

    nEvent = ImsFdSet::EVENT_READ | ImsFdSet::EVENT_WRITE | ImsFdSet::EVENT_EXCEPT |
            ImsFdSet::EVENT_TCP_C;

    EXPECT_EQ((nEvent & ~ImsFdSet::EVENT_TCP_C), objOsPollFdSet.SetEvent(nFd, nEvent));
    EXPECT_EQ(0, objOsPollFdSet.SetEvent(nFd, nEvent));
}

TEST_F(OsPollFdSetTest, GetSignaledEvents)
{
    OsPollFdSet objOsPollFdSet;

    IMS_SINT32 nFd = STDOUT_FILENO;  // stdout fd
    IMS_SINT32 nSignaledCount = -1;

    EXPECT_EQ(0, objOsPollFdSet.GetSignaledEvents(nFd, nSignaledCount));
    EXPECT_EQ(0, nSignaledCount);

    IMS_SINT32 nEvent = ImsFdSet::EVENT_READ | ImsFdSet::EVENT_WRITE;

    EXPECT_EQ(nEvent, objOsPollFdSet.SetEvent(nFd, nEvent));

    EXPECT_EQ(1, objOsPollFdSet.WaitForEvents(0));

    EXPECT_EQ(ImsFdSet::EVENT_WRITE, objOsPollFdSet.GetSignaledEvents(nFd, nSignaledCount));
    EXPECT_EQ(1, nSignaledCount);
}

TEST_F(OsPollFdSetTest, ClearEvent)
{
    OsPollFdSet objOsPollFdSet;

    IMS_SINT32 nEvent = ImsFdSet::EVENT_READ | ImsFdSet::EVENT_WRITE | ImsFdSet::EVENT_EXCEPT |
            ImsFdSet::EVENT_TCP_C;
    IMS_SINT32 nFd = 1;

    EXPECT_EQ(0, objOsPollFdSet.ClearEvent(nFd, nEvent));

    EXPECT_EQ((nEvent & ~ImsFdSet::EVENT_TCP_C), objOsPollFdSet.SetEvent(nFd, nEvent));

    EXPECT_EQ((nEvent & ~ImsFdSet::EVENT_TCP_C), objOsPollFdSet.ClearEvent(nFd, nEvent));
}

}  // namespace android
