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
#include "CarrierConfig.h"
#include "IIpcan.h"
#include "INetworkWatcher.h"
#include "ISipClientConnection.h"
#include "ImsTypeDef.h"
#include "MediaDef.h"
#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "MockISipClientConnection.h"
#include "MockISipMessage.h"
#include "MockISipServerConnection.h"
#include "MtcDef.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "aos/ImsAosReason.h"
#include "call/IMtcCall.h"
#include "call/MockEpsFallbackTrigger.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "call/TestMtcPendingOperationHolder.h"
#include "call/UpdatingInfo.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/block/MockIMtcBlockChecker.h"
#include "call/extension/MockIMtcExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/state/EstablishedState.h"
#include "call/state/MtcCallState.h"
#include "conferencecall/MockIConferenceController.h"
#include "conferencecall/MockIConferenceManager.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "emergency/MockCurrentLocationDiscoveryController.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "ussi/MockUssiController.h"
#include "utility/MockIMessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;

MATCHER_P(IsEqualCallReason, reason, "")
{
    return arg == reason;
}

namespace android
{

MATCHER_P(IsSameCallReasonInfoCode, code, "")
{
    return arg.nCode == code;
}

MATCHER(IsEmptyString, "")
{
    return arg.IsEmpty();
}

class EstablishedStateTest : public ::testing::Test
{
public:
    EstablishedState* pEstablishedState;
    MockIMtcCallContext objMockCallContext;
    MockIMtcService objService;
    MockIMtcMediaManager objMockMediaManager;
    MockIMtcSession objMockMtcSession;
    MockIMtcUiNotifier objUiNotifier;
    MockISession objMockISession;
    MockIMtcPreconditionManager objMockPreconditionManager;
    MockMtcTimerWrapper objTimerWrapper;
    MockIMessage objMessage;
    MockIMessageUtils objMessageUtils;
    MockIMtcBlockChecker* pBlockChecker;
    MockMtcConfigurationProxy* pConfigurationProxy;
    UpdatingInfo* pUpdatingInfo;
    MediaInfo objMediaInfo;
    CallInfo objCallInfo;
    MtcSupplementaryService* pSupplementaryService;
    MockIMtcCallManager objMockIMtcCallManager;
    MockUssiController* pMockUssiController;
    MockEpsFallbackTrigger* pEpsFbTrigger;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockCallContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objMockCallContext, GetMediaManager).WillByDefault(ReturnRef(objMockMediaManager));
        ON_CALL(objMockMediaManager, GetMediaInfo(Ref(objMockISession)))
                .WillByDefault(ReturnRef(objMediaInfo));
        ON_CALL(objMockCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objMockCallContext, GetSession()).WillByDefault(Return(&objMockMtcSession));
        ON_CALL(objMockMtcSession, GetISession).WillByDefault(ReturnRef(objMockISession));
        ON_CALL(objMockISession, GetPreviousRequest(_)).WillByDefault(Return(&objMessage));

        ON_CALL(objMockCallContext, GetUiNotifier).WillByDefault(ReturnRef(objUiNotifier));

        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objMockCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        pUpdatingInfo = new UpdatingInfo(objMockCallContext);
        ON_CALL(objMockCallContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));
        ON_CALL(objMockCallContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objMockPreconditionManager));
        ON_CALL(objMockMediaManager, FormSdp(&objMockISession, CallType::VOIP, IMS_TRUE))
                .WillByDefault(Return(IMS_SUCCESS));

        ON_CALL(objMockCallContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        ON_CALL(objMockPreconditionManager, FormPreconditionSdp(_, _)).WillByDefault(Return());

        ON_CALL(objMockCallContext, GetTimer).WillByDefault(ReturnRef(objTimerWrapper));
        pSupplementaryService =
                new MtcSupplementaryService(objMockCallContext, *pConfigurationProxy);
        ON_CALL(objMockCallContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));

        // This will be deleted by EstablishedState.
        pBlockChecker = new MockIMtcBlockChecker();
        ON_CALL(objMockCallContext, CreateBlockChecker).WillByDefault(Return(pBlockChecker));
        ON_CALL(objMockCallContext, GetCallManager)
                .WillByDefault(ReturnRef(objMockIMtcCallManager));
        pMockUssiController = new MockUssiController(objMockCallContext, new UssiDataParser());
        ON_CALL(objMockCallContext, GetUssiController).WillByDefault(Return(pMockUssiController));

        pEpsFbTrigger = new MockEpsFallbackTrigger(objMockCallContext);
        ON_CALL(objMockCallContext, GetEpsFallbackTrigger).WillByDefault(ReturnRef(*pEpsFbTrigger));

        pEstablishedState = new EstablishedState(objMockCallContext);
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pUpdatingInfo;
        delete pEstablishedState;
        delete pSupplementaryService;
        delete pMockUssiController;
        delete pEpsFbTrigger;
    }

    MtcExtensionSet GetTestExtensionSet(IN const AString& strOptionTag1,
            IN const IMS_BOOL& bSupoort1, IN const AString& strOptionTag2 = AString::ConstNull(),
            IN const IMS_BOOL& bSupoort2 = IMS_FALSE)
    {
        ImsList<IMtcExtension*> objExtensions;
        MockIMtcExtension* pExtension1 = new MockIMtcExtension();
        ON_CALL(*pExtension1, GetOptionTag).WillByDefault(ReturnRef(strOptionTag1));
        ON_CALL(*pExtension1, IsAvailableOnRemote).WillByDefault(Return(bSupoort1));
        objExtensions.Append(pExtension1);

        if (!strOptionTag2.IsNull())
        {
            MockIMtcExtension* pExtension2 = new MockIMtcExtension();
            ON_CALL(*pExtension2, GetOptionTag).WillByDefault(ReturnRef(strOptionTag2));
            ON_CALL(*pExtension2, IsAvailableOnRemote).WillByDefault(Return(bSupoort2));
            ON_CALL(*pExtension2, IsRequiredOnRemote).WillByDefault(Return(bSupoort2));
            objExtensions.Append(pExtension2);
        }

        MtcExtensionSet objMtcExtensionSet(objMockCallContext, objExtensions);
        return objMtcExtensionSet;
    }
};

