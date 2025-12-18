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

#include "IMessage.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ImsAosParameter.h"
#include "ImsList.h"
#include "MediaDef.h"
#include "MockIMessage.h"
#include "MockIMtcImsEventReceiver.h"
#include "MockIMtcService.h"
#include "MockIPhoneInfoLocation.h"
#include "MockISession.h"
#include "MockISipMessage.h"
#include "MtcDef.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/MockEpsFallbackTrigger.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "call/ParticipantInfo.h"
#include "call/block/MockIMtcBlockChecker.h"
#include "call/extension/MockIMtcExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/message/IMtcMessageHandler.h"
#include "call/radio/IMtcRadioChecker.h"
#include "call/radio/MockIMtcRadioChecker.h"
#include "call/state/IdleState.h"
#include "call/state/MtcCallState.h"
#include "conferencecall/ConferenceDef.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/MockIMtcDialingPlan.h"
#include "dialogevent/MockIMultiEndpointManager.h"
#include "helper/MockILastComeFirstServedHelper.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MockIPassiveTimerHolder.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MtcLocationRefresher.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "ussi/MockUssiController.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;

// IMessage
MATCHER_P(IsEqualMessage, message, "")
{
    return &arg == message;
}

MATCHER_P(IsCopiedUser, originalUser, "")
{
    return arg.GetAt(0) != &originalUser && *arg.GetAt(0) == originalUser;
}

class IdleStateTest : public ::testing::Test
{
public:
    IdleState* pIdleState;
    MockMtcConfigurationProxy* pConfigurationProxy;
    MtcSupplementaryService* pSupplementaryService;
    ParticipantInfo* pParticipantInfo;
    MockIMtcCallContext objCallContext;
    MockIMtcDialingPlan objDialingPlan;
    MockIMtcMediaManager objMediaManager;
    MockIMtcService objService;
    MockIMtcImsEventReceiver objImsEventReceiver;
    MockIMtcCallManager objCallManager;
    MockIMtcRadioChecker objMockIMtcRadioChecker;
    MockIMtcBlockChecker* pBlockChecker;
    MockEpsFallbackTrigger* pEpsfbTrigger;
    MockIMtcSession objMtcSession;
    MockIMtcUiNotifier objUiNotifier;
    MockISession objSession;
    MockIMtcPreconditionManager objPreconditionManager;
    MockIMtcAosConnector objAosConnector;
    MockMtcTimerWrapper objTimerWrapper;
    MockUssiController* pUssiController;
    MockIMessageUtils objMessageUtils;
    MockIPassiveTimerHolder objPassiveTimerHolder;
    MockILastComeFirstServedHelper objLastComeFirstServedHelper;
    MockILocationInfo objLocationInfo;
    MtcLocationRefresher* pLocationRefresher;

    MediaInfo objInputMediaInfo;
    MediaInfo objOutputMediaInfo;
    ImsList<SuppService*> objInputSuppServices;
    CallInfo objCallInfo;

protected:
    virtual void SetUp() override
    {
        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        pSupplementaryService = new MtcSupplementaryService(objCallContext, *pConfigurationProxy);
        ON_CALL(*pConfigurationProxy,
                GetBoolean(ConfigVoice::KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL))
                .WillByDefault(Return(IMS_TRUE));

        ON_CALL(objCallContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));

        pBlockChecker = new MockIMtcBlockChecker();

        pParticipantInfo = new ParticipantInfo(objCallContext);
        ON_CALL(objCallContext, GetParticipantInfo).WillByDefault(ReturnRef(*pParticipantInfo));

        pEpsfbTrigger = new MockEpsFallbackTrigger(objCallContext);

        ON_CALL(objCallContext, GetDialingPlan).WillByDefault(ReturnRef(objDialingPlan));
        ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objCallContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objCallContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objCallContext, GetImsEventReceiver).WillByDefault(ReturnRef(objImsEventReceiver));
        ON_CALL(objCallContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));
        ON_CALL(objCallContext, GetRadioChecker).WillByDefault(ReturnRef(objMockIMtcRadioChecker));
        ON_CALL(objMockIMtcRadioChecker, Check(_, _, _, _, _, _))
                .WillByDefault(Return(IMtcRadioChecker::CheckResult::Blocked()));
        ON_CALL(objCallContext, CreateBlockChecker).WillByDefault(Return(pBlockChecker));
        ON_CALL(objCallContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objCallContext, GetUiNotifier).WillByDefault(ReturnRef(objUiNotifier));
        ON_CALL(objCallContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objPreconditionManager));
        ON_CALL(objCallContext, GetTimer).WillByDefault(ReturnRef(objTimerWrapper));
        ON_CALL(objCallContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objCallContext, GetPassiveTimerHolder)
                .WillByDefault(ReturnRef(objPassiveTimerHolder));
        ON_CALL(objCallContext, GetLastComeFirstServedHelper)
                .WillByDefault(ReturnRef(objLastComeFirstServedHelper));
        ON_CALL(objCallContext, GetEpsFallbackTrigger).WillByDefault(ReturnRef(*pEpsfbTrigger));

        pUssiController = new MockUssiController(objCallContext, IMS_NULL);
        ON_CALL(objCallContext, GetUssiController).WillByDefault(Return(pUssiController));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));

        pLocationRefresher = new MtcLocationRefresher(objLocationInfo);
        ON_CALL(objCallContext, GetLocationRefresher).WillByDefault(ReturnRef(*pLocationRefresher));

        ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));

        ON_CALL(objMediaManager, GetMediaInfo(Ref(objSession)))
                .WillByDefault(ReturnRef(objOutputMediaInfo));

        pIdleState = new IdleState(objCallContext);
    }

    virtual void TearDown() override
    {
        delete pIdleState;
        delete pConfigurationProxy;
        delete pSupplementaryService;
        delete pParticipantInfo;
        delete pUssiController;
        delete pEpsfbTrigger;
        delete pLocationRefresher;
    }

    MtcExtensionSet GetTestExtensionSet(IN const AString& strOptionTag)
    {
        ImsList<IMtcExtension*> objExtensions;
        MockIMtcExtension* pExtension = new MockIMtcExtension();
        ON_CALL(*pExtension, GetOptionTag).WillByDefault(ReturnRef(strOptionTag));
        ON_CALL(*pExtension, IsAvailableOnRemote).WillByDefault(Return(IMS_TRUE));
        objExtensions.Append(pExtension);
        MtcExtensionSet objMtcExtensionSet(objCallContext, objExtensions);
        return objMtcExtensionSet;
    }
};

TEST_F(IdleStateTest, OnEnterInvokesPerformPreRadioCheckForMo)
{
    objCallInfo.ePeerType = PeerType::MO;
    EXPECT_CALL(objMockIMtcRadioChecker, Check(_, _, _, _, _, _)).Times(1);

    pIdleState->OnEnter();
}

