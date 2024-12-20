/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "PlatformContext.h"
#include "TestPhoneInfoService.h"

#include "SipUrnHelper.h"

using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Unused;

namespace android
{

class SipUrnHelperTest : public ::testing::Test
{
protected:
    TestPhoneInfoService m_objPhoneInfoService;

    AString m_strImei;
    AString m_strSv;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);

        EXPECT_CALL(m_objPhoneInfoService.GetMockDeviceInfo(), GetDeviceId)
                .Times(AnyNumber())
                .WillRepeatedly(Invoke(
                        [&](Unused, AString& strImei)
                        {
                            strImei = m_strImei;
                            return IMS_TRUE;
                        }));
        EXPECT_CALL(m_objPhoneInfoService.GetMockDeviceInfo(), GetDeviceSoftwareVersion)
                .Times(AnyNumber())
                .WillRepeatedly(Invoke(
                        [&](Unused, AString& strSv)
                        {
                            strSv = m_strSv;
                            return IMS_TRUE;
                        }));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
    }
};

TEST_F(SipUrnHelperTest, GetUrn)
{
    AString strUrnImei = "urn:gsma:imei:00000000-000000-0";
    AString strUrnImeiSv = "urn:gsma:imei:00000000-000000-0;svn=00";

    EXPECT_EQ(SipUrnHelper::GetUrn(IMS_SLOT_0, SipUrnHelper::GSMA_IMEI), strUrnImei);
    EXPECT_EQ(SipUrnHelper::GetUrn(IMS_SLOT_0, SipUrnHelper::GSMA_IMEISV), strUrnImeiSv);

    m_strImei = "11223344";
    m_strSv = "1";
    strUrnImei = "urn:gsma:imei:11223344-000000-0";
    strUrnImeiSv = "urn:gsma:imei:11223344-000000-0;svn=01";

    EXPECT_EQ(SipUrnHelper::GetUrn(IMS_SLOT_0, SipUrnHelper::GSMA_IMEI), strUrnImei);
    EXPECT_EQ(SipUrnHelper::GetUrn(IMS_SLOT_0, SipUrnHelper::GSMA_IMEISV), strUrnImeiSv);

    m_strImei = "1122334455667788";
    m_strSv = "123";
    strUrnImei = "urn:gsma:imei:11223344-556677-0";
    strUrnImeiSv = "urn:gsma:imei:11223344-556677-0;svn=23";

    EXPECT_EQ(SipUrnHelper::GetUrn(IMS_SLOT_0, SipUrnHelper::GSMA_IMEI), strUrnImei);
    EXPECT_EQ(SipUrnHelper::GetUrn(IMS_SLOT_0, SipUrnHelper::GSMA_IMEISV), strUrnImeiSv);

    m_strImei = "11223344556677";
    const AString strUrnUuidPrefix("urn:uuid:");
    AString strUrnUuid;

    strUrnUuid = SipUrnHelper::GetUrn(IMS_SLOT_0, SipUrnHelper::UUID_IMEI_MD5);
    EXPECT_TRUE(strUrnUuid.StartsWith(strUrnUuidPrefix));

    strUrnUuid = SipUrnHelper::GetUrn(IMS_SLOT_0, SipUrnHelper::UUID_IMEI_SHA1);
    EXPECT_TRUE(strUrnUuid.StartsWith(strUrnUuidPrefix));

    strUrnUuid = SipUrnHelper::GetUrn(IMS_SLOT_0, SipUrnHelper::UUID_IMEI_NAMED_V3);
    EXPECT_TRUE(strUrnUuid.StartsWith(strUrnUuidPrefix));

    strUrnUuid = SipUrnHelper::GetUrn(IMS_SLOT_0, SipUrnHelper::UUID_IMEI_NAMED_V5);
    EXPECT_TRUE(strUrnUuid.StartsWith(strUrnUuidPrefix));

    strUrnUuid = SipUrnHelper::GetUrn(IMS_SLOT_0, SipUrnHelper::UUID_IMEI_V4);
    EXPECT_TRUE(strUrnUuid.StartsWith(strUrnUuidPrefix));

    strUrnUuid = SipUrnHelper::GetUrn(IMS_SLOT_0, -1);
    EXPECT_EQ(strUrnUuid.GetLength(), 0);
}

}  // namespace android
