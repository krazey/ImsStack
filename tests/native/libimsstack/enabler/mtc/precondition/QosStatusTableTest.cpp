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

#include "MtcDef.h"
#include "SdpAttribute.h"
#include "SdpMedia.h"
#include "media/MockIMedia.h"
#include "media/MockIMediaDescriptor.h"
#include "offeranswer/SdpPrecondition.h"
#include "offeranswer/SdpSegmentedPrecondition.h"
#include "precondition/QosStatusTable.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

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

    void AssertTableNotChanged(IN IMS_SINT32 eSdpMediaType)
    {
        QosStatusTable objInitialStatusTable;
        objInitialStatusTable.InitializeRecords(eSdpMediaType);

        auto lstRecords = pQosStatusTable->GetRecords(eSdpMediaType);
        auto lstInitialRecords = objInitialStatusTable.GetRecords(eSdpMediaType);

        ASSERT_EQ(lstRecords.GetSize(), lstInitialRecords.GetSize());
        for (IMS_UINT32 i = 0; i < lstRecords.GetSize(); i++)
        {
            EXPECT_EQ(*lstRecords.GetAt(i), *lstInitialRecords.GetAt(i));
        }
    }
};

TEST_F(QosStatusTableTest, QosStatusRecordDefaultValue)
{
    QosStatusRecord objRecord;

    EXPECT_EQ(objRecord.eSdpMediaType, SdpMedia::TYPE_INVALID);
    EXPECT_EQ(objRecord.eAttrType, SdpAttribute::ATTRIBUTE_INVALID);
    EXPECT_EQ(objRecord.eStatusType, SdpPrecondition::STATUS_INVALID);
    EXPECT_EQ(objRecord.eDirTag, SdpPrecondition::DIRECTION_NONE);
    EXPECT_EQ(objRecord.eStrengthTag, SdpPrecondition::STRENGTH_NOTUSED);
    EXPECT_EQ(objRecord.bDesiredCheck, IMS_FALSE);
    EXPECT_EQ(objRecord.bLocalResourceConfirmed, IMS_FALSE);
}

TEST_F(QosStatusTableTest, QosStatusRecordAssignmentOperator)
{
    QosStatusRecord objAssignedRecord;
    QosStatusRecord objRecord(SdpMedia::TYPE_APPLICATION, SdpAttribute::ATTRIBUTE_MAX,
            SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_RECV,
            SdpPrecondition::STRENGTH_MANDATORY);
    objRecord.bDesiredCheck = !objRecord.bDesiredCheck;
    objRecord.bLocalResourceConfirmed = !objRecord.bLocalResourceConfirmed;

    objAssignedRecord = objRecord;

    EXPECT_EQ(objAssignedRecord.eSdpMediaType, objRecord.eSdpMediaType);
    EXPECT_EQ(objAssignedRecord.eAttrType, objRecord.eAttrType);
    EXPECT_EQ(objAssignedRecord.eStatusType, objRecord.eStatusType);
    EXPECT_EQ(objAssignedRecord.eDirTag, objRecord.eDirTag);
    EXPECT_EQ(objAssignedRecord.eStrengthTag, objRecord.eStrengthTag);
    EXPECT_EQ(objAssignedRecord.bDesiredCheck, objRecord.bDesiredCheck);
    EXPECT_EQ(objAssignedRecord.bLocalResourceConfirmed, objRecord.bLocalResourceConfirmed);
}

TEST_F(QosStatusTableTest, QosStatusRecordEqualToOperator)
{
    QosStatusRecord objRecordToCompare(SdpMedia::TYPE_APPLICATION, SdpAttribute::ATTRIBUTE_MAX,
            SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_RECV,
            SdpPrecondition::STRENGTH_MANDATORY);
    QosStatusRecord objRecord(SdpMedia::TYPE_APPLICATION, SdpAttribute::ATTRIBUTE_MAX,
            SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_RECV,
            SdpPrecondition::STRENGTH_MANDATORY);

    EXPECT_TRUE(objRecord == objRecord);
    EXPECT_TRUE(objRecordToCompare == objRecord);
}

TEST_F(QosStatusTableTest, GetRecordsReturnsEmptyListIfNoRecord)
{
    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).IsEmpty());
    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).IsEmpty());
    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).IsEmpty());
}

TEST_F(QosStatusTableTest, GetRecordsReturnsEmptyListWithInvalidMediaType)
{
    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_INVALID).IsEmpty());
}

