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
#include "MockIIsimListener.h"
#include "MockISystem.h"
#include "MockIThread.h"
#include "OsParcel.h"
#include "PlatformContext.h"
#include "TestPhoneInfoService.h"
#include "TestThreadService.h"
#include "device/OsIsim.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;

namespace android
{

class OsIsimTest : public ::testing::Test
{
public:
    MockIDigestAkaListener m_objMockDigestAkaListener;
    MockIIsimListener m_objMockIsimListener;
    MockISystem m_objMockSystem;
    MockIThread m_objMockThread;

    IDigestAka* m_pDigestAka;
    ImsIsim* m_pIsim;
    ISystem* m_piDefaultSystem;
    ISystemListener* m_piSystemListener;
    OsIsim* m_pOsIsim;

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

        m_pOsIsim = new OsIsim(IMS_SLOT_0);
        ASSERT_TRUE(m_pOsIsim != nullptr);

        m_pIsim = static_cast<ImsIsim*>(m_pOsIsim);
        m_pDigestAka = m_pIsim->CreateDigestAka();
        ASSERT_TRUE(m_pDigestAka != nullptr);
        m_pDigestAka->SetListener(&m_objMockDigestAkaListener);
        m_objPhoneInfoService.SetIsim(m_pIsim);

        m_piSystemListener = static_cast<ISystemListener*>(m_pOsIsim);
        m_pIsim->AddListener(&m_objMockIsimListener);

        AString strOut("NOT_READY");
        EXPECT_CALL(m_objMockSystem, GetIsimState(_)).Times(1).WillRepeatedly(Return(strOut));

        m_pOsIsim->Init();
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetSystem(m_piDefaultSystem);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);

        if (m_pOsIsim != IMS_NULL)
        {
            if (m_pDigestAka != IMS_NULL)
            {
                m_pOsIsim->DestroyDigestAka(static_cast<OsIsimDigestAka*>(m_pDigestAka));
                EXPECT_EQ(IMS_FALSE,
                        m_pOsIsim->IsDigestAkaPresent(static_cast<OsIsimDigestAka*>(m_pDigestAka)));
            }
            m_pOsIsim->Release();
            EXPECT_EQ(m_pOsIsim->GetState(), IIsim::STATE_IDLE);

            m_pIsim->RemoveListener(&m_objMockIsimListener);
            delete m_pOsIsim;
            m_pOsIsim = IMS_NULL;
        }
    }
};

