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

#include "IImsRadio.h"
#include "IIpcan.h"
#include "INetworkWatcher.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "MockIImsRadio.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "MockINetworkWatcher.h"
#include "PlatformContext.h"
#include "TestImsRadioService.h"
#include "TestPhoneInfoService.h"
#include "call/IMtcCall.h"
#include "call/traffic/MockIMtcCallTrafficChecker.h"
#include "call/traffic/MockIMtcRadioConnectionFailureListener.h"
#include "call/traffic/MtcCallTrafficChecker.h"
#include "helper/IMtcAosStateListener.h"
#include "helper/MockICallStateProxy.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL const IMS_SINT32 SLOT_ID = 0;
LOCAL const CallKey CALL_KEY1 = 123;
LOCAL const CallKey CALL_KEY2 = 456;

namespace android
{

class MtcCallTrafficCheckerTest : public ::testing::Test
{
public:
    MtcCallTrafficCheckerTest() :
            m_objContext(),
            m_objConnectionFailureListener(),
            m_objCallStateProxy(),
            m_objPhoneInfoService(),
            m_objImsRadioService()
    {
    }

public:
    MockIMtcContext m_objContext;
    MockIMtcService m_objNormalService;
    MockIMtcService m_objEmergencyService;
    MockIMtcRadioConnectionFailureListener m_objConnectionFailureListener;
    MockIMtcCallTrafficCheckerListener m_objIMtcCallTrafficCheckerListener;
    MockICallStateProxy m_objCallStateProxy;
    TestPhoneInfoService m_objPhoneInfoService;
    TestImsRadioService m_objImsRadioService;
    MtcCallTrafficChecker* m_pMtcCallTrafficChecker;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, &m_objImsRadioService);

        ON_CALL(m_objContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objContext, GetCallStateProxy).WillByDefault(ReturnRef(m_objCallStateProxy));

        ON_CALL(m_objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
                .WillByDefault(Return(NW_REPORT_RADIO_LTE));

        ON_CALL(m_objContext, GetServiceByType(ServiceType::NORMAL))
                .WillByDefault(Return(&m_objNormalService));
        ON_CALL(m_objNormalService, IsEmergency).WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objContext, GetServiceByType(ServiceType::EMERGENCY))
                .WillByDefault(Return(&m_objEmergencyService));
        ON_CALL(m_objEmergencyService, IsEmergency).WillByDefault(Return(IMS_TRUE));

        m_pMtcCallTrafficChecker =
                new MtcCallTrafficChecker(m_objContext, m_objConnectionFailureListener);
        m_pMtcCallTrafficChecker->Init();
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_RADIO, IMS_NULL);

        delete m_pMtcCallTrafficChecker;
    }

    IMS_BOOL IsCallKeyExist(IN ImsList<CallKey>& objCallKeys, IN CallKey nCallKeyIn) const
    {
        for (IMS_UINT32 nIndex = 0; nIndex < objCallKeys.GetSize(); nIndex++)
        {
            const CallKey nCallKey = objCallKeys.GetAt(nIndex);

            if (nCallKey == nCallKeyIn)
            {
                return IMS_TRUE;
            }
        }

        return IMS_FALSE;
    }

    void CreateCallsAndTrafficActivateForAllTrafficTypes()
    {
        m_pMtcCallTrafficChecker->SetTrafficStatus(IImsRadio::TRAFFIC_TYPE_VOICE, IMS_TRUE);
        m_pMtcCallTrafficChecker->SetTrafficStatus(IImsRadio::TRAFFIC_TYPE_VIDEO, IMS_TRUE);
        m_pMtcCallTrafficChecker->SetTrafficStatus(IImsRadio::TRAFFIC_TYPE_EMERGENCY, IMS_TRUE);

        m_pMtcCallTrafficChecker->GetCallKeys(IImsRadio::TRAFFIC_TYPE_VOICE).Append(CALL_KEY1);
        m_pMtcCallTrafficChecker->GetCallKeys(IImsRadio::TRAFFIC_TYPE_VIDEO).Append(CALL_KEY1);
        m_pMtcCallTrafficChecker->GetCallKeys(IImsRadio::TRAFFIC_TYPE_EMERGENCY).Append(CALL_KEY1);
    }
};

