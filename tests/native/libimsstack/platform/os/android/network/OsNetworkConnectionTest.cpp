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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ImsNetworkConnectionState.h"
#include "ImsNetworkPing.h"
#include "ISystemListener.h"
#include "MockINetworkConnection.h"
#include "MockISystem.h"
#include "MockIThread.h"
#include "PlatformContext.h"
#include "ServiceNetworkPolicy.h"
#include "TestThreadService.h"
#include "network/OsNetworkConnection.h"
#include "network/OsNetworkConstants.h"
#include "system-intf/SystemConstants.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;

namespace android
{

class OsNetworkConnectionTest : public ::testing::Test
{
public:
    inline OsNetworkConnectionTest() :
            m_strApnName("mobile_ims"),
            m_strIfaceName("rmnet1"),
            m_strLocalIpv4Addr("192.168.1.5"),
            m_strLocalIpv6Addr("2002::2"),
            m_nPreferredIpVersion(IpAddress::UNKNOWN),
            m_pOldSystem(IMS_NULL),
            m_pThreadService(IMS_NULL)
    {
    }

protected:
    virtual void SetUp() override
    {
        m_pOldSystem = PlatformContext::GetInstance()->SetSystem(&m_objSystem);

        m_pThreadService = new TestThreadService();
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, m_pThreadService);
        m_pThreadService->SetThread(&m_objMockThread);

        EXPECT_CALL(m_objSystem, AddListener(SystemConstants::CATEGORY_NETWORK, _, _))
                .Times(AnyNumber());
        EXPECT_CALL(m_objSystem, RemoveListener(SystemConstants::CATEGORY_NETWORK, _, _))
                .Times(AnyNumber());

        m_nPreferredIpVersion = IpAddress::UNKNOWN;
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetSystem(m_pOldSystem);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        delete m_pThreadService;
    }

    void UpdateNetworkTypeAndAccessNetworkInfo(IN IMS_SINT32 nDefaultNetworkType,
            const AString& strMccOrSectorId, const AString& strMncOrSubnet,
            const AString& strCellId, const AString& strTacOrLac, const AString& strMode)
    {
        EXPECT_CALL(m_objSystem, GetNetworkType(_)).WillOnce(Return(nDefaultNetworkType));
        ON_CALL(m_objSystem, GetAccessNetworkInfo(_, _, _, _))
                .WillByDefault(Invoke(
                        [strMccOrSectorId, strMncOrSubnet, strCellId, strTacOrLac, strMode](
                                IN IMS_SINT32 nDefaultNetworkType, OUT IMS_SINT32& nNetworkType,
                                OUT AStringArray& objAccessNetInfo, Unused)
                        {
                            nNetworkType = nDefaultNetworkType;

                            if (strMccOrSectorId != AString::ConstNull())
                            {
                                objAccessNetInfo.AddElement(strMccOrSectorId);  // MCC or Sector ID
                            }

                            if (strMncOrSubnet != AString::ConstNull())
                            {
                                objAccessNetInfo.AddElement(strMncOrSubnet);  // MNC or Subnet
                            }

                            if (strCellId != AString::ConstNull())
                            {
                                objAccessNetInfo.AddElement(strCellId);  // CellID
                            }

                            if (strTacOrLac != AString::ConstNull())
                            {
                                objAccessNetInfo.AddElement(strTacOrLac);  // TAC or LAC
                            }

                            if (strMode != AString::ConstNull())
                            {
                                objAccessNetInfo.AddElement(strMode);  // MODE
                            }

                            return 1;
                        }));
    }

