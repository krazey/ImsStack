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

#include "ImsNetworkConnectionState.h"

namespace android
{
static const IMS_BOOL CONNECTION_WIFI = IMS_FALSE;
static const IMS_BOOL CONNECTION_MOBILE = IMS_TRUE;

static const IMS_SINT32 APN_TYPE_ONE = 1;
static const IMS_SINT32 APN_TYPE_TWO = 2;
static const IMS_SINT32 APN_TYPE_THREE = 3;

static const AString PROFILE_ONE("one");
static const AString PROFILE_TWO("two");
static const AString PROFILE_THREE("three");

static const IpAddress IPADDRESS_ONE(AString("192.168.2.21"));
static const IpAddress IPADDRESS_TWO(AString("192.168.2.22"));
static const IpAddress IPADDRESS_THREE(AString("192.168.2.23"));

enum
{
    HANDLE_MOBILE_MIN = 20001,
    HANDLE_MOBILE_MAX = 20020,

    HANDLE_WIFI_MIN = 20101,
    HANDLE_WIFI_MAX = 20120
};

class TestImsNetworkConnection : public ImsNetworkConnection
{
public:
    inline TestImsNetworkConnection(const AString& strProfileName, IMS_SINT32 nApnType,
            const IpAddress& objIpAddr, IMS_SINT32 nSlotId = IMS_SLOT_0) :
            ImsNetworkConnection(nSlotId),
            m_nConnectionHandle(0),
            m_strProfileName(strProfileName),
            m_nApnType(nApnType),
            m_bMobileConnection(IMS_TRUE),
            m_objIpAddr(objIpAddr)
    {
    }
    ~TestImsNetworkConnection() override = default;

public:
    inline IMS_BOOL Create(IN const AString& /*strNetProfile*/) override
    {
        m_nConnectionHandle = ImsNetworkConnectionState::GetInstance()->GetAndIncrementHandle(
                m_bMobileConnection);
        return IMS_TRUE;
    }
    inline IMS_BOOL Create(IN IMS_SINT32 /*nApnType*/) override
    {
        m_nConnectionHandle = ImsNetworkConnectionState::GetInstance()->GetAndIncrementHandle(
                m_bMobileConnection);
        return IMS_TRUE;
    }
    inline void DispatchServiceMessage(IN IMS_UINTP /*nWparam*/, IN IMS_UINTP /*nLparam*/) override
    {
    }
    inline IMS_BOOL Equals(IN const IpAddress& objIpAddr) const override
    {
        return objIpAddr.Equals(m_objIpAddr);
    }
    inline IMS_CONNECTION GetHandle() const override { return m_nConnectionHandle; }
    inline const AString& GetProfileName() const override { return m_strProfileName; }
    inline IMS_SINT32 GetApnType() const override { return m_nApnType; }
    inline void SetConnectionType(IMS_BOOL bMobileConnection /* mobile or wifi network */)
    {
        m_bMobileConnection = bMobileConnection;
    }

private:
    RESULT_ENTYPE Activate(IN IMS_BOOL /*bEnableApn = IMS_FALSE*/) override { return RESULT_DONE; }
    RESULT_ENTYPE Deactivate(IN IMS_BOOL /*bDisableApn = IMS_FALSE*/) override
    {
        return RESULT_DONE;
    }
    void GetAccessNetworkInfo(OUT AccessNetworkInfo& /*objAccessNetInfo*/) override {}
    void GetLastAccessNetworkInfo(OUT AccessNetworkInfo& /*objAccessNetInfo*/,
            OUT AString& /*strTimestamp*/, OUT AString& /*strCellInfoAge*/) override
    {
    }
    IMS_BOOL GetExtraInfo(IN const AString& /*strType*/, OUT AString& /*strInfo*/) override
    {
        return IMS_TRUE;
    }
    IMS_SINT32 GetHostByName(IN const AString& /*strHostName*/,
            OUT ImsList<IpAddress>& /*objIpAddrs*/,
            IN IMS_SINT32 /*nIpVersion = 0*/ /* default-local-address-based */) override
    {
        return 0;
    }
    IMS_SINT32 GetIfaceId() const override { return 0; }
    const AString& GetIfaceName() const override { return AString::ConstNull(); }
    const IpAddress& GetLocalAddress(
            IN IMS_SINT32 /*nIpVersion = 0*/ /* configuration-based */) const override
    {
        // IpAddress objIpAddress;
        return *(new IpAddress());
    }
    const AStringArray& GetPcscfAddress(
            IN IMS_SINT32 /*nIpVersion = 0*/ /* configuration-based */) override
    {
        // AStringArray objAStringArray;
        return AStringArray::ConstNull();
    }
    STATE_ENTYPE GetState() const override { return STATE_CONNECTED; }
    IMS_BOOL IsConnected(IN IMS_SINT32 /*nCategory = IIpcan::CATEGORY_ANY*/) const override
    {
        return IMS_TRUE;
    }
    IMS_BOOL IsePDGEnabled() const override { return IMS_TRUE; }
    IMS_BOOL IsIpv6Preferred() const override { return IMS_TRUE; }
    IMS_BOOL IsMobileDataEnabled() const override { return IMS_TRUE; }
    IMS_SINT32 GetMtu() const override { return 0; }
    void SetListener(IN INetworkConnectionListener* /*piListener*/) override {}
    void SetPreferredIpVersion(
            IN IMS_SINT32 /*nPreferredIpVersion = 0*/ /* default-aos-connection-profile */) override
    {
    }
    INetworkPing* CreatePing() override { return IMS_NULL; }
    void AddReferenceListener(IN INetworkConnectionListener* /*piListener*/) override {}
    void RemoveReferenceListener(IN INetworkConnectionListener* /*piListener*/) override {}

public:
    IMS_CONNECTION m_nConnectionHandle;
    AString m_strProfileName;
    IMS_SINT32 m_nApnType;
    IMS_BOOL m_bMobileConnection;
    IpAddress m_objIpAddr;
};

class ImsNetworkConnectionStateTest : public ::testing::Test
{
public:
    inline ImsNetworkConnectionStateTest() :
            m_pImsNetworkConnectionState(IMS_NULL)
    {
    }

protected:
    virtual void SetUp() override
    {
        m_pImsNetworkConnectionState = ImsNetworkConnectionState::GetInstance();
        ASSERT_TRUE(m_pImsNetworkConnectionState != nullptr);

        // detach all the handles if any present.
        m_pImsNetworkConnectionState->DetachAll();

        // No handles attached
        EXPECT_TRUE(m_pImsNetworkConnectionState->IsEmpty());
    }
    virtual void TearDown() override {}

public:
    ImsNetworkConnectionState* m_pImsNetworkConnectionState;
};

TEST_F(ImsNetworkConnectionStateTest, LookupHandle_ConnectionHandle)
{
    TestImsNetworkConnection objTestImsNetworkConnection1(PROFILE_ONE, APN_TYPE_ONE, IPADDRESS_ONE);
    TestImsNetworkConnection objTestImsNetworkConnection2(PROFILE_TWO, APN_TYPE_TWO, IPADDRESS_TWO);

    objTestImsNetworkConnection1.Create(PROFILE_ONE);
    objTestImsNetworkConnection2.Create(APN_TYPE_TWO);

    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection1);
    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection2);

    // invalid IpAddress
    EXPECT_TRUE(m_pImsNetworkConnectionState->LookupHandle(HANDLE_MOBILE_MAX + 1) == IMS_NULL);

    // valid handle - success
    ImsNetworkConnection* pImsNetworkConnection =
            m_pImsNetworkConnectionState->LookupHandle(objTestImsNetworkConnection1.GetHandle());
    ASSERT_TRUE(pImsNetworkConnection != nullptr);
    EXPECT_EQ(APN_TYPE_ONE, objTestImsNetworkConnection1.GetApnType());

    // valid handle - success
    pImsNetworkConnection =
            m_pImsNetworkConnectionState->LookupHandle(objTestImsNetworkConnection2.GetHandle());
    ASSERT_TRUE(pImsNetworkConnection != nullptr);
    EXPECT_EQ(PROFILE_TWO, objTestImsNetworkConnection2.GetProfileName());

    m_pImsNetworkConnectionState->DetachHandle(PROFILE_ONE, IMS_SLOT_0);
    m_pImsNetworkConnectionState->DetachHandle(PROFILE_TWO, IMS_SLOT_0);

    // No handles attached
    EXPECT_TRUE(m_pImsNetworkConnectionState->IsEmpty());
}