TEST_F(QosStatusTableTest, InitializeRecordsSetsInitialValueOfAudio)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);

    EXPECT_FALSE(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).IsEmpty());
    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).IsEmpty());
    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).IsEmpty());
}

TEST_F(QosStatusTableTest, InitializeRecordsSetsInitialValueOfVideo)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_VIDEO);

    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).IsEmpty());
    EXPECT_FALSE(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).IsEmpty());
    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).IsEmpty());
}

TEST_F(QosStatusTableTest, InitializeRecordsSetsInitialValueOfText)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_TEXT);

    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).IsEmpty());
    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).IsEmpty());
    EXPECT_FALSE(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).IsEmpty());
}

TEST_F(QosStatusTableTest, InitializeRecordsNotSetInitialValueOfInvalidMediaType)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_INVALID);

    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).IsEmpty());
    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).IsEmpty());
    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).IsEmpty());
}

TEST_F(QosStatusTableTest, RemoveUnusedRecordsRemovesOtherThanAudioRecords)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_VIDEO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_TEXT);

    pQosStatusTable->RemoveUnusedRecords(MEDIATYPE_AUDIO);

    EXPECT_FALSE(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).IsEmpty());
    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).IsEmpty());
    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).IsEmpty());
}

TEST_F(QosStatusTableTest, RemoveUnusedRecordsRemovesOtherThanVideoRecords)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_VIDEO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_TEXT);

    pQosStatusTable->RemoveUnusedRecords(MEDIATYPE_VIDEO);

    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).IsEmpty());
    EXPECT_FALSE(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).IsEmpty());
    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).IsEmpty());
}

TEST_F(QosStatusTableTest, RemoveUnusedRecordsRemovesOtherThanTextRecords)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_VIDEO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_TEXT);

    pQosStatusTable->RemoveUnusedRecords(MEDIATYPE_TEXT);

    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).IsEmpty());
    EXPECT_TRUE(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).IsEmpty());
    EXPECT_FALSE(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).IsEmpty());
}

TEST_F(QosStatusTableTest, RemoveUnusedRecordsNotRemoveRecordsIfAllUsed)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_VIDEO);
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_TEXT);

    pQosStatusTable->RemoveUnusedRecords(MEDIATYPE_AUDIO | MEDIATYPE_VIDEO | MEDIATYPE_TEXT);

    EXPECT_FALSE(pQosStatusTable->GetRecords(SdpMedia::TYPE_AUDIO).IsEmpty());
    EXPECT_FALSE(pQosStatusTable->GetRecords(SdpMedia::TYPE_VIDEO).IsEmpty());
    EXPECT_FALSE(pQosStatusTable->GetRecords(SdpMedia::TYPE_TEXT).IsEmpty());
}

TEST_F(QosStatusTableTest, UpdateStatusTableWithRemoteSdpDoesNothingIfMediaDescriptorIsNull)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);

    MockIMedia objMedia;

    ON_CALL(objMedia, GetUpdateState).WillByDefault(Return(IMedia::UPDATE_MODIFIED));
    ON_CALL(objMedia, GetProposal).WillByDefault(Return(&objMedia));
    ON_CALL(objMedia, GetMediaDescriptor).WillByDefault(Return(nullptr));

    pQosStatusTable->UpdateStatusTableWithRemoteSdp(objMedia);

    AssertTableNotChanged(SdpMedia::TYPE_AUDIO);

    ON_CALL(objMedia, GetUpdateState).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objMedia, GetMediaDescriptor).WillByDefault(Return(nullptr));

    pQosStatusTable->UpdateStatusTableWithRemoteSdp(objMedia);

    AssertTableNotChanged(SdpMedia::TYPE_AUDIO);
}

TEST_F(QosStatusTableTest, UpdateStatusTableWithRemoteSdpDoesNothingIfRemoteSdpIsNull)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);

    MockIMedia objMedia;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(objMedia, GetUpdateState).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objMedia, GetMediaDescriptor).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objMediaDescriptor, GetMediaDescriptionEx).WillByDefault(Return(nullptr));

    pQosStatusTable->UpdateStatusTableWithRemoteSdp(objMedia);

    AssertTableNotChanged(SdpMedia::TYPE_AUDIO);
}

