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

TEST_F(QosStatusTableTest, IsLocalResourceConfirmedReturnsFalseIfNoQosStatusRecord)
{
    pQosStatusTable->SetLocalResourceConfirmed(SdpMedia::TYPE_AUDIO, IMS_TRUE);
    EXPECT_FALSE(pQosStatusTable->IsLocalResourceConfirmed(SdpMedia::TYPE_AUDIO));
}

TEST_F(QosStatusTableTest, IsLocalResourceConfirmedReturnsTheSetValue)
{
    pQosStatusTable->CreateStatusRecords(SdpMedia::TYPE_AUDIO);
    EXPECT_FALSE(pQosStatusTable->IsLocalResourceConfirmed(SdpMedia::TYPE_AUDIO));

    pQosStatusTable->SetLocalResourceConfirmed(SdpMedia::TYPE_AUDIO, IMS_TRUE);
    EXPECT_TRUE(pQosStatusTable->IsLocalResourceConfirmed(SdpMedia::TYPE_AUDIO));
}