    void ActivateConnection(OsNetworkConnection& objOsNetworkConnection,
            IN IMS_SINT32 nCategory = IIpcan::CATEGORY_MOBILE)
    {
        INetworkConnection* pINetworkConnection = &objOsNetworkConnection;
        EXPECT_CALL(m_objSystem, GetDataConnectionState(_, _))
                .Times(AnyNumber())
                .WillOnce(Return(DATA_DISCONNECTED))      // First call to Activate()
                .WillOnce(Return(DATA_DISCONNECTED))      // SetPreferredIpVersion()
                .WillRepeatedly(Return(DATA_CONNECTED));  // second call to Activate()

        EXPECT_CALL(m_objSystem, RequestNetwork(_, _)).Times(AnyNumber()).WillOnce(Return(1));

        EXPECT_EQ(INetworkConnection::RESULT_DOING, pINetworkConnection->Activate(IMS_TRUE));
        EXPECT_EQ(INetworkConnection::STATE_DISCONNECTED, pINetworkConnection->GetState());

        EXPECT_CALL(m_objSystem, GetLocalAddress(_, -1, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(AString::ConstNull()));
        EXPECT_CALL(m_objSystem, GetLocalAddress(_, IpAddress::IPV4, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(m_strLocalIpv4Addr));
        EXPECT_CALL(m_objSystem, GetLocalAddress(_, IpAddress::IPV6, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(m_strLocalIpv6Addr));
        EXPECT_CALL(m_objSystem, GetApnName(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(m_strApnName));
        EXPECT_CALL(m_objSystem, GetIfaceId(_, _)).Times(AnyNumber()).WillRepeatedly(Return(2));
        EXPECT_CALL(m_objSystem, GetIfaceName(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(m_strIfaceName));
        EXPECT_CALL(m_objSystem, GetIpcanCategory(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(nCategory));
        EXPECT_CALL(m_objMockThread, PostMessageI(_))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        m_nPreferredIpVersion = (m_nPreferredIpVersion == IpAddress::IPV4)
                ? IpAddress::IPV6
                : IpAddress::IPV4;  // changing IP version to not skip GetDataConnectionState() call

        pINetworkConnection->SetPreferredIpVersion(m_nPreferredIpVersion);

        IMS_UINTP nWparam = OsNetworkConnection::NET_CONNECTED;
        IMS_UINTP nLparam = 30;

        pINetworkConnection->SetListener(&m_objMockINetworkConnectionListener);
        pINetworkConnection->AddReferenceListener(&m_objMockINetworkConnectionRefListener);

        EXPECT_CALL(m_objMockINetworkConnectionListener, NetworkConnection_OnConnected(_)).Times(1);
        EXPECT_CALL(m_objMockINetworkConnectionRefListener, NetworkConnection_OnConnected(_))
                .Times(1);

        static_cast<ImsNetworkConnection*>(&objOsNetworkConnection)
                ->DispatchServiceMessage(nWparam, nLparam);

        EXPECT_EQ(INetworkConnection::STATE_CONNECTED, pINetworkConnection->GetState());
        EXPECT_EQ(INetworkConnection::RESULT_DONE, pINetworkConnection->Activate(IMS_TRUE));
    }

protected:
    AString m_strApnName;
    AString m_strIfaceName;
    AString m_strLocalIpv4Addr;
    AString m_strLocalIpv6Addr;
    IMS_SINT32 m_nPreferredIpVersion;
    ISystem* m_pOldSystem;
    MockINetworkConnectionListener m_objMockINetworkConnectionListener;
    MockINetworkConnectionListener m_objMockINetworkConnectionRefListener;
    MockISystem m_objSystem;
    MockIThread m_objMockThread;
    TestThreadService* m_pThreadService;
};

TEST_F(OsNetworkConnectionTest, GetLocalAddress)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);

    EXPECT_EQ(IpAddress::NONE, objOsNetworkConnection.GetLocalAddress(IpAddress::UNKNOWN));
    EXPECT_EQ(IpAddress::NONE, objOsNetworkConnection.GetLocalAddress(IpAddress::IPV4));
    EXPECT_EQ(IpAddress::IPv6NONE, objOsNetworkConnection.GetLocalAddress(IpAddress::IPV6));
}

TEST_F(OsNetworkConnectionTest, Activate)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    INetworkConnection* pINetworkConnection = &objOsNetworkConnection;

    EXPECT_CALL(m_objSystem, GetDataConnectionState(_, _))
            .Times(AnyNumber())
            .WillOnce(Return(DATA_DISCONNECTED));

    EXPECT_CALL(m_objSystem, RequestNetwork(_, _)).Times(AnyNumber()).WillOnce(Return(0));

    EXPECT_EQ(INetworkConnection::RESULT_FAILED, pINetworkConnection->Activate(IMS_TRUE));
    EXPECT_EQ(INetworkConnection::STATE_DISCONNECTED, pINetworkConnection->GetState());

    ActivateConnection(objOsNetworkConnection);

    EXPECT_EQ(2, pINetworkConnection->GetIfaceId());
    EXPECT_EQ(m_strIfaceName, pINetworkConnection->GetIfaceName());

    pINetworkConnection->RemoveReferenceListener(&m_objMockINetworkConnectionRefListener);
}

TEST_F(OsNetworkConnectionTest, Deactivate)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    INetworkConnection* pINetworkConnection = &objOsNetworkConnection;

    EXPECT_CALL(m_objSystem, ReleaseNetwork(_, _))
            .Times(3)
            .WillOnce(Return(1))
            .WillOnce(Return(0))
            .WillOnce(Return(1));

    EXPECT_EQ(INetworkConnection::RESULT_DOING, pINetworkConnection->Deactivate(IMS_TRUE));
    EXPECT_EQ(INetworkConnection::RESULT_DONE, pINetworkConnection->Deactivate(IMS_TRUE));
    EXPECT_EQ(INetworkConnection::RESULT_DONE, pINetworkConnection->Deactivate(IMS_TRUE));

    EXPECT_EQ(INetworkConnection::STATE_DISCONNECTED, pINetworkConnection->GetState());
}

TEST_F(OsNetworkConnectionTest, GetAccessNetworkInfo)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    INetworkConnection* pINetworkConnection = &objOsNetworkConnection;

    AccessNetworkInfo objAccessNetInfo;

    EXPECT_CALL(m_objSystem, GetAccessNetworkInfo(_, _, _, _)).Times(AnyNumber());

    // LTE access network info - success
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_LTE, AString("450"), AString("06F"),
            AString("1177a00"), AString("d1"), AString("FDD"));

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD);
    EXPECT_STREQ(&objAccessNetInfo.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[0], "4500600d11177a00");

    // NR access network info - success
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_NR, AString("450"), AString("06F"),
            AString("1177a0000"), AString("d1"), AString("TDD"));

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_NR_TDD);
    EXPECT_STREQ(&objAccessNetInfo.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[0],
            "450060000d11177a0000");

    // GSM EDGE access network info - success
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_GPRS, AString("450"), AString("06F"),
            AString("1177"), AString("d1"), AString::ConstNull());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_GERAN);
    EXPECT_STREQ(&objAccessNetInfo.uniAI.cgi_3gpp.acCGI[0], "4500600d11177");

    // eHRPD access network info - success
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_EHRPD, AString("a022382"), AString("9"),
            AString::ConstNull(), AString::ConstNull(), AString::ConstNull());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP2_1X_HRPD);
    EXPECT_STREQ(&objAccessNetInfo.uniAI.ci_3gpp2.acCI[0], "0000000000000000000000000a02238209");

    // LTE access network info - missing elements in cell identities
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_LTE, AString("450"), AString("06F"),
            AString::ConstNull(), AString::ConstNull(), AString::ConstNull());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // NR access network info - missing elements in cell identities
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_NR, AString("450"), AString("06F"),
            AString("1177a0000"), AString::ConstNull(), AString::ConstNull());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_NR_FDD);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // GSM EDGE access network info - missing elements in cell identities
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_GPRS, AString("450"), AString("06F"),
            AString::ConstNull(), AString::ConstNull(), AString::ConstNull());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_GERAN);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // eHRPD access network info - missing elements in cell identities
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_EHRPD, AString("a022382"),
            AString::ConstNull(), AString::ConstNull(), AString::ConstNull(), AString::ConstNull());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP2_1X_HRPD);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // LTE access network info - MCC length > 3
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_LTE, AString("1450"), AString("06F"),
            AString("1177a00"), AString("d1"), AString("FDD"));

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // LTE access network info - MNC length > 3
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_LTE, AString("450"), AString("2650"),
            AString("1177a00"), AString("d1"), AString("FDD"));

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // LTE access network info - CellID length > 7
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_LTE, AString("450"), AString("06F"),
            AString("1177abcdef"), AString("d1"), AString("FDD"));

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // LTE access network info - TAC length > 4
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_LTE, AString("450"), AString("06F"),
            AString("1177a00"), AString("d1d2d3d4"), AString("FDD"));

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // NR access network info - MCC length > 3
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_NR, AString("45230"), AString("06F"),
            AString("1177a0000"), AString("d1"), AString("TDD"));

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_NR_TDD);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // NR access network info - MNC length > 3
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_NR, AString("450"), AString("06DEFF"),
            AString("1177a0000"), AString("d1"), AString("TDD"));

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_NR_TDD);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // NR access network info - CellID length > 9
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_NR, AString("450"), AString("06F"),
            AString("1177a00001234"), AString("d1"), AString("TDD"));

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_NR_TDD);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // NR access network info - TAC length > 6
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_NR, AString("450"), AString("06F"),
            AString("1177a0000"), AString("d1d2d3d4d5d6d7"), AString("TDD"));

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_NR_TDD);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // GSM EDGE access network info - MCC length > 3
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_GPRS, AString("45012"), AString("06F"),
            AString("1177"), AString("d1"), AString::ConstNull());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_GERAN);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // GSM EDGE access network info - MNC length > 3
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_GPRS, AString("450"), AString("06123F"),
            AString("1177"), AString("d1"), AString::ConstNull());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_GERAN);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // GSM EDGE access network info - CellID length > 4
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_GPRS, AString("450"), AString("06F"),
            AString("11771234"), AString("d1"), AString::ConstNull());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_GERAN);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // GSM EDGE access network info - LAC length > 4
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_GPRS, AString("450"), AString("06F"),
            AString("1177"), AString("d1d2d3d4"), AString::ConstNull());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_GERAN);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // eHRPD access network info - Sector ID length > 32
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_EHRPD,
            AString("a022382a022382a022382a022382a022382"), AString("9"), AString::ConstNull(),
            AString::ConstNull(), AString::ConstNull());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP2_1X_HRPD);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // eHRPD access network info - Subnet length > 2
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_EHRPD, AString("a022382"), AString("9999"),
            AString::ConstNull(), AString::ConstNull(), AString::ConstNull());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP2_1X_HRPD);
    EXPECT_EQ(IMS_FALSE, objAccessNetInfo.bIsAccessInfoRequired);

    // LTE access network info - all empty values
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_LTE, AString::ConstEmpty(),
            AString::ConstEmpty(), AString::ConstEmpty(), AString::ConstEmpty(),
            AString::ConstEmpty());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD);
    EXPECT_STREQ(&objAccessNetInfo.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[0], "00000000000");

    // NR access network info - all empty values
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_NR, AString::ConstEmpty(),
            AString::ConstEmpty(), AString::ConstEmpty(), AString::ConstEmpty(),
            AString::ConstEmpty());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_NR_FDD);
    EXPECT_STREQ(
            &objAccessNetInfo.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[0], "000000000000000");

    // GSM EDGE access network info - all empty values
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_GPRS, AString::ConstEmpty(),
            AString::ConstEmpty(), AString::ConstEmpty(), AString::ConstEmpty(),
            AString::ConstNull());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_GERAN);
    EXPECT_STREQ(&objAccessNetInfo.uniAI.cgi_3gpp.acCGI[0], "00000000");

    // eHRPD access network info - all empty values
    UpdateNetworkTypeAndAccessNetworkInfo(RADIOTECH_TYPE_EHRPD, AString::ConstEmpty(),
            AString::ConstEmpty(), AString::ConstNull(), AString::ConstNull(),
            AString::ConstNull());

    pINetworkConnection->GetAccessNetworkInfo(objAccessNetInfo);

    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP2_1X_HRPD);
    EXPECT_STREQ(&objAccessNetInfo.uniAI.ci_3gpp2.acCI[0], "0000000000000000000000000000000000");

    // For IPCan WLAN
    ActivateConnection(objOsNetworkConnection, IIpcan::CATEGORY_WLAN);

    AccessNetworkInfo objWifiAccessNetInfo;

    EXPECT_CALL(m_objSystem, GetWifiBssId())
            .Times(AnyNumber())
            .WillOnce(Return(AString("8c:3b:ad:8c:31:d0:8c:40")))
            .WillOnce(Return(AString(":::::")))
            .WillOnce(Return(AString("8c:3b:ad:8c:31:d0")));

    pINetworkConnection->GetAccessNetworkInfo(objWifiAccessNetInfo);

    EXPECT_EQ(objWifiAccessNetInfo.nType, AccessNetworkInfo::TYPE_NONE);
    EXPECT_EQ(objWifiAccessNetInfo.nClass, AccessNetworkInfo::CLASS_NONE);

    pINetworkConnection->GetAccessNetworkInfo(objWifiAccessNetInfo);

    EXPECT_EQ(objWifiAccessNetInfo.nType, AccessNetworkInfo::TYPE_IEEE_802_11);
    EXPECT_EQ(objWifiAccessNetInfo.nClass, AccessNetworkInfo::CLASS_NONE);
    EXPECT_STREQ(reinterpret_cast<char*>(&objWifiAccessNetInfo.uniAI.i_wlan_node_id.aMAC[0]),
            "\xFF\xFF\xFF\xFF\xFF\xFF");

    pINetworkConnection->GetAccessNetworkInfo(objWifiAccessNetInfo);

    EXPECT_EQ(objWifiAccessNetInfo.nType, AccessNetworkInfo::TYPE_IEEE_802_11);
    EXPECT_EQ(objWifiAccessNetInfo.nClass, AccessNetworkInfo::CLASS_NONE);
    EXPECT_STREQ(reinterpret_cast<char*>(&objWifiAccessNetInfo.uniAI.i_wlan_node_id.aMAC[0]),
            "\x8C\x3B\xAD\x8C\x31\xD0");
}

