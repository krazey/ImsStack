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

#include "CallReasonInfo.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(CallReasonInfoTest, CopyConstructor)
{
    CallReasonInfo objReason(1, 1, AString("Extra Value"));
    CallReasonInfo objCopiedReason(objReason);

    EXPECT_EQ(objReason.nCode, objCopiedReason.nCode);
    EXPECT_EQ(objReason.nExtraCode, objCopiedReason.nExtraCode);
    EXPECT_STREQ(objReason.strExtraMessage.GetStr(), objCopiedReason.strExtraMessage.GetStr());
}

TEST(CallReasonInfoTest, InitialValue)
{
    IMS_SINT32 nReason = 1;
    CallReasonInfo objReason(nReason);

    EXPECT_EQ(objReason.nCode, nReason);
    EXPECT_EQ(objReason.nExtraCode, -1);
    EXPECT_EQ(objReason.strExtraMessage, AString::ConstNull());
}

TEST(CallReasonInfoTest, AssignmentOperator)
{
    CallReasonInfo objAssignedReason(0);
    CallReasonInfo objReason(1, 1, AString("Extra Value"));

    objAssignedReason = objReason;

    EXPECT_EQ(objReason.nCode, objAssignedReason.nCode);
    EXPECT_EQ(objReason.nExtraCode, objAssignedReason.nExtraCode);
    EXPECT_STREQ(objReason.strExtraMessage.GetStr(), objAssignedReason.strExtraMessage.GetStr());
}

TEST(CallReasonInfoTest, EqualToOperator)
{
    CallReasonInfo objReasonToCompare(1, 1, AString("Extra Value"));
    CallReasonInfo objReason(1, 1, AString("Extra Value"));

    EXPECT_EQ(objReasonToCompare, objReason);
}

TEST(CallReasonInfoTest, EqualToOperatorByAddress)
{
    CallReasonInfo objReason(1);
    CallReasonInfo* pReason = &objReason;
    EXPECT_EQ(objReason, *pReason);
}

TEST(CallReasonInfoTest, NotEqualToOperator)
{
    CallReasonInfo objReasonToCompare(0);
    CallReasonInfo objReason(1, 1, AString("Extra Value"));
    EXPECT_NE(objReasonToCompare, objReason);
}

TEST(CallReasonInfoTest, IsTerminateRequiredChecksCodes)
{
    EXPECT_FALSE(CallReasonInfo::IsTerminateRequired(CODE_LOCAL_NOT_REGISTERED));
    EXPECT_FALSE(CallReasonInfo::IsTerminateRequired(CODE_LOCAL_NETWORK_NO_SERVICE));
    EXPECT_FALSE(CallReasonInfo::IsTerminateRequired(CODE_LOCAL_NETWORK_NO_LTE_COVERAGE));
    EXPECT_FALSE(CallReasonInfo::IsTerminateRequired(CODE_LOCAL_CALL_VCC_ON_PROGRESSING));

    EXPECT_TRUE(CallReasonInfo::IsTerminateRequired(CODE_UNSPECIFIED));
}

TEST(CallReasonInfoTest, ToStringReturnsValidLogString)
{
    CallReasonInfo objReason(1, 1, AString("Extra Value"));

    AString strExpectedLog = "Code[1] Extra[1][Extra Value]";
    EXPECT_STREQ(objReason.ToString().GetStr(), strExpectedLog.GetStr());
}

TEST(CallReasonInfoTest, ConvertFromInternalReturnsConvertedValue)
{
    EXPECT_EQ(CallReasonInfo(CODE_UNSPECIFIED, 1, "Extra Value").ConvertFromInternal(),
            CallReasonInfo(CODE_UNSPECIFIED, 1, "Extra Value"));

    EXPECT_EQ(CallReasonInfo(CODE_INTERNAL_EARLYDIALOG_FORKED_TERMINATED, 1, "Extra Value")
                      .ConvertFromInternal(),
            CallReasonInfo(CODE_LOCAL_ILLEGAL_STATE));
    EXPECT_EQ(CallReasonInfo(CODE_INTERNAL_REDIAL, 1, "Extra Value").ConvertFromInternal(),
            CallReasonInfo(CODE_LOCAL_ILLEGAL_STATE));
    EXPECT_EQ(CallReasonInfo(CODE_INTERNAL_RRC_REJECT, 1, "Extra Value").ConvertFromInternal(),
            CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE));
}
