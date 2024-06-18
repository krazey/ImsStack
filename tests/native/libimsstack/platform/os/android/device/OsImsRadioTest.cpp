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
 * WITHout WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ImsTypeDef.h"
#include "MockIImsRadioListener.h"
#include "MockISystem.h"
#include "MockIThread.h"
#include "OsParcel.h"
#include "PlatformContext.h"
#include "TestImsRadioService.h"
#include "TestThreadService.h"
#include "device/OsImsRadio.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::Unused;

namespace android
{

class OsImsRadioTest : public ::testing::Test
{
public:
    int EVENT_CONNECTION_FAILED = 1;
    int EVENT_CONNECTION_SETUP_PREPARED = 2;
    int EVENT_SSAC_STATE_CHANGED = 3;
    int EVENT_SIMULTANEOUS_CALLING_SUPPORT_CHANGED = 4;

    MockIImsRadioConnectionListener objImsRadioConnectionListener;
    MockIImsRadioConnectionListener objImsRadioConnectionListener2;
    MockISystem m_objMockSystem;
    MockIThread m_objMockThread;

    ISystem* m_piDefaultSystem;
    ISystemListener* m_piSystemListener;

    PlatformService* m_pThreadPlatformService;
    PlatformService* m_pRadioPlatformService;

    OsImsRadio* m_pOsImsRadio;
    ImsRadio* m_pImsRadio;

    TestImsRadioService m_objRadioService;
    TestThreadService m_objThreadService;

protected:
    virtual void SetUp() override
    {
        m_objThreadService.SetThread(&m_objMockThread);
        m_pThreadPlatformService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);
        m_pRadioPlatformService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, &m_objRadioService);

        EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
        m_piDefaultSystem = PlatformContext::GetInstance()->SetSystem(&m_objMockSystem);

        m_pOsImsRadio = new OsImsRadio(IMS_SLOT_0);
        ASSERT_TRUE(m_pOsImsRadio != nullptr);

        m_piSystemListener = static_cast<ISystemListener*>(m_pOsImsRadio);
        m_pImsRadio = static_cast<ImsRadio*>(m_pOsImsRadio);
    }

    virtual void TearDown() override
    {
        EXPECT_CALL(m_objMockSystem, RemoveListener(_, _, _)).Times(1);
        if (m_pOsImsRadio != IMS_NULL)
        {
            delete m_pOsImsRadio;
            m_pOsImsRadio = IMS_NULL;
        }
        PlatformContext::GetInstance()->SetSystem(m_piDefaultSystem);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, m_pRadioPlatformService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, m_pThreadPlatformService);
    }
};