TEST_F(EstablishedStateTest, OnEnterInvokesStartPeriodicLocationDiscovery)
{
    MockCurrentLocationDiscoveryController objLocationController(objMockCallContext);
    ON_CALL(objMockCallContext, GetCurrentLocationDiscoveryController)
            .WillByDefault(ReturnRef(objLocationController));

    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(*pConfigurationProxy,
            GetInt(CarrierConfig::ImsEmergency::KEY_CALL_PERIODIC_LOCATION_DISCOVERY_METHOD_INT))
            .WillByDefault(Return(1));

    EXPECT_CALL(objLocationController, StartPeriodicLocationDiscovery());

    pEstablishedState->OnEnter();
}

TEST_F(EstablishedStateTest, OnEnterRunsPendingOperationSynchronously)
{
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMockCallContext, RunPendingOperationIfPossible());
    pEstablishedState->OnEnter();
}

TEST_F(EstablishedStateTest,
        OnEnterRunsPendingOperationAsynchronouslyIfOnRefreshingOrTimerForDelayingUpdateIsActive)
{
    EXPECT_CALL(objMockISession, IsSessionRefreshInProgress)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(objMockCallContext, RunAsyncOperation(pEstablishedState, _)).Times(2);
    pEstablishedState->OnEnter();
    pEstablishedState->OnEnter();
}

TEST_F(EstablishedStateTest, OnEnterInvokesCreateRttAutoUpgrader)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objMockMtcSession, GetCallType()).WillByDefault(Return(CallType::RTT));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigEmergency::KEY_EMERGENCY_RTT_GUARD_TIMER_MILLIS_INT))
            .WillByDefault(Return(180000));

    EXPECT_CALL(objMockCallContext, CreateRttAutoUpgrader).Times(1);
    pEstablishedState->OnEnter();
}

TEST_F(EstablishedStateTest, TerminateByUserActionWhenNoReceivingAudioPackets)
{
    EXPECT_CALL(objMockMediaManager, IsAudioInactive)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(objMockMtcSession, Terminate(_, _)).Times(2);

    CallReasonInfo objReason(CODE_USER_TERMINATED);
    pEstablishedState->Terminate(objReason);
    pEstablishedState->Terminate(objReason);
}

TEST_F(EstablishedStateTest, TerminateUssiWithUssiCompletedCodeInvokesNormalTerminate)
{
    CallReasonInfo objReason(CODE_INTERNAL_USSI_COMPLETED);
    EXPECT_CALL(objMockMtcSession, Terminate(IMS_TRUE, IsEqualCallReason(objReason)));
    pEstablishedState->TerminateUssi(objReason);
}

TEST_F(EstablishedStateTest, TerminateUssiInvokesSendInfoUssiWithErrorCode1)
{
    CallReasonInfo objReason(CODE_USER_TERMINATED);
    EXPECT_CALL(objMockMtcSession, Terminate(_, _)).Times(0);
    EXPECT_CALL(*pMockUssiController, SendInfo(_, _, UssiError::CODE_1));
    EXPECT_CALL(*pMockUssiController, SetNextActionByTerminateUssi);

    pEstablishedState->TerminateUssi(objReason);
}

TEST_F(EstablishedStateTest, SessionTerminatedInvokesSendTerminated)
{
    ImsList<IMtcCall*> objEmptyCalls;
    ON_CALL(objMockIMtcCallManager, GetCallsInConference).WillByDefault(Return(objEmptyCalls));
    ON_CALL(objMockISession, GetTerminationReason)
            .WillByDefault(Return(ISession::TERMINATION_REASON_USER_ACTION));
    EXPECT_CALL(objUiNotifier, SendTerminated(IsSameCallReasonInfoCode(CODE_USER_TERMINATED)));

    pEstablishedState->SessionTerminated(&objMockISession);
}

TEST_F(EstablishedStateTest, SessionTerminatedInvokesSendTerminatedWithConf)
{
    MockIConferenceManager objMockConferenceManager;
    MockIConferenceController objMockConferenceController;
    ON_CALL(objMockCallContext, GetConferenceManager())
            .WillByDefault(ReturnRef(objMockConferenceManager));
    ON_CALL(objMockConferenceManager, GetController(_))
            .WillByDefault(Return(&objMockConferenceController));
    ON_CALL(objMockConferenceController, GetCallStatusInConference(_))
            .WillByDefault(Return(IndividualCallState::JOINING));

    ImsList<IMtcCall*> objConfCalls;
    MockIMtcCall objMockMtcCall;
    objConfCalls.Append(&objMockMtcCall);
    ON_CALL(objMockIMtcCallManager, GetCallsInConference).WillByDefault(Return(objConfCalls));
    ON_CALL(objMockISession, GetTerminationReason)
            .WillByDefault(Return(ISession::TERMINATION_REASON_USER_ACTION));
    EXPECT_CALL(objUiNotifier,
            SendTerminated(IsSameCallReasonInfoCode(CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE)));

    pEstablishedState->SessionTerminated(&objMockISession);
}