TEST_F(IdleStateTest, OnEnterDoesNotInvokePreRadioCheckForMt)
{
    objCallInfo.ePeerType = PeerType::MT;
    EXPECT_CALL(objMockIMtcRadioChecker, Check(_, _, _, _, _, _)).Times(0);
    pIdleState->OnEnter();
}

TEST_F(IdleStateTest, StartSetsUpCallInfo)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    IMS_BOOL bUssi = IMS_FALSE;

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(bUssi));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strTarget));

    pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices);

    EXPECT_EQ(eCallType, objCallInfo.eInitialCallType);
    EXPECT_EQ(PeerType::MO, objCallInfo.ePeerType);
    EXPECT_EQ(bUssi, objCallInfo.bUssi);
    EXPECT_EQ(IMS_FALSE, objCallInfo.bConference);
}

TEST_F(IdleStateTest, StartSetsUpParticipantInfo)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strTarget));

    pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices);

    EXPECT_EQ(strTarget, pParticipantInfo->GetRemoteNumber());
}

TEST_F(IdleStateTest, StartSetsMediaInfo)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    ImsList<ConfUser*> lstUsers;

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_SUCCESS));
    EXPECT_CALL(objMediaManager, SetMediaInfo(Ref(objSession), objInputMediaInfo));
    pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices);
}

TEST_F(IdleStateTest, StartSetsUpSupplementaryService)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");

    SuppService* pService = new SuppService();
    pService->nType = static_cast<IMS_SINT32>(SuppType::GEOLOCATION);
    objInputSuppServices.Append(pService);

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strTarget));

    pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices);

    EXPECT_NE(nullptr, pSupplementaryService->Get(SuppType::GEOLOCATION));
}

TEST_F(IdleStateTest, StartUpdatesRemoteNumberIfCallerIdRestrictionIsIncluded)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ImsVector<AString> objCodeRestricted;
    objCodeRestricted.Push("*123");
    objCodeRestricted.Push("*67");
    ON_CALL(*pConfigurationProxy,
            GetStringArray(ConfigVoice::KEY_CALLER_ID_SERVICE_CODES_FOR_RESTRICTION_STRING_ARRAY))
            .WillByDefault(Return(objCodeRestricted));

    CallType eCallType = CallType::VOIP;
    AString strTargetWithCallerId("*67some_target");
    AString strTargetWithoutCallerId("some_target");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strTargetWithoutCallerId));

    pIdleState->Start(eCallType, strTargetWithCallerId, objInputMediaInfo, objInputSuppServices);

    EXPECT_EQ(strTargetWithoutCallerId, pParticipantInfo->GetRemoteDisplayName());
    EXPECT_EQ(CALLERID_RESTRICTED, pSupplementaryService->Get(SuppType::CALLER_ID)->nValue);
}

TEST_F(IdleStateTest, StartUpdatesRemoteNumberIfCallerIdIdentityIsIncluded)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ImsVector<AString> objCodeIdentity;
    objCodeIdentity.Push("*123");
    objCodeIdentity.Push("*82");
    ON_CALL(*pConfigurationProxy,
            GetStringArray(ConfigVoice::KEY_CALLER_ID_SERVICE_CODES_FOR_IDENTITY_STRING_ARRAY))
            .WillByDefault(Return(objCodeIdentity));

    CallType eCallType = CallType::VOIP;
    AString strTargetWithCallerId("*82some_target");
    AString strTargetWithoutCallerId("some_target");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strTargetWithoutCallerId));

    pIdleState->Start(eCallType, strTargetWithCallerId, objInputMediaInfo, objInputSuppServices);

    EXPECT_EQ(strTargetWithoutCallerId, pParticipantInfo->GetRemoteDisplayName());
    EXPECT_EQ(CALLERID_IDENTITY, pSupplementaryService->Get(SuppType::CALLER_ID)->nValue);
}

TEST_F(IdleStateTest, StartDoesNotUpdateRemoteNumberIfCallerIdRestrictionIsIncludedButConfigIsOn)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ImsVector<AString> objCodeRestricted;
    objCodeRestricted.Push("*123");
    objCodeRestricted.Push("*67");
    ON_CALL(*pConfigurationProxy,
            GetStringArray(ConfigVoice::KEY_CALLER_ID_SERVICE_CODES_FOR_RESTRICTION_STRING_ARRAY))
            .WillByDefault(Return(objCodeRestricted));

    CallType eCallType = CallType::VOIP;
    AString strTargetWithCallerId("*67some_target");
    AString strTargetWithoutCallerId("some_target");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strTargetWithoutCallerId));

    pIdleState->Start(eCallType, strTargetWithCallerId, objInputMediaInfo, objInputSuppServices);

    EXPECT_EQ(strTargetWithCallerId, pParticipantInfo->GetRemoteDisplayName());
    EXPECT_EQ(nullptr, pSupplementaryService->Get(SuppType::CALLER_ID));
}

TEST_F(IdleStateTest, StartDoesNotUpdateRemoteNumberIfCallerIdRestrictionIsNotIncluded)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ImsVector<AString> objCodeRestricted;
    objCodeRestricted.Push("*123");
    objCodeRestricted.Push("*67");
    ON_CALL(*pConfigurationProxy,
            GetStringArray(ConfigVoice::KEY_CALLER_ID_SERVICE_CODES_FOR_RESTRICTION_STRING_ARRAY))
            .WillByDefault(Return(objCodeRestricted));

    CallType eCallType = CallType::VOIP;
    AString strTargetWithoutCallerId("some_target");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strTargetWithoutCallerId));

    pIdleState->Start(eCallType, strTargetWithoutCallerId, objInputMediaInfo, objInputSuppServices);

    EXPECT_EQ(strTargetWithoutCallerId, pParticipantInfo->GetRemoteDisplayName());
    EXPECT_EQ(nullptr, pSupplementaryService->Get(SuppType::CALLER_ID));
}

TEST_F(IdleStateTest, StartDoesNotUpdateRemoteNumberIfCallerIdServiceCodesIsEmpty)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ImsVector<AString> objCodeRestricted;
    ON_CALL(*pConfigurationProxy,
            GetStringArray(ConfigVoice::KEY_CALLER_ID_SERVICE_CODES_FOR_IDENTITY_STRING_ARRAY))
            .WillByDefault(Return(objCodeRestricted));

    CallType eCallType = CallType::VOIP;
    AString strTargetWithCallerId("*67some_target");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strTargetWithCallerId));

    pIdleState->Start(eCallType, strTargetWithCallerId, objInputMediaInfo, objInputSuppServices);

    EXPECT_EQ(strTargetWithCallerId, pParticipantInfo->GetRemoteDisplayName());
    EXPECT_EQ(nullptr, pSupplementaryService->Get(SuppType::CALLER_ID));
}

