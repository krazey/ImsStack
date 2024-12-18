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
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallManager.h"
#include "call/radio/MockIMtcRadioChecker.h"
#include "call/radio/MtcRadioChecker.h"
#include "helper/IMtcAosStateListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSessionInterfaceHolder.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::WithArgs;

LOCAL const IMS_SINT32 SLOT_ID = 0;
LOCAL const CallKey CALL_KEY1 = 123;
LOCAL const CallKey CALL_KEY2 = 456;

namespace android
{

class MtcRadioCheckerTest : public ::testing::Test
{
public:
    MtcRadioCheckerTest() :
            m_objContext(),
            m_objNormalService(),
            m_objEmergencyService(),
            m_objIMtcRadioCheckerListener(),
            m_objSipInterfaceFactory(),
            m_objSessionInterfaceHolder(),
            m_objPhoneInfoService(),
            m_objImsRadioService(),
            m_piImsRadioConnectionListener(IMS_NULL),
            m_pMtcRadioChecker(IMS_NULL)
    {
    }

public:
    MockIMtcContext m_objContext;
    MockIMtcService m_objNormalService;
    MockIMtcService m_objEmergencyService;
    MockIMtcRadioCheckerListener m_objIMtcRadioCheckerListener;
    MockIMtcSipInterfaceFactory m_objSipInterfaceFactory;
    MockIMtcCallManager m_objCallManager;
    MockIMtcCall m_objCall;
    MockSessionInterfaceHolder m_objSessionInterfaceHolder;
    TestPhoneInfoService m_objPhoneInfoService;
    TestImsRadioService m_objImsRadioService;
    IImsRadioConnectionListener* m_piImsRadioConnectionListener;
    MtcRadioChecker* m_pMtcRadioChecker;

    void CaptureIImsRadioConnectionListener(
            IN IImsRadioConnectionListener* piImsRadioConnectionListener)
    {
        m_piImsRadioConnectionListener = piImsRadioConnectionListener;
    }

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, &m_objImsRadioService);

        ON_CALL(m_objContext, GetSlotId).WillByDefault(Return(SLOT_ID));

        ON_CALL(m_objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
                .WillByDefault(Return(NW_REPORT_RADIO_LTE));

        ON_CALL(m_objContext, GetServiceByType(ServiceType::NORMAL))
                .WillByDefault(Return(&m_objNormalService));
        ON_CALL(m_objNormalService, IsEmergency).WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objContext, GetServiceByType(ServiceType::EMERGENCY))
                .WillByDefault(Return(&m_objEmergencyService));
        ON_CALL(m_objEmergencyService, IsEmergency).WillByDefault(Return(IMS_TRUE));

        ON_CALL(m_objContext, GetSipInterfaceFactory)
                .WillByDefault(ReturnRef(m_objSipInterfaceFactory));
        ON_CALL(m_objSipInterfaceFactory, GetISessionHolder)
                .WillByDefault(ReturnRef(m_objSessionInterfaceHolder));

        ON_CALL(m_objContext, GetCallManager).WillByDefault(ReturnRef(m_objCallManager));
        ON_CALL(m_objCallManager, GetCallByCallKey(_)).WillByDefault(Return(&m_objCall));
        ON_CALL(m_objCall, GetKey).WillByDefault(Return(IMtcCall::CALL_KEY_INVALID));

        m_pMtcRadioChecker = new MtcRadioChecker(m_objContext);
        m_pMtcRadioChecker->Init();
    }

    virtual void TearDown() override
    {
        delete m_pMtcRadioChecker;

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_RADIO, IMS_NULL);
    }
};

TEST_F(MtcRadioCheckerTest, InitAddsListenersAndDeInitRemovesListeners)
{
    delete m_pMtcRadioChecker;
    m_pMtcRadioChecker = IMS_NULL;

    MtcRadioChecker objTempObject(m_objContext);

    EXPECT_CALL(m_objSessionInterfaceHolder, AddListener(_));
    EXPECT_CALL(m_objNormalService, AddAosStateListener(_));
    EXPECT_CALL(m_objEmergencyService, AddAosStateListener(_));

    objTempObject.Init();

    EXPECT_CALL(m_objSessionInterfaceHolder, RemoveListener(&objTempObject));
    EXPECT_CALL(m_objNormalService, RemoveAosStateListener(&objTempObject));
    EXPECT_CALL(m_objEmergencyService, RemoveAosStateListener(&objTempObject));
}

