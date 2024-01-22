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

#include "interface/IAosAppContext.h"
#include "interface/IAosBlockListener.h"
#include "condition/AosBlock.h"
#include "provider/AosStaticProfile.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosBlockListener.h"
#include "app/MockAosAppContext.h"

using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::ValuesIn;

class TestAosBlock : public AosBlock
{
public:
    inline explicit TestAosBlock(IN IAosAppContext* piAppContext) :
            AosBlock(piAppContext)
    {
    }

    inline ImsList<IAosBlockListener*> GetBlockListeners() { return m_objListeners; }
};

struct FormattingTestCase
{
    IMS_UINT32 nReason;
    const IMS_CHAR* pszString;
};

class AosBlockTest : public testing::TestWithParam<FormattingTestCase>
{
public:
    TestAosBlock* m_pAosBlock;

protected:
    virtual void SetUp() override
    {
        MockIAosAppContext objMockIAosAppContext;
        EXPECT_CALL(objMockIAosAppContext, GetSlotId())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(objMockIAosAppContext, GetProfileId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strValue));

        m_pAosBlock = new TestAosBlock(&objMockIAosAppContext);
        ASSERT_TRUE(m_pAosBlock != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pAosBlock)
        {
            delete m_pAosBlock;
        }
    }
};

INSTANTIATE_TEST_SUITE_P(AosBlockTestInstantiation, AosBlockTest,
        ValuesIn<FormattingTestCase>({
                {BLOCK_AC_INCOMPLETED,                "AC_INCOMPLETED"               },
                {BLOCK_AUTHENTICATION_FAILED,         "AUTHENTICATION_FAILED"        },
                {BLOCK_AOS_INCOMPLETED,               "AOS_INCOMPLETED"              },
                {BLOCK_CSCALL_STARTED,                "CSCALL_STARTED"               },
                {BLOCK_PERMANENT_DATA_FAILED,         "PERMANENT_DATA_FAILED"        },
                {BLOCK_ENABLER_DETACHED,              "ENABLER_DETACHED"             },
                {BLOCK_IMS_DISABLED,                  "IMS_DISABLED"                 },
                {BLOCK_PERMANENT_REG_FAILED,          "PERMANENT_REG_FAILED"         },
                {BLOCK_POWER_OFF,                     "POWER_OFF"                    },
                {BLOCK_SERVICE_CONNECTING,            "SERVICE_CONNECTING"           },
                {BLOCK_SUBSCRIBER_INCOMPLETED,        "SUBSCRIBER_INCOMPLETED"       },
                {BLOCK_TTY_MODE_ON,                   "TTY_MODE_ON"                  },
                {BLOCK_TEMPORARY_DATA_DEACTIVATED,    "TEMPORARY_DATA_DEACTIVATED"   },
                {BLOCK_IMS_SERVICE_DISABLED,          "IMS_SERVICE_DISABLED"         },
                {BLOCK_EPS_FALLBACK_STARTED,          "EPS_FALLBACK_STARTED"         },
                {BLOCK_CELLULAR_AIRPLANE_MODE_ON,     "CELLULAR_AIRPLANE_MODE_ON"    },
                {BLOCK_CELLULAR_NO_NETWORK,           "CELLULAR_NO_NETWORK"          },
                {BLOCK_CELLULAR_OUT_OF_SERVICE,       "CELLULAR_OUT_OF_SERVICE"      },
                {BLOCK_CELLULAR_ROAMING,              "CELLULAR_ROAMING"             },
                {BLOCK_CELLULAR_VOPS_OFF,             "CELLULAR_VOPS_OFF"            },
                {BLOCK_WIFI_BAD_CONNECTION,           "WIFI_BAD_CONNECTION"          },
                {BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE, "WIFI_COUNTRY_CODE_UNAVAILABLE"},
                {BLOCK_WIFI_AIRPLANE_MODE_ON,         "WIFI_AIRPLANE_MODE_ON"        },
                {BLOCK_WIFI_NO_WIFI,                  "WIFI_NO_WIFI"                 },
                {BLOCK_WIFI_TEMPORARILY_BLOCKED,      "WIFI_TEMPORARILY_BLOCKED"     },
                {BLOCK_MAX,                           "INVALID"                      }
}));

TEST_F(AosBlockTest, FailsSetListenerWhenListenerIsNull)
{
    // GIVEN
    EXPECT_EQ(m_pAosBlock->GetBlockListeners().GetSize(), 0);

    // WHEN
    m_pAosBlock->SetListener(IMS_NULL);

    // THEN
    EXPECT_EQ(m_pAosBlock->GetBlockListeners().GetSize(), 0);
}

