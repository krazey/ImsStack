/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "private/CapProperty.h"

namespace android
{
static const IMS_SINT32 SECTOR_ID = CapProperty::SECTOR_SESSION;
static const IMS_SINT32 MESSAGE_TYPE = CapProperty::MESSAGE_TYPE_REQUEST;

class CapPropertyTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(CapPropertyTest, CopyConstructor)
{
    CapProperty objDefaultCapProperty;
    CapProperty objCapProperty(SECTOR_ID, MESSAGE_TYPE);

    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    CapProperty objCopiedCapProperty(objCapProperty);

    AString strCapKey = CapProperty::CreateCapKey(SECTOR_ID, MESSAGE_TYPE);

    EXPECT_FALSE(objDefaultCapProperty.Equals(strCapKey));
    EXPECT_TRUE(objCopiedCapProperty.Equals(strCapKey));
}

TEST_F(CapPropertyTest, AssignmentOperator)
{
    CapProperty objDefaultCapProperty;
    CapProperty objCapProperty(SECTOR_ID, MESSAGE_TYPE);
    CapProperty objAssignedCapProperty;

    objAssignedCapProperty = objCapProperty;

    AString strCapKey = CapProperty::CreateCapKey(SECTOR_ID, MESSAGE_TYPE);

    EXPECT_FALSE(objDefaultCapProperty.Equals(strCapKey));
    EXPECT_TRUE(objAssignedCapProperty.Equals(strCapKey));
}

TEST_F(CapPropertyTest, AddValueAndGetValues)
{
    CapProperty objCapProperty;

    EXPECT_TRUE(objCapProperty.GetValues().GetCount() == 0);

    AStringArray objSdpFields;
    objSdpFields.AddElement(AString("a=attribute:value"));

    objCapProperty.AddValue(objSdpFields.GetFirstElement());

    EXPECT_EQ(objCapProperty.GetValues().GetFirstElement(), objSdpFields.GetFirstElement());
    EXPECT_EQ(objCapProperty.GetValues().GetCount(), objSdpFields.GetCount());
}

TEST_F(CapPropertyTest, SetKey)
{
    CapProperty objCapProperty;

    AString strCapKey = CapProperty::CreateCapKey(SECTOR_ID, MESSAGE_TYPE);

    EXPECT_FALSE(objCapProperty.Equals(strCapKey));

    objCapProperty.SetKey(SECTOR_ID, MESSAGE_TYPE);

    EXPECT_TRUE(objCapProperty.Equals(strCapKey));
}

TEST_F(CapPropertyTest, MessageTypeToString)
{
    EXPECT_EQ(CapProperty::MessageTypeToString(MESSAGE_TYPE),
            AString(CapProperty::MESSAGE_TYPE_STRING[MESSAGE_TYPE]));
    EXPECT_EQ(CapProperty::MessageTypeToString(CapProperty::MESSAGE_TYPE_INVALID),
            AString::ConstNull());
}

TEST_F(CapPropertyTest, SectorIdToString)
{
    EXPECT_FALSE(CapProperty::SectorIdToString(SECTOR_ID).Equals(AString::ConstNull()));
    EXPECT_TRUE(CapProperty::SectorIdToString(CapProperty::SECTOR_INVALID)
                        .Equals(AString::ConstNull()));
}

TEST_F(CapPropertyTest, StringToMessageType)
{
    EXPECT_EQ(CapProperty::StringToMessageType(
                      AString(CapProperty::MESSAGE_TYPE_STRING[MESSAGE_TYPE])),
            MESSAGE_TYPE);
    EXPECT_EQ(CapProperty::StringToMessageType(AString("invalid")),
            CapProperty::MESSAGE_TYPE_INVALID);
}

TEST_F(CapPropertyTest, StringToSectorId)
{
    EXPECT_EQ(CapProperty::StringToSectorId(AString(CapProperty::SECTOR_STRING[SECTOR_ID])),
            SECTOR_ID);
    EXPECT_EQ(CapProperty::StringToSectorId(AString("invalid")), CapProperty::SECTOR_INVALID);
}

}  // namespace android
