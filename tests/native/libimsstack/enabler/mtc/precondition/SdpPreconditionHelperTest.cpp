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

#include "core/MockISession.h"
#include "core/media/IMedia.h"
#include "media/MockIMedia.h"
#include "media/MockIMediaDescriptor.h"
#include "precondition/MockQosStatusTable.h"
#include "precondition/SdpPreconditionHelper.h"
#include "sdp/SdpAttribute.h"
#include "sdp/SdpMedia.h"
#include "sdp/offeranswer/SdpPrecondition.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

class SdpPreconditionHelperTest : public ::testing::Test
{
public:
    inline SdpPreconditionHelperTest() :
            objISession(),
            objIMediaDescriptor(),
            objQosStatusTable(),
            pSdpPreconditionHelper(IMS_NULL)
    {
    }

public:
    MockISession objISession;
    MockIMediaDescriptor objIMediaDescriptor;
    MockQosStatusTable objQosStatusTable;
    SdpPreconditionHelper* pSdpPreconditionHelper;

protected:
    virtual void SetUp() override { pSdpPreconditionHelper = new SdpPreconditionHelper(); }

    virtual void TearDown() override { delete pSdpPreconditionHelper; }
};

TEST_F(SdpPreconditionHelperTest, FormPreconditionSdpWithReservedLocalResource)
{
    ImsList<IMedia*> lstMedias;
    MockIMedia objAudioMedia;
    lstMedias.Append(&objAudioMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));
    ON_CALL(objAudioMedia, GetState()).WillByDefault(Return(IMedia::STATE_ACTIVE));
    ON_CALL(objAudioMedia, GetUpdateState()).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objAudioMedia, GetMediaDescriptor()).WillByDefault(Return(&objIMediaDescriptor));

    SdpMedia objSdpMedia;
    objSdpMedia.SetType(SdpMedia::TYPE_AUDIO);
    ON_CALL(objIMediaDescriptor, GetMediaDescriptionExAsLocal())
            .WillByDefault(Return(&objSdpMedia));
    ON_CALL(objQosStatusTable, GetDirectionTag(SdpMedia::TYPE_AUDIO, SdpAttribute::CURR, _))
            .WillByDefault(Return(SdpPrecondition::DIRECTION_SENDRECV));
    ON_CALL(objQosStatusTable,
            IsCurrentStatusEnabled(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_LOCAL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objQosStatusTable, SetLocalResourceConfirmed(SdpMedia::TYPE_AUDIO, IMS_TRUE))
            .Times(1);
    EXPECT_CALL(objIMediaDescriptor, SetPrecondition(SdpAttribute::CURR, _)).Times(1);

    ON_CALL(objQosStatusTable, GetStrengthTag(_, _, _))
            .WillByDefault(Return(SdpPrecondition::STRENGTH_MANDATORY));
    EXPECT_CALL(objIMediaDescriptor, SetPrecondition(SdpAttribute::DES, _)).Times(1);

    pSdpPreconditionHelper->FormPreconditionSdp(&objISession, &objQosStatusTable, IMS_FALSE);
}
