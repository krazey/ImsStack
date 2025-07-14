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

#include "IMtcContext.h"
#include "ImsTypeDef.h"
#include "MockIMtcContext.h"
#include "MtcContextRepository.h"
#include <gtest/gtest.h>

namespace android
{

class MtcContextRepositoryTest : public ::testing::Test
{
public:
    // cppcheck-suppress unusedStructMember
    MockIMtcContext objContext;

protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(MtcContextRepositoryTest, GetInstanceCreateInstanceOnlyOnce)
{
    const MtcContextRepository* pContextRepository1 = MtcContextRepository::GetInstance();
    const MtcContextRepository* pContextRepository2 = MtcContextRepository::GetInstance();

    EXPECT_EQ(pContextRepository1, pContextRepository2);
}

TEST_F(MtcContextRepositoryTest, AddContextToSlotId0AndGetContextWithInvalidSlotId)
{
    MtcContextRepository::GetInstance()->AddContext(IMS_SLOT_0, &objContext);
    const IMtcContext* pContextByGetter = MtcContextRepository::GetContext();
    MtcContextRepository::GetInstance()->RemoveContext(IMS_SLOT_0);

    EXPECT_EQ(&objContext, pContextByGetter);
}

TEST_F(MtcContextRepositoryTest, AddContextToSlotId0AndGetContextWithSlotId0)
{
    MtcContextRepository::GetInstance()->AddContext(IMS_SLOT_0, &objContext);
    const IMtcContext* pContextByGetter = MtcContextRepository::GetContext(IMS_SLOT_0);
    MtcContextRepository::GetInstance()->RemoveContext(IMS_SLOT_0);

    EXPECT_EQ(&objContext, pContextByGetter);
}

TEST_F(MtcContextRepositoryTest, AddContextToSlotId0AndGetContextBySlot)
{
    MtcContextRepository::GetInstance()->AddContext(IMS_SLOT_0, &objContext);
    const IMtcContext* pContextByGetter =
            MtcContextRepository::GetInstance()->GetContextBySlot(IMS_SLOT_0);
    MtcContextRepository::GetInstance()->RemoveContext(IMS_SLOT_0);

    EXPECT_EQ(&objContext, pContextByGetter);
}

TEST_F(MtcContextRepositoryTest, AddContextsWithDifferentSlot)
{
    MockIMtcContext objContextSlot1;
    MtcContextRepository::GetInstance()->AddContext(IMS_SLOT_0, &objContext);
    MtcContextRepository::GetInstance()->AddContext(IMS_SLOT_1, &objContextSlot1);

    const IMtcContext* pContextByGetterSlot0 =
            MtcContextRepository::GetInstance()->GetContextBySlot(IMS_SLOT_0);
    const IMtcContext* pContextByGetterSlot1 =
            MtcContextRepository::GetInstance()->GetContextBySlot(IMS_SLOT_1);

    MtcContextRepository::GetInstance()->RemoveContext(IMS_SLOT_0);
    MtcContextRepository::GetInstance()->RemoveContext(IMS_SLOT_1);
    EXPECT_NE(pContextByGetterSlot0, pContextByGetterSlot1);
}

TEST_F(MtcContextRepositoryTest, AddContextAndRemove)
{
    MtcContextRepository::GetInstance()->AddContext(IMS_SLOT_0, &objContext);
    MtcContextRepository::GetInstance()->RemoveContext(IMS_SLOT_0);

    const IMtcContext* pContextByGetterSlot0 =
            MtcContextRepository::GetInstance()->GetContextBySlot(IMS_SLOT_0);

    EXPECT_EQ(pContextByGetterSlot0, nullptr);
}

}  // namespace android