TEST_F(MtcRadioCheckerTest, CheckTrafficPreparedReturnsUnblocked)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(IImsRadio::TRAFFIC_TYPE_VOICE,
            IImsRadio::DIRECTION_MO, IMS_TRUE, IMtcCall::CALL_KEY_INVALID);

    EXPECT_EQ(CheckResult::UNBLOCKED,
            m_pMtcRadioChecker->Check(
                    CallType::VOIP, IMS_FALSE, PeerType::MO, IMS_FALSE, IMS_FALSE, CALL_KEY1));
}

TEST_F(MtcRadioCheckerTest, CheckMtReturnsUnblocked)
{
    EXPECT_EQ(CheckResult::UNBLOCKED,
            m_pMtcRadioChecker->Check(
                    CallType::VOIP, IMS_FALSE, PeerType::MT, IMS_FALSE, IMS_FALSE, CALL_KEY1));
}

TEST_F(MtcRadioCheckerTest, CheckNotTrafficAllowedReturnsBlocked)
{
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    EXPECT_EQ(CheckResult::BLOCKED,
            m_pMtcRadioChecker->Check(
                    CallType::VOIP, IMS_TRUE, PeerType::MO, IMS_FALSE, IMS_FALSE, CALL_KEY1));
}

TEST_F(MtcRadioCheckerTest, CheckTrafficAllowedReturnsPending)
{
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_EQ(CheckResult::PENDING,
            m_pMtcRadioChecker->Check(
                    CallType::VOIP, IMS_FALSE, PeerType::MO, IMS_FALSE, IMS_FALSE, CALL_KEY1));
}

TEST_F(MtcRadioCheckerTest, CheckTrafficAllowedReturnsUnblockedIfUssi)
{
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_)).Times(0);

    EXPECT_EQ(CheckResult::UNBLOCKED,
            m_pMtcRadioChecker->Check(
                    CallType::VOIP, IMS_FALSE, PeerType::MO, IMS_FALSE, IMS_TRUE, CALL_KEY1));
}

TEST_F(MtcRadioCheckerTest, CheckStartTrafficChecking)
{
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN,
                    IImsRadio::DIRECTION_MO, _))
            .Times(1);

    EXPECT_EQ(CheckResult::PENDING,
            m_pMtcRadioChecker->Check(
                    CallType::VT, IMS_FALSE, PeerType::MO, IMS_FALSE, IMS_FALSE, CALL_KEY1));
}

TEST_F(MtcRadioCheckerTest, CheckStartTrafficCheckingAddCallKey)
{
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN,
                    IImsRadio::DIRECTION_MO, _))
            .Times(1);

    EXPECT_EQ(CheckResult::PENDING,
            m_pMtcRadioChecker->Check(
                    CallType::VT, IMS_FALSE, PeerType::MO, IMS_FALSE, IMS_FALSE, CALL_KEY1));

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), StopImsTraffic(_)).Times(1);

    m_pMtcRadioChecker->OnSessionInterfaceReleased(CALL_KEY1);
}

TEST_F(MtcRadioCheckerTest, CheckStartTrafficCheckingExistsMtcTrafficInfoWithCallKey)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::DIRECTION_MO, IMS_FALSE, CALL_KEY1);

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN,
                    IImsRadio::DIRECTION_MO, _))
            .Times(0);

    EXPECT_EQ(CheckResult::PENDING,
            m_pMtcRadioChecker->Check(
                    CallType::VT, IMS_FALSE, PeerType::MO, IMS_FALSE, IMS_FALSE, CALL_KEY1));
}

TEST_F(MtcRadioCheckerTest, CheckStartTrafficCheckingEmergency)
{
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY,
                    IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, IImsRadio::DIRECTION_MO, _))
            .Times(1);

    EXPECT_EQ(CheckResult::PENDING,
            m_pMtcRadioChecker->Check(
                    CallType::VT, IMS_TRUE, PeerType::MO, IMS_FALSE, IMS_FALSE, CALL_KEY1));
}

TEST_F(MtcRadioCheckerTest, OnAosStateChangedDoesNothing)
{
    m_pMtcRadioChecker->OnAosStateChanged(m_objNormalService, MtcAosState::CONNECTED, 0);
}