TEST_F(AosBlockTest, SucceedsSetListener)
{
    // GIVEN
    EXPECT_EQ(m_pAosBlock->GetBlockListeners().GetSize(), 0);
    MockIAosBlockListener objMockIAosBlockListener1;
    MockIAosBlockListener objMockIAosBlockListener2;
    MockIAosBlockListener objMockIAosBlockListener3;

    // WHEN
    m_pAosBlock->SetListener(&objMockIAosBlockListener1);
    m_pAosBlock->SetListener(&objMockIAosBlockListener2);
    m_pAosBlock->SetListener(&objMockIAosBlockListener3);

    // THEN
    EXPECT_EQ(m_pAosBlock->GetBlockListeners().GetSize(), 3);
}

TEST_F(AosBlockTest, FailsSetListenerWhenSameListenerIsExist)
{
    // GIVEN
    EXPECT_EQ(m_pAosBlock->GetBlockListeners().GetSize(), 0);
    MockIAosBlockListener objMockIAosBlockListener1;
    MockIAosBlockListener objMockIAosBlockListener2;
    MockIAosBlockListener objMockIAosBlockListener3;

    m_pAosBlock->SetListener(&objMockIAosBlockListener1);
    m_pAosBlock->SetListener(&objMockIAosBlockListener2);
    m_pAosBlock->SetListener(&objMockIAosBlockListener3);

    EXPECT_EQ(m_pAosBlock->GetBlockListeners().GetSize(), 3);

    // WHEN
    m_pAosBlock->SetListener(&objMockIAosBlockListener1);
    m_pAosBlock->SetListener(&objMockIAosBlockListener2);
    m_pAosBlock->SetListener(&objMockIAosBlockListener3);

    // THEN
    EXPECT_EQ(m_pAosBlock->GetBlockListeners().GetSize(), 3);
}

TEST_F(AosBlockTest, FailsRemoveListenerWhenListenerIsNull)
{
    // GIVEN
    MockIAosBlockListener objMockIAosBlockListener1;
    MockIAosBlockListener objMockIAosBlockListener2;
    MockIAosBlockListener objMockIAosBlockListener3;

    m_pAosBlock->SetListener(&objMockIAosBlockListener1);
    m_pAosBlock->SetListener(&objMockIAosBlockListener2);
    m_pAosBlock->SetListener(&objMockIAosBlockListener3);

    EXPECT_EQ(m_pAosBlock->GetBlockListeners().GetSize(), 3);

    // WHEN
    m_pAosBlock->RemoveListener(IMS_NULL);

    // THEN
    EXPECT_EQ(m_pAosBlock->GetBlockListeners().GetSize(), 3);
}

TEST_F(AosBlockTest, SucceedsRemoveListener)
{
    // GIVEN
    MockIAosBlockListener objMockIAosBlockListener1;
    MockIAosBlockListener objMockIAosBlockListener2;
    MockIAosBlockListener objMockIAosBlockListener3;

    m_pAosBlock->SetListener(&objMockIAosBlockListener1);
    m_pAosBlock->SetListener(&objMockIAosBlockListener2);
    m_pAosBlock->SetListener(&objMockIAosBlockListener3);

    EXPECT_EQ(m_pAosBlock->GetBlockListeners().GetSize(), 3);

    // WHEN
    m_pAosBlock->RemoveListener(&objMockIAosBlockListener3);
    m_pAosBlock->RemoveListener(&objMockIAosBlockListener2);
    m_pAosBlock->RemoveListener(&objMockIAosBlockListener1);

    // THEN
    EXPECT_EQ(m_pAosBlock->GetBlockListeners().GetSize(), 0);
}

TEST_P(AosBlockTest, FailsSetBlockReasonWhenReasonIsBlocked)
{
    // GIVEN
    const FormattingTestCase& objTestCase = GetParam();

    // WHEN
    m_pAosBlock->SetBlockReason(static_cast<BLOCK_REASON>(objTestCase.nReason));

    // THEN
    EXPECT_FALSE(m_pAosBlock->SetBlockReason(static_cast<BLOCK_REASON>(objTestCase.nReason)));
}

