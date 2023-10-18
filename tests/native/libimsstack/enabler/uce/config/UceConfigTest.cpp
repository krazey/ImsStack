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

#include "ImsTypeDef.h"
#include "config/UceConfig.h"
#include "config/UceAssetItems.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

class UceConfigTestTest : public ::testing::Test
{
public:
    UceConfig* pUceConfig;

protected:
    virtual void SetUp() override
    {
        pUceConfig = new UceConfig();
        ASSERT_TRUE(pUceConfig != nullptr);
    }

    virtual void TearDown() override
    {
        if (pUceConfig)
        {
            delete pUceConfig;
        }
    }
};

TEST_F(UceConfigTestTest, GetAStringValue)
{
    IMS_TRACE_D("GetAStringValue", 0, 0, 0);
    AString rlsUri = AString::ConstEmpty();
    AString returnValue = pUceConfig->GetAStringValue(UceConfig::KEY_RLS_URI, 10);
    EXPECT_STREQ(returnValue.GetStr(), rlsUri.GetStr());

    rlsUri = "TestUri";
    UceAssetItems* objNew = new UceAssetItems();
    objNew->m_strRlsUri = rlsUri;
    pUceConfig->SetConfig(10, objNew);
    returnValue = pUceConfig->GetAStringValue(UceConfig::KEY_RLS_URI, 10);
    EXPECT_STREQ(returnValue.GetStr(), rlsUri.GetStr());
}

TEST_F(UceConfigTestTest, GetIntValue)
{
    IMS_TRACE_D("GetIntValue", 0, 0, 0);
    IMS_UINT32 expectedValue = 0;
    IMS_UINT32 returnValue = pUceConfig->GetIntValue(UceConfig::KEY_EXPIRE_VALUE_PUBLISH, 10);
    EXPECT_EQ(returnValue, expectedValue);

    UceAssetItems* objNew = new UceAssetItems();
    // KEY_EXPIRE_VALUE_PUBLISH
    IMS_UINT32 expireValuePublish = 1200;
    objNew->m_nExpireValuePublish = expireValuePublish;
    // KEY_EXTENDED_EXPIRE_VALUE_PUBLISH
    IMS_UINT32 extendedExpireValuePublish = 3600;
    objNew->m_nExtendedExpireValuePublish = extendedExpireValuePublish;
    // KEY_PUBLISH_REFRESH_RATIO
    IMS_UINT32 publishRefreshRatio = 85;
    objNew->m_nPublishRefreshRatio = publishRefreshRatio;
    // KEY_EXPIRE_VALUE_LIST_SUBSCRIBE
    IMS_UINT32 expireValueListSubscribe = 100;
    objNew->m_nExpireValueListSubscribe = expireValueListSubscribe;
    // KEY_ANONYMOUS_FETCH_METHOD_INT
    IMS_UINT32 anonymousFetchMethod = 1;
    objNew->m_nAnonymousFetchMethod = anonymousFetchMethod;
    // KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE_MAX_COUNT
    IMS_UINT32 immediatelyRetryPublishResponseMaxCount = 1;
    objNew->m_nImmediatelyRetryPublishResponseMaxCount = immediatelyRetryPublishResponseMaxCount;
    // KEY_RETRY_PUBLISH_RESPONSE_MAX_COUNT
    IMS_UINT32 retryPublishResponseMaxCount = 5;
    objNew->m_nRetryPublishResponseMaxCount = retryPublishResponseMaxCount;
    // KEY_RETRY_PUBLISH_RESPONSE_TIME_SEC
    IMS_UINT32 retryPublishResponseTimeSec = 100;
    objNew->m_nRetryPublishResponseTimeSec = retryPublishResponseTimeSec;
    // KEY_VARIABLE_RETRY_PUBLISH_RESPONSE_MAX_COUNT
    IMS_UINT32 variableRetryPublishResponseMaxCount = 5;
    objNew->m_nVariableRetryPublishResponseMaxCount = variableRetryPublishResponseMaxCount;

    pUceConfig->SetConfig(10, objNew);

    returnValue = pUceConfig->GetIntValue(UceConfig::KEY_EXPIRE_VALUE_PUBLISH, 10);
    EXPECT_EQ(returnValue, expireValuePublish);

    returnValue = pUceConfig->GetIntValue(UceConfig::KEY_EXTENDED_EXPIRE_VALUE_PUBLISH, 10);
    EXPECT_EQ(returnValue, extendedExpireValuePublish);

    returnValue = pUceConfig->GetIntValue(UceConfig::KEY_PUBLISH_REFRESH_RATIO, 10);
    EXPECT_EQ(returnValue, publishRefreshRatio);

    returnValue = pUceConfig->GetIntValue(UceConfig::KEY_EXPIRE_VALUE_LIST_SUBSCRIBE, 10);
    EXPECT_EQ(returnValue, expireValueListSubscribe);

    returnValue = pUceConfig->GetIntValue(UceConfig::KEY_ANONYMOUS_FETCH_METHOD_INT, 10);
    EXPECT_EQ(returnValue, anonymousFetchMethod);

    returnValue = pUceConfig->GetIntValue(
            UceConfig::KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE_MAX_COUNT, 10);
    EXPECT_EQ(returnValue, immediatelyRetryPublishResponseMaxCount);

    returnValue = pUceConfig->GetIntValue(UceConfig::KEY_RETRY_PUBLISH_RESPONSE_MAX_COUNT, 10);
    EXPECT_EQ(returnValue, retryPublishResponseMaxCount);

    returnValue = pUceConfig->GetIntValue(UceConfig::KEY_RETRY_PUBLISH_RESPONSE_TIME_SEC, 10);
    EXPECT_EQ(returnValue, retryPublishResponseTimeSec);

    returnValue =
            pUceConfig->GetIntValue(UceConfig::KEY_VARIABLE_RETRY_PUBLISH_RESPONSE_MAX_COUNT, 10);
    EXPECT_EQ(returnValue, variableRetryPublishResponseMaxCount);
}

