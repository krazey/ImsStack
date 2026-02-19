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

#include "MockISession.h"
#include "SdpAttribute.h"
#include "SdpMedia.h"
#include "media/IMedia.h"
#include "media/MockIMedia.h"
#include "media/MockIMediaDescriptor.h"
#include "offeranswer/SdpPrecondition.h"
#include "precondition/MockQosStatusTable.h"
#include "precondition/SdpPreconditionHelper.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

class SdpPreconditionHelperTest : public ::testing::Test
{
public:
    inline SdpPreconditionHelperTest() :
            objISession(),
            objQosStatusTable(),
            lstMedias(),
            objAudioMedia(),
            objAudioMediaDescriptor(),
            objAudioSdpMedia(),
            objVideoMedia(),
            objVideoMediaDescriptor(),
            objVideoSdpMedia(),
            pSdpPreconditionHelper(IMS_NULL)
    {
    }

public:
    MockISession objISession;
    MockQosStatusTable objQosStatusTable;
    ImsList<IMedia*> lstMedias;
    MockIMedia objAudioMedia;
    MockIMediaDescriptor objAudioMediaDescriptor;
    SdpMedia objAudioSdpMedia;
    MockIMedia objVideoMedia;
    MockIMediaDescriptor objVideoMediaDescriptor;
    SdpMedia objVideoSdpMedia;
    SdpPreconditionHelper* pSdpPreconditionHelper;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_CREATED));

        ON_CALL(objAudioMedia, GetState()).WillByDefault(Return(IMedia::STATE_ACTIVE));
        ON_CALL(objAudioMedia, GetUpdateState()).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
        ON_CALL(objAudioMedia, GetMediaDescriptor())
                .WillByDefault(Return(&objAudioMediaDescriptor));
        ON_CALL(objAudioMediaDescriptor, GetMediaDescriptionExAsLocal())
                .WillByDefault(Return(&objAudioSdpMedia));
        objAudioSdpMedia.SetType(SdpMedia::TYPE_AUDIO);

        ON_CALL(objVideoMedia, GetState()).WillByDefault(Return(IMedia::STATE_ACTIVE));
        ON_CALL(objVideoMedia, GetUpdateState()).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
        ON_CALL(objVideoMedia, GetMediaDescriptor())
                .WillByDefault(Return(&objVideoMediaDescriptor));
        ON_CALL(objVideoMediaDescriptor, GetMediaDescriptionExAsLocal())
                .WillByDefault(Return(&objVideoSdpMedia));
        objVideoSdpMedia.SetType(SdpMedia::TYPE_VIDEO);

        pSdpPreconditionHelper = new SdpPreconditionHelper();
    }

    virtual void TearDown() override { delete pSdpPreconditionHelper; }
};

TEST_F(SdpPreconditionHelperTest, FormPreconditionSdpDoesNothingIfSessionIsNull)
{
    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::CURR, _)).Times(0);
    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::DES, _)).Times(0);

    pSdpPreconditionHelper->FormPreconditionSdp(IMS_NULL, &objQosStatusTable, IMS_FALSE);
}

TEST_F(SdpPreconditionHelperTest, FormPreconditionSdpDoesNothingIfStatusTableIsNull)
{
    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::CURR, _)).Times(0);
    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::DES, _)).Times(0);

    pSdpPreconditionHelper->FormPreconditionSdp(&objISession, IMS_NULL, IMS_FALSE);
}

TEST_F(SdpPreconditionHelperTest, FormPreconditionSdpDoesNothingIfMediasIsEmpty)
{
    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::CURR, _)).Times(0);
    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::DES, _)).Times(0);

    pSdpPreconditionHelper->FormPreconditionSdp(&objISession, &objQosStatusTable, IMS_FALSE);
}

TEST_F(SdpPreconditionHelperTest, FormPreconditionSdpDoesNothingIfMediaIsNull)
{
    lstMedias.Append(IMS_NULL);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));

    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::CURR, _)).Times(0);
    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::DES, _)).Times(0);

    pSdpPreconditionHelper->FormPreconditionSdp(&objISession, &objQosStatusTable, IMS_FALSE);
}

TEST_F(SdpPreconditionHelperTest, FormPreconditionSdpDoesNothingIfMediaStateIsDeleted)
{
    ON_CALL(objAudioMedia, GetState()).WillByDefault(Return(IMedia::STATE_DELETED));
    lstMedias.Append(&objAudioMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));

    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::CURR, _)).Times(0);
    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::DES, _)).Times(0);

    pSdpPreconditionHelper->FormPreconditionSdp(&objISession, &objQosStatusTable, IMS_FALSE);
}