TEST_F(QosStatusTableTest, UpdateStatusTableWithRemoteSdpDoesNothingIfRemoteSdpPortIsZero)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);

    MockIMedia objMedia;
    MockIMediaDescriptor objMediaDescriptor;
    SdpMedia objSdp;

    ON_CALL(objMedia, GetUpdateState).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objMedia, GetMediaDescriptor).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objMediaDescriptor, GetMediaDescriptionEx).WillByDefault(Return(&objSdp));
    objSdp.SetPort(0);

    pQosStatusTable->UpdateStatusTableWithRemoteSdp(objMedia);

    AssertTableNotChanged(SdpMedia::TYPE_AUDIO);
}

TEST_F(QosStatusTableTest,
        UpdateStatusTableWithRemoteSdpNotUpdatesCurrentStatusIfPreconditionIsNull)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR,
            SdpPrecondition::STATUS_REMOTE, SdpPrecondition::DIRECTION_SEND);

    MockIMedia objMedia;
    MockIMediaDescriptor objMediaDescriptor;
    SdpMedia objSdp;
    ON_CALL(objMedia, GetUpdateState).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objMedia, GetMediaDescriptor).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objMediaDescriptor, GetMediaDescriptionEx).WillByDefault(Return(&objSdp));
    objSdp.SetPort(1);
    objSdp.SetType(SdpMedia::TYPE_AUDIO);
    ON_CALL(objMediaDescriptor, GetPrecondition(_, _)).WillByDefault(Return(nullptr));

    pQosStatusTable->UpdateStatusTableWithRemoteSdp(objMedia);

    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::CURR, SdpPrecondition::STATUS_REMOTE),
            SdpPrecondition::DIRECTION_SEND);
}

TEST_F(QosStatusTableTest, UpdateStatusTableWithRemoteSdpUpdatesCurrentStatus)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR,
            SdpPrecondition::STATUS_REMOTE, SdpPrecondition::DIRECTION_SEND);

    MockIMedia objMedia;
    MockIMediaDescriptor objMediaDescriptor;
    SdpMedia objSdp;
    SdpSegmentedPrecondition objSdpPrecondition;
    ON_CALL(objMedia, GetUpdateState).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objMedia, GetMediaDescriptor).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objMediaDescriptor, GetMediaDescriptionEx).WillByDefault(Return(&objSdp));
    objSdp.SetPort(1);
    objSdp.SetType(SdpMedia::TYPE_AUDIO);
    ON_CALL(objMediaDescriptor, GetPrecondition(SdpAttribute::CURR, SdpPrecondition::TYPE_QOS))
            .WillByDefault(Return(&objSdpPrecondition));
    objSdpPrecondition.AddStatus(SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_NONE,
            SdpPrecondition::STRENGTH_NOTUSED);
    objSdpPrecondition.AddStatus(SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_SENDRECV,
            SdpPrecondition::STRENGTH_NOTUSED);

    pQosStatusTable->UpdateStatusTableWithRemoteSdp(objMedia);

    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::CURR, SdpPrecondition::STATUS_REMOTE),
            SdpPrecondition::DIRECTION_SENDRECV);
}

TEST_F(QosStatusTableTest,
        UpdateStatusTableWithRemoteSdpNotUpdatesDesiredStatusIfPreconditionIsNull)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);

    MockIMedia objMedia;
    MockIMediaDescriptor objMediaDescriptor;
    SdpMedia objSdp;
    ON_CALL(objMedia, GetUpdateState).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objMedia, GetMediaDescriptor).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objMediaDescriptor, GetMediaDescriptionEx).WillByDefault(Return(&objSdp));
    objSdp.SetPort(1);
    objSdp.SetType(SdpMedia::TYPE_AUDIO);
    ON_CALL(objMediaDescriptor, GetPrecondition(_, _)).WillByDefault(Return(nullptr));

    pQosStatusTable->UpdateStatusTableWithRemoteSdp(objMedia);

    AssertTableNotChanged(SdpMedia::TYPE_AUDIO);
}

