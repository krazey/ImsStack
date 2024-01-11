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

#include "precondition/QosStatusTable.h"
#include "sdp/SdpMedia.h"
#include <gtest/gtest.h>

class QosStatusTableTest : public ::testing::Test
{
public:
    inline QosStatusTableTest() :
            pQosStatusTable(IMS_NULL)
    {
    }

public:
    QosStatusTable* pQosStatusTable;

protected:
    virtual void SetUp() override { pQosStatusTable = new QosStatusTable(); }

    virtual void TearDown() override { delete pQosStatusTable; }
};

TEST_F(QosStatusTableTest, GetRecordsReturnsEmptyListInitially)
{
    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).GetSize(), 0);
    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).GetSize(), 0);
    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).GetSize(), 0);
}

TEST_F(QosStatusTableTest, GetRecordsReturnsEmptyListWithInvalidMediaType)
{
    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_INVALID).GetSize(), 0);
}

TEST_F(QosStatusTableTest, InitializeRecordsSetsInitialValueOfAudio)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);

    EXPECT_GT(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).GetSize(), 0);
    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).GetSize(), 0);
    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).GetSize(), 0);
}

TEST_F(QosStatusTableTest, InitializeRecordsSetsInitialValueOfVideo)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_VIDEO);

    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).GetSize(), 0);
    EXPECT_GT(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).GetSize(), 0);
    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).GetSize(), 0);
}

TEST_F(QosStatusTableTest, InitializeRecordsSetsInitialValueOfText)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_TEXT);

    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).GetSize(), 0);
    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).GetSize(), 0);
    EXPECT_GT(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).GetSize(), 0);
}

TEST_F(QosStatusTableTest, InitializeRecordsNotSetInitialValueOfInvalidMediaType)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_INVALID);

    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).GetSize(), 0);
    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).GetSize(), 0);
    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).GetSize(), 0);
}

TEST_F(QosStatusTableTest, ClearRecordsClearsRecordsOfAudio)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_VIDEO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_TEXT);

    pQosStatusTable->ClearRecords(SdpMedia::TYPE_AUDIO);

    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).GetSize(), 0);
    EXPECT_GT(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).GetSize(), 0);
    EXPECT_GT(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).GetSize(), 0);
}

TEST_F(QosStatusTableTest, ClearRecordsClearsRecordsOfVideo)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_VIDEO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_TEXT);

    pQosStatusTable->ClearRecords(SdpMedia::TYPE_VIDEO);

    EXPECT_GT(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).GetSize(), 0);
    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).GetSize(), 0);
    EXPECT_GT(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).GetSize(), 0);
}

TEST_F(QosStatusTableTest, ClearRecordsClearsRecordsOfText)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_VIDEO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_TEXT);

    pQosStatusTable->ClearRecords(SdpMedia::TYPE_TEXT);

    EXPECT_GT(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).GetSize(), 0);
    EXPECT_GT(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).GetSize(), 0);
    EXPECT_EQ(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).GetSize(), 0);
}

TEST_F(QosStatusTableTest, ClearRecordsNotClearRecordsOfInvalidMediaType)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_VIDEO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_TEXT);

    pQosStatusTable->ClearRecords(SdpMedia::TYPE_INVALID);

    EXPECT_GT(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).GetSize(), 0);
    EXPECT_GT(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).GetSize(), 0);
    EXPECT_GT(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).GetSize(), 0);
}

TEST_F(QosStatusTableTest, IsLocalResourceConfirmedReturnsFalseIfNoQosStatusRecord)
{
    pQosStatusTable->SetLocalResourceConfirmed(SdpMedia::TYPE_AUDIO, IMS_TRUE);
    EXPECT_FALSE(pQosStatusTable->IsLocalResourceConfirmed(SdpMedia::TYPE_AUDIO));
}

TEST_F(QosStatusTableTest, IsLocalResourceConfirmedReturnsTheSetValue)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    EXPECT_FALSE(pQosStatusTable->IsLocalResourceConfirmed(SdpMedia::TYPE_AUDIO));

    pQosStatusTable->SetLocalResourceConfirmed(SdpMedia::TYPE_AUDIO, IMS_TRUE);
    EXPECT_TRUE(pQosStatusTable->IsLocalResourceConfirmed(SdpMedia::TYPE_AUDIO));
}
