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

#include "CarrierConfig.h"
#include "Engine.h"
#include "IConfiguration.h"
#include "ImsTypeDef.h"
#include "MockIMtcService.h"
#include "PlatformContext.h"
#include "SipMethod.h"
#include "TestConfigService.h"
#include "call/MockIMtcCallContext.h"
#include "call/termination/UpdateErrorHandler.h"
#include "core/MockIMessage.h"
#include "helper/IPassiveTimerHolder.h"
#include "helper/MockIPassiveTimerHolder.h"
#include "helper/MockIMtcAosConnector.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/SipStatusCode.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>
#include <array>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class UpdateErrorHandlerTest : public ::testing::Test
{
public:
    MockISipMessage objSipMessage;
    MockIMessage objMessage;
    MockIMtcCallContext objContext;
    CallInfo objCallInfo;
    UpdateErrorHandler* pHandler;
    MockIMtcAosConnector objAosConnector;
    MockIMessageUtils objMessageUtils;
    MockIMtcService objMtcService;
    TestConfigService* m_pConfigService;
    MockIPassiveTimerHolder objPassiveTimerHolder;

protected:
    virtual void SetUp() override
    {
        m_pConfigService = new TestConfigService();
        m_pConfigService->SetCarrierConfig(&(m_pConfigService->GetMockCarrierConfig()));
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, m_pConfigService);

        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));

        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objMtcService));
        ON_CALL(objContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimerHolder));
        ON_CALL(objMtcService, GetAosConnector).WillByDefault(Return(&objAosConnector));

        pHandler = new UpdateErrorHandler(objContext);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
        delete m_pConfigService;
    }
};

TEST_F(UpdateErrorHandlerTest, HandleNullMessageReturnsServerError)
{
    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVER_ERROR), pHandler->Handle(IMS_NULL));
}

TEST_F(UpdateErrorHandlerTest, Handle3xxMessageReturnsServerError)
{
    for (IMS_SINT32 nStatusCode = SipStatusCode::SC_300; nStatusCode < SipStatusCode::SC_400;
            nStatusCode++)
    {
        ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

        EXPECT_EQ(
                CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode), pHandler->Handle(&objMessage));
    }
}

TEST_F(UpdateErrorHandlerTest, Handle4xxMessageReturnsTerminatedByRemote)
{
    std::array<IMS_SINT32, 11> objStatusCodes = {
            SipStatusCode::SC_404,
            SipStatusCode::SC_405,
            SipStatusCode::SC_410,
            SipStatusCode::SC_416,
            SipStatusCode::SC_480,
            SipStatusCode::SC_481,
            SipStatusCode::SC_482,
            SipStatusCode::SC_483,
            SipStatusCode::SC_484,
            SipStatusCode::SC_485,
            SipStatusCode::SC_489,
    };

    for (IMS_SINT32 nStatusCode : objStatusCodes)
    {
        ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

        EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nStatusCode),
                pHandler->Handle(&objMessage));
    }
}

TEST_F(UpdateErrorHandlerTest, Handle400MessageReturnsServerError)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_400;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode), pHandler->Handle(&objMessage));
}

TEST_F(UpdateErrorHandlerTest, Handle491MessageReturnsRequestPendingForMo)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_491;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

    objCallInfo.ePeerType = PeerType::MO;

    EXPECT_EQ(CODE_SIP_REQUEST_PENDING, pHandler->Handle(&objMessage).nCode);
}

TEST_F(UpdateErrorHandlerTest, Handle491MessageReturnsRequestPendingForMt)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_491;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

    objCallInfo.ePeerType = PeerType::MT;

    EXPECT_EQ(CODE_SIP_REQUEST_PENDING, pHandler->Handle(&objMessage).nCode);
}

TEST_F(UpdateErrorHandlerTest, Handle5xxMessageReturnsTerminatedByRemote)
{
    std::array<IMS_SINT32, 2> objStatusCodes = {
            SipStatusCode::SC_501,
            SipStatusCode::SC_502,
    };

    for (IMS_SINT32 nStatusCode : objStatusCodes)
    {
        ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

        EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nStatusCode),
                pHandler->Handle(&objMessage));
    }
}

TEST_F(UpdateErrorHandlerTest, Handle500MessageReturnsServerError)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_500;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode), pHandler->Handle(&objMessage));
}

TEST_F(UpdateErrorHandlerTest,
        Handle503ResponseWithoutRetryAfterReturnsServiceUnavailableWithCallingAos)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_503;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
    SipMethod objMethod(SipMethod::INVITE);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(-1));

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(0)).Times(1);
    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVICE_UNAVAILABLE, nStatusCode),
            pHandler->Handle(&objMessage));
}

TEST_F(UpdateErrorHandlerTest,
        Handle503ResponseWithRetryAfterToInviteRequestReturnsServiceUnavailableWithCallingAos)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_503;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
    SipMethod objMethod(SipMethod::INVITE);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    IMS_SINT32 nAnyRetryAfter = 10;
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(m_pConfigService->GetMockCarrierConfig(),
            GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_B_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter - 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objContext.GetSlotId());

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(nAnyRetryAfter)).Times(1);
    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVICE_UNAVAILABLE, nStatusCode),
            pHandler->Handle(&objMessage));
}

TEST_F(UpdateErrorHandlerTest,
        Handle503ResponseWithRetryAfterToUpdateRequestReturnsServiceUnavailableWithCallingAos)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_503;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
    SipMethod objMethod(SipMethod::UPDATE);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    IMS_SINT32 nAnyRetryAfter = 10;
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(m_pConfigService->GetMockCarrierConfig(),
            GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_F_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter - 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objContext.GetSlotId());

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(nAnyRetryAfter)).Times(1);
    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVICE_UNAVAILABLE, nStatusCode),
            pHandler->Handle(&objMessage));
}

TEST_F(UpdateErrorHandlerTest,
        Handle503ResponseWithRetryAfterReturnsServiceUnavailableWithOutCallingAos)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_503;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
    SipMethod objMethod(SipMethod::UPDATE);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    IMS_SINT32 nAnyRetryAfter = 10;
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(m_pConfigService->GetMockCarrierConfig(),
            GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_F_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter + 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objContext.GetSlotId());

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(_)).Times(0);
    EXPECT_CALL(objPassiveTimerHolder,
            AddTimer(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, nAnyRetryAfter * 1000,
                    IMS_FALSE))
            .Times(1);
    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode), pHandler->Handle(&objMessage));
}

TEST_F(UpdateErrorHandlerTest, Handle6xxMessageReturnsTerminatedByRemote)
{
    std::array<IMS_SINT32, 1> objStatusCodes = {
            SipStatusCode::SC_604,
    };

    for (IMS_SINT32 nStatusCode : objStatusCodes)
    {
        ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

        EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nStatusCode),
                pHandler->Handle(&objMessage));
    }
}

TEST_F(UpdateErrorHandlerTest, Handle600MessageReturnsServerError)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_600;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode), pHandler->Handle(&objMessage));
}