TEST_F(QosStatusTableTest, UpdateStatusTableWithRemoteSdpUpdatesDesiredStatus)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);

    MockIMedia objMedia;
    MockIMediaDescriptor objMediaDescriptor;
    SdpMedia objSdp;
    SdpSegmentedPrecondition objSdpPrecondition;
    ON_CALL(objMedia, GetUpdateState).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objMedia, GetMediaDescriptor).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objMediaDescriptor, GetMediaDescriptionEx).WillByDefault(Return(&objSdp));
    objSdp.SetPort(1);
    objSdp.SetType(SdpMedia::TYPE_AUDIO);
    ON_CALL(objMediaDescriptor, GetPrecondition(SdpAttribute::DES, SdpPrecondition::TYPE_QOS))
            .WillByDefault(Return(&objSdpPrecondition));
    objSdpPrecondition.AddStatus(SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_NONE,
            SdpPrecondition::STRENGTH_OPTIONAL);
    objSdpPrecondition.AddStatus(SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_SENDRECV,
            SdpPrecondition::STRENGTH_NOTUSED);
    objSdpPrecondition.AddStatus(SdpPrecondition::STATUS_REMOTE, SdpPrecondition::DIRECTION_NONE,
            SdpPrecondition::STRENGTH_MANDATORY);
    objSdpPrecondition.AddStatus(SdpPrecondition::STATUS_REMOTE,
            SdpPrecondition::DIRECTION_SENDRECV, SdpPrecondition::STRENGTH_NOTUSED);

    pQosStatusTable->UpdateStatusTableWithRemoteSdp(objMedia);

    EXPECT_EQ(pQosStatusTable->GetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_LOCAL,
                      SdpPrecondition::DIRECTION_SEND),
            SdpPrecondition::STRENGTH_NOTUSED);
    EXPECT_EQ(pQosStatusTable->GetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_LOCAL,
                      SdpPrecondition::DIRECTION_RECV),
            SdpPrecondition::STRENGTH_NOTUSED);
    EXPECT_EQ(pQosStatusTable->GetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
                      SdpPrecondition::DIRECTION_SEND),
            SdpPrecondition::STRENGTH_NOTUSED);
    EXPECT_EQ(pQosStatusTable->GetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
                      SdpPrecondition::DIRECTION_RECV),
            SdpPrecondition::STRENGTH_NOTUSED);
}

TEST_F(QosStatusTableTest, UpdateStatusTableWithRemoteSdpUpdatesUnusedDesiredStatus)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);

    MockIMedia objMedia;
    MockIMediaDescriptor objMediaDescriptor;
    SdpMedia objSdp;
    SdpSegmentedPrecondition objSdpPrecondition;
    ON_CALL(objMedia, GetUpdateState).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objMedia, GetMediaDescriptor).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objMediaDescriptor, GetMediaDescriptionEx).WillByDefault(Return(&objSdp));
    objSdp.SetPort(1);
    objSdp.SetType(SdpMedia::TYPE_AUDIO);
    ON_CALL(objMediaDescriptor, GetPrecondition(SdpAttribute::DES, SdpPrecondition::TYPE_QOS))
            .WillByDefault(Return(&objSdpPrecondition));

    pQosStatusTable->UpdateStatusTableWithRemoteSdp(objMedia);

    EXPECT_EQ(pQosStatusTable->GetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_LOCAL,
                      SdpPrecondition::DIRECTION_SEND),
            SdpPrecondition::STRENGTH_NOTUSED);
    EXPECT_EQ(pQosStatusTable->GetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_LOCAL,
                      SdpPrecondition::DIRECTION_RECV),
            SdpPrecondition::STRENGTH_NOTUSED);
    EXPECT_EQ(pQosStatusTable->GetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
                      SdpPrecondition::DIRECTION_SEND),
            SdpPrecondition::STRENGTH_NOTUSED);
    EXPECT_EQ(pQosStatusTable->GetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
                      SdpPrecondition::DIRECTION_RECV),
            SdpPrecondition::STRENGTH_NOTUSED);
}

TEST_F(QosStatusTableTest, UpdateLocalCurrentStatusNotEnablesIfAlreadyEnabled)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR,
            SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_SENDRECV);
    // Desired local directions are "sendrecv" after initialization

    pQosStatusTable->UpdateLocalCurrentStatus(SdpMedia::TYPE_AUDIO, IMS_TRUE);
    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::CURR, SdpPrecondition::STATUS_LOCAL),
            SdpPrecondition::DIRECTION_SENDRECV);
}

TEST_F(QosStatusTableTest, UpdateLocalCurrentStatusEnablesLocalDirectionToDesiredStatusIfEnabled)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR,
            SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_NONE);
    // Desired local directions are "sendrecv" after initialization

    pQosStatusTable->UpdateLocalCurrentStatus(SdpMedia::TYPE_AUDIO, IMS_TRUE);
    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::CURR, SdpPrecondition::STATUS_LOCAL),
            SdpPrecondition::DIRECTION_SENDRECV);
}

