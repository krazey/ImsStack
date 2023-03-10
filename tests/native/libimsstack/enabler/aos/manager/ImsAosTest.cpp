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

#include "manager/ImsAosManager.h"
#include "ImsAos.h"

const IMS_SINT32 SLOT_ID = 0;

class ImsAosTest : public ::testing::Test
{
public:
    ImsAos* pImsAos;

protected:
    virtual void SetUp() override
    {
        pImsAos = new ImsAos();
        ASSERT_TRUE(pImsAos != nullptr);
    }

    virtual void TearDown() override
    {
        if (pImsAos)
        {
            delete pImsAos;
        }
    }
};

TEST_F(ImsAosTest, GetImsAos)
{
    ImsList<IImsAos*> objImsAosList =
            ImsAos::GetImsAosList("ims.app.mtc", "ims.service.mtc", SLOT_ID);
    EXPECT_LE(0, objImsAosList.GetSize());

    objImsAosList = ImsAos::GetImsAosList("ims.app.mtc", "ims.service.mtc.emergency", SLOT_ID);
    EXPECT_LE(0, objImsAosList.GetSize());

    objImsAosList = ImsAos::GetImsAosList("ims.app.mts", "ims.service.mts", SLOT_ID);
    EXPECT_LE(0, objImsAosList.GetSize());

    ImsList<IImsAos*> objImsAosList2 = ImsAos::GetImsAosList("ims.app.mtc", SLOT_ID);
    EXPECT_LE(0, objImsAosList2.GetSize());

    objImsAosList2 = ImsAos::GetImsAosList("ims.app.mts", SLOT_ID);
    EXPECT_LE(0, objImsAosList2.GetSize());
}
