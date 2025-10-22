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

#include "ImsAccessNetworkInfoType.h"
#include "ImsIdentity.h"
#include "PlatformContext.h"
#include "TestPhoneInfoService.h"
#include "private/ConfigurationManager.h"

using ::testing::AnyNumber;
using ::testing::Invoke;

namespace android
{

class ImsIdentityTest : public ::testing::Test
{
protected:
    TestPhoneInfoService m_objPhoneInfoService;

    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);

        EXPECT_CALL(m_objPhoneInfoService.GetMockSubscriberInfo(), GetPhoneNumber)
                .Times(AnyNumber())
                .WillRepeatedly(Invoke(
                        [&](AString& strPhoneNumber)
                        {
                            strPhoneNumber = "6587323423";
                            return IMS_TRUE;
                        }));
        EXPECT_CALL(m_objPhoneInfoService.GetMockSubscriberInfo(), GetSimMcc)
                .Times(AnyNumber())
                .WillRepeatedly(Invoke(
                        [&](AString& strMcc)
                        {
                            strMcc = "414";
                            return IMS_TRUE;
                        }));
        EXPECT_CALL(m_objPhoneInfoService.GetMockSubscriberInfo(), GetSimMnc)
                .Times(AnyNumber())
                .WillRepeatedly(Invoke(
                        [&](AString& strMnc)
                        {
                            strMnc = "01";
                            return IMS_TRUE;
                        }));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
    }
};

TEST_F(ImsIdentityTest, CreateSipUserIdWithPhoneWithPhoneContext)
{
    const IMS_SINT32 SLOT_ID = 0;
    AString strDialString = "#SomeDialString";
    AString strPhoneContext = "some.Phone.Context";
    AString strExpectedSipUri =
            "sip:%23SomeDialString;phone-context=some.Phone.Context@;user=phone";

    AString strUri = ImsIdentity::CreateSipUserIdWithPhone(strDialString, SLOT_ID, strPhoneContext);

    EXPECT_EQ(strExpectedSipUri, strUri);
}

TEST_F(ImsIdentityTest, CreateSipUserIdWithPhoneWithoutPhoneContext)
{
    const IMS_SINT32 SLOT_ID = 0;
    AString strDialString = "#SomeDialString";
    AString strExpectedSipUri = "sip:%23SomeDialString;phone-context=@;user=phone";

    AString strUri = ImsIdentity::CreateSipUserIdWithPhone(strDialString, SLOT_ID);

    EXPECT_EQ(strExpectedSipUri, strUri);
}

TEST_F(ImsIdentityTest, CreateSipUserId)
{
    AString strExpectedSipUri = "sip:6587323423@;user=phone";

    AString strUri = ImsIdentity::CreateSipUserId(IMS_SLOT_0, IMS_TRUE);
    EXPECT_EQ(strExpectedSipUri, strUri);

    strExpectedSipUri = "sip:6587323423@";
    strUri = ImsIdentity::CreateSipUserId(AString::ConstNull(), IMS_SLOT_0, IMS_TRUE);
    EXPECT_EQ(strExpectedSipUri, strUri);

    strExpectedSipUri = "sip:%234444@test.com.ut;user=phone";
    strUri = ImsIdentity::CreateSipUserId(AString("#4444"), IMS_SLOT_0, IMS_TRUE, "test.com.ut");
    EXPECT_EQ(strExpectedSipUri, strUri);
}

TEST_F(ImsIdentityTest, CreateSipUserIdWithDialString)
{
    AString strDialString = "#1234567890";
    AString strPhoneContext = "test.com.ut";
    AString strExpectedSipUri = "sip:%231234567890;phone-context=test.com.ut@;user=dialstring";

    AString strUri =
            ImsIdentity::CreateSipUserIdWithDialString(strDialString, IMS_SLOT_0, strPhoneContext);
    EXPECT_EQ(strExpectedSipUri, strUri);

    EXPECT_EQ(ImsIdentity::CreateSipUserIdWithDialString(
                      AString::ConstNull(), IMS_SLOT_0, strPhoneContext),
            AString::ConstNull());

    strUri = ImsIdentity::CreateSipUserIdWithDialString(
            strDialString, IMS_SLOT_0, AString::ConstNull());
    EXPECT_EQ("sip:%231234567890;phone-context=@;user=dialstring", strUri);
}