TEST_F(IdleStateTest, StartDoesNotUpdateRemoteNumberIfEmergencyCall)
{
    EXPECT_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL))
            .Times(0);

    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    CallType eCallType = CallType::VOIP;
    AString strTargetWithoutCallerId("some_target");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strTargetWithoutCallerId));

    pIdleState->Start(eCallType, strTargetWithoutCallerId, objInputMediaInfo, objInputSuppServices);

    EXPECT_EQ(strTargetWithoutCallerId, pParticipantInfo->GetRemoteDisplayName());
    EXPECT_EQ(nullptr, pSupplementaryService->Get(SuppType::CALLER_ID));
}

TEST_F(IdleStateTest, StartSetsUpMediaManager)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strTarget));

    pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices);
}

TEST_F(IdleStateTest, StartInvokesSendStartFailedIfBlockCheckerBlocked)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");

    CallReasonInfo objReasonInfo(CODE_LOCAL_NOT_REGISTERED);
    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(Return(IMtcBlockChecker::Result(
                    IMtcBlockChecker::Result::Status::BLOCKED, objReasonInfo)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonInfo));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, StartInvokesSendStartFailedIfBlockCheckerBlockedForEmergencyCall)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;

    CallReasonInfo objReasonInfo(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY);
    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(Return(IMtcBlockChecker::Result(
                    IMtcBlockChecker::Result::Status::BLOCKED, objReasonInfo)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonInfo));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, StartInvokesSendStartFailedIfCreateSessionFailed)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");

    CallReasonInfo objReasonInfo(CODE_REJECT_INTERNAL_ERROR);
    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(nullptr));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonInfo));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, StartInvokesSendStartFailedIfCreateSessionFailedForEmergencyCall)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;

    CallReasonInfo objReasonInfo(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY);
    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(nullptr));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonInfo));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, StartInvokesSendStartFailedIfStartSessionFailed)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");

    CallReasonInfo objReasonInfo(CODE_REJECT_INTERNAL_ERROR);
    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_FAILURE));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonInfo));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, StartInvokesSendStartFailedIfStartSessionFailedForEmergencyCall)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;

    CallReasonInfo objReasonInfo(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY);
    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_FAILURE));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonInfo));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, StartSetsMoTimersAndTransitsToOutgoingState)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_SUCCESS));
    IMS_SINT32 nResponseTimeoutForReasonTimer = 10000;
    IMS_SINT32 n18xWaitTimer = 20000;
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_USER_CANCEL_REASON_AFTER_RESPONSE_TIMEOUT_TIMER_MILLIS_INT))
            .WillByDefault(Return(nResponseTimeoutForReasonTimer));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_18X_TIMER_MILLIS_INT))
            .WillByDefault(Return(n18xWaitTimer));

    EXPECT_CALL(objTimerWrapper,
            Start(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON,
                    nResponseTimeoutForReasonTimer));
    EXPECT_CALL(objTimerWrapper,
            Start(MtcCallState::TimerType::TIMER_MO_CALL_INITIATION_TO_18X_WAIT, _))
            .Times(0);
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TimerType::TIMER_MO_18X_WAIT, n18xWaitTimer))
            .Times(0);

    EXPECT_EQ(CallStateName::OUTGOING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, StartSetsMoTimersForEmergencyCall)
{
    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_SUCCESS));

    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    IMS_SINT32 nResponseTimeoutForReasonTimer = 10000;
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_USER_CANCEL_REASON_AFTER_RESPONSE_TIMEOUT_TIMER_MILLIS_INT))
            .WillByDefault(Return(nResponseTimeoutForReasonTimer));
    EXPECT_CALL(objTimerWrapper,
            Start(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON,
                    nResponseTimeoutForReasonTimer));

    IMS_SINT32 nInviteTo18xWaitTimer = 20000;
    ON_CALL(objTimerWrapper,
            IsActive(MtcCallState::TimerType::TIMER_MO_CALL_INITIATION_TO_18X_WAIT))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            GetIntFromArray(ConfigVoice::KEY_CALL_INITIATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 0))
            .WillByDefault(Return(nInviteTo18xWaitTimer));
    EXPECT_CALL(objTimerWrapper,
            Start(MtcCallState::TimerType::TIMER_MO_CALL_INITIATION_TO_18X_WAIT,
                    nInviteTo18xWaitTimer));

    EXPECT_CALL(objTimerWrapper,
            Start(MtcCallState::TimerType::TIMER_MO_18X_WAIT, nInviteTo18xWaitTimer))
            .Times(0);

    pIdleState->Start(CallType::VOIP, "target", objInputMediaInfo, objInputSuppServices);
}

TEST_F(IdleStateTest, StartSetsMoTimersForWifiEmergencyCall)
{
    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_SUCCESS));

    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    IMS_SINT32 nResponseTimeoutForReasonTimer = 10000;
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_USER_CANCEL_REASON_AFTER_RESPONSE_TIMEOUT_TIMER_MILLIS_INT))
            .WillByDefault(Return(nResponseTimeoutForReasonTimer));
    EXPECT_CALL(objTimerWrapper,
            Start(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON,
                    nResponseTimeoutForReasonTimer));

    IMS_SINT32 nInviteTo18xWaitTimer = 20000;
    ON_CALL(objTimerWrapper,
            IsActive(MtcCallState::TimerType::TIMER_MO_CALL_INITIATION_TO_18X_WAIT))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            GetIntFromArray(ConfigVoice::KEY_CALL_INITIATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 1))
            .WillByDefault(Return(nInviteTo18xWaitTimer));
    EXPECT_CALL(objTimerWrapper,
            Start(MtcCallState::TimerType::TIMER_MO_CALL_INITIATION_TO_18X_WAIT,
                    nInviteTo18xWaitTimer));

    EXPECT_CALL(objTimerWrapper,
            Start(MtcCallState::TimerType::TIMER_MO_18X_WAIT, nInviteTo18xWaitTimer))
            .Times(0);

    pIdleState->Start(CallType::VOIP, "target", objInputMediaInfo, objInputSuppServices);
}

TEST_F(IdleStateTest, StartDoesNotStartInviteTo18xTimerIsAlreadyActive)
{
    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_SUCCESS));

    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    IMS_SINT32 nResponseTimeoutForReasonTimer = 10000;
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_USER_CANCEL_REASON_AFTER_RESPONSE_TIMEOUT_TIMER_MILLIS_INT))
            .WillByDefault(Return(nResponseTimeoutForReasonTimer));

    EXPECT_CALL(objTimerWrapper,
            Start(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON,
                    nResponseTimeoutForReasonTimer));
    ON_CALL(objTimerWrapper,
            IsActive(MtcCallState::TimerType::TIMER_MO_CALL_INITIATION_TO_18X_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimerWrapper,
            Start(MtcCallState::TimerType::TIMER_MO_CALL_INITIATION_TO_18X_WAIT, _))
            .Times(0);

    pIdleState->Start(CallType::VOIP, "target", objInputMediaInfo, objInputSuppServices);
}

