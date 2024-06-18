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

#ifndef CARRIER_CONFIG_ITEMS_H_
#define CARRIER_CONFIG_ITEMS_H_

#include "AString.h"
#include "ImsTypeDef.h"
#include "ImsVector.h"

struct CarrierConfigItems
{
public:
    CarrierConfigItems() :
            strConferenceFactoryUri(AString::ConstEmpty()),
            objCallRejectReasonPhrases(ImsVector<AString>()),
            objCallTerminateReasonHeaders(ImsVector<AString>()),
            objCarrierSpecificSipHeaders(ImsVector<AString>()),
            objPidfShortCodes(ImsVector<AString>()),
            objAudioInactivityCallEndReasons(ImsVector<IMS_SINT32>()),
            objCallMaintainingOnRegistrationSuspendeds(ImsVector<IMS_SINT32>()),
            objInformationLevelOfGeolocationPidfs(ImsVector<IMS_SINT32>()),
            objMessageTypesSupportGeolocationPidf(ImsVector<IMS_SINT32>()),
            objRegistrationDisconnectReasonToIgnore(ImsVector<IMS_SINT32>()),
            objRejectCodeForCsfbs(ImsVector<IMS_SINT32>()),
            objRequiringEmergencyCallWhenVideoEmergencyCallFaileds(ImsVector<IMS_SINT32>()),
            objShortCallCodes(ImsVector<IMS_SINT32>()),
            objSrvccTypes(ImsVector<IMS_SINT32>()),
            objSupportGeolocationPidfInSipInvite(ImsVector<IMS_SINT32>()),
            objVilteToVolteRetryFailureResponseCodes(ImsVector<IMS_SINT32>()),
            n18xTimer(32000),
            nCallMaxCount(3),
            nCallRejectCodeForNotAcceptableCallType(488),
            nCallTypeAfterAudioAndVideoCallMerged(1),
            nConferenceDropReferToUriSourceType(0),
            nConferenceInvitingReferType(1),
            nConferenceSipFlowOrder(1),
            nConferenceSubscribeType(1),
            nConvertRemoteResponseTimer(20000),
            nConvertUserResponseTimer(20000),
            nCountryCode(0),
            nDedicatedBearerWaitTimer(8000),
            nEmergency18xTimer(20000),
            nEmergencyRingbackTimer(10000),
            nEmergencyRttGuardTimer(0),
            nEmergencyTCallTimer(10000),
            nEpsFallbackWatchDogTime(-1),
            nIncomingCallRejectCodeForNoAnswer(486),
            nIncomingCallRejectCodeForUserDecline(486),
            nMaximumWaitTimerForGeolocationPidfInfo(0),
            nMediaTypeForOfferlessReinvite(0),
            nMinimumBatteryLevelForLimitVideoCall(0),
            nMoCallRequestTimeout(5000),
            nOipTypeForUnavailable(1),
            nPolicyFor403ResponseForInvite(1),
            nPolicyForAlertNotUsingPreconditionMechanism(0),
            nPolicyForCheckingQosWhileCallUpgrading(0),
            nPolicyForEmergencyUrnEscvMapping(0),
            nPolicyForLocalRingbackToneWith180Response(0),
            nPolicyForMediaTypeRestrictionOnCellular(0),
            nPolicyForMediaTypeRestrictionOnCellularInRoaming(0),
            nPolicyForTcallTimerExpiryOfVolteCall(2),
            nPolicyForTcallTimerExpiryOfVolteEmergencyCall(2),
            nPolicyForTcallTimerExpiryOfVowifiCall(0),
            nPolicyForTextWithVideo(0),
            nPolicyOfLocalNumbers(2),
            nPolicyOnAudioQosDeactivation(0),
            nPolicyOnTextQosDeactivation(2),
            nPolicyOnVideoQosDeactivation(2),
            nPolicyQosPreconditionMechanismWhileCallModification(1),
            nPrackUpdateResponseWaitTimer(3000),
            nPreAlertingTimer(0),
            nRegistrationRestorationModeOn504ForInvite(1),
            nRequestUriType(0),
            nRingbackTimer(90000),
            nRingingTimer(90000),
            nSendUdpKeepAliveIntervalTime(-1),
            nSessionPrivacyType(0),
            nSessionRefreshTriggerInterval(0),
            nSilentRedialInterval(0),
            nSilentRedialMaxRetryCount(0),
            nWifiEmergency18xTimer(0),
            bAddReplaceHeaderForConference(IMS_FALSE),
            bAllowIncomingHoldRequestDuringConferenceCall(IMS_FALSE),
            bAllowMultipleCallIncludingVideoCall(IMS_TRUE),
            bBlockWifiEmergencyCallIfNotProvisioned(IMS_FALSE),
            bCheckAvchangeFeatureForCallConvertingCapability(IMS_FALSE),
            bCheckConferenceEventPackageVersion(IMS_TRUE),
            bCheckServerOutageReasonForVxlteCall(IMS_FALSE),
            bConferenceReferToUriSourcePaid(IMS_TRUE),
            bDefaultEpsBearerContextUsageRestrictionOnCellular(IMS_TRUE),
            bEmergencyCallOverEmergencyPdn(IMS_FALSE),  // wifi
            bEmergencyCallOverEmergencyPdnOnCellular(IMS_TRUE),
            bEmergencyQosPreconditionSupported(IMS_TRUE),
            bEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall(IMS_FALSE),
            bEnableConferenceSubscribeByParticipant(IMS_FALSE),
            bEnableFakeQosCallFlowOnWifi(IMS_FALSE),
            bEnableOipHeaderPolicyFallBack(IMS_FALSE),
            bEnableRegistrationRecoveryWhenCallRejectedByServerError(IMS_FALSE),
            bEnableRegistrationRecoveryWhenCallRetryUnavailable(IMS_FALSE),
            bEnableSendReinviteOnRatChange(IMS_FALSE),
            bIgnore180After183Response(IMS_FALSE),
            bIgnorePrackDeliveryFailure(IMS_FALSE),
            bIncludeCallerIdServiceCodesInSipInvite(IMS_FALSE),
            bInitializePemWhenNoHeader(IMS_FALSE),
            bMaintainMultipleEarlySessionsByForking(IMS_TRUE),
            bMultiendpointSupported(IMS_FALSE),
            bOipSourceFromHeader(IMS_FALSE),
            bPrackSupportedFor18x(IMS_TRUE),
            bRejectOfferlessInvite(IMS_FALSE),
            bRejectVowifiVoiceCallWhenVowifiSettingOff(IMS_FALSE),
            bRetryEmergencyCallOverEmergencyPdnWithNextPcscf(IMS_FALSE),
            bRetryEmergencyOnImsPdnBool(IMS_FALSE),
            bSessionTimerSupported(IMS_TRUE),
            bSetVideoTextFeatureExclusivelyInContactHeaderBySessionType(IMS_FALSE),
            bStopRingbackTimerBy183WithSdpBody(IMS_FALSE),
            bSupportCanidInfo(IMS_FALSE),
            bSupportConferenceReferSubscribe(IMS_TRUE),
            bSupportEarlySession(IMS_FALSE),
            bSupportRegistrationRecoveryForFailureOfSessionRefresh(IMS_FALSE),
            bSupportSipSessionIdHeader(IMS_FALSE),
            bSupportVideoCallOnlyInVopsOffStatus(IMS_FALSE),
            bSupportVideoCallUpgradeRegardlessOfFeatureTags(IMS_FALSE),
            bTextOnDefaultBearerSupported(IMS_FALSE),
            bTextQosPreconditionSupported(IMS_TRUE),
            bUseCarrierSpecificContactHeaderForOptionsResponse(IMS_FALSE),
            bUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration(IMS_FALSE),
            bUseEmergencyNumberTranslationInRoamingStatus(IMS_FALSE),
            bUseLtePreferredStatusForServiceCapability(IMS_FALSE),
            bUseMcidSupplementaryService(IMS_FALSE),
            bUseMmcSupplementaryService(IMS_FALSE),
            bVideoOnDefaultBearerSupported(IMS_FALSE),
            bVideoQosPreconditionSupported(IMS_TRUE),
            bVoiceOnDefaultBearerSupported(IMS_FALSE),
            bVoiceQosPreconditionSupported(IMS_TRUE),
            bReleaseEmergencyPdnWithEmergencyCallFail(IMS_FALSE),
            bRequiredCdmalessFeatureTag(IMS_FALSE),
            bEmergencyCallCurrentLocationDiscoverySupported(IMS_TRUE),
            bCheckUiConditionForIncomingResume(IMS_FALSE)
    {
    }
    ~CarrierConfigItems()
    {
        objCallRejectReasonPhrases.Clear();
        objCallTerminateReasonHeaders.Clear();
        objCarrierSpecificSipHeaders.Clear();
        objPidfShortCodes.Clear();
        objAudioInactivityCallEndReasons.Clear();
        objCallMaintainingOnRegistrationSuspendeds.Clear();
        objInformationLevelOfGeolocationPidfs.Clear();
        objMessageTypesSupportGeolocationPidf.Clear();
        objRegistrationDisconnectReasonToIgnore.Clear();
        objRejectCodeForCsfbs.Clear();
        objRequiringEmergencyCallWhenVideoEmergencyCallFaileds.Clear();
        objShortCallCodes.Clear();
        objSrvccTypes.Clear();
        objSupportGeolocationPidfInSipInvite.Clear();
        objVilteToVolteRetryFailureResponseCodes.Clear();
    }

