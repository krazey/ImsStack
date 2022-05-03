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
#include "IMSList.h"
#include "interface/IAosBlock.h"
#include "provider/AosUtil.h"

class AosUtilTest : public ::testing::Test {
public:
    AosUtil* pAosUtil;

protected:
    virtual void SetUp() override {
        pAosUtil = new AosUtil();
        ASSERT_TRUE(pAosUtil != nullptr);
    }

    virtual void TearDown() override {
        if (pAosUtil) {
            delete pAosUtil;
        }
    }
};

TEST_F(AosUtilTest, SetRetryTimeDuration) {
    EXPECT_LE(30, pAosUtil->CalculateUpperBoundTime(30, 1800, 1));
    EXPECT_GE(1800, pAosUtil->CalculateUpperBoundTime(30, 1800, 1));

    EXPECT_LE(30, pAosUtil->WaitTimeForFlowRecovery(30, 1800, 1));
    EXPECT_LE(60, pAosUtil->WaitTimeForFlowRecovery(30, 1800, 2));
    EXPECT_GE(1800, pAosUtil->WaitTimeForFlowRecovery(30, 1800, 1));
}

TEST_F(AosUtilTest, SetRetryTimeDurationExecption) {
    EXPECT_EQ(1800, pAosUtil->CalculateUpperBoundTime(30, 1800, 30));
    EXPECT_GE(1800, pAosUtil->WaitTimeForFlowRecovery(30, 1800, 30));

    EXPECT_EQ(1800, pAosUtil->CalculateUpperBoundTime(1800, 1800, 1));
    EXPECT_EQ(1800, pAosUtil->CalculateUpperBoundTime(1800, 1800, 30));

    EXPECT_GE(1800, pAosUtil->WaitTimeForFlowRecovery(1800, 1800, 1));
    EXPECT_GE(1800, pAosUtil->WaitTimeForFlowRecovery(1800, 1800, 30));
}

TEST_F(AosUtilTest, CompareList) {

    AStringArray leftArray;
    leftArray.AddElement("1234@ims.google.com");
    leftArray.AddElement("sip:1234@ims.google.com");
    leftArray.AddElement("tel:+1234@ims.google.com");

    AStringArray leftArrayOutOfOrder;
    leftArrayOutOfOrder.AddElement("tel:+1234@ims.google.com");
    leftArrayOutOfOrder.AddElement("1234@ims.google.com");
    leftArrayOutOfOrder.AddElement("sip:1234@ims.google.com");

    AStringArray leftArrayAllDiff;
    leftArrayAllDiff.AddElement("1234@google.com");
    leftArrayAllDiff.AddElement("sip:4567@ims.google.com");
    leftArrayAllDiff.AddElement("tel:+8901@ims.google.com");

    AString leftArrayExist = "tel:+1234@ims.google.com";
    AString leftArrayNotExist = "sip:4567@ims.google.com";

    AStringArray rightArray;
    rightArray.AddElement("1234@ims.google.com");
    rightArray.AddElement("sip:1234@ims.google.com");
    rightArray.AddElement("tel:+1234@ims.google.com");

    EXPECT_TRUE(pAosUtil->IsListEqual(leftArray, rightArray, IMS_FALSE));
    EXPECT_TRUE(pAosUtil->IsListEquivalent(leftArray, rightArray, IMS_FALSE));
    EXPECT_FALSE(pAosUtil->IsListAllDifferent(leftArray, rightArray, IMS_FALSE));

    EXPECT_TRUE(pAosUtil->IsListEqual(leftArrayOutOfOrder, rightArray, IMS_FALSE));
    EXPECT_FALSE(pAosUtil->IsListEquivalent(leftArrayOutOfOrder, rightArray, IMS_FALSE));
    EXPECT_FALSE(pAosUtil->IsListAllDifferent(leftArrayOutOfOrder, rightArray, IMS_FALSE));

    EXPECT_FALSE(pAosUtil->IsListEqual(leftArrayAllDiff, rightArray, IMS_FALSE));
    EXPECT_FALSE(pAosUtil->IsListEquivalent(leftArrayAllDiff, rightArray, IMS_FALSE));
    EXPECT_TRUE(pAosUtil->IsListAllDifferent(leftArrayAllDiff, rightArray, IMS_FALSE));

    EXPECT_TRUE(pAosUtil->IsStrExistInList(leftArrayExist, rightArray, IMS_FALSE));
    EXPECT_FALSE(pAosUtil->IsStrExistInList(leftArrayNotExist, rightArray, IMS_FALSE));
}

