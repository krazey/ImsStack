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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "AString.h"
#include "ImsMap.h"
#include "CarrierConfig.h"
#include "Credential.h"
#include "IImsRadio.h"
#include "IIpcan.h"
#include "INetworkWatcher.h"
#include "MockIThread.h"
#include "MockITimer.h"
#include "PlatformContext.h"
#include "SipStatusCode.h"
#include "TestThreadService.h"
#include "TestTimerService.h"

#include "../../../config/interface/ImsServiceConfig.h"
#include "../../../config/interface/common/MockIConfigurable.h"
#include "../../../config/interface/common/MockISipConfigV.h"
#include "../../../engine/interface/sipcore/MockISipMessage.h"
#include "../../../engine/interface/registration/MockIRegistration.h"
#include "../../../engine/interface/registration/MockIRegContact.h"
#include "../../../engine/interface/registration/MockIRegParameter.h"
#include "../../../engine/interface/registration/MockIRegSubscription.h"

#include "../../../enabler/interface/aos/AoSAppRequestType.h"
#include "../../../enabler/interface/aos/ImsAosParameter.h"

#include "../../interface/aos/MockIAosService.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosHandle.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosPcscf.h"
#include "interface/MockIAosRegistrationListener.h"
#include "interface/MockIAosRetryRepository.h"
#include "interface/MockIAosSubscriber.h"
#include "interface/MockIAosTransaction.h"
#include "registration/MockAosIpsecHelper.h"
#include "registration/MockAosSubscription.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "handle/AosFeatureTag.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "provider/AosString.h"
#include "registration/AosIpsecHelper.h"
#include "registration/AosRegistration.h"
#include "registration/AosSubscription.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArgReferee;

const IMS_SINT32 SLOT_ID = 0;

class TestAosRegistration : public AosRegistration
{
public:
    explicit inline TestAosRegistration(IN IAosAppContext* piAppContext, IN AString& strRegId) :
            AosRegistration(piAppContext, strRegId),
            m_piRegistrationInstance(IMS_NULL),
            m_pSubscriptionInstance(IMS_NULL)
    {
    }
    inline void SetMockRegistration(IN IRegistration* piReg) { m_piRegistrationInstance = piReg; }
    inline void SetMockSubscription(IN AosSubscription* pSubs) { m_pSubscriptionInstance = pSubs; }

    FRIEND_TEST(AosRegistrationTest, StartWhileTransactionIsNotStartedNotTriesCreateRegistration);
    FRIEND_TEST(AosRegistrationTest, StartWhileTransactionIsNotAllowedNotTriesCreateRegistration);
    FRIEND_TEST(AosRegistrationTest, StartFailsToCreateRegistration);
    FRIEND_TEST(AosRegistrationTest, StartSucceedsToCreateRegistration);
    FRIEND_TEST(AosRegistrationTest, StopWhileInNotRegisteredStateReportsSuccessWithouDeregister);
    FRIEND_TEST(
            AosRegistrationTest, StopWhileInRegisteredStateReportsSuccessWhenFailToSendDeregister);
    FRIEND_TEST(AosRegistrationTest,
            StopWhileInRegisteredStateReportsTryingWhenSucceedToSendDeregister);
    FRIEND_TEST(AosRegistrationTest, StopWhileInDeregisteringStateDoesNothing);
    FRIEND_TEST(AosRegistrationTest, UpdateWhileInRegStopedStateTriggersRetrying);
    FRIEND_TEST(AosRegistrationTest, UpdateWhileInRegisteredStateTriggersRefreshing);
    FRIEND_TEST(AosRegistrationTest, UpdateWhileInRegisteringStateIsPending);
    FRIEND_TEST(AosRegistrationTest, UpdateWhileInDeregisteringStateDoesNothing);
    FRIEND_TEST(AosRegistrationTest, ReconfigWhileInRegStopStateTriggersUpdateRegBinding);
    FRIEND_TEST(AosRegistrationTest,
            ReconfigWhileInRegisteredStateTriggersUpdateRegBindingAndProcessReregister);
    FRIEND_TEST(AosRegistrationTest, ReconfigWhileInRegisteringStateIsPending);
    FRIEND_TEST(AosRegistrationTest, ReconfigWhileInOfflineStateIsIgnored);
    FRIEND_TEST(AosRegistrationTest, InitPcscfCommandSetsFirstPcscfIndex);
    FRIEND_TEST(AosRegistrationTest, InitAwtCommandSetsRetryTime);
    FRIEND_TEST(AosRegistrationTest, ClearRetryCountCommandClearsConsecutiveFailureCount);
    FRIEND_TEST(
            AosRegistrationTest, IpsecCommandForEnableWhileSupportIpsecTriggersDefaultFlowRecovery);
    FRIEND_TEST(AosRegistrationTest, IpsecCommandForDisableUpdatesIpsecSupportedStatus);
    FRIEND_TEST(AosRegistrationTest, UninterestedIpsecCommandDoesNothing);
    FRIEND_TEST(AosRegistrationTest, RefreshRegInfoCommandTriggersRecalculateCallerCapabilities);
    FRIEND_TEST(AosRegistrationTest, UpdateRegBindingCommandCreatesRegBinding);
    FRIEND_TEST(AosRegistrationTest, IpcanChangedCommandHoldsUpdateWhileInCall);
    FRIEND_TEST(AosRegistrationTest, IpcanChangedCommandTriggersUpdateWhileInNoCall);
    FRIEND_TEST(AosRegistrationTest, UpdateIpcanCommandUpdatesBlockStatus);
    FRIEND_TEST(AosRegistrationTest, ScscfRestorationCommandTriggersRegisterAfterRefreshingPcscf);
    FRIEND_TEST(AosRegistrationTest, ClearServerSocketErrorCountCommandClearsErrorCount);
    FRIEND_TEST(AosRegistrationTest, UnavailableFeatureTagCommandUpdatesDetailRegState);
    FRIEND_TEST(AosRegistrationTest, IncreaseFailureCountForPdnReactivatedCommandIncreasesCount);
    FRIEND_TEST(AosRegistrationTest, SetEps5gsOnlyCommandUpdatesVariable);
    FRIEND_TEST(AosRegistrationTest, HandleUninterestedCommandDoesNothing);
    FRIEND_TEST(AosRegistrationTest, CheckMode);
    FRIEND_TEST(AosRegistrationTest, GetProperty);
    FRIEND_TEST(AosRegistrationTest, CheckBool);
    FRIEND_TEST(AosRegistrationTest, FeatureTagForMtc);
    FRIEND_TEST(AosRegistrationTest, BlockChanged);
    FRIEND_TEST(AosRegistrationTest, IpsecSupported);
    FRIEND_TEST(AosRegistrationTest, UpdatePreloadedRoute);
    FRIEND_TEST(AosRegistrationTest, RetryCount);
    FRIEND_TEST(AosRegistrationTest, AddSpecificOperationWhileInRoamingAddsIpsecBlockReason);
    FRIEND_TEST(AosRegistrationTest, AddAccesstypeFeatureTagWithNumericalValue);
    FRIEND_TEST(AosRegistrationTest, AddAccesstypeFeatureTagWithoutNumericalValue);
    FRIEND_TEST(AosRegistrationTest, GetNetworkTypeForImsRegState);
    FRIEND_TEST(AosRegistrationTest, StartFailed_TxnTimeout);
    FRIEND_TEST(AosRegistrationTest, UpdateFailed_TxnTimeout);
    FRIEND_TEST(AosRegistrationTest, UpdateTransactionStarted);
    FRIEND_TEST(AosRegistrationTest, StandardPcscfSelection);
    FRIEND_TEST(AosRegistrationTest, Terminated);
    FRIEND_TEST(AosRegistrationTest, SendRegisterEx);
    FRIEND_TEST(AosRegistrationTest, UpdateFeatureTag);
    FRIEND_TEST(AosRegistrationTest, SetStaticIpQos);
    FRIEND_TEST(AosRegistrationTest, SetDynamicIpQos);
    FRIEND_TEST(AosRegistrationTest, GetActualWaitTime);
    FRIEND_TEST(AosRegistrationTest, TryNextPcscf);
    FRIEND_TEST(AosRegistrationTest, SetNextPcscf_SamePcscf);
    FRIEND_TEST(AosRegistrationTest, IsRetryOnSamePcscfRequired);
    FRIEND_TEST(AosRegistrationTest, RegReinitiateMessageTriggersReinitiate);
    FRIEND_TEST(AosRegistrationTest, RegUpdateMessageTriggersUpdate);
    FRIEND_TEST(AosRegistrationTest, RegReconfigMessageTriggersReconfig);
    FRIEND_TEST(AosRegistrationTest, RegRequiredWithWaitTimeMessageStartsOfflineRecoverTimer);
    FRIEND_TEST(AosRegistrationTest, RegRequiredWithNextPcscfMessageTriggersRegister);
    FRIEND_TEST(AosRegistrationTest, RegRequiredWithAvailableNextPcscfMessageTriggersRegister);
    FRIEND_TEST(AosRegistrationTest, RegTerminatedByNotifyMessageNotifiesFailure);
    FRIEND_TEST(AosRegistrationTest, SubReinitiateMessageCreatesSubscription);
    FRIEND_TEST(AosRegistrationTest, SubTerminatedMessageDestroiesSubscription);
    FRIEND_TEST(AosRegistrationTest, RegisteredEventMessageClearsRetryCount);
    FRIEND_TEST(AosRegistrationTest, HandleUninterestedMessageDoesNothing);
    FRIEND_TEST(AosRegistrationTest, InitializeSetsFeaturesAndListeners);
    FRIEND_TEST(AosRegistrationTest, CleanUpRemovesListeners);
    FRIEND_TEST(AosRegistrationTest,
            GeolocationInfoIsRequiredForNormalTypeIfPidfIsSupportedForCellular);
    FRIEND_TEST(
            AosRegistrationTest, GeolocationInfoIsRequiredForNormalTypeIfPidfIsSupportedForWifi);
    FRIEND_TEST(AosRegistrationTest,
            GeolocationInfoIsRequiredForEmergencyTypeIfPidfIsSupportedForCellular);
    FRIEND_TEST(
            AosRegistrationTest, GeolocationInfoIsRequiredForEmergencyTypeIfPidfIsSupportedForWifi);
    FRIEND_TEST(AosRegistrationTest, GeolocationInfoIsNotRequiredForFakeType);
    FRIEND_TEST(AosRegistrationTest,
            GeolocationInfoIsRequiredForNormalTypeIfPidfIsNotSupportedForCellular);
    FRIEND_TEST(AosRegistrationTest, GeolocationInfoIsNotRequiredIfIpsecHelperIsNull);
    FRIEND_TEST(AosRegistrationTest, GeolocationInfoIsNotRequiredIfIpsecIsNotEstablished);
    FRIEND_TEST(AosRegistrationTest, CheckPendingWhilePendingReconfigExistSendsRegReconfigMessage);
    FRIEND_TEST(AosRegistrationTest, CheckPendingWhilePendingUpdateExistSendsRegUpdateMessage);
    FRIEND_TEST(AosRegistrationTest, ProcessPendingStartTriggersStartRegister);
    FRIEND_TEST(AosRegistrationTest,
            ProcessPendingTrafficWhileInRefreshStopStateTriggersUpdateRegister);
    FRIEND_TEST(AosRegistrationTest, ProcessPendingTrafficWhileInOfflineStateTriggersStartRegister);
    FRIEND_TEST(AosRegistrationTest, ProcessPendingTransactionWhileRetryTimerExistDoesNothing);
    FRIEND_TEST(
            AosRegistrationTest, ProcessPendingTransactionWhileRetryIsHeldButFailsToSendRegister);
    FRIEND_TEST(AosRegistrationTest,
            ProcessPendingTransactionWhileRetryIsHeldAndSucceedsToSendRegister);
    FRIEND_TEST(AosRegistrationTest,
            ProcessPendingTransactionWhileRetryIsNotHeldAndInOfflineStateButFailsToRegister);
    FRIEND_TEST(AosRegistrationTest,
            ProcessPendingTransactionWhileRetryIsNotHeldAndInOfflineStateAndSucceedsToRegister);
    FRIEND_TEST(
            AosRegistrationTest, ProcessPendingTransactionWhileRetryIsNotHeldAndNotInOfflineState);
    FRIEND_TEST(AosRegistrationTest, ProcessPendingSubscriptionTriggersStartSubscription);
    FRIEND_TEST(AosRegistrationTest, ProcessRetryInRegStoppedWhileRetryIsNotHeldNotTriesRegister);
    FRIEND_TEST(AosRegistrationTest, ProcessRetryInRegStoppedWhileRetryTimerExistNotTriesRegister);
    FRIEND_TEST(AosRegistrationTest,
            ProcessRetryInRegStoppedWhileTransactionIsNotStartedNotTriesRegister);
    FRIEND_TEST(AosRegistrationTest,
            ProcessRetryInRegStoppedWhileTransactionIsNotAllowedNotTriesRegister);
    FRIEND_TEST(AosRegistrationTest, ProcessRetryInRegStoppedButFailsToSendRegister);
    FRIEND_TEST(AosRegistrationTest, ProcessReregister);
    FRIEND_TEST(AosRegistrationTest, ProcessRegTerminatedWhileCallExist);
    FRIEND_TEST(AosRegistrationTest, ProcessRegTerminatedWhileRetryTimerExist);
    FRIEND_TEST(AosRegistrationTest, ProcessAuthenticationFailed);
    FRIEND_TEST(AosRegistrationTest, ProcessForbiddenFailed);
    FRIEND_TEST(AosRegistrationTest, ProcessRegRequiredWithAvailableNextPcscf);
    FRIEND_TEST(AosRegistrationTest, ProcessSubscriberFailed);
    FRIEND_TEST(AosRegistrationTest, ProcessDefaultFlowRecovery_Start);
    FRIEND_TEST(AosRegistrationTest, ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy);
    FRIEND_TEST(AosRegistrationTest, ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy);
    FRIEND_TEST(AosRegistrationTest, ProcessDefaultFlowRecovery_Update);
    FRIEND_TEST(AosRegistrationTest, ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_TxnTimeout_RegRetryCountPerPcscfConfigured);
    FRIEND_TEST(AosRegistrationTest,
            ProcessStartFailed_TxnTimeout_RegRetryCountOnSinglePcscfConfigured);
    FRIEND_TEST(AosRegistrationTest, ProcessIpVersionChange);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_StatusCode);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_StatusCode_ProcessStartFailed_305);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_Success);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_Failure);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_HasNextPcscf);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_Others);
    FRIEND_TEST(AosRegistrationTest, ProcessUpdateFailed_StatusCode);
    FRIEND_TEST(AosRegistrationTest, ProcessUpdateFailed_StatusCode_ProcessUpdateFailed_305);
    FRIEND_TEST(AosRegistrationTest, ProcessUpdateFailed_StatusCodeWhenReceived500);
    FRIEND_TEST(AosRegistrationTest, AuthenticationChallengedIsNotHandledWhenRegistrationIsNull);
    FRIEND_TEST(AosRegistrationTest,
            AuthenticationChallengedIsNotHandledWhenMoreAuthChallengeIsNotAllowed);
    FRIEND_TEST(AosRegistrationTest, FailToProcessAuthenticationChallengedTriggersRegReInitate);
    FRIEND_TEST(AosRegistrationTest, NotifyAkaResponseIsNotHandledWhenIpsecIsNotSupported);
    FRIEND_TEST(AosRegistrationTest, NotifyAkaResponseTriggersRegTerminatedWhenResultIsNotOk);
    FRIEND_TEST(
            AosRegistrationTest, NotifyAkaResponseTriggersRegTerminatedWhenButFailToSetPcscfPort);
    FRIEND_TEST(AosRegistrationTest,
            NotifyAkaResponseTriggersRegTerminatedWhenFailToUpdatePreloadedRoute);
    FRIEND_TEST(AosRegistrationTest, NotifyAkaResponseTriggersRegTerminatedWhenFailToMakeSas);
    FRIEND_TEST(AosRegistrationTest, NotifyAkaResponseReturnsSaResultAsTrueWhenSucceedToMakeSas);
    FRIEND_TEST(AosRegistrationTest, RefreshTimerExpiredIsNotHandledWhenRegistrationIsNull);
    FRIEND_TEST(AosRegistrationTest, RefreshTimerExpiredIsNotHandledWhenNotRegisteredState);
    FRIEND_TEST(AosRegistrationTest, RefreshTimerExpiredReportsFailureWhenSuspended);
    FRIEND_TEST(AosRegistrationTest, RefreshTimerExpiredStopsRefreshWhenTransactionIsNotStarted);
    FRIEND_TEST(AosRegistrationTest, RefreshTimerExpiredStopsRefreshWhenTransactionIsStopped);
    FRIEND_TEST(AosRegistrationTest, RefreshTimerExpiredTriggersRegTerminatedWhenFailToCreateIpsec);
    FRIEND_TEST(AosRegistrationTest, RefreshTimerExpiredTriggersRegRefresh);
    FRIEND_TEST(AosRegistrationTest, RegistrationStartedWhenRegistrationIsNullDoesNothing);
    FRIEND_TEST(AosRegistrationTest, RegistrationStartedTriggersReinitiateIfIpsecIsNotEstablished);
    FRIEND_TEST(AosRegistrationTest, RegistrationStartedReportsSuccess);
    FRIEND_TEST(AosRegistrationTest, RegistrationStartFailedWhenRegistrationIsNullDoesNothing);
    FRIEND_TEST(AosRegistrationTest,
            RegistrationStartFailedWhenMoreAuthChallengeIsNotAllowedHandlesAuthFailure);
    FRIEND_TEST(AosRegistrationTest, RegistrationStartFailedWithStatusCode);
    FRIEND_TEST(AosRegistrationTest, RegistrationStartFailedWithTransactionTimeout);
    FRIEND_TEST(AosRegistrationTest, RegistrationStartFailedWithOtherReason);
    FRIEND_TEST(AosRegistrationTest, Registration_StartFailed_WfcErrReg403);
    FRIEND_TEST(AosRegistrationTest, Registration_StartFailed_WfcErrReg403_NotSupportErrMessage);
    FRIEND_TEST(AosRegistrationTest, Registration_StartFailed_WfcErrReg403_DuplicateReason);
    FRIEND_TEST(AosRegistrationTest, Registration_StartFailed_WfcErrReg500);
    FRIEND_TEST(AosRegistrationTest, Registration_StartFailed_WfcErrReg500_NotSupportErrMessage);
    FRIEND_TEST(AosRegistrationTest, Registration_StartFailed_WfcErrReg500_DuplicateReason);
    FRIEND_TEST(AosRegistrationTest, Registration_StartFailed_WfcErrOtherFailures);
    FRIEND_TEST(
            AosRegistrationTest, Registration_StartFailed_WfcErrOtherFailures_NotSupportErrMessage);
    FRIEND_TEST(AosRegistrationTest, Registration_StartFailed_WfcErrOtherFailures_DuplicateReason);
    FRIEND_TEST(AosRegistrationTest, Registration_StartFailed_RegEventChange_UnconditionalDownload);
    FRIEND_TEST(AosRegistrationTest, Registration_StartFailed_RegEventChange_NotDownload);
    FRIEND_TEST(AosRegistrationTest, Registration_Updated);
    FRIEND_TEST(AosRegistrationTest, Registration_UpdateFailed_CallEndByReregErr);
    FRIEND_TEST(AosRegistrationTest, Registration_UpdateFailed_FailOnRoaming);
    FRIEND_TEST(AosRegistrationTest, Registration_UpdateFailed_FailOnRoaming_HeldByCall);
    FRIEND_TEST(AosRegistrationTest, Registration_UpdateFailed_ProcessUpdateFailed_Others);
    FRIEND_TEST(AosRegistrationTest, Registration_Removed);
    FRIEND_TEST(AosRegistrationTest, StopTimer);
    FRIEND_TEST(AosRegistrationTest, ClearTimer);
    FRIEND_TEST(AosRegistrationTest, TimerExpired);
    FRIEND_TEST(AosRegistrationTest, ProcessOfflineRecoverTimerExpired);
    FRIEND_TEST(AosRegistrationTest, ProcessStopRetryTimerExpired);
    FRIEND_TEST(AosRegistrationTest, CreateAndDestroySubscription);
    FRIEND_TEST(AosRegistrationTest, Subscription_StateChanged);
    FRIEND_TEST(AosRegistrationTest, Subscription_CanBeTransmitted);
    FRIEND_TEST(AosRegistrationTest, Subscription_NotifyReceived);
    FRIEND_TEST(AosRegistrationTest, Subscription_Request);
    FRIEND_TEST(AosRegistrationTest, NConfiguration_NotifyConfigChanged);
    FRIEND_TEST(AosRegistrationTest, Transaction_OnConnectionFailed);
    FRIEND_TEST(AosRegistrationTest, MessageMediator_AdjustMessage);
    FRIEND_TEST(AosRegistrationTest, AddLocationHeaderBody);

