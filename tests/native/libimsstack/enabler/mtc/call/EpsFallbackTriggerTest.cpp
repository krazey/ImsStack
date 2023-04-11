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

#include "CallReasonInfo.h"
#include "IImsAosInfo.h"
#include "MockIMtcService.h"
#include "MtcDef.h"
#include "PlatformContext.h"
#include "TestImsRadioService.h"
#include "TestPhoneInfoService.h"
#include "TestTimerService.h"
#include "call/EpsFallbackTrigger.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockISession.h"
#include "helper/MockIMtcAosConnector.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class EpsFallbackTriggerTest : public ::testing::Test
{
public:
    inline EpsFallbackTriggerTest() :
            pConfigurationManager(IMS_NULL),
            pConfigurationProxy(IMS_NULL),
            objTimerService(),
            objTimer(objTimerService.GetMockTimer()),
            pEpsFbTrigger(IMS_NULL)
    {
    }

    MockIMtcCallContext objContext;
    MockIMtcService objService;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    TestPhoneInfoService objPhoneInfoService;
    TestImsRadioService objImsRadioService;
    TestTimerService objTimerService;
    MockITimer& objTimer;
    EpsFallbackTrigger* pEpsFbTrigger;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &objTimerService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, &objImsRadioService);

        ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
                .WillByDefault(Return(NW_REPORT_RADIO_NR));

        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

        pEpsFbTrigger = new EpsFallbackTrigger(objContext);
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pEpsFbTrigger;

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_RADIO, IMS_NULL);
    }
};

TEST_F(EpsFallbackTriggerTest, IsRequiredChecksWatchdogTimeConfiguration)
{
    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(-1));
    EXPECT_FALSE(pEpsFbTrigger->IsRequired(*pConfigurationProxy));

    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(0));
    EXPECT_FALSE(pEpsFbTrigger->IsRequired(*pConfigurationProxy));

    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(6000));
    EXPECT_TRUE(pEpsFbTrigger->IsRequired(*pConfigurationProxy));
}

TEST_F(EpsFallbackTriggerTest, IsVoNrChecksWifiFirst)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    EXPECT_FALSE(pEpsFbTrigger->IsVoNr());
}

TEST_F(EpsFallbackTriggerTest, IsVoNrChecksRadioInfo)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    EXPECT_FALSE(pEpsFbTrigger->IsVoNr());

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));
    EXPECT_TRUE(pEpsFbTrigger->IsVoNr());
}

TEST_F(EpsFallbackTriggerTest, StartWatchdogSetsTimer)
{
    IMS_SINT32 nAnyWatchdogTime = 6000;
    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime)
            .WillByDefault(Return(nAnyWatchdogTime));

    EXPECT_CALL(objTimer, SetTimer(nAnyWatchdogTime, pEpsFbTrigger));
    pEpsFbTrigger->StartWatchdog();
}

TEST_F(EpsFallbackTriggerTest, StartWatchdogAndTimerExpiredNotTriggersEpsFallbackIfQosAndNotInNr)
{
    IMS_SINT32 nAnyWatchdogTime = 6000;
    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime)
            .WillByDefault(Return(nAnyWatchdogTime));
    pEpsFbTrigger->StartWatchdog();

    MockIMtcSession objMtcSession;
    MockISession objSession;
    ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));

    MockIMtcPreconditionManager objPreconditionManager;
    ON_CALL(objContext, GetPreconditionManager).WillByDefault(ReturnRef(objPreconditionManager));

    ON_CALL(objPreconditionManager, IsDedicatedBearerAllocated(&objSession, MEDIATYPE_AUDIO))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), TriggerEpsFallback(_)).Times(0);
    pEpsFbTrigger->Timer_TimerExpired(&objTimer);
}

TEST_F(EpsFallbackTriggerTest, StartWatchdogAndTimerExpiredNotTriggersEpsFallbackIfQosAndInNr)
{
    IMS_SINT32 nAnyWatchdogTime = 6000;
    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime)
            .WillByDefault(Return(nAnyWatchdogTime));
    pEpsFbTrigger->StartWatchdog();

    MockIMtcSession objMtcSession;
    MockISession objSession;
    ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));

    MockIMtcPreconditionManager objPreconditionManager;
    ON_CALL(objContext, GetPreconditionManager).WillByDefault(ReturnRef(objPreconditionManager));

    ON_CALL(objPreconditionManager, IsDedicatedBearerAllocated(&objSession, MEDIATYPE_AUDIO))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));

    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), TriggerEpsFallback(_)).Times(0);
    pEpsFbTrigger->Timer_TimerExpired(&objTimer);
}

