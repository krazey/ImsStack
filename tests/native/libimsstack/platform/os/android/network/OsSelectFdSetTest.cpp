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

#include "network/OsSelectFdSet.h"

namespace android
{

class OsSelectFdSetTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(OsSelectFdSetTest, CopyConstructor)
{
    OsSelectFdSet objOsSelectFdSet;

    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    OsSelectFdSet objCopiedOsSelectFdSet(objOsSelectFdSet);
}

TEST_F(OsSelectFdSetTest, CopyFrom)
{
    OsSelectFdSet objOsSelectFdSet;
    OsSelectFdSet objCopiedOsSelectFdSet;

    objCopiedOsSelectFdSet.CopyFrom(nullptr);
    objCopiedOsSelectFdSet.CopyFrom(&objOsSelectFdSet);
    objCopiedOsSelectFdSet.CopyFrom(&objCopiedOsSelectFdSet);
}

TEST_F(OsSelectFdSetTest, SetEvent)
{
    OsSelectFdSet objOsSelectFdSet;
    IMS_SINT32 nEvent = 0;
    IMS_SINT32 nFd = -1;

    EXPECT_EQ(0, objOsSelectFdSet.SetEvent(nFd, nEvent));

    nFd = 1;
    nEvent = ImsFdSet::EVENT_READ | ImsFdSet::EVENT_WRITE | ImsFdSet::EVENT_EXCEPT;

    EXPECT_EQ(nEvent, objOsSelectFdSet.SetEvent(nFd, nEvent));
    EXPECT_EQ(0, objOsSelectFdSet.SetEvent(nFd, nEvent));
}

TEST_F(OsSelectFdSetTest, GetSignaledEvents)
{
    OsSelectFdSet objOsSelectFdSet;
    IMS_SINT32 nFd1 = STDIN_FILENO;   // stdin fd, 0
    IMS_SINT32 nFd2 = STDOUT_FILENO;  // stdout fd, 1
    IMS_SINT32 nFd3 = STDERR_FILENO;  // stderr fd, 2
    IMS_SINT32 nSignaledCount = -1;

    EXPECT_EQ(0, objOsSelectFdSet.GetSignaledEvents(nFd1, nSignaledCount));
    EXPECT_EQ(0, nSignaledCount);

    IMS_SINT32 nEvent = ImsFdSet::EVENT_READ | ImsFdSet::EVENT_WRITE | ImsFdSet::EVENT_EXCEPT;

    EXPECT_EQ(nEvent, objOsSelectFdSet.SetEvent(nFd1, nEvent));
    EXPECT_EQ(nEvent, objOsSelectFdSet.SetEvent(nFd2, nEvent));
    EXPECT_EQ(nEvent, objOsSelectFdSet.SetEvent(nFd3, nEvent));

    if (objOsSelectFdSet.IsHighestFdRequired())
    {
        objOsSelectFdSet.SetHighestFd(nFd3 + 1);
    }

    objOsSelectFdSet.WaitForEvents(0);

    objOsSelectFdSet.GetSignaledEvents(nFd1, nSignaledCount);
}

TEST_F(OsSelectFdSetTest, ClearEvent)
{
    OsSelectFdSet objOsSelectFdSet;
    IMS_SINT32 nEvent = ImsFdSet::EVENT_READ | ImsFdSet::EVENT_WRITE | ImsFdSet::EVENT_EXCEPT;
    IMS_SINT32 nFd = 1;

    EXPECT_EQ(0, objOsSelectFdSet.ClearEvent(nFd, nEvent));

    EXPECT_EQ(nEvent, objOsSelectFdSet.SetEvent(nFd, nEvent));

    EXPECT_EQ(0, objOsSelectFdSet.ClearEvent(-1, nEvent));
    EXPECT_EQ(nEvent, objOsSelectFdSet.ClearEvent(nFd, nEvent));
    EXPECT_EQ(0, objOsSelectFdSet.ClearEvent(nFd, nEvent));
}

}  // namespace android