TEST_F(QosStatusTableTest, UpdateLocalCurrentStatusDisablesLocalDirectionToNone)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR,
            SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_SENDRECV);
    // Desired local directions are "sendrecv" after initialization

    pQosStatusTable->UpdateLocalCurrentStatus(SdpMedia::TYPE_AUDIO, IMS_FALSE);
    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::CURR, SdpPrecondition::STATUS_LOCAL),
            SdpPrecondition::DIRECTION_NONE);
}

TEST_F(QosStatusTableTest, EnableRemoteCurrentStatusNotUpdateIfAlreadyEnabled)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR,
            SdpPrecondition::STATUS_REMOTE, SdpPrecondition::DIRECTION_SEND);
    // Desired local directions are "sendrecv" after initialization

    pQosStatusTable->EnableRemoteCurrentStatus(SdpMedia::TYPE_AUDIO);
    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::CURR, SdpPrecondition::STATUS_REMOTE),
            SdpPrecondition::DIRECTION_SEND);
}

TEST_F(QosStatusTableTest, EnableRemoteCurrentStatusUpdatesRemoteDirectionToDesiredStatus)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR,
            SdpPrecondition::STATUS_REMOTE, SdpPrecondition::DIRECTION_NONE);
    // Desired local directions are "sendrecv" after initialization

    pQosStatusTable->EnableRemoteCurrentStatus(SdpMedia::TYPE_AUDIO);
    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::CURR, SdpPrecondition::STATUS_REMOTE),
            SdpPrecondition::DIRECTION_SENDRECV);
}

TEST_F(QosStatusTableTest, IsCurrentStatusEnabledReturnsFalseIfCurrentDirectionIsNone)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    // Current local directions are "none" after initialization

    EXPECT_FALSE(pQosStatusTable->IsCurrentStatusEnabled(
            SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_LOCAL));
}

TEST_F(QosStatusTableTest, IsCurrentStatusEnabledReturnsFalseIfCurrentDirectionIsLowerThanDesired)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    // Desired local directions are "sendrecv" after initialization

    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR,
            SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_SEND);
    EXPECT_FALSE(pQosStatusTable->IsCurrentStatusEnabled(
            SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_LOCAL));

    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR,
            SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_RECV);
    EXPECT_FALSE(pQosStatusTable->IsCurrentStatusEnabled(
            SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_LOCAL));
}

TEST_F(QosStatusTableTest, IsCurrentStatusEnabledReturnsTrueIfCurrentDirectionIsNotLowerThanDesired)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    // Desired local directions are "sendrecv" after initialization

    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR,
            SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_SENDRECV);

    EXPECT_TRUE(pQosStatusTable->IsCurrentStatusEnabled(
            SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_LOCAL));
}

TEST_F(QosStatusTableTest, GetDirectionTagReturnsInvalidIfNoRecord)
{
    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR,
            SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_SEND);
    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::CURR, SdpPrecondition::STATUS_LOCAL),
            SdpPrecondition::DIRECTION_INVALID);

    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_INVALID, SdpAttribute::CURR,
            SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_SEND);
    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_INVALID, SdpAttribute::CURR, SdpPrecondition::STATUS_LOCAL),
            SdpPrecondition::DIRECTION_INVALID);
}

TEST_F(QosStatusTableTest, GetDirectionTagReturnsDirection)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::CURR, SdpPrecondition::STATUS_LOCAL),
            SdpPrecondition::DIRECTION_NONE);

    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR,
            SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_SEND);
    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::CURR, SdpPrecondition::STATUS_LOCAL),
            SdpPrecondition::DIRECTION_SEND);

    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR,
            SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_RECV);
    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::CURR, SdpPrecondition::STATUS_LOCAL),
            SdpPrecondition::DIRECTION_RECV);

    pQosStatusTable->SetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR,
            SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_SENDRECV);
    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::CURR, SdpPrecondition::STATUS_LOCAL),
            SdpPrecondition::DIRECTION_SENDRECV);
}

TEST_F(QosStatusTableTest, GetDirectionTagForDesReturnsSendIfOnlySendStrengthIsNotNone)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);

    // SetStrengthTag sets the desired status
    pQosStatusTable->SetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
            SdpPrecondition::DIRECTION_RECV, SdpPrecondition::STRENGTH_NOTUSED);

    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::DES, SdpPrecondition::STATUS_REMOTE),
            SdpPrecondition::DIRECTION_SEND);
}

TEST_F(QosStatusTableTest, GetDirectionTagForDesReturnsRecvIfOnlyRecvStrengthIsNotNone)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);

    // SetStrengthTag sets the desired status
    pQosStatusTable->SetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
            SdpPrecondition::DIRECTION_SEND, SdpPrecondition::STRENGTH_NOTUSED);

    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::DES, SdpPrecondition::STATUS_REMOTE),
            SdpPrecondition::DIRECTION_RECV);
}