TEST_F(ImsNetworkConnectionStateTest, LookupHandle_IpAddress)
{
    TestImsNetworkConnection objTestImsNetworkConnection1(PROFILE_ONE, APN_TYPE_ONE, IPADDRESS_ONE);
    TestImsNetworkConnection objTestImsNetworkConnection2(PROFILE_TWO, APN_TYPE_TWO, IPADDRESS_TWO);

    objTestImsNetworkConnection1.Create(PROFILE_ONE);
    objTestImsNetworkConnection2.Create(APN_TYPE_TWO);

    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection1);
    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection2);

    // invalid IpAddress
    EXPECT_TRUE(m_pImsNetworkConnectionState->LookupHandle(IPADDRESS_THREE) == IMS_NULL);

    // valid IpAddress - success
    ImsNetworkConnection* pImsNetworkConnection =
            m_pImsNetworkConnectionState->LookupHandle(IPADDRESS_ONE);
    ASSERT_TRUE(pImsNetworkConnection != nullptr);
    EXPECT_EQ(APN_TYPE_ONE, objTestImsNetworkConnection1.GetApnType());

    // valid IpAddress - success
    pImsNetworkConnection = m_pImsNetworkConnectionState->LookupHandle(IPADDRESS_TWO);
    ASSERT_TRUE(pImsNetworkConnection != nullptr);
    EXPECT_EQ(PROFILE_TWO, objTestImsNetworkConnection2.GetProfileName());

    m_pImsNetworkConnectionState->DetachHandle(PROFILE_ONE, IMS_SLOT_0);
    m_pImsNetworkConnectionState->DetachHandle(PROFILE_TWO, IMS_SLOT_0);

    // No handles attached
    EXPECT_TRUE(m_pImsNetworkConnectionState->IsEmpty());
}

