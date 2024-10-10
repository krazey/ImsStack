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

#include "IuMtsService.h"
#include "MtsStringDef.h"
#include <gtest/gtest.h>

namespace android
{

class MtsStringDefTest : public ::testing::Test
{
public:
    MtsStringDef* pMtsStringDef;

protected:
    virtual void SetUp() override { pMtsStringDef = new MtsStringDef(); }

    virtual void TearDown() override { delete pMtsStringDef; }
};

TEST_F(MtsStringDefTest, Constructor)
{
    ASSERT_NE(pMtsStringDef, nullptr);
}

TEST_F(MtsStringDefTest, PrintStringAccessNetworkType)
{
    EXPECT_STREQ(PS_AccessNetworkType(IImsRadio::ACCESS_NETWORK_TYPE_UTRAN), "UTRAN");
    EXPECT_STREQ(PS_AccessNetworkType(IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN), "EUTRAN");
    EXPECT_STREQ(PS_AccessNetworkType(IImsRadio::ACCESS_NETWORK_TYPE_NGRAN), "NGRAN");
    EXPECT_STREQ(PS_AccessNetworkType(IImsRadio::ACCESS_NETWORK_TYPE_IWLAN), "IWLAN");
    EXPECT_STREQ(PS_AccessNetworkType(IImsRadio::ACCESS_NETWORK_TYPE_UNKNOWN), "__INVALID__");
}

TEST_F(MtsStringDefTest, PrintStringIpcan)
{
    EXPECT_STREQ(PS_Ipcan(IIpcan::CATEGORY_MOBILE), "CATEGORY_MOBILE");
    EXPECT_STREQ(PS_Ipcan(IIpcan::CATEGORY_WLAN), "CATEGORY_WLAN");
    EXPECT_STREQ(PS_Ipcan(IIpcan::CATEGORY_ANY), "CATEGORY_ANY");
    EXPECT_STREQ(PS_Ipcan(-1), "__INVALID__");
}

TEST_F(MtsStringDefTest, PrintStringMtiStringFrom3gpp)
{
    EXPECT_STREQ(
            PS_MtiStringFrom3gpp(SMS_3GPP_MTI_RP_DATA_FROM_MS), "SMS_3GPP_MTI_RP_DATA_FROM_MS");
    EXPECT_STREQ(PS_MtiStringFrom3gpp(SMS_3GPP_MTI_RP_DATA_FROM_N), "SMS_3GPP_MTI_RP_DATA_FROM_N");
    EXPECT_STREQ(PS_MtiStringFrom3gpp(SMS_3GPP_MTI_RP_ACK_FROM_MS), "SMS_3GPP_MTI_RP_ACK_FROM_MS");
    EXPECT_STREQ(PS_MtiStringFrom3gpp(SMS_3GPP_MTI_RP_ACK_FROM_N), "SMS_3GPP_MTI_RP_ACK_FROM_N");
    EXPECT_STREQ(
            PS_MtiStringFrom3gpp(SMS_3GPP_MTI_RP_ERROR_FROM_MS), "SMS_3GPP_MTI_RP_ERROR_FROM_MS");
    EXPECT_STREQ(
            PS_MtiStringFrom3gpp(SMS_3GPP_MTI_RP_ERROR_FROM_N), "SMS_3GPP_MTI_RP_ERROR_FROM_N");
    EXPECT_STREQ(PS_MtiStringFrom3gpp(SMS_3GPP_MTI_RP_SMMA), "SMS_3GPP_MTI_RP_SMMA");
    EXPECT_STREQ(PS_MtiStringFrom3gpp(SMS_MTI_NONE), "__INVALID__");
}

TEST_F(MtsStringDefTest, PrintStringMtiStringFrom3gpp2)
{
    EXPECT_STREQ(
            PS_MtiStringFrom3gpp2(SMS_3GPP2_MTI_POINT_TO_POINT), "SMS_3GPP2_MTI_POINT_TO_POINT");
    EXPECT_STREQ(PS_MtiStringFrom3gpp2(SMS_3GPP2_MTI_BROADCAST), "SMS_3GPP2_MTI_BROADCAST");
    EXPECT_STREQ(PS_MtiStringFrom3gpp2(SMS_3GPP2_MTI_ACKNOWLEDGE), "SMS_3GPP2_MTI_ACKNOWLEDGE");
    EXPECT_STREQ(PS_MtiStringFrom3gpp2(SMS_MTI_NONE), "__INVALID__");
}

TEST_F(MtsStringDefTest, PrintStringMoStatus)
{
    EXPECT_STREQ(PS_MoStatus(MO_SUCCESS), "MO_SUCCESS");
    EXPECT_STREQ(PS_MoStatus(MO_ERROR_RETRY), "MO_ERROR_RETRY");
    EXPECT_STREQ(PS_MoStatus(MO_ERROR_GENERIC), "MO_ERROR_GENERIC");
    EXPECT_STREQ(PS_MoStatus(MO_INVALID), "__INVALID__");
}

TEST_F(MtsStringDefTest, PrintStringRadioTechType)
{
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_UNKNOWN), "UNKNOWN");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_GPRS), "GPRS");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_EDGE), "EDGE");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_UMTS), "UMTS");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_CDMA), "CDMA");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_EVDO_0), "EVDO_0");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_EVDO_A), "EVDO_A");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_1xRTT), "1xRTT");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_HSDPA), "HSDPA");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_HSUPA), "HSUPA");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_HSPA), "HSPA");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_IDEN), "IDEN");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_EVDO_B), "EVDO_B");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_LTE), "LTE");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_EHRPD), "EHRPD");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_HSPAP), "HSPAP");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_GSM), "GSM");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_TD_SCDMA), "TD_SCDMA");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_IWLAN), "IWLAN");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_LTE_CA), "LTE_CA");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_NR), "NR");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_MAX), "MAX");
    EXPECT_STREQ(PS_RadioTechType(INetworkWatcher::RADIOTECH_TYPE_INVALID), "__INVALID__");
}