protected:
    inline IRegistration* GetRegistration() override { return m_piRegistrationInstance; }
    inline void CreateIpsecHelper() override {}
    inline void DestroyIpsecHelper() override { m_pIpsecHelper = IMS_NULL; }
    inline AosSubscription* GetSubscription(IN IRegSubscription*) override
    {
        return m_pSubscriptionInstance;
    }

public:
    inline IMS_BOOL IsTxnPendingOn(IN IMS_UINT32 nFeature)
    {
        return m_pUtil->IsFeatureOn(nFeature, m_nTxnPending);
    }

    inline IMS_BOOL IsFeatureOn(IN IMS_UINT32 nFeature)
    {
        return m_pUtil->IsFeatureOn(nFeature, m_nFeature);
    }

    inline IMS_BOOL IsTimerRunning(IN IMS_UINT32 nType)
    {
        if (nType == TestAosRegistration::TIMER_OFFLINE_RECOVER)
        {
            return (m_piOfflineRecoverTimer != IMS_NULL);
        }

        if (nType == TestAosRegistration::TIMER_STOP_RETRY)
        {
            return (m_piStopRetryTimer != IMS_NULL);
        }

        if (nType == TestAosRegistration::TIMER_REFRESH)
        {
            return (m_piRefreshTimer != IMS_NULL);
        }

        if (nType == TestAosRegistration::TIMER_EXPIRED)
        {
            return (m_piExpiredTimer != IMS_NULL);
        }

        if (nType == TestAosRegistration::TIMER_MODE)
        {
            return (m_piModeTimer != IMS_NULL);
        }

        if (nType == TestAosRegistration::TIMER_TRANSACTION)
        {
            return (m_piTransactionTimer != IMS_NULL);
        }

        if (nType == TestAosRegistration::TIMER_INTERNAL_ERROR)
        {
            return (m_piInternalErrorTimer != IMS_NULL);
        }

        return IMS_FALSE;
    }

private:
    IRegistration* m_piRegistrationInstance;
    AosSubscription* m_pSubscriptionInstance;
};

MATCHER_P(IsSameMsg, message, "")
{
    return arg.nMSG == message;
}

class AosRegistrationTest : public ::testing::Test
{
public:
    AosRegistrationTest()
    {
        m_pAosStaticProfile = new AosStaticProfile();
        m_pAosStaticProfile->SetProfileType(AosStaticProfile::Type::NORMAL);

        m_objThreadService.SetThread(&m_objMockThread);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);
        m_objTimerService.SetTimer(&m_objMockITimer);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &m_objTimerService);

        m_piAosCallTracker = AosProvider::GetInstance()->GetCallTracker(SLOT_ID);
        AosProvider::GetInstance()->SetCallTracker(&m_objMockIAosCallTracker, SLOT_ID);
        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration(SLOT_ID);
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration, SLOT_ID);
        m_piAosRetryRepository = AosProvider::GetInstance()->GetRetryRepository(SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(&m_objMockIAosRetryRepository, SLOT_ID);
        m_piAosService = AosProvider::GetInstance()->GetService(SLOT_ID);
        AosProvider::GetInstance()->SetService(&m_objMockIAosService, SLOT_ID);
        m_piAosTransaction = AosProvider::GetInstance()->GetTransaction(SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(&m_objMockIAosTransaction, SLOT_ID);
    }
    virtual ~AosRegistrationTest()
    {
        AosProvider::GetInstance()->SetTransaction(m_piAosTransaction, SLOT_ID);
        AosProvider::GetInstance()->SetService(m_piAosService, SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(m_piAosRetryRepository, SLOT_ID);
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        AosProvider::GetInstance()->SetCallTracker(m_piAosCallTracker, SLOT_ID);

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);

        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }
    }

    TestAosRegistration* m_pAosRegistration;
    TestThreadService m_objThreadService;
    TestTimerService m_objTimerService;
    AosStaticProfile* m_pAosStaticProfile;
    IAosCallTracker* m_piAosCallTracker;
    IAosNConfiguration* m_piAosNConfiguration;
    IAosRetryRepository* m_piAosRetryRepository;
    IAosService* m_piAosService;
    IAosTransaction* m_piAosTransaction;

    MockAosIpsecHelper m_objMockAosIpsecHelper;
    MockAosSubscription m_objMockAosSubscription;
    MockISipConfigV m_objMockISipConfigV;
    MockIConfigurable m_objMockIConfigurable;
    MockISipMessage m_objMockISipMessage;
    MockIRegistration m_objMockIRegistration;
    MockIRegContact m_objMockIRegContact;
    MockIRegParameter m_objMockIRegParameter;
    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosBlock m_objMockIAosBlock;
    MockIAosCallTracker m_objMockIAosCallTracker;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosHandle m_objMockIAosHandle;
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    MockIAosNetTracker m_objMockIAosNetTracker;
    MockIAosService m_objMockIAosService;
    MockIAosSubscriber m_objMockIAosSubscriber;
    MockIAosPcscf m_objMockIAosPcscf;
    MockIAosRegistrationListener m_objMockIAosRegistrationListener;
    MockIAosRetryRepository m_objMockIAosRetryRepository;
    MockIAosTransaction m_objMockIAosTransaction;
    MockIRegSubscription m_objMockIRegSubscription;
    MockIThread m_objMockThread;
    MockITimer m_objMockITimer;

    AString m_strAppId = AString("ims.app.test");
    AString m_strServiceId = AString("ims.service.test");
    SipAddress m_objSipAddress = SipAddress("sip:1111@1.1.1.1");
    ImsMap<AString, IAosHandle*> m_objHandles;
    AStringArray m_objImpus;
    AStringArray m_objPcscfs;
    AStringArray m_objUris;
    ImsList<IMS_SINT32> m_objPcscfPorts;
    ImsVector<IMS_SINT32> m_objMaxCount;
    ImsVector<IMS_SINT32> m_objErrCode;
    ImsVector<IMS_SINT32> m_objRegRetryInterval;
    AosFeatureTagList m_objFeatureTagList;
    AosFeatureTagList m_objBindedFeatureTagList;

protected:
    virtual void SetUp() override
    {
        // IAosAppContext
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosAppContext, GetStaticProfile())
                .WillByDefault(Return(m_pAosStaticProfile));
        ON_CALL(m_objMockIAosAppContext, GetSubscriber())
                .WillByDefault(Return(&m_objMockIAosSubscriber));
        ON_CALL(m_objMockIAosAppContext, GetBlock()).WillByDefault(Return(&m_objMockIAosBlock));
        m_objHandles.Add(m_strServiceId, &m_objMockIAosHandle);
        ON_CALL(m_objMockIAosAppContext, GetHandles()).WillByDefault(ReturnRef(m_objHandles));
        ON_CALL(m_objMockIAosAppContext, GetPcscf()).WillByDefault(Return(&m_objMockIAosPcscf));
        ON_CALL(m_objMockIAosAppContext, GetConnection())
                .WillByDefault(Return(&m_objMockIAosConnection));
        ON_CALL(m_objMockIAosAppContext, GetNetTracker())
                .WillByDefault(Return(&m_objMockIAosNetTracker));

        // IRegistration
        ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));
        ON_CALL(m_objMockIRegistration, Deregister()).WillByDefault(Return(IMS_SUCCESS));
        m_objUris.AddElement(AString("TestUri"));
        ON_CALL(m_objMockIRegistration, GetAssociatedUris()).WillByDefault(ReturnRef(m_objUris));
        ON_CALL(m_objMockIRegistration, GetPreferredContact())
                .WillByDefault(Return(&m_objMockIRegContact));
        ON_CALL(m_objMockIRegistration, CreateContact(_, _, _, _))
                .WillByDefault(Return(&m_objMockIRegContact));
        ON_CALL(m_objMockIRegistration, GetParameter())
                .WillByDefault(Return(&m_objMockIRegParameter));
        ON_CALL(m_objMockIRegistration, GetPreviousRequest())
                .WillByDefault(Return(&m_objMockISipMessage));
        ON_CALL(m_objMockIRegistration, GetPreviousResponse())
                .WillByDefault(Return(&m_objMockISipMessage));

        // IAosNConfiguration
        ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
                .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));
        ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
                .WillByDefault(ReturnRef(m_objErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetExtraReregErrCode())
                .WillByDefault(ReturnRef(m_objErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetImsSignallingDscp()).WillByDefault(Return(46));
        ON_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
                .WillByDefault(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));
        ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
                .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
        ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
                .WillByDefault(ReturnRef(m_objErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
                .WillByDefault(ReturnRef(m_objErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithRetryAfterTime())
                .WillByDefault(ReturnRef(m_objErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
                .WillByDefault(
                        Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED));
        ON_CALL(m_objMockIAosNConfiguration, GetRegistrationPrivateHeader())
                .WillByDefault(Return(CarrierConfig::ImsWfc::REGISTRATION_P_NOT_SUPPORTED));
        ON_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryBaseTime())
                .WillByDefault(Return(30000));
        ON_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryMaxTime())
                .WillByDefault(Return(1800000));
        ON_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
                .WillByDefault(ReturnRef(m_objErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrMaxCount())
                .WillByDefault(ReturnRef(m_objMaxCount));
        ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
                .WillByDefault(
                        Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));
        ON_CALL(m_objMockIAosNConfiguration, GetRegRetryIntervals())
                .WillByDefault(ReturnRef(m_objRegRetryInterval));
        ON_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForCallEnd())
                .WillByDefault(ReturnRef(m_objErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForImsPdnReactivation())
                .WillByDefault(ReturnRef(m_objErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForInitRegWithAvailablePcscf())
                .WillByDefault(ReturnRef(m_objErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetReregRetryErrCodeForInitRegWithSamePcscf())
                .WillByDefault(ReturnRef(m_objErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetSipMessageThresholdForTransportChange())
                .WillByDefault(Return(340));
        ON_CALL(m_objMockIAosNConfiguration, GetSipPreferredTransport())
                .WillByDefault(Return(CarrierConfig::Ims::PREFERRED_TRANSPORT_UDP));
        ON_CALL(m_objMockIAosNConfiguration, IsContactUriValidationChecked())
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, IsUserInfoInContactSupported())
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_FALSE));

        // IAosSubscriber
        m_objImpus.AddElement(AString("sip:1111@ims.co.kr"));
        m_objImpus.AddElement(AString("sip:2222@ims.co.kr"));
        ON_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillByDefault(ReturnRef(m_objImpus));
        ON_CALL(m_objMockIAosSubscriber, GetFakeImpus()).WillByDefault(ReturnRef(m_objImpus));

        // IAosHandle
        ON_CALL(m_objMockIAosHandle, GetAppId()).WillByDefault(ReturnRef(m_strAppId));
        ON_CALL(m_objMockIAosHandle, GetServiceId()).WillByDefault(ReturnRef(m_strServiceId));
        ON_CALL(m_objMockIAosHandle, GetRequestType()).WillByDefault(Return(IAosHandle::ATTACH));
        ON_CALL(m_objMockIAosHandle, GetFeatureTagList())
                .WillByDefault(ReturnRef(m_objFeatureTagList));
        ON_CALL(m_objMockIAosHandle, GetBindedFeatureTagList())
                .WillByDefault(ReturnRef(m_objBindedFeatureTagList));
        ON_CALL(m_objMockIAosHandle, GetServiceType())
                .WillByDefault(Return(static_cast<IMS_UINT32>(ImsAosService::MTS)));
        ON_CALL(m_objMockIAosHandle, IsRegBinded()).WillByDefault(Return(IMS_TRUE));

        // IAosPcscf
        ON_CALL(m_objMockIAosPcscf, GetCurrentIndex()).WillByDefault(Return(0));
        ON_CALL(m_objMockIAosPcscf, HasPcscf(_)).WillByDefault(Return(IMS_TRUE));
        m_objPcscfs.AddElement(AString("192.168.0.100"));
        m_objPcscfs.AddElement(AString("192.168.0.101"));
        m_objPcscfs.AddElement(AString("192.168.0.102"));
        ON_CALL(m_objMockIAosPcscf, GetPcscfs()).WillByDefault(ReturnRef(m_objPcscfs));
        m_objPcscfPorts.Append(5060);
        m_objPcscfPorts.Append(5061);
        m_objPcscfPorts.Append(5062);
        ON_CALL(m_objMockIAosPcscf, GetPcscfsPorts()).WillByDefault(ReturnRef(m_objPcscfPorts));

        // IAosConnection
        ON_CALL(m_objMockIAosConnection, GetIpcanCategory())
                .WillByDefault(Return(IIpcan::CATEGORY_MOBILE));
        ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosConnection, GetMtu()).WillByDefault(Return(1400));
        ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
                .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

        // IAosNetTracker
        ON_CALL(m_objMockIAosNetTracker, GetNetworkType())
                .WillByDefault(Return(static_cast<IMS_UINT32>(AosNetworkType::LTE)));

        // IRegContact
        ON_CALL(m_objMockIRegContact, AddHeaderParameter(_, _)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIRegContact, GetContactAddress())
                .WillByDefault(ReturnRef(m_objSipAddress));

        // IAosTransaction
        ON_CALL(m_objMockIAosTransaction, StartTraffic(_, _)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIAosTransaction, IsTransactionAllowed(_)).WillByDefault(Return(IMS_TRUE));

        // IRegParameter and ISipMessage
        ON_CALL(m_objMockIRegParameter, AddPreloadedRoute(_, _, _)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockISipMessage, GetHeader(_, _, _)).WillByDefault(Return(AString("regtest")));

        // ISipConfigV and IConfigurable
        ON_CALL(m_objMockISipConfigV, GetConfigurable())
                .WillByDefault(Return(&m_objMockIConfigurable));
        ON_CALL(m_objMockIConfigurable, AddListener(_, _)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIConfigurable, Update(_, _)).WillByDefault(Return(IMS_TRUE));

        m_pAosRegistration = new TestAosRegistration(
                &m_objMockIAosAppContext, m_pAosStaticProfile->GetRegistrationId());

        m_pAosRegistration->SetMockRegistration(&m_objMockIRegistration);
        m_pAosRegistration->SetMockSubscription(&m_objMockAosSubscription);
        m_pAosRegistration->SetListener(&m_objMockIAosRegistrationListener);
    }

    virtual void TearDown() override
    {
        if (m_pAosRegistration)
        {
            delete m_pAosRegistration;
            m_pAosRegistration = IMS_NULL;
        }
    }
};

TEST_F(AosRegistrationTest, StartWhileTransactionIsNotStartedNotTriesCreateRegistration)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->m_bIsTransactionStarted = IMS_FALSE;

    EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber()).Times(0);

    m_pAosRegistration->Start();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_START));
}

TEST_F(AosRegistrationTest, StartWhileTransactionIsNotAllowedNotTriesCreateRegistration)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(IAosTransaction::TYPE_REG))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber()).Times(0);

    m_pAosRegistration->Start();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_TRAFFIC));
}

TEST_F(AosRegistrationTest, StartFailsToCreateRegistration)
{
    m_pAosRegistration->m_eRegType = AosRegistrationType::EMERGENCY;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    AStringArray objEmptyImpus;
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillOnce(ReturnRef(objEmptyImpus));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->Start();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, StartSucceedsToCreateRegistration)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->Start();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, StopWhileInNotRegisteredStateReportsSuccessWithouDeregister)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    EXPECT_CALL(m_objMockIRegistration, Deregister()).Times(0);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE));

    m_pAosRegistration->Stop();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, StopWhileInRegisteredStateReportsSuccessWhenFailToSendDeregister)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, Deregister()).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE));

    m_pAosRegistration->Stop();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, StopWhileInRegisteredStateReportsTryingWhenSucceedToSendDeregister)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, Deregister()).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_STOP));

    m_pAosRegistration->Stop();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_DEREGISTERING);
}

TEST_F(AosRegistrationTest, StopWhileInDeregisteringStateDoesNothing)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_DEREGISTERING);

    EXPECT_CALL(m_objMockIRegistration, Deregister()).Times(0);
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _)).Times(0);

    m_pAosRegistration->Stop();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_DEREGISTERING);
}