TEST_F(AosUtilTest, CompareListIPv4) {

    AStringArray leftArray;
    leftArray.AddElement("10.168.219.102");
    leftArray.AddElement("10.168.219.104");
    leftArray.AddElement("10.168.219.106");

    AStringArray leftArrayOutOfOrder;
    leftArrayOutOfOrder.AddElement("10.168.219.104");
    leftArrayOutOfOrder.AddElement("10.168.219.106");
    leftArrayOutOfOrder.AddElement("10.168.219.102");

    AStringArray leftArrayAllDiff;
    leftArrayAllDiff.AddElement("10.168.216.104");
    leftArrayAllDiff.AddElement("10.168.216.106");
    leftArrayAllDiff.AddElement("10.168.216.102");

    AString leftArrayExist = "10.168.219.106";
    AString leftArrayNotExist = "10.168.216.102";

    AStringArray rightArray;
    rightArray.AddElement("10.168.219.102");
    rightArray.AddElement("10.168.219.104");
    rightArray.AddElement("10.168.219.106");

    EXPECT_TRUE(pAosUtil->IsListEqual(leftArray, rightArray, IMS_TRUE));
    EXPECT_TRUE(pAosUtil->IsListEquivalent(leftArray, rightArray, IMS_TRUE));
    EXPECT_FALSE(pAosUtil->IsListAllDifferent(leftArray, rightArray, IMS_TRUE));

    EXPECT_TRUE(pAosUtil->IsListEqual(leftArrayOutOfOrder, rightArray, IMS_TRUE));
    EXPECT_FALSE(pAosUtil->IsListEquivalent(leftArrayOutOfOrder, rightArray, IMS_TRUE));
    EXPECT_FALSE(pAosUtil->IsListAllDifferent(leftArrayOutOfOrder, rightArray, IMS_TRUE));

    EXPECT_FALSE(pAosUtil->IsListEqual(leftArrayAllDiff, rightArray, IMS_TRUE));
    EXPECT_FALSE(pAosUtil->IsListEquivalent(leftArrayAllDiff, rightArray, IMS_TRUE));
    EXPECT_TRUE(pAosUtil->IsListAllDifferent(leftArrayAllDiff, rightArray, IMS_TRUE));

    EXPECT_TRUE(pAosUtil->IsStrExistInList(leftArrayExist, rightArray, IMS_TRUE));
    EXPECT_FALSE(pAosUtil->IsStrExistInList(leftArrayNotExist, rightArray, IMS_TRUE));
}