TEST_F(IdleStateTest, StartNotifiesInitiating)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(objUiNotifier, SendInitiating);

    EXPECT_EQ(CallStateName::OUTGOING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, StartHandlesCallPullIfCallPullIsEnabledButFailsIfMepIsNotSupported)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("any_target");

    ON_CALL(objCallContext, GetMultiEndpointManager).WillByDefault(Return(nullptr));
    SuppService* pService = new SuppService();
    pService->nType = static_cast<IMS_SINT32>(SuppType::CALL_PULL);
    pService->nValue = 12345;
    objInputSuppServices.Append(pService);
    CallReasonInfo objReasonInfo(CODE_MULTIENDPOINT_NOT_SUPPORTED);
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonInfo));

    pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices);
}

TEST_F(IdleStateTest, StartHandlesCallPullButFailsIfNoMatchedDialogExists)
{
    MockIMultiEndpointManager objMepManager;
    ON_CALL(objCallContext, GetMultiEndpointManager).WillByDefault(Return(&objMepManager));

    CallType eCallType = CallType::VOIP;
    AString strTarget("any_target");
    IMS_BOOL bUssi = IMS_FALSE;

    SuppService* pService = new SuppService();
    pService->nType = static_cast<IMS_SINT32>(SuppType::CALL_PULL);
    pService->nValue = 12345;
    objInputSuppServices.Append(pService);

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(bUssi));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));

    IMultiEndpointManager::PullingDialogInfo objDialogInfo;

    CallReasonInfo objReasonInfo(CODE_CALL_PULL_OUT_OF_SYNC);
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonInfo));

    pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices);
}

TEST_F(IdleStateTest, StartHandlesCallPullIfCallPullIsEnabled)
{
    MockIMultiEndpointManager objMepManager;
    ON_CALL(objCallContext, GetMultiEndpointManager).WillByDefault(Return(&objMepManager));

    CallType eCallType = CallType::VOIP;
    AString strTarget("any_target");
    IMS_BOOL bUssi = IMS_FALSE;

    SuppService* pService = new SuppService();
    pService->nType = static_cast<IMS_SINT32>(SuppType::CALL_PULL);
    pService->nValue = 12345;
    objInputSuppServices.Append(pService);

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(bUssi));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));

    IMultiEndpointManager::PullingDialogInfo objDialogInfo;
    objDialogInfo.strCallId = "anyCallId";
    objDialogInfo.strLocalTag = "anyLocalTag";
    objDialogInfo.strRemoteTag = "anyRemoteTag";
    objDialogInfo.eCallType = CallType::VT;
    objDialogInfo.pMediaInfo = new MediaInfo(DIRECTION_SEND_RECEIVE, DIRECTION_SEND,
            DIRECTION_RECEIVE, AUDIO_QUALITY_AMR_NB, VIDEO_QUALITY_HD_PR, GTT_MODE_FULL);
    EXPECT_CALL(objMepManager, GetDialogInfo(pService->nValue)).WillOnce(Return(objDialogInfo));

    pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices);
    EXPECT_EQ(objDialogInfo.eCallType, objCallInfo.eInitialCallType);

    delete objDialogInfo.pMediaInfo;
}

TEST_F(IdleStateTest, StartUssiSetsMediaInfo)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    ImsList<ConfUser*> lstUsers;

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_SUCCESS));
    EXPECT_CALL(objMediaManager, SetMediaInfo(Ref(objSession), objInputMediaInfo));
    pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices);
}

TEST_F(IdleStateTest, StartUssiInvokesSendStartFailedIfCreateSessionFailed)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");

    CallReasonInfo objReasonInfo(CODE_REJECT_INTERNAL_ERROR);
    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(nullptr));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonInfo));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, StartUssiInvokesSendStartFailedIfFormUssiFailed)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");

    CallReasonInfo objReasonInfo(CODE_REJECT_INTERNAL_ERROR);
    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(*pUssiController, FormStartUssiRequest(_)).WillByDefault(Return(IMS_FAILURE));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonInfo));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, StartUssiInvokesSendStartFailedIfStartSessionFailed)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");

    CallReasonInfo objReasonInfo(CODE_REJECT_INTERNAL_ERROR);
    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(*pUssiController, FormStartUssiRequest(_)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objMediaManager, SetRtpPort(&objSession, MEDIATYPE_AUDIO, 0));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonInfo));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, StartUssiSetsMoTimersAndTransitsToOutgoingState)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_SUCCESS));
    IMS_SINT32 nResponseTimeoutForReasonTimer = 10000;
    IMS_SINT32 n18xWaitTimer = 20000;
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_USER_CANCEL_REASON_AFTER_RESPONSE_TIMEOUT_TIMER_MILLIS_INT))
            .WillByDefault(Return(nResponseTimeoutForReasonTimer));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_18X_TIMER_MILLIS_INT))
            .WillByDefault(Return(n18xWaitTimer));

    EXPECT_CALL(objTimerWrapper,
            Start(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON,
                    nResponseTimeoutForReasonTimer));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TimerType::TIMER_MO_18X_WAIT, n18xWaitTimer))
            .Times(0);

    EXPECT_EQ(CallStateName::OUTGOING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, StarUssitNotifiesInitiating)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(objUiNotifier, SendInitiating);

    EXPECT_EQ(CallStateName::OUTGOING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, StartConferenceWithMediaSetsMediaInfo)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    ImsList<ConfUser*> lstUsers;

    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));

    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    MockIMessage objMessage;
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMediaManager, SetMediaInfo(Ref(objSession), objInputMediaInfo));

    pIdleState->StartConference(
            eCallType, strTarget, objInputMediaInfo, objInputSuppServices, lstUsers);
}

TEST_F(IdleStateTest, StartConferenceWithoutMediaSetsMediaInfo)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    ImsList<ConfUser*> lstUsers;

    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));

    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    MockIMessage objMessage;
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));

    MediaInfo objMediaInfo(DIRECTION_SEND_RECEIVE, DIRECTION_INVALID, DIRECTION_INVALID,
            AUDIO_QUALITY_NONE, VIDEO_QUALITY_NONE, GTT_MODE_INVALID);
    EXPECT_CALL(objMediaManager, SetMediaInfo(Ref(objSession), objMediaInfo));

    pIdleState->StartConference(eCallType, strTarget, lstUsers);
}

TEST_F(IdleStateTest, StartConferenceForVtWithoutMediaSetsMediaInfo)
{
    CallType eCallType = CallType::VT;
    AString strTarget("some_target");
    ImsList<ConfUser*> lstUsers;

    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));

    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    MockIMessage objMessage;
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));

    MediaInfo objMediaInfo(DIRECTION_SEND_RECEIVE, DIRECTION_SEND_RECEIVE, DIRECTION_INVALID,
            AUDIO_QUALITY_NONE, VIDEO_QUALITY_NONE, GTT_MODE_INVALID);
    EXPECT_CALL(objMediaManager, SetMediaInfo(Ref(objSession), objMediaInfo));

    pIdleState->StartConference(eCallType, strTarget, lstUsers);
}