TEST_F(SdpPreconditionHelperTest, FormPreconditionSdpDoesNothingIfMediaDescriptorIsNull)
{
    ON_CALL(objAudioMedia, GetMediaDescriptor()).WillByDefault(Return(IMS_NULL));
    lstMedias.Append(&objAudioMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));

    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::CURR, _)).Times(0);
    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::DES, _)).Times(0);

    pSdpPreconditionHelper->FormPreconditionSdp(&objISession, &objQosStatusTable, IMS_FALSE);
}

TEST_F(SdpPreconditionHelperTest, FormPreconditionSdpDoesNothingIfLocalSdpIsNull)
{
    ON_CALL(objAudioMediaDescriptor, GetMediaDescriptionExAsLocal())
            .WillByDefault(Return(IMS_NULL));
    lstMedias.Append(&objAudioMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));

    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::CURR, _)).Times(0);
    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::DES, _)).Times(0);

    pSdpPreconditionHelper->FormPreconditionSdp(&objISession, &objQosStatusTable, IMS_FALSE);
}

TEST_F(SdpPreconditionHelperTest,
        FormPreconditionSdpDoesNothingForVideoIfPreconditionIsNotIncludedInSdp)
{
    ON_CALL(objVideoMediaDescriptor, GetMediaDescriptionEx())
            .WillByDefault(Return(&objVideoSdpMedia));
    ON_CALL(objVideoMediaDescriptor, GetPrecondition(SdpAttribute::CURR, SdpPrecondition::TYPE_QOS))
            .WillByDefault(Return(IMS_NULL));

    lstMedias.Append(&objAudioMedia);
    lstMedias.Append(&objVideoMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));

    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::CURR, _));
    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::DES, _));
    EXPECT_CALL(objVideoMediaDescriptor, SetPrecondition(SdpAttribute::CURR, _)).Times(0);
    EXPECT_CALL(objVideoMediaDescriptor, SetPrecondition(SdpAttribute::DES, _)).Times(0);

    pSdpPreconditionHelper->FormPreconditionSdp(&objISession, &objQosStatusTable, IMS_FALSE);
}

TEST_F(SdpPreconditionHelperTest, FormPreconditionSdpSetPreconditionForVideoIfInitialOffer)
{
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    ON_CALL(objVideoMedia, GetState()).WillByDefault(Return(IMedia::STATE_INACTIVE));

    ON_CALL(objVideoMediaDescriptor, GetMediaDescriptionEx())
            .WillByDefault(Return(&objVideoSdpMedia));
    ON_CALL(objVideoMediaDescriptor, GetPrecondition(SdpAttribute::CURR, SdpPrecondition::TYPE_QOS))
            .WillByDefault(Return(IMS_NULL));

    lstMedias.Append(&objAudioMedia);
    lstMedias.Append(&objVideoMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));

    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::CURR, _));
    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::DES, _));
    EXPECT_CALL(objVideoMediaDescriptor, SetPrecondition(SdpAttribute::CURR, _));
    EXPECT_CALL(objVideoMediaDescriptor, SetPrecondition(SdpAttribute::DES, _));

    pSdpPreconditionHelper->FormPreconditionSdp(&objISession, &objQosStatusTable, IMS_FALSE);
}

TEST_F(SdpPreconditionHelperTest, FormPreconditionSdpWithReservedLocalResource)
{
    lstMedias.Append(&objAudioMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));

    ON_CALL(objQosStatusTable, GetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR, _))
            .WillByDefault(Return(SdpPrecondition::DIRECTION_SENDRECV));
    ON_CALL(objQosStatusTable,
            IsCurrentStatusEnabled(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_LOCAL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objQosStatusTable, SetLocalResourceConfirmed(SdpMedia::TYPE_AUDIO, IMS_TRUE))
            .Times(1);
    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::CURR, _)).Times(1);

    ON_CALL(objQosStatusTable, GetStrengthTag(_, _, _))
            .WillByDefault(Return(SdpPrecondition::STRENGTH_MANDATORY));
    EXPECT_CALL(objAudioMediaDescriptor, SetPrecondition(SdpAttribute::DES, _)).Times(1);

    pSdpPreconditionHelper->FormPreconditionSdp(&objISession, &objQosStatusTable, IMS_FALSE);
}
