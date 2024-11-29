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
#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockISipMessage.h"
#include "PlatformContext.h"
#include "SipStatusCode.h"
#include "TestConfigService.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/termination/EarlyUpdateErrorHandler.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MockIPassiveTimerHolder.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class EarlyUpdateErrorHandlerTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockISipMessage objSipMessage;
    MockIMessage objMessage;
    CallInfo objCallInfo;
    MockIMtcCallManager objCallManager;
    TestConfigService* pConfigService;
    MockIMessageUtils objMessageUtils;
    ImsList<IMtcCall*> objCalls;
    MockIMtcService objMtcService;
    MockIMtcAosConnector objAosConnector;
    MockIPassiveTimerHolder objPassiveTimer;

protected:
    virtual void SetUp() override
    {
        pConfigService = new TestConfigService();
        pConfigService->SetCarrierConfig(&(pConfigService->GetMockCarrierConfig()));
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, pConfigService);

        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));
        ON_CALL(objCallManager, GetCallsByState(_)).WillByDefault(Return(objCalls));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objMtcService));
        ON_CALL(objMtcService, GetAosConnector).WillByDefault(Return(&objAosConnector));
        ON_CALL(objContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimer));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);

        delete pConfigService;
    }
};

TEST_F(EarlyUpdateErrorHandlerTest, HandleNullMessageReturnsTimeout)
{
    EXPECT_EQ(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE),
            EarlyUpdateErrorHandler(objContext).Handle(IMS_NULL));
}

TEST_F(EarlyUpdateErrorHandlerTest, HandleMessageWithInvalidStatusCodeReturnsTimeout)
{
    ON_CALL(objSipMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_INVALID));

    EXPECT_EQ(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE),
            EarlyUpdateErrorHandler(objContext).Handle(&objMessage));
}

TEST_F(EarlyUpdateErrorHandlerTest,
        Handle3xx4xx5xx6xxExcept491MessageAnd503MessageReturnsInternalErrorWithCode)
{
    for (IMS_SINT32 nStatusCode = SipStatusCode::SC_300; nStatusCode <= SipStatusCode::SC_699;
            nStatusCode++)
    {
        if (nStatusCode == SipStatusCode::SC_491 || nStatusCode == SipStatusCode::SC_503)
        {
            continue;
        }
        ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

        EXPECT_EQ(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR, nStatusCode),
                EarlyUpdateErrorHandler(objContext).Handle(&objMessage));
    }
}

TEST_F(EarlyUpdateErrorHandlerTest, Handle491MessageReturnsRequestPendingError)
{
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_491));
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    EXPECT_EQ(CODE_SIP_REQUEST_PENDING,
            EarlyUpdateErrorHandler(objContext).Handle(&objMessage).nCode);
}

TEST_F(EarlyUpdateErrorHandlerTest, Handle503MessageWithExistedCallReturnsInternalError)
{
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_503));
    MockIMtcCall objCall;
    objCalls.Append(&objCall);
    ON_CALL(objCallManager, GetCallsByState(_)).WillByDefault(Return(objCalls));

    EXPECT_EQ(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR, SipStatusCode::SC_503),
            EarlyUpdateErrorHandler(objContext).Handle(&objMessage));
}

TEST_F(EarlyUpdateErrorHandlerTest, Handle503MessageReturnsInternalRedial)
{
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_503));
    IMS_SINT32 nAnyRetryAfter = 10;
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(pConfigService->GetMockCarrierConfig(),
            GetInt(ConfigIms::KEY_SIP_TIMER_F_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter - 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objContext.GetSlotId());

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(nAnyRetryAfter)).Times(1);
    EXPECT_EQ(CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF),
            EarlyUpdateErrorHandler(objContext).Handle(&objMessage));
}

TEST_F(EarlyUpdateErrorHandlerTest, Handle503MessageWithNoAosConnectorReturnsInternalError)
{
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_503));
    IMS_SINT32 nAnyRetryAfter = 10;
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(pConfigService->GetMockCarrierConfig(),
            GetInt(ConfigIms::KEY_SIP_TIMER_F_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter - 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objContext.GetSlotId());
    ON_CALL(objMtcService, GetAosConnector).WillByDefault(Return(nullptr));

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(nAnyRetryAfter)).Times(0);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR),
            EarlyUpdateErrorHandler(objContext).Handle(&objMessage));
}

TEST_F(EarlyUpdateErrorHandlerTest, Handle503MessageWithCombinedAttachReturnsCsRetryRequired)
{
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_503));
    IMS_SINT32 nAnyRetryAfter = 10;
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(pConfigService->GetMockCarrierConfig(),
            GetInt(ConfigIms::KEY_SIP_TIMER_F_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter + 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objContext.GetSlotId());
    ON_CALL(objMtcService, IsCsfbAvailable).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(
            objPassiveTimer, AddTimer(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, _, _))
            .Times(1);

    EXPECT_EQ(
            CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL),
            EarlyUpdateErrorHandler(objContext).Handle(&objMessage));
}

TEST_F(EarlyUpdateErrorHandlerTest, Handle503MessageWithNotCombinedAttachReturnsInternalRedial)
{
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_503));
    IMS_SINT32 nAnyRetryAfter = 10;
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(pConfigService->GetMockCarrierConfig(),
            GetInt(ConfigIms::KEY_SIP_TIMER_F_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter + 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objContext.GetSlotId());
    ON_CALL(objMtcService, IsCsfbAvailable).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMtcService, GetAosConnector).Times(0);
    EXPECT_CALL(
            objPassiveTimer, AddTimer(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, _, _))
            .Times(0);

    AString strRetryAfter;
    strRetryAfter.SetNumber(nAnyRetryAfter * 1000);
    EXPECT_EQ(CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER, strRetryAfter),
            EarlyUpdateErrorHandler(objContext).Handle(&objMessage));
}