TEST_F(AosRegistrationTest, UpdateWhileInRegStopedStateTriggersRetrying)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->Update(IMS_TRUE, IMS_TRUE);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, UpdateWhileInRegisteredStateTriggersRefreshing)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->Update(IMS_TRUE, IMS_TRUE);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosRegistrationTest, UpdateWhileInRegisteringStateIsPending)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);

    m_pAosRegistration->Update(IMS_TRUE, IMS_TRUE);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_UPDATE));
}

TEST_F(AosRegistrationTest, UpdateWhileInDeregisteringStateDoesNothing)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_DEREGISTERING);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);

    m_pAosRegistration->Update(IMS_TRUE, IMS_TRUE);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_DEREGISTERING);
    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_UPDATE));
}

TEST_F(AosRegistrationTest, ReconfigWhileInRegStopStateTriggersUpdateRegBinding)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_piRegContact = &m_objMockIRegContact;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);

    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType()).WillRepeatedly(Return(IAosHandle::DETACH));
    EXPECT_CALL(m_objMockIRegContact, RemoveService(_, _)).Times(1);
    EXPECT_CALL(m_objMockIRegistration, DestroyBinding(_, _)).Times(1);
    EXPECT_CALL(m_objMockIRegContact, RecalculateCallerCapabilities()).Times(1);

    m_pAosRegistration->Reconfig();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_RECONFIG));
}

TEST_F(AosRegistrationTest,
        ReconfigWhileInRegisteredStateTriggersUpdateRegBindingAndProcessReregister)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_piRegContact = &m_objMockIRegContact;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType()).WillRepeatedly(Return(IAosHandle::ATTACH));
    m_objFeatureTagList.AddFeatureTag(FeatureTags::CDMALESS);
    EXPECT_CALL(m_objMockIAosHandle, GetFeatureTagList())
            .WillRepeatedly(ReturnRef(m_objFeatureTagList));
    EXPECT_CALL(m_objMockIRegContact, RecalculateCallerCapabilities()).Times(1);

    m_pAosRegistration->Reconfig();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);
    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_RECONFIG));
}

TEST_F(AosRegistrationTest, ReconfigWhileInRegisteringStateIsPending)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);

    m_pAosRegistration->Reconfig();

    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_RECONFIG));
}

TEST_F(AosRegistrationTest, ReconfigWhileInOfflineStateIsIgnored)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded()).Times(0);

    m_pAosRegistration->Reconfig();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_RECONFIG));
}

TEST_F(AosRegistrationTest, InitPcscfCommandSetsFirstPcscfIndex)
{
    EXPECT_CALL(m_objMockIAosPcscf, SetFirstPcscfIndex()).Times(1);

    m_pAosRegistration->RequestCmd(
            IAosRegistration::CMD_INIT_PCSCF, IAosRegistration::REASON_INIT_PCSCF_CLEAR);
}

TEST_F(AosRegistrationTest, InitAwtCommandSetsRetryTime)
{
    IMS_UINT32 nConfiguredBaseTime = 30000;
    IMS_UINT32 nConfiguredMaxTime = 1800000;

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryBaseTime())
            .WillOnce(Return(nConfiguredBaseTime));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryMaxTime())
            .WillOnce(Return(nConfiguredMaxTime));

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_INIT_AWT);

    EXPECT_EQ(m_pAosRegistration->m_nRetryBaseTime, nConfiguredBaseTime / 1000);
    EXPECT_EQ(m_pAosRegistration->m_nRetryMaxTime, nConfiguredMaxTime / 1000);
}

TEST_F(AosRegistrationTest, ClearRetryCountCommandClearsConsecutiveFailureCount)
{
    m_pAosRegistration->IncreaseConsecutiveFailCount();
    EXPECT_NE(m_pAosRegistration->m_nConsecutiveFailure, 0);

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_CLEAR_RETRY_COUNT);

    EXPECT_EQ(m_pAosRegistration->m_nConsecutiveFailure, 0);
}

TEST_F(AosRegistrationTest, IpsecCommandForEnableWhileSupportIpsecTriggersDefaultFlowRecovery)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_IPSEC;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .WillOnce(Return(AString("60")));
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT));

    m_pAosRegistration->RequestCmd(
            IAosRegistration::CMD_SET_IPSEC, IAosRegistration::REASON_SET_IPSEC_ENABLE);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosRegistrationTest, IpsecCommandForDisableUpdatesIpsecSupportedStatus)
{
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_IPSEC;

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->RequestCmd(
            IAosRegistration::CMD_SET_IPSEC, IAosRegistration::REASON_SET_IPSEC_DISABLE);

    EXPECT_FALSE(m_pAosRegistration->IsIpsecSupported());
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, UninterestedIpsecCommandDoesNothing)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined()).Times(0);
    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);

    m_pAosRegistration->RequestCmd(
            IAosRegistration::CMD_SET_IPSEC, IAosRegistration::REASON_SET_IPSEC_INIT);
}

TEST_F(AosRegistrationTest, RefreshRegInfoCommandTriggersRecalculateCallerCapabilities)
{
    m_pAosRegistration->m_piRegContact = &m_objMockIRegContact;

    EXPECT_CALL(m_objMockIRegContact, RecalculateCallerCapabilities()).Times(1);

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_REFRESH_REGINFO);
}

TEST_F(AosRegistrationTest, UpdateRegBindingCommandCreatesRegBinding)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_piRegContact = &m_objMockIRegContact;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType()).WillRepeatedly(Return(IAosHandle::ATTACH));
    EXPECT_CALL(m_objMockIRegistration, CreateBinding(_, _)).Times(1);
    EXPECT_CALL(m_objMockIAosHandle, SetRegBinded(IMS_TRUE)).Times(1);

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_UPDATE_REG_BINDING);
}

TEST_F(AosRegistrationTest, IpcanChangedCommandHoldsUpdateWhileInCall)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetImsCall(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithIpcanChangedDuringImsCallHeld())
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .WillOnce(Return(IIpcan::CATEGORY_MOBILE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_UPDATE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE));

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_IPCAN_CHANGED);

    EXPECT_TRUE(
            m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_UPDATE_HELD_BY_CALL));
    EXPECT_EQ(m_pAosRegistration->GetRegIpcanCategory(), IIpcan::CATEGORY_MOBILE);
}

TEST_F(AosRegistrationTest, IpcanChangedCommandTriggersUpdateWhileInNoCall)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosRegistration->SetImsCall(IMS_FALSE);

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_IPCAN_CHANGED);

    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_UPDATE));
}

TEST_F(AosRegistrationTest, UpdateIpcanCommandUpdatesBlockStatus)
{
    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_)).WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_UPDATE_IPCAN);

    EXPECT_TRUE(m_pAosRegistration->IsBlocked());
    EXPECT_FALSE(m_pAosRegistration->m_bIsTransactionStarted);
}

TEST_F(AosRegistrationTest, ScscfRestorationCommandTriggersRegisterAfterRefreshingPcscf)
{
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).WillOnce(Return(2));
    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_SCSCF_RESTORATION);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ClearServerSocketErrorCountCommandClearsErrorCount)
{
    m_pAosRegistration->m_nErrorCountForServerSocket = 2;

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_CLEAR_SERVER_SOCKET_ERROR_COUNT);

    EXPECT_EQ(m_pAosRegistration->m_nErrorCountForServerSocket, 0);
}

TEST_F(AosRegistrationTest, UnavailableFeatureTagCommandUpdatesDetailRegState)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->m_nImsRegState = TestAosRegistration::IMS_REG_STATE_REGISTERING;

    EXPECT_CALL(m_objMockIAosService, NotifyRegistered(_, _, _)).Times(1);

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_UNAVAILABLE_FEATURE_TAG);
}

TEST_F(AosRegistrationTest, IncreaseFailureCountForPdnReactivatedCommandIncreasesCount)
{
    IMS_UINT32 nCurrentCnt = m_pAosRegistration->m_nConsecutiveFailureForPdnReactivated;
    m_pAosRegistration->m_bEps5GsOnly = IMS_TRUE;

    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));

    m_pAosRegistration->RequestCmd(
            IAosRegistration::CMD_INCREASE_FAILURE_COUNT_FOR_PDN_REACTIVATED);

    EXPECT_EQ(m_pAosRegistration->m_nConsecutiveFailureForPdnReactivated, nCurrentCnt + 1);
}

TEST_F(AosRegistrationTest, SetEps5gsOnlyCommandUpdatesVariable)
{
    m_pAosRegistration->m_bEps5GsOnly = IMS_FALSE;

    m_pAosRegistration->RequestCmd(
            IAosRegistration::CMD_SET_EPS_5GS_ONLY, IAosRegistration::REASON_SET_ENABLE);

    EXPECT_TRUE(m_pAosRegistration->m_bEps5GsOnly);
}

TEST_F(AosRegistrationTest, HandleUninterestedCommandDoesNothing)
{
    m_pAosRegistration->m_bEps5GsOnly = IMS_TRUE;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_piRegContact = &m_objMockIRegContact;

    EXPECT_CALL(m_objMockIAosPcscf, SetFirstPcscfIndex()).Times(0);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryBaseTime()).Times(0);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined()).Times(0);
    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);
    EXPECT_CALL(m_objMockIRegContact, RecalculateCallerCapabilities()).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_)).Times(0);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy()).Times(0);

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_ECALL_INIT);

    EXPECT_TRUE(m_pAosRegistration->m_bEps5GsOnly);
}

TEST_F(AosRegistrationTest, CheckMode)
{
    m_pAosRegistration->SetMode(IAosRegistration::MODE_NORMAL);
    EXPECT_EQ(m_pAosRegistration->GetMode(), IAosRegistration::MODE_NORMAL);

    m_pAosRegistration->SetMode(IAosRegistration::MODE_FAKE);
    EXPECT_EQ(m_pAosRegistration->GetMode(), IAosRegistration::MODE_FAKE);
}

TEST_F(AosRegistrationTest, GetProperty)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    IMS_UINT32 nValue;
    AString strValue;

    // PROPERTY_LOCAL_PORT
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_LOCAL_PORT, nValue, strValue);
    EXPECT_EQ(nValue, m_pAosRegistration->m_pUtil->GetLocalPort(SLOT_ID));

    // PROPERTY_LOCAL_ADDRESS
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_LOCAL_ADDRESS, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), m_pAosRegistration->m_objIpa.ToString().GetStr());

    // PROPERTY_PCSCF_PORT
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_PCSCF_PORT, nValue, strValue);
    EXPECT_EQ(nValue, m_pAosRegistration->m_nPcscfPort);

    // PROPERTY_PCSCF_ADDRESS
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_PCSCF_ADDRESS, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), m_pAosRegistration->m_strPcscf.GetStr());

    // PROPERTY_ASSOCIATED_URI
    EXPECT_CALL(m_objMockIRegistration, GetAssociatedUris()).WillOnce(ReturnRef(m_objUris));
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_ASSOCIATED_URI, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestUri");

    // PROPERTY_PATH
    AString strPath = AString("TestPath");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::PATH, _, _)).WillOnce(Return(strPath));
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_PATH, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestPath");

    // PROPERTY_SERVICE_ROUTE
    AString strServiceRoute = AString("TestServiceRoute");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::SERVICE_ROUTE, _, _))
            .WillOnce(Return(strServiceRoute));
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_SERVICE_ROUTE, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestServiceRoute");

    // PROPERTY_LAST_PATH
    ImsList<AString> strPaths;
    strPaths.Append(AString("TestLastPath"));
    EXPECT_CALL(m_objMockISipMessage, GetHeaders(ISipHeader::PATH, _)).WillOnce(Return(strPaths));
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_LAST_PATH, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestLastPath");

    // PROPERTY_SUPPORTED
    AString strSupported = AString("TestSupported");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::SUPPORTED, _, _))
            .WillOnce(Return(strSupported));
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_SUPPORTED, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestSupported");

    // PROPERTY_PROTECTED - IpcanCategory is CATEGORY_MOBILE
    m_pAosRegistration->m_pIpsecHelper = &m_objMockAosIpsecHelper;
    EXPECT_CALL(m_objMockAosIpsecHelper, IsEstablished()).WillOnce(Return(IMS_TRUE));
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_PROTECTED, nValue, strValue);
    EXPECT_EQ(nValue, AoSRegProtectedType::REG_PROTECTED);

    // PROPERTY_PROTECTED - IpcanCategory is CATEGORY_WLAN
    EXPECT_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .WillOnce(Return(IIpcan::CATEGORY_WLAN));
    m_pAosRegistration->UpdateRegIpcanCategory();
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_PROTECTED, nValue, strValue);
    EXPECT_EQ(nValue, AoSRegProtectedType::REG_PROTECTED);

    // PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION - m_bCallingNumberVerificationSupported if false
    m_pAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION, nValue, strValue);
    EXPECT_EQ(nValue, AoSSupportability::NOT_SUPPORTED);

    // PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION - m_bCallingNumberVerificationSupported if true
    m_pAosRegistration->m_bCallingNumberVerificationSupported = true;
    m_pAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION, nValue, strValue);
    EXPECT_EQ(nValue, AoSSupportability::SUPPORTED);

    // PROPERTY_NETWORK_BINDING_FEATURES
    m_pAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_NETWORK_BINDING_FEATURES, nValue, strValue);
    EXPECT_EQ(nValue, m_pAosRegistration->m_nNetworkBindingFeatures);

    // PROPERTY_PDN_REACIVATE_WAIT_TIME
    m_pAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_PDN_REACIVATE_WAIT_TIME, nValue, strValue);
    EXPECT_EQ(nValue, 30);

    // PROPERTY_REG_FAILURE_COUNT
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_REG_FAILURE_COUNT, nValue, strValue);
    EXPECT_EQ(nValue, m_pAosRegistration->m_nConsecutiveFailure);
}

TEST_F(AosRegistrationTest, CheckBool)
{
    m_pAosRegistration->m_eRegType = AosRegistrationType::EMERGENCY;

    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_TRUE(m_pAosRegistration->IsRegistered());

    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    EXPECT_TRUE(m_pAosRegistration->IsRefreshing());

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_STOP_RETRY, 10000);
    EXPECT_TRUE(m_pAosRegistration->IsRetryTimer());
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_STOP_RETRY);

    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHSTOP);
    EXPECT_TRUE(m_pAosRegistration->IsRetryHeld());
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_TERMINATED;
    EXPECT_TRUE(m_pAosRegistration->IsTerminated());
    m_pAosRegistration->m_nTxnPending &= ~(TestAosRegistration::PENDING_TERMINATED);

    m_pAosRegistration->SetAppReady(IMS_TRUE);
    EXPECT_TRUE(m_pAosRegistration->IsAppReady());
    m_pAosRegistration->SetAppReady(IMS_FALSE);
    EXPECT_FALSE(m_pAosRegistration->IsAppReady());

    m_pAosRegistration->m_eRegType = AosRegistrationType::NORMAL;
}

TEST_F(AosRegistrationTest, FeatureTagForMtc)
{
    m_pAosRegistration->m_pUtil->SetISipConfigV(&m_objMockISipConfigV);
    m_pAosRegistration->m_piRegContact = &m_objMockIRegContact;

    EXPECT_CALL(m_objMockISipConfigV, GetFeatureTagOptions())
            .Times(AnyNumber())
            .WillRepeatedly(Return(ISipConfigV::FEATURE_TAG_MEDIA_STREAM));

    EXPECT_TRUE(m_pAosRegistration->AddFeatureTagForMtc(ImsAosFeature::VIDEO, IMS_FALSE));
    EXPECT_TRUE(m_pAosRegistration->AddFeatureTagForMtc(ImsAosFeature::TEXT, IMS_FALSE));
    EXPECT_TRUE(m_pAosRegistration->AddFeatureTagForMtc(
            ImsAosFeature::VIDEO | ImsAosFeature::TEXT, IMS_FALSE));

    EXPECT_CALL(m_objMockISipConfigV, GetFeatureTagOptions())
            .Times(AnyNumber())
            .WillRepeatedly(Return(ISipConfigV::FEATURE_TAG_MEDIA_STREAM_VIDEO |
                    ISipConfigV::FEATURE_TAG_MEDIA_STREAM_TEXT));

    EXPECT_TRUE(m_pAosRegistration->RemoveFeatureTagForMtc(ImsAosFeature::VIDEO));
    EXPECT_TRUE(m_pAosRegistration->RemoveFeatureTagForMtc(ImsAosFeature::TEXT));
    EXPECT_TRUE(
            m_pAosRegistration->RemoveFeatureTagForMtc(ImsAosFeature::VIDEO | ImsAosFeature::TEXT));

    EXPECT_CALL(m_objMockIRegContact,
            AddExtraCapability(
                    AString(FeatureTags::CALL_COMPOSER_VIA_TELEPHONY), AString::ConstNull()));
    EXPECT_CALL(m_objMockIRegContact,
            RemoveExtraCapability(
                    AString(FeatureTags::CALL_COMPOSER_VIA_TELEPHONY), AString::ConstNull()));
    m_pAosRegistration->AddFeatureTagForMtc(ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY, IMS_FALSE);
    m_pAosRegistration->AddFeatureTagForMtc(ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY, IMS_TRUE);
    m_pAosRegistration->RemoveFeatureTagForMtc(ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY);

    m_pAosRegistration->m_piRegContact = IMS_NULL;
    m_pAosRegistration->m_pUtil->SetISipConfigV(IMS_NULL);
}

TEST_F(AosRegistrationTest, BlockChanged)
{
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosRegistration->SetAppReady(IMS_FALSE);

    EXPECT_FALSE(m_pAosRegistration->IsBlocked());

    m_pAosRegistration->AosRegistration::Block_Changed();

    EXPECT_TRUE(m_pAosRegistration->IsBlocked());
    EXPECT_FALSE(m_pAosRegistration->IsTransactionStarted());
}