TEST_F(IdleStateTest, StartConferenceInvokesSendStartFailedIfCreateSessionFailed)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    ImsList<ConfUser*> lstUsers;

    CallReasonInfo objReasonInfo(CODE_REJECT_INTERNAL_ERROR);
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(nullptr));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING,
            pIdleState->StartConference(
                    eCallType, strTarget, objInputMediaInfo, objInputSuppServices, lstUsers));
}

TEST_F(IdleStateTest, StartConferenceInvokesSendStartFailedIfStartSessionFailed)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    ImsList<ConfUser*> lstUsers;

    CallReasonInfo objReasonInfo(CODE_REJECT_INTERNAL_ERROR);
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_FAILURE));

    MockIMessage objMessage;
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));

    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING,
            pIdleState->StartConference(
                    eCallType, strTarget, objInputMediaInfo, objInputSuppServices, lstUsers));
}

TEST_F(IdleStateTest, StartConferenceSetsMoTimersAndTransitsOutgoingState)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    ImsList<ConfUser*> lstUsers;

    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_SUCCESS));

    MockIMessage objMessage;
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));

    IMS_SINT32 nResponseTimeoutForReasonTimer = 10000;
    IMS_SINT32 n18xWaitTimer = 20000;
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_USER_CANCEL_REASON_AFTER_RESPONSE_TIMEOUT_TIMER_MILLIS_INT))
            .WillByDefault(Return(nResponseTimeoutForReasonTimer));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_18X_TIMER_MILLIS_INT))
            .WillByDefault(Return(n18xWaitTimer));
    EXPECT_CALL(objTimerWrapper,
            Start(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON,
                    nResponseTimeoutForReasonTimer));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TimerType::TIMER_MO_18X_WAIT, n18xWaitTimer))
            .Times(0);

    EXPECT_EQ(CallStateName::OUTGOING,
            pIdleState->StartConference(
                    eCallType, strTarget, objInputMediaInfo, objInputSuppServices, lstUsers));
}

TEST_F(IdleStateTest, StartConferenceNotifiesInitiating)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    ImsList<ConfUser*> lstUsers;

    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_SUCCESS));

    MockIMessage objMessage;
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));

    EXPECT_CALL(objUiNotifier, SendInitiating);

    EXPECT_EQ(CallStateName::OUTGOING,
            pIdleState->StartConference(
                    eCallType, strTarget, objInputMediaInfo, objInputSuppServices, lstUsers));
}

TEST_F(IdleStateTest, StartConferenceUsesCopiedConfUsers)
{
    AString strTarget("some_target");
    ImsList<ConfUser*> lstUsers;
    ConfUser objOriginalUser;
    objOriginalUser.nConnectionId = 12345;
    lstUsers.Append(&objOriginalUser);

    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    MockIMessage objMessage;
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));

    EXPECT_CALL(objMessageUtils,
            SetResourceList(&objMessage, IsCopiedUser(objOriginalUser), IMS_TRUE, IMS_TRUE));

    pIdleState->StartConference(CallType::VOIP, strTarget, lstUsers);
}

TEST_F(IdleStateTest, HandleIncomingRejectsIfCreateSessionFailed)
{
    ON_CALL(objCallContext, CreateSession(&objSession)).WillByDefault(Return(nullptr));

    CallReasonInfo objReasonInfo(CODE_REJECT_INTERNAL_ERROR);
    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->HandleIncoming(&objSession));
}

TEST_F(IdleStateTest, HandleIncomingRejectsIfUnsupportedExtensionIsRequired)
{
    ON_CALL(objCallContext, CreateSession(&objSession)).WillByDefault(Return(&objMtcSession));
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    AString strUnsupportedExtension("unsupportedExtension");
    ImsList<AString> lstRequiredExtensions;
    lstRequiredExtensions.Append(strUnsupportedExtension);
    ON_CALL(objMessageUtils, GetHeaders(pMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(lstRequiredExtensions));
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objMtcSession, HandleRequest(RequestType::START, IsEqualMessage(pMessage)));

    CallReasonInfo objReasonInfo(CODE_REJECT_UNSUPPORTED_SIP_HEADERS, -1, strUnsupportedExtension);
    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->HandleIncoming(&objSession));

    delete pMessage;
}

TEST_F(IdleStateTest, HandleIncomingRejectsIfOfferlessInviteIsNotSupported)
{
    ON_CALL(objCallContext, CreateSession(&objSession)).WillByDefault(Return(&objMtcSession));
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    ImsList<AString> lstRequiredExtensions;
    lstRequiredExtensions.Append(AString("supportedExtension"));
    ON_CALL(objMessageUtils, GetHeaders(pMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(lstRequiredExtensions));
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objMtcSession, HandleRequest(RequestType::START, IsEqualMessage(pMessage)));

    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasSdp(pMessage)).WillByDefault(Return(IMS_FALSE));

    CallReasonInfo objReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE);
    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->HandleIncoming(&objSession));

    delete pMessage;
}

TEST_F(IdleStateTest, HandleIncomingReturnsIdleStateIfBlockCheckResultIsPending)
{
    ON_CALL(objCallContext, CreateSession(&objSession)).WillByDefault(Return(&objMtcSession));
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    ImsList<AString> lstRequiredExtensions;
    lstRequiredExtensions.Append(AString("supportedExtension"));
    ON_CALL(objMessageUtils, GetHeaders(pMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(lstRequiredExtensions));
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objMtcSession, HandleRequest(RequestType::START, IsEqualMessage(pMessage)));

    IMS_SINT32 nMtAlertingTimer = 60;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_RINGING_TIMER_MILLIS_INT))
            .WillByDefault(Return(nMtAlertingTimer));
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(
            objTimerWrapper, Start(MtcCallState::TimerType::TIMER_MT_ALERTING, nMtAlertingTimer));

    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));

    EXPECT_EQ(CallStateName::IDLE, pIdleState->HandleIncoming(&objSession));

    delete pMessage;
}

TEST_F(IdleStateTest, HandleIncomingRejectsIfBlockCheckResultIsBlocked)
{
    objCallInfo.ePeerType = PeerType::MT;
    ON_CALL(objCallContext, CreateSession(&objSession)).WillByDefault(Return(&objMtcSession));
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    ImsList<AString> lstRequiredExtensions;
    lstRequiredExtensions.Append(AString("supportedExtension"));
    ON_CALL(objMessageUtils, GetHeaders(pMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(lstRequiredExtensions));
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objMtcSession, HandleRequest(RequestType::START, IsEqualMessage(pMessage)));

    IMS_SINT32 nMtAlertingTimer = 60;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_RINGING_TIMER_MILLIS_INT))
            .WillByDefault(Return(nMtAlertingTimer));
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(
            objTimerWrapper, Start(MtcCallState::TimerType::TIMER_MT_ALERTING, nMtAlertingTimer));

    CallReasonInfo objReasonInfo(CODE_LOCAL_NOT_REGISTERED);
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(Return(IMtcBlockChecker::Result(
                    IMtcBlockChecker::Result::Status::BLOCKED, objReasonInfo)));

    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));
    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->HandleIncoming(&objSession));

    delete pMessage;
}

