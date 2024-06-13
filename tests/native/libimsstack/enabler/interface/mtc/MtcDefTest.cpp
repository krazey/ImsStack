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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "MtcDef.h"

// TODO: Unit tests of only the uncovered lines are added. The other cases will be added.

LOCAL IMS_SINT32 ANY_A_DIR = 1;
LOCAL IMS_SINT32 ANY_V_DIR = 2;
LOCAL IMS_SINT32 ANY_T_DIR = 3;
LOCAL IMS_SINT32 ANY_A_QUALITY = 1;
LOCAL IMS_SINT32 ANY_V_QUALITY = 2;
LOCAL IMS_SINT32 ANY_GTT_MODE = 1;

LOCAL AString ANY_SUPP_STR = "any";
LOCAL IMS_SINT32 ANY_SUPP_INT = 10;
LOCAL IMS_BOOL ANY_SUPP_BOOL = IMS_TRUE;

TEST(MtcDefTest, MediaInfoConstructorWithParams)
{
    MediaInfo objMediaInfo(
            ANY_A_DIR, ANY_V_DIR, ANY_T_DIR, ANY_A_QUALITY, ANY_V_QUALITY, ANY_GTT_MODE);

    EXPECT_EQ(objMediaInfo.eAudioDirection, ANY_A_DIR);
    EXPECT_EQ(objMediaInfo.eVideoDirection, ANY_V_DIR);
    EXPECT_EQ(objMediaInfo.eTextDirection, ANY_T_DIR);
    EXPECT_EQ(objMediaInfo.eAudioQuality, ANY_A_QUALITY);
    EXPECT_EQ(objMediaInfo.eVideoQuality, ANY_V_QUALITY);
    EXPECT_EQ(objMediaInfo.eGttMode, ANY_GTT_MODE);
}

TEST(MtcDefTest, MediaInfoAssignmentOperator)
{
    MediaInfo objMediaInfo;
    MediaInfo objRightHandSide(ANY_A_DIR, ANY_V_DIR, 3, 1, 2, 1);
    objMediaInfo = objRightHandSide;

    EXPECT_EQ(objMediaInfo.eAudioDirection, objRightHandSide.eAudioDirection);
    EXPECT_EQ(objMediaInfo.eVideoDirection, objRightHandSide.eVideoDirection);
    EXPECT_EQ(objMediaInfo.eTextDirection, objRightHandSide.eTextDirection);
    EXPECT_EQ(objMediaInfo.eAudioQuality, objRightHandSide.eAudioQuality);
    EXPECT_EQ(objMediaInfo.eVideoQuality, objRightHandSide.eVideoQuality);
    EXPECT_EQ(objMediaInfo.eGttMode, objRightHandSide.eGttMode);
}

TEST(MtcDefTest, MediaInfoEqualToOperator)
{
    MediaInfo objMediaInfoWithValues(
            ANY_A_DIR, ANY_V_DIR, ANY_T_DIR, ANY_A_QUALITY, ANY_V_QUALITY, ANY_GTT_MODE);
    MediaInfo objMediaInfoWithValuesToCompare(
            ANY_A_DIR, ANY_V_DIR, ANY_T_DIR, ANY_A_QUALITY, ANY_V_QUALITY, ANY_GTT_MODE);
    EXPECT_TRUE(objMediaInfoWithValues == objMediaInfoWithValuesToCompare);
}

TEST(MtcDefTest, MediaInfoNotEqual)
{
    IMS_SINT32 __DIFF_A_DIR__ = 2;

    MediaInfo objMediaInfoWithValues(
            ANY_A_DIR, ANY_V_DIR, ANY_T_DIR, ANY_A_QUALITY, ANY_V_QUALITY, ANY_GTT_MODE);
    MediaInfo objMediaInfoWithValuesToCompare(
            __DIFF_A_DIR__, ANY_V_DIR, ANY_T_DIR, ANY_A_QUALITY, ANY_V_QUALITY, ANY_GTT_MODE);
    EXPECT_TRUE(objMediaInfoWithValues != objMediaInfoWithValuesToCompare);
}

TEST(MtcDefTest, SuppServiceCopyConstructor)
{
    SuppService objSuppServiceToCopy;
    objSuppServiceToCopy.strValue = ANY_SUPP_STR;
    objSuppServiceToCopy.nValue = ANY_SUPP_INT;
    objSuppServiceToCopy.bValue = ANY_SUPP_BOOL;

    SuppService objSuppService(objSuppServiceToCopy);

    EXPECT_STREQ(objSuppService.strValue.GetStr(), objSuppServiceToCopy.strValue.GetStr());
    EXPECT_EQ(objSuppService.nValue, objSuppServiceToCopy.nValue);
    EXPECT_EQ(objSuppService.bValue, objSuppServiceToCopy.bValue);
}

TEST(MtcDefTest, SuppServiceAssignmentOperator)
{
    SuppService objSuppService;
    SuppService objRightHandSide;
    objRightHandSide.strValue = ANY_SUPP_STR;
    objRightHandSide.nValue = ANY_SUPP_INT;
    objRightHandSide.bValue = ANY_SUPP_BOOL;

    objSuppService = objRightHandSide;

    EXPECT_STREQ(objSuppService.strValue.GetStr(), objRightHandSide.strValue.GetStr());
    EXPECT_EQ(objSuppService.nValue, objRightHandSide.nValue);
    EXPECT_EQ(objSuppService.bValue, objRightHandSide.bValue);
}

TEST(MtcDefTest, SuppServiceEqualToOperator)
{
    SuppService objSuppServiceWithValues;
    objSuppServiceWithValues.strValue = ANY_SUPP_STR;
    objSuppServiceWithValues.nValue = ANY_SUPP_INT;
    objSuppServiceWithValues.bValue = ANY_SUPP_BOOL;

    SuppService objSuppServiceWithValuesToCompare;
    objSuppServiceWithValuesToCompare.strValue = ANY_SUPP_STR;
    objSuppServiceWithValuesToCompare.nValue = ANY_SUPP_INT;
    objSuppServiceWithValuesToCompare.bValue = ANY_SUPP_BOOL;

    EXPECT_TRUE(objSuppServiceWithValues == objSuppServiceWithValuesToCompare);
}