TEST_F(OsNetworkConnectionTest, GetLastAccessNetworkInfo)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    INetworkConnection* pINetworkConnection = &objOsNetworkConnection;

    AccessNetworkInfo objAccessNetInfo;
    AString strTimestamp;
    AString strCellInfoAge;

    EXPECT_CALL(m_objSystem, GetLastAccessNetworkInfo(_, _)).Times(AnyNumber());

    // Empty cell identities received
    ON_CALL(m_objSystem, GetLastAccessNetworkInfo(_, _))
            .WillByDefault(Invoke(
                    [](Unused, Unused)
                    {
                        return AStringArray::ConstNull();
                    }));

    pINetworkConnection->GetLastAccessNetworkInfo(objAccessNetInfo, strTimestamp, strCellInfoAge);

    EXPECT_EQ(strTimestamp, AString::ConstNull());
    EXPECT_EQ(strCellInfoAge, AString::ConstNull());

    // Access network info not received
    ON_CALL(m_objSystem, GetLastAccessNetworkInfo(_, _))
            .WillByDefault(Invoke(
                    [](Unused, Unused)
                    {
                        AStringArray objCellIdentities;

                        objCellIdentities.AddElement(AString("13"));  // Network type
                        objCellIdentities.AddElement(
                                AString("08:56:31Z"));                // timestamp as UTC format
                        objCellIdentities.AddElement(AString("60"));  // cell age as seconds format

                        return objCellIdentities;
                    }));

    pINetworkConnection->GetLastAccessNetworkInfo(objAccessNetInfo, strTimestamp, strCellInfoAge);

    EXPECT_EQ(strTimestamp, AString::ConstNull());
    EXPECT_EQ(strCellInfoAge, AString::ConstNull());

    // All information received
    ON_CALL(m_objSystem, GetLastAccessNetworkInfo(_, _))
            .WillByDefault(Invoke(
                    [](Unused, Unused)
                    {
                        AStringArray objCellIdentities;

                        objCellIdentities.AddElement(AString("13"));  // Network type
                        objCellIdentities.AddElement(
                                AString("08:56:31Z"));                 // timestamp as UTC format
                        objCellIdentities.AddElement(AString("60"));   // cell age as seconds format
                        objCellIdentities.AddElement(AString("450"));  // MCC or Sector ID
                        objCellIdentities.AddElement(AString("06F"));  // MNC or Subnet
                        objCellIdentities.AddElement(AString("1177a00"));  // CellID
                        objCellIdentities.AddElement(AString("d1"));       // TAC or LAC
                        objCellIdentities.AddElement(AString("FDD"));      // MODE

                        return objCellIdentities;
                    }));

    pINetworkConnection->GetLastAccessNetworkInfo(objAccessNetInfo, strTimestamp, strCellInfoAge);

    EXPECT_EQ(strTimestamp, AString("08:56:31Z"));
    EXPECT_EQ(strCellInfoAge, AString("60"));
    EXPECT_EQ(objAccessNetInfo.nType, AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD);
    EXPECT_STREQ(&objAccessNetInfo.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[0], "4500600d11177a00");
}