TEST_F(AosRegistrationTest, IpsecSupported)
{
    // precondition for checking Ipsec supported - IsFakeRegistration()
    m_pAosRegistration->SetFakeReg(IMS_FALSE);

    m_pAosRegistration->m_eRegType = AosRegistrationType::NORMAL;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSubscription())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    // FEATURE_IPSEC is not set
    m_pAosRegistration->InitFeatures();

    m_pAosRegistration->ClearIpsecBlock();
    EXPECT_FALSE(m_pAosRegistration->IsIpsecSupported());

    m_pAosRegistration->UpdateIpsecSupported(IMS_FALSE, TestAosRegistration::IPSEC_BLOCK_ERROR);
    EXPECT_FALSE(m_pAosRegistration->IsIpsecSupported());

    m_pAosRegistration->UpdateIpsecSupported(IMS_TRUE, TestAosRegistration::IPSEC_BLOCK_ERROR);
    EXPECT_FALSE(m_pAosRegistration->IsIpsecSupported());

    // FEATURE_IPSEC is set
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_IPSEC;

    m_pAosRegistration->UpdateIpsecSupported(IMS_FALSE, TestAosRegistration::IPSEC_BLOCK_ERROR);
    EXPECT_FALSE(m_pAosRegistration->IsIpsecSupported());

    m_pAosRegistration->UpdateIpsecSupported(
            IMS_FALSE, TestAosRegistration::IPSEC_BLOCK_AUTENTICATION);
    EXPECT_FALSE(m_pAosRegistration->IsIpsecSupported());

    m_pAosRegistration->UpdateIpsecSupported(
            IMS_TRUE, TestAosRegistration::IPSEC_BLOCK_AUTENTICATION);
    EXPECT_FALSE(m_pAosRegistration->IsIpsecSupported());

    m_pAosRegistration->UpdateIpsecSupported(IMS_TRUE, TestAosRegistration::IPSEC_BLOCK_ERROR);
    EXPECT_TRUE(m_pAosRegistration->IsIpsecSupported());

    m_pAosRegistration->UpdateIpsecSupported(IMS_FALSE, TestAosRegistration::IPSEC_BLOCK_ROAMING);
    EXPECT_FALSE(m_pAosRegistration->IsIpsecSupported());

    m_pAosRegistration->UpdateIpsecSupported(
            IMS_FALSE, TestAosRegistration::IPSEC_BLOCK_NOT_ESTABLISHED);
    EXPECT_FALSE(m_pAosRegistration->IsIpsecSupported());

    m_pAosRegistration->UpdateIpsecSupported(
            IMS_TRUE, TestAosRegistration::IPSEC_BLOCK_NOT_ESTABLISHED);
    EXPECT_FALSE(m_pAosRegistration->IsIpsecSupported());

    m_pAosRegistration->UpdateIpsecSupported(IMS_TRUE, TestAosRegistration::IPSEC_BLOCK_ROAMING);
    EXPECT_TRUE(m_pAosRegistration->IsIpsecSupported());
}

TEST_F(AosRegistrationTest, UpdatePreloadedRoute)
{
    // m_piRegParameter is null
    EXPECT_FALSE(m_pAosRegistration->UpdatePreloadedRoute(AString::ConstNull(), 0));

    // m_piRegParameter is not null
    m_pAosRegistration->m_piRegParameter = &m_objMockIRegParameter;
    EXPECT_CALL(m_objMockIRegParameter, RemoveAllPreloadedRoutes()).Times(1);
    EXPECT_TRUE(m_pAosRegistration->UpdatePreloadedRoute(AString("192.168.0.1"), 80));
}

TEST_F(AosRegistrationTest, RetryCount)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // TEST_F : IncreaseConsecutiveFailCount
    m_pAosRegistration->IncreaseConsecutiveFailCount();
    EXPECT_NE(m_pAosRegistration->m_nConsecutiveFailure, 0);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION));

    // TEST_F : ClearRetryCount
    m_pAosRegistration->ClearRetryCount();
    EXPECT_NE(m_pAosRegistration->m_nConsecutiveFailure, 0);

    m_pAosRegistration->ClearRetryCount(IMS_TRUE);
    EXPECT_EQ(m_pAosRegistration->m_nConsecutiveFailure, 0);
}

TEST_F(AosRegistrationTest, AddSpecificOperationWhileInRoamingAddsIpsecBlockReason)
{
    m_pAosRegistration->m_eRegType = AosRegistrationType::EMERGENCY;
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_IPSEC;
    m_pAosRegistration->m_piRegContact = &m_objMockIRegContact;

    EXPECT_CALL(m_objMockIRegContact, AddUriParameter(_, _)).Times(1);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsSipOverIpsecInRoamingEnabled())
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNetTracker, IsRoaming()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED));

    m_pAosRegistration->AddSpecificOperation();

    EXPECT_EQ(m_pAosRegistration->m_nIpsecBlockReason, TestAosRegistration::IPSEC_BLOCK_ROAMING);
}

TEST_F(AosRegistrationTest, AddAccesstypeFeatureTagWithNumericalValue)
{
    m_pAosRegistration->m_piRegContact = &m_objMockIRegContact;

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED));
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIRegContact, AddHeaderParameter(_, _)).Times(1);

    m_pAosRegistration->AddAccesstypeFeatureTag();
}

TEST_F(AosRegistrationTest, AddAccesstypeFeatureTagWithoutNumericalValue)
{
    m_pAosRegistration->m_piRegContact = &m_objMockIRegContact;

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
            .WillOnce(Return(CarrierConfig::Ims::
                            PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED_WITHOUT_NUMERICAL_VALUE));
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIRegContact, AddHeaderParameter(_, _)).Times(1);

    m_pAosRegistration->AddAccesstypeFeatureTag();
}

TEST_F(AosRegistrationTest, GetNetworkTypeForImsRegState)
{
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillOnce(Return(NW_REPORT_RADIO_WLAN))
            .WillOnce(Return(NW_REPORT_RADIO_LTE))
            .WillOnce(Return(NW_REPORT_RADIO_NR))
            .WillOnce(Return(NW_REPORT_RADIO_WCDMA))
            .WillOnce(Return(NW_REPORT_RADIO_NOSRV));

    EXPECT_EQ(m_pAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::IWLAN);
    EXPECT_EQ(m_pAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::LTE);
    EXPECT_EQ(m_pAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::NR);
    EXPECT_EQ(m_pAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::UTRAN);
    EXPECT_EQ(m_pAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::NONE);
}

TEST_F(AosRegistrationTest, StartFailed_TxnTimeout)
{
    // Covers GetRegErrCodeForPcscfDiscovery() == CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Clear();
    objRegErrCodeForPcscfDiscovery.Add(0);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));

    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(AnyNumber());

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryTimerFPolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::TIMER_F_POLICY_SPEC))
            .WillOnce(Return(CarrierConfig::Assets::TIMER_F_POLICY_SPEC_WITH_AWT));

    m_pAosRegistration->ProcessStartFailed_TxnTimeout();

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillOnce(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->ProcessStartFailed_TxnTimeout();
}

TEST_F(AosRegistrationTest, UpdateFailed_TxnTimeout)
{
    // Covers GetReregErrCodeForInitRegWithAvailablePcscf() ==
    // CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F
    m_pAosRegistration->m_eRegType = AosRegistrationType::EMERGENCY;

    ImsVector<IMS_SINT32> objReregErrCodeForInitRegWithAvailablePcscf;
    objReregErrCodeForInitRegWithAvailablePcscf.Clear();
    objReregErrCodeForInitRegWithAvailablePcscf.Add(CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForInitRegWithAvailablePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objReregErrCodeForInitRegWithAvailablePcscf));

    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(AnyNumber());

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryTimerFPolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::TIMER_F_POLICY_SPEC))
            .WillOnce(Return(CarrierConfig::Assets::TIMER_F_POLICY_SPEC_WITH_AWT));

    m_pAosRegistration->ProcessUpdateFailed_TxnTimeout();

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillOnce(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->ProcessUpdateFailed_TxnTimeout();
}

TEST_F(AosRegistrationTest, UpdateTransactionStarted)
{
    m_pAosRegistration->m_eRegType = AosRegistrationType::EMERGENCY;
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    m_pAosRegistration->UpdateTransactionStarted();
    EXPECT_TRUE(m_pAosRegistration->IsTransactionStarted());

    m_pAosRegistration->m_eRegType = AosRegistrationType::NORMAL;
    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .WillOnce(Return(IMS_TRUE));
    m_pAosRegistration->SetHeldByCall(IMS_TRUE);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHSTOP);
    m_pAosRegistration->SetBlocked(IMS_FALSE);
    m_pAosRegistration->SetRadioWaiting(IMS_FALSE);
    m_pAosRegistration->UpdateTransactionStarted();
    EXPECT_TRUE(m_pAosRegistration->IsTransactionStarted());
}

TEST_F(AosRegistrationTest, StandardPcscfSelection)
{
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT))
            .Times(AnyNumber());

    m_pAosRegistration->ProcessStandardPcscfSelection();

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT))
            .Times(AnyNumber());

    m_pAosRegistration->ProcessStandardPcscfSelection(30);
}

TEST_F(AosRegistrationTest, Terminated)
{
    // IsHandlingServerSocketErrorRequired - IMS_TRUE
    m_pAosRegistration->m_eRegType = AosRegistrationType::NORMAL;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_TRUE(m_pAosRegistration->IsReconnectingServerSocketErrorAllowed());

    m_pAosRegistration->Registration_Terminated(IRegistration::REASON_SERVER_SOCKET_ERROR);
    EXPECT_EQ(m_pAosRegistration->m_nErrorCountForServerSocket, 1);
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_INTERNAL_ERROR));

    m_pAosRegistration->Registration_Terminated(IRegistration::REASON_SERVER_SOCKET_ERROR);
    EXPECT_EQ(m_pAosRegistration->m_nErrorCountForServerSocket, 1);
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_INTERNAL_ERROR);
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    // IsHandlingServerSocketErrorRequired - IMS_FALSE
    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    // IsCallEndAndPdnReactivationByRegTerminated - IMS_TRUE
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->Registration_Terminated(IRegistration::REASON_REFRESH_TIMEOUT);

    m_pAosRegistration->SetImsCall(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED));

    m_pAosRegistration->Registration_Terminated(IRegistration::REASON_REFRESH_TIMEOUT);

    // IsCallEndAndPdnReactivationByRegTerminated - IMS_FALSE
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED));

    m_pAosRegistration->Registration_Terminated(IRegistration::REASON_REFRESH_TIMEOUT);
}

TEST_F(AosRegistrationTest, SendRegisterEx)
{
    // m_piRegistration is null
    m_pAosRegistration->m_piRegistration = IMS_NULL;
    EXPECT_FALSE(m_pAosRegistration->SendRegisterEx(1800, IMS_TRUE));

    // m_piRegistration is not null
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockIRegistration, Register(_))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FAILURE))
            .WillOnce(Return(IMS_SUCCESS));

    // fail to Register
    EXPECT_FALSE(m_pAosRegistration->SendRegisterEx(1800, IMS_FALSE));

    // succeed to Register
    EXPECT_TRUE(m_pAosRegistration->SendRegisterEx(0, IMS_FALSE));
}

TEST_F(AosRegistrationTest, UpdateFeatureTag)
{
    m_pAosRegistration->m_piRegContact = &m_objMockIRegContact;
    EXPECT_CALL(m_objMockIAosHandle, GetFeatureTagList())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(m_objFeatureTagList));
    EXPECT_CALL(m_objMockIAosHandle, GetBindedFeatureTagList())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(m_objBindedFeatureTagList));

    // same with binded feature tag
    EXPECT_FALSE(m_pAosRegistration->UpdateFeatureTag(&m_objMockIAosHandle));

    // Not same with binded feature tag
    m_objFeatureTagList.AddFeatureTag(FeatureTags::CDMALESS);
    m_objBindedFeatureTagList.AddFeatureTag(FeatureTags::VIDEO);
    EXPECT_CALL(m_objMockIRegContact, RemoveHeaderParameter(_, _)).Times(1);
    EXPECT_CALL(m_objMockIRegContact, AddHeaderParameter(_, _)).Times(1);
    EXPECT_TRUE(m_pAosRegistration->UpdateFeatureTag(&m_objMockIAosHandle));
}

TEST_F(AosRegistrationTest, SetStaticIpQos)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetImsSignallingDscp())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(46));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_WIFI))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    // nPreferredImsDscp is PREFERRED_DSCP_CELLULAR but GetImsSignallingDscp is 0
    m_pAosRegistration->SetStaticIpQos();

    // nPreferredImsDscp is PREFERRED_DSCP_CELLULAR but ePDG is enabled
    m_pAosRegistration->SetStaticIpQos();

    // nPreferredImsDscp is PREFERRED_DSCP_WIFI but ePDG is not enabled
    m_pAosRegistration->SetStaticIpQos();

    // nPreferredImsDscp is PREFERRED_DSCP_NONE
    m_pAosRegistration->SetStaticIpQos();
}

TEST_F(AosRegistrationTest, SetDynamicIpQos)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetImsSignallingDscp())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(46));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_WIFI))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    // nPreferredImsDscp is PREFERRED_DSCP_CELLULAR but GetImsSignallingDscp is 0
    m_pAosRegistration->SetDynamicIpQos();

    // nPreferredImsDscp is PREFERRED_DSCP_CELLULAR but ePDG is enabled
    m_pAosRegistration->SetDynamicIpQos();

    // nPreferredImsDscp is PREFERRED_DSCP_WIFI but ePDG is not enabled
    m_pAosRegistration->SetDynamicIpQos();

    // nPreferredImsDscp is PREFERRED_DSCP_NONE
    m_pAosRegistration->SetDynamicIpQos();
}

TEST_F(AosRegistrationTest, GetActualWaitTime)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_SPECIFIED_INTERVAL));
    m_pAosRegistration->m_nConsecutiveFailure = 2;

    // size of RegRetryIntervals and RegRandomRetryIntervals is different
    ImsVector<IMS_SINT32> objInterval;
    objInterval.Add(1000);
    objInterval.Add(2000);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryIntervals())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objInterval));
    ImsVector<IMS_SINT32> objRandomInterval;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRandomRetryIntervals())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRandomInterval));
    m_pAosRegistration->GetActualWaitTime();

    // size of RegRetryIntervals and RegRandomRetryIntervals is same
    objRandomInterval.Add(1000);
    objRandomInterval.Add(2000);
    m_pAosRegistration->GetActualWaitTime();
}

TEST_F(AosRegistrationTest, TryNextPcscf)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;

    // succeed to SetNextPcscf - bHonorRetryAfter is true
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(AString("60")));

    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(m_pAosRegistration->TryNextPcscf(IMS_TRUE, IMS_TRUE));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_STOP_RETRY);

    // succeed to SetNextPcscf - bHonorRetryAfter is false - succeed to SendRegister
    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(1);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(m_pAosRegistration->TryNextPcscf(IMS_TRUE, IMS_FALSE));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);

    // succeed to SetNextPcscf - bHonorRetryAfter is false - fail to SendRegister
    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, Register(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FAILURE));
    EXPECT_FALSE(m_pAosRegistration->TryNextPcscf(IMS_TRUE, IMS_FALSE));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // fail to SetNextPcscf - bFlowRecoveryOnAllFail is true
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(4));

    EXPECT_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).WillOnce(Return(IMS_FALSE));
    EXPECT_TRUE(m_pAosRegistration->TryNextPcscf(IMS_TRUE, IMS_FALSE));

    // fail to SetNextPcscf - bFlowRecoveryOnAllFail is false
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    EXPECT_FALSE(m_pAosRegistration->TryNextPcscf(IMS_FALSE, IMS_FALSE));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosRegistrationTest, SetNextPcscf_SamePcscf)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));

    ImsList<IMS_SINT32> objPcscfPorts;
    objPcscfPorts.Append(5060);
    objPcscfPorts.Append(5060);
    objPcscfPorts.Append(5060);
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfsPorts())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfPorts));

    m_pAosRegistration->SetPcscf();

    ASSERT_TRUE(m_pAosRegistration->m_strPcscf.Equals(m_objPcscfs.GetElementAt(0)));
    ASSERT_EQ(m_pAosRegistration->m_nPcscfPort, objPcscfPorts.GetAt(0));

    AString strCurrentPcscf = m_pAosRegistration->m_strPcscf;
    IMS_UINT32 nCurrentPort = m_pAosRegistration->m_nPcscfPort;

    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    m_pAosRegistration->SetNextPcscf();

    EXPECT_TRUE(strCurrentPcscf.Equals(m_pAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPort, m_pAosRegistration->m_nPcscfPort);
}

TEST_F(AosRegistrationTest, IsRetryOnSamePcscfRequired)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(3)
            .WillOnce(Return(0))
            .WillOnce(Return(2))
            .WillOnce(Return(2));

    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(2)
            .WillOnce(Return(3))
            .WillOnce(Return(0));

    EXPECT_FALSE(m_pAosRegistration->IsRetryOnSamePcscfRequired());
    EXPECT_FALSE(m_pAosRegistration->IsRetryOnSamePcscfRequired());
    EXPECT_TRUE(m_pAosRegistration->IsRetryOnSamePcscfRequired());
}

TEST_F(AosRegistrationTest, RegReinitiateMessageTriggersReinitiate)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    ImsMessage objMsg(TestAosRegistration::MSG_REG_REINITIATE, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, RegUpdateMessageTriggersUpdate)
{
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_UPDATE;

    ImsMessage objMsg(TestAosRegistration::MSG_REG_UPDATE, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_UPDATE));
}

TEST_F(AosRegistrationTest, RegReconfigMessageTriggersReconfig)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_RECONFIG;
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_UPDATE;

    ImsMessage objMsg(TestAosRegistration::MSG_REG_RECONFIG, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_UPDATE));
    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_RECONFIG));
}

TEST_F(AosRegistrationTest, RegRequiredWithWaitTimeMessageStartsOfflineRecoverTimer)
{
    IMS_SINT32 nWaitTime = 30;
    m_pAosRegistration->SetAppReady(IMS_TRUE);

    EXPECT_CALL(m_objMockITimer, SetTimer(nWaitTime * 1000, _)).Times(1);

    ImsMessage objMsg(TestAosRegistration::MSG_REG_REQUIRED_WITH_WAIT_TIME, nWaitTime, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, RegRequiredWithNextPcscfMessageTriggersRegister)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillOnce(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf()).WillOnce(Return(0));
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    ImsMessage objMsg(TestAosRegistration::MSG_REG_REQUIRED_WITH_NEXT_PCSCF, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, RegRequiredWithAvailableNextPcscfMessageTriggersRegister)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillOnce(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount()).WillOnce(Return(1));
    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    ImsMessage objMsg(TestAosRegistration::MSG_REG_REQUIRED_WITH_NEXT_PCSCF, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, RegTerminatedByNotifyMessageNotifiesFailure)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_FORBIDDEN));

    ImsMessage objMsg(TestAosRegistration::MSG_REG_TERMINATED_BY_NOTIFY, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, SubReinitiateMessageCreatesSubscription)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_SUBSCRIPTION;
    m_pAosRegistration->m_pSubscription = &m_objMockAosSubscription;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;

    EXPECT_CALL(m_objMockAosSubscription, Destroy()).Times(1);
    EXPECT_CALL(m_objMockIRegistration, CreateSubscription(_))
            .WillOnce(Return(&m_objMockIRegSubscription));
    EXPECT_CALL(m_objMockAosSubscription, Start(_)).Times(1);

    ImsMessage objMsg(TestAosRegistration::MSG_SUB_REINITIATE, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);
}