TEST_F(QosStatusTableTest, GetDirectionTagForDesReturnsSendRecvIfBothStrengthAreNotNone)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    // Strength tags of des - remote - send/recv are "optional" after initialization

    EXPECT_EQ(pQosStatusTable->GetDirectionTag(
                      SdpMedia::TYPE_AUDIO, SdpAttribute::DES, SdpPrecondition::STATUS_REMOTE),
            SdpPrecondition::DIRECTION_SENDRECV);
}

TEST_F(QosStatusTableTest, GetStrengthTagReturnsNotUsedIfNoRecord)
{
    pQosStatusTable->SetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
            SdpPrecondition::DIRECTION_SEND, SdpPrecondition::STRENGTH_MANDATORY);
    EXPECT_EQ(pQosStatusTable->GetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
                      SdpPrecondition::DIRECTION_SEND),
            SdpPrecondition::STRENGTH_NOTUSED);

    pQosStatusTable->SetStrengthTag(SdpMedia::TYPE_INVALID, SdpPrecondition::STATUS_REMOTE,
            SdpPrecondition::DIRECTION_SEND, SdpPrecondition::STRENGTH_MANDATORY);
    EXPECT_EQ(pQosStatusTable->GetStrengthTag(SdpMedia::TYPE_INVALID,
                      SdpPrecondition::STATUS_REMOTE, SdpPrecondition::DIRECTION_SEND),
            SdpPrecondition::STRENGTH_NOTUSED);
}

TEST_F(QosStatusTableTest, SetStrengthTagUpgradeStrength)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    EXPECT_EQ(pQosStatusTable->GetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
                      SdpPrecondition::DIRECTION_SEND),
            SdpPrecondition::STRENGTH_OPTIONAL);

    pQosStatusTable->SetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
            SdpPrecondition::DIRECTION_SEND, SdpPrecondition::STRENGTH_MANDATORY);
    EXPECT_EQ(pQosStatusTable->GetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
                      SdpPrecondition::DIRECTION_SEND),
            SdpPrecondition::STRENGTH_MANDATORY);
}

TEST_F(QosStatusTableTest, SetStrengthTagNotDowngradeStrengthExceptNotUsed)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    pQosStatusTable->SetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
            SdpPrecondition::DIRECTION_SEND, SdpPrecondition::STRENGTH_MANDATORY);

    pQosStatusTable->SetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
            SdpPrecondition::DIRECTION_SEND, SdpPrecondition::STRENGTH_OPTIONAL);
    EXPECT_EQ(pQosStatusTable->GetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
                      SdpPrecondition::DIRECTION_SEND),
            SdpPrecondition::STRENGTH_MANDATORY);

    pQosStatusTable->SetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
            SdpPrecondition::DIRECTION_SEND, SdpPrecondition::STRENGTH_NOTUSED);
    EXPECT_EQ(pQosStatusTable->GetStrengthTag(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE,
                      SdpPrecondition::DIRECTION_SEND),
            SdpPrecondition::STRENGTH_NOTUSED);
}

TEST_F(QosStatusTableTest, IsLocalResourceConfirmedReturnsFalseIfNoRecord)
{
    pQosStatusTable->SetLocalResourceConfirmed(SdpMedia::TYPE_AUDIO, IMS_TRUE);
    EXPECT_FALSE(pQosStatusTable->IsLocalResourceConfirmed(SdpMedia::TYPE_AUDIO));

    pQosStatusTable->SetLocalResourceConfirmed(SdpMedia::TYPE_INVALID, IMS_TRUE);
    EXPECT_FALSE(pQosStatusTable->IsLocalResourceConfirmed(SdpMedia::TYPE_INVALID));
}

TEST_F(QosStatusTableTest, IsLocalResourceConfirmedReturnsSetValue)
{
    pQosStatusTable->InitializeRecords(SdpMedia::TYPE_AUDIO);
    EXPECT_FALSE(pQosStatusTable->IsLocalResourceConfirmed(SdpMedia::TYPE_AUDIO));

    pQosStatusTable->SetLocalResourceConfirmed(SdpMedia::TYPE_AUDIO, IMS_TRUE);
    EXPECT_TRUE(pQosStatusTable->IsLocalResourceConfirmed(SdpMedia::TYPE_AUDIO));
}
