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

#include "CarrierConfig.h"
#include "MockICarrierConfig.h"
#include "PlatformContext.h"
#include "TestConfigService.h"
#include "configuration/MtcConfigurationProxy.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

LOCAL const IMS_CHAR VALID_KEY_BOOL[] = "valid_key_bool";
LOCAL const IMS_CHAR INVALID_KEY_BOOL[] = "invalid_key_bool";
LOCAL const IMS_CHAR VALID_KEY_INT[] = "valid_key_int";
LOCAL const IMS_CHAR INVALID_KEY_INT[] = "invalid_key_int";
LOCAL const IMS_CHAR VALID_KEY_STRING[] = "valid_key_string";
LOCAL const IMS_CHAR INVALID_KEY_STRING[] = "invalid_key_string";
LOCAL const IMS_CHAR VALID_KEY_INT_ARRAY[] = "valid_key_int_array";
LOCAL const IMS_CHAR INVALID_KEY_INT_ARRAY[] = "invalid_key_int_array";
LOCAL const IMS_CHAR VALID_KEY_STRING_ARRAY[] = "valid_key_string_array";
LOCAL const IMS_CHAR INVALID_KEY_STRING_ARRAY[] = "invalid_key_string_array";

LOCAL const IMS_BOOL VALID_BOOL_VALUE = IMS_TRUE;
LOCAL const IMS_BOOL INVALID_BOOL_VALUE = IMS_FALSE;
LOCAL const IMS_SINT32 VALID_INT_VALUE = 1;
LOCAL const IMS_SINT32 INVALID_INT_VALUE = -1;
LOCAL const IMS_SINT32 VALID_INT_VALUE2 = 2;
LOCAL const AString VALID_STRING_VALUE = "valid_string_value";
LOCAL const AString INVALID_STRING_VALUE = "invalid_string_value";
LOCAL const AString VALID_STRING_VALUE2 = "valid_string_value2";

class MtcConfigurationProxyTest : public ::testing::Test
{
public:
    inline MtcConfigurationProxyTest() :
            pProxy(IMS_NULL),
            objConfigService(),
            piCc(IMS_NULL),
            objIntArray(),
            objStringArray()
    {
    }

    MtcConfigurationProxy* pProxy;
    TestConfigService objConfigService;
    MockICarrierConfig* piCc;
    ImsVector<IMS_SINT32> objIntArray;
    ImsVector<AString> objStringArray;
    ImsVector<IMS_SINT32> objEmptyIntArray;
    ImsVector<AString> objEmptyStringArray;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &objConfigService);
        piCc = &objConfigService.GetMockCarrierConfig();
        pProxy = new MtcConfigurationProxy();

        ON_CALL(*piCc, GetBoolean(VALID_KEY_BOOL, _)).WillByDefault(Return(VALID_BOOL_VALUE));
        ON_CALL(*piCc, GetBoolean(INVALID_KEY_BOOL, _)).WillByDefault(Return(INVALID_BOOL_VALUE));

        ON_CALL(*piCc, GetInt(VALID_KEY_INT, _)).WillByDefault(Return(VALID_INT_VALUE));
        ON_CALL(*piCc, GetInt(INVALID_KEY_INT, _)).WillByDefault(Return(INVALID_INT_VALUE));

        ON_CALL(*piCc, GetString(VALID_KEY_STRING, _)).WillByDefault(Return(VALID_STRING_VALUE));
        ON_CALL(*piCc, GetString(INVALID_KEY_STRING, _))
                .WillByDefault(Return(INVALID_STRING_VALUE));

        objIntArray.Push(VALID_INT_VALUE);
        objIntArray.Push(VALID_INT_VALUE2);
        ON_CALL(*piCc, GetIntArray(VALID_KEY_INT_ARRAY)).WillByDefault(Return(objIntArray));
        ON_CALL(*piCc, GetIntArray(VALID_KEY_STRING_ARRAY)).WillByDefault(Return(objEmptyIntArray));

        objStringArray.Push(VALID_STRING_VALUE);
        objStringArray.Push(VALID_STRING_VALUE2);
        ON_CALL(*piCc, GetStringArray(VALID_KEY_STRING_ARRAY))
                .WillByDefault(Return(objStringArray));
        ON_CALL(*piCc, GetStringArray(VALID_KEY_INT_ARRAY))
                .WillByDefault(Return(objEmptyStringArray));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
        delete pProxy;
    }
};

TEST_F(MtcConfigurationProxyTest, GetBooleanReturnsValidValue)
{
    EXPECT_EQ(pProxy->GetBoolean(VALID_KEY_BOOL), VALID_BOOL_VALUE);
}

TEST_F(MtcConfigurationProxyTest, GetBooleanReturnsDefaultValueIfKeyIsInvalid)
{
    EXPECT_EQ(pProxy->GetBoolean(INVALID_KEY_BOOL), INVALID_BOOL_VALUE);
}

TEST_F(MtcConfigurationProxyTest, GetBooleanReturnsCachingValueIfExitsts)
{
    pProxy->PutCache(INVALID_KEY_BOOL, VALID_BOOL_VALUE);
    EXPECT_EQ(pProxy->GetBoolean(INVALID_KEY_BOOL), VALID_BOOL_VALUE);

    pProxy->OnRegistrationRefreshed();
    EXPECT_EQ(pProxy->GetBoolean(INVALID_KEY_BOOL), INVALID_BOOL_VALUE);
}