TEST_F(OsIsimTest, Init)
{
    m_pOsIsim->Release();

    EXPECT_CALL(m_objMockSystem, GetIsimState(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return("LOADED"));
    EXPECT_CALL(m_objMockThread, PostMessageI(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pOsIsim->Init();
}

TEST_F(OsIsimTest, DigestAka)
{
    EXPECT_EQ(IMS_TRUE, m_pOsIsim->IsDigestAkaPresent(static_cast<OsIsimDigestAka*>(m_pDigestAka)));
    m_pOsIsim->DestroyDigestAka(IMS_NULL);
    EXPECT_EQ(IMS_FALSE, m_pOsIsim->IsDigestAkaPresent(IMS_NULL));
}

TEST_F(OsIsimTest, GetAuthResponse)
{
    AString strNonce("ZAJ46jOvJRnliuPOQ/9GDH3dyEeU1QAA8Op1t+942Cw=");
    const IMS_BYTE* pbyNonce = reinterpret_cast<const IMS_BYTE*>(strNonce.GetStr());
    ByteArray objChallenge;

    objChallenge.Append(0x10);
    objChallenge.Append(&pbyNonce[0], 16);
    objChallenge.Append(0x10);
    objChallenge.Append(&pbyNonce[16], 16);

    EXPECT_CALL(m_objMockSystem, RequestIsimAuthentication(_, _, _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_EQ(m_pDigestAka->GetAuthResponse(objChallenge), IMS_SUCCESS);

    EXPECT_CALL(m_objMockSystem, RequestIsimAuthentication(_, _, _))
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));
    EXPECT_EQ(m_pDigestAka->GetAuthResponse(objChallenge), IMS_FAILURE);
}

TEST_F(OsIsimTest, DispatchServiceMessage_IsimAuth)
{
    android::Parcel in;

    in.writeString16(String16("2wjNM+AkzPlkohAdvR2RA3K9zcXw2uA6cnYvELrR4yl9RARQBKH4Zfu2w94A"));
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
            OsIsim::NOTIFICATION_ISIM_AUTH, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockDigestAkaListener, DigestAka_OnResponse(_, _, _)).Times(1);
    m_pIsim->DispatchServiceMessage(wParam, lParam);

    EXPECT_CALL(m_objMockDigestAkaListener, DigestAka_OnResponse(_, _, _)).Times(0);
    m_pIsim->DispatchServiceMessage(0, 0);

    EXPECT_CALL(m_objMockThread, PostMessageI(_, _, _)).Times(0);
    m_piSystemListener->System_NotifyEvent(7843, 0, 0);
}

TEST_F(OsIsimTest, DispatchServiceMessage_IsimAuthFailure)
{
    IMS_UINTP wParam;
    IMS_UINTP lParam;
    EXPECT_CALL(m_objMockThread, PostMessageI(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Invoke(
                    [&](Unused, IMS_UINTP nWparam, IN IMS_UINTP nLparam)
                    {
                        wParam = nWparam;
                        lParam = nLparam;
                        return IMS_TRUE;
                    }));

    android::Parcel in;

    in.writeString16(String16("response"));
    in.writeInt64(reinterpret_cast<IMS_SINTP>(m_pDigestAka));
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_AUTH, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockDigestAkaListener, DigestAka_OnMacFailed()).Times(1);
    m_pIsim->DispatchServiceMessage(wParam, lParam);

    in.setDataPosition(0);
    in.writeString16(String16(""));
    in.writeInt64(reinterpret_cast<IMS_SINTP>(m_pDigestAka));
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_AUTH, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockDigestAkaListener, DigestAka_OnMacFailed()).Times(1);
    m_pIsim->DispatchServiceMessage(wParam, lParam);

    in.setDataPosition(0);
    in.writeString16(
            String16("3DSh9+ShECcRx3jKBdoU3jzhkmmsI8IQEq5VXwAvk8/XCkpkiPQM6Ag8iTnRK3Jm8Q=="));
    in.writeInt64(reinterpret_cast<IMS_SINTP>(m_pDigestAka));
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_AUTH, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockDigestAkaListener, DigestAka_OnAutsFailed(_)).Times(1);
    m_pIsim->DispatchServiceMessage(wParam, lParam);
}

TEST_F(OsIsimTest, DispatchServiceMessage_IsimAuthWithNullListener)
{
    m_pDigestAka->SetListener(IMS_NULL);
    IMS_UINTP wParam;
    IMS_UINTP lParam;
    EXPECT_CALL(m_objMockThread, PostMessageI(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Invoke(
                    [&](Unused, IMS_UINTP nWparam, IN IMS_UINTP nLparam)
                    {
                        wParam = nWparam;
                        lParam = nLparam;
                        return IMS_TRUE;
                    }));

    android::Parcel in;

    in.writeString16(String16("response"));
    in.writeInt64(reinterpret_cast<IMS_SINTP>(m_pDigestAka));
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_AUTH, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockDigestAkaListener, DigestAka_OnMacFailed()).Times(0);
    m_pIsim->DispatchServiceMessage(wParam, lParam);

    in.setDataPosition(0);
    in.writeString16(String16(""));
    in.writeInt64(reinterpret_cast<IMS_SINTP>(m_pDigestAka));
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_AUTH, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockDigestAkaListener, DigestAka_OnMacFailed()).Times(0);
    m_pIsim->DispatchServiceMessage(wParam, lParam);

    in.setDataPosition(0);
    in.writeString16(
            String16("3DSh9+ShECcRx3jKBdoU3jzhkmmsI8IQEq5VXwAvk8/XCkpkiPQM6Ag8iTnRK3Jm8Q=="));
    in.writeInt64(reinterpret_cast<IMS_SINTP>(m_pDigestAka));
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_AUTH, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockDigestAkaListener, DigestAka_OnAutsFailed(_)).Times(0);
    m_pIsim->DispatchServiceMessage(wParam, lParam);

    in.setDataPosition(0);
    in.writeString16(String16("2wjNM+AkzPlkohAdvR2RA3K9zcXw2uA6cnYvELrR4yl9RARQBKH4Zfu2w94A"));
    in.writeInt64(reinterpret_cast<IMS_SINTP>(m_pDigestAka));
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_AUTH, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockDigestAkaListener, DigestAka_OnResponse(_, _, _)).Times(0);
    m_pIsim->DispatchServiceMessage(wParam, lParam);
}

