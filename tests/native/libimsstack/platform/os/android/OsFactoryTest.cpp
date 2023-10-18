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

#include "ImsCarrierConfig.h"
#include "ImsFdSet.h"
#include "ImsFile.h"
#include "ImsIsim.h"
#include "ImsMutex.h"
#include "ImsNetworkConnection.h"
#include "ImsRadio.h"
#include "ImsSocket.h"
#include "ImsThread.h"
#include "ImsTimer.h"
#include "ImsTrace.h"
#include "ImsUsim.h"
#include "INetworkWatcher.h"
#include "MockISystem.h"
#include "OsFactory.h"
#include "PlatformContext.h"

using ::testing::_;

namespace android
{

class OsFactoryTest : public ::testing::Test
{
public:
    OsFactory* m_pOsFactory;
    ISystem* m_piDefaultSystem;
    MockISystem m_objMockSystem;

protected:
    virtual void SetUp() override
    {
        m_piDefaultSystem = PlatformContext::GetInstance()->SetSystem(&m_objMockSystem);

        m_pOsFactory = new OsFactory();
        ASSERT_TRUE(m_pOsFactory != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pOsFactory != IMS_NULL)
        {
            delete m_pOsFactory;
            m_pOsFactory = IMS_NULL;
        }
        PlatformContext::GetInstance()->SetSystem(m_piDefaultSystem);
    }
};

TEST_F(OsFactoryTest, CreateTrace)
{
    ImsTrace* pImsTrace = m_pOsFactory->CreateTrace();
    ASSERT_TRUE(pImsTrace != nullptr);
    delete pImsTrace;
}

TEST_F(OsFactoryTest, CreateMutex)
{
    ImsMutex* pImsMutex = m_pOsFactory->CreateMutex();
    ASSERT_TRUE(pImsMutex != nullptr);
    delete pImsMutex;
}

TEST_F(OsFactoryTest, CreateFile)
{
    ImsFile* pImsFile = m_pOsFactory->CreateFile();
    ASSERT_TRUE(pImsFile != nullptr);
    delete pImsFile;
}

TEST_F(OsFactoryTest, CreateFileUtil)
{
    IFileUtil* pIFileUtil = m_pOsFactory->CreateFileUtil();
    ASSERT_TRUE(pIFileUtil != nullptr);
    m_pOsFactory->DestroyFileUtil(pIFileUtil);
}

TEST_F(OsFactoryTest, CreateThread)
{
    AString strName("TestThread");
    ImsThread* pImsThread = m_pOsFactory->CreateThread(strName, IMS_SLOT_0);
    ASSERT_TRUE(pImsThread != nullptr);
    delete pImsThread;
}

TEST_F(OsFactoryTest, GetCurrentThreadId)
{
    EXPECT_GT(m_pOsFactory->GetCurrentThreadId(), 0);
}

TEST_F(OsFactoryTest, CreateTimer)
{
    ImsTimer* pImsTimer = m_pOsFactory->CreateTimer();
    ASSERT_TRUE(pImsTimer != nullptr);
    delete pImsTimer;
}

TEST_F(OsFactoryTest, CreateSystemTime)
{
    ISystemTime* pSysTimer = m_pOsFactory->CreateSystemTime();
    ASSERT_TRUE(pSysTimer != nullptr);
    m_pOsFactory->DestroySystemTime(pSysTimer);
}

TEST_F(OsFactoryTest, CreateEventReceiver)
{
    IEventReceiver* pEventReceiver = m_pOsFactory->CreateEventReceiver(IMS_SLOT_0);
    ASSERT_TRUE(pEventReceiver != nullptr);
    m_pOsFactory->DestroyEventReceiver(pEventReceiver);
}

TEST_F(OsFactoryTest, CreateEventSender)
{
    IEventSender* pEventSender = m_pOsFactory->CreateEventSender();
    ASSERT_TRUE(pEventSender != nullptr);
    m_pOsFactory->DestroyEventSender(pEventSender);
}

TEST_F(OsFactoryTest, CreateNetworkConnection)
{
    EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(2);
    EXPECT_CALL(m_objMockSystem, RemoveListener(_, _, _)).Times(2);

    ImsNetworkConnection* pImsNetworkConnection =
            m_pOsFactory->CreateNetworkConnection(NetworkPolicy::APN_IMS, IMS_SLOT_0);
    ASSERT_TRUE(pImsNetworkConnection != nullptr);
    delete pImsNetworkConnection;

    ImsNetworkConnection* pWifiConnection =
            m_pOsFactory->CreateNetworkConnection(NetworkPolicy::APN_WIFI, IMS_SLOT_0);
    ASSERT_TRUE(pWifiConnection != nullptr);
    delete pWifiConnection;

    ASSERT_TRUE(
            m_pOsFactory->CreateNetworkConnection(NetworkPolicy::APN_NONE, IMS_SLOT_0) == nullptr);
}

TEST_F(OsFactoryTest, CreateNetworkIpSec)
{
    INetworkIpSec* pNetworkIpSec = m_pOsFactory->CreateNetworkIpSec(IMS_SLOT_0);
    ASSERT_TRUE(pNetworkIpSec != nullptr);
    m_pOsFactory->DestroyNetworkIpSec(pNetworkIpSec);
}

TEST_F(OsFactoryTest, CreateIpcan)
{
    IIpcan* pIpcan = m_pOsFactory->CreateIpcan();
    ASSERT_TRUE(pIpcan != nullptr);
    m_pOsFactory->DestroyIpcan(pIpcan);
}

TEST_F(OsFactoryTest, CreateFdSet)
{
    ImsFdSet* pFdSet = m_pOsFactory->CreateFdSet(ImsFdSet::TYPE_POLL);
    ASSERT_TRUE(pFdSet != nullptr);
    delete pFdSet;

    ImsFdSet* pSelectFdSet = m_pOsFactory->CreateFdSet();
    ASSERT_TRUE(pSelectFdSet != nullptr);
    delete pSelectFdSet;
}

TEST_F(OsFactoryTest, CreateSocket)
{
    ImsSocket* pImsSocket = m_pOsFactory->CreateSocket();
    ASSERT_TRUE(pImsSocket != nullptr);
    delete pImsSocket;
}

TEST_F(OsFactoryTest, CreateSslSocket)
{
    ImsSocket* pOsSslSocket = m_pOsFactory->CreateSslSocket(IMS_NULL);
    ASSERT_TRUE(pOsSslSocket != nullptr);
    delete pOsSslSocket;
}

TEST_F(OsFactoryTest, GetSystemUtil)
{
    ISystemUtil* pSystemUtil = m_pOsFactory->GetSystemUtil();
    ASSERT_TRUE(pSystemUtil != nullptr);
}

TEST_F(OsFactoryTest, GetSystemProperty)
{
    ISystemProperty* pSystemProperty = m_pOsFactory->GetSystemProperty();
    ASSERT_TRUE(pSystemProperty != nullptr);
}

TEST_F(OsFactoryTest, GetZLib)
{
    IZLib* pZlib = m_pOsFactory->GetZLib();
    ASSERT_TRUE(pZlib != nullptr);
}

TEST_F(OsFactoryTest, CreatePowerInfo)
{
    EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
    EXPECT_CALL(m_objMockSystem, RemoveListener(_, _, _)).Times(1);

    IPowerInfo* pPowerInfo = m_pOsFactory->CreatePowerInfo();
    ASSERT_TRUE(pPowerInfo != nullptr);
    m_pOsFactory->DestroyPowerInfo(pPowerInfo);
}

TEST_F(OsFactoryTest, CreateDeviceInfo)
{
    IDeviceInfo* pDeviceInfo = m_pOsFactory->CreateDeviceInfo();
    ASSERT_TRUE(pDeviceInfo != nullptr);
    m_pOsFactory->DestroyDeviceInfo(pDeviceInfo);
}

TEST_F(OsFactoryTest, CreateSubscriberInfo)
{
    ISubscriberInfo* pSubscriberInfo = m_pOsFactory->CreateSubscriberInfo(IMS_SLOT_0);
    ASSERT_TRUE(pSubscriberInfo != nullptr);
    m_pOsFactory->DestroySubscriberInfo(pSubscriberInfo);
}

TEST_F(OsFactoryTest, CreateNetworkWatcher)
{
    EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
    EXPECT_CALL(m_objMockSystem, RemoveListener(_, _, _)).Times(1);

    INetworkWatcher* pNetworkWatcher = m_pOsFactory->CreateNetworkWatcher(IMS_SLOT_0);
    ASSERT_TRUE(pNetworkWatcher != nullptr);
    m_pOsFactory->DestroyNetworkWatcher(pNetworkWatcher);
}

TEST_F(OsFactoryTest, CreateWifiWatcher)
{
    EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
    EXPECT_CALL(m_objMockSystem, RemoveListener(_, _, _)).Times(1);

    IWifiWatcher* pWifiWatcher = m_pOsFactory->CreateWifiWatcher();
    ASSERT_TRUE(pWifiWatcher != nullptr);
    m_pOsFactory->DestroyWifiWatcher(pWifiWatcher);
}

TEST_F(OsFactoryTest, CreateCallInfo)
{
    ICallInfo* pCallInfo = m_pOsFactory->CreateCallInfo(IMS_SLOT_0);
    ASSERT_TRUE(pCallInfo != nullptr);
    m_pOsFactory->DestroyCallInfo(pCallInfo);
}

TEST_F(OsFactoryTest, CreateLocationInfo)
{
    ILocationInfo* pLocationInfo = m_pOsFactory->CreateLocationInfo(IMS_SLOT_0);
    ASSERT_TRUE(pLocationInfo != nullptr);
    m_pOsFactory->DestroyLocationInfo(pLocationInfo);
}

TEST_F(OsFactoryTest, CreateIsim)
{
    EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
    EXPECT_CALL(m_objMockSystem, RemoveListener(_, _, _)).Times(1);

    ImsIsim* pIsim = m_pOsFactory->CreateIsim(IMS_SLOT_0);
    ASSERT_TRUE(pIsim != nullptr);
    pIsim->Destroy();
}

TEST_F(OsFactoryTest, CreateUsim)
{
    EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
    EXPECT_CALL(m_objMockSystem, RemoveListener(_, _, _)).Times(1);

    ImsUsim* pUsim = m_pOsFactory->CreateUsim(IMS_SLOT_0);
    ASSERT_TRUE(pUsim != nullptr);
    pUsim->Destroy();
}

TEST_F(OsFactoryTest, CreateCarrierConfig)
{
    EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
    EXPECT_CALL(m_objMockSystem, RemoveListener(_, _, _)).Times(1);

    ImsCarrierConfig* pImsCarrierConfig = m_pOsFactory->CreateCarrierConfig(IMS_SLOT_0);
    ASSERT_TRUE(pImsCarrierConfig != nullptr);
    pImsCarrierConfig->Destroy();
}

TEST_F(OsFactoryTest, CreateImsRadio)
{
    EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
    EXPECT_CALL(m_objMockSystem, RemoveListener(_, _, _)).Times(1);

    ImsRadio* pImsRadio = m_pOsFactory->CreateImsRadio(IMS_SLOT_0);
    ASSERT_TRUE(pImsRadio != nullptr);
    delete pImsRadio;
}

}  // namespace android