TEST_F(OsNetworkConnectionTest, GetExtraInfo)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    INetworkConnection* pINetworkConnection = &objOsNetworkConnection;

    AString strInfo;

    EXPECT_EQ(IMS_FALSE, pINetworkConnection->GetExtraInfo(AString("invalid"), strInfo));

    EXPECT_CALL(m_objSystem, GetNetworkType(_))
            .WillOnce(Return(RADIOTECH_TYPE_LTE))
            .WillOnce(Return(RADIOTECH_TYPE_LTE_CA))
            .WillOnce(Return(RADIOTECH_TYPE_NR))
            .WillOnce(Return(RADIOTECH_TYPE_UMTS))
            .WillOnce(Return(RADIOTECH_TYPE_HSDPA))
            .WillOnce(Return(RADIOTECH_TYPE_HSUPA))
            .WillOnce(Return(RADIOTECH_TYPE_HSPA))
            .WillOnce(Return(RADIOTECH_TYPE_EHRPD))
            .WillOnce(Return(RADIOTECH_TYPE_HSPAP))
            .WillOnce(Return(RADIOTECH_TYPE_UNKNOWN))
            .WillOnce(Return(RADIOTECH_TYPE_GPRS));

    EXPECT_EQ(IMS_TRUE, pINetworkConnection->GetExtraInfo(AString("rat"), strInfo));
    EXPECT_EQ(AString("LTE"), strInfo);

    EXPECT_EQ(IMS_TRUE, pINetworkConnection->GetExtraInfo(AString("rat"), strInfo));
    EXPECT_EQ(AString("LTE"), strInfo);

    EXPECT_EQ(IMS_TRUE, pINetworkConnection->GetExtraInfo(AString("rat"), strInfo));
    EXPECT_EQ(AString("NR"), strInfo);

    EXPECT_EQ(IMS_TRUE, pINetworkConnection->GetExtraInfo(AString("rat"), strInfo));
    EXPECT_EQ(AString("3G"), strInfo);

    EXPECT_EQ(IMS_TRUE, pINetworkConnection->GetExtraInfo(AString("rat"), strInfo));
    EXPECT_EQ(AString("3G"), strInfo);

    EXPECT_EQ(IMS_TRUE, pINetworkConnection->GetExtraInfo(AString("rat"), strInfo));
    EXPECT_EQ(AString("3G"), strInfo);

    EXPECT_EQ(IMS_TRUE, pINetworkConnection->GetExtraInfo(AString("rat"), strInfo));
    EXPECT_EQ(AString("3G"), strInfo);

    EXPECT_EQ(IMS_TRUE, pINetworkConnection->GetExtraInfo(AString("rat"), strInfo));
    EXPECT_EQ(AString("3G"), strInfo);

    EXPECT_EQ(IMS_TRUE, pINetworkConnection->GetExtraInfo(AString("rat"), strInfo));
    EXPECT_EQ(AString("3G"), strInfo);

    EXPECT_EQ(IMS_TRUE, pINetworkConnection->GetExtraInfo(AString("rat"), strInfo));
    EXPECT_EQ(AString("Unknown"), strInfo);

    EXPECT_EQ(IMS_TRUE, pINetworkConnection->GetExtraInfo(AString("rat"), strInfo));
    EXPECT_EQ(AString("2G"), strInfo);

    strInfo = AString::ConstNull();

    EXPECT_EQ(IMS_TRUE, pINetworkConnection->GetExtraInfo(AString("policy_name"), strInfo));
    EXPECT_EQ(AString::ConstNull(), strInfo);

    EXPECT_EQ(IMS_TRUE, pINetworkConnection->GetExtraInfo(AString("apn"), strInfo));
    EXPECT_EQ(AString::ConstNull(), strInfo);

    ActivateConnection(objOsNetworkConnection);

    EXPECT_EQ(IMS_TRUE, pINetworkConnection->GetExtraInfo(AString("apn"), strInfo));
    EXPECT_EQ(m_strApnName, strInfo);
}