    CarrierConfigItems(IN const CarrierConfigItems&) = delete;
    CarrierConfigItems& operator=(IN const CarrierConfigItems&) = delete;

public:
    AString strConferenceFactoryUri;

    ImsVector<AString> objCallRejectReasonPhrases;
    ImsVector<AString> objCallTerminateReasonHeaders;
    ImsVector<AString> objCarrierSpecificSipHeaders;
    ImsVector<AString> objPidfShortCodes;

    ImsVector<IMS_SINT32> objAudioInactivityCallEndReasons;
    ImsVector<IMS_SINT32> objCallMaintainingOnRegistrationSuspendeds;  // name?
    ImsVector<IMS_SINT32> objInformationLevelOfGeolocationPidfs;
    ImsVector<IMS_SINT32> objMessageTypesSupportGeolocationPidf;
    ImsVector<IMS_SINT32> objRegistrationDisconnectReasonToIgnore;
    ImsVector<IMS_SINT32> objRejectCodeForCsfbs;
    ImsVector<IMS_SINT32> objRequiringEmergencyCallWhenVideoEmergencyCallFaileds;  // name?
    ImsVector<IMS_SINT32> objShortCallCodes;
    ImsVector<IMS_SINT32> objSrvccTypes;
    ImsVector<IMS_SINT32> objSupportGeolocationPidfInSipInvite;
    ImsVector<IMS_SINT32> objVilteToVolteRetryFailureResponseCodes;