TEST_F(MtcConfigurationProxyTest, GetIntReturnsValidValue)
{
    EXPECT_EQ(pProxy->GetInt(VALID_KEY_INT), VALID_INT_VALUE);
}

TEST_F(MtcConfigurationProxyTest, GetIntReturnsDefaultValueIfKeyIsInvalid)
{
    EXPECT_EQ(pProxy->GetInt(INVALID_KEY_INT), INVALID_INT_VALUE);
}

TEST_F(MtcConfigurationProxyTest, GetIntReturnsCachingValueIfExitsts)
{
    pProxy->PutCache(INVALID_KEY_INT, VALID_INT_VALUE);
    EXPECT_EQ(pProxy->GetInt(INVALID_KEY_INT), VALID_INT_VALUE);

    pProxy->OnRegistrationRefreshed();
    EXPECT_EQ(pProxy->GetInt(INVALID_KEY_INT), INVALID_INT_VALUE);
}

TEST_F(MtcConfigurationProxyTest, GetStringReturnsValidValue)
{
    EXPECT_EQ(pProxy->GetString(VALID_KEY_STRING), VALID_STRING_VALUE);
}

TEST_F(MtcConfigurationProxyTest, GetStringReturnsDefaultValueIfKeyIsInvalid)
{
    EXPECT_EQ(pProxy->GetString(INVALID_KEY_STRING), INVALID_STRING_VALUE);
}

TEST_F(MtcConfigurationProxyTest, GetStringReturnsCachingValueIfExitsts)
{
    pProxy->PutCache(INVALID_KEY_STRING, VALID_STRING_VALUE.GetStr());
    EXPECT_EQ(pProxy->GetString(INVALID_KEY_STRING), VALID_STRING_VALUE);

    pProxy->OnRegistrationRefreshed();
    EXPECT_EQ(pProxy->GetString(INVALID_KEY_STRING), INVALID_STRING_VALUE);
}

TEST_F(MtcConfigurationProxyTest, GetIntArrayReturnsValue)
{
    EXPECT_EQ(pProxy->GetIntArray(VALID_KEY_INT_ARRAY), objIntArray);
}

TEST_F(MtcConfigurationProxyTest, GetIntArrayReturnsDefaultValueIfKeyIsInvalid)
{
    ImsVector<IMS_SINT32> objArray = pProxy->GetIntArray(INVALID_KEY_INT_ARRAY);
    EXPECT_EQ(objArray.GetSize(), 0);
}

TEST_F(MtcConfigurationProxyTest, GetStringArrayReturnsValue)
{
    EXPECT_EQ(pProxy->GetStringArray(VALID_KEY_STRING_ARRAY), objStringArray);
}

TEST_F(MtcConfigurationProxyTest, GetStringArrayReturnsDefaultValueIfKeyIsInvalid)
{
    ImsVector<AString> objArray = pProxy->GetStringArray(INVALID_KEY_STRING_ARRAY);
    EXPECT_EQ(objArray.GetSize(), 0);
}

TEST_F(MtcConfigurationProxyTest, ContainsIntValueReturnsTrue)
{
    EXPECT_TRUE(pProxy->Contains(VALID_KEY_INT_ARRAY, VALID_INT_VALUE));
    EXPECT_TRUE(pProxy->Contains(VALID_KEY_INT_ARRAY, VALID_INT_VALUE2));
}

TEST_F(MtcConfigurationProxyTest, ContainsIntValueReturnsFalse)
{
    EXPECT_FALSE(pProxy->Contains(VALID_KEY_STRING_ARRAY, INVALID_INT_VALUE));
}

TEST_F(MtcConfigurationProxyTest, ContainsStringValueReturnsTrue)
{
    EXPECT_TRUE(pProxy->Contains(VALID_KEY_STRING_ARRAY, VALID_STRING_VALUE.GetStr()));
    EXPECT_TRUE(pProxy->Contains(VALID_KEY_STRING_ARRAY, VALID_STRING_VALUE.GetStr()));
}

TEST_F(MtcConfigurationProxyTest, ContainsStringValueReturnsFalse)
{
    EXPECT_FALSE(pProxy->Contains(VALID_KEY_INT_ARRAY, INVALID_STRING_VALUE.GetStr()));
}

TEST_F(MtcConfigurationProxyTest, GetIntFromArrayReturnsValue)
{
    EXPECT_EQ(pProxy->GetIntFromArray(VALID_KEY_INT_ARRAY, 0), VALID_INT_VALUE);
    EXPECT_EQ(pProxy->GetIntFromArray(VALID_KEY_INT_ARRAY, 1), VALID_INT_VALUE2);
}

TEST_F(MtcConfigurationProxyTest, GetIntFromArrayAssertOnInvalidIndex)
{
    EXPECT_DEATH(pProxy->GetIntFromArray(VALID_KEY_INT_ARRAY, 2), "");
}

TEST_F(MtcConfigurationProxyTest, GetStringFromArrayReturnsValue)
{
    EXPECT_EQ(pProxy->GetStringFromArray(VALID_KEY_STRING_ARRAY, 0), VALID_STRING_VALUE);
    EXPECT_EQ(pProxy->GetStringFromArray(VALID_KEY_STRING_ARRAY, 1), VALID_STRING_VALUE2);
}

TEST_F(MtcConfigurationProxyTest, GetStringFromArrayAssertOnInvalidIndex)
{
    EXPECT_DEATH(pProxy->GetStringFromArray(VALID_KEY_STRING_ARRAY, 2), "");
}