TEST_F(AosRegistrationTest, SubTerminatedMessageDestroiesSubscription)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_SUBSCRIPTION;
    m_pAosRegistration->m_pSubscription = &m_objMockAosSubscription;

    EXPECT_CALL(m_objMockAosSubscription, Destroy()).Times(1);

    ImsMessage objMsg(TestAosRegistration::MSG_SUB_TERMINATED, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);
}

TEST_F(AosRegistrationTest, RegisteredEventMessageClearsRetryCount)
{
    m_pAosRegistration->m_nConsecutiveFailure = 2;

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .WillOnce(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_NOTIFY));

    ImsMessage objMsg(TestAosRegistration::MSG_REG_EVENT_REGISTERED, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_EQ(m_pAosRegistration->m_nConsecutiveFailure, 0);
}

TEST_F(AosRegistrationTest, HandleUninterestedMessageDoesNothing)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy()).Times(0);

    ImsMessage objMsg(TestAosRegistration::MSG_REG_START, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);
}

TEST_F(AosRegistrationTest, InitializeSetsFeaturesAndListeners)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsSubscription()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosBlock, SetListener(_)).Times(1);
    EXPECT_CALL(m_objMockIAosCallTracker, SetListener(_)).Times(1);
    EXPECT_CALL(m_objMockIAosTransaction, SetListener(_, _)).Times(1);

    m_pAosRegistration->Init();

    EXPECT_TRUE(m_pAosRegistration->IsFeatureOn(TestAosRegistration::FEATURE_SUBSCRIPTION));
    EXPECT_TRUE(m_pAosRegistration->IsFeatureOn(TestAosRegistration::FEATURE_IPSEC));
}

TEST_F(AosRegistrationTest, CleanUpRemovesListeners)
{
    EXPECT_CALL(m_objMockIAosBlock, RemoveListener(_)).Times(1);
    EXPECT_CALL(m_objMockIAosCallTracker, RemoveListener(_)).Times(1);
    EXPECT_CALL(m_objMockIAosTransaction, RemoveListener(_, _)).Times(1);

    m_pAosRegistration->CleanUp();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, GeolocationInfoIsRequiredForNormalTypeIfPidfIsSupportedForCellular)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosRegistration->m_eRegType = AosRegistrationType::NORMAL;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_FALSE));

    EXPECT_TRUE(m_pAosRegistration->IsGeolocationInfoRequired());
}

TEST_F(AosRegistrationTest, GeolocationInfoIsRequiredForNormalTypeIfPidfIsSupportedForWifi)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    m_pAosRegistration->UpdateRegIpcanCategory();
    m_pAosRegistration->m_eRegType = AosRegistrationType::NORMAL;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_FALSE));

    EXPECT_TRUE(m_pAosRegistration->IsGeolocationInfoRequired());
}

TEST_F(AosRegistrationTest, GeolocationInfoIsRequiredForEmergencyTypeIfPidfIsSupportedForCellular)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosRegistration->m_eRegType = AosRegistrationType::EMERGENCY;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_FALSE));

    EXPECT_TRUE(m_pAosRegistration->IsGeolocationInfoRequired());
}

TEST_F(AosRegistrationTest, GeolocationInfoIsRequiredForEmergencyTypeIfPidfIsSupportedForWifi)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    m_pAosRegistration->UpdateRegIpcanCategory();
    m_pAosRegistration->m_eRegType = AosRegistrationType::EMERGENCY;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_FALSE));

    EXPECT_TRUE(m_pAosRegistration->IsGeolocationInfoRequired());
}

TEST_F(AosRegistrationTest, GeolocationInfoIsNotRequiredForFakeType)
{
    m_pAosRegistration->m_eRegType = AosRegistrationType::FAKE;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).Times(0);

    EXPECT_FALSE(m_pAosRegistration->IsGeolocationInfoRequired());
}

TEST_F(AosRegistrationTest, GeolocationInfoIsRequiredForNormalTypeIfPidfIsNotSupportedForCellular)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_FALSE));
    m_pAosRegistration->m_eRegType = AosRegistrationType::NORMAL;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).Times(0);

    EXPECT_FALSE(m_pAosRegistration->IsGeolocationInfoRequired());
}

TEST_F(AosRegistrationTest, GeolocationInfoIsNotRequiredIfIpsecHelperIsNull)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosRegistration->m_eRegType = AosRegistrationType::NORMAL;
    m_pAosRegistration->m_pIpsecHelper = IMS_NULL;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_TRUE));

    EXPECT_FALSE(m_pAosRegistration->IsGeolocationInfoRequired());
}

TEST_F(AosRegistrationTest, GeolocationInfoIsNotRequiredIfIpsecIsNotEstablished)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosRegistration->m_eRegType = AosRegistrationType::NORMAL;
    m_pAosRegistration->m_pIpsecHelper = &m_objMockAosIpsecHelper;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockAosIpsecHelper, IsEstablished()).WillOnce(Return(IMS_FALSE));

    EXPECT_FALSE(m_pAosRegistration->IsGeolocationInfoRequired());
}

TEST_F(AosRegistrationTest, CheckPendingWhilePendingReconfigExistSendsRegReconfigMessage)
{
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_RECONFIG;

    EXPECT_CALL(m_objMockThread, PostMessageI(IsSameMsg(TestAosRegistration::MSG_REG_RECONFIG)));

    m_pAosRegistration->CheckPending();
}

TEST_F(AosRegistrationTest, CheckPendingWhilePendingUpdateExistSendsRegUpdateMessage)
{
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_UPDATE;

    EXPECT_CALL(m_objMockThread, PostMessageI(IsSameMsg(TestAosRegistration::MSG_REG_UPDATE)));

    m_pAosRegistration->CheckPending();
}

TEST_F(AosRegistrationTest, ProcessPendingStartTriggersStartRegister)
{
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_START;

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_START));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ProcessPendingTrafficWhileInRefreshStopStateTriggersUpdateRegister)
{
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_TRAFFIC;
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_TRANSACTION;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHSTOP);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_TRAFFIC));
    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_TRANSACTION));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosRegistrationTest, ProcessPendingTrafficWhileInOfflineStateTriggersStartRegister)
{
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_TRAFFIC;
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_TRANSACTION;

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_TRAFFIC));
    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_TRANSACTION));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ProcessPendingTransactionWhileRetryTimerExistDoesNothing)
{
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_TRANSACTION;
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_STOP_RETRY, 10000);

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_TRANSACTION));
}

TEST_F(AosRegistrationTest, ProcessPendingTransactionWhileRetryIsHeldButFailsToSendRegister)
{
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_TRANSACTION;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_TRANSACTION));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, ProcessPendingTransactionWhileRetryIsHeldAndSucceedsToSendRegister)
{
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_TRANSACTION;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_TRANSACTION));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest,
        ProcessPendingTransactionWhileRetryIsNotHeldAndInOfflineStateButFailsToRegister)
{
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_TRANSACTION;

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_TRANSACTION));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest,
        ProcessPendingTransactionWhileRetryIsNotHeldAndInOfflineStateAndSucceedsToRegister)
{
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_TRANSACTION;

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_TRANSACTION));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ProcessPendingTransactionWhileRetryIsNotHeldAndNotInOfflineState)
{
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_TRANSACTION;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_TRANSACTION));
}

TEST_F(AosRegistrationTest, ProcessPendingSubscriptionTriggersStartSubscription)
{
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_SUBSCRIPTION;
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_SUBSCRIPTION;
    m_pAosRegistration->m_pSubscription = &m_objMockAosSubscription;

    EXPECT_CALL(m_objMockAosSubscription, Start(_)).Times(1);

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_SUBSCRIPTION));
}

TEST_F(AosRegistrationTest, ProcessRetryInRegStoppedWhileRetryIsNotHeldNotTriesRegister)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);

    m_pAosRegistration->ProcessRetryInRegStopped(IMS_TRUE);
}

TEST_F(AosRegistrationTest, ProcessRetryInRegStoppedWhileRetryTimerExistNotTriesRegister)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_STOP_RETRY, 10000);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);

    m_pAosRegistration->ProcessRetryInRegStopped(IMS_FALSE);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_STOP_RETRY);
}

TEST_F(AosRegistrationTest, ProcessRetryInRegStoppedWhileTransactionIsNotStartedNotTriesRegister)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pAosRegistration->m_bIsTransactionStarted = IMS_FALSE;

    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);

    m_pAosRegistration->ProcessRetryInRegStopped(IMS_FALSE);

    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_TRAFFIC));
}

TEST_F(AosRegistrationTest, ProcessRetryInRegStoppedWhileTransactionIsNotAllowedNotTriesRegister)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);

    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(IAosTransaction::TYPE_REG))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);

    m_pAosRegistration->ProcessRetryInRegStopped(IMS_FALSE);

    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_TRAFFIC));
}

TEST_F(AosRegistrationTest, ProcessRetryInRegStoppedButFailsToSendRegister)
{
    m_pAosRegistration->m_eRegType = AosRegistrationType::EMERGENCY;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessRetryInRegStopped(IMS_FALSE);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, ProcessReregister)
{
    // not STATE_REGISTERED
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pAosRegistration->ProcessReregister();
    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(IAosTransaction::TYPE_REG)).Times(0);

    // CheckRadioReadyAndSetTxnPending() is false
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(IAosTransaction::TYPE_REG))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE));
    m_pAosRegistration->ProcessReregister();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);

    // fail to SendRegister - ProcessUnpredictableFailureHeldByCall return true
    m_pAosRegistration->m_bIsTransactionStarted = IMS_TRUE;
    m_pAosRegistration->m_piRegistration = IMS_NULL;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    m_pAosRegistration->ProcessReregister();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);

    // fail to SendRegister - ProcessUnpredictableFailureHeldByCall return false
    m_pAosRegistration->m_bIsTransactionStarted = IMS_TRUE;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetImsCall(IMS_FALSE);
    m_pAosRegistration->ProcessReregister();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // succeed to SendRegister
    m_pAosRegistration->m_bIsTransactionStarted = IMS_TRUE;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_UPDATE));
    m_pAosRegistration->ProcessReregister();
}

TEST_F(AosRegistrationTest, ProcessRegTerminatedWhileCallExist)
{
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _)).Times(0);

    m_pAosRegistration->ProcessRegTerminated();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_TRUE(m_pAosRegistration->m_bIsHeldByCall);
    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_TRANSACTION));
}

TEST_F(AosRegistrationTest, ProcessRegTerminatedWhileRetryTimerExist)
{
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_STOP_RETRY, 10000);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _)).Times(0);

    m_pAosRegistration->ProcessRegTerminated();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, ProcessAuthenticationFailed)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED))
            .WillOnce(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED))
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));
    m_pAosRegistration->m_piRegistration = IMS_NULL;

    // ERROR_POLICY_PDN_REACTIVATED - STATE_REGISTERING
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosRegistration->ProcessAuthenticationFailed();

    // ERROR_POLICY_PDN_REACTIVATED - STATE_REFRESHING
    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    m_pAosRegistration->ProcessAuthenticationFailed();

    // Not ERROR_POLICY_PDN_REACTIVATED
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_AUTHENTICATION));
    m_pAosRegistration->ProcessAuthenticationFailed();
}

TEST_F(AosRegistrationTest, ProcessForbiddenFailed)
{
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrCode));
    ImsVector<IMS_SINT32> objCount;
    objCount.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrMaxCount())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objCount));

    // Error code is exist in RegPermanentErrCode
    EXPECT_FALSE(m_pAosRegistration->ProcessForbiddenFailed(SipStatusCode::SC_403));

    // m_nForbiddenCount is greater than or equal to nMaxCount
    objErrCode.Add(403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_TYPE_CRITICAL));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_FORBIDDEN))
            .Times(AnyNumber());
    EXPECT_TRUE(m_pAosRegistration->ProcessForbiddenFailed(SipStatusCode::SC_403));

    // m_nForbiddenCount is less than nMaxCount - Not Registered
    objCount.Add(5);
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());
    EXPECT_TRUE(m_pAosRegistration->ProcessForbiddenFailed(SipStatusCode::SC_403));

    // m_nForbiddenCount is less than nMaxCount - Registered
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_TRUE(m_pAosRegistration->ProcessForbiddenFailed(SipStatusCode::SC_403));
}

TEST_F(AosRegistrationTest, ProcessRegRequiredWithAvailableNextPcscf)
{
    // fail to SetNextPcscf
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));
    m_pAosRegistration->ProcessRegRequiredWithAvailableNextPcscf(IMS_FALSE, 0);

    // succeed to SetNextPcscf
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));

    // fail to CreateRegistration
    AStringArray objEmptyImpus;
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .Times(AnyNumber())
            .WillOnce(ReturnRef(objEmptyImpus))
            .WillRepeatedly(ReturnRef(m_objImpus));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pAosRegistration->ProcessRegRequiredWithAvailableNextPcscf(IMS_FALSE, 0);

    // succeed to CreateRegistration
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pAosRegistration->ProcessRegRequiredWithAvailableNextPcscf(IMS_FALSE, 0);

    // nRetryAfter > 0
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(AString("60")));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).Times(AnyNumber()).WillRepeatedly(Return(1));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pAosRegistration->ProcessRegRequiredWithAvailableNextPcscf(IMS_FALSE, 0);
}

TEST_F(AosRegistrationTest, ProcessSubscriberFailed)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_SUBSCRIBER_FAILED));
    ImsVector<IMS_SINT32> objReregErrCode;
    objReregErrCode.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraReregErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objReregErrCode));
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objExtraRegErrCode));

    // STATE_REGISTERED - GetExtraReregErrCode does not have ErrCode
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(0);
    EXPECT_FALSE(m_pAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403));

    // STATE_REFRESHING - m_nConsecutiveFailure is 1
    objReregErrCode.Add(403);
    m_pAosRegistration->m_nConsecutiveFailure = 1;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(0);
    EXPECT_TRUE(m_pAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403));
    EXPECT_EQ(m_pAosRegistration->m_nConsecutiveFailure, 0);

    // STATE_OFFLINE - GetExtraRegErrCode does not have ErrCode
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(0);
    EXPECT_FALSE(m_pAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403));

    // there are PUIDs more than 1 - succeed to SetNextPcscf
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(AnyNumber());
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->m_strPuid = m_objImpus.GetElementAt(0);
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillOnce(ReturnRef(m_objImpus));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    ImsVector<IMS_SINT32> objRegErrWaitTime;
    objRegErrWaitTime.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrWaitTime())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrWaitTime));
    EXPECT_TRUE(m_pAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403));

    // there is not PUID more than 1
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    AStringArray objEmptyImpus;
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillOnce(ReturnRef(objEmptyImpus));
    EXPECT_TRUE(m_pAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403));
}

TEST_F(AosRegistrationTest, ProcessDefaultFlowRecovery_Start)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    ImsVector<IMS_SINT32> objErrorCode;
    objErrorCode.Clear();
    objErrorCode.Add(SipStatusCode::SC_600);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithRetryAfterTime())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrorCode));

    // ActualWaitTimePolicy is AWT_POLICY_FAILURE_TO_EVERY_PCSCF
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EVERY_PCSCF));
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(IMS_FALSE, 0)).Times(1);
    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    // ActualWaitTimePolicy is AWT_POLICY_SPECIFIED_INTERVAL
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::AWT_POLICY_SPECIFIED_INTERVAL));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfTried()).Times(1);
    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    // ActualWaitTimePolicy is AWT_POLICY_FAILURE_TO_EACH_PCSCF - succeed to SetNextPcscf
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EACH_PCSCF));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_STOP_RETRY);

    // ActualWaitTimePolicy is AWT_POLICY_FAILURE_TO_EACH_PCSCF - fail to SetNextPcscf
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(4));
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));
    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    // ActualWaitTimePolicy is AWT_POLICY_RFC_RULE
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(IMS_TRUE, _)).Times(1);

    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_600);
}

TEST_F(AosRegistrationTest, ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy)
{
    // succeed to SetNextPcscf - nRetryAfter is greater than 0
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(10);
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));

    // succeed to SetNextPcscf - nRetryAfter is 0 - fail to SendRegister
    m_pAosRegistration->m_piRegistration = IMS_NULL;
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(0);

    // succeed to SetNextPcscf - nRetryAfter is 0 - succeed to SendRegister
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_CALL(m_objMockIRegistration, Register(_))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(0);

    // fail to SetNextPcscf - succeed to SetFirstPcscf
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(4));
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));
    m_pAosRegistration->m_piRegParameter = &m_objMockIRegParameter;
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(0);
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));

    // fail to SetNextPcscf - fail to SetFirstPcscf
    EXPECT_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(0);
}

TEST_F(AosRegistrationTest, ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy)
{
    // retry counter is shared between REGISTER and SUBSCRIBE - retry count can be increased
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objInterval;
    objInterval.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryIntervals())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objInterval));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL))
            .Times(2);
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));

    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_STOP_RETRY);
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(0);
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));

    // retry counter is shared between REGISTER and SUBSCRIBE - retry count reaches max count
    // succeed to SetNextPcscf
    EXPECT_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);

    // retry counter is shared between REGISTER and SUBSCRIBE - retry count reaches max count
    // fail to SetNextPcscf
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount()).WillOnce(Return(4));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);

    // retry counter is not shared between REGISTER and SUBSCRIBE - Retry count can be increased
    // succeed to SetNextPcscf
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount()).WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(0);

    // retry counter is not shared between REGISTER and SUBSCRIBE - Retry count can be increased
    // fail to SetNextPcscf
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount()).WillOnce(Return(4));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT));
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);
}