TEST_F(IdleStateTest, HandleIncomingInvokesSendPreIncomingCallReceived)
{
    ON_CALL(objCallContext, CreateSession(&objSession)).WillByDefault(Return(&objMtcSession));
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    ImsList<AString> lstRequiredExtensions;
    lstRequiredExtensions.Append(AString("supportedExtension"));
    ON_CALL(objMessageUtils, GetHeaders(pMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(lstRequiredExtensions));
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objMtcSession, HandleRequest(RequestType::START, IsEqualMessage(pMessage)));

    IMS_SINT32 nMtAlertingTimer = 60;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_RINGING_TIMER_MILLIS_INT))
            .WillByDefault(Return(nMtAlertingTimer));
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(
            objTimerWrapper, Start(MtcCallState::TimerType::TIMER_MT_ALERTING, nMtAlertingTimer));

    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));

    EXPECT_CALL(objUiNotifier, SendPreIncomingCallReceived);

    EXPECT_EQ(CallStateName::IDLE, pIdleState->HandleIncoming(&objSession));

    delete pMessage;
}

TEST_F(IdleStateTest, HandleIncomingInvokesOnCallReceivedForLastComeFirstServedHelperIfSupported)
{
    ON_CALL(objCallContext, CreateSession(&objSession)).WillByDefault(Return(&objMtcSession));
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    ImsList<AString> objRequiredExtensions;
    objRequiredExtensions.Append(AString("supportedExtension"));
    ON_CALL(objMessageUtils, GetHeaders(pMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(objRequiredExtensions));
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    IMS_SINT32 nMtAlertingTimer = 60;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_RINGING_TIMER_MILLIS_INT))
            .WillByDefault(Return(nMtAlertingTimer));
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_PREALERTING_TIMER_MILLIS_INT))
            .WillByDefault(Return(7000));

    EXPECT_CALL(
            objTimerWrapper, Start(MtcCallState::TimerType::TIMER_MT_ALERTING, nMtAlertingTimer));

    const IMS_UINTP ANY_KEY = 100;
    ON_CALL(objCallContext, GetCallKey()).WillByDefault(Return(ANY_KEY));
    EXPECT_CALL(objLastComeFirstServedHelper, OnCallReceived(ANY_KEY)).Times(1);

    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));

    EXPECT_EQ(CallStateName::IDLE, pIdleState->HandleIncoming(&objSession));

    delete pMessage;
}

TEST_F(IdleStateTest, HandleIncomingDoesNothingForLastComeFirstServedHelperIfNotSupported)
{
    ON_CALL(objCallContext, CreateSession(&objSession)).WillByDefault(Return(&objMtcSession));
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    ImsList<AString> objRequiredExtensions;
    objRequiredExtensions.Append(AString("supportedExtension"));
    ON_CALL(objMessageUtils, GetHeaders(pMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(objRequiredExtensions));
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    IMS_SINT32 nMtAlertingTimer = 60;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_RINGING_TIMER_MILLIS_INT))
            .WillByDefault(Return(nMtAlertingTimer));
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_PREALERTING_TIMER_MILLIS_INT))
            .WillByDefault(Return(0));

    EXPECT_CALL(
            objTimerWrapper, Start(MtcCallState::TimerType::TIMER_MT_ALERTING, nMtAlertingTimer));

    const IMS_UINTP ANY_KEY = 100;
    ON_CALL(objCallContext, GetCallKey()).WillByDefault(Return(ANY_KEY));
    EXPECT_CALL(objLastComeFirstServedHelper, OnCallReceived(ANY_KEY)).Times(0);

    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));

    EXPECT_EQ(CallStateName::IDLE, pIdleState->HandleIncoming(&objSession));

    delete pMessage;
}

TEST_F(IdleStateTest, TerminateInvokesSendStartFailed)
{
    const CallReasonInfo objReasonInfo(CODE_MEDIA_UNSPECIFIED);
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->Terminate(objReasonInfo));
}

TEST_F(IdleStateTest, SessionTerminatedTransitsStateToTerminated)
{
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_TERMINATE))
            .WillByDefault(Return(nullptr));
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(_));
    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->SessionTerminated(&objSession));

    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_TERMINATE))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(_));
    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->SessionTerminated(&objSession));
}

TEST_F(IdleStateTest, OnAttachedRejectsIfSdpOaFailed)
{
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    SipMethod objMethod(SipMethod::ACK);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objMethod));

    const CallReasonInfo objReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE);
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReasonInfo));
    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->OnAttached());
}

TEST_F(IdleStateTest, OnAttachedRejectsIfNoCodecMatched)
{
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    SipMethod objMethod(SipMethod::ACK);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    ON_CALL(objMediaManager, NegotiateSdp(&objSession))
            .WillByDefault(Return(SdpNegotiationResult(MEDIA_NEGO_ERROR_NO_CODEC_MATCHED)));

    const CallReasonInfo objReasonInfo(
            CODE_MEDIA_NOT_ACCEPTABLE, MEDIA_NEGO_ERROR_NO_CODEC_MATCHED);
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReasonInfo));
    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->OnAttached());
}

TEST_F(IdleStateTest, OnAttachedRejectsIfInvalidDescriptor)
{
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    SipMethod objMethod(SipMethod::ACK);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    ON_CALL(objMediaManager, NegotiateSdp(&objSession))
            .WillByDefault(Return(SdpNegotiationResult(MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR)));

    const CallReasonInfo objReasonInfo(
            CODE_REJECT_UNSUPPORTED_SDP_HEADERS, MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR);
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReasonInfo));
    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->OnAttached());
}

TEST_F(IdleStateTest, OnAttachedRejectsIfSendProvisionalResponseFailed)
{
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));

    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(MtcExtensionSet::OPTION_TAG_RPR));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMtcSession, SendProvisionalResponse(IMS_FALSE, IMS_TRUE))
            .WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TimerType::TIMER_MT_PRACK_WAIT, _)).Times(0);

    const CallReasonInfo objReasonInfo(CODE_REJECT_INTERNAL_ERROR);
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReasonInfo));
    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->OnAttached());
}

TEST_F(IdleStateTest, OnAttachedTransitsIncomingState)
{
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_START))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));

    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(MtcExtensionSet::OPTION_TAG_RPR));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMtcSession, SendProvisionalResponse(IMS_FALSE, IMS_TRUE))
            .WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_180));

    IMS_SINT32 nPrackWaitTimer = 10000;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_PRACK_WAIT_TIMER_MILLIS_INT))
            .WillByDefault(Return(nPrackWaitTimer));

    EXPECT_CALL(
            objTimerWrapper, Start(MtcCallState::TimerType::TIMER_MT_PRACK_WAIT, nPrackWaitTimer));
    EXPECT_EQ(CallStateName::INCOMING, pIdleState->OnAttached());
}