TEST_F(ImsNetworkConnectionStateTest, LookupHandle_Profile_Slot)
{
    TestImsNetworkConnection objTestImsNetworkConnection1(PROFILE_ONE, APN_TYPE_ONE, IPADDRESS_ONE);
    TestImsNetworkConnection objTestImsNetworkConnection2(PROFILE_TWO, APN_TYPE_TWO, IPADDRESS_TWO);
    TestImsNetworkConnection objTestImsNetworkConnection3(
            PROFILE_ONE, APN_TYPE_ONE, IPADDRESS_ONE, IMS_SLOT_1);
    TestImsNetworkConnection objTestImsNetworkConnection4(
            PROFILE_THREE, APN_TYPE_THREE, IPADDRESS_THREE, IMS_SLOT_1);

    objTestImsNetworkConnection1.Create(PROFILE_ONE);
    objTestImsNetworkConnection2.Create(APN_TYPE_TWO);
    objTestImsNetworkConnection3.Create(PROFILE_ONE);
    objTestImsNetworkConnection4.Create(APN_TYPE_THREE);

    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection1);
    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection2);
    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection3);
    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection4);

    // invalid input - profile three not present i slot-0 and profile two not present in slot-1
    EXPECT_TRUE(m_pImsNetworkConnectionState->LookupHandle(PROFILE_THREE, IMS_SLOT_0) == IMS_NULL);
    EXPECT_TRUE(m_pImsNetworkConnectionState->LookupHandle(PROFILE_TWO, IMS_SLOT_1) == IMS_NULL);

    // valid handle - success
    ImsNetworkConnection* pImsNetworkConnection =
            m_pImsNetworkConnectionState->LookupHandle(PROFILE_TWO, IMS_SLOT_0);
    ASSERT_TRUE(pImsNetworkConnection != nullptr);
    EXPECT_EQ(APN_TYPE_TWO, pImsNetworkConnection->GetApnType());

    // valid handle - success
    pImsNetworkConnection = m_pImsNetworkConnectionState->LookupHandle(PROFILE_ONE, IMS_SLOT_1);
    ASSERT_TRUE(pImsNetworkConnection != nullptr);
    EXPECT_EQ(APN_TYPE_ONE, pImsNetworkConnection->GetApnType());

    // valid handle - success
    pImsNetworkConnection = m_pImsNetworkConnectionState->LookupHandle(PROFILE_THREE, IMS_SLOT_1);
    ASSERT_TRUE(pImsNetworkConnection != nullptr);
    EXPECT_EQ(PROFILE_THREE, pImsNetworkConnection->GetProfileName());

    m_pImsNetworkConnectionState->DetachHandle(PROFILE_ONE, IMS_SLOT_0);
    m_pImsNetworkConnectionState->DetachHandle(PROFILE_TWO, IMS_SLOT_0);
    m_pImsNetworkConnectionState->DetachHandle(PROFILE_ONE, IMS_SLOT_1);
    m_pImsNetworkConnectionState->DetachHandle(PROFILE_THREE, IMS_SLOT_1);

    // No handles attached
    EXPECT_TRUE(m_pImsNetworkConnectionState->IsEmpty());
}