TEST_F(UceConfigTestTest, GetBoolValue)
{
    IMS_TRACE_D("GetBoolValue", 0, 0, 0);
    IMS_BOOL returnValue = pUceConfig->GetBoolValue(UceConfig::KEY_ENCODE_SUBSCRIBE_BODY, 10);
    EXPECT_FALSE(returnValue);

    UceAssetItems* objNew = new UceAssetItems();

    // KEY_SUBSCRIBE_INDEPENDENT_OF_PUBLISH
    objNew->m_bSubscribeIndepedentOfPublish = IMS_TRUE;
    // KEY_ENCODE_PUBLISH_BODY
    objNew->m_bEncodePublishBody = IMS_TRUE;
    // KEY_ENCODE_SUBSCRIBE_BODY
    objNew->m_bEncodeSubscribeBody = IMS_TRUE;
    // KEY_SUPPORT_OPTIONS
    objNew->m_bSupportOptions = IMS_TRUE;
    // KEY_USE_CONTACT_HEADER_IN_PUBLISH
    objNew->m_bUseContactHeaderInPublish = IMS_TRUE;
    // KEY_USE_CONTACT_HEADER_IN_SUBSCRIBE
    objNew->m_bUseContactHeaderInSubscribe = IMS_TRUE;
    // KEY_ADD_VIDEO_TAG_CONTACT_HEADER_IN_PUBLISH
    objNew->m_bAddVideoTagContactHeaderInPublish = IMS_TRUE;

    pUceConfig->SetConfig(10, objNew);

    returnValue = pUceConfig->GetBoolValue(UceConfig::KEY_SUBSCRIBE_INDEPENDENT_OF_PUBLISH, 10);
    EXPECT_TRUE(returnValue);
    returnValue = pUceConfig->GetBoolValue(UceConfig::KEY_ENCODE_PUBLISH_BODY, 10);
    EXPECT_TRUE(returnValue);
    returnValue = pUceConfig->GetBoolValue(UceConfig::KEY_ENCODE_SUBSCRIBE_BODY, 10);
    EXPECT_TRUE(returnValue);
    returnValue = pUceConfig->GetBoolValue(UceConfig::KEY_SUPPORT_OPTIONS, 10);
    EXPECT_TRUE(returnValue);
    returnValue = pUceConfig->GetBoolValue(UceConfig::KEY_USE_CONTACT_HEADER_IN_PUBLISH, 10);
    EXPECT_TRUE(returnValue);
    returnValue = pUceConfig->GetBoolValue(UceConfig::KEY_USE_CONTACT_HEADER_IN_SUBSCRIBE, 10);
    EXPECT_TRUE(returnValue);
    returnValue =
            pUceConfig->GetBoolValue(UceConfig::KEY_ADD_VIDEO_TAG_CONTACT_HEADER_IN_PUBLISH, 10);
    EXPECT_TRUE(returnValue);
}