TEST_F(IdleStateTest, OnAttachedInvokesSendIncomingCallReceivedIfRprNotSupported)
{
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));
    EXPECT_CALL(objPreconditionManager, OnMessageReceived(&objSession, &objMessage));

    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("no100rel")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived);

    EXPECT_EQ(CallStateName::ALERTING, pIdleState->OnAttached());
}

TEST_F(IdleStateTest, OnAttachedInvokesSendIncomingCallReceivedIfRequirePrackAndRprNotSupported)
{
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_RECEIVED));
    EXPECT_CALL(objPreconditionManager, OnMessageReceived(&objSession, &objMessage));

    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("no100rel")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived);

    EXPECT_EQ(CallStateName::ALERTING, pIdleState->OnAttached());
}

TEST_F(IdleStateTest, OnAttachedUpdatesCallTypeAndMediaInfoToNegotiatedTypeIfSdpExists)
{
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(&objMessage));
    SipMethod objMethod(SipMethod::INVITE);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    MockISipMessage objSipMessage;
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    ON_CALL(objSipMessage, GetType()).WillByDefault(Return(ISipMessage::TYPE_REQUEST));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objSession))
            .WillByDefault(Return(CallType::VT));
    ON_CALL(objMediaManager, NegotiateSdp(&objSession))
            .WillByDefault(Return(SdpNegotiationResult(MEDIA_NEGO_NO_ERROR)));
    ON_CALL(objMtcSession, IsVideoCapable).WillByDefault(Return(IMS_TRUE));

    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("no100rel")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objMtcSession, SetCapableCallType(CallType::VT));
    EXPECT_CALL(objMediaManager, SetMediaInfo(_, _)).Times(2);  // InitMediaSession & refined
    EXPECT_EQ(CallStateName::ALERTING, pIdleState->OnAttached());
}

TEST_F(IdleStateTest, OnAttachedDoesNotUpdateCallTypeToNegotiatedTypeIfNoSdp)
{
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objSession))
            .WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMediaManager, NegotiateSdp(&objSession))
            .WillByDefault(Return(SdpNegotiationResult(MEDIA_NEGO_NO_ERROR)));
    MediaInfo objMediaInfo;
    ON_CALL(objMediaManager, GetMediaInfo(_)).WillByDefault(ReturnRef(objMediaInfo));
    ON_CALL(objMtcSession, IsVideoCapable).WillByDefault(Return(IMS_TRUE));

    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("no100rel")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objMtcSession, SetCapableCallType(_)).Times(0);
    EXPECT_CALL(objMediaManager, SetMediaInfo(_, _)).Times(1);  // InitMediaSession, no refined
    EXPECT_EQ(CallStateName::ALERTING, pIdleState->OnAttached());
}

TEST_F(IdleStateTest, OnAttachedInvokesSendIncomingCallReceived)
{
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));
    EXPECT_CALL(objPreconditionManager, OnMessageReceived(&objSession, &objMessage));

    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("no100rel")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived);
    EXPECT_CALL(objMtcSession, SendProvisionalResponse(IMS_TRUE, IMS_TRUE)).Times(0);

    EXPECT_EQ(CallStateName::ALERTING, pIdleState->OnAttached());
}

TEST_F(IdleStateTest, HandleIncomingUssiRejectsIfOfferlessInviteIsNotSupported)
{
    ON_CALL(objCallContext, CreateSession(&objSession)).WillByDefault(Return(&objMtcSession));
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasSdp(pMessage)).WillByDefault(Return(IMS_FALSE));

    CallReasonInfo objReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE);
    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->HandleIncomingUssi(&objSession));

    delete pMessage;
}

TEST_F(IdleStateTest, HandleIncomingUssiRejectsIfMessageHasInvalidXml)
{
    ON_CALL(objCallContext, CreateSession(&objSession)).WillByDefault(Return(&objMtcSession));
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pUssiController, HasValidXmlBodyForNetworkInitiatedUssi(pMessage))
            .WillByDefault(Return(IMS_FALSE));

    CallReasonInfo objReasonInfo(CODE_REJECT_UNKNOWN);
    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->HandleIncomingUssi(&objSession));

    delete pMessage;
}

TEST_F(IdleStateTest, HandleIncomingUssiReturnsIdleStateIfBlockCheckResultIsPending)
{
    ON_CALL(objCallContext, CreateSession(&objSession)).WillByDefault(Return(&objMtcSession));
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pUssiController, HasValidXmlBodyForNetworkInitiatedUssi(pMessage))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));

    EXPECT_EQ(CallStateName::IDLE, pIdleState->HandleIncomingUssi(&objSession));

    delete pMessage;
}

TEST_F(IdleStateTest, HandleIncomingUssiRejectsIfBlockCheckResultIsBlocked)
{
    objCallInfo.ePeerType = PeerType::MT;
    ON_CALL(objCallContext, CreateSession(&objSession)).WillByDefault(Return(&objMtcSession));
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pUssiController, HasValidXmlBodyForNetworkInitiatedUssi(pMessage))
            .WillByDefault(Return(IMS_TRUE));
    CallReasonInfo objReasonInfo(CODE_LOCAL_NOT_REGISTERED);
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(Return(IMtcBlockChecker::Result(
                    IMtcBlockChecker::Result::Status::BLOCKED, objReasonInfo)));

    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));
    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->HandleIncomingUssi(&objSession));

    delete pMessage;
}

TEST_F(IdleStateTest, OnUssiAttachedRejectsIfUnsupportedExtensionIsRequired)
{
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    AString strUnsupportedExtension("unsupportedExtension");
    ImsList<AString> lstRequiredExtensions;
    lstRequiredExtensions.Append(strUnsupportedExtension);
    ON_CALL(objMessageUtils, GetHeaders(pMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(lstRequiredExtensions));
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objMtcSession, HandleRequest(RequestType::START, IsEqualMessage(pMessage)));

    CallReasonInfo objReasonInfo(CODE_REJECT_UNSUPPORTED_SIP_HEADERS, -1, strUnsupportedExtension);
    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->OnUssiAttached());

    delete pMessage;
}

TEST_F(IdleStateTest, OnUssiAttachedRejectsIfSdpOaFailed)
{
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    EXPECT_CALL(objMtcSession, HandleRequest(RequestType::START, IsEqualMessage(pMessage)));

    ImsList<AString> lstRequiredExtensions;
    lstRequiredExtensions.Append(AString("supportedExtension"));
    ON_CALL(objMessageUtils, GetHeaders(pMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(lstRequiredExtensions));
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    ON_CALL(objMessageUtils, HasSdp(pMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    SipMethod objMethod(SipMethod::ACK);
    ON_CALL(*pMessage, GetMethod).WillByDefault(ReturnRef(objMethod));

    const CallReasonInfo objReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE);
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReasonInfo));
    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->OnUssiAttached());

    delete pMessage;
}