TEST_F(EstablishedStateTest, RefreshNotifyCompletedRunsPendingOperationAsynchronously)
{
    EXPECT_CALL(objMockCallContext, RunAsyncOperation(pEstablishedState, _));

    ISipClientConnection* piFakeConnection = reinterpret_cast<ISipClientConnection*>(0x1);
    pEstablishedState->Refresh_NotifyCompleted(piFakeConnection);
}

TEST_F(EstablishedStateTest, OnReceivingMediaDataFailedWithAudioTerminatesCall)
{
    EXPECT_CALL(objService, IsWlanIpCanType).WillOnce(Return(IMS_TRUE)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockMtcSession,
            Terminate(IMS_TRUE, IsEqualCallReason(CallReasonInfo(CODE_MEDIA_NO_DATA))))
            .Times(2);
    EXPECT_CALL(
            objUiNotifier, SendTerminated(IsEqualCallReason(CallReasonInfo(CODE_MEDIA_NO_DATA))))
            .Times(2);

    EXPECT_EQ(CallStateName::TERMINATING,
            pEstablishedState->OnReceivingMediaDataFailed(MEDIATYPE_AUDIO, MEDIA_PROTOCOL_RTP));
    EXPECT_EQ(CallStateName::TERMINATING,
            pEstablishedState->OnReceivingMediaDataFailed(MEDIATYPE_AUDIO, MEDIA_PROTOCOL_RTP));
}

TEST_F(EstablishedStateTest, OnReceivingMediaDataFailedWithAudioOverridesReasonHeader)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigWfc::KEY_OVERRIDE_MEDIA_INACTIVITY_TO_WIFI_LOST_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objService, IsWlanIpCanType).WillOnce(Return(IMS_TRUE)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockMtcSession,
            Terminate(IMS_TRUE, IsEqualCallReason(CallReasonInfo(CODE_WIFI_LOST))));
    EXPECT_CALL(objMockMtcSession,
            Terminate(IMS_TRUE, IsEqualCallReason(CallReasonInfo(CODE_MEDIA_NO_DATA))));
    EXPECT_CALL(objUiNotifier, SendTerminated(IsEqualCallReason(CallReasonInfo(CODE_WIFI_LOST))));
    EXPECT_CALL(
            objUiNotifier, SendTerminated(IsEqualCallReason(CallReasonInfo(CODE_MEDIA_NO_DATA))));

    EXPECT_EQ(CallStateName::TERMINATING,
            pEstablishedState->OnReceivingMediaDataFailed(MEDIATYPE_AUDIO, MEDIA_PROTOCOL_RTP));
    EXPECT_EQ(CallStateName::TERMINATING,
            pEstablishedState->OnReceivingMediaDataFailed(MEDIATYPE_AUDIO, MEDIA_PROTOCOL_RTP));
}

TEST_F(EstablishedStateTest, OnReceivingMediaDataFailedWithVideoInvokesDowngrade)
{
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtcSession, GetCallType).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _));

    pEstablishedState->OnReceivingMediaDataFailed(MEDIATYPE_VIDEO, MEDIA_PROTOCOL_RTP);
}

TEST_F(EstablishedStateTest, OnReceivingMediaDataFailedWithTextInvokesDowngrade)
{
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtcSession, GetCallType).WillByDefault(Return(CallType::RTT));

    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _));

    pEstablishedState->OnReceivingMediaDataFailed(MEDIATYPE_TEXT, MEDIA_PROTOCOL_RTP);
}

TEST_F(EstablishedStateTest, OnVideoLowestBitRateInvokesDowngradeIfCallTypeIsVt)
{
    ON_CALL(objMockMtcSession, GetCallType).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objMockMtcSession, SetCallType(CallType::VOIP));
    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _)).Times(1);

    pEstablishedState->OnVideoLowestBitRate();
}

TEST_F(EstablishedStateTest, OnVideoLowestBitRateInvokesDowngradeIfCallTypeIsVideoRtt)
{
    ON_CALL(objMockMtcSession, GetCallType).WillByDefault(Return(CallType::VIDEO_RTT));

    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objMockMtcSession, SetCallType(CallType::RTT));
    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _)).Times(1);

    pEstablishedState->OnVideoLowestBitRate();
}

TEST_F(EstablishedStateTest, OnMediaFailed)
{
    EXPECT_CALL(objMockMtcSession, Terminate(IMS_TRUE, CallReasonInfo(CODE_MEDIA_INIT_FAILED)))
            .Times(1);

    pEstablishedState->OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED));
}

TEST_F(EstablishedStateTest, QosReserveFailedInvokesSendTerminated)
{
    EXPECT_CALL(objMockMtcSession,
            Terminate(IMS_TRUE, CallReasonInfo(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED)))
            .Times(1);
    EXPECT_CALL(objUiNotifier,
            SendTerminated(CallReasonInfo(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED)));

    EXPECT_EQ(CallStateName::TERMINATING,
            pEstablishedState->QosReserveFailed(&objMockISession, QosLossPolicy::RELEASE));
}

TEST_F(EstablishedStateTest, QosReserveFailedDowngradesVtCallToVoipCall)
{
    ON_CALL(objMockMtcSession, GetCallType).WillByDefault(Return(CallType::VT));
    EXPECT_CALL(objMockMtcSession, SetCallType(CallType::VOIP));
    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE));

    EXPECT_EQ(CallStateName::UPDATING,
            pEstablishedState->QosReserveFailed(&objMockISession, QosLossPolicy::MODIFY));
}