TEST_F(EpsFallbackTriggerTest, StartWatchdogAndTimerExpiredNotTriggersEpsFallbackIfNoQosAndNotInNr)
{
    IMS_SINT32 nAnyWatchdogTime = 6000;
    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime)
            .WillByDefault(Return(nAnyWatchdogTime));
    pEpsFbTrigger->StartWatchdog();

    MockIMtcSession objMtcSession;
    MockISession objSession;
    ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));

    MockIMtcPreconditionManager objPreconditionManager;
    ON_CALL(objContext, GetPreconditionManager).WillByDefault(ReturnRef(objPreconditionManager));

    ON_CALL(objPreconditionManager, IsDedicatedBearerAllocated(&objSession, MEDIATYPE_AUDIO))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), TriggerEpsFallback(_)).Times(0);
    pEpsFbTrigger->Timer_TimerExpired(&objTimer);
}

TEST_F(EpsFallbackTriggerTest, StartWatchdogAndTimerExpiredTriggersEpsFallbackIfNoQosAndInNr)
{
    IMS_SINT32 nAnyWatchdogTime = 6000;
    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime)
            .WillByDefault(Return(nAnyWatchdogTime));
    pEpsFbTrigger->StartWatchdog();

    MockIMtcSession objMtcSession;
    MockISession objSession;
    ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));

    MockIMtcPreconditionManager objPreconditionManager;
    ON_CALL(objContext, GetPreconditionManager).WillByDefault(ReturnRef(objPreconditionManager));

    ON_CALL(objPreconditionManager, IsDedicatedBearerAllocated(&objSession, MEDIATYPE_AUDIO))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));

    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            TriggerEpsFallback(IImsRadio::EPSFB_REASON_NO_NETWORK_TRIGGER))
            .Times(1);
    pEpsFbTrigger->Timer_TimerExpired(&objTimer);
    EXPECT_TRUE(pEpsFbTrigger->IsWaitingEpsFallbackForNoTrigger());
    pEpsFbTrigger->OnEpsFallbackCompleted();
    EXPECT_FALSE(pEpsFbTrigger->IsWaitingEpsFallbackForNoTrigger());
}

TEST_F(EpsFallbackTriggerTest, TriggerNoResponseEpsFallbackSetsTimerAndTriggersEpsFallback)
{
    MockIMtcAosConnector objAosConnector;
    ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));
    EXPECT_CALL(objAosConnector, NotifyEpsfbCallState(IImsAosInfo::EPSFB_CALL_START));
    EXPECT_CALL(objTimer, SetTimer(12000, pEpsFbTrigger));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            TriggerEpsFallback(IImsRadio::EPSFB_REASON_NO_NETWORK_RESPONSE));

    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_RESPONSE);

    EXPECT_TRUE(pEpsFbTrigger->IsWaitingEpsFallbackForNoResponse());

    // call terminated without OnEpsFallbackCompleted() case.
    EXPECT_CALL(objAosConnector, NotifyEpsfbCallState(IImsAosInfo::EPSFB_CALL_FAILED));
    delete pEpsFbTrigger;
    pEpsFbTrigger = IMS_NULL;
}

TEST_F(EpsFallbackTriggerTest, OnEpsFallbackCompletedAfterTriggerEpsfbStopsTimer)
{
    MockIMtcAosConnector objAosConnector;
    ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));

    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_RESPONSE);

    EXPECT_CALL(objTimer, KillTimer);

    pEpsFbTrigger->OnEpsFallbackCompleted();

    EXPECT_FALSE(pEpsFbTrigger->IsWaitingEpsFallbackForNoResponse());
}

TEST_F(EpsFallbackTriggerTest, TriggerNoResponseEpsFallbackAndTimerExpiredTerminatesCall)
{
    MockIMtcAosConnector objAosConnector;
    ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));

    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_RESPONSE);

    MockIMtcCall objCall;
    ON_CALL(objContext, GetCall).WillByDefault(ReturnRef(objCall));

    EXPECT_CALL(objCall,
            Terminate(CallReasonInfo(
                    CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL)));

    EXPECT_CALL(objAosConnector, NotifyEpsfbCallState(IImsAosInfo::EPSFB_CALL_FAILED));

    pEpsFbTrigger->Timer_TimerExpired(&objTimer);
}