TEST_F(AosRegistrationTest, ProcessDefaultFlowRecovery_Update)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objErrorCode;
    objErrorCode.Clear();
    objErrorCode.Add(SipStatusCode::SC_600);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregErrCodeWithRetryAfterTime())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrorCode));
    ImsVector<IMS_SINT32> objInterval;
    objInterval.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryIntervals())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objInterval));

    // ActualWaitTimePolicy is AWT_POLICY_FAILURE_TO_EVERY_PCSCF - nRetryAfter is less than 0
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EVERY_PCSCF));
    AStringArray objEmptyImpus;
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .Times(AnyNumber())
            .WillOnce(ReturnRef(objEmptyImpus))
            .WillRepeatedly(ReturnRef(m_objImpus));

    // fail to CreateRegistration
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_600);

    // succeed to CreateRegistration
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_600);

    // ActualWaitTimePolicy is AWT_POLICY_FAILURE_TO_EVERY_PCSCF - nRetryAfter is greater than 0
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());

    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(AString("60")));
    m_pAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_600);
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_OFFLINE_RECOVER));
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_OFFLINE_RECOVER);

    // ActualWaitTimePolicy is AWT_POLICY_SPECIFIED_INTERVAL
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_SPECIFIED_INTERVAL));
    m_pAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_300);

    // ActualWaitTimePolicy is AWT_POLICY_RFC_RULE
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    m_pAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_300);
}

TEST_F(AosRegistrationTest, ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy)
{
    ImsVector<IMS_SINT32> objErrorCode;
    objErrorCode.Clear();
    objErrorCode.Add(SipStatusCode::SC_600);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraReregErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrorCode));
    ImsVector<IMS_SINT32> objInterval;
    objInterval.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryIntervals())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objInterval));
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());

    // retry counter is shared between REGISTER and SUBSCRIBE
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    // GetExtraReregErrCode have nStatusCode
    m_pAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_600, 0);

    // GetExtraReregErrCode dose not have nStatusCode - retry count can be increased
    EXPECT_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));
    m_pAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_300, 0);

    // GetExtraReregErrCode dose not have nStatusCode - retry count reaches max count
    m_pAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_300, 0);

    // retry counter is not shared between REGISTER and SUBSCRIBE
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // nStatusCode is SC_481
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_481, 0);

    // m_nConsecutiveFailure is 1 and Reg is not Expired During Awt
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));
    m_pAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_300, 0);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
    EXPECT_EQ(m_pAosRegistration->m_nConsecutiveFailure, 1);

    // m_nConsecutiveFailure is 1 and Reg is Expired During Awt
    m_pAosRegistration->m_piRegContact = &m_objMockIRegContact;
    EXPECT_CALL(m_objMockIRegContact, GetExpires()).Times(AnyNumber()).WillRepeatedly(Return(100));
    m_pAosRegistration->m_nConsecutiveFailure = 0;
    m_pAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_300, 100);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_EQ(m_pAosRegistration->m_nConsecutiveFailure, 1);

    // m_nConsecutiveFailure is 2
    m_pAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_300, 0);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
    EXPECT_EQ(m_pAosRegistration->m_nConsecutiveFailure, 2);
}

TEST_F(AosRegistrationTest, ProcessStartFailed_TxnTimeout_RegRetryCountPerPcscfConfigured)
{
    // BEGIN uninteresting preparation
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    ImsVector<IMS_SINT32> objErrWithoutIpsec;
    objErrWithoutIpsec.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrWithoutIpsec));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(AnyNumber());

    AString strHeader = AString("regtest");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(strHeader));

    EXPECT_CALL(m_objMockIAosService, NotifyRegistering(_, _, _)).Times(AnyNumber());

    m_pAosRegistration->m_piRegParameter = &m_objMockIRegParameter;

    ImsVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    // END uninteresting preparation

    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));

    EXPECT_CALL(m_objMockIAosPcscf, IncreaseCurrentPcscfTriedCount()).Times(AnyNumber());

    m_pAosRegistration->SetPcscf();

    AString strCurrentPcscf = m_pAosRegistration->m_strPcscf;
    IMS_UINT32 nCurrentPcscfPort = m_pAosRegistration->m_nPcscfPort;

    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(IMS_TRUE, _)).Times(2);

    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgReferee<0>(m_objPcscfs.GetElementAt(1)),
                    SetArgReferee<1>(m_objPcscfPorts.GetAt(1)), Return(IMS_TRUE)))
            .WillOnce(Return(IMS_FALSE));

    // 1st Try failed
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    m_pAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(strCurrentPcscf.Equals(m_pAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPcscfPort, m_pAosRegistration->m_nPcscfPort);

    // 2nd Try failed
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));
    m_pAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(strCurrentPcscf.Equals(m_pAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPcscfPort, m_pAosRegistration->m_nPcscfPort);

    // 3rd Try failed
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    m_pAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(strCurrentPcscf.Equals(m_pAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPcscfPort, m_pAosRegistration->m_nPcscfPort);

    // 4th Try failed (on next pcscf)
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(4));
    m_pAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_FALSE(strCurrentPcscf.Equals(m_pAosRegistration->m_strPcscf));
    EXPECT_NE(nCurrentPcscfPort, m_pAosRegistration->m_nPcscfPort);
    strCurrentPcscf = m_pAosRegistration->m_strPcscf;
    nCurrentPcscfPort = m_pAosRegistration->m_nPcscfPort;

    // 5th Try failed
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    m_pAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(strCurrentPcscf.Equals(m_pAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPcscfPort, m_pAosRegistration->m_nPcscfPort);

    // 6th Try failed
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));
    m_pAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(strCurrentPcscf.Equals(m_pAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPcscfPort, m_pAosRegistration->m_nPcscfPort);

    // 7th Try failed
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    m_pAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(strCurrentPcscf.Equals(m_pAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPcscfPort, m_pAosRegistration->m_nPcscfPort);

    // 8th Try failed (failed next pcscf)
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(4));
    m_pAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosRegistrationTest, ProcessStartFailed_TxnTimeout_RegRetryCountOnSinglePcscfConfigured)
{
    // BEGIN uninteresting preparation
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(AnyNumber());

    AString strHeader = AString("regtest");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(strHeader));

    EXPECT_CALL(m_objMockIAosService, NotifyRegistering(_, _, _)).Times(AnyNumber());

    ImsVector<IMS_SINT32> objExtraRegErrCodeList;
    objExtraRegErrCodeList.Add(0);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objExtraRegErrCodeList));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryTimerFPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::TIMER_F_POLICY_SPEC_WITH_AWT));

    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(AnyNumber());
    // END uninteresting preparation

    ImsVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));

    EXPECT_CALL(m_objMockIAosPcscf, IncreaseCurrentPcscfTriedCount()).Times(AnyNumber());

    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).Times(AnyNumber()).WillRepeatedly(Return(1));

    m_pAosRegistration->SetPcscf();

    AString strCurrentPcscf = m_pAosRegistration->m_strPcscf;
    IMS_UINT32 nCurrentPcscfPort = m_pAosRegistration->m_nPcscfPort;

    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_FALSE));

    // 1st Try failed
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    m_pAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(strCurrentPcscf.Equals(m_pAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPcscfPort, m_pAosRegistration->m_nPcscfPort);

    // 2nd Try failed (failed next pcscf)
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));
    m_pAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosRegistrationTest, ProcessIpVersionChange)
{
    AString strIpv4Addr = "192.168.0.1";
    AString strIpv6Addr = "fc01:cafe::1";
    m_pAosRegistration->m_strPcscf = strIpv4Addr;

    IpAddress objLocalIpv4Addr(AString("192.186.0.100"));
    IpAddress objLocalIpv6Addr(AString("fc01:abab:cdcd:efe0::1"));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objLocalIpv4Addr));
    EXPECT_FALSE(m_pAosRegistration->ProcessIpVersionChange());

    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objLocalIpv6Addr));

    AStringArray objPcscfs;
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV6))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfs));
    EXPECT_FALSE(m_pAosRegistration->ProcessIpVersionChange());

    objPcscfs.AddElement(strIpv6Addr);
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV6))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfs));

    EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(2);
    EXPECT_CALL(m_objMockIAosPcscf, SetFirstPcscfIndex()).Times(2);
    EXPECT_TRUE(m_pAosRegistration->ProcessIpVersionChange());

    m_pAosRegistration->m_strPcscf = strIpv6Addr;
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objLocalIpv4Addr));
    objPcscfs.RemoveAllElements();
    objPcscfs.AddElement(strIpv4Addr);
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV4))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfs));
    EXPECT_TRUE(m_pAosRegistration->ProcessIpVersionChange());
}

TEST_F(AosRegistrationTest, ProcessStartFailed_StatusCode)
{
    // ProcessStandardPcscfSelection
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Clear();
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_300);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_300);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // ProcessForbiddenFailed return true and ProcessSubscriberFailed return false
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Clear();
    objErrCode.Add(SipStatusCode::SC_403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrCode));
    ImsVector<IMS_SINT32> objCount;
    objCount.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrMaxCount())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objCount));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_403);

    // ProcessIpsecFallback
    ImsVector<IMS_SINT32> objErrWithoutIpsec;
    objErrWithoutIpsec.Clear();
    objErrWithoutIpsec.Add(SipStatusCode::SC_406);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrWithoutIpsec));
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_406);

    // PDN reactivation is required
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED))
            .WillOnce(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED))
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));

    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Clear();
    objExtraRegErrCode.Add(SipStatusCode::SC_408);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objExtraRegErrCode));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .Times(AnyNumber())
            .WillOnce(Return(1));
    EXPECT_CALL(
            m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .Times(AnyNumber())
            .WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).Times(AnyNumber()).WillOnce(Return(1));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));
    m_pAosRegistration->m_bEps5GsOnly = IMS_TRUE;
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_408);

    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());

    // ProcessStartFailed_503 - nRetryAfter is 0
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetrySip503CodePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::SIP_503_CODE_POLICY_3GPP));
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_503);

    // ProcessStartFailed_420 - bIsExtensionUnsupported is false
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_420);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // ProcessStartFailed_421 - bIsExtensionRequired  is false
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_421);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // ProcessStartFailed_423 - nMinTime is less than 0
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_423);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // Other 4xx, 5xx, 6xx response
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_580);
}

TEST_F(AosRegistrationTest, ProcessStartFailed_StatusCode_ProcessStartFailed_305)
{
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrCode));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));

    // SIP_305_CODE_POLICY_3GPP
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetrySip305CodePolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP));
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF - first PCSCF
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetrySip305CodePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF));
    EXPECT_CALL(m_objMockIAosPcscf, IsFirstPcscf())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF - not first PCSCF - fail to SetFirstPcscf
    EXPECT_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);

    // SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF - not first PCSCF - succeed to SetFirstPcscf
    // fail to SendRegister
    m_pAosRegistration->m_piRegParameter = &m_objMockIRegParameter;
    m_pAosRegistration->m_piRegistration = IMS_NULL;
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);

    // succeed to SendRegister
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);

    // SIP_305_CODE_POLICY_DEFAULT
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetrySip305CodePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT));
    ImsVector<IMS_SINT32> objErrWithoutIpsec;
    objErrWithoutIpsec.Clear();
    objErrWithoutIpsec.Add(SipStatusCode::SC_305);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrWithoutIpsec));
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);
}

TEST_F(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_Success)
{
    // BEGIN uninteresting preparation
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(AnyNumber());

    AString strHeader = AString("regtest");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(strHeader));

    EXPECT_CALL(m_objMockIAosService, NotifyRegistering(_, _, _)).Times(AnyNumber());

    ImsVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));
    // END uninteresting preparation

    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(m_objMockIAosPcscf, HasNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosPcscf, IncreaseCurrentPcscfTriedCount()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTriedCount()).Times(AnyNumber());

    AStringArray objPcscfsIpv6;
    objPcscfsIpv6.AddElement(AString("fc01:cafe::1"));
    objPcscfsIpv6.AddElement(AString("fc01:cafe::2"));
    objPcscfsIpv6.AddElement(AString("fc01:cafe::3"));

    AStringArray objPcscfsIpv4;
    objPcscfsIpv4.AddElement(AString("192.168.0.1"));
    objPcscfsIpv4.AddElement(AString("192.168.0.2"));
    objPcscfsIpv4.AddElement(AString("192.168.0.3"));

    IpAddress objLocalIpa = IpAddress(AString("192.168.0.100"));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objLocalIpa));

    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV4))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfsIpv4));
    EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(AnyNumber());

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegRetryWithIpVerFallback())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL))
            .Times(100);

    for (IMS_SINT32 i = SipStatusCode::SC_500; i < SipStatusCode::SC_600; i++)
    {
        EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(objPcscfsIpv6));

        m_pAosRegistration->SetPcscf();

        EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(objPcscfsIpv4));

        m_pAosRegistration->ProcessStartFailed_StatusCode(i);
        EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    }
}

TEST_F(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_Failure)
{
    // BEGIN uninteresting preparation
    ImsVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetrySip503CodePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosPcscf, IncreaseCurrentPcscfTriedCount()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, HasNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    // END uninteresting preparation

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegRetryWithIpVerFallback())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    AString strIpv4Addr = "192.168.0.1";
    m_pAosRegistration->m_strPcscf = strIpv4Addr;
    m_pAosRegistration->m_nState = IAosRegistration::STATE_REGISTERING;

    IpAddress objLocalIpv4Addr(AString("192.186.0.100"));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objLocalIpv4Addr));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT));
    m_pAosRegistration->ProcessStartFailed_StatusCode(500);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_HasNextPcscf)
{
    // BEGIN uninteresting preparation
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(AnyNumber());

    AString strHeader = AString("regtest");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(strHeader));

    EXPECT_CALL(m_objMockIAosService, NotifyRegistering(_, _, _)).Times(AnyNumber());

    m_pAosRegistration->m_piRegParameter = &m_objMockIRegParameter;

    ImsVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));
    // END uninteresting preparation

    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(m_objMockIAosPcscf, HasNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosPcscf, IncreaseCurrentPcscfTriedCount()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTriedCount()).Times(AnyNumber());

    AStringArray objPcscfsIpv6;
    objPcscfsIpv6.AddElement(AString("fc01:cafe::1"));
    objPcscfsIpv6.AddElement(AString("fc01:cafe::2"));
    objPcscfsIpv6.AddElement(AString("fc01:cafe::3"));

    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _))
            .WillOnce(DoAll(SetArgReferee<0>(objPcscfsIpv6.GetElementAt(1)),
                    SetArgReferee<1>(m_objPcscfPorts.GetAt(1)), Return(IMS_TRUE)));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegRetryWithIpVerFallback())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillRepeatedly(Return(IMS_SUCCESS));

    m_pAosRegistration->m_nState = IAosRegistration::STATE_REGISTERING;

    m_pAosRegistration->ProcessStartFailed_StatusCode(500);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ProcessStartFailed_Others)
{
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .WillRepeatedly(ReturnRef(objExtraRegErrCode));
    m_pAosRegistration->m_bEps5GsOnly = IMS_FALSE;

    // ProcessAwtRecovery
    m_pAosRegistration->ProcessStartFailed_Others(IRegistration::REASON_NONE);
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_STOP_RETRY);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // REASON_CLIENT_SOCKET_ERROR - PDN reactivation is not required
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));
    m_pAosRegistration->ProcessStartFailed_Others(IRegistration::REASON_CLIENT_SOCKET_ERROR);

    // REASON_CLIENT_SOCKET_ERROR - PDN reactivation is required
    objExtraRegErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_TRANSPORT);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .Times(AnyNumber())
            .WillOnce(Return(1));
    EXPECT_CALL(
            m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .Times(AnyNumber())
            .WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).Times(AnyNumber()).WillOnce(Return(1));
    m_pAosRegistration->ProcessStartFailed_Others(IRegistration::REASON_CLIENT_SOCKET_ERROR);

    // REG_ERROR_CODE_OTHER is exist in ExtraRegErrCode
    objExtraRegErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_OTHER);
    m_pAosRegistration->ProcessStartFailed_Others(IRegistration::REASON_NONE);
}

TEST_F(AosRegistrationTest, ProcessUpdateFailed_StatusCode)
{
    m_pAosRegistration->SetImsCall(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraReregErrInRoamingAsFailureHandled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // SipStatusCode is in GetRegErrCodeWithoutIpsec
    ImsVector<IMS_SINT32> objErrWithoutIpsec;
    objErrWithoutIpsec.Add(SipStatusCode::SC_406);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrWithoutIpsec));
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_406);
    objErrWithoutIpsec.Clear();

    // SipStatusCode is in GetReregErrCodeForImsPdnReactivation
    ImsVector<IMS_SINT32> objReregErrCodeForImsPdnReactivation;
    objReregErrCodeForImsPdnReactivation.Add(SipStatusCode::SC_407);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForImsPdnReactivation())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objReregErrCodeForImsPdnReactivation));
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_407);
    objReregErrCodeForImsPdnReactivation.Clear();

    // ProcessForbiddenFailed return true and ProcessSubscriberFailed return false
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Add(SipStatusCode::SC_403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrCode));
    ImsVector<IMS_SINT32> objCount;
    objCount.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrMaxCount())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objCount));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_403);
    objErrCode.Clear();

    // REG_ERROR_CODE_ALL_RESP is in GetReregRetryErrCodeForInitRegWithSamePcscf
    ImsVector<IMS_SINT32> ReregRetryErrCodeForInitRegWithSamePcscf;
    ReregRetryErrCodeForInitRegWithSamePcscf.Add(CarrierConfig::Assets::REG_ERROR_CODE_ALL_RESP);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregRetryErrCodeForInitRegWithSamePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(ReregRetryErrCodeForInitRegWithSamePcscf));
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_302);
    ReregRetryErrCodeForInitRegWithSamePcscf.Clear();

    // ExtraRegErrPolicy is ERROR_POLICY_PDN_REACTIVATED - m_bEps5GsOnly is false
    m_pAosRegistration->m_bEps5GsOnly = IMS_FALSE;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_302);

    // ExtraRegErrPolicy is ERROR_POLICY_PDN_REACTIVATED - m_bEps5GsOnly is true
    m_pAosRegistration->m_bEps5GsOnly = IMS_TRUE;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .WillOnce(Return(1));
    EXPECT_CALL(
            m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).Times(AnyNumber()).WillOnce(Return(1));
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_302);

    // ExtraRegErrPolicy is AWT_POLICY_RFC_RULE  - SipStatusCode::SC_403
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_403);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);

    // ExtraRegErrPolicy is AWT_POLICY_RFC_RULE  - SipStatusCode::SC_600
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(4));
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_600);

    // ProcessUpdateFailed_423
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::MIN_EXPIRES, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(AString("60")));
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_423);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosRegistrationTest, ProcessUpdateFailed_StatusCode_ProcessUpdateFailed_305)
{
    m_pAosRegistration->SetImsCall(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraReregErrInRoamingAsFailureHandled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // SIP_305_CODE_POLICY_3GPP
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregRetrySip305CodePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP));
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    // SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF - first PCSCF
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregRetrySip305CodePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF));
    EXPECT_CALL(m_objMockIAosPcscf, IsFirstPcscf())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    // SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF - not first PCSCF - fail to SetFirstPcscf
    EXPECT_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    // SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF - not first PCSCF - succeed to SetFirstPcscf
    // fail to CreateRegistration
    AStringArray objEmptyImpus;
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .Times(AnyNumber())
            .WillOnce(ReturnRef(objEmptyImpus))
            .WillRepeatedly(ReturnRef(m_objImpus));
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    // SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF - not first PCSCF - succeed to SetFirstPcscf
    // succeed to CreateRegistration
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    // SIP_305_CODE_POLICY_DEFAULT
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregRetrySip305CodePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT));
    ImsVector<IMS_SINT32> objErrWithoutIpsec;
    objErrWithoutIpsec.Clear();
    objErrWithoutIpsec.Add(SipStatusCode::SC_305);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrWithoutIpsec));
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);
}