TEST_F(EstablishedStateTest, OnIpcanChangedNotHandledIfConfigurationIsOff)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ENABLE_SEND_REINVITE_ON_RAT_CHANGE_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnIpcanChanged(_)).Times(0);

    pEstablishedState->OnIpcanChanged(IIpcan::CATEGORY_ANY);
}

TEST_F(EstablishedStateTest, OnIpcanChangedSendsUpdateIfNoNeedToPendAndConfigurationIsOn)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ENABLE_SEND_REINVITE_ON_RAT_CHANGE_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillRepeatedly(Return(IMS_FALSE));

    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnIpcanChanged(_)).Times(0);
    EXPECT_CALL(objMockMtcSession, Update(UpdateType::NORMAL, _, _));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _));

    pEstablishedState->OnIpcanChanged(IIpcan::CATEGORY_ANY);
}

TEST_F(EstablishedStateTest, OnIpcanChangedPushesPendingOperation)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ENABLE_SEND_REINVITE_ON_RAT_CHANGE_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMockISession, IsSessionRefreshInProgress)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnIpcanChanged(IIpcan::CATEGORY_ANY)).Times(2);

    pEstablishedState->OnIpcanChanged(IIpcan::CATEGORY_ANY);
    pEstablishedState->OnIpcanChanged(IIpcan::CATEGORY_ANY);
}

TEST_F(EstablishedStateTest, OnIpcanChangedHandlesFailure)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ENABLE_SEND_REINVITE_ON_RAT_CHANGE_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtcSession, Update(_, _, _)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_EQ(CallStateName::ESTABLISHED, pEstablishedState->OnIpcanChanged(IIpcan::CATEGORY_ANY));
}

TEST_F(EstablishedStateTest, OnTimerExpiredBringsAsyncRunner)
{
    EXPECT_CALL(objMockCallContext, RunAsyncOperation(pEstablishedState, _));

    pEstablishedState->OnTimerExpired(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED);
}

TEST_F(EstablishedStateTest, SendUpdateBySrvccByCanceled)
{
    EXPECT_CALL(objMockMtcSession,
            Update(UpdateType::SRVCC_RECOVERED_CANCEL, IMS_FALSE, SipMethod::INVITE))
            .Times(1);

    EXPECT_EQ(
            CallStateName::UPDATING, pEstablishedState->OnSrvccStateUpdated(SrvccState::CANCELED));
}

TEST_F(EstablishedStateTest, SendUpdateBySrvccByFailed)
{
    EXPECT_CALL(objMockMtcSession,
            Update(UpdateType::SRVCC_RECOVERED_FAILURE, IMS_FALSE, SipMethod::INVITE))
            .Times(1);

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->OnSrvccStateUpdated(SrvccState::FAILED));
}

