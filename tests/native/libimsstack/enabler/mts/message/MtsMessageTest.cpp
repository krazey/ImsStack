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

#include "message/MtsMessage.h"
#include <gtest/gtest.h>

namespace android
{

const IMS_SINT32 SLOT_ID = 0;

class MtsMessageTest : public ::testing::Test
{
public:
    MtsMessage* pMtsMessage;

protected:
    virtual void SetUp() override { pMtsMessage = new MtsMessage(SLOT_ID); }

    virtual void TearDown() override { delete pMtsMessage; }
};

TEST_F(MtsMessageTest, Constructor)
{
    ASSERT_NE(pMtsMessage, nullptr);
}

TEST_F(MtsMessageTest, GetterAndSetterForDestination)
{
    AString strDestination("TargetAddress");
    pMtsMessage->SetDestination(strDestination);

    EXPECT_STREQ(strDestination.GetStr(), pMtsMessage->GetDestination().GetStr());
}

TEST_F(MtsMessageTest, GetterAndSetterForImpu)
{
    AString strImpu("FakeImpu");
    pMtsMessage->SetImpu(strImpu);

    EXPECT_STREQ(strImpu.GetStr(), pMtsMessage->GetImpu().GetStr());
}

TEST_F(MtsMessageTest, GetterAndSetterForMessageReference)
{
    IMS_SINT32 nMrOfRp = 0;
    pMtsMessage->SetMessageReference(nMrOfRp);

    EXPECT_EQ(nMrOfRp, pMtsMessage->GetMessageReference());
}

TEST_F(MtsMessageTest, GetterAndSetterForMti)
{
    IMS_SINT32 nMti = 0;
    pMtsMessage->SetMti(nMti);

    EXPECT_EQ(nMti, pMtsMessage->GetMti());
}

// TODO: Need to improve
TEST_F(MtsMessageTest, GetterAndSetterForPageMessage)
{
    IPageMessage* piPageMessage = IMS_NULL;
    pMtsMessage->SetPageMessage(piPageMessage);

    EXPECT_EQ(piPageMessage, pMtsMessage->GetPageMessage());
}

TEST_F(MtsMessageTest, GetterAndSetterForSeqId)
{
    IMS_SINT32 nSeqId = 0;
    pMtsMessage->SetSeqId(nSeqId);

    EXPECT_EQ(nSeqId, pMtsMessage->GetSeqId());
}

TEST_F(MtsMessageTest, GetterAndSetterForSlotId)
{
    IMS_SINT32 nSlotId = 1;
    pMtsMessage->SetSlotId(nSlotId);

    EXPECT_EQ(nSlotId, pMtsMessage->GetSlotId());
}

TEST_F(MtsMessageTest, GetterAndSetterForSmsFormat)
{
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    pMtsMessage->SetSmsFormat(eSmsFormat);

    EXPECT_EQ(eSmsFormat, pMtsMessage->GetSmsFormat());
}

TEST_F(MtsMessageTest, GetterAndSetterForSmSize)
{
    IMS_SINT32 nSmSize = 0;
    pMtsMessage->SetSmSize(nSmSize);

    EXPECT_EQ(nSmSize, pMtsMessage->GetSmSize());
}

TEST_F(MtsMessageTest, GetterAndSetterForTransactionType)
{
    MtsTransactionType eTransactionType = MtsTransactionType::MESSAGE_TYPE_RECEIVE;
    pMtsMessage->SetTransactionType(eTransactionType);

    EXPECT_EQ(eTransactionType, pMtsMessage->GetTransactionType());
}

}  // namespace android
