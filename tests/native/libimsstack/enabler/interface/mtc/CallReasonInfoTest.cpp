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

TEST(CallReasonInfoTest, AssignOperator)
{
    CallReasonInfo objAssignedReason(0);
    CallReasonInfo objReason(1, 1, AString("Extra Value"));

    objAssignedReason = objReason;

    EXPECT_EQ(objReason.nCode, objAssignedReason.nCode);
    EXPECT_EQ(objReason.nExtraCode, objAssignedReason.nExtraCode);
    EXPECT_STREQ(objReason.strExtraMessage.GetStr(), objAssignedReason.strExtraMessage.GetStr());
}