TEST_F(EstablishedStateTest, SendUpdateBySrvccHandlesFailure)
{
    ON_CALL(objMockMtcSession, Update(_, _, _)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_EQ(CallStateName::ESTABLISHED,
            pEstablishedState->OnSrvccStateUpdated(SrvccState::CANCELED));
}

TEST_F(EstablishedStateTest, SendOfferWithFullCapaOnResponseToReInvite)
{
    ON_CALL(objMessageUtils, GetCallTypeFromSdp(_, _, _, _))
            .WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    SipMethod objSipMethod(SipMethod::INVITE);
    ON_CALL(objMessage, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(objMockCallContext, IsHeldByMe).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtcSession, GetCallType()).WillByDefault(Return(CallType::VOIP));

    EXPECT_CALL(objMockMtcSession, AcceptUpdate()).Times(1).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SendIncomingUpdateIsInvokedIfUpdateNeedsToAlert)
{
    ON_CALL(objMessageUtils, GetCallType(_, _, _)).WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));

    // SetUp IsNeedToAlert() true.
    ON_CALL(objMockMtcSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMockMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));

    MtcExtensionSet objMtcExtensionSet(
            GetTestExtensionSet(MtcExtensionSet::OPTION_TAG_RPR, IMS_FALSE));
    ON_CALL(objMockMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_));

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SendProvisionalResponseIsInvokedIfPreconditionIsSupported)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT))
            .WillByDefault(
                    Return(ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_DURING_UPGRADING));

    ON_CALL(objMessageUtils, GetCallTypeFromSdp(_, _, _, _))
            .WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));

    // SetUp IsNeedToAlert() true.
    ON_CALL(objMockMtcSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMockMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));

    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(MtcExtensionSet::OPTION_TAG_PRECONDITION,
            IMS_TRUE, MtcExtensionSet::OPTION_TAG_RPR, IMS_TRUE));
    ON_CALL(objMockMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(0);
    EXPECT_CALL(objMockMtcSession, SendProvisionalResponse(IMS_FALSE, IMS_TRUE));

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SendProvisionalResponseIsNotInvokedIfRprIsNotSupported)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT))
            .WillByDefault(
                    Return(ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_DURING_UPGRADING));

    ON_CALL(objMessageUtils, GetCallTypeFromSdp(_, _, _, _))
            .WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));

    // SetUp IsNeedToAlert() true.
    ON_CALL(objMockMtcSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMockMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));

    MtcExtensionSet objMtcExtensionSet(
            GetTestExtensionSet(MtcExtensionSet::OPTION_TAG_RPR, IMS_FALSE));

    ON_CALL(objMockMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(1);
    EXPECT_CALL(objMockMtcSession, SendProvisionalResponse(IMS_FALSE, _)).Times(0);

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SendProvisionalResponseIsNotInvokedIfPreconditionIsNotSupported)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT))
            .WillByDefault(
                    Return(ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_DURING_UPGRADING));

    ON_CALL(objMessageUtils, GetCallTypeFromSdp(_, _, _, _))
            .WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));

    // SetUp IsNeedToAlert() true.
    ON_CALL(objMockMtcSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMockMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));

    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(MtcExtensionSet::OPTION_TAG_PRECONDITION,
            IMS_FALSE, MtcExtensionSet::OPTION_TAG_RPR, IMS_TRUE));
    ON_CALL(objMockMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(1);
    EXPECT_CALL(objMockMtcSession, SendProvisionalResponse(IMS_FALSE, _)).Times(0);

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SessionUpdateReceivedAcceptsByPreviousCallTypeIfConfiguredCodeIs200)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT))
            .WillByDefault(Return(SipStatusCode::SC_200));
    ON_CALL(objMockMtcSession, GetPreviousCallType).WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, GetCallTypeFromSdp(_, _, _, _))
            .WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    CallReasonInfo objAnyReason(CODE_UNSPECIFIED);
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(Return(IMtcBlockChecker::Result(
                    IMtcBlockChecker::Result::Status::BLOCKED, objAnyReason)));
    ON_CALL(objMockMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(STATE_OFFER_RECEIVED));

    EXPECT_CALL(objMockMtcSession, SetCallType(CallType::VT));
    EXPECT_CALL(objMockMtcSession, AcceptUpdate);
    EXPECT_CALL(objMockMediaManager, FinalizeSdp(&objMockISession));

    EXPECT_EQ(
            CallStateName::ESTABLISHED, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SessionUpdateReceivedRejectsIfBlocked)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT))
            .WillByDefault(Return(SipStatusCode::SC_603));
    ON_CALL(objMockMtcSession, GetPreviousCallType).WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, GetCallTypeFromSdp(_, _, _, _))
            .WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    CallReasonInfo objAnyReason(CODE_UNSPECIFIED);
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(Return(IMtcBlockChecker::Result(
                    IMtcBlockChecker::Result::Status::BLOCKED, objAnyReason)));
    ON_CALL(objMockMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(STATE_OFFER_RECEIVED));

    EXPECT_CALL(objMockMtcSession, SetCallType(CallType::VT));
    EXPECT_CALL(objMockMtcSession, Reject(objAnyReason));
    EXPECT_CALL(objMockMediaManager, FinalizeSdp(&objMockISession));

    EXPECT_EQ(
            CallStateName::ESTABLISHED, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest,
        SessionUpdateReceivedAcceptsByPreviousCallTypeIfConfiguredCodeIs200WithSdp)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT))
            .WillByDefault(Return(SipStatusCode::SC_200));
    ON_CALL(objMockMtcSession, GetPreviousCallType).WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, GetCallTypeFromSdp(_, _, _, _))
            .WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    CallReasonInfo objAnyReason(CODE_UNSPECIFIED);
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(Return(IMtcBlockChecker::Result(
                    IMtcBlockChecker::Result::Status::BLOCKED, objAnyReason)));
    ON_CALL(objMockMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMockMediaManager, NegotiateSdp(&objMockISession))
            .WillOnce(Return(SdpNegotiationResult(MEDIA_NEGO_NO_ERROR)));
    EXPECT_CALL(objMockMtcSession, SetCallType(CallType::VT));
    EXPECT_CALL(objMockMtcSession, AcceptUpdate);
    EXPECT_CALL(objMockMediaManager, FinalizeSdp(&objMockISession));

    EXPECT_EQ(
            CallStateName::ESTABLISHED, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SessionUpdateReceivedRejectsIfMediaNegoFailed)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT))
            .WillByDefault(Return(SipStatusCode::SC_603));
    ON_CALL(objMockMtcSession, GetPreviousCallType).WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objMockMediaManager, GetNegotiationState(_)).WillByDefault(Return(STATE_IDLE));

    const SdpNegotiationResult objNegoResult(MEDIA_NEGO_ERROR_NO_CODEC_MATCHED);
    CallReasonInfo objMediaNegoFailReason(CODE_MEDIA_NOT_ACCEPTABLE, objNegoResult.eResult);
    ON_CALL(objMockMediaManager, NegotiateSdp(&objMockISession))
            .WillByDefault(Return(objNegoResult));
    EXPECT_CALL(objMockMtcSession, SetCallType(CallType::VT));
    EXPECT_CALL(objMockMtcSession, Reject(objMediaNegoFailReason));
    EXPECT_CALL(objMockMediaManager, FinalizeSdp(&objMockISession));

    EXPECT_EQ(
            CallStateName::ESTABLISHED, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SessionUpdateReceivedRejectsIfMediaNegoFailedWithInvalidDescriptor)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT))
            .WillByDefault(Return(SipStatusCode::SC_603));
    ON_CALL(objMockMtcSession, GetPreviousCallType).WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objMockMediaManager, GetNegotiationState(_)).WillByDefault(Return(STATE_IDLE));

    const SdpNegotiationResult objNegoResult(MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR);
    MediaNegoResult eNegoFailReason = MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR;
    CallReasonInfo objMediaNegoFailReason(CODE_REJECT_UNSUPPORTED_SDP_HEADERS, eNegoFailReason);
    ON_CALL(objMockMediaManager, NegotiateSdp(&objMockISession))
            .WillByDefault(Return(objNegoResult));
    EXPECT_CALL(objMockMtcSession, SetCallType(CallType::VT));
    EXPECT_CALL(objMockMtcSession, Reject(objMediaNegoFailReason));
    EXPECT_CALL(objMockMediaManager, FinalizeSdp(&objMockISession));

    EXPECT_EQ(
            CallStateName::ESTABLISHED, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SessionUpdateReceivedInvokesSendIncomingResume)
{
    delete pUpdatingInfo;
    ON_CALL(objMockMtcSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    pUpdatingInfo = new UpdatingInfo(objMockCallContext);
    ON_CALL(objMockCallContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objMockMediaManager, NegotiateSdp(&objMockISession))
            .WillByDefault(Return(SdpNegotiationResult(MEDIA_NEGO_NO_ERROR)));
    ON_CALL(objMockMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VOIP));
    objMediaInfo.eAudioDirection = DIRECTION_RECEIVE;
    pUpdatingInfo->GetModifiedInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_CHECK_UI_CONDITION_FOR_INCOMING_RESUME_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMockPreconditionManager, OnSdpReceived(&objMockISession)).Times(1);
    EXPECT_CALL(objUiNotifier, SendIncomingResume).Times(1);
    EXPECT_CALL(objTimerWrapper, Start(_, _)).Times(1);

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest,
        SessionUpdateReceivedAnswersDirectlyWithoutInteractionWithAnotherModule)
{
    delete pUpdatingInfo;
    ON_CALL(objMockMtcSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    pUpdatingInfo = new UpdatingInfo(objMockCallContext);
    ON_CALL(objMockCallContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));

    // Hold, Resume without interaction with Java side, Refresh
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objMockMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VOIP));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_CHECK_UI_CONDITION_FOR_INCOMING_RESUME_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    SipMethod objSipMethod(SipMethod::UPDATE);
    ON_CALL(objMessage, GetMethod()).WillByDefault(ReturnRef(objSipMethod));

    EXPECT_CALL(objMockMtcSession, AcceptUpdate()).Times(1);
    EXPECT_CALL(objMockMediaManager, Run(_, _, _)).Times(1);
    EXPECT_EQ(
            CallStateName::ESTABLISHED, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, UssiTerminateInvokesCheckingUssiBodyAndSendTerminated)
{
    MockISipMessage objMockISipMessage;
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    SipMethod objSipMethod(SipMethod::PUBLISH);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objSipMethod));
    UssiResult objResult(UssiNextAction::NOTHING, UssiError::CODE_NONE);
    EXPECT_CALL(*pMockUssiController, HandleUssiBody(_, _)).Times(1).WillOnce(Return(objResult));
    EXPECT_CALL(objUiNotifier, SendTerminated(_)).Times(1);

    pEstablishedState->UssiTerminated(&objMockISession);
}