TEST_F(MtcRadioCheckerTest, OnIpcanChangedNoStartImsTrafficBecauseEmptyCallKey)
{
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), StartImsTraffic(_, _, _, _)).Times(0);

    m_pMtcRadioChecker->OnIpcanChanged(m_objEmergencyService, IIpcan::CATEGORY_MOBILE);
    m_pMtcRadioChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_MOBILE);
    m_pMtcRadioChecker->OnIpcanChanged(m_objEmergencyService, IIpcan::CATEGORY_WLAN);
    m_pMtcRadioChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_WLAN);

    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(IImsRadio::TRAFFIC_TYPE_VOICE,
            IImsRadio::DIRECTION_MO, IMS_TRUE, IMtcCall::CALL_KEY_INVALID);
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(IImsRadio::TRAFFIC_TYPE_VIDEO,
            IImsRadio::DIRECTION_MO, IMS_TRUE, IMtcCall::CALL_KEY_INVALID);
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(IImsRadio::TRAFFIC_TYPE_EMERGENCY,
            IImsRadio::DIRECTION_MO, IMS_TRUE, IMtcCall::CALL_KEY_INVALID);

    m_pMtcRadioChecker->OnIpcanChanged(m_objEmergencyService, IIpcan::CATEGORY_MOBILE);
    m_pMtcRadioChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_MOBILE);
    m_pMtcRadioChecker->OnIpcanChanged(m_objEmergencyService, IIpcan::CATEGORY_WLAN);
    m_pMtcRadioChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_WLAN);
}

TEST_F(MtcRadioCheckerTest, OnIpcanChangedNoStartImsTrafficBecauseNotActive)
{
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), StartImsTraffic(_, _, _, _)).Times(0);

    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MO, IMS_FALSE, CALL_KEY1);
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::DIRECTION_MO, IMS_FALSE, CALL_KEY1);
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY, IImsRadio::DIRECTION_MO, IMS_FALSE, CALL_KEY1);

    m_pMtcRadioChecker->OnIpcanChanged(m_objEmergencyService, IIpcan::CATEGORY_MOBILE);
    m_pMtcRadioChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_MOBILE);
    m_pMtcRadioChecker->OnIpcanChanged(m_objEmergencyService, IIpcan::CATEGORY_WLAN);
    m_pMtcRadioChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_WLAN);
}

TEST_F(MtcRadioCheckerTest, OnIpcanChangedStartImsTrafficNormalLte)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN,
                    IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN,
                    IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY,
                    IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, IImsRadio::DIRECTION_MT, _))
            .Times(0);

    m_pMtcRadioChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_MOBILE);
}

TEST_F(MtcRadioCheckerTest, OnIpcanChangedStartImsTrafficNormalNr)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);

    EXPECT_CALL(m_objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .Times(2)
            .WillRepeatedly(Return(NW_REPORT_RADIO_NR));

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::ACCESS_NETWORK_TYPE_NGRAN,
                    IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_NGRAN,
                    IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY, IImsRadio::ACCESS_NETWORK_TYPE_NGRAN,
                    IImsRadio::DIRECTION_MT, _))
            .Times(0);

    m_pMtcRadioChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_MOBILE);
}

TEST_F(MtcRadioCheckerTest, OnIpcanChangedStartImsTrafficNormalWifi)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::ACCESS_NETWORK_TYPE_IWLAN,
                    IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_IWLAN,
                    IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY, IImsRadio::ACCESS_NETWORK_TYPE_IWLAN,
                    IImsRadio::DIRECTION_MT, _))
            .Times(0);

    m_pMtcRadioChecker->OnIpcanChanged(m_objNormalService, IIpcan::CATEGORY_WLAN);
}

TEST_F(MtcRadioCheckerTest, OnIpcanChangedStartImsTrafficEmergency)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN,
                    IImsRadio::DIRECTION_MT, _))
            .Times(0);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN,
                    IImsRadio::DIRECTION_MT, _))
            .Times(0);
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY,
                    IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, IImsRadio::DIRECTION_MT, _))
            .Times(1);

    m_pMtcRadioChecker->OnIpcanChanged(m_objEmergencyService, IIpcan::CATEGORY_MOBILE);
}

TEST_F(MtcRadioCheckerTest, OnSessionInterfaceReleasedInvokesStopImsTraffic)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(IImsRadio::TRAFFIC_TYPE_VOICE,
            IImsRadio::DIRECTION_MO, IMS_TRUE, IMtcCall::CALL_KEY_INVALID);

    // no keys, just delete `MtcTrafficInfo`.
    m_pMtcRadioChecker->OnSessionInterfaceReleased(CALL_KEY1);

    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MO, IMS_TRUE, CALL_KEY1);

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), StopImsTraffic(_)).Times(1);

    m_pMtcRadioChecker->OnSessionInterfaceReleased(CALL_KEY1);

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), StopImsTraffic(_)).Times(0);

    m_pMtcRadioChecker->OnSessionInterfaceReleased(CALL_KEY1);
}

