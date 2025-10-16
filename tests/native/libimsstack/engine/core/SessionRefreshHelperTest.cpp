/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "PlatformContext.h"
#include "TestConfigService.h"

#include "CoreService.h"
#include "MockISipMessage.h"
#include "MockISipServerConnection.h"
#include "SessionRefreshHelper.h"
#include "TestCoreBase.h"
#include "util/MockIRefreshable.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Unused;

namespace android
{

class SessionRefreshHelperTest : public TestCoreBase
{
public:
    inline SessionRefreshHelperTest() :
            TestCoreBase(),
            m_pConfigService(IMS_NULL),
            m_pSessionRefreshHelper(IMS_NULL)
    {
    }
    inline ~SessionRefreshHelperTest() override { Clear(); }

protected:
    virtual void SetUp() override
    {
        m_pConfigService = new TestConfigService();
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, m_pConfigService);

        ON_CALL(m_pConfigService->GetMockCarrierConfig(), AddListener(_)).WillByDefault(Return());
        ON_CALL(m_pConfigService->GetMockCarrierConfig(), RemoveListener(_))
                .WillByDefault(Return());
        ON_CALL(m_pConfigService->GetMockCarrierConfig(), GetBoolean(_, _))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_pConfigService->GetMockCarrierConfig(), GetInt(_, _))
                .WillByDefault(Invoke(
                        [](Unused, IMS_SINT32 nDefaultValue)
                        {
                            return nDefaultValue;
                        }));

        const SipConfigV* pSipConfig = GetCoreService()->GetSipConfigV();
        const_cast<SipConfigV*>(pSipConfig)->Refresh();

        m_pSessionRefreshHelper = new SessionRefreshHelper(GetCoreService(), &m_objRefreshable);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
        Clear();
    }

    void Clear()
    {
        if (m_pSessionRefreshHelper != IMS_NULL)
        {
            delete m_pSessionRefreshHelper;
            m_pSessionRefreshHelper = IMS_NULL;
        }

        if (m_pConfigService != IMS_NULL)
        {
            delete m_pConfigService;
            m_pConfigService = IMS_NULL;
        }
    }

protected:
    MockIRefreshable m_objRefreshable;
    TestConfigService* m_pConfigService;
    SessionRefreshHelper* m_pSessionRefreshHelper;
};

TEST_F(SessionRefreshHelperTest, GetRefreshMethod)
{
    EXPECT_EQ(m_pSessionRefreshHelper->GetRefreshMethod(), SipMethod::INVITE);

    const SipMethod objMethod(SipMethod::INVITE);
    MockISipMessage& objSipMsg = GetSipMsg();
    MockISipClientConnection& objScc = GetScc();
    ON_CALL(objScc, GetMessage()).WillByDefault(Return(&objSipMsg));
    ON_CALL(objSipMsg, GetType()).WillByDefault(Return(ISipMessage::TYPE_RESPONSE));
    ON_CALL(objSipMsg, GetMethod()).WillByDefault(ReturnRef(objMethod));
    ON_CALL(objSipMsg, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_200));

    ImsList<AString> objAllowHeader;
    objAllowHeader.Append(SipMethod::ToName(SipMethod::UPDATE));
    ON_CALL(objSipMsg, GetHeaders(Eq(ISipHeader::ALLOW), _)).WillByDefault(Return(objAllowHeader));

    // Allow header contains "UPDATE" method.
    m_pSessionRefreshHelper->UpdateOnMessageReceived(&objScc);

    EXPECT_EQ(m_pSessionRefreshHelper->GetRefreshMethod(), SipMethod::UPDATE);

    MockISipServerConnection& objSsc = GetSsc();
    ON_CALL(objSsc, GetMessage()).WillByDefault(Return(&objSipMsg));
    ON_CALL(objSipMsg, GetType()).WillByDefault(Return(ISipMessage::TYPE_REQUEST));

    objAllowHeader.Clear();
    ON_CALL(objSipMsg, GetHeaders(Eq(ISipHeader::ALLOW), _)).WillByDefault(Return(objAllowHeader));

    // Allow header doesn't contain "UPDATE" method.
    m_pSessionRefreshHelper->UpdateOnMessageReceived(&objSsc);

    EXPECT_EQ(m_pSessionRefreshHelper->GetRefreshMethod(), SipMethod::UPDATE);
}

}  // namespace android