TEST_F(MtsStringDefTest, PrintStringServiceState)
{
    EXPECT_STREQ(PS_ServiceState(STATE_INIT), "STATE_INIT");
    EXPECT_STREQ(PS_ServiceState(STATE_READY), "STATE_READY");
    EXPECT_STREQ(PS_ServiceState(STATE_LIMITED), "STATE_LIMITED");
    EXPECT_STREQ(PS_ServiceState(STATE_NOTREADY), "STATE_NOTREADY");
    EXPECT_STREQ(PS_ServiceState(-1), "__INVALID__");
}

TEST_F(MtsStringDefTest, PrintStringSmsFormatType)
{
    EXPECT_STREQ(PS_SmsFormatType(SmsFormatType::SMSFORMAT_3GPP), "3GPP");
    EXPECT_STREQ(PS_SmsFormatType(SmsFormatType::SMSFORMAT_3GPP2), "3GPP2");
    EXPECT_STREQ(PS_SmsFormatType(SmsFormatType::SMSFORMAT_INVALID), "__INVALID__");
}

TEST_F(MtsStringDefTest, PrintStringTrafficDirection)
{
    EXPECT_STREQ(PS_TrafficDirection(IImsRadio::DIRECTION_MO), "MO");
    EXPECT_STREQ(PS_TrafficDirection(IImsRadio::DIRECTION_MT), "MT");
    EXPECT_STREQ(PS_TrafficDirection(-1), "__INVALID__");
}

TEST_F(MtsStringDefTest, PrintStringTrafficType)
{
    EXPECT_STREQ(PS_TrafficType(IImsRadio::TRAFFIC_TYPE_SMS), "SMS");
    EXPECT_STREQ(PS_TrafficType(IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS), "EMERGENCY_SMS");
    EXPECT_STREQ(PS_TrafficType(IImsRadio::TRAFFIC_TYPE_REGISTRATION), "__INVALID__");
}

}  // namespace android