TEST_F(OsNetworkConnectionTest, GetHostByName)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    INetworkConnection* pINetworkConnection = &objOsNetworkConnection;

    ImsList<IpAddress> objIpAddrs;
    AStringArray objIpAddresses;
    AStringArray objInvalidIpAddresses;

    objIpAddresses.AddElement(AString("192.168.2.5"));
    objIpAddresses.AddElement(AString("192.168.2.10"));

    objInvalidIpAddresses.AddElement(AString("192-168-2-5"));
    objInvalidIpAddresses.AddElement(AString("192-168-2-10"));

    EXPECT_EQ(-1, pINetworkConnection->GetHostByName(AString("hostname"), objIpAddrs));
    EXPECT_TRUE(objIpAddrs.IsEmpty());

    EXPECT_CALL(m_objSystem, GetHostByName(_, _, _, _))
            .WillOnce(Return(AStringArray::ConstNull()))
            .WillOnce(Return(objInvalidIpAddresses))
            .WillOnce(Return(objIpAddresses));

    EXPECT_EQ(-1, pINetworkConnection->GetHostByName(AString("hostname"), objIpAddrs));
    EXPECT_TRUE(objIpAddrs.IsEmpty());

    EXPECT_EQ(-1, pINetworkConnection->GetHostByName(AString("hostname"), objIpAddrs));
    EXPECT_TRUE(objIpAddrs.IsEmpty());

    EXPECT_EQ(1, pINetworkConnection->GetHostByName(AString("hostname"), objIpAddrs));
    EXPECT_FALSE(objIpAddrs.IsEmpty());
}

TEST_F(OsNetworkConnectionTest, GetPcscfAddress)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    INetworkConnection* pINetworkConnection = &objOsNetworkConnection;

    AStringArray objPcscfAddresses;

    objPcscfAddresses.AddElement(AString("192.168.2.5"));
    objPcscfAddresses.AddElement(AString("192.168.2.10"));

    EXPECT_CALL(m_objSystem, GetPcscfAddresses(_, _, _)).WillOnce(Return(objPcscfAddresses));

    const AStringArray& objPcscfs = pINetworkConnection->GetPcscfAddress();

    EXPECT_EQ(objPcscfAddresses.GetElementAt(0), objPcscfs.GetElementAt(0));
    EXPECT_EQ(objPcscfAddresses.GetElementAt(1), objPcscfs.GetElementAt(1));
}

