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
#include "ISipHeader.h"
#include "ImsTypeDef.h"
#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockISipMessage.h"
#include "PlatformContext.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "TestConfigService.h"
#include "call/MockIMtcCallContext.h"
#include "call/termination/DefaultStatusCodeAndReasonCodeSets.h"
#include "call/termination/UpdateErrorHandler.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/IPassiveTimerHolder.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MockIPassiveTimerHolder.h"
#include "utility/MockIMessageUtils.h"
#include <array>
#include <gtest/gtest.h>

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
    MockMtcConfigurationProxy objConfigurationProxy;
    ImsVector<AString> objActionSets;

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

        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));

        pHandler = new UpdateErrorHandler(objContext);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
        delete m_pConfigService;
    }

    void SetActionConfigs(IN IMS_SINT32 nStatusCode, std::initializer_list<IMS_SINT32> objActions)
    {
        AString strActionSet;
        strActionSet.SetNumber(nStatusCode);
        strActionSet += ":";

        bool bFirst = true;
        for (IMS_SINT32 nAction : objActions)
        {
            if (!bFirst)
            {
                strActionSet += ",";
            }
            AString strAction;
            strAction.SetNumber(nAction);
            strActionSet += strAction;
            bFirst = false;
        }

        objActionSets.Add(strActionSet);
        ON_CALL(objConfigurationProxy,
                GetStringArray(ConfigVoice::KEY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY))
                .WillByDefault(Return(objActionSets));
    }
};

TEST_F(UpdateErrorHandlerTest, HandleNullMessageReturnsServerError)
{
    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVER_ERROR), pHandler->Handle(IMS_NULL));
}

TEST_F(UpdateErrorHandlerTest, HandleMessageReturnsUnspecifiedReasonIfNoActionIsConfigured)
{
    for (IMS_SINT32 nStatusCode = SipStatusCode::SC_300; nStatusCode < SipStatusCode::SC_MAX;
            nStatusCode++)
    {
        SetActionConfigs(nStatusCode, {});
        ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

        EXPECT_EQ(CallReasonInfo(CODE_UNSPECIFIED), pHandler->Handle(&objMessage));
    }
}

TEST_F(UpdateErrorHandlerTest, HandleMessageReturnsDefaultReasonIfTerminateAction)
{
    for (const auto& objCase : s_defaultStatusCodeAndReasonCodeMap)
    {
        IMS_SINT32 nStatusCode = objCase.first;
        IMS_SINT32 nExpectedReasonCode = objCase.second;

        SetActionConfigs(nStatusCode, {ConfigVoice::UPDATE_ERROR_ACTION_TERMINATE});
        ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

        EXPECT_EQ(nExpectedReasonCode, pHandler->Handle(&objMessage).nCode);
    }
}

TEST_F(UpdateErrorHandlerTest, HandleMessageReturnsUnspecifiedIfRetryAfterActionButNoRetryAfter)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_500;
    SetActionConfigs(nStatusCode, {ConfigVoice::UPDATE_ERROR_ACTION_RETRY});
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(-1));

    EXPECT_EQ(CODE_UNSPECIFIED, pHandler->Handle(&objMessage).nCode);
}

TEST_F(UpdateErrorHandlerTest, HandleMessageReturnsInternalRetryUpdateIfRetryAfterAction)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_500;
    SetActionConfigs(nStatusCode, {ConfigVoice::UPDATE_ERROR_ACTION_RETRY});
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(10));

    EXPECT_EQ(CODE_INTERNAL_RETRY_UPDATE, pHandler->Handle(&objMessage).nCode);
}

TEST_F(UpdateErrorHandlerTest, HandleMessageReturnsInternalRetryUpdateForMoIfGlareConditionAction)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_491;
    SetActionConfigs(nStatusCode, {ConfigVoice::UPDATE_ERROR_ACTION_GLARE_CONDITION});
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

    objCallInfo.ePeerType = PeerType::MO;

    EXPECT_EQ(CODE_INTERNAL_RETRY_UPDATE, pHandler->Handle(&objMessage).nCode);
}

TEST_F(UpdateErrorHandlerTest, HandleMessageReturnsInternalRetryUpdateForMtIfGlareConditionAction)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_491;
    SetActionConfigs(nStatusCode, {ConfigVoice::UPDATE_ERROR_ACTION_GLARE_CONDITION});
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

    objCallInfo.ePeerType = PeerType::MT;

    EXPECT_EQ(CODE_INTERNAL_RETRY_UPDATE, pHandler->Handle(&objMessage).nCode);
}

TEST_F(UpdateErrorHandlerTest,
        HandleMessageReturnsServiceUnavailableIfBlockActionButWithoutRetryAfter)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_503;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
    SetActionConfigs(nStatusCode, {ConfigVoice::UPDATE_ERROR_ACTION_BLOCK_CALL_BY_TIMER});
    SipMethod objMethod(SipMethod::INVITE);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(-1));

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(0)).Times(1);
    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVICE_UNAVAILABLE, nStatusCode),
            pHandler->Handle(&objMessage));
}

TEST_F(UpdateErrorHandlerTest,
        HandleMessageWithBlockActionWithRetryAfterToInviteRequestReturnsServiceUnavailableWithCallingAos)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_503;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
    SetActionConfigs(nStatusCode, {ConfigVoice::UPDATE_ERROR_ACTION_BLOCK_CALL_BY_TIMER});
    SipMethod objMethod(SipMethod::INVITE);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    IMS_SINT32 nAnyRetryAfter = 10;
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(m_pConfigService->GetMockCarrierConfig(),
            GetInt(ConfigIms::KEY_SIP_TIMER_B_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter - 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objContext.GetSlotId());

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(nAnyRetryAfter)).Times(1);
    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVICE_UNAVAILABLE, nStatusCode),
            pHandler->Handle(&objMessage));
}

TEST_F(UpdateErrorHandlerTest,
        HandleMessageWithBlockActionWithRetryAfterToUpdateRequestReturnsServiceUnavailableWithCallingAos)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_503;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
    SetActionConfigs(nStatusCode, {ConfigVoice::UPDATE_ERROR_ACTION_BLOCK_CALL_BY_TIMER});
    SipMethod objMethod(SipMethod::UPDATE);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    IMS_SINT32 nAnyRetryAfter = 10;
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(m_pConfigService->GetMockCarrierConfig(),
            GetInt(ConfigIms::KEY_SIP_TIMER_F_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter - 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objContext.GetSlotId());

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(nAnyRetryAfter)).Times(1);
    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVICE_UNAVAILABLE, nStatusCode),
            pHandler->Handle(&objMessage));
}

TEST_F(UpdateErrorHandlerTest,
        HandleMessageWithBlockActionWithRetryAfterReturnsServerErrorWithOutCallingAos)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_503;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
    SetActionConfigs(nStatusCode, {ConfigVoice::UPDATE_ERROR_ACTION_BLOCK_CALL_BY_TIMER});
    SipMethod objMethod(SipMethod::UPDATE);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    IMS_SINT32 nAnyRetryAfter = 10;
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(m_pConfigService->GetMockCarrierConfig(),
            GetInt(ConfigIms::KEY_SIP_TIMER_F_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter + 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objContext.GetSlotId());

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(_)).Times(0);
    EXPECT_CALL(objPassiveTimerHolder,
            AddTimer(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, nAnyRetryAfter * 1000,
                    IMS_FALSE, IMS_FALSE))
            .Times(1);
    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode), pHandler->Handle(&objMessage));
}