    IMS_SINT32 n18xTimer;
    IMS_SINT32 nCallMaxCount;
    IMS_SINT32 nCallRejectCodeForNotAcceptableCallType;
    IMS_SINT32 nCallTypeAfterAudioAndVideoCallMerged;
    IMS_SINT32 nConferenceDropReferToUriSourceType;
    IMS_SINT32 nConferenceInvitingReferType;
    IMS_SINT32 nConferenceSipFlowOrder;
    IMS_SINT32 nConferenceSubscribeType;
    IMS_SINT32 nConvertRemoteResponseTimer;
    IMS_SINT32 nConvertUserResponseTimer;
    IMS_SINT32 nCountryCode;
    IMS_SINT32 nDedicatedBearerWaitTimer;
    IMS_SINT32 nEmergency18xTimer;
    IMS_SINT32 nEmergencyRingbackTimer;
    IMS_SINT32 nEmergencyRttGuardTimer;
    IMS_SINT32 nEmergencyTCallTimer;
    IMS_SINT32 nEpsFallbackWatchDogTime;
    IMS_SINT32 nIncomingCallRejectCodeForNoAnswer;
    IMS_SINT32 nIncomingCallRejectCodeForUserDecline;
    IMS_SINT32 nMaximumWaitTimerForGeolocationPidfInfo;
    IMS_SINT32 nMediaTypeForOfferlessReinvite;
    IMS_SINT32 nMinimumBatteryLevelForLimitVideoCall;
    IMS_SINT32 nMoCallRequestTimeout;
    IMS_SINT32 nOipTypeForUnavailable;
    IMS_SINT32 nPolicyFor403ResponseForInvite;
    IMS_SINT32 nPolicyForAlertNotUsingPreconditionMechanism;
    IMS_SINT32 nPolicyForCheckingQosWhileCallUpgrading;
    IMS_SINT32 nPolicyForEmergencyUrnEscvMapping;
    IMS_SINT32 nPolicyForLocalRingbackToneWith180Response;
    IMS_SINT32 nPolicyForMediaTypeRestrictionOnCellular;
    IMS_SINT32 nPolicyForMediaTypeRestrictionOnCellularInRoaming;
    IMS_SINT32 nPolicyForTcallTimerExpiryOfVolteCall;
    IMS_SINT32 nPolicyForTcallTimerExpiryOfVolteEmergencyCall;
    IMS_SINT32 nPolicyForTcallTimerExpiryOfVowifiCall;
    IMS_SINT32 nPolicyForTextWithVideo;
    IMS_SINT32 nPolicyOfLocalNumbers;
    IMS_SINT32 nPolicyOnAudioQosDeactivation;
    IMS_SINT32 nPolicyOnTextQosDeactivation;
    IMS_SINT32 nPolicyOnVideoQosDeactivation;
    IMS_SINT32 nPolicyQosPreconditionMechanismWhileCallModification;
    IMS_SINT32 nPrackUpdateResponseWaitTimer;
    IMS_SINT32 nPreAlertingTimer;
    IMS_SINT32 nRegistrationRestorationModeOn504ForInvite;
    IMS_SINT32 nRequestUriType;
    IMS_SINT32 nRingbackTimer;
    IMS_SINT32 nRingingTimer;
    IMS_SINT32 nSendUdpKeepAliveIntervalTime;
    IMS_SINT32 nSessionPrivacyType;
    IMS_SINT32 nSessionRefreshTriggerInterval;
    IMS_SINT32 nSilentRedialInterval;
    IMS_SINT32 nSilentRedialMaxRetryCount;
    IMS_SINT32 nWifiEmergency18xTimer;