TEST_F(OsImsRadioTest, StartAndStopImsTraffic)
{
    EXPECT_CALL(m_objRadioService.GetMockImsTraffic(), IsAllowed(_, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_EQ(m_pOsImsRadio->IsImsTrafficAllowed(IImsRadio::TRAFFIC_TYPE_REGISTRATION), IMS_TRUE);

    EXPECT_CALL(m_objMockSystem, StartImsTraffic(_, _, _, _, _)).Times(0);
    m_pOsImsRadio->StartImsTraffic(IImsRadio::TRAFFIC_TYPE_REGISTRATION,
            IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, IImsRadio::DIRECTION_MO, IMS_NULL);

    EXPECT_CALL(m_objMockSystem, StartImsTraffic(_, _, _, _, _))
            .Times(1)
            .WillRepeatedly(Return(-1));

    EXPECT_CALL(m_objRadioService.GetMockImsTraffic(), Start(_, _)).Times(1);

    m_pOsImsRadio->StartImsTraffic(IImsRadio::TRAFFIC_TYPE_REGISTRATION,
            IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, IImsRadio::DIRECTION_MO,
            &objImsRadioConnectionListener);

    EXPECT_CALL(m_objMockSystem, StartImsTraffic(_, _, _, _, _)).Times(1).WillRepeatedly(Return(1));

    m_pOsImsRadio->StartImsTraffic(IImsRadio::TRAFFIC_TYPE_REGISTRATION,
            IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, IImsRadio::DIRECTION_MO,
            &objImsRadioConnectionListener);

    EXPECT_CALL(m_objMockSystem, StopImsTraffic(_, _)).Times(0);
    m_pOsImsRadio->StopImsTraffic(IMS_NULL);

    EXPECT_CALL(m_objMockSystem, StopImsTraffic(_, _)).Times(1);
    EXPECT_CALL(m_objRadioService.GetMockImsTraffic(), Stop(_, _)).Times(1);
    m_pOsImsRadio->StopImsTraffic(&objImsRadioConnectionListener);
}

TEST_F(OsImsRadioTest, MultipleStartAndStopImsTraffic)
{
    EXPECT_CALL(m_objMockSystem, StartImsTraffic(_, _, _, _, _)).Times(2).WillRepeatedly(Return(1));
    EXPECT_CALL(m_objRadioService.GetMockImsTraffic(), Start(_, _)).Times(1);
    m_pOsImsRadio->StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VOICE,
            IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, IImsRadio::DIRECTION_MO,
            &objImsRadioConnectionListener);
    m_pOsImsRadio->StartImsTraffic(IImsRadio::TRAFFIC_TYPE_VOICE,
            IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, IImsRadio::DIRECTION_MT,
            &objImsRadioConnectionListener2);

    EXPECT_CALL(m_objMockSystem, StopImsTraffic(_, _)).Times(2);
    EXPECT_CALL(m_objRadioService.GetMockImsTraffic(), Stop(_, _)).Times(1);
    m_pOsImsRadio->StopImsTraffic(&objImsRadioConnectionListener);
    m_pOsImsRadio->StopImsTraffic(&objImsRadioConnectionListener2);
}

TEST_F(OsImsRadioTest, TriggerEpsFallback)
{
    EXPECT_CALL(m_objMockSystem, TriggerEpsFallback(_, _)).Times(1).WillOnce(Return(IMS_FAILURE));

    m_pOsImsRadio->TriggerEpsFallback(IImsRadio::EPSFB_REASON_NO_NETWORK_RESPONSE);

    EXPECT_CALL(m_objMockSystem, TriggerEpsFallback(_, _)).Times(1).WillOnce(Return(IMS_SUCCESS));

    m_pOsImsRadio->TriggerEpsFallback(IImsRadio::EPSFB_REASON_NO_NETWORK_RESPONSE);
}

TEST_F(OsImsRadioTest, SsacAddListener)
{
    MockIImsRadioSsacListener objImsRadioSsacListener;
    m_pOsImsRadio->AddListenerForSsac(IMS_NULL);
    m_pOsImsRadio->AddListenerForSsac(&objImsRadioSsacListener);

    IMS_UINTP wParam;
    IMS_UINTP lParam;

    EXPECT_CALL(m_objMockThread, PostMessageI(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [&](Unused, IMS_UINTP nWparam, IN IMS_UINTP nLparam)
                    {
                        wParam = nWparam;
                        lParam = nLparam;
                        return IMS_TRUE;
                    }));

    android::Parcel in;
    in.writeInt32(1);
    in.writeInt32(1);
    in.writeInt32(1);
    in.writeInt32(1);
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            EVENT_SSAC_STATE_CHANGED, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(objImsRadioSsacListener, ImsRadio_OnSsacChanged(_)).Times(1);
    m_pImsRadio->DispatchServiceMessage(wParam, lParam);

    SsacInfo objSsacInfo = m_pOsImsRadio->GetSsacInfo();

    EXPECT_EQ(objSsacInfo.nBarringFactorForVoice, 1);
    EXPECT_EQ(objSsacInfo.nBarringTimeSecForVoice, 1);
    EXPECT_EQ(objSsacInfo.nBarringFactorForVideo, 1);
    EXPECT_EQ(objSsacInfo.nBarringTimeSecForVideo, 1);

    m_pOsImsRadio->RemoveListenerForSsac(IMS_NULL);
    m_pOsImsRadio->RemoveListenerForSsac(&objImsRadioSsacListener);
}

TEST_F(OsImsRadioTest, TrafficPriorityListener)
{
    MockIImsRadioTrafficPriorityListener objTrafficPriorityListener;
    m_pOsImsRadio->AddListenerForTrafficPriority(IMS_NULL);
    m_pOsImsRadio->AddListenerForTrafficPriority(&objTrafficPriorityListener);

    m_pOsImsRadio->RemoveListenerForTrafficPriority(IMS_NULL);
    m_pOsImsRadio->RemoveListenerForTrafficPriority(&objTrafficPriorityListener);
}

TEST_F(OsImsRadioTest, ImsRadioConnectionListener_ConnectionFailed)
{
    EXPECT_CALL(m_objMockSystem, StartImsTraffic(_, _, _, _, _)).Times(1).WillRepeatedly(Return(1));

    m_pOsImsRadio->StartImsTraffic(IImsRadio::TRAFFIC_TYPE_REGISTRATION,
            IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, IImsRadio::DIRECTION_MO,
            &objImsRadioConnectionListener);

    IMS_UINTP wParam;
    IMS_UINTP lParam;

    EXPECT_CALL(m_objMockThread, PostMessageI(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [&](Unused, IMS_UINTP nWparam, IN IMS_UINTP nLparam)
                    {
                        wParam = nWparam;
                        lParam = nLparam;
                        return IMS_TRUE;
                    }));

    android::Parcel in;
    in.writeInt32(1);
    in.writeInt32(31);
    in.writeInt32(14);
    in.writeInt32(71);
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            EVENT_CONNECTION_FAILED, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(objImsRadioConnectionListener, ImsRadio_OnConnectionFailed(_, _, _)).Times(1);
    m_pImsRadio->DispatchServiceMessage(wParam, lParam);

    EXPECT_CALL(m_objMockSystem, StopImsTraffic(_, _)).Times(1);
    m_pOsImsRadio->StopImsTraffic(&objImsRadioConnectionListener);
}

TEST_F(OsImsRadioTest, ImsRadioConnectionListener)
{
    EXPECT_CALL(m_objMockSystem, StartImsTraffic(_, _, _, _, _)).Times(1).WillRepeatedly(Return(1));

    m_pOsImsRadio->StartImsTraffic(IImsRadio::TRAFFIC_TYPE_REGISTRATION,
            IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, IImsRadio::DIRECTION_MO,
            &objImsRadioConnectionListener);

    IMS_UINTP wParam;
    IMS_UINTP lParam;

    EXPECT_CALL(m_objMockThread, PostMessageI(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [&](Unused, IMS_UINTP nWparam, IN IMS_UINTP nLparam)
                    {
                        wParam = nWparam;
                        lParam = nLparam;
                        return IMS_TRUE;
                    }));

    android::Parcel in;
    in.writeInt32(1);
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            EVENT_CONNECTION_SETUP_PREPARED, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(objImsRadioConnectionListener, ImsRadio_OnConnectionSetupPrepared()).Times(1);
    m_pImsRadio->DispatchServiceMessage(wParam, lParam);

    EXPECT_CALL(m_objMockSystem, StopImsTraffic(_, _)).Times(1);
    m_pOsImsRadio->StopImsTraffic(&objImsRadioConnectionListener);
}

TEST_F(OsImsRadioTest, SetScSupportToImsTrafficWithTheValueGivenByScSupportChangedEvent)
{
    // GIVEN
    IMS_UINTP wParam, lParam;
    ON_CALL(m_objMockThread, PostMessageI(_, _, _))
            .WillByDefault(DoAll(SaveArg<1>(&wParam), SaveArg<2>(&lParam), Return(IMS_TRUE)));

    EXPECT_CALL(
            m_objRadioService.GetMockImsTraffic(), SetSimultaneousCallingSupported(_, IMS_TRUE));

    // WHEN
    android::Parcel in;
    in.writeInt32(1);
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            EVENT_SIMULTANEOUS_CALLING_SUPPORT_CHANGED, 0, reinterpret_cast<IMS_UINTP>(&in));
    m_pImsRadio->DispatchServiceMessage(wParam, lParam);

    // THEN: The GIVEN condition should be met.
}

}  // namespace android
