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

#include "ImsEventDef.h"
#include "ImsTypeDef.h"
#include "MockIDigestAkaListener.h"
#include "MockISystem.h"
#include "MockIThread.h"
#include "OsParcel.h"
#include "PlatformContext.h"
#include "TestPhoneInfoService.h"
#include "TestThreadService.h"
#include "device/OsUsim.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;

namespace android
{

class OsUsimTest : public ::testing::Test
{
public:
    MockIDigestAkaListener m_objMockDigestAkaListener;
    MockISystem m_objMockSystem;
    MockIThread m_objMockThread;

    IDigestAka* m_pDigestAka;
    ImsUsim* m_pIUsim;
    ISystem* m_piDefaultSystem;
    ISystemListener* m_piSystemListener;
    OsUsim* m_pOsUsim;

    TestPhoneInfoService m_objPhoneInfoService;
    TestThreadService m_objThreadService;

protected:
    virtual void SetUp() override
    {
        m_objThreadService.SetThread(&m_objMockThread);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);

        EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
        m_piDefaultSystem = PlatformContext::GetInstance()->SetSystem(&m_objMockSystem);

        m_pOsUsim = new OsUsim(IMS_SLOT_0);
        ASSERT_TRUE(m_pOsUsim != nullptr);

        m_pIUsim = static_cast<ImsUsim*>(m_pOsUsim);
        m_pDigestAka = m_pIUsim->CreateDigestAka();
        ASSERT_TRUE(m_pDigestAka != nullptr);
        m_pDigestAka->SetListener(&m_objMockDigestAkaListener);

        m_piSystemListener = static_cast<ISystemListener*>(m_pOsUsim);
        m_objPhoneInfoService.SetUsim(m_pIUsim);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetSystem(m_piDefaultSystem);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);

        if (m_pOsUsim != IMS_NULL)
        {
            if (m_pDigestAka != IMS_NULL)
            {
                m_pOsUsim->DestroyDigestAka(static_cast<OsUsimDigestAka*>(m_pDigestAka));
            }
            EXPECT_EQ(IMS_FALSE,
                    m_pOsUsim->IsDigestAkaPresent(static_cast<OsUsimDigestAka*>(m_pDigestAka)));
            delete m_pOsUsim;
            m_pOsUsim = IMS_NULL;
        }
    }
};

TEST_F(OsUsimTest, DigestAka)
{
    EXPECT_EQ(IMS_TRUE, m_pOsUsim->IsDigestAkaPresent(static_cast<OsUsimDigestAka*>(m_pDigestAka)));
    EXPECT_EQ(IMS_FALSE, m_pOsUsim->IsDigestAkaPresent(IMS_NULL));
    m_pOsUsim->DestroyDigestAka(IMS_NULL);
}

TEST_F(OsUsimTest, GetAuthResponse)
{
    ByteArray objChallenge;
    AString strNonce("ZAJ46jOvJRnliuPOQ/9GDH3dyEeU1QAA8Op1t+942Cw=");
    const IMS_BYTE* pbyNonce = reinterpret_cast<const IMS_BYTE*>(strNonce.GetStr());

    objChallenge.Append(0x10);
    objChallenge.Append(&pbyNonce[0], 16);
    objChallenge.Append(0x10);
    objChallenge.Append(&pbyNonce[16], 16);

    EXPECT_CALL(m_objMockSystem, RequestUsimAuthentication(_, _, _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_EQ(m_pDigestAka->GetAuthResponse(objChallenge), IMS_SUCCESS);

    EXPECT_CALL(m_objMockSystem, RequestUsimAuthentication(_, _, _))
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));
    EXPECT_EQ(m_pDigestAka->GetAuthResponse(objChallenge), IMS_FAILURE);
}

TEST_F(OsUsimTest, DispatchServiceMessage_WithResponse)
{
    android::Parcel in;
    in.writeString16(
            String16("2wSh9+ShECcRx3jKBdoU3jzhkmmsI8IQEq5VXwAvk8/XCkpkiPQM6Ag8iTnRK3Jm8Q=="));
    in.writeInt64(reinterpret_cast<IMS_SINTP>(m_pDigestAka));
    in.setDataPosition(0);

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
    m_piSystemListener->System_NotifyEvent(
            OsUsim::NOTIFICATION_USIM_AUTH, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockDigestAkaListener, DigestAka_OnResponse(_, _, _)).Times(1);
    m_pIUsim->DispatchServiceMessage(wParam, lParam);

    EXPECT_CALL(m_objMockDigestAkaListener, DigestAka_OnResponse(_, _, _)).Times(0);
    m_pIUsim->DispatchServiceMessage(0, IMS_NULL);

    EXPECT_CALL(m_objMockThread, PostMessageI(_, _, _)).Times(0);
    m_piSystemListener->System_NotifyEvent(OsUsim::NOTIFICATION_USIM_AUTH, 0, IMS_NULL);
}

TEST_F(OsUsimTest, DispatchServiceMessage_WithFailure)
{
    android::Parcel in;

    in.writeString16(String16("response"));
    in.writeInt64(reinterpret_cast<IMS_SINTP>(m_pDigestAka));
    in.setDataPosition(0);

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
    m_piSystemListener->System_NotifyEvent(
            OsUsim::NOTIFICATION_USIM_AUTH, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockDigestAkaListener, DigestAka_OnMacFailed()).Times(1);
    m_pIUsim->DispatchServiceMessage(wParam, lParam);
}

TEST_F(OsUsimTest, DispatchServiceMessage_WithNullResponse)
{
    android::Parcel in;

    in.writeString16(String16(""));
    in.writeInt64(reinterpret_cast<IMS_SINTP>(m_pDigestAka));
    in.setDataPosition(0);

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
    m_piSystemListener->System_NotifyEvent(
            OsUsim::NOTIFICATION_USIM_AUTH, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockDigestAkaListener, DigestAka_OnMacFailed()).Times(1);
    m_pIUsim->DispatchServiceMessage(wParam, lParam);
}

TEST_F(OsUsimTest, DispatchServiceMessage_WithAuthFailed)
{
    android::Parcel in;

    in.writeString16(
            String16("3DSh9+ShECcRx3jKBdoU3jzhkmmsI8IQEq5VXwAvk8/XCkpkiPQM6Ag8iTnRK3Jm8Q=="));
    in.writeInt64(reinterpret_cast<IMS_SINTP>(m_pDigestAka));
    in.setDataPosition(0);

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
    m_piSystemListener->System_NotifyEvent(
            OsUsim::NOTIFICATION_USIM_AUTH, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockDigestAkaListener, DigestAka_OnAutsFailed(_)).Times(1);
    m_pIUsim->DispatchServiceMessage(wParam, lParam);
}

}  // namespace android