TEST_F(ImsIdentityTest, CreateTelUserId)
{
    AString strPhoneContext = "test.com.ut";
    AString strExpectedValue = "tel:6587323423;phone-context=test.com.ut";

    AString strUri =
            ImsIdentity::CreateTelUserId(AString::ConstNull(), strPhoneContext, IMS_SLOT_0);

    EXPECT_EQ(strExpectedValue, strUri);

    strExpectedValue = "tel:1111;phone-context=test.com.ut";
    strUri = ImsIdentity::CreateTelUserId(AString("1111"), strPhoneContext, IMS_SLOT_0);

    EXPECT_EQ(strExpectedValue, strUri);

    EXPECT_EQ(ImsIdentity::CreateTelUserId(AString("1111"), AString::ConstNull(), IMS_SLOT_0),
            AString("tel:1111;phone-context="));
}

TEST_F(ImsIdentityTest, CreateTemporaryHomeDomainName)
{
    AString strExpectedValue = "ims.mnc001.mcc414.3gppnetwork.org";

    AString strUri = ImsIdentity::CreateTemporaryHomeDomainName(IMS_SLOT_0);

    EXPECT_EQ(strExpectedValue, strUri);
}

TEST_F(ImsIdentityTest, CreateTemporaryPrivateAndPublicUserId)
{
    EXPECT_CALL(m_objPhoneInfoService.GetMockSubscriberInfo(), GetSubscriberId)
            .Times(2)
            .WillRepeatedly(Invoke(
                    [&](AString& strImsi)
                    {
                        strImsi = AString::ConstNull();
                        return IMS_TRUE;
                    }));
    EXPECT_EQ(ImsIdentity::CreateTemporaryPrivateUserId(IMS_SLOT_0), AString::ConstNull());
    EXPECT_EQ(ImsIdentity::CreateTemporaryPublicUserId(IMS_SLOT_0), AString::ConstNull());

    EXPECT_CALL(m_objPhoneInfoService.GetMockSubscriberInfo(), GetSubscriberId)
            .Times(2)
            .WillRepeatedly(Invoke(
                    [&](AString& strImsi)
                    {
                        strImsi = "123123123456";
                        return IMS_TRUE;
                    }));
    AString strExpectedSipUri = "123123123456@ims.mnc001.mcc414.3gppnetwork.org";
    AString strUri = ImsIdentity::CreateTemporaryPrivateUserId(IMS_SLOT_0);
    EXPECT_EQ(strExpectedSipUri, strUri);

    strExpectedSipUri = "sip:123123123456@ims.mnc001.mcc414.3gppnetwork.org";
    strUri = ImsIdentity::CreateTemporaryPublicUserId(IMS_SLOT_0);
    EXPECT_EQ(strExpectedSipUri, strUri);
}

TEST_F(ImsIdentityTest, GetAnonymousUserId)
{
    EXPECT_EQ(ImsIdentity::GetAnonymousUserId(), AString("sip:anonymous@anonymous.invalid"));
}

TEST_F(ImsIdentityTest, GetUnavailableUserId)
{
    EXPECT_EQ(ImsIdentity::GetUnavailableUserId(), AString("sip:unavailable@unknown.invalid"));
}