TEST_F(OsNetworkConnectionTest, IsePDGEnabled)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    INetworkConnection* pINetworkConnection = &objOsNetworkConnection;

    ActivateConnection(objOsNetworkConnection, IIpcan::CATEGORY_WLAN);

    EXPECT_TRUE(pINetworkConnection->IsePDGEnabled());
}

TEST_F(OsNetworkConnectionTest, IsIpv6Preferred)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    INetworkConnection* pINetworkConnection = &objOsNetworkConnection;

    EXPECT_CALL(m_objSystem, IsIpv6Preferred(_, _)).WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(pINetworkConnection->IsIpv6Preferred());
}

TEST_F(OsNetworkConnectionTest, IsMobileDataEnabled)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    INetworkConnection* pINetworkConnection = &objOsNetworkConnection;

    EXPECT_CALL(m_objSystem, IsMobileDataEnabled(_)).WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(pINetworkConnection->IsMobileDataEnabled());
}

TEST_F(OsNetworkConnectionTest, GetMtu)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    INetworkConnection* pINetworkConnection = &objOsNetworkConnection;

    EXPECT_CALL(m_objSystem, GetMtu(_, _)).WillOnce(Return(1200));

    EXPECT_EQ(1200, pINetworkConnection->GetMtu());
}

TEST_F(OsNetworkConnectionTest, CreatePing)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    INetworkConnection* pINetworkConnection = &objOsNetworkConnection;
    INetworkPing* pINetworkPing = pINetworkConnection->CreatePing();

    ASSERT_TRUE(pINetworkPing != nullptr);
    pINetworkPing->Destroy();
}

TEST_F(OsNetworkConnectionTest, Create_ProfileName)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    ImsNetworkConnection* pImsNetworkConnection = &objOsNetworkConnection;

    EXPECT_EQ(IMS_TRUE, pImsNetworkConnection->Create(AString("")));
    EXPECT_EQ(m_strApnName, pImsNetworkConnection->GetProfileName());
    EXPECT_EQ(NetworkPolicy::APN_IMS, pImsNetworkConnection->GetApnType());

    ImsNetworkConnectionState::GetInstance()->DetachHandle(m_strApnName, IMS_SLOT_0);

    NetworkServicePolicy* pNetworkServicePolicy = NetworkServicePolicy::GetInstance();
    NetworkPolicy objNetworkPolicy(IMS_TRUE, AString("mobile_test"), NetworkPolicy::APN_WIFI_MAX);

    pNetworkServicePolicy->AddPolicy(AString("mobile_test"), objNetworkPolicy, IMS_SLOT_0);

    EXPECT_EQ(IMS_TRUE, pImsNetworkConnection->Create(AString("mobile_test")));
    EXPECT_EQ(AString("mobile_test"), pImsNetworkConnection->GetProfileName());
    EXPECT_EQ(NetworkPolicy::APN_WIFI_MAX, pImsNetworkConnection->GetApnType());

    EXPECT_TRUE(pImsNetworkConnection->GetHandle() != 0);

    ImsNetworkConnectionState::GetInstance()->DetachHandle(AString("mobile_test"), IMS_SLOT_0);

    pNetworkServicePolicy->RemovePolicy(AString("mobile_test"), IMS_SLOT_0);
}

TEST_F(OsNetworkConnectionTest, Create_ApnType)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    ImsNetworkConnection* pImsNetworkConnection = &objOsNetworkConnection;

    EXPECT_EQ(IMS_TRUE, pImsNetworkConnection->Create(NetworkPolicy::APN_NONE));
    EXPECT_EQ(m_strApnName, pImsNetworkConnection->GetProfileName());
    EXPECT_EQ(NetworkPolicy::APN_IMS, pImsNetworkConnection->GetApnType());

    ImsNetworkConnectionState::GetInstance()->DetachHandle(m_strApnName, IMS_SLOT_0);

    NetworkServicePolicy* pNetworkServicePolicy = NetworkServicePolicy::GetInstance();
    NetworkPolicy objNetworkPolicy(IMS_TRUE, AString("mobile_test"), NetworkPolicy::APN_WIFI_MAX);

    pNetworkServicePolicy->AddPolicy(AString("mobile_test"), objNetworkPolicy, IMS_SLOT_0);

    EXPECT_EQ(IMS_TRUE, pImsNetworkConnection->Create(NetworkPolicy::APN_WIFI_MAX));
    EXPECT_EQ(AString("mobile_test"), pImsNetworkConnection->GetProfileName());
    EXPECT_EQ(NetworkPolicy::APN_WIFI_MAX, pImsNetworkConnection->GetApnType());

    EXPECT_TRUE(pImsNetworkConnection->GetHandle() != 0);

    ImsNetworkConnectionState::GetInstance()->DetachHandle(AString("mobile_test"), IMS_SLOT_0);

    pNetworkServicePolicy->RemovePolicy(AString("mobile_test"), IMS_SLOT_0);
}