TEST_F(EstablishedStateTest, UssiInfoReceivedInvokesJustTransactionResponseIfNotInfoMethod)
{
    MockISipServerConnection objMockISipServerConnection;
    SipMethod objSipMethod(SipMethod::PUBLISH);
    ON_CALL(objMockISipServerConnection, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    EXPECT_CALL(objMockISipServerConnection, InitResponse(SipStatusCode::SC_200));
    EXPECT_CALL(objMockISipServerConnection, Send);
    EXPECT_CALL(objMockISipServerConnection, Close);
    EXPECT_CALL(*pMockUssiController, HandleUssiBody(_, _)).Times(0);

    pEstablishedState->UssiInfoReceived(&objMockISession, &objMockISipServerConnection);
}

TEST_F(EstablishedStateTest, UssiInfoReceivedInvokesJustTransactionResponseIfNotValidInfoMethod)
{
    MockISipServerConnection objMockISipServerConnection;
    SipMethod objSipMethod(SipMethod::INFO);
    ON_CALL(objMockISipServerConnection, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(*pMockUssiController, IsUssiInfoReceived(_)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pMockUssiController, HasXmlBodyInInfo(_)).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMockISipServerConnection, InitResponse(SipStatusCode::SC_469));
    EXPECT_CALL(objMockISipServerConnection, Send);
    EXPECT_CALL(objMockISipServerConnection, Close);
    EXPECT_CALL(*pMockUssiController, HandleUssiBody(_, _)).Times(0);

    pEstablishedState->UssiInfoReceived(&objMockISession, &objMockISipServerConnection);
}

TEST_F(EstablishedStateTest,
        UssiInfoReceivedInvokesTransactionResponseAndSendInfoForUssiWithErrorCode)
{
    MockISipServerConnection objMockISipServerConnection;
    SipMethod objSipMethod(SipMethod::INFO);
    ON_CALL(objMockISipServerConnection, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(*pMockUssiController, IsUssiInfoReceived(_)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pMockUssiController, HasXmlBodyInInfo(_)).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMockISipServerConnection, InitResponse(SipStatusCode::SC_200));
    EXPECT_CALL(objMockISipServerConnection, Send);
    EXPECT_CALL(objMockISipServerConnection, Close);
    UssiResult objUssiResult(UssiNextAction::SEND_INFO_WITH_ERROR_CODE, UssiError::CODE_1);
    EXPECT_CALL(*pMockUssiController, HandleUssiBody(_, _))
            .Times(1)
            .WillOnce(Return(objUssiResult));

    pEstablishedState->UssiInfoReceived(&objMockISession, &objMockISipServerConnection);
}

TEST_F(EstablishedStateTest, UssiInfoReceivedInvokesTransactionResponseAndSendInfoForUssi)
{
    MockISipServerConnection objMockISipServerConnection;
    SipMethod objSipMethod(SipMethod::INFO);
    ON_CALL(objMockISipServerConnection, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(*pMockUssiController, IsUssiInfoReceived(_)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pMockUssiController, HasXmlBodyInInfo(_)).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMockISipServerConnection, InitResponse(SipStatusCode::SC_200));
    EXPECT_CALL(objMockISipServerConnection, Send);
    EXPECT_CALL(objMockISipServerConnection, Close);
    UssiResult objUssiResult(UssiNextAction::SEND_INFO_WITH_NOTIFY_ELEMENT, UssiError::CODE_NONE);
    EXPECT_CALL(*pMockUssiController, HandleUssiBody(_, _))
            .Times(1)
            .WillOnce(Return(objUssiResult));

    pEstablishedState->UssiInfoReceived(&objMockISession, &objMockISipServerConnection);
}

TEST_F(EstablishedStateTest, UssiInfoReceivedInvokesJustTransactionResponseIfNoProperAction)
{
    MockISipServerConnection objMockISipServerConnection;
    SipMethod objSipMethod(SipMethod::INFO);
    ON_CALL(objMockISipServerConnection, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(*pMockUssiController, IsUssiInfoReceived(_)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pMockUssiController, HasXmlBodyInInfo(_)).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMockISipServerConnection, InitResponse(SipStatusCode::SC_200));
    EXPECT_CALL(objMockISipServerConnection, Send);
    EXPECT_CALL(objMockISipServerConnection, Close);
    UssiResult objUssiResult(UssiNextAction::NOTHING, UssiError::CODE_NONE);
    EXPECT_CALL(*pMockUssiController, HandleUssiBody(_, _))
            .Times(1)
            .WillOnce(Return(objUssiResult));

    pEstablishedState->UssiInfoReceived(&objMockISession, &objMockISipServerConnection);
}

TEST_F(EstablishedStateTest, OnReceivingNetworkToneStartedAndFailedInvokesSendHeldBy)
{
    EXPECT_CALL(objUiNotifier, SendHeldBy).Times(2);
    EXPECT_EQ(CallStateName::ESTABLISHED, pEstablishedState->OnReceivingNetworkToneStarted());
    EXPECT_EQ(CallStateName::ESTABLISHED, pEstablishedState->OnReceivingNetworkToneFailed());
}

TEST_F(EstablishedStateTest,
        UpdatePushesPendingOperationDuringRefreshOrTimerForDelayingUpdateIsActive)
{
    EXPECT_CALL(objMockISession, IsSessionRefreshInProgress)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    EXPECT_CALL(objPendingOperationHolder.GetMock(), Update(CallType::VT, objMediaInfo)).Times(2);

    pEstablishedState->Update(CallType::VT, objMediaInfo);
    pEstablishedState->Update(CallType::VT, objMediaInfo);
}

TEST_F(EstablishedStateTest, UpdateDoesNotRefreshConvertRemoteResponseTimer)
{
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE))
            .Times(3)
            .WillRepeatedly(Return(IMS_SUCCESS));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .Times(3)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _)).Times(1);

    pEstablishedState->Update(CallType::VT, objMediaInfo);
    pEstablishedState->Update(CallType::VT, objMediaInfo);
    pEstablishedState->Update(CallType::VT, objMediaInfo);
}