TEST_F(ImsNetworkConnectionStateTest, LookupHandle_ApnType_Slot)
{
    TestImsNetworkConnection objTestImsNetworkConnection1(PROFILE_ONE, APN_TYPE_ONE, IPADDRESS_ONE);
    TestImsNetworkConnection objTestImsNetworkConnection2(PROFILE_TWO, APN_TYPE_TWO, IPADDRESS_TWO);
    TestImsNetworkConnection objTestImsNetworkConnection3(
            PROFILE_ONE, APN_TYPE_ONE, IPADDRESS_ONE, IMS_SLOT_1);
    TestImsNetworkConnection objTestImsNetworkConnection4(
            PROFILE_THREE, APN_TYPE_THREE, IPADDRESS_THREE, IMS_SLOT_1);

    // update connection handles
    objTestImsNetworkConnection1.Create(PROFILE_ONE);
    objTestImsNetworkConnection2.Create(APN_TYPE_TWO);
    objTestImsNetworkConnection3.Create(PROFILE_ONE);
    objTestImsNetworkConnection4.Create(APN_TYPE_THREE);

    // attach connections
    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection1);
    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection2);
    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection3);
    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection4);

    // invalid input - profile three not present i slot-0 and profile two not present in slot-1
    EXPECT_TRUE(m_pImsNetworkConnectionState->LookupHandle(PROFILE_THREE, IMS_SLOT_0) == IMS_NULL);
    EXPECT_TRUE(m_pImsNetworkConnectionState->LookupHandle(PROFILE_TWO, IMS_SLOT_1) == IMS_NULL);

    // valid handle - success
    ImsNetworkConnection* pImsNetworkConnection =
            m_pImsNetworkConnectionState->LookupHandle(APN_TYPE_TWO, IMS_SLOT_0);
    ASSERT_TRUE(pImsNetworkConnection != nullptr);
    EXPECT_EQ(APN_TYPE_TWO, pImsNetworkConnection->GetApnType());

    // valid handle - success
    pImsNetworkConnection = m_pImsNetworkConnectionState->LookupHandle(APN_TYPE_ONE, IMS_SLOT_1);
    ASSERT_TRUE(pImsNetworkConnection != nullptr);
    EXPECT_EQ(APN_TYPE_ONE, pImsNetworkConnection->GetApnType());

    // valid handle - success
    pImsNetworkConnection = m_pImsNetworkConnectionState->LookupHandle(APN_TYPE_THREE, IMS_SLOT_1);
    ASSERT_TRUE(pImsNetworkConnection != nullptr);
    EXPECT_EQ(PROFILE_THREE, pImsNetworkConnection->GetProfileName());

    m_pImsNetworkConnectionState->DetachHandle(PROFILE_ONE, IMS_SLOT_0);
    m_pImsNetworkConnectionState->DetachHandle(PROFILE_TWO, IMS_SLOT_0);
    m_pImsNetworkConnectionState->DetachHandle(PROFILE_ONE, IMS_SLOT_1);
    m_pImsNetworkConnectionState->DetachHandle(PROFILE_THREE, IMS_SLOT_1);

    // No handles attached
    EXPECT_TRUE(m_pImsNetworkConnectionState->IsEmpty());
}

TEST_F(ImsNetworkConnectionStateTest, IsHandlePresent)
{
    TestImsNetworkConnection objTestImsNetworkConnection1(PROFILE_ONE, APN_TYPE_ONE, IPADDRESS_ONE);
    TestImsNetworkConnection objTestImsNetworkConnection2(PROFILE_TWO, APN_TYPE_TWO, IPADDRESS_TWO);

    objTestImsNetworkConnection1.Create(PROFILE_ONE);
    objTestImsNetworkConnection2.Create(APN_TYPE_TWO);

    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection1);
    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection2);

    // invalid IpAddress
    EXPECT_FALSE(m_pImsNetworkConnectionState->IsHandlePresent(HANDLE_MOBILE_MAX + 1));

    // valid handle - success
    EXPECT_TRUE(m_pImsNetworkConnectionState->IsHandlePresent(
            objTestImsNetworkConnection1.GetHandle()));

    // valid handle - success
    EXPECT_TRUE(m_pImsNetworkConnectionState->IsHandlePresent(
            objTestImsNetworkConnection2.GetHandle()));

    m_pImsNetworkConnectionState->DetachHandle(PROFILE_ONE, IMS_SLOT_0);
    m_pImsNetworkConnectionState->DetachHandle(PROFILE_TWO, IMS_SLOT_0);

    // No handles attached
    EXPECT_TRUE(m_pImsNetworkConnectionState->IsEmpty());
}