TEST_F(MtcCallTrafficCheckerTest, IsTrafficPrepared)
{
    EXPECT_FALSE(m_pMtcCallTrafficChecker->IsTrafficPrepared(CallType::RTT, IMS_FALSE));
    EXPECT_FALSE(m_pMtcCallTrafficChecker->IsTrafficPrepared(CallType::VIDEO_RTT, IMS_FALSE));
    EXPECT_FALSE(m_pMtcCallTrafficChecker->IsTrafficPrepared(CallType::VOIP, IMS_TRUE));

    m_pMtcCallTrafficChecker->SetTrafficStatus(IImsRadio::TRAFFIC_TYPE_VOICE, IMS_TRUE);
    m_pMtcCallTrafficChecker->SetTrafficStatus(IImsRadio::TRAFFIC_TYPE_VIDEO, IMS_TRUE);
    m_pMtcCallTrafficChecker->SetTrafficStatus(IImsRadio::TRAFFIC_TYPE_EMERGENCY, IMS_TRUE);

    EXPECT_TRUE(m_pMtcCallTrafficChecker->IsTrafficPrepared(CallType::RTT, IMS_FALSE));
    EXPECT_TRUE(m_pMtcCallTrafficChecker->IsTrafficPrepared(CallType::VIDEO_RTT, IMS_FALSE));
    EXPECT_TRUE(m_pMtcCallTrafficChecker->IsTrafficPrepared(CallType::VOIP, IMS_TRUE));
}

TEST_F(MtcCallTrafficCheckerTest, IsTrafficAllowed)
{
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    CallType eCallType = CallType::VOIP;

    EXPECT_TRUE(m_pMtcCallTrafficChecker->IsTrafficAllowed(eCallType, IMS_TRUE));
    EXPECT_FALSE(m_pMtcCallTrafficChecker->IsTrafficAllowed(eCallType, IMS_FALSE));
}

TEST_F(MtcCallTrafficCheckerTest, StartTrafficCheckingVoice)
{
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, _, _));

    m_pMtcCallTrafficChecker->StartTrafficChecking(CallType::VOIP, IMS_FALSE, IMS_FALSE);

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::ACCESS_NETWORK_TYPE_IWLAN, _, _));

    m_pMtcCallTrafficChecker->StartTrafficChecking(CallType::RTT, IMS_FALSE, IMS_TRUE);
}

TEST_F(MtcCallTrafficCheckerTest, StartTrafficCheckingVideo)
{
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, _, _));

    m_pMtcCallTrafficChecker->StartTrafficChecking(CallType::VT, IMS_FALSE, IMS_FALSE);

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_IWLAN, _, _));

    m_pMtcCallTrafficChecker->StartTrafficChecking(CallType::VIDEO_RTT, IMS_FALSE, IMS_TRUE);
}

TEST_F(MtcCallTrafficCheckerTest, StartTrafficCheckingEmergency)
{
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY,
                    IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, _, _));

    m_pMtcCallTrafficChecker->StartTrafficChecking(CallType::VT, IMS_TRUE, IMS_FALSE);

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_EMERGENCY, IImsRadio::ACCESS_NETWORK_TYPE_IWLAN, _, _));

    m_pMtcCallTrafficChecker->StartTrafficChecking(CallType::VIDEO_RTT, IMS_TRUE, IMS_TRUE);
}

TEST_F(MtcCallTrafficCheckerTest, StopTrafficChecking)
{
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), StopImsTraffic(_));

    m_pMtcCallTrafficChecker->StopTrafficChecking(IImsRadio::TRAFFIC_TYPE_EMERGENCY);
}

TEST_F(MtcCallTrafficCheckerTest, OnAosStateChangedDoesNothing)
{
    m_pMtcCallTrafficChecker->OnAosStateChanged(m_objNormalService, MtcAosState::CONNECTED, 0);
}