TEST_F(EstablishedStateTest,
        HandleAosDisconnectedWithRegTerminatingInvokesTerminateWithLocalServiceUnavailable)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

    MockEpsFallbackTrigger* pEpsFbTrigger = new MockEpsFallbackTrigger(objMockCallContext);
    ON_CALL(objMockCallContext, GetEpsFallbackTrigger).WillByDefault(ReturnRef(*pEpsFbTrigger));
    ON_CALL(*pEpsFbTrigger, IsWaitingRegistration).WillByDefault(Return(IMS_FALSE));

    const CallReasonInfo objReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE);
    EXPECT_CALL(objMockMtcSession, Terminate(IMS_TRUE, objReasonInfo));
    EXPECT_CALL(objUiNotifier, SendTerminated(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING,
            pEstablishedState->OnAosStateChanged(
                    MtcAosState::DISCONNECTED, ImsAosReason::REG_TERMINATING, 0));
}

TEST_F(EstablishedStateTest,
        HandleAosDisconnectedWithAirplaneModeInvokesTerminateWithWifiLostIfIwlan)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));
    ON_CALL(*pEpsFbTrigger, IsWaitingEpsFallback).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objService, GetLastConnectedRatType)
            .WillByDefault(Return(INetworkWatcher::RADIOTECH_TYPE_IWLAN));

    const CallReasonInfo objReasonInfo(CODE_WIFI_LOST);
    EXPECT_CALL(objMockMtcSession, Terminate(IMS_TRUE, objReasonInfo));
    EXPECT_CALL(objUiNotifier, SendTerminated(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING,
            pEstablishedState->OnAosStateChanged(
                    MtcAosState::DISCONNECTED, ImsAosReason::WIFI_OFF, 0));
}

TEST_F(EstablishedStateTest,
        HandleAosDisconnectedWithAirplaneModeInvokesTerminateWithRadioOffIfNotIwlan)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));
    ON_CALL(*pEpsFbTrigger, IsWaitingEpsFallback).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objService, GetLastConnectedRatType)
            .WillByDefault(Return(INetworkWatcher::RADIOTECH_TYPE_LTE));

    const CallReasonInfo objReasonInfo(CODE_RADIO_OFF);
    EXPECT_CALL(objMockMtcSession, Terminate(IMS_TRUE, objReasonInfo));
    EXPECT_CALL(objUiNotifier, SendTerminated(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING,
            pEstablishedState->OnAosStateChanged(
                    MtcAosState::DISCONNECTED, ImsAosReason::AIRPLANE_MODE, 0));
}