TEST_F(ImsNetworkConnectionStateTest, GetAndIncrementHandle)
{
    TestImsNetworkConnection objTestImsNetworkConnection1(PROFILE_ONE, APN_TYPE_ONE, IPADDRESS_ONE);
    TestImsNetworkConnection objTestImsNetworkConnection2(PROFILE_TWO, APN_TYPE_TWO, IPADDRESS_TWO);

    objTestImsNetworkConnection2.SetConnectionType(CONNECTION_WIFI);

    // update connection handles - GetAndIncrementHandle()
    objTestImsNetworkConnection1.Create(PROFILE_ONE);
    objTestImsNetworkConnection2.Create(APN_TYPE_TWO);

    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection1);
    m_pImsNetworkConnectionState->AttachHandle(&objTestImsNetworkConnection2);

    // mobile conection handle should be in between HANDLE_MOBILE_MIN and HANDLE_MOBILE_MAX
    EXPECT_GE(objTestImsNetworkConnection1.GetHandle(), HANDLE_MOBILE_MIN);
    EXPECT_LE(objTestImsNetworkConnection1.GetHandle(), HANDLE_MOBILE_MAX);

    // wifi conection handle should be in between HANDLE_WIFI_MIN and HANDLE_WIFI_MAX
    EXPECT_GE(objTestImsNetworkConnection2.GetHandle(), HANDLE_WIFI_MIN);
    EXPECT_LE(objTestImsNetworkConnection2.GetHandle(), HANDLE_WIFI_MAX);

    // detach handles
    m_pImsNetworkConnectionState->DetachHandle(PROFILE_ONE, IMS_SLOT_0);
    m_pImsNetworkConnectionState->DetachHandle(PROFILE_TWO, IMS_SLOT_0);

    // No handles attached
    EXPECT_TRUE(m_pImsNetworkConnectionState->IsEmpty());

    IMS_BOOL bMobileConnection = CONNECTION_MOBILE;

    IMS_SINT32 nNumberOfHandles =
            (HANDLE_MOBILE_MAX - HANDLE_MOBILE_MIN + 1) + (HANDLE_WIFI_MAX - HANDLE_WIFI_MIN + 1);

    // fill all the mobile and wifi handles
    for (IMS_SINT32 i = 1; i <= nNumberOfHandles; i++)
    {
        AString strProfileName("profile_");
        strProfileName.Append(i - '0');

        AString strIpAddress("192.168.5.");
        strIpAddress.Append(i - '0');
        IpAddress objIpAddress(strIpAddress);

        TestImsNetworkConnection* pTestImsNetworkConnection =
                new TestImsNetworkConnection(strProfileName, i, objIpAddress);

        bMobileConnection = bMobileConnection ? CONNECTION_WIFI : CONNECTION_MOBILE;
        pTestImsNetworkConnection->SetConnectionType(bMobileConnection);
        // update connection handles - GetAndIncrementHandle()
        pTestImsNetworkConnection->Create(strProfileName);

        m_pImsNetworkConnectionState->AttachHandle(pTestImsNetworkConnection);

        EXPECT_TRUE(m_pImsNetworkConnectionState->IsHandlePresent(
                pTestImsNetworkConnection->GetHandle()));
    }

    TestImsNetworkConnection objMobileConnection(PROFILE_ONE, 100, IPADDRESS_ONE);
    // update connection handles - GetAndIncrementHandle()
    objMobileConnection.Create(PROFILE_ONE);

    // mobile handle should be 0 as all handles filled in above loop.
    EXPECT_EQ(objMobileConnection.GetHandle(), 0);
    EXPECT_FALSE(m_pImsNetworkConnectionState->IsHandlePresent(objMobileConnection.GetHandle()));

    objMobileConnection.SetConnectionType(CONNECTION_WIFI);
    objMobileConnection.Create(PROFILE_ONE);

    // wifi handle should be 0 as all handles filled in above loop.
    EXPECT_EQ(objMobileConnection.GetHandle(), 0);
    EXPECT_FALSE(m_pImsNetworkConnectionState->IsHandlePresent(objMobileConnection.GetHandle()));

    // detach all the handles
    m_pImsNetworkConnectionState->DetachAll();

    // No handles attached
    EXPECT_TRUE(m_pImsNetworkConnectionState->IsEmpty());
}

}  // namespace android