TEST_F(AosRegistrationTest, ProcessUpdateFailed_StatusCodeWhenReceived500)
{
    m_pAosRegistration->SetImsCall(IMS_FALSE);
    // return IMS_FALSE in ProcessUnpredictableFailureHeldByCall()
    ON_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .WillByDefault(Return(IMS_TRUE));

    // return IMS_FALSE in ProcessPlmnBlockOnUpdateFailure()
    ON_CALL(m_objMockIAosNConfiguration, IsExtraReregErrInRoamingAsFailureHandled())
            .WillByDefault(Return(IMS_FALSE));

    // for executing the ProcessDefaultFlowRecovery_Update
    ON_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .WillByDefault(Return(IMS_FALSE));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_500);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, AuthenticationChallengedIsNotHandledWhenRegistrationIsNull)
{
    IMS_BOOL bResponseToChallenge = IMS_FALSE;
    m_pAosRegistration->Registration_AuthenticationChallenged(
            Credential::TYPE_MD5, bResponseToChallenge);

    EXPECT_FALSE(bResponseToChallenge);
    EXPECT_EQ(m_pAosRegistration->m_nAuthChallengeCount, 0);
}

TEST_F(AosRegistrationTest, AuthenticationChallengedIsNotHandledWhenMoreAuthChallengeIsNotAllowed)
{
    IMS_UINT32 nCounterThreshold = TestAosRegistration::AUTHENTICATION_RETRY_MAX_COUNT;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_nAuthChallengeCount = nCounterThreshold;

    IMS_BOOL bResponseToChallenge = IMS_FALSE;
    m_pAosRegistration->Registration_AuthenticationChallenged(
            Credential::TYPE_MD5, bResponseToChallenge);

    EXPECT_FALSE(bResponseToChallenge);
    EXPECT_EQ(m_pAosRegistration->m_nAuthChallengeCount, nCounterThreshold + 1);
}

TEST_F(AosRegistrationTest, FailToProcessAuthenticationChallengedTriggersRegReInitate)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosRegistration->m_pIpsecHelper = &m_objMockAosIpsecHelper;
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_IPSEC;

    EXPECT_CALL(m_objMockAosIpsecHelper, ProcessAuthChallenged(_)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountWithIpsecOnAuthFailure())
            .WillOnce(Return(0));

    IMS_BOOL bResponseToChallenge = IMS_FALSE;
    m_pAosRegistration->Registration_AuthenticationChallenged(
            Credential::TYPE_MD5, bResponseToChallenge);

    EXPECT_FALSE(bResponseToChallenge);
    EXPECT_EQ(m_pAosRegistration->m_pIpsecHelper, nullptr);
    EXPECT_EQ(m_pAosRegistration->m_nIpsecBlockReason,
            TestAosRegistration::IPSEC_BLOCK_AUTENTICATION);
}

TEST_F(AosRegistrationTest, NotifyAkaResponseIsNotHandledWhenIpsecIsNotSupported)
{
    EXPECT_FALSE(m_pAosRegistration->IsIpsecSupported());

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_NOK_MAC_INVALID, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_FALSE(bResultOfSA);
}

TEST_F(AosRegistrationTest, NotifyAkaResponseTriggersRegTerminatedWhenResultIsNotOk)
{
    m_pAosRegistration->m_pIpsecHelper = &m_objMockAosIpsecHelper;
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_IPSEC;

    EXPECT_CALL(m_objMockAosIpsecHelper, Create(IMS_FALSE)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED));

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_NOK_MAC_INVALID, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_TRUE(bResultOfSA);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, NotifyAkaResponseTriggersRegTerminatedWhenButFailToSetPcscfPort)
{
    m_pAosRegistration->m_pIpsecHelper = &m_objMockAosIpsecHelper;
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_IPSEC;
    m_pAosRegistration->m_nAuthChallengeCount = 3;

    EXPECT_CALL(m_objMockAosIpsecHelper, CreateOnChallenging()).Times(1);
    EXPECT_CALL(m_objMockAosIpsecHelper, SetPcscfPortnSpi()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED));

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_OK, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_FALSE(bResultOfSA);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, NotifyAkaResponseTriggersRegTerminatedWhenFailToUpdatePreloadedRoute)
{
    m_pAosRegistration->m_pIpsecHelper = &m_objMockAosIpsecHelper;
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_IPSEC;

    EXPECT_CALL(m_objMockAosIpsecHelper, SetPcscfPortnSpi()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockAosIpsecHelper, IsPcscfServerPortDifferent()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockAosIpsecHelper, UpdatePreloadedRoute(_)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED));

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_OK, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_FALSE(bResultOfSA);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, NotifyAkaResponseTriggersRegTerminatedWhenFailToMakeSas)
{
    m_pAosRegistration->m_pIpsecHelper = &m_objMockAosIpsecHelper;
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_IPSEC;

    EXPECT_CALL(m_objMockAosIpsecHelper, SetPcscfPortnSpi()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockAosIpsecHelper, IsPcscfServerPortDifferent()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockAosIpsecHelper, MakeSas(_, _, _, _)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED));

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_OK, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_FALSE(bResultOfSA);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, NotifyAkaResponseReturnsSaResultAsTrueWhenSucceedToMakeSas)
{
    m_pAosRegistration->m_pIpsecHelper = &m_objMockAosIpsecHelper;
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_IPSEC;

    EXPECT_CALL(m_objMockAosIpsecHelper, SetPcscfPortnSpi()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockAosIpsecHelper, IsPcscfServerPortDifferent()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockAosIpsecHelper, MakeSas(_, _, _, _)).WillOnce(Return(IMS_TRUE));

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_OK, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_TRUE(bResultOfSA);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, RefreshTimerExpiredIsNotHandledWhenRegistrationIsNull)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosRegistration->m_piRegistration = IMS_NULL;

    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
}

TEST_F(AosRegistrationTest, RefreshTimerExpiredIsNotHandledWhenNotRegisteredState)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
}

TEST_F(AosRegistrationTest, RefreshTimerExpiredReportsFailureWhenSuspended)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegOutOfServicePolicy())
            .WillOnce(Return(CarrierConfig::Assets::REG_OOS_POLICY_DESTROY));
    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, RefreshTimerExpiredStopsRefreshWhenTransactionIsNotStarted)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_bIsTransactionStarted = IMS_FALSE;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegOutOfServicePolicy())
            .WillOnce(Return(CarrierConfig::Assets::REG_OOS_POLICY_DEFAULT));

    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosRegistrationTest, RefreshTimerExpiredStopsRefreshWhenTransactionIsStopped)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_bIsTransactionStarted = IMS_TRUE;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegOutOfServicePolicy())
            .WillOnce(Return(CarrierConfig::Assets::REG_OOS_POLICY_DEFAULT));
    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(_)).WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosRegistrationTest, RefreshTimerExpiredTriggersRegTerminatedWhenFailToCreateIpsec)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_pIpsecHelper = &m_objMockAosIpsecHelper;
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_IPSEC;
    m_pAosRegistration->m_bIsTransactionStarted = IMS_TRUE;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegOutOfServicePolicy())
            .WillOnce(Return(CarrierConfig::Assets::REG_OOS_POLICY_DEFAULT));
    EXPECT_CALL(m_objMockAosIpsecHelper, Create(IMS_FALSE)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED));

    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, RefreshTimerExpiredTriggersRegRefresh)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_bIsTransactionStarted = IMS_TRUE;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegOutOfServicePolicy())
            .WillOnce(Return(CarrierConfig::Assets::REG_OOS_POLICY_DEFAULT));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_UPDATE));

    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_TRUE(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosRegistrationTest, RegistrationStartedWhenRegistrationIsNullDoesNothing)
{
    m_pAosRegistration->m_piRegistration = IMS_NULL;

    EXPECT_CALL(m_objMockIAosPcscf, ResetCurrentPcscfTriedCount()).Times(0);

    m_pAosRegistration->Registration_Started();
}

TEST_F(AosRegistrationTest, RegistrationStartedTriggersReinitiateIfIpsecIsNotEstablished)
{
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_IPSEC;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_pIpsecHelper = &m_objMockAosIpsecHelper;

    EXPECT_CALL(m_objMockIAosPcscf, ResetCurrentPcscfTriedCount()).Times(1);
    EXPECT_CALL(m_objMockAosIpsecHelper, IsEstablished()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockThread, PostMessageI(IsSameMsg(TestAosRegistration::MSG_REG_REINITIATE)));

    m_pAosRegistration->Registration_Started();
}

TEST_F(AosRegistrationTest, RegistrationStartedReportsSuccess)
{
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_IPSEC;
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_pIpsecHelper = &m_objMockAosIpsecHelper;

    EXPECT_CALL(m_objMockIAosPcscf, ResetCurrentPcscfTriedCount()).Times(1);
    EXPECT_CALL(m_objMockAosIpsecHelper, IsEstablished()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockAosIpsecHelper, ProcessRegStarted()).Times(1);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE));

    m_pAosRegistration->Registration_Started();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERED);
}

TEST_F(AosRegistrationTest, RegistrationStartFailedWhenRegistrationIsNullDoesNothing)
{
    m_pAosRegistration->m_piRegistration = IMS_NULL;
    m_pAosRegistration->m_nAuthChallengeCount = 0;

    EXPECT_CALL(m_objMockIRegistration, GetPreviousResponse()).Times(0);

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest,
        RegistrationStartFailedWhenMoreAuthChallengeIsNotAllowedHandlesAuthFailure)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    IMS_UINT32 nCounterThreshold = TestAosRegistration::AUTHENTICATION_RETRY_MAX_COUNT;
    m_pAosRegistration->m_nAuthChallengeCount = nCounterThreshold;

    EXPECT_CALL(m_objMockIRegistration, GetPreviousResponse()).Times(0);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_AUTHENTICATION));

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest, RegistrationStartFailedWithStatusCode)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_nAuthChallengeCount = 0;

    EXPECT_CALL(m_objMockIRegistration, GetPreviousResponse())
            .WillOnce(Return(&m_objMockISipMessage));
    EXPECT_CALL(m_objMockISipMessage, GetStatusCode()).WillOnce(Return(SipStatusCode::SC_403));
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .WillOnce(ReturnRef(objRegErrCodeForPcscfDiscovery));
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(1);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryTimerFPolicy()).Times(0);

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest, RegistrationStartFailedWithTransactionTimeout)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_nAuthChallengeCount = 0;

    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .WillOnce(ReturnRef(objRegErrCodeForPcscfDiscovery));
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(1);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryTimerFPolicy())
            .WillOnce(Return(CarrierConfig::Assets::TIMER_F_POLICY_NONE));

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_TRANSACTION_TIMEOUT);
}

TEST_F(AosRegistrationTest, RegistrationStartFailedWithOtherReason)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_nAuthChallengeCount = 0;

    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_OTHER);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .WillOnce(ReturnRef(objExtraRegErrCode));

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_NONE);
}

TEST_F(AosRegistrationTest, Registration_StartFailed_WfcErrReg403)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;

    m_pAosRegistration->m_nAuthChallengeCount = 0;

    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(SipStatusCode::SC_403));

    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_REG_403))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_NOT_SUPPORTED_COUNTRY))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(_, AosReasonCode::REGISTRATION_ERROR_WFC_REG_403));
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest, Registration_StartFailed_WfcErrReg403_NotSupportErrMessage)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;

    m_pAosRegistration->m_nAuthChallengeCount = 0;

    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(SipStatusCode::SC_403));

    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_REG_403))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_NOT_SUPPORTED_COUNTRY))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(_, AosReasonCode::REGISTRATION_ERROR_WFC_REG_403))
            .Times(0);
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest, Registration_StartFailed_WfcErrReg403_DuplicateReason)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;

    m_pAosRegistration->m_nAuthChallengeCount = 0;

    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(SipStatusCode::SC_403));

    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_REG_403))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_NOT_SUPPORTED_COUNTRY))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    // Set duplicate reason
    m_pAosRegistration->m_eImsReasonCode = AosReasonCode::REGISTRATION_ERROR_WFC_REG_403;

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(_, AosReasonCode::REGISTRATION_ERROR_WFC_REG_403))
            .Times(0);
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest, Registration_StartFailed_WfcErrReg500)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;

    m_pAosRegistration->m_nAuthChallengeCount = 0;

    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(SipStatusCode::SC_500));

    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_500);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_REG_500))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(_, AosReasonCode::REGISTRATION_ERROR_WFC_REG_500));
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest, Registration_StartFailed_WfcErrReg500_NotSupportErrMessage)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;

    m_pAosRegistration->m_nAuthChallengeCount = 0;

    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(SipStatusCode::SC_500));

    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_500);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_REG_500))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(_, AosReasonCode::REGISTRATION_ERROR_WFC_REG_500))
            .Times(0);
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest, Registration_StartFailed_WfcErrReg500_DuplicateReason)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;

    m_pAosRegistration->m_nAuthChallengeCount = 0;

    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(SipStatusCode::SC_500));

    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_500);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_REG_500))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    // Set duplicate reason
    m_pAosRegistration->m_eImsReasonCode = AosReasonCode::REGISTRATION_ERROR_WFC_REG_500;

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(_, AosReasonCode::REGISTRATION_ERROR_WFC_REG_500))
            .Times(0);
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest, Registration_StartFailed_WfcErrOtherFailures)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;

    m_pAosRegistration->m_nAuthChallengeCount = 0;

    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(SipStatusCode::SC_600));

    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_600);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_OTHER_FAILURES))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(_, AosReasonCode::REGISTRATION_ERROR_WFC_OTHER_FAILURES));
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest, Registration_StartFailed_WfcErrOtherFailures_NotSupportErrMessage)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;

    m_pAosRegistration->m_nAuthChallengeCount = 0;

    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(SipStatusCode::SC_600));

    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_600);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_OTHER_FAILURES))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(_, AosReasonCode::REGISTRATION_ERROR_WFC_OTHER_FAILURES))
            .Times(0);
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest, Registration_StartFailed_WfcErrOtherFailures_DuplicateReason)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;

    m_pAosRegistration->m_nAuthChallengeCount = 0;

    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(SipStatusCode::SC_600));

    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_600);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_OTHER_FAILURES))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    // Set duplicate reason
    m_pAosRegistration->m_eImsReasonCode = AosReasonCode::REGISTRATION_ERROR_WFC_OTHER_FAILURES;

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(_, AosReasonCode::REGISTRATION_ERROR_WFC_OTHER_FAILURES))
            .Times(0);
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest, Registration_StartFailed_RegEventChange_UnconditionalDownload)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;

    m_pAosRegistration->m_nAuthChallengeCount = 0;

    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(SipStatusCode::SC_403));

    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetUsatRegEventDownloadPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD));

    ImsList<AString> objImpus;
    EXPECT_CALL(m_objMockIAosService, NotifyRegEventState(SipStatusCode::SC_403, objImpus));
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest, Registration_StartFailed_RegEventChange_NotDownload)
{
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;

    m_pAosRegistration->m_nAuthChallengeCount = 0;

    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(SipStatusCode::SC_403));

    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetUsatRegEventDownloadPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::USAT_REG_EVENT_NOT_DOWNLOAD));

    ImsList<AString> objImpus;
    EXPECT_CALL(m_objMockIAosService, NotifyRegEventState(SipStatusCode::SC_403, objImpus))
            .Times(0);
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest, Registration_Updated)
{
    // m_piRegistration is null
    m_pAosRegistration->Registration_Updated();

    // m_piRegistration is not null
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));

    // IPSec is not supported
    AStringArray objEmptyUris;
    EXPECT_CALL(m_objMockIRegistration, GetAssociatedUris()).WillOnce(ReturnRef(objEmptyUris));
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosRegistration->Registration_Updated();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERED);

    // IPSec is supported - m_pSubscription is not null
    m_pAosRegistration->m_pIpsecHelper = &m_objMockAosIpsecHelper;
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_IPSEC;
    m_pAosRegistration->m_nTxnPending |= TestAosRegistration::PENDING_SUBSCRIPTION;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosRegistration->Registration_Updated();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // IPSec is supported - m_pSubscription is null
    m_pAosRegistration->m_pSubscription = IMS_NULL;
    m_pAosRegistration->Registration_Updated();
}

TEST_F(AosRegistrationTest, Registration_UpdateFailed_CallEndByReregErr)
{
    // BEGIN uninteresting preparation
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));

    EXPECT_CALL(m_objMockIAosPcscf, SetFirstPcscfIndex()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTried()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTriedCount()).Times(AnyNumber());

    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(403));

    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, SetListener(IMS_NULL)).Times(AnyNumber());

    // SetState - for ignoring UpdateDetailState()
    m_pAosRegistration->m_eRegType = AosRegistrationType::EMERGENCY;
    // END uninteresting preparation

    m_pAosRegistration->m_nState = IAosRegistration::STATE_REFRESHING;

    // IsImsCall() GetReregErrCodeForCallEnd()
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    ImsVector<IMS_SINT32> objReregErrCodeForCallEnd;
    objReregErrCodeForCallEnd.Clear();
    objReregErrCodeForCallEnd.Add(403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForCallEnd())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objReregErrCodeForCallEnd));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_REG_TERMINATING));
    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_STATUS_CODE);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, Registration_UpdateFailed_FailOnRoaming)
{
    // BEGIN uninteresting preparation
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));

    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTriedCount()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, SetListener(IMS_NULL)).Times(AnyNumber());

    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(480));
    // END uninteresting preparation

    m_pAosRegistration->m_nState = IAosRegistration::STATE_REFRESHING;
    m_pAosRegistration->m_nImsRegState = TestAosRegistration::IMS_REG_STATE_REGISTERING;
    m_pAosRegistration->m_bEps5GsOnly = IMS_TRUE;

    EXPECT_CALL(m_objMockIAosNetTracker, IsRoaming())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraReregErrInRoamingAsFailureHandled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL))
            .Times(2);

    EXPECT_CALL(m_objMockIAosService, NotifyDeregistered(_, AosReasonCode::PLMN_BLOCK)).Times(2);

    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_STATUS_CODE);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_EQ(m_pAosRegistration->m_nImsRegState, TestAosRegistration::IMS_REG_STATE_DEREGISTERED);

    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_nState = IAosRegistration::STATE_REFRESHING;
    m_pAosRegistration->m_nImsRegState = TestAosRegistration::IMS_REG_STATE_REGISTERING;

    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_TRANSACTION_TIMEOUT);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_EQ(m_pAosRegistration->m_nImsRegState, TestAosRegistration::IMS_REG_STATE_DEREGISTERED);
}

