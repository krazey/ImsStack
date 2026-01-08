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

#include "ImsActivity.h"
#include "interface/AosInternalMsgDef.h"
#include "manager/ImsAosManager.h"

class ImsAosManagerTest : public ::testing::Test
{
public:
    ImsAosManager* pImsAosManager;

protected:
    void SetUp() override
    {
        pImsAosManager = new ImsAosManager("ims.app.mtc");
        ASSERT_TRUE(pImsAosManager != nullptr);
    }

    void TearDown() override
    {
        if (pImsAosManager)
        {
            delete pImsAosManager;
        }
    }
};

TEST_F(ImsAosManagerTest, GetImsAos)
{
    EXPECT_EQ(IMS_NULL, pImsAosManager->GetImsAos("ims.app.mtc", "ims.service.mtc"));
}

TEST_F(ImsAosManagerTest, GetImsAosList)
{
    EXPECT_EQ(ImsList<IImsAos*>(), pImsAosManager->GetImsAosList("ims.app.mtc", "ims.service.mtc"));
    EXPECT_EQ(ImsList<IImsAos*>(), pImsAosManager->GetImsAosList("ims.app.mtc"));
}

TEST_F(ImsAosManagerTest, OnMessage)
{
    ImsMessage objMsg(AOSMSG_SERVICE_INTERNAL, 0, 0);
    EXPECT_TRUE(pImsAosManager->OnPreprocess(objMsg));
    EXPECT_TRUE(pImsAosManager->OnMessage(objMsg));
    EXPECT_TRUE(pImsAosManager->OnPostprocess(objMsg));
}