TEST_F(AosUtilTest, CompareListIPv6) {

    AStringArray leftArray;
    leftArray.AddElement("fd29:cc43:7fb9:2:20c:29ff:fe66:b4c7");
    leftArray.AddElement("240a:3:4400:3420::7");
    leftArray.AddElement("fc01:abab:cdcd:6fee::1");

    AStringArray leftArrayOutOfOrder;
    leftArrayOutOfOrder.AddElement("fc01:abab:cdcd:6fee::1");
    leftArrayOutOfOrder.AddElement("240a:3:4400:3420::7");
    leftArrayOutOfOrder.AddElement("fd29:cc43:7fb9:2:20c:29ff:fe66:b4c7");

    AStringArray leftArrayAllDiff;
    leftArrayAllDiff.AddElement("240a:3:4400:3420::6");
    leftArrayAllDiff.AddElement("fc02:abab:cdcd:6fee::1");
    leftArrayAllDiff.AddElement("fd29:cc42:7fb9:2:20c:29ff:fe66:b4c7");

    AString leftArrayExist = "240a:3:4400:3420::7";
    AString leftArrayNotExist = "fd29:cc42:7fb9:2:20c:29ff:fe66:b4c7";

    AStringArray rightArray;
    rightArray.AddElement("fd29:cc43:7fb9:2:20c:29ff:fe66:b4c7");
    rightArray.AddElement("240a:3:4400:3420:0:0:0:7");
    rightArray.AddElement("fc01:abab:cdcd:6fee::1");

    EXPECT_TRUE(pAosUtil->IsListEqual(leftArray, rightArray, IMS_TRUE));
    EXPECT_TRUE(pAosUtil->IsListEquivalent(leftArray, rightArray, IMS_TRUE));
    EXPECT_FALSE(pAosUtil->IsListAllDifferent(leftArray, rightArray, IMS_TRUE));

    EXPECT_TRUE(pAosUtil->IsListEqual(leftArrayOutOfOrder, rightArray, IMS_TRUE));
    EXPECT_FALSE(pAosUtil->IsListEquivalent(leftArrayOutOfOrder, rightArray, IMS_TRUE));
    EXPECT_FALSE(pAosUtil->IsListAllDifferent(leftArrayOutOfOrder, rightArray, IMS_TRUE));

    EXPECT_FALSE(pAosUtil->IsListEqual(leftArrayAllDiff, rightArray, IMS_TRUE));
    EXPECT_FALSE(pAosUtil->IsListEquivalent(leftArrayAllDiff, rightArray, IMS_TRUE));
    EXPECT_TRUE(pAosUtil->IsListAllDifferent(leftArrayAllDiff, rightArray, IMS_TRUE));

    EXPECT_TRUE(pAosUtil->IsStrExistInList(leftArrayExist, rightArray, IMS_TRUE));
    EXPECT_FALSE(pAosUtil->IsStrExistInList(leftArrayNotExist, rightArray, IMS_TRUE));
}

TEST_F(AosUtilTest, ManageIntList) {
    IMSList<IMS_UINT32> reasons;
    IMSList<IMS_UINT32> compareReasons;
    IMSList<IMS_UINT32> combineReasons;
    reasons.Clear();
    compareReasons.Clear();
    combineReasons.Clear();

    pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, reasons);
    pAosUtil->RemoveElementToList(BLOCK_CELLULAR_OUT_OF_SERVICE, reasons);
    pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, reasons);

    pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, compareReasons);
    pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, compareReasons);

    EXPECT_TRUE(pAosUtil->IsListEqual(reasons, compareReasons, IMS_TRUE));
    EXPECT_TRUE(pAosUtil->IsElementExistInList(compareReasons, reasons));

    pAosUtil->RemoveElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, reasons);
    pAosUtil->RemoveElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, reasons);
    pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, reasons);
    pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, reasons);
    EXPECT_TRUE(pAosUtil->IsListEqual(reasons, compareReasons, IMS_FALSE));

    pAosUtil->AddElementToList(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE, reasons);
    EXPECT_TRUE(pAosUtil->IsElementExistInList(compareReasons, reasons));

    pAosUtil->RemoveElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, reasons);
    EXPECT_TRUE(pAosUtil->IsElementExistInList(compareReasons, reasons));

    pAosUtil->RemoveElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, reasons);
    pAosUtil->AddElementToList(BLOCK_CELLULAR_VOPS_OFF, reasons);
    EXPECT_FALSE(pAosUtil->IsElementExistInList(reasons, compareReasons));

    pAosUtil->CombineLists(reasons, compareReasons, combineReasons);
    EXPECT_TRUE(pAosUtil->IsElementExistInList(reasons, combineReasons));

    pAosUtil->RemoveElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, combineReasons);
    EXPECT_TRUE(pAosUtil->IsElementExistInList(reasons, combineReasons));

    pAosUtil->RemoveElementToList(BLOCK_CELLULAR_VOPS_OFF, combineReasons);
    pAosUtil->RemoveElementToList(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE, combineReasons);
    EXPECT_FALSE(pAosUtil->IsElementExistInList(reasons, combineReasons));
}