TEST_F(OsNetworkConnectionTest, Equals)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    ImsNetworkConnection* pImsNetworkConnection = &objOsNetworkConnection;
    IpAddress onjIpv4Address(m_strLocalIpv4Addr);
    IpAddress onjIpv6Address(m_strLocalIpv6Addr);

    EXPECT_FALSE(pImsNetworkConnection->Equals(onjIpv4Address));
    EXPECT_FALSE(pImsNetworkConnection->Equals(onjIpv6Address));

    // Create active connection - Ipv4 and Ipv6 address in ActivateConnection() are same as above
    ActivateConnection(objOsNetworkConnection);

    EXPECT_TRUE(pImsNetworkConnection->Equals(onjIpv4Address));
    EXPECT_TRUE(pImsNetworkConnection->Equals(onjIpv6Address));
}

TEST_F(OsNetworkConnectionTest, System_NotifyEvent)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    ISystemListener* pISystemListener = &objOsNetworkConnection;

    ImsMessage objMessage;

    EXPECT_CALL(m_objMockThread, PostMessageI(_)).Times(8);

    ON_CALL(m_objMockThread, PostMessageI(_))
            .WillByDefault(Invoke(
                    [&](IN const ImsMessage& objMsg)
                    {
                        objMessage = objMsg;
                        return IMS_TRUE;
                    }));

    pISystemListener->System_NotifyEvent(
            IMS_SYSTEM_DATACONNECTION_FAILED, NetworkPolicy::APN_IMS, 0);

    EXPECT_EQ(OsNetworkConnection::NET_CONNECT_FAILED, objMessage.nWparam);

    pISystemListener->System_NotifyEvent(
            IMS_SYSTEM_DATACONNECTION_STATE_CHANGED, NetworkPolicy::APN_IMS, DATA_DISCONNECTED);

    EXPECT_EQ(OsNetworkConnection::NET_DISCONNECTED, objMessage.nWparam);

    pISystemListener->System_NotifyEvent(
            IMS_SYSTEM_DATACONNECTION_STATE_CHANGED, NetworkPolicy::APN_IMS, DATA_CONNECTED);

    EXPECT_EQ(OsNetworkConnection::NET_CONNECTED, objMessage.nWparam);

    pISystemListener->System_NotifyEvent(
            IMS_SYSTEM_DATACONNECTION_STATE_CHANGED, NetworkPolicy::APN_IMS, DATA_CONNECT_FAILED);

    EXPECT_EQ(OsNetworkConnection::NET_CONNECT_FAILED, objMessage.nWparam);

    pISystemListener->System_NotifyEvent(
            IMS_SYSTEM_DATACONNECTION_STATE_CHANGED, NetworkPolicy::APN_IMS, DATA_IPCHANGED);

    EXPECT_EQ(OsNetworkConnection::NET_IP_CHANGED, objMessage.nWparam);

    pISystemListener->System_NotifyEvent(
            IMS_SYSTEM_DATACONNECTION_STATE_CHANGED, NetworkPolicy::APN_IMS, DATA_PCSCF_CHANGED);

    EXPECT_EQ(OsNetworkConnection::NET_PCSCF_CHANGED, objMessage.nWparam);

    pISystemListener->System_NotifyEvent(
            IMS_SYSTEM_DATACONNECTION_IPCAN_CHANGED, NetworkPolicy::APN_IMS, IIpcan::CATEGORY_WLAN);

    EXPECT_EQ(OsNetworkConnection::NET_IPCAN_CAT_CHANGED, objMessage.nWparam);

    pISystemListener->System_NotifyEvent(IMS_SYSTEM_DATACONNECTION_IPCAN_CHANGED,
            NetworkPolicy::APN_IMS, IIpcan::CATEGORY_MOBILE);

    EXPECT_EQ(OsNetworkConnection::NET_IPCAN_CAT_CHANGED, objMessage.nWparam);

    // PostMessageI() should not be called
    pISystemListener->System_NotifyEvent(
            IMS_SYSTEM_DATACONNECTION_FAILED, NetworkPolicy::APN_NONE, 0);  // Apn type mismatch
    pISystemListener->System_NotifyEvent(IMS_SYSTEM_DATACONNECTION_IPCAN_CHANGED,
            NetworkPolicy::APN_IMS,
            IIpcan::CATEGORY_MOBILE);  // previous IPCan is same - IIpcan::CATEGORY_MOBILE
    pISystemListener->System_NotifyEvent(IMS_SYSTEM_DATACONNECTION_STATE_CHANGED,
            NetworkPolicy::APN_IMS, DATA_INVALID);  // DATA_INVALID invalid message
    pISystemListener->System_NotifyEvent(IMS_SYSTEM_SERVICE_STATE_CHANGED, NetworkPolicy::APN_IMS,
            DATA_CONNECTED);  // IMS_SYSTEM_SERVICE_STATE_CHANGED not handled
}