TEST_F(MtcRadioCheckerTest, OnSessionInterfaceReleasedDoesNotInvokeStopImsTrafficDuringSilentRedial)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(IImsRadio::TRAFFIC_TYPE_VOICE,
            IImsRadio::DIRECTION_MO, IMS_TRUE, IMtcCall::CALL_KEY_INVALID);

    // no keys, just delete `MtcTrafficInfo`.
    m_pMtcRadioChecker->OnSessionInterfaceReleased(CALL_KEY1);

    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MO, IMS_TRUE, CALL_KEY1);

    // Call is silent redialing, so not in TERMINATING state
    ON_CALL(m_objCall, GetKey).WillByDefault(Return(CALL_KEY1));
    ON_CALL(m_objCall, GetState).WillByDefault(Return(IMtcCall::State::OUTGOING));

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), StopImsTraffic(_)).Times(0);

    m_pMtcRadioChecker->OnSessionInterfaceReleased(CALL_KEY1);

    // Destructor triggers it
    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), StopImsTraffic(_));
}

TEST_F(MtcRadioCheckerTest, OnConnectionFailedPermanentlyWithOutMtcRadioCheckerListener)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);

    EXPECT_CALL(m_objIMtcRadioCheckerListener, OnConnectionFailed).Times(0);
    EXPECT_CALL(m_objIMtcRadioCheckerListener, OnConnectionSetupPrepared).Times(0);

    m_pMtcRadioChecker->OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MT,
            IImsRadio::REASON_ACCESS_DENIED, 0, 0);
}

TEST_F(MtcRadioCheckerTest, OnConnectionFailedPermanentlyWithOutMtcTrafficInfo)
{
    m_pMtcRadioChecker->SetTrafficCheckerListener(&m_objIMtcRadioCheckerListener);

    EXPECT_CALL(m_objIMtcRadioCheckerListener, OnConnectionFailed).Times(1);
    EXPECT_CALL(m_objIMtcRadioCheckerListener, OnConnectionSetupPrepared).Times(0);

    m_pMtcRadioChecker->OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MT,
            IImsRadio::REASON_ACCESS_DENIED, 0, 0);
}

TEST_F(MtcRadioCheckerTest, OnConnectionFailedPermanentlyExistsMtcTrafficInfoWithCallKey)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);
    m_pMtcRadioChecker->SetTrafficCheckerListener(&m_objIMtcRadioCheckerListener);

    EXPECT_CALL(m_objIMtcRadioCheckerListener, OnConnectionFailed).Times(1);
    EXPECT_CALL(m_objIMtcRadioCheckerListener, OnConnectionSetupPrepared).Times(0);

    m_pMtcRadioChecker->OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MT,
            IImsRadio::REASON_ACCESS_DENIED, 0, 0);
}

TEST_F(MtcRadioCheckerTest, OnConnectionFailedPermanentlyExistsMtcTrafficInfoWithOutCallKey)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(IImsRadio::TRAFFIC_TYPE_VOICE,
            IImsRadio::DIRECTION_MT, IMS_TRUE, IMtcCall::CALL_KEY_INVALID);
    m_pMtcRadioChecker->SetTrafficCheckerListener(&m_objIMtcRadioCheckerListener);

    EXPECT_CALL(m_objIMtcRadioCheckerListener, OnConnectionFailed).Times(1);
    EXPECT_CALL(m_objIMtcRadioCheckerListener, OnConnectionSetupPrepared).Times(0);

    m_pMtcRadioChecker->OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MT,
            IImsRadio::REASON_INTERNAL_ERROR, 0, 0);
}