TEST_F(AosBlockTest, SetBlockReason_Success)
{
    IAosBlockListener* piAosBlockListener1 = new MockIAosBlockListener();
    IAosBlockListener* piAosBlockListener2 = new MockIAosBlockListener();
    IAosBlockListener* piAosBlockListener3 = new MockIAosBlockListener();

    m_pAosBlock->SetListener(piAosBlockListener1);
    m_pAosBlock->SetListener(piAosBlockListener2);
    m_pAosBlock->SetListener(piAosBlockListener3);
    EXPECT_EQ(m_pAosBlock->GetBlockListeners().GetSize(), 3);

    EXPECT_TRUE(m_pAosBlock->IsCleared());

    m_pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    m_pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    m_pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);

    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_AC_INCOMPLETED));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));

    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);

    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_AIRPLANE_MODE_ON));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_NO_NETWORK));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_OUT_OF_SERVICE));

    m_pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);

    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_WIFI_BAD_CONNECTION));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_WIFI_AIRPLANE_MODE_ON));

    ImsList<IMS_UINT32> objReason;
    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(objReason.GetSize(), 9);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(objReason.GetSize(), 6);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(objReason.GetSize(), 6);
}

TEST_F(AosBlockTest, ResetBlockReason_IsNotReasonBlocked)
{
    EXPECT_TRUE(m_pAosBlock->IsCleared());

    EXPECT_FALSE(m_pAosBlock->ResetBlockReason(BLOCK_AC_INCOMPLETED));
    EXPECT_FALSE(m_pAosBlock->ResetBlockReason(BLOCK_AUTHENTICATION_FAILED));
    EXPECT_FALSE(m_pAosBlock->ResetBlockReason(BLOCK_AOS_INCOMPLETED));
    EXPECT_FALSE(m_pAosBlock->ResetBlockReason(BLOCK_CSCALL_STARTED));
    EXPECT_FALSE(m_pAosBlock->ResetBlockReason(BLOCK_PERMANENT_DATA_FAILED));
}

TEST_F(AosBlockTest, ResetBlockReason_Success)
{
    IAosBlockListener* piAosBlockListener1 = new MockIAosBlockListener();
    IAosBlockListener* piAosBlockListener2 = new MockIAosBlockListener();
    IAosBlockListener* piAosBlockListener3 = new MockIAosBlockListener();

    m_pAosBlock->SetListener(piAosBlockListener1);
    m_pAosBlock->SetListener(piAosBlockListener2);
    m_pAosBlock->SetListener(piAosBlockListener3);
    EXPECT_EQ(m_pAosBlock->GetBlockListeners().GetSize(), 3);

    EXPECT_TRUE(m_pAosBlock->IsCleared());

    m_pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    m_pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    m_pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);

    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);

    m_pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);

    ImsList<IMS_UINT32> objReason;
    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(objReason.GetSize(), 9);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(objReason.GetSize(), 6);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(objReason.GetSize(), 6);

    m_pAosBlock->ResetBlockReason(BLOCK_AC_INCOMPLETED);
    m_pAosBlock->ResetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    m_pAosBlock->ResetBlockReason(BLOCK_AOS_INCOMPLETED);

    m_pAosBlock->ResetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosBlock->ResetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosBlock->ResetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);

    m_pAosBlock->ResetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosBlock->ResetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosBlock->ResetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);

    EXPECT_TRUE(m_pAosBlock->IsCleared());
}

TEST_F(AosBlockTest, ClearAllBlockReasons)
{
    EXPECT_TRUE(m_pAosBlock->IsCleared());

    m_pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    m_pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    m_pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);

    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);

    m_pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);

    ImsList<IMS_UINT32> objReason;
    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(objReason.GetSize(), 9);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(objReason.GetSize(), 6);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(objReason.GetSize(), 6);

    m_pAosBlock->ClearAllBlockReasons();
    EXPECT_TRUE(m_pAosBlock->IsCleared());
}

TEST_F(AosBlockTest, PrintBlockReasons)
{
    m_pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    m_pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    m_pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);

    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);

    m_pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);

    EXPECT_TRUE(m_pAosBlock->PrintBlockReasons());
}

TEST_F(AosBlockTest, GetBlockReasons)
{
    EXPECT_TRUE(m_pAosBlock->IsCleared());

    m_pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    m_pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    m_pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);

    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);

    m_pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);

    ImsList<IMS_UINT32> objReason;
    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(objReason.GetSize(), 9);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(objReason.GetSize(), 6);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(objReason.GetSize(), 6);
}

