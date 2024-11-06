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

#ifndef MOCK_I_MTC_CONFIGURATION_MANAGER_H_
#define MOCK_I_MTC_CONFIGURATION_MANAGER_H_

#include "AString.h"
#include "ImsTypeDef.h"
#include "configuration/IMtcConfigurationManager.h"
#include <gmock/gmock.h>

class ICarrierConfig;

class MockIMtcConfigurationManager : public IMtcConfigurationManager
{
public:
    ~MockIMtcConfigurationManager() {}
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, UpdateFullConfig, (ICarrierConfig* piCc), (override));
    MOCK_METHOD(IMS_SINT32, GetRequestUriType, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSupportGeolocationPidfInSipInvite, (IN IMS_SINT32 nType),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsSupportSipSessionIdHeader, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsIncludeCallerIdServiceCodesInSipInvite, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsMultiendpointSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSessionTimerSupported, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSessionPrivacyType, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsPrackSupportedFor18x, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetConferenceSubscribeType, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVoiceQosPreconditionSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVoiceOnDefaultBearerSupported, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetDedicatedBearerWaitTimer, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSrvccType, (IN IMS_SINT32 nType), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRingingTimer, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRingbackTimer, (), (const, override));
    MOCK_METHOD(AString, GetConferenceFactoryUri, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsOipSourceFromHeader, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetMoCallRequestTimeout, (), (const, override));
    MOCK_METHOD(
            IMS_BOOL, IsAudioInactivityCallEndReason, (IN IMS_SINT32 nReason), (const, override));
    MOCK_METHOD(IMS_SINT32, Get18xTimer, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSupportConferenceReferSubscribe, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEnableConferenceSubscribeByParticipant, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetConferenceSipFlowOrder, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetConferenceInvitingReferType, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPolicyQosPreconditionMechanismWhileCallModification, (),
            (const, override));
    MOCK_METHOD(IMS_SINT32, GetIncomingCallRejectCodeForUserDecline, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetIncomingCallRejectCodeForNoAnswer, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPrackUpdateResponseWaitTimer, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSessionRefreshTriggerInterval, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegistrationRestorationModeOn504ForInvite, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPolicyOnAudioQosDeactivation, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEnableSendReinviteOnRatChange, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPolicyForMediaTypeRestrictionOnCellular, (), (const, override));
    MOCK_METHOD(
            IMS_SINT32, GetPolicyForMediaTypeRestrictionOnCellularInRoaming, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPolicyOfLocalNumbers, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSilentRedialInterval, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetCallTypeAfterAudioAndVideoCallMerged, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsShortCallCode, (IN IMS_SINT32 nCode), (const, override));
    MOCK_METHOD(IMS_BOOL, IsAllowMultipleCallIncludingVideoCall, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRejectCodeForCsfb, (IN IMS_SINT32 nCode), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSilentRedialMaxRetryCount, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPolicyFor403ResponseForInvite, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPolicyForCheckingQosWhileCallUpgrading, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRejectOfferlessInvite, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetCallMaxCount, (), (const, override));
    MOCK_METHOD(AString, GetCallTerminateReasonHeader, (IN TerminateType eType), (const, override));
    MOCK_METHOD(AString, GetCallRejectReasonPhrase, (IN RejectType eType), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVideoOnDefaultBearerSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVideoQosPreconditionSupported, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetConvertRemoteResponseTimer, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetConvertUserResponseTimer, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPolicyOnVideoQosDeactivation, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSupportEarlySession, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPolicyForTextWithVideo, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetMinimumBatteryLevelForLimitVideoCall, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsTextOnDefaultBearerSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsTextQosPreconditionSupported, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPolicyOnTextQosDeactivation, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsPidfShortCode, (IN const AString& strCode), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyCallOverEmergencyPdn, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetCountryCode, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRetryEmergencyOnImsPdnBool, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyQosPreconditionSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyCallOverEmergencyPdnOnCellular, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall,
            (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetEmergencyTCallTimer, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetEmergencyRingbackTimer, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetEmergency18xTimer, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPolicyForEmergencyUrnEscvMapping, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsCheckConferenceEventPackageVersion, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsConferenceReferToUriSourcePaid, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetConferenceDropReferToUriSourceType, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEnableFakeQosCallFlowOnWifi, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetMediaTypeForOfferlessInvite, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetMediaTypeForOfferlessReinvite, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSupportVideoCallUpgradeRegardlessOfFeatureTags, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetOipTypeForUnavailable, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEnableOipHeaderPolicyFallBack, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetDelayUpdateAfterConnectedTimer, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetEmergencyRttGuardTimer, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPreAlertingTimer, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPolicyForTcallTimerExpiryOfVolteCall, (), (const, override));
    MOCK_METHOD(
            IMS_SINT32, GetPolicyForTcallTimerExpiryOfVolteEmergencyCall, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPolicyForTcallTimerExpiryOfVowifiCall, (), (const, override));
    MOCK_METHOD(
            IMS_BOOL, IsCarrierSpecificSipHeader, (IN const AString& strHeader), (const, override));
    MOCK_METHOD(IMS_BOOL, IsCheckAvchangeFeatureForCallConvertingCapability, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSupportRegistrationRecoveryForFailureOfSessionRefresh, (),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsCallMaintainingOnRegistrationSuspended, (IN IMS_SINT32 nSuspendType),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsRequiringEmergencyCallWhenVideoEmergencyCallFailed,
            (IN IMS_SINT32 nCode), (const, override));
    MOCK_METHOD(IMS_BOOL, IsUseMcidSupplementaryService, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsUseMmcSupplementaryService, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsUseLtePreferredStatusForServiceCapability, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsAllowIncomingHoldRequestDuringConferenceCall, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsIgnore180After183Response, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsAddReplaceHeaderForConference, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVilteToVolteRetryFailureResponseCode, (IN IMS_SINT32 nCode),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsUseEmergencyNumberTranslationInRoamingStatus, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsIgnorePrackDeliveryFailure, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSupportVideoCallOnlyInVopsOffStatus, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsBlockWifiEmergencyCallIfNotProvisioned, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRegistrationDisconnectReasonToIgnore, (IN IMS_SINT32 nReason),
            (const, override));
    MOCK_METHOD(IMS_SINT32, GetWifiEmergency18xTimer, (), (const, override));
    MOCK_METHOD(
            IMS_BOOL, IsUseCarrierSpecificContactHeaderForOptionsResponse, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration, (),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsEnableRegistrationRecoveryWhenCallRejectedByServerError, (),
            (const, override));
    MOCK_METHOD(
            IMS_BOOL, IsEnableRegistrationRecoveryWhenCallRetryUnavailable, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRejectVowifiVoiceCallWhenVowifiSettingOff, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsCheckServerOutageReasonForVxlteCall, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType, (),
            (const, override));
    MOCK_METHOD(IMS_SINT32, GetMaximumWaitTimerForGeolocationPidfInfo, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsMaintainMultipleEarlySessionsByForking, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsStopRingbackTimerBy183WithSdpBody, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetInformationLevelOfGeolocationPidf,
            (IN IMS_BOOL bEmergency, IN IMS_BOOL bWifi, IN IMS_BOOL bShortCode), (const, override));
    MOCK_METHOD(IMS_BOOL, IsMessageTypeSupportGeolocationPidf,
            (IN MessageTypeForGeolocationPidf eType), (const, override));
    MOCK_METHOD(IMS_BOOL, IsInitializePemWhenNoHeader, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPolicyForLocalRingbackToneWith180Response, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetEpsFallbackWatchdogTime, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSendUdpKeepAliveIntervalTime, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetCallRejectCodeForNotAcceptableCallType, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsReleaseEmergencyPdnWithEmergencyCallFail, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRequiredCdmalessFeatureTag, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyCallCurrentLocationDiscoverySupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsCheckUiConditionForIncomingResume, (), (const, override));
    MOCK_METHOD(AString, GetPEmergencyInfoHeaderInInvite, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetEmergencyRegistrationTo18xTimer, (IN IMS_BOOL), (const, override));
};

#endif