TEST_F(ImsIdentityTest, GetPhoneContext)
{
    ConfigurationManager* pConfigurationManager = ConfigurationManager::GetInstance();
    ASSERT_TRUE(pConfigurationManager != IMS_NULL);
    EXPECT_TRUE(pConfigurationManager->Initialize());

    AccessNetworkInfo objAni;
    // LTE Info
    objAni.nClass = AccessNetworkInfo::CLASS_3GPP_E_UTRAN;
    IMS_MEM_Memset(&objAni.uniAI, 0, sizeof(objAni.uniAI));

    EXPECT_EQ(ImsIdentity::GetPhoneContext(
                      ImsIdentity::DIALING_POLICY_GEO_LOCAL, IMS_SLOT_0, &objAni),
            AString("414.01.eps."));

    AString strCellInfo("4140010011003b932");
    IMS_MEM_Memcpy(&objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[0], strCellInfo.GetStr(),
            strCellInfo.GetLength());

    EXPECT_EQ((ImsIdentity::GetPhoneContext(
                      ImsIdentity::DIALING_POLICY_GEO_LOCAL, IMS_SLOT_0, &objAni)),
            AString("414.001.eps."));

    // NR Info
    objAni.nClass = AccessNetworkInfo::CLASS_3GPP_NR;
    IMS_MEM_Memset(&objAni.uniAI, 0, sizeof(objAni.uniAI));

    EXPECT_EQ(ImsIdentity::GetPhoneContext(
                      ImsIdentity::DIALING_POLICY_GEO_LOCAL, IMS_SLOT_0, &objAni),
            AString("414.01.5gs."));

    IMS_MEM_Memcpy(&objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[0], strCellInfo.GetStr(),
            strCellInfo.GetLength());

    EXPECT_EQ(ImsIdentity::GetPhoneContext(
                      ImsIdentity::DIALING_POLICY_GEO_LOCAL, IMS_SLOT_0, &objAni),
            AString("414.001.5gs."));

    // WLAN Info
    objAni.nClass = AccessNetworkInfo::CLASS_3GPP_WLAN;
    IMS_MEM_Memset(&objAni.uniAI, 0, sizeof(objAni.uniAI));

    EXPECT_EQ(ImsIdentity::GetPhoneContext(
                      ImsIdentity::DIALING_POLICY_GEO_LOCAL, IMS_SLOT_0, &objAni),
            AString("geo-local."));

    IMS_MEM_Memcpy(&objAni.uniAI.i_wlan_node_id.acMAC[0], "283b82a9d534", strlen("283b82a9d534"));

    EXPECT_EQ(ImsIdentity::GetPhoneContext(
                      ImsIdentity::DIALING_POLICY_GEO_LOCAL, IMS_SLOT_0, &objAni),
            AString("geo-local."));

    // GPRS Info
    objAni.nClass = AccessNetworkInfo::CLASS_3GPP_GERAN;
    IMS_MEM_Memset(&objAni.uniAI, 0, sizeof(objAni.uniAI));
    objAni.uniAI.cgi_3gpp.aPLMNId[0] = 64;
    objAni.uniAI.cgi_3gpp.aPLMNId[1] = 88;
    objAni.uniAI.cgi_3gpp.aPLMNId[2] = 97;

    EXPECT_EQ(ImsIdentity::GetPhoneContext(
                      ImsIdentity::DIALING_POLICY_GEO_LOCAL, IMS_SLOT_0, &objAni),
            AString("405.861.gprs."));

    IMS_MEM_Memset(&objAni.uniAI, 0, sizeof(objAni.uniAI));
    IMS_MEM_Memcpy(&objAni.uniAI.cgi_3gpp.acCGI[0], strCellInfo.GetStr(), strCellInfo.GetLength());

    EXPECT_EQ(ImsIdentity::GetPhoneContext(
                      ImsIdentity::DIALING_POLICY_GEO_LOCAL, IMS_SLOT_0, &objAni),
            AString("414.001.gprs."));

    objAni.nClass = AccessNetworkInfo::CLASS_3GPP_UTRAN;
    IMS_MEM_Memset(&objAni.uniAI, 0, sizeof(objAni.uniAI));
    objAni.uniAI.utran_cell_id_3gpp.aPLMNId[0] = 64;
    objAni.uniAI.utran_cell_id_3gpp.aPLMNId[1] = 88;
    objAni.uniAI.utran_cell_id_3gpp.aPLMNId[2] = 97;

    EXPECT_EQ(ImsIdentity::GetPhoneContext(
                      ImsIdentity::DIALING_POLICY_GEO_LOCAL, IMS_SLOT_0, &objAni),
            AString("405.861.gprs."));

    IMS_MEM_Memset(&objAni.uniAI, 0, sizeof(objAni.uniAI));
    IMS_MEM_Memcpy(&objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[0], strCellInfo.GetStr(),
            strCellInfo.GetLength());

    EXPECT_EQ(ImsIdentity::GetPhoneContext(
                      ImsIdentity::DIALING_POLICY_GEO_LOCAL, IMS_SLOT_0, &objAni),
            AString("414.001.gprs."));

    pConfigurationManager->DestroyConfigs();
}

}  // namespace android