TEST_F(MtcRadioCheckerTest, OnConnectionFailedTemporarily)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MT, IMS_TRUE, CALL_KEY1);
    m_pMtcRadioChecker->SetTrafficCheckerListener(&m_objIMtcRadioCheckerListener);

    EXPECT_CALL(m_objIMtcRadioCheckerListener, OnConnectionFailed).Times(0);
    EXPECT_CALL(m_objIMtcRadioCheckerListener, OnConnectionSetupPrepared).Times(7);

    m_pMtcRadioChecker->OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::DIRECTION_MT,
            IImsRadio::REASON_NAS_FAILURE, 0, 0);
    m_pMtcRadioChecker->OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::DIRECTION_MT,
            IImsRadio::REASON_RACH_FAILURE, 0, 0);
    m_pMtcRadioChecker->OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::DIRECTION_MT,
            IImsRadio::REASON_RLC_FAILURE, 0, 0);
    m_pMtcRadioChecker->OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::DIRECTION_MT,
            IImsRadio::REASON_RRC_TIMEOUT, 0, 0);
    m_pMtcRadioChecker->OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::DIRECTION_MT,
            IImsRadio::REASON_NO_SERVICE, 0, 0);
    m_pMtcRadioChecker->OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::DIRECTION_MT,
            IImsRadio::REASON_PDN_NOT_AVAILABLE, 0, 0);
    m_pMtcRadioChecker->OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::DIRECTION_MT,
            IImsRadio::REASON_RF_BUSY, 0, 0);
}

TEST_F(MtcRadioCheckerTest, OnConnectionSetupPrepared)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MT, IMS_FALSE, CALL_KEY1);
    m_pMtcRadioChecker->SetTrafficCheckerListener(&m_objIMtcRadioCheckerListener);

    EXPECT_CALL(m_objIMtcRadioCheckerListener, OnConnectionFailed).Times(0);
    EXPECT_CALL(m_objIMtcRadioCheckerListener, OnConnectionSetupPrepared).Times(2);

    m_pMtcRadioChecker->OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY, IImsRadio::DIRECTION_MT);

    m_pMtcRadioChecker->OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY, IImsRadio::DIRECTION_MT);
}

TEST_F(MtcRadioCheckerTest, MtcTrafficInfoImsRadioOnConnectionFailedNotifies)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(IImsRadio::TRAFFIC_TYPE_VIDEO,
            IImsRadio::DIRECTION_MO, IMS_FALSE, IMtcCall::CALL_KEY_INVALID);
    m_pMtcRadioChecker->SetTrafficCheckerListener(&m_objIMtcRadioCheckerListener);

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN,
                    IImsRadio::DIRECTION_MO, _))
            .Times(1)
            .WillOnce(WithArgs<3>(
                    Invoke(this, &MtcRadioCheckerTest::CaptureIImsRadioConnectionListener)));

    EXPECT_EQ(CheckResult::PENDING,
            m_pMtcRadioChecker->Check(
                    CallType::VT, IMS_FALSE, PeerType::MO, IMS_FALSE, IMS_FALSE, CALL_KEY1));

    EXPECT_CALL(m_objIMtcRadioCheckerListener, OnConnectionFailed).Times(3);

    m_piImsRadioConnectionListener->ImsRadio_OnConnectionFailed(
            IImsRadio::REASON_ACCESS_DENIED, 0, 0);
    m_piImsRadioConnectionListener->ImsRadio_OnConnectionFailed(
            IImsRadio::REASON_INTERNAL_ERROR, 0, 0);
    m_piImsRadioConnectionListener->ImsRadio_OnConnectionFailed(IImsRadio::REASON_RRC_REJECT, 0, 0);
}

TEST_F(MtcRadioCheckerTest, MtcTrafficInfoImsRadioOnConnectionSetupPreparedNotifies)
{
    m_pMtcRadioChecker->CreateCallTrafficInfoWithGivenValue(IImsRadio::TRAFFIC_TYPE_VIDEO,
            IImsRadio::DIRECTION_MO, IMS_FALSE, IMtcCall::CALL_KEY_INVALID);
    m_pMtcRadioChecker->SetTrafficCheckerListener(&m_objIMtcRadioCheckerListener);

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VIDEO, IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN,
                    IImsRadio::DIRECTION_MO, _))
            .Times(1)
            .WillOnce(WithArgs<3>(
                    Invoke(this, &MtcRadioCheckerTest::CaptureIImsRadioConnectionListener)));

    EXPECT_EQ(CheckResult::PENDING,
            m_pMtcRadioChecker->Check(
                    CallType::VT, IMS_FALSE, PeerType::MO, IMS_FALSE, IMS_FALSE, CALL_KEY1));

    EXPECT_CALL(m_objIMtcRadioCheckerListener, OnConnectionSetupPrepared).Times(1);

    m_piImsRadioConnectionListener->ImsRadio_OnConnectionSetupPrepared();
}

}  // namespace android