TEST_F(EstablishedStateTest,
        HandleAosDisconnectedWithAirplaneModeInvokesTerminateWithOemCause3IfCst)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));
    ON_CALL(*pEpsFbTrigger, IsWaitingEpsFallback).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objService, IsCrossSimConnected).WillByDefault(Return(IMS_TRUE));

    const CallReasonInfo objReasonInfo(CODE_OEM_CAUSE_3);
    EXPECT_CALL(objMockMtcSession, Terminate(IMS_TRUE, objReasonInfo));
    EXPECT_CALL(objUiNotifier, SendTerminated(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING,
            pEstablishedState->OnAosStateChanged(
                    MtcAosState::DISCONNECTED, ImsAosReason::AIRPLANE_MODE, 0));
}

TEST_F(EstablishedStateTest, HandleAosDisconnectedWithWifiOffInvokesTerminateWithWifiLost)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));
    ON_CALL(*pEpsFbTrigger, IsWaitingEpsFallback).WillByDefault(Return(IMS_FALSE));

    const CallReasonInfo objReasonInfo(CODE_WIFI_LOST);
    EXPECT_CALL(objMockMtcSession, Terminate(IMS_TRUE, objReasonInfo));
    EXPECT_CALL(objUiNotifier, SendTerminated(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING,
            pEstablishedState->OnAosStateChanged(
                    MtcAosState::DISCONNECTED, ImsAosReason::WIFI_OFF, 0));
}

TEST_F(EstablishedStateTest, HandleAosDisconnectedWithDataFailCauseReason)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));
    ON_CALL(*pEpsFbTrigger, IsWaitingEpsFallback).WillByDefault(Return(IMS_FALSE));

    IMS_SINT32 nDataFailureReason = 1;
    EXPECT_CALL(objMockMtcSession,
            Terminate(IMS_TRUE, CallReasonInfo(CODE_NETWORK_DETACH, nDataFailureReason)));
    EXPECT_CALL(
            objUiNotifier, SendTerminated(CallReasonInfo(CODE_NETWORK_DETACH, nDataFailureReason)));
    EXPECT_EQ(CallStateName::TERMINATING,
            pEstablishedState->OnAosStateChanged(MtcAosState::DISCONNECTED,
                    ImsAosReason::DATA_DISCONNECTED, nDataFailureReason));
}

TEST_F(EstablishedStateTest, HoldPushesPendingOperation)
{
    EXPECT_CALL(objMockISession, IsSessionRefreshInProgress)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    EXPECT_CALL(objPendingOperationHolder.GetMock(), Hold(objMediaInfo)).Times(2);

    pEstablishedState->Hold(objMediaInfo);
    pEstablishedState->Hold(objMediaInfo);
}

TEST_F(EstablishedStateTest, HoldInvokesSendHeldWithAudioDirectionActive)
{
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));
    objMediaInfo.eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_CALL(objMockCallContext, SetHeldByMe(IMS_TRUE));
    EXPECT_CALL(objUiNotifier, SendHeld());

    pEstablishedState->Hold(objMediaInfo);
}

TEST_F(EstablishedStateTest, HoldInvokesHoldFailed)
{
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));

    objMediaInfo.eAudioDirection = DIRECTION_RECEIVE;
    MediaInfo objNewMediaInfo;
    objNewMediaInfo.eAudioDirection = DIRECTION_SEND;
    EXPECT_CALL(objUiNotifier, SendHoldFailed(CallReasonInfo(CODE_SUPP_SVC_FAILED)));

    pEstablishedState->Hold(objNewMediaInfo);
}

TEST_F(EstablishedStateTest, HoldInvokesHandleUpdate)
{
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMockMtcSession, Update(UpdateType::HOLD, IMS_FALSE, SipMethod::INVITE));

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->Hold(objMediaInfo));
    EXPECT_EQ(UpdateType::HOLD, pUpdatingInfo->GetRequestingType());
}

TEST_F(EstablishedStateTest, HoldHandlesFailure)
{
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtcSession, Update(_, _, _)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objUiNotifier, SendHoldFailed(CallReasonInfo(CODE_SUPP_SVC_FAILED)));
    EXPECT_EQ(CallStateName::ESTABLISHED, pEstablishedState->Hold(objMediaInfo));
}

TEST_F(EstablishedStateTest, ResumePushesPendingOperation)
{
    EXPECT_CALL(objMockISession, IsSessionRefreshInProgress)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    EXPECT_CALL(objPendingOperationHolder.GetMock(), Resume(objMediaInfo)).Times(2);

    pEstablishedState->Resume(objMediaInfo);
    pEstablishedState->Resume(objMediaInfo);
}

TEST_F(EstablishedStateTest, ResumeInvokesResumeFailed)
{
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));

    objMediaInfo.eAudioDirection = DIRECTION_INACTIVE;
    MediaInfo objNewMediaInfo;
    objNewMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    EXPECT_CALL(objUiNotifier, SendResumeFailed(CallReasonInfo(CODE_SUPP_SVC_FAILED)));

    pEstablishedState->Resume(objNewMediaInfo);
}

TEST_F(EstablishedStateTest, ResumeInvokesHandleUpdate)
{
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMockMtcSession, Update(UpdateType::RESUME, IMS_FALSE, SipMethod::INVITE));

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->Resume(objMediaInfo));
    EXPECT_EQ(UpdateType::RESUME, pUpdatingInfo->GetRequestingType());
}

TEST_F(EstablishedStateTest, ResumeHandlesFailure)
{
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtcSession, Update(_, _, _)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objUiNotifier, SendResumeFailed(CallReasonInfo(CODE_SUPP_SVC_FAILED)));
    EXPECT_EQ(CallStateName::ESTABLISHED, pEstablishedState->Resume(objMediaInfo));
}

}  // namespace android
