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

#ifndef MTC_CONFIGURATION_MANAGER_H_
#define MTC_CONFIGURATION_MANAGER_H_

#include "AString.h"
#include "ICarrierConfigListener.h"
#include "ImsTypeDef.h"
#include "ImsVector.h"
#include "configuration/CarrierConfigItems.h"
#include "configuration/ConfigDef.h"
#include "configuration/IMtcConfigurationManager.h"
#include "configuration/MtcConfigurationUpdater.h"

class ICarrierConfig;

class MtcConfigurationManager final : public IMtcConfigurationManager, public ICarrierConfigListener
{
public:
    MtcConfigurationManager();
    virtual ~MtcConfigurationManager();
    MtcConfigurationManager(IN const MtcConfigurationManager&) = delete;
    MtcConfigurationManager& operator=(IN const MtcConfigurationManager&) = delete;

    void Init() override;
    void UpdateFullConfig(ICarrierConfig* piCc) override;

    void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId) override;

    IMS_SINT32 GetRequestUriType() const override;
    IMS_BOOL IsSupportGeolocationPidfInSipInvite(IN IMS_SINT32 nType) const override;
    IMS_BOOL IsSupportSipSessionIdHeader() const override;

    IMS_BOOL IsIncludeCallerIdServiceCodesInSipInvite() const override;
    IMS_BOOL IsMultiendpointSupported() const override;
    IMS_BOOL IsSessionTimerSupported() const override;
    IMS_SINT32 GetSessionPrivacyType() const override;
    IMS_BOOL IsPrackSupportedFor18x() const override;
    IMS_SINT32 GetConferenceSubscribeType() const override;
    IMS_BOOL IsVoiceQosPreconditionSupported() const override;
    IMS_BOOL IsVoiceOnDefaultBearerSupported() const override;
    IMS_SINT32 GetDedicatedBearerWaitTimer() const override;
    IMS_BOOL IsSrvccType(IN IMS_SINT32 nType) const override;
    IMS_SINT32 GetRingingTimer() const override;
    IMS_SINT32 GetRingbackTimer() const override;
    AString GetConferenceFactoryUri() const override;
    IMS_BOOL IsOipSourceFromHeader() const override;
    IMS_SINT32 GetMoCallRequestTimeout() const override;
    IMS_BOOL IsAudioInactivityCallEndReason(IN IMS_SINT32 nReason) const override;
    IMS_SINT32 Get18xTimer() const override;
    IMS_BOOL IsSupportConferenceReferSubscribe() const override;
    IMS_BOOL IsEnableConferenceSubscribeByParticipant() const override;
    IMS_SINT32 GetConferenceSipFlowOrder() const override;
    IMS_SINT32 GetConferenceInvitingReferType() const override;
    IMS_SINT32 GetPolicyQosPreconditionMechanismWhileCallModification() const override;
    IMS_SINT32 GetIncomingCallRejectCodeForUserDecline() const override;
    IMS_SINT32 GetIncomingCallRejectCodeForNoAnswer() const override;
    IMS_SINT32 GetPrackUpdateResponseWaitTimer() const override;
    IMS_SINT32 GetSessionRefreshTriggerInterval() const override;
    IMS_SINT32 GetRegistrationRestorationModeOn504ForInvite() const override;
    IMS_SINT32 GetPolicyOnAudioQosDeactivation() const override;
    IMS_BOOL IsEnableSendReinviteOnRatChange() const override;
    IMS_SINT32 GetPolicyForMediaTypeRestrictionOnCellular() const override;
    IMS_SINT32 GetPolicyForMediaTypeRestrictionOnCellularInRoaming() const override;
    IMS_SINT32 GetPolicyOfLocalNumbers() const override;
    IMS_BOOL IsDefaultEpsBearerContextUsageRestrictionOnCellular() const override;
    IMS_SINT32 GetSilentRedialInterval() const override;
    IMS_SINT32 GetCallTypeAfterAudioAndVideoCallMerged() const override;
    IMS_BOOL IsShortCallCode(IN IMS_SINT32 nCode) const override;
    IMS_BOOL IsAllowMultipleCallIncludingVideoCall() const override;
    IMS_BOOL IsRejectCodeForCsfb(IN IMS_SINT32 nCode) const override;
    IMS_SINT32 GetSilentRedialMaxRetryCount() const override;
    IMS_SINT32 GetPolicyFor403ResponseForInvite() const override;
    IMS_SINT32 GetPolicyForCheckingQosWhileCallUpgrading() const override;
    IMS_BOOL IsRejectOfferlessInvite() const override;
    IMS_SINT32 GetCallMaxCount() const override;
    AString GetCallTerminateReasonHeader(IN TerminateType eType) const override;
    AString GetCallRejectReasonPhrase(IN RejectType eType) const override;

    IMS_BOOL IsVideoOnDefaultBearerSupported() const override;
    IMS_BOOL IsVideoQosPreconditionSupported() const override;
    IMS_SINT32 GetConvertRemoteResponseTimer() const override;
    IMS_SINT32 GetConvertUserResponseTimer() const override;
    IMS_SINT32 GetPolicyOnVideoQosDeactivation() const override;
    IMS_BOOL IsSupportEarlySession() const override;
    IMS_SINT32 GetPolicyForTextWithVideo() const override;
    IMS_SINT32 GetMinimumBatteryLevelForLimitVideoCall() const override;

    IMS_BOOL IsTextOnDefaultBearerSupported() const override;
    IMS_BOOL IsTextQosPreconditionSupported() const override;
    IMS_SINT32 GetPolicyOnTextQosDeactivation() const override;

    IMS_BOOL IsPidfShortCode(const AString& strCode) const override;
    IMS_BOOL IsEmergencyCallOverEmergencyPdn() const override;
    IMS_SINT32 GetCountryCode() const override;

    IMS_BOOL IsRetryEmergencyOnImsPdnBool() const override;
    IMS_BOOL IsEmergencyQosPreconditionSupported() const override;
    IMS_BOOL IsEmergencyCallOverEmergencyPdnOnCellular() const override;
    IMS_BOOL IsEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall()
            const override;
    IMS_SINT32 GetEmergencyTCallTimer() const override;
    IMS_SINT32 GetEmergencyRingbackTimer() const override;
    IMS_SINT32 GetEmergency18xTimer() const override;
    IMS_SINT32 GetPolicyForEmergencyUrnEscvMapping() const override;

    IMS_BOOL IsCheckConferenceEventPackageVersion() const override;
    IMS_BOOL IsConferenceReferToUriSourcePaid() const override;
    IMS_SINT32 GetConferenceDropReferToUriSourceType() const override;
    IMS_BOOL IsEnableFakeQosCallFlowOnWifi() const override;
    IMS_SINT32 GetMediaTypeForOfferlessReinvite() const override;
    IMS_BOOL IsSupportVideoCallUpgradeRegardlessOfFeatureTags() const override;
    IMS_SINT32 GetOipTypeForUnavailable() const override;
    IMS_BOOL IsEnableOipHeaderPolicyFallBack() const override;
    IMS_SINT32 GetEmergencyRttGuardTimer() const override;
    IMS_BOOL IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf() const override;
    IMS_SINT32 GetPreAlertingTimer() const override;
    IMS_SINT32 GetPolicyForTcallTimerExpiryOfVolteCall() const override;
    IMS_SINT32 GetPolicyForTcallTimerExpiryOfVolteEmergencyCall() const override;
    IMS_SINT32 GetPolicyForTcallTimerExpiryOfVowifiCall() const override;
    IMS_BOOL IsCarrierSpecificSipHeader(IN const AString& strHeader) const override;
    IMS_BOOL IsCheckAvchangeFeatureForCallConvertingCapability() const override;
    IMS_BOOL IsSupportRegistrationRecoveryForFailureOfSessionRefresh() const override;
    IMS_BOOL IsCallMaintainingOnRegistrationSuspended(IN IMS_SINT32 nSuspendType) const override;
    IMS_BOOL IsRequiringEmergencyCallWhenVideoEmergencyCallFailed(
            IN IMS_SINT32 nCode) const override;
    IMS_BOOL IsUseMcidSupplementaryService() const override;
    IMS_BOOL IsUseMmcSupplementaryService() const override;
    IMS_BOOL IsUseLtePreferredStatusForServiceCapability() const override;
    IMS_BOOL IsAllowIncomingHoldRequestDuringConferenceCall() const override;
    IMS_BOOL IsIgnore180After183Response() const override;
    IMS_BOOL IsAddReplaceHeaderForConference() const override;
    IMS_BOOL IsVilteToVolteRetryFailureResponseCode(IN IMS_SINT32 nCode) const override;
    IMS_BOOL IsUseEmergencyNumberTranslationInRoamingStatus() const override;
    IMS_BOOL IsIgnorePrackDeliveryFailure() const override;
    IMS_BOOL IsSupportVideoCallOnlyInVopsOffStatus() const override;
    IMS_BOOL IsBlockWifiEmergencyCallIfNotProvisioned() const override;
    IMS_BOOL IsRegistrationDisconnectReasonToTerminateOngoingCall(
            IN IMS_SINT32 nReason) const override;
    IMS_SINT32 GetWifiEmergency18xTimer() const override;
    IMS_BOOL IsSupportCanidInfo() const override;
    IMS_BOOL IsUseCarrierSpecificContactHeaderForOptionsResponse() const override;
    IMS_BOOL IsUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration() const override;
    IMS_BOOL IsEnableRegistrationRecoveryWhenCallRejectedByServerError() const override;
    IMS_BOOL IsEnableRegistrationRecoveryWhenCallRetryUnavailable() const override;
    IMS_BOOL IsRejectVowifiVoiceCallWhenVowifiSettingOff() const override;
    IMS_BOOL IsCheckServerOutageReasonForVxlteCall() const override;
    IMS_BOOL IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType() const override;
    IMS_SINT32 GetMaximumWaitTimerForGeolocationPidfInfo() const override;
    IMS_BOOL IsMaintainMultipleEarlySessionsByForking() const override;
    IMS_BOOL IsStopRingbackTimerBy183WithSdpBody() const override;
    IMS_SINT32 GetInformationLevelOfGeolocationPidf(
            IN IMS_BOOL bEmergency, IN IMS_BOOL bWifi, IN IMS_BOOL bShortCode) const override;
    IMS_BOOL IsMessageTypeSupportGeolocationPidf(
            IN MessageTypeForGeolocationPidf eType) const override;
    IMS_BOOL IsInitializePemWhenNoHeader() const override;
    IMS_SINT32 GetPolicyForLocalRingbackToneWith180Response() const override;
    IMS_SINT32 GetEpsFallbackWatchdogTime() const override;
    IMS_SINT32 GetSendUdpKeepAliveIntervalTime() const override;
    IMS_SINT32 GetCallRejectCodeForNotAcceptableCallType() const override;
    IMS_BOOL IsReleaseEmergencyPdnWithEmergencyCallFail() const override;
    IMS_SINT32 GetPolicyForAlertNotUsingPreconditionMechanism() const override;
    IMS_BOOL IsRequiredCdmalessFeatureTag() const override;
    IMS_BOOL IsEmergencyCallCurrentLocationDiscoverySupported() const override;
    IMS_BOOL IsCheckUiConditionForIncomingResume() const override;

private:
    static IMS_BOOL ContainsValue(IN const ImsVector<IMS_SINT32>& lstList, IN IMS_SINT32 nValue);
    static IMS_BOOL ContainsValue(IN const ImsVector<AString>& lstList, IN const AString& strValue);

    CarrierConfigItems m_objCarrierConfig;

    LOCAL const IMS_UINT32 TERMINATE_BY_ANY_AOS_REASON = 999;
};

#endif