TEST_F(MtcCallTrafficCheckerTest, OnIpcanChangedNoStartImsTraffic)
{
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), StartImsTraffic(_, _, _, _)).Times(0);

    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objEmergencyService, IIpcan::CATEGORY_MOBILE);
    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_MOBILE);
    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objEmergencyService, IIpcan::CATEGORY_WLAN);
    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_WLAN);

    m_pMtcCallTrafficChecker->SetTrafficStatus(IImsRadio::TRAFFIC_TYPE_VOICE, IMS_TRUE);
    m_pMtcCallTrafficChecker->SetTrafficStatus(IImsRadio::TRAFFIC_TYPE_VIDEO, IMS_TRUE);
    m_pMtcCallTrafficChecker->SetTrafficStatus(IImsRadio::TRAFFIC_TYPE_EMERGENCY, IMS_TRUE);

    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objEmergencyService, IIpcan::CATEGORY_MOBILE);
    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_MOBILE);
    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objEmergencyService, IIpcan::CATEGORY_WLAN);
    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_WLAN);

    m_pMtcCallTrafficChecker->SetTrafficStatus(IImsRadio::TRAFFIC_TYPE_VOICE, IMS_FALSE);
    m_pMtcCallTrafficChecker->SetTrafficStatus(IImsRadio::TRAFFIC_TYPE_VIDEO, IMS_FALSE);
    m_pMtcCallTrafficChecker->SetTrafficStatus(IImsRadio::TRAFFIC_TYPE_EMERGENCY, IMS_FALSE);

    m_pMtcCallTrafficChecker->GetCallKeys(IImsRadio::TRAFFIC_TYPE_VOICE).Append(CALL_KEY1);
    m_pMtcCallTrafficChecker->GetCallKeys(IImsRadio::TRAFFIC_TYPE_VIDEO).Append(CALL_KEY1);
    m_pMtcCallTrafficChecker->GetCallKeys(IImsRadio::TRAFFIC_TYPE_EMERGENCY).Append(CALL_KEY1);

    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objEmergencyService, IIpcan::CATEGORY_MOBILE);
    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_MOBILE);
    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objEmergencyService, IIpcan::CATEGORY_WLAN);
    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_WLAN);
}

TEST_F(MtcCallTrafficCheckerTest, OnIpcanChangedStartImsTrafficNormalLte)
{
    CreateCallsAndTrafficActivateForAllTrafficTypes();

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, _, _))
            .Times(1);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, _, _))
            .Times(1);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_EMERGENCY, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, _, _))
            .Times(0);

    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_MOBILE);
}

TEST_F(MtcCallTrafficCheckerTest, OnIpcanChangedStartImsTrafficNormalNr)
{
    CreateCallsAndTrafficActivateForAllTrafficTypes();

    EXPECT_CALL(m_objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .Times(2)
            .WillRepeatedly(Return(NW_REPORT_RADIO_NR));

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::ACCESS_NETWORK_TYPE_NGRAN, _, _))
            .Times(1);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_NGRAN, _, _))
            .Times(1);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_EMERGENCY, IImsRadio::ACCESS_NETWORK_TYPE_NGRAN, _, _))
            .Times(0);

    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_MOBILE);
}

TEST_F(MtcCallTrafficCheckerTest, OnIpcanChangedStartImsTrafficNormalWifi)
{
    CreateCallsAndTrafficActivateForAllTrafficTypes();

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::ACCESS_NETWORK_TYPE_IWLAN, _, _))
            .Times(1);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_IWLAN, _, _))
            .Times(1);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_EMERGENCY, IImsRadio::ACCESS_NETWORK_TYPE_IWLAN, _, _))
            .Times(0);

    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_WLAN);
}

TEST_F(MtcCallTrafficCheckerTest, OnIpcanChangedStartImsTrafficEmergency)
{
    CreateCallsAndTrafficActivateForAllTrafficTypes();

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, _, _))
            .Times(0);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, _, _))
            .Times(0);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(
                    IImsRadio::TRAFFIC_TYPE_EMERGENCY, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, _, _))
            .Times(1);

    m_pMtcCallTrafficChecker->OnIpcanChanged(m_objEmergencyService, IIpcan::CATEGORY_MOBILE);
}

TEST_F(MtcCallTrafficCheckerTest, OnCallStateChanged)
{
    m_pMtcCallTrafficChecker->OnCallStateChanged(
            CALL_KEY1, IMtcCall::State::OUTGOING, CallType::VOIP, IMS_FALSE, 0);

    EXPECT_TRUE(IsCallKeyExist(
            m_pMtcCallTrafficChecker->GetCallKeys(IImsRadio::TRAFFIC_TYPE_VOICE), CALL_KEY1));

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), StopImsTraffic(_)).Times(1);

    m_pMtcCallTrafficChecker->OnCallStateChanged(
            CALL_KEY1, IMtcCall::State::TERMINATING, CallType::VT, IMS_FALSE, 0);

    EXPECT_FALSE(IsCallKeyExist(
            m_pMtcCallTrafficChecker->GetCallKeys(IImsRadio::TRAFFIC_TYPE_VOICE), CALL_KEY1));

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), StopImsTraffic(_)).Times(0);

    m_pMtcCallTrafficChecker->OnCallStateChanged(
            CALL_KEY2, IMtcCall::State::TERMINATING, CallType::VOIP, IMS_FALSE, 0);
}

