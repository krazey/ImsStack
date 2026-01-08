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

#include "IIpcan.h"
#include "INetworkWatcher.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"
#include "MockIImsAos.h"
#include "MockIImsAosInfo.h"
#include "ServiceNetworkPolicy.h"
#include "helper/MtcAosConnector.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

class MtcAosConnectorTest : public ::testing::Test
{
public:
    MockIImsAos objMockIImsAos;
    MockIImsAosInfo objMockIImsAosInfo;

    MtcAosConnector* pConnector;

protected:
    virtual void SetUp() override
    {
        pConnector = new MtcAosConnector(objMockIImsAos, objMockIImsAosInfo);
    }

    virtual void TearDown() override { delete pConnector; }
};

// IImsAos
TEST_F(MtcAosConnectorTest, GetFeaturesInvokesImsAosApi)
{
    IMS_UINT32 nFeatures = ImsAosFeature::MMTEL | ImsAosFeature::VIDEO;
    ON_CALL(objMockIImsAos, GetFeatures).WillByDefault(Return(nFeatures));

    EXPECT_EQ(pConnector->GetFeatures(), nFeatures);
}

TEST_F(MtcAosConnectorTest, GetSuspendedReasonInvokesImsAosApi)
{
    ON_CALL(objMockIImsAos, GetSuspendedReason)
            .WillByDefault(Return(ImsAosReason::SUSPEND_NO_RAT_COVERAGE));

    EXPECT_EQ(pConnector->GetSuspendedReason(), ImsAosReason::SUSPEND_NO_RAT_COVERAGE);
}