TEST_F(AosBlockTest, IsReasonBlocked_OnlyEnabled)
{
    EXPECT_TRUE(m_pAosBlock->IsCleared());

    m_pAosBlock->SetBlockReason(BLOCK_POWER_OFF);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_ROAMING);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_NO_WIFI);
    EXPECT_FALSE(m_pAosBlock->IsReasonBlocked(BLOCK_POWER_OFF, IMS_TRUE, SERVICE_WHOLE));

    m_pAosBlock->ClearAllBlockReasons();
    EXPECT_TRUE(m_pAosBlock->IsCleared());

    m_pAosBlock->SetBlockReason(BLOCK_POWER_OFF);
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_POWER_OFF, IMS_TRUE, SERVICE_WHOLE));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_POWER_OFF, IMS_TRUE, SERVICE_CELLULAR));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_POWER_OFF, IMS_TRUE, SERVICE_WIFI));

    m_pAosBlock->ClearAllBlockReasons();
    EXPECT_TRUE(m_pAosBlock->IsCleared());

    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_ROAMING);
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_ROAMING, IMS_TRUE, SERVICE_WHOLE));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_ROAMING, IMS_TRUE, SERVICE_CELLULAR));
    EXPECT_FALSE(m_pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_ROAMING, IMS_TRUE, SERVICE_WIFI));

    m_pAosBlock->ClearAllBlockReasons();
    EXPECT_TRUE(m_pAosBlock->IsCleared());

    m_pAosBlock->SetBlockReason(BLOCK_WIFI_NO_WIFI);
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_WIFI_NO_WIFI, IMS_TRUE, SERVICE_WHOLE));
    EXPECT_FALSE(m_pAosBlock->IsReasonBlocked(BLOCK_WIFI_NO_WIFI, IMS_TRUE, SERVICE_CELLULAR));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_WIFI_NO_WIFI, IMS_TRUE, SERVICE_WIFI));

    m_pAosBlock->ClearAllBlockReasons();
    EXPECT_TRUE(m_pAosBlock->IsCleared());
}

TEST_F(AosBlockTest, IsReasonBlocked_NotOnlyEnabled)
{
    EXPECT_TRUE(m_pAosBlock->IsCleared());

    m_pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    m_pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    m_pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);

    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);

    m_pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);

    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_AC_INCOMPLETED));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));

    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_AIRPLANE_MODE_ON));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_NO_NETWORK));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_OUT_OF_SERVICE));

    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_WIFI_BAD_CONNECTION));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE));
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_WIFI_AIRPLANE_MODE_ON));
}

TEST_F(AosBlockTest, IsCleared_Cellular)
{
    EXPECT_TRUE(m_pAosBlock->IsCleared());

    m_pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    m_pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    m_pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);

    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);

    m_pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);

    ImsList<IMS_UINT32> objReason;
    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(objReason.GetSize(), 9);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(objReason.GetSize(), 6);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(objReason.GetSize(), 6);

    EXPECT_FALSE(m_pAosBlock->IsCleared(SERVICE_CELLULAR));

    m_pAosBlock->ClearAllBlockReasons();
    EXPECT_TRUE(m_pAosBlock->IsCleared(SERVICE_CELLULAR));
}

TEST_F(AosBlockTest, IsCleared_Wifi)
{
    EXPECT_TRUE(m_pAosBlock->IsCleared());

    m_pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    m_pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    m_pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);

    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);

    m_pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);

    ImsList<IMS_UINT32> objReason;
    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(objReason.GetSize(), 9);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(objReason.GetSize(), 6);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(objReason.GetSize(), 6);

    EXPECT_FALSE(m_pAosBlock->IsCleared(SERVICE_WIFI));

    m_pAosBlock->ClearAllBlockReasons();
    EXPECT_TRUE(m_pAosBlock->IsCleared(SERVICE_WIFI));
}

TEST_F(AosBlockTest, IsCleared_Whole)
{
    EXPECT_TRUE(m_pAosBlock->IsCleared());

    m_pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    m_pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    m_pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);

    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);

    m_pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);

    ImsList<IMS_UINT32> objReason;
    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(objReason.GetSize(), 9);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(objReason.GetSize(), 6);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(objReason.GetSize(), 6);

    EXPECT_FALSE(m_pAosBlock->IsCleared(SERVICE_WHOLE));

    m_pAosBlock->ClearAllBlockReasons();
    EXPECT_TRUE(m_pAosBlock->IsCleared(SERVICE_WHOLE));
}

TEST_P(AosBlockTest, BlockReasonToString)
{
    // GIVEN
    const FormattingTestCase& objTestCase = GetParam();

    // WHEN
    const IMS_CHAR* pszActual = m_pAosBlock->BlockReasonToString(objTestCase.nReason);

    // THEN
    EXPECT_STREQ(objTestCase.pszString, pszActual);
}