TEST_F(OsNetworkConnectionTest, DispatchServiceMessage)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    INetworkConnection* pINetworkConnection = &objOsNetworkConnection;
    ImsNetworkConnection* pImsNetworkConnection = &objOsNetworkConnection;

    // Create active connection
    ActivateConnection(objOsNetworkConnection);

    EXPECT_CALL(m_objMockINetworkConnectionListener, NetworkConnection_OnDisconnected(_, _))
            .Times(1);
    EXPECT_CALL(m_objMockINetworkConnectionRefListener, NetworkConnection_OnDisconnected(_, _))
            .Times(1);

    // Connection disconnected
    pImsNetworkConnection->DispatchServiceMessage(OsNetworkConnection::NET_DISCONNECTED, 0);

    EXPECT_EQ(INetworkConnection::STATE_DISCONNECTED, pINetworkConnection->GetState());

    EXPECT_CALL(m_objSystem, GetDataConnectionState(_, _))
            .Times(AnyNumber())
            .WillOnce(Return(DATA_DISCONNECTED));

    EXPECT_CALL(m_objSystem, RequestNetwork(_, _)).Times(AnyNumber()).WillRepeatedly(Return(1));

    EXPECT_EQ(INetworkConnection::RESULT_DOING, pINetworkConnection->Activate(IMS_TRUE));

    EXPECT_CALL(m_objMockINetworkConnectionListener, NetworkConnection_OnConnectionFailed(_, _))
            .Times(1);
    EXPECT_CALL(m_objMockINetworkConnectionRefListener, NetworkConnection_OnConnectionFailed(_, _))
            .Times(1);

    // Connection fail
    pImsNetworkConnection->DispatchServiceMessage(OsNetworkConnection::NET_CONNECT_FAILED, 0);

    EXPECT_EQ(INetworkConnection::STATE_DISCONNECTED, pINetworkConnection->GetState());

    // IP changed with no active connection
    EXPECT_CALL(m_objMockINetworkConnectionListener, NetworkConnection_OnIpChanged(_)).Times(0);
    EXPECT_CALL(m_objMockINetworkConnectionRefListener, NetworkConnection_OnIpChanged(_)).Times(0);

    pImsNetworkConnection->DispatchServiceMessage(OsNetworkConnection::NET_IP_CHANGED, 0);

    // IPCan category changed with no active connection
    EXPECT_CALL(m_objMockINetworkConnectionListener, NetworkConnection_OnIpcanChanged(_)).Times(0);
    EXPECT_CALL(m_objMockINetworkConnectionRefListener, NetworkConnection_OnIpcanChanged(_))
            .Times(0);

    pImsNetworkConnection->DispatchServiceMessage(OsNetworkConnection::NET_IPCAN_CAT_CHANGED, 0);

    // pcscf changed with no active connection
    EXPECT_CALL(m_objMockINetworkConnectionListener, NetworkConnection_OnPcscfChanged(_)).Times(0);
    EXPECT_CALL(m_objMockINetworkConnectionRefListener, NetworkConnection_OnPcscfChanged(_))
            .Times(0);

    pImsNetworkConnection->DispatchServiceMessage(OsNetworkConnection::NET_PCSCF_CHANGED, 0);

    // Create active connection
    ActivateConnection(objOsNetworkConnection);

    EXPECT_CALL(m_objMockINetworkConnectionListener, NetworkConnection_OnIpChanged(_)).Times(1);
    EXPECT_CALL(m_objMockINetworkConnectionRefListener, NetworkConnection_OnIpChanged(_)).Times(1);

    // IP changed with active connection
    pImsNetworkConnection->DispatchServiceMessage(OsNetworkConnection::NET_IP_CHANGED, 0);

    EXPECT_EQ(INetworkConnection::STATE_CONNECTED, pINetworkConnection->GetState());

    EXPECT_CALL(m_objMockINetworkConnectionListener, NetworkConnection_OnIpcanChanged(_)).Times(1);
    EXPECT_CALL(m_objMockINetworkConnectionRefListener, NetworkConnection_OnIpcanChanged(_))
            .Times(1);

    // IPCan category changed with active connection
    pImsNetworkConnection->DispatchServiceMessage(OsNetworkConnection::NET_IPCAN_CAT_CHANGED, 0);

    EXPECT_EQ(INetworkConnection::STATE_CONNECTED, pINetworkConnection->GetState());

    EXPECT_CALL(m_objMockINetworkConnectionListener, NetworkConnection_OnPcscfChanged(_)).Times(1);
    EXPECT_CALL(m_objMockINetworkConnectionRefListener, NetworkConnection_OnPcscfChanged(_))
            .Times(1);

    // pcscf changed with active connection
    pImsNetworkConnection->DispatchServiceMessage(OsNetworkConnection::NET_PCSCF_CHANGED, 0);

    EXPECT_EQ(INetworkConnection::STATE_CONNECTED, pINetworkConnection->GetState());
}

TEST_F(OsNetworkConnectionTest, HandleEmergencyPdnOnIpChanged)
{
    OsNetworkConnection objOsNetworkConnection(IMS_SLOT_0);
    ImsNetworkConnection* pImsNetworkConnection = &objOsNetworkConnection;

    ASSERT_TRUE(pImsNetworkConnection->Create(AString("mobile_emergency")));

    // Create active connection
    ActivateConnection(objOsNetworkConnection);
    EXPECT_EQ(NetworkPolicy::APN_EMERGENCY, pImsNetworkConnection->GetApnType());

    EXPECT_CALL(m_objMockINetworkConnectionListener, NetworkConnection_OnIpChanged(_)).Times(1);
    EXPECT_CALL(m_objMockINetworkConnectionRefListener, NetworkConnection_OnIpChanged(_)).Times(1);

    EXPECT_CALL(m_objSystem, GetLocalAddress(_, -1, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(AString::ConstNull()));
    EXPECT_CALL(m_objSystem, GetLocalAddress(_, IpAddress::IPV4, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(AString("192.168.1.7")));
    EXPECT_CALL(m_objSystem, GetLocalAddress(_, IpAddress::IPV6, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(AString("2002::9")));

    // IP changed with active connection
    pImsNetworkConnection->DispatchServiceMessage(OsNetworkConnection::NET_IP_CHANGED, 0);
}

}  // namespace android