TEST_F(MtcAosConnectorTest, IsFeatureConnectedInvokesImsAosApi)
{
    ON_CALL(objMockIImsAos, IsFeatureConnected(ImsAosFeature::MMTEL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockIImsAos, IsFeatureConnected(ImsAosFeature::VIDEO))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(pConnector->IsFeatureConnected(ImsAosFeature::MMTEL), IMS_FALSE);
    EXPECT_EQ(pConnector->IsFeatureConnected(ImsAosFeature::VIDEO), IMS_TRUE);
}

TEST_F(MtcAosConnectorTest, IsImsConnectedInvokesImsAosApi)
{
    ON_CALL(objMockIImsAos, IsImsConnected).WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(pConnector->IsImsConnected(), IMS_FALSE);

    ON_CALL(objMockIImsAos, IsImsConnected).WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(pConnector->IsImsConnected(), IMS_TRUE);
}

TEST_F(MtcAosConnectorTest, IsImsSuspendedInvokesImsAosApi)
{
    ON_CALL(objMockIImsAos, IsImsSuspended).WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(pConnector->IsImsSuspended(), IMS_FALSE);

    ON_CALL(objMockIImsAos, IsImsSuspended).WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(pConnector->IsImsSuspended(), IMS_TRUE);
}

TEST_F(MtcAosConnectorTest, SetReadyInvokesImsAosApi)
{
    EXPECT_CALL(objMockIImsAos, SetReady(_, _)).Times(4);

    pConnector->SetReady(IMS_TRUE, ImsAosService::MTC);
    pConnector->SetReady(IMS_TRUE, ImsAosService::EMERGENCY_MTC);
    pConnector->SetReady(IMS_FALSE, ImsAosService::MTC);
    pConnector->SetReady(IMS_FALSE, ImsAosService::EMERGENCY_MTC);
}

TEST_F(MtcAosConnectorTest, UpdateFeatureInvokesImsAosApi)
{
    IMS_UINT32 nFeatures = ImsAosFeature::MMTEL | ImsAosFeature::VIDEO;
    EXPECT_CALL(objMockIImsAos, UpdateFeature(nFeatures)).Times(1);

    pConnector->UpdateFeature(nFeatures);
}

TEST_F(MtcAosConnectorTest, ControlInvokesImsAosApi)
{
    EXPECT_CALL(objMockIImsAos, Control(ImsAosControl::REGISTER_START)).Times(1);
    EXPECT_CALL(objMockIImsAos, Control(ImsAosControl::REGISTER_REFRESH)).Times(1);
    EXPECT_CALL(objMockIImsAos, Control(ImsAosControl::REGISTER_STOP)).Times(1);
    EXPECT_CALL(objMockIImsAos, Control(ImsAosControl::REGISTER_REINITIATE)).Times(1);

    EXPECT_CALL(objMockIImsAos, Control(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF)).Times(0);
    EXPECT_CALL(objMockIImsAos, Control(ImsAosControl::PCSCF_NEXT)).Times(0);
    EXPECT_CALL(objMockIImsAos, Control(ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY)).Times(0);

    pConnector->Control(ImsAosControl::REGISTER_START);
    pConnector->Control(ImsAosControl::REGISTER_REFRESH);
    pConnector->Control(ImsAosControl::REGISTER_STOP);
    pConnector->Control(ImsAosControl::REGISTER_REINITIATE);
}

// IImsAosInfo
TEST_F(MtcAosConnectorTest, GetAssociatedUriInvokesImsAosInfoApi)
{
    AString strUri = "sip:testuri@ims.google.com";
    ON_CALL(objMockIImsAosInfo, GetAssociatedUri()).WillByDefault(Return(strUri));

    EXPECT_STREQ(pConnector->GetAssociatedUri().GetStr(), strUri.GetStr());
}

TEST_F(MtcAosConnectorTest, GetConnectionTypeInvokesImsAosInfoApi)
{
    for (IMS_SINT32 i = NetworkPolicy::APN_IMS; i < NetworkPolicy::APN_WIFI_MAX; ++i)
    {
        ON_CALL(objMockIImsAosInfo, GetConnectionType()).WillByDefault(Return(i));
        EXPECT_EQ(pConnector->GetConnectionType(), i);
    }
}

TEST_F(MtcAosConnectorTest, GetImsStateInvokesImsAosInfoApi)
{
    IMS_ASSERT(IImsAosInfo::IMS_STATE_AVAILABLE < IImsAosInfo::IMS_STATE_UNSUBSCRIBED);
    for (IMS_SINT32 i = IImsAosInfo::IMS_STATE_AVAILABLE; i <= IImsAosInfo::IMS_STATE_UNSUBSCRIBED;
            ++i)
    {
        ON_CALL(objMockIImsAosInfo, GetImsState()).WillByDefault(Return(i));
        EXPECT_EQ(pConnector->GetImsState(), i);
    }
}

TEST_F(MtcAosConnectorTest, GetIpcanTypeInvokesImsAosInfoApi)
{
    IMS_ASSERT(IIpcan::CATEGORY_MOBILE < IIpcan::CATEGORY_ANY);
    for (IMS_SINT32 i = IIpcan::CATEGORY_MOBILE; i <= IIpcan::CATEGORY_ANY; ++i)
    {
        ON_CALL(objMockIImsAosInfo, GetIpcanType()).WillByDefault(Return(i));
        EXPECT_EQ(pConnector->GetIpcanType(), i);
    }
}

TEST_F(MtcAosConnectorTest, GetLastPathHeaderValueInvokesImsAosInfoApi)
{
    AString strLastPathHeader = "sip:192.168.0.1;lr";
    ON_CALL(objMockIImsAosInfo, GetLastPathHeaderValue()).WillByDefault(Return(strLastPathHeader));

    EXPECT_STREQ(pConnector->GetLastPathHeaderValue().GetStr(), strLastPathHeader.GetStr());
}

TEST_F(MtcAosConnectorTest, GetLocalAddressInvokesImsAosInfoApi)
{
    AString strLocalAddress = "192.168.0.123";
    ON_CALL(objMockIImsAosInfo, GetLocalAddress()).WillByDefault(Return(strLocalAddress));

    EXPECT_STREQ(pConnector->GetLocalAddress().GetStr(), strLocalAddress.GetStr());
}

TEST_F(MtcAosConnectorTest, GetLocalPortInvokesImsAosInfoApi)
{
    IMS_UINT32 nPort = 12345;
    ON_CALL(objMockIImsAosInfo, GetLocalPort()).WillByDefault(Return(nPort));

    EXPECT_EQ(pConnector->GetLocalPort(), nPort);
}

TEST_F(MtcAosConnectorTest, GetRegisteredNetworkTypeInvokesImsAosInfoApi)
{
    ON_CALL(objMockIImsAosInfo, GetRegisteredNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    EXPECT_EQ(pConnector->GetRegisteredNetworkType(), NW_REPORT_RADIO_LTE);

    ON_CALL(objMockIImsAosInfo, GetRegisteredNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_WLAN));
    EXPECT_EQ(pConnector->GetRegisteredNetworkType(), NW_REPORT_RADIO_WLAN);
}

TEST_F(MtcAosConnectorTest, GetPathHeaderValueInvokesImsAosInfoApi)
{
    AString strPathHeader = "sip:192.168.0.1;lr";
    ON_CALL(objMockIImsAosInfo, GetPathHeaderValue()).WillByDefault(Return(strPathHeader));

    EXPECT_STREQ(pConnector->GetPathHeaderValue().GetStr(), strPathHeader.GetStr());
}

TEST_F(MtcAosConnectorTest, GetPcscfAddressInvokesImsAosInfoApi)
{
    AString strPcscfAddress = "192.168.0.1";
    ON_CALL(objMockIImsAosInfo, GetPcscfAddress()).WillByDefault(Return(strPcscfAddress));

    EXPECT_STREQ(pConnector->GetPcscfAddress().GetStr(), strPcscfAddress.GetStr());
}

TEST_F(MtcAosConnectorTest, GetPcscfPortInvokesImsAosInfoApi)
{
    IMS_UINT32 nPort = 12345;
    ON_CALL(objMockIImsAosInfo, GetPcscfPort()).WillByDefault(Return(nPort));

    EXPECT_EQ(pConnector->GetPcscfPort(), nPort);
}

TEST_F(MtcAosConnectorTest, GetRegistrationModeInvokesImsAosInfoApi)
{
    IMS_ASSERT(IImsAosInfo::REG_MODE_UNKNOWN < IImsAosInfo::REG_MODE_NOUICC);
    for (IMS_SINT32 i = IImsAosInfo::REG_MODE_UNKNOWN; i <= IImsAosInfo::REG_MODE_NOUICC; ++i)
    {
        ON_CALL(objMockIImsAosInfo, GetRegistrationMode()).WillByDefault(Return(i));
        EXPECT_EQ(pConnector->GetRegistrationMode(), i);
    }
}

TEST_F(MtcAosConnectorTest, GetSupportedHeaderValueInvokesImsAosInfoApi)
{
    AString strSupportedHeader = "featureTag1, fetureTag2";
    ON_CALL(objMockIImsAosInfo, GetSupportedHeaderValue())
            .WillByDefault(Return(strSupportedHeader));

    EXPECT_STREQ(pConnector->GetSupportedHeaderValue().GetStr(), strSupportedHeader.GetStr());
}

TEST_F(MtcAosConnectorTest, GetServiceRouteHeaderValueInvokesImsAosInfoApi)
{
    AString strSrHeader = "sip:192.168.0.1;lr";
    ON_CALL(objMockIImsAosInfo, GetServiceRouteHeaderValue()).WillByDefault(Return(strSrHeader));

    EXPECT_STREQ(pConnector->GetServiceRouteHeaderValue().GetStr(), strSrHeader.GetStr());
}

TEST_F(MtcAosConnectorTest, IsCrossSimConnectedInvokesImsAosInfoApi)
{
    ON_CALL(objMockIImsAosInfo, IsCrossSimConnected()).WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(pConnector->IsCrossSimConnected(), IMS_TRUE);
}

TEST_F(MtcAosConnectorTest, NotifyEmergencyCallStateInvokesImsAosInfoApi)
{
    EXPECT_CALL(objMockIImsAosInfo, NotifyEmergencyCallState(IMS_TRUE)).Times(1);
    EXPECT_CALL(objMockIImsAosInfo, NotifyEmergencyCallState(IMS_FALSE)).Times(1);

    pConnector->NotifyEmergencyCallState(IMS_TRUE);
    pConnector->NotifyEmergencyCallState(IMS_FALSE);
}

TEST_F(MtcAosConnectorTest, NotifyEpsfbCallStateInvokesImsAosInfoApi)
{
    IMS_UINT32 nAnyState = 0;
    EXPECT_CALL(objMockIImsAosInfo, NotifyEpsfbCallState(nAnyState)).Times(1);

    pConnector->NotifyEpsfbCallState(nAnyState);
}

TEST_F(MtcAosConnectorTest, RegisterWithNextPcscfInvokesImsAosApi)
{
    IMS_UINT32 nAnyTime = 0;
    EXPECT_CALL(objMockIImsAos, RegisterWithNextPcscf(nAnyTime)).Times(1);

    pConnector->RegisterWithNextPcscf(nAnyTime);
}

TEST_F(MtcAosConnectorTest, ReinitiateRegistrationInvokesImsAosApi)
{
    IMS_UINT32 nAnyTime = 0;
    EXPECT_CALL(objMockIImsAos, ReinitiateRegistration(nAnyTime)).Times(1);

    pConnector->ReinitiateRegistration(nAnyTime);
}

}  // namespace android