TEST_F(IdleStateTest, OnUssiAttachedRejectsIfNoCodecMatched)
{
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    EXPECT_CALL(objMtcSession, HandleRequest(RequestType::START, IsEqualMessage(pMessage)));

    ImsList<AString> lstRequiredExtensions;
    lstRequiredExtensions.Append(AString("supportedExtension"));
    ON_CALL(objMessageUtils, GetHeaders(pMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(lstRequiredExtensions));
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    ON_CALL(objMessageUtils, HasSdp(pMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    SipMethod objMethod(SipMethod::ACK);
    ON_CALL(*pMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    ON_CALL(objMediaManager, NegotiateSdp(&objSession))
            .WillByDefault(Return(SdpNegotiationResult(MEDIA_NEGO_ERROR_NO_CODEC_MATCHED)));

    const CallReasonInfo objReasonInfo(
            CODE_MEDIA_NOT_ACCEPTABLE, MEDIA_NEGO_ERROR_NO_CODEC_MATCHED);
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReasonInfo));
    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->OnUssiAttached());

    delete pMessage;
}

TEST_F(IdleStateTest, OnUssiAttachedRejectsIfInvalidDescriptor)
{
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    EXPECT_CALL(objMtcSession, HandleRequest(RequestType::START, IsEqualMessage(pMessage)));

    ImsList<AString> lstRequiredExtensions;
    lstRequiredExtensions.Append(AString("supportedExtension"));
    ON_CALL(objMessageUtils, GetHeaders(pMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(lstRequiredExtensions));
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    ON_CALL(objMessageUtils, HasSdp(pMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    SipMethod objMethod(SipMethod::ACK);
    ON_CALL(*pMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    ON_CALL(objMediaManager, NegotiateSdp(&objSession))
            .WillByDefault(Return(SdpNegotiationResult(MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR)));

    const CallReasonInfo objReasonInfo(
            CODE_REJECT_UNSUPPORTED_SDP_HEADERS, MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR);
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReasonInfo));
    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->OnUssiAttached());

    delete pMessage;
}

TEST_F(IdleStateTest, OnUssiAttachedTransitsAlertingState)
{
    MockIMessage* pMessage = new MockIMessage();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_START))
            .WillByDefault(Return(pMessage));

    ImsList<AString> lstRequiredExtensions;
    lstRequiredExtensions.Append(AString("supportedExtension"));
    ON_CALL(objMessageUtils, GetHeaders(pMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(lstRequiredExtensions));
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    ON_CALL(objMessageUtils, HasSdp(pMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));

    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived);
    EXPECT_EQ(CallStateName::ALERTING, pIdleState->OnUssiAttached());

    delete pMessage;
}

TEST_F(IdleStateTest, OnBlockCheckedDoesNotInvokeEpsfbIfNotRequired)
{
    objCallInfo.ePeerType = PeerType::MT;
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    const CallReasonInfo objReasonInfo(CODE_ACCESS_CLASS_BLOCKED);  // Can triggers EPSFB
    const IMtcBlockChecker::Result objBlockResult(
            IMtcBlockChecker::Result::Status::BLOCKED, objReasonInfo);

    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReasonInfo));
    EXPECT_CALL(*pEpsfbTrigger, TriggerEpsFallback(_)).Times(0);
    EXPECT_EQ(CallStateName::TERMINATING, pIdleState->OnBlockChecked(objBlockResult));
}

TEST_F(IdleStateTest, OnBlockCheckedTriggersEpsfbIfRequired)
{
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_EPS_FALLBACK_TRIGGER_BY_AC_BARRING_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    const CallReasonInfo objReasonInfo(CODE_ACCESS_CLASS_BLOCKED);
    const IMtcBlockChecker::Result objBlockResult(
            IMtcBlockChecker::Result::Status::BLOCKED, objReasonInfo);

    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);
    EXPECT_CALL(*pEpsfbTrigger, TriggerEpsFallback(EpsFallbackReason::RADIO_CHECK_BLOCK));
    EXPECT_EQ(CallStateName::IDLE, pIdleState->OnBlockChecked(objBlockResult));
}

TEST_F(IdleStateTest, OnTimerExpiredRejectIncomingCallIfAlertingTimerExpired)
{
    const CallReasonInfo objReason(CODE_LOCAL_INTERNAL_ERROR);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReason));

    EXPECT_EQ(CallStateName::TERMINATING,
            pIdleState->OnTimerExpired(MtcCallState::TIMER_MT_ALERTING));
}

TEST_F(IdleStateTest, HandleAosConnectedDoesNothingIfNoEpsFallbackOngoing)
{
    ON_CALL(*pEpsfbTrigger, IsWaitingRegistration()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pEpsfbTrigger, OnEpsFallbackCompleted()).Times(0);
    EXPECT_EQ(CallStateName::IDLE, pIdleState->OnAosStateChanged(MtcAosState::CONNECTED, 0, 0));
}

TEST_F(IdleStateTest, HandleAosConnectedNotifiesEpsFallbackCompletedIfEpsFallbackOngoing)
{
    ON_CALL(*pEpsfbTrigger, IsWaitingRegistration()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));

    EXPECT_CALL(*pEpsfbTrigger, OnEpsFallbackCompleted()).Times(1);
    EXPECT_EQ(CallStateName::IDLE, pIdleState->OnAosStateChanged(MtcAosState::CONNECTED, 0, 0));
}

TEST_F(IdleStateTest, EmergencyCallStartFailsTriggersDeregistration)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    CallType eCallType = CallType::VOIP;
    AString strTarget("911");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_FAILURE));
    ON_CALL(objService, IsEmergency).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objAosConnector, Control(ImsAosControl::REGISTER_STOP)).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_STOP)).Times(1);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);

    EXPECT_EQ(CallStateName::IDLE,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, EmergencyCallStartFailsNullAosConnector)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    CallType eCallType = CallType::VOIP;
    AString strTarget("911");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_FAILURE));
    ON_CALL(objService, IsEmergency).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, GetAosConnector).WillByDefault(Return(nullptr));

    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_STOP)).Times(0);
    EXPECT_CALL(objUiNotifier,
            SendStartFailed(CallReasonInfo(
                    CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY)));

    EXPECT_EQ(CallStateName::TERMINATING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}

TEST_F(IdleStateTest, NonEmergencyCallStartFailsNoDeregistration)
{
    objCallInfo.eEmergencyType = EmergencyType::NONE;
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objCallContext, CreateSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, Start).WillByDefault(Return(IMS_FAILURE));
    ON_CALL(objService, IsEmergency).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_STOP)).Times(0);
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));

    EXPECT_EQ(CallStateName::TERMINATING,
            pIdleState->Start(eCallType, strTarget, objInputMediaInfo, objInputSuppServices));
}