    IMS_BOOL bAddReplaceHeaderForConference;
    IMS_BOOL bAllowIncomingHoldRequestDuringConferenceCall;
    IMS_BOOL bAllowMultipleCallIncludingVideoCall;
    IMS_BOOL bBlockWifiEmergencyCallIfNotProvisioned;
    IMS_BOOL bCheckAvchangeFeatureForCallConvertingCapability;
    IMS_BOOL bCheckConferenceEventPackageVersion;
    IMS_BOOL bCheckServerOutageReasonForVxlteCall;
    IMS_BOOL bConferenceReferToUriSourcePaid;
    IMS_BOOL bDefaultEpsBearerContextUsageRestrictionOnCellular;
    IMS_BOOL bEmergencyCallOverEmergencyPdn;
    IMS_BOOL bEmergencyCallOverEmergencyPdnOnCellular;
    IMS_BOOL bEmergencyQosPreconditionSupported;
    IMS_BOOL bEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall;
    IMS_BOOL bEnableConferenceSubscribeByParticipant;
    IMS_BOOL bEnableFakeQosCallFlowOnWifi;
    IMS_BOOL bEnableOipHeaderPolicyFallBack;
    IMS_BOOL bEnableRegistrationRecoveryWhenCallRejectedByServerError;
    IMS_BOOL bEnableRegistrationRecoveryWhenCallRetryUnavailable;
    IMS_BOOL bEnableSendReinviteOnRatChange;
    IMS_BOOL bIgnore180After183Response;
    IMS_BOOL bIgnorePrackDeliveryFailure;
    IMS_BOOL bIncludeCallerIdServiceCodesInSipInvite;
    IMS_BOOL bInitializePemWhenNoHeader;
    IMS_BOOL bMaintainMultipleEarlySessionsByForking;
    IMS_BOOL bMultiendpointSupported;
    IMS_BOOL bOipSourceFromHeader;
    IMS_BOOL bPrackSupportedFor18x;
    IMS_BOOL bRejectOfferlessInvite;
    IMS_BOOL bRejectVowifiVoiceCallWhenVowifiSettingOff;
    IMS_BOOL bRetryEmergencyCallOverEmergencyPdnWithNextPcscf;
    IMS_BOOL bRetryEmergencyOnImsPdnBool;
    IMS_BOOL bSessionTimerSupported;
    IMS_BOOL bSetVideoTextFeatureExclusivelyInContactHeaderBySessionType;
    IMS_BOOL bStopRingbackTimerBy183WithSdpBody;
    IMS_BOOL bSupportCanidInfo;
    IMS_BOOL bSupportConferenceReferSubscribe;
    IMS_BOOL bSupportEarlySession;
    IMS_BOOL bSupportRegistrationRecoveryForFailureOfSessionRefresh;
    IMS_BOOL bSupportSipSessionIdHeader;
    IMS_BOOL bSupportVideoCallOnlyInVopsOffStatus;
    IMS_BOOL bSupportVideoCallUpgradeRegardlessOfFeatureTags;
    IMS_BOOL bTextOnDefaultBearerSupported;
    IMS_BOOL bTextQosPreconditionSupported;
    IMS_BOOL bUseCarrierSpecificContactHeaderForOptionsResponse;
    IMS_BOOL bUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration;
    IMS_BOOL bUseEmergencyNumberTranslationInRoamingStatus;
    IMS_BOOL bUseLtePreferredStatusForServiceCapability;
    IMS_BOOL bUseMcidSupplementaryService;
    IMS_BOOL bUseMmcSupplementaryService;
    IMS_BOOL bVideoOnDefaultBearerSupported;
    IMS_BOOL bVideoQosPreconditionSupported;
    IMS_BOOL bVoiceOnDefaultBearerSupported;
    IMS_BOOL bVoiceQosPreconditionSupported;
    IMS_BOOL bReleaseEmergencyPdnWithEmergencyCallFail;
    IMS_BOOL bRequiredCdmalessFeatureTag;
    IMS_BOOL bEmergencyCallCurrentLocationDiscoverySupported;
    IMS_BOOL bCheckUiConditionForIncomingResume;
};

#endif
