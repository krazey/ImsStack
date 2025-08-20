/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "ImsList.h"
#include "MtcDef.h"
#include "utility/SuppServiceUtils.h"
#include <gtest/gtest.h>

class SuppServiceUtilsTest : public ::testing::Test
{
};

TEST_F(SuppServiceUtilsTest, AddAndDeleteRemovesService)
{
    ImsList<SuppService*> suppServices;
    SuppServiceUtils::Add(suppServices,
            static_cast<IMS_SINT32>(PermanentSuppType::TB_CB_INCOMING_ROAMING_VOICE), IMS_TRUE);
    SuppServiceUtils::Add(suppServices,
            static_cast<IMS_SINT32>(PermanentSuppType::TB_CB_INCOMING_ANONYMOUS_VIDEO), 0);
    AString srtTest("test");
    SuppServiceUtils::Add(suppServices,
            static_cast<IMS_SINT32>(PermanentSuppType::TB_CB_INCOMING_ALL_VOICE), srtTest);
    EXPECT_EQ(3, suppServices.GetSize());
    SuppService* service = SuppServiceUtils::Get(
            suppServices, static_cast<IMS_SINT32>(PermanentSuppType::TB_CB_INCOMING_ALL_VOICE));
    EXPECT_NE(IMS_NULL, service);

    SuppServiceUtils::Delete(
            suppServices, static_cast<IMS_SINT32>(PermanentSuppType::TB_CB_INCOMING_ALL_VOICE));
    EXPECT_EQ(2, suppServices.GetSize());
    service = SuppServiceUtils::Get(
            suppServices, static_cast<IMS_SINT32>(PermanentSuppType::TB_CB_INCOMING_ALL_VOICE));
    EXPECT_EQ(IMS_NULL, service);

    SuppServiceUtils::DeleteServices(suppServices);
    EXPECT_EQ(0, suppServices.GetSize());
}

TEST_F(SuppServiceUtilsTest, CloneAndIsSameSuppServicesReturnsTrue)
{
    ImsList<SuppService*> suppServices;
    SuppServiceUtils::Add(suppServices,
            static_cast<IMS_SINT32>(PermanentSuppType::TB_CB_INCOMING_ROAMING_VOICE), IMS_TRUE);
    SuppServiceUtils::Add(suppServices,
            static_cast<IMS_SINT32>(PermanentSuppType::TB_CB_INCOMING_ANONYMOUS_VIDEO), 0);
    AString srtTest("test");
    SuppServiceUtils::Add(suppServices,
            static_cast<IMS_SINT32>(PermanentSuppType::TB_CB_INCOMING_ALL_VOICE), srtTest);

    ImsList<SuppService*> suppServices2;
    suppServices2 = SuppServiceUtils::Clone(suppServices);
    EXPECT_TRUE(SuppServiceUtils::IsSameSuppServices(suppServices, suppServices2));

    SuppServiceUtils::Add(suppServices,
            static_cast<IMS_SINT32>(PermanentSuppType::TB_CB_INCOMING_ROAMING_VOICE), IMS_FALSE);
    EXPECT_FALSE(SuppServiceUtils::IsSameSuppServices(suppServices, suppServices2));
}