TEST_F(MtcCallTrafficCheckerTest, OnConnectionFailedPermanently)
{
    CreateCallsAndTrafficActivateForAllTrafficTypes();
    m_pMtcCallTrafficChecker->SetTrafficCheckerListener(&m_objIMtcCallTrafficCheckerListener);

    EXPECT_CALL(m_objIMtcCallTrafficCheckerListener, OnConnectionFailed).Times(1);
    EXPECT_CALL(m_objIMtcCallTrafficCheckerListener, OnConnectionSetupPrepared).Times(0);
    EXPECT_CALL(m_objConnectionFailureListener, OnConnectionFailed(_)).Times(2);

    m_pMtcCallTrafficChecker->OnConnectionFailed(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::REASON_ACCESS_DENIED, 0, 0);

    EXPECT_CALL(m_objIMtcCallTrafficCheckerListener, OnConnectionFailed).Times(0);

    m_pMtcCallTrafficChecker->OnConnectionFailed(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::REASON_INTERNAL_ERROR, 0, 0);

    m_pMtcCallTrafficChecker->GetCallKeys(IImsRadio::TRAFFIC_TYPE_VOICE).Clear();

    EXPECT_CALL(m_objConnectionFailureListener, OnConnectionFailed(_)).Times(0);

    m_pMtcCallTrafficChecker->OnConnectionFailed(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::REASON_ACCESS_DENIED, 0, 0);
}

TEST_F(MtcCallTrafficCheckerTest, OnConnectionFailedTemporarily)
{
    CreateCallsAndTrafficActivateForAllTrafficTypes();
    m_pMtcCallTrafficChecker->SetTrafficCheckerListener(&m_objIMtcCallTrafficCheckerListener);

    EXPECT_CALL(m_objIMtcCallTrafficCheckerListener, OnConnectionFailed).Times(0);
    EXPECT_CALL(m_objIMtcCallTrafficCheckerListener, OnConnectionSetupPrepared).Times(1);
    EXPECT_CALL(m_objConnectionFailureListener, OnConnectionFailed(_)).Times(0);

    m_pMtcCallTrafficChecker->OnConnectionFailed(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::REASON_NAS_FAILURE, 0, 0);

    EXPECT_CALL(m_objIMtcCallTrafficCheckerListener, OnConnectionSetupPrepared).Times(0);

    m_pMtcCallTrafficChecker->OnConnectionFailed(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::REASON_RACH_FAILURE, 0, 0);
    m_pMtcCallTrafficChecker->OnConnectionFailed(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::REASON_RLC_FAILURE, 0, 0);
    m_pMtcCallTrafficChecker->OnConnectionFailed(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::REASON_RRC_REJECT, 0, 0);
    m_pMtcCallTrafficChecker->OnConnectionFailed(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::REASON_RRC_TIMEOUT, 0, 0);
    m_pMtcCallTrafficChecker->OnConnectionFailed(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::REASON_NO_SERVICE, 0, 0);
    m_pMtcCallTrafficChecker->OnConnectionFailed(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::REASON_PDN_NOT_AVAILABLE, 0, 0);
    m_pMtcCallTrafficChecker->OnConnectionFailed(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::REASON_RF_BUSY, 0, 0);
}

TEST_F(MtcCallTrafficCheckerTest, OnConnectionSetupPrepared)
{
    m_pMtcCallTrafficChecker->SetTrafficCheckerListener(&m_objIMtcCallTrafficCheckerListener);

    EXPECT_CALL(m_objIMtcCallTrafficCheckerListener, OnConnectionFailed).Times(0);
    EXPECT_CALL(m_objIMtcCallTrafficCheckerListener, OnConnectionSetupPrepared).Times(1);

    m_pMtcCallTrafficChecker->OnConnectionSetupPrepared(IImsRadio::TRAFFIC_TYPE_EMERGENCY);

    EXPECT_CALL(m_objIMtcCallTrafficCheckerListener, OnConnectionSetupPrepared).Times(0);

    m_pMtcCallTrafficChecker->OnConnectionSetupPrepared(IImsRadio::TRAFFIC_TYPE_EMERGENCY);
}

}  // namespace android