TEST_F(UceConfigTestTest, GetExponentialRetryPublishRespTimeArray)
{
    IMS_TRACE_D("GetExponentialRetryPublishRespTimeArray", 0, 0, 0);
    ImsVector<IMS_SINT32> returnValue = pUceConfig->GetExponentialRetryPublishRespTimeArray(10);
    EXPECT_EQ(returnValue.GetSize(), 0);

    UceAssetItems* objNew = new UceAssetItems();
    ImsVector<IMS_SINT32> objExponentialRetryTimeSec;
    objExponentialRetryTimeSec.Push(10);
    objNew->m_objVariableRetryPublishResponseTimeSec = objExponentialRetryTimeSec;
    pUceConfig->SetConfig(10, objNew);

    returnValue = pUceConfig->GetExponentialRetryPublishRespTimeArray(10);
    EXPECT_EQ(returnValue.GetSize(), 1);
    EXPECT_EQ(returnValue.GetAt(0), 10);
}

TEST_F(UceConfigTestTest, GetPublishRetryType)
{
    IMS_TRACE_D("GetPublishRetryType", 0, 0, 0);
    EXPECT_EQ(pUceConfig->GetPublishRetryType(200, 10), UceConfig::NONE);

    UceAssetItems* objNew = new UceAssetItems();
    ImsVector<IMS_SINT32> objVariableRetryPublishResponse;
    objVariableRetryPublishResponse.Push(200);
    objNew->m_objVariableRetryPublishResponse = objVariableRetryPublishResponse;
    pUceConfig->SetConfig(10, objNew);
    EXPECT_EQ(pUceConfig->GetPublishRetryType(200, 10), UceConfig::EXPONENTIAL);

    ImsVector<IMS_SINT32> objRetryPublishResponse;
    objRetryPublishResponse.Push(200);
    objNew->m_objRetryPublishResponse = objRetryPublishResponse;
    pUceConfig->SetConfig(10, objNew);
    EXPECT_EQ(pUceConfig->GetPublishRetryType(200, 10), UceConfig::RETRY);

    ImsVector<IMS_SINT32> objImmediatelyRetryPublishResponse;
    objImmediatelyRetryPublishResponse.Push(200);
    objNew->m_objImmediatelyRetryPublishResponse = objImmediatelyRetryPublishResponse;
    pUceConfig->SetConfig(10, objNew);
    EXPECT_EQ(pUceConfig->GetPublishRetryType(200, 10), UceConfig::IMMEDIATELY);

    EXPECT_EQ(pUceConfig->GetPublishRetryType(403, 10), UceConfig::NONE);
}

TEST_F(UceConfigTestTest, IsImsRegistrationRequired)
{
    IMS_TRACE_D("IsImsRegistrationRequired", 0, 0, 0);
    EXPECT_FALSE(pUceConfig->IsImsRegistrationRequired(IMS_TRUE, 200, 10));

    UceAssetItems* objNew = new UceAssetItems();
    ImsVector<IMS_SINT32> objReAttemptPublishResponse;
    objReAttemptPublishResponse.Push(200);
    objNew->m_objReAttemptRegistrationPublishResponse = objReAttemptPublishResponse;

    ImsVector<IMS_SINT32> objReAttemptSubscribeResponse;
    objReAttemptSubscribeResponse.Push(200);
    objNew->m_objReAttemptRegistrationSubscribeResponse = objReAttemptSubscribeResponse;

    pUceConfig->SetConfig(10, objNew);

    EXPECT_TRUE(pUceConfig->IsImsRegistrationRequired(IMS_TRUE, 200, 10));
    EXPECT_TRUE(pUceConfig->IsImsRegistrationRequired(IMS_FALSE, 200, 10));
    EXPECT_FALSE(pUceConfig->IsImsRegistrationRequired(IMS_FALSE, 403, 10));
}