TEST_F(AosRegistrationTest, Registration_UpdateFailed_FailOnRoaming_HeldByCall)
{
    // BEGIN uninteresting preparation
    ImsVector<IMS_SINT32> objReregErrCodeForCallEnd;
    objReregErrCodeForCallEnd.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForCallEnd())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objReregErrCodeForCallEnd));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));

    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTriedCount()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, SetListener(IMS_NULL)).Times(AnyNumber());

    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(480));
    // END uninteresting preparation

    m_pAosRegistration->SetImsCall(IMS_TRUE);
    m_pAosRegistration->m_nState = IAosRegistration::STATE_REFRESHING;
    m_pAosRegistration->m_nImsRegState = TestAosRegistration::IMS_REG_STATE_REGISTERING;
    m_pAosRegistration->m_bEps5GsOnly = IMS_TRUE;

    EXPECT_CALL(m_objMockIAosNetTracker, IsRoaming())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraReregErrInRoamingAsFailureHandled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_STATUS_CODE);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
    EXPECT_TRUE(m_pAosRegistration->IsHeldByCall());
    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(
            TestAosRegistration::PENDING_PLMN_BLOCK_HELD_BY_CALL));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));
    EXPECT_CALL(m_objMockIAosService, NotifyDeregistered(_, AosReasonCode::PLMN_BLOCK));

    m_pAosRegistration->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);
    EXPECT_FALSE(m_pAosRegistration->IsHeldByCall());
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(
            TestAosRegistration::PENDING_PLMN_BLOCK_HELD_BY_CALL));
}

TEST_F(AosRegistrationTest, Registration_UpdateFailed_ProcessUpdateFailed_Others)
{
    // m_nAuthChallengeCount is greater than AUTHENTICATION_RETRY_MAX_COUNT
    // Can not invoke ProcessUpdateFailed_Others
    m_pAosRegistration->m_nAuthChallengeCount = 100;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));
    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_INTERNAL_ERROR);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    m_pAosRegistration->m_nAuthChallengeCount = 0;

    // AWT policy is AWT_POLICY_SPECIFIED_INTERVAL
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::AWT_POLICY_SPECIFIED_INTERVAL))
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_INTERNAL_ERROR);

    // REG_ERROR_CODE_OTHER is in GetReregRetryErrCodeForInitRegWithSamePcscf
    ImsVector<IMS_SINT32> ReregRetryErrCodeForInitRegWithSamePcscf;
    ReregRetryErrCodeForInitRegWithSamePcscf.Add(CarrierConfig::Assets::REG_ERROR_CODE_OTHER);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregRetryErrCodeForInitRegWithSamePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(ReregRetryErrCodeForInitRegWithSamePcscf));
    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_INTERNAL_ERROR);
    ReregRetryErrCodeForInitRegWithSamePcscf.Clear();

    // ProcessAwtRecovery
    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_INTERNAL_ERROR);
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_STOP_RETRY);

    // nReason is REASON_CLIENT_SOCKET_ERROR - PDN reactivation is not required
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_TRANSPORT);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .WillRepeatedly(ReturnRef(objExtraRegErrCode));
    m_pAosRegistration->m_bEps5GsOnly = IMS_FALSE;
    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_CLIENT_SOCKET_ERROR);

    // nReason is REASON_CLIENT_SOCKET_ERROR - PDN reactivation is required
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .Times(AnyNumber())
            .WillOnce(Return(1));
    EXPECT_CALL(
            m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .Times(AnyNumber())
            .WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).Times(AnyNumber()).WillOnce(Return(1));
    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_CLIENT_SOCKET_ERROR);
}

TEST_F(AosRegistrationTest, Registration_Removed)
{
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE));
    m_pAosRegistration->SetState(IAosRegistration::STATE_DEREGISTERING);
    m_pAosRegistration->Registration_Removed();
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, StopTimer)
{
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_OFFLINE_RECOVER, 5000);
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_OFFLINE_RECOVER);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_OFFLINE_RECOVER));

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_STOP_RETRY, 5000);
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_STOP_RETRY);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_REFRESH, 5000);
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_REFRESH);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_REFRESH));

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_EXPIRED, 5000);
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_EXPIRED);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_EXPIRED));

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_MODE, 5000);
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_MODE);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_MODE));

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_TRANSACTION, 5000);
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_TRANSACTION);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_TRANSACTION));

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_INTERNAL_ERROR, 5000);
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_INTERNAL_ERROR);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_INTERNAL_ERROR));
}

TEST_F(AosRegistrationTest, ClearTimer)
{
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_OFFLINE_RECOVER, 5000);
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_STOP_RETRY, 5000);
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_REFRESH, 5000);
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_EXPIRED, 5000);
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_MODE, 5000);
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_TRANSACTION, 5000);
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_INTERNAL_ERROR, 5000);

    m_pAosRegistration->ClearTimers();

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_OFFLINE_RECOVER));
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_REFRESH));
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_EXPIRED));
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_MODE));
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_TRANSACTION));
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_INTERNAL_ERROR));

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_STOP_RETRY, 5000);
    m_pAosRegistration->ClearRetryTimers();
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_OFFLINE_RECOVER));
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, TimerExpired)
{
    m_pAosRegistration->Timer_TimerExpired(IMS_NULL);

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_OFFLINE_RECOVER, 5000);
    m_pAosRegistration->Timer_TimerExpired(m_pAosRegistration->m_piOfflineRecoverTimer);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_OFFLINE_RECOVER));

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_STOP_RETRY, 5000);
    m_pAosRegistration->Timer_TimerExpired(m_pAosRegistration->m_piStopRetryTimer);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_REFRESH, 5000);
    m_pAosRegistration->Timer_TimerExpired(m_pAosRegistration->m_piRefreshTimer);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_REFRESH));

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_EXPIRED, 5000);
    m_pAosRegistration->Timer_TimerExpired(m_pAosRegistration->m_piExpiredTimer);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_EXPIRED));

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_MODE, 5000);
    m_pAosRegistration->Timer_TimerExpired(m_pAosRegistration->m_piModeTimer);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_MODE));

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_TRANSACTION, 5000);
    m_pAosRegistration->Timer_TimerExpired(m_pAosRegistration->m_piTransactionTimer);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_TRANSACTION));

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_INTERNAL_ERROR, 5000);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockIRegistration, RestoreActiveBindings()).Times(1);
    m_pAosRegistration->Timer_TimerExpired(m_pAosRegistration->m_piInternalErrorTimer);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_INTERNAL_ERROR));
}

TEST_F(AosRegistrationTest, ProcessOfflineRecoverTimerExpired)
{
    // Not STATE_OFFLINE
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_OFFLINE_RECOVER, 5000);
    m_pAosRegistration->ProcessOfflineRecoverTimerExpired();
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_OFFLINE_RECOVER));

    // Not AppReady
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pAosRegistration->SetAppReady(IMS_FALSE);
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_OFFLINE_RECOVER, 5000);
    m_pAosRegistration->ProcessOfflineRecoverTimerExpired();
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_OFFLINE_RECOVER));

    // State is STATE_OFFLINE and AppReady
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pAosRegistration->SetAppReady(IMS_TRUE);

    // Transaction is started - fail to CreateRegistration
    m_pAosRegistration->m_bIsTransactionStarted = IMS_TRUE;
    AStringArray objEmptyImpus;
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .Times(AnyNumber())
            .WillOnce(ReturnRef(objEmptyImpus))
            .WillRepeatedly(ReturnRef(m_objImpus));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_OFFLINE_RECOVER, 5000);
    m_pAosRegistration->ProcessOfflineRecoverTimerExpired();
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_OFFLINE_RECOVER));

    // Transaction is started - succeed to CreateRegistration
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_OFFLINE_RECOVER, 5000);
    m_pAosRegistration->ProcessOfflineRecoverTimerExpired();
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_OFFLINE_RECOVER));

    // Transaction is not started
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    m_pAosRegistration->m_bIsTransactionStarted = IMS_FALSE;

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_OFFLINE_RECOVER, 5000);
    m_pAosRegistration->ProcessOfflineRecoverTimerExpired();
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_OFFLINE_RECOVER));
}

TEST_F(AosRegistrationTest, ProcessStopRetryTimerExpired)
{
    // Retry is not held
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_STOP_RETRY, 5000);
    m_pAosRegistration->ProcessStopRetryTimerExpired();
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));

    // Transaction is started - fail to SendRegister
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pAosRegistration->m_bIsTransactionStarted = IMS_TRUE;
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_STOP_RETRY, 5000);
    m_pAosRegistration->ProcessStopRetryTimerExpired();
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));

    // Transaction is started - succeed to SendRegister
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_STOP_RETRY, 5000);
    m_pAosRegistration->ProcessStopRetryTimerExpired();
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));

    // Transaction is not started
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    m_pAosRegistration->m_bIsTransactionStarted = IMS_FALSE;

    m_pAosRegistration->StartTimer(TestAosRegistration::TIMER_STOP_RETRY, 5000);
    m_pAosRegistration->ProcessStopRetryTimerExpired();
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, CreateAndDestroySubscription)
{
    m_pAosRegistration->m_nFeature |= TestAosRegistration::FEATURE_SUBSCRIPTION;
    m_pAosRegistration->m_pSubscription = &m_objMockAosSubscription;

    // StartSubscription - m_pSubscription is not null
    EXPECT_TRUE(m_pAosRegistration->StartSubscription(IMS_TRUE));

    // StopSubscription - m_pSubscription is not null
    EXPECT_TRUE(m_pAosRegistration->StopSubscription());

    // DestroySubscription - m_pSubscription is not null
    EXPECT_TRUE(m_pAosRegistration->DestroySubscription());

    // StartSubscription - m_pSubscription is null
    EXPECT_FALSE(m_pAosRegistration->StartSubscription(IMS_TRUE));

    // StopSubscription - m_pSubscription is null
    EXPECT_FALSE(m_pAosRegistration->StopSubscription());

    // CreateSubscription - m_piRegistration is null
    EXPECT_FALSE(m_pAosRegistration->CreateSubscription());

    // CreateSubscription - piRegSubscription is null
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockIRegistration, CreateSubscription(_))
            .Times(AnyNumber())
            .WillOnce(Return(nullptr))
            .WillRepeatedly(Return(&m_objMockIRegSubscription));
    EXPECT_FALSE(m_pAosRegistration->CreateSubscription());

    // CreateSubscription - m_piRegistration and piRegSubscription is not null
    EXPECT_TRUE(m_pAosRegistration->CreateSubscription());
}

TEST_F(AosRegistrationTest, Subscription_StateChanged)
{
    // m_piRegistration is null
    m_pAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_NONE);

    // ProcessSubscription_Success
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->m_pSubscription = &m_objMockAosSubscription;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION));
    m_pAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_SUB_ESTABLISHED);
    EXPECT_EQ(m_pAosRegistration->m_nConsecutiveFailure, 0);

    // ProcessSubscription_Failed
    m_pAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_SUB_FAILED);

    // MSG_SUB_TERMINATED
    m_pAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_SUB_TERMINATED);

    // reason is REASON_NONE - It doesn't do anything
    m_pAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_NONE);
}

TEST_F(AosRegistrationTest, Subscription_CanBeTransmitted)
{
    // Transaction is not started
    m_pAosRegistration->m_bIsTransactionStarted = IMS_FALSE;
    EXPECT_FALSE(m_pAosRegistration->Subscription_CanBeTransmitted());
    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(TestAosRegistration::PENDING_SUBSCRIPTION));

    // Transaction is started
    m_pAosRegistration->m_bIsTransactionStarted = IMS_TRUE;
    EXPECT_TRUE(m_pAosRegistration->Subscription_CanBeTransmitted());
}

TEST_F(AosRegistrationTest, Subscription_NotifyReceived)
{
    // m_piRegistration is null
    m_pAosRegistration->Subscription_NotifyReceived(AosSubscription::EVENT_UNKNOWN);

    // EVENT_REGISTERED
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    m_pAosRegistration->Subscription_NotifyReceived(AosSubscription::EVENT_REGISTERED);
}

TEST_F(AosRegistrationTest, Subscription_Request)
{
    // CMD_REG_REQUIRED
    m_pAosRegistration->Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 0, IMS_FALSE);

    // CMD_REG_REQUIRED_WITH_SUB_403_MSG
    m_pAosRegistration->Subscription_Request(
            AosSubscription::CMD_REG_REQUIRED_WITH_SUB_403_MSG, 0, IMS_FALSE);

    // CMD_REG_REQUIRED_WITH_NOTIFY_TERMINATED_MSG
    m_pAosRegistration->Subscription_Request(
            AosSubscription::CMD_REG_REQUIRED_WITH_NOTIFY_TERMINATED_MSG, 0, IMS_FALSE);

    // CMD_REG_REQUIRED_WITH_NEXT_PCSCF
    m_pAosRegistration->Subscription_Request(
            AosSubscription::CMD_REG_REQUIRED_WITH_NEXT_PCSCF, 0, IMS_FALSE);

    // CMD_REG_REQUIRED with AWT
    m_pAosRegistration->Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 0, IMS_TRUE);

    // CMD_REG_REQUIRED_WITH_SUB_403_MSG with AWT
    m_pAosRegistration->Subscription_Request(
            AosSubscription::CMD_REG_REQUIRED_WITH_SUB_403_MSG, 0, IMS_TRUE);

    // CMD_REG_REQUIRED_WITH_NOTIFY_TERMINATED_MSG with AWT
    m_pAosRegistration->Subscription_Request(
            AosSubscription::CMD_REG_REQUIRED_WITH_NOTIFY_TERMINATED_MSG, 0, IMS_TRUE);

    // CMD_REG_TERMINATED
    m_pAosRegistration->Subscription_Request(AosSubscription::CMD_REG_TERMINATED, 0, IMS_FALSE);

    // CMD_SUB_REQUIRED
    m_pAosRegistration->Subscription_Request(AosSubscription::CMD_SUB_REQUIRED, 0, IMS_FALSE);

    // CMD_SUB_TERMINATED
    m_pAosRegistration->Subscription_Request(AosSubscription::CMD_SUB_TERMINATED, 0, IMS_FALSE);
}

TEST_F(AosRegistrationTest, NConfiguration_NotifyConfigChanged)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsSubscription())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled())
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    // FEATURE_SUBSCRIPTION is enabled and FEATURE_IPSEC is disabled
    m_pAosRegistration->NConfiguration_NotifyConfigChanged();
    EXPECT_TRUE(m_pAosRegistration->IsFeatureOn(TestAosRegistration::FEATURE_SUBSCRIPTION));
    EXPECT_FALSE(m_pAosRegistration->IsFeatureOn(TestAosRegistration::FEATURE_IPSEC));

    // FEATURE_SUBSCRIPTION is disabled and FEATURE_IPSEC is enabled
    m_pAosRegistration->NConfiguration_NotifyConfigChanged();
    EXPECT_FALSE(m_pAosRegistration->IsFeatureOn(TestAosRegistration::FEATURE_SUBSCRIPTION));
    EXPECT_TRUE(m_pAosRegistration->IsFeatureOn(TestAosRegistration::FEATURE_IPSEC));
}

TEST_F(AosRegistrationTest, Transaction_OnConnectionFailed)
{
    // BEGIN uninteresting preparation
    m_pAosRegistration->m_piRegistration = &m_objMockIRegistration;
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(AnyNumber());
    m_pAosRegistration->SetListener(&m_objMockIAosRegistrationListener);
    // END uninteresting preparation

    // IImsRadio::REASON_ACCESS_DENIED
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    m_pAosRegistration->Transaction_OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 0, 0);
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_OFFLINE_RECOVER));
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_OFFLINE_RECOVER);

    // IImsRadio::REASON_NAS_FAILURE
    m_pAosRegistration->SetRadioWaiting(IMS_TRUE);
    m_pAosRegistration->Transaction_OnConnectionFailed(IImsRadio::REASON_NAS_FAILURE, 0, 15000);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_OFFLINE_RECOVER));
    EXPECT_FALSE(m_pAosRegistration->IsRadioWaiting());

    // IImsRadio::REASON_RRC_REJECT
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pAosRegistration->Transaction_OnConnectionFailed(IImsRadio::REASON_RRC_REJECT, 0, 20000);
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(TestAosRegistration::TIMER_OFFLINE_RECOVER));
    m_pAosRegistration->StopTimer(TestAosRegistration::TIMER_OFFLINE_RECOVER);
}

TEST_F(AosRegistrationTest, MessageMediator_AdjustMessage)
{
    // piSipMsg is null
    EXPECT_EQ(m_pAosRegistration->MessageMediator_AdjustMessage(
                      IMS_NULL, IMessageMediator::MESSAGE_NORMAL),
            IMS_FAILURE);

    // fail to AddLocationHeaderBody
    EXPECT_EQ(m_pAosRegistration->MessageMediator_AdjustMessage(
                      &m_objMockISipMessage, IMessageMediator::MESSAGE_NORMAL),
            IMS_FAILURE);
}

TEST_F(AosRegistrationTest, AddLocationHeaderBody)
{
    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // IsGeolocationInfoRequired return false
    EXPECT_FALSE(m_pAosRegistration->AddLocationHeaderBody(
            &m_objMockISipMessage, IMessageMediator::MESSAGE_NORMAL));

    // IsGeolocationInfoRequired return true
    m_pAosRegistration->m_eRegType = AosRegistrationType::NORMAL;
    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockISipMessage, RemoveBodyParts()).Times(1);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetGeolocationPidfFormingPolicy()).Times(0);
    EXPECT_FALSE(m_pAosRegistration->AddLocationHeaderBody(
            &m_objMockISipMessage, IMessageMediator::MESSAGE_RESUBMIT));
}