TEST_F(OsIsimTest, DispatchServiceMessage_IsimStateChange)
{
    android::Parcel in;

    in.writeString16(String16("LOADED"));
    in.setDataPosition(0);

    IMS_UINTP wParam;
    IMS_UINTP lParam;
    EXPECT_CALL(m_objMockThread, PostMessageI(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Invoke(
                    [&](Unused, IMS_UINTP nWparam, IN IMS_UINTP nLparam)
                    {
                        wParam = nWparam;
                        lParam = nLparam;
                        return IMS_TRUE;
                    }));
    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_STATE_CHANGED, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockIsimListener, Isim_OnStateChanged(_)).Times(1);
    m_pIsim->DispatchServiceMessage(wParam, lParam);

    in.setDataPosition(0);
    in.writeString16(String16("REFRESH_STARTED"));
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_STATE_CHANGED, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockIsimListener, Isim_OnStateChanged(_)).Times(1);
    m_pIsim->DispatchServiceMessage(wParam, lParam);

    in.setDataPosition(0);
    in.writeString16(String16("LOADED"));
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_STATE_CHANGED, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockIsimListener, Isim_OnStateChanged(_)).Times(1);
    m_pIsim->DispatchServiceMessage(wParam, lParam);

    in.setDataPosition(0);
    in.writeString16(String16("REFRESH_STARTED"));
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_STATE_CHANGED, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockIsimListener, Isim_OnStateChanged(_)).Times(1);
    m_pIsim->DispatchServiceMessage(wParam, lParam);

    in.setDataPosition(0);
    in.writeString16(String16("REFRESH_COMPLETED"));
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_STATE_CHANGED, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockIsimListener, Isim_OnStateChanged(_)).Times(1);
    m_pIsim->DispatchServiceMessage(wParam, lParam);

    in.setDataPosition(0);
    in.writeString16(String16("NOT_READY"));
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_STATE_CHANGED, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockIsimListener, Isim_OnStateChanged(_)).Times(1);
    m_pIsim->DispatchServiceMessage(wParam, lParam);

    in.setDataPosition(0);
    in.writeString16(String16("SIM_REMOVED"));
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_STATE_CHANGED, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockIsimListener, Isim_OnStateChanged(_)).Times(1);
    m_pIsim->DispatchServiceMessage(wParam, lParam);
    EXPECT_EQ(m_pOsIsim->IsLoadCompleted(), IMS_FALSE);

    in.setDataPosition(0);
    in.writeString16(String16("NOT_PRESENT"));
    in.setDataPosition(0);

    m_piSystemListener->System_NotifyEvent(
            OsIsim::NOTIFICATION_ISIM_STATE_CHANGED, 0, reinterpret_cast<IMS_UINTP>(&in));

    EXPECT_CALL(m_objMockIsimListener, Isim_OnStateChanged(_)).Times(1);
    m_pIsim->DispatchServiceMessage(wParam, lParam);
}

}  // namespace android
