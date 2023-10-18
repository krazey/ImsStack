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

#ifndef INTERFACE_MTC_CONFIGURATION_MANAGER_H_
#define INTERFACE_MTC_CONFIGURATION_MANAGER_H_

#include "AString.h"
#include "ConfigDef.h"
#include "ImsTypeDef.h"

class ICarrierConfig;

class IMtcConfigurationManager
{
public:
    virtual ~IMtcConfigurationManager() {}

    /**
     * @brief Inits
     *
     */
    virtual void Init() = 0;

    /**
     * @brief Updates
     *
     */
    virtual void UpdateFullConfig(ICarrierConfig* piCc) = 0;

    // ims public carrier-configs ==> can be obtained by engine config.

    /**
     * @brief Gets
     *
     * @param KEY_REQUEST_URI_TYPE_INT
     * @param tel
     * @param 1
     * @return
     */
    virtual IMS_SINT32 GetRequestUriType()
            const = 0;  // KEY_REQUEST_URI_TYPE_INT // tel = 0, sip = 1

    /**
     * @brief Checks
     *
     * @param nType
     * @return
     */
    virtual IMS_BOOL IsSupportGeolocationPidfInSipInvite(IN IMS_SINT32 nType) const = 0;

    /**
     * @brief Checks
     *
     * @param KEY_SUPPORT_SIP_SESSION_ID_HEADER_BOOL
     * @return
     */
    virtual IMS_BOOL IsSupportSipSessionIdHeader()
            const = 0;  // KEY_SUPPORT_SIP_SESSION_ID_HEADER_BOOL

    // voice configurations

    /**
     * @brief Checks
     *
     * @param KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL
     * @return
     */
    virtual IMS_BOOL IsIncludeCallerIdServiceCodesInSipInvite()
            const = 0;  // KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL

    /**
     * @brief Checks
     *
     * @param KEY_MULTIENDPOINT_SUPPORTED_BOOL
     * @return
     */
    virtual IMS_BOOL IsMultiendpointSupported() const = 0;      // KEY_MULTIENDPOINT_SUPPORTED_BOOL

    /**
     * @brief Checks
     *
     * @param KEY_SESSION_TIMER_SUPPORTED_BOOL
     * @return
     */
    virtual IMS_BOOL IsSessionTimerSupported() const = 0;       // KEY_SESSION_TIMER_SUPPORTED_BOOL

    /**
     * @brief Gets
     *
     * @param KEY_SESSION_PRIVACY_TYPE_INT
     * @return
     */
    virtual IMS_SINT32 GetSessionPrivacyType() const = 0;       // KEY_SESSION_PRIVACY_TYPE_INT

    /**
     * @brief Checks
     *
     * @param KEY_PRACK_SUPPORTED_FOR_18X_BOOL
     * @return
     */
    virtual IMS_BOOL IsPrackSupportedFor18x() const = 0;        // KEY_PRACK_SUPPORTED_FOR_18X_BOOL

    /**
     * @brief Gets
     *
     * @param KEY_CONFERENCE_SUBSCRIBE_TYPE_INT
     * @return
     */
    virtual IMS_SINT32 GetConferenceSubscribeType() const = 0;  // KEY_CONFERENCE_SUBSCRIBE_TYPE_INT

    /**
     * @brief Checks
     *
     * @param KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL
     * @return
     */
    virtual IMS_BOOL IsVoiceQosPreconditionSupported()
            const = 0;  // KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL

    /**
     * @brief Checks
     *
     * @param KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL
     * @return
     */
    virtual IMS_BOOL IsVoiceOnDefaultBearerSupported()
            const = 0;  // KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL

    /**
     * @brief Gets
     *
     * @param KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT
     * @return
     */
    virtual IMS_SINT32 GetDedicatedBearerWaitTimer()
            const = 0;  // KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT

    /**
     * @brief Checks
     *
     * @param nType
     * @param KEY_SRVCC_TYPE_INT_ARRAY
     * @return
     */
    virtual IMS_BOOL IsSrvccType(IN IMS_SINT32 nType) const = 0;  // KEY_SRVCC_TYPE_INT_ARRAY

    /**
     * @brief Gets
     *
     * @param KEY_RINGING_TIMER_MILLIS_INT
     * @return
     */
    virtual IMS_SINT32 GetRingingTimer() const = 0;               // KEY_RINGING_TIMER_MILLIS_INT

    /**
     * @brief Gets
     *
     * @param KEY_RINGBACK_TIMER_MILLIS_INT
     * @return
     */
    virtual IMS_SINT32 GetRingbackTimer() const = 0;              // KEY_RINGBACK_TIMER_MILLIS_INT

    /**
     * @brief Gets
     *
     * @param KEY_CONFERENCE_FACTORY_URI_STRING
     * @return
     */
    virtual AString GetConferenceFactoryUri() const = 0;  // KEY_CONFERENCE_FACTORY_URI_STRING

    /**
     * @brief Checks
     *
     * @param KEY_OIP_SOURCE_FROM_HEADER_BOOL
     * @return
     */
    virtual IMS_BOOL IsOipSourceFromHeader() const = 0;   // KEY_OIP_SOURCE_FROM_HEADER_BOOL

    /**
     * @brief Gets
     *
     * @param KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT
     * @return
     */
    virtual IMS_SINT32 GetMoCallRequestTimeout()
            const = 0;  // KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT

    /**
     * @brief Checks
     *
     * @param nReason
     * @param KEY_AUDIO_INACTIVITY_CALL_END_REASONS_INT_ARRAY
     * @return
     */
    virtual IMS_BOOL IsAudioInactivityCallEndReason(
            IN IMS_SINT32 nReason) const = 0;  // KEY_AUDIO_INACTIVITY_CALL_END_REASONS_INT_ARRAY

    /**
     * @brief Get18xs
     *
     * @return
     */
    virtual IMS_SINT32 Get18xTimer() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsSupportConferenceReferSubscribe() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsEnableConferenceSubscribeByParticipant() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetConferenceSipFlowOrder() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetConferenceInvitingReferType() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyQosPreconditionMechanismWhileCallModification() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetIncomingCallRejectCodeForUserDecline() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetIncomingCallRejectCodeForNoAnswer() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPrackUpdateResponseWaitTimer() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetSessionRefreshTriggerInterval() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetRegistrationRestorationModeOn504ForInvite() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyOnAudioQosDeactivation() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsEnableSendReinviteOnRatChange() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyForMediaTypeRestrictionOnCellular() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyForMediaTypeRestrictionOnCellularInRoaming() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyOfLocalNumbers() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsDefaultEpsBearerContextUsageRestrictionOnCellular() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetSilentRedialInterval() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetCallTypeAfterAudioAndVideoCallMerged() const = 0;

    /**
     * @brief Checks
     *
     * @param nCode
     * @param int
     * @param parsing.
     * @return
     */
    virtual IMS_BOOL IsShortCallCode(IN IMS_SINT32 nCode) const = 0;  // int array parsing.

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsAllowMultipleCallIncludingVideoCall() const = 0;

    /**
     * @brief Checks
     *
     * @param nCode
     * @param int
     * @param parsing.
     * @return
     */
    virtual IMS_BOOL IsRejectCodeForCsfb(IN IMS_SINT32 nCode) const = 0;  // int array parsing.

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetSilentRedialMaxRetryCount() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyFor403ResponseForInvite() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyForCheckingQosWhileCallUpgrading() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsRejectOfferlessInvite() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetCallMaxCount() const = 0;

    /**
     * @brief Gets
     *
     * @param eType
     * @return
     */
    virtual AString GetCallTerminateReasonHeader(IN TerminateType eType) const = 0;

    /**
     * @brief Gets
     *
     * @param eType
     * @return
     */
    virtual AString GetCallRejectReasonPhrase(IN RejectType eType) const = 0;

    // vt configurations

    /**
     * @brief Checks
     *
     * @param KEY_VIDEO_ON_DEFAULT_BEARER_SUPPORTED_BOOL
     * @return
     */
    virtual IMS_BOOL IsVideoOnDefaultBearerSupported()
            const = 0;  // KEY_VIDEO_ON_DEFAULT_BEARER_SUPPORTED_BOOL

    /**
     * @brief Checks
     *
     * @param KEY_VIDEO_QOS_PRECONDITION_SUPPORTED_BOOL
     * @return
     */
    virtual IMS_BOOL IsVideoQosPreconditionSupported()
            const = 0;  // KEY_VIDEO_QOS_PRECONDITION_SUPPORTED_BOOL

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetConvertRemoteResponseTimer() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetConvertUserResponseTimer() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyOnVideoQosDeactivation() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsSupportEarlySession() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyForTextWithVideo() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetMinimumBatteryLevelForLimitVideoCall() const = 0;

    // rtt configurations

    /**
     * @brief Checks
     *
     * @param KEY_TEXT_ON_DEFAULT_BEARER_SUPPORTED_BOOL
     * @return
     */
    virtual IMS_BOOL IsTextOnDefaultBearerSupported()
            const = 0;  // KEY_TEXT_ON_DEFAULT_BEARER_SUPPORTED_BOOL

    /**
     * @brief Checks
     *
     * @param KEY_TEXT_QOS_PRECONDITION_SUPPORTED_BOOL
     * @return
     */
    virtual IMS_BOOL IsTextQosPreconditionSupported()
            const = 0;  // KEY_TEXT_QOS_PRECONDITION_SUPPORTED_BOOL

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyOnTextQosDeactivation() const = 0;

    // wfc configurations

    /**
     * @brief Checks
     *
     * @param strCode
     * @param KEY_PIDF_SHORT_CODE_STRING_ARRAY
     * @return
     */
    virtual IMS_BOOL IsPidfShortCode(
            IN const AString& strCode) const = 0;  // KEY_PIDF_SHORT_CODE_STRING_ARRAY

    /**
     * @brief Checks
     *
     * @param KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_BOOL
     * @return
     */
    virtual IMS_BOOL IsEmergencyCallOverEmergencyPdn()
            const = 0;  // KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_BOOL

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetCountryCode() const = 0;

    // emergency configurations

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsRetryEmergencyOnImsPdnBool() const = 0;

    /**
     * @brief Checks
     *
     * @param KEY_EMERGENCY_QOS_PRECONDITION_SUPPORTED_BOOL
     * @return
     */
    virtual IMS_BOOL IsEmergencyQosPreconditionSupported()
            const = 0;  // KEY_EMERGENCY_QOS_PRECONDITION_SUPPORTED_BOOL

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsEmergencyCallOverEmergencyPdnOnCellular() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall()
            const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetEmergencyTCallTimer() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetEmergencyRingbackTimer() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetEmergency18xTimer() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyForEmergencyUrnEscvMapping() const = 0;

    // asset - separated?

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsCheckConferenceEventPackageVersion() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsConferenceReferToUriSourcePaid() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetConferenceDropReferToUriSourceType() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsEnableFakeQosCallFlowOnWifi() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetMediaTypeForOfferlessReinvite() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsSupportVideoCallUpgradeRegardlessOfFeatureTags() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetOipTypeForUnavailable() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsEnableOipHeaderPolicyFallBack() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetEmergencyRttGuardTimer() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPreAlertingTimer() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyForTcallTimerExpiryOfVolteCall() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyForTcallTimerExpiryOfVolteEmergencyCall() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyForTcallTimerExpiryOfVowifiCall() const = 0;

    /**
     * @brief Checks
     *
     * @param strHeader
     * @return
     */
    virtual IMS_BOOL IsCarrierSpecificSipHeader(IN const AString& strHeader) const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsCheckAvchangeFeatureForCallConvertingCapability() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsSupportRegistrationRecoveryForFailureOfSessionRefresh() const = 0;

    /**
     * @brief Checks
     *
     * @param nSuspendType
     * @param AoSReason?
     * @return
     */
    virtual IMS_BOOL IsCallMaintainingOnRegistrationSuspended(
            IN IMS_SINT32 nSuspendType) const = 0;  // AoSReason?

    /**
     * @brief Checks
     *
     * @param nCode
     * @return
     */
    virtual IMS_BOOL IsRequiringEmergencyCallWhenVideoEmergencyCallFailed(
            IN IMS_SINT32 nCode) const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsUseMcidSupplementaryService() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsUseMmcSupplementaryService() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsUseLtePreferredStatusForServiceCapability() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsAllowIncomingHoldRequestDuringConferenceCall() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsIgnore180After183Response() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsAddReplaceHeaderForConference() const = 0;

    /**
     * @brief Checks
     *
     * @param nCode
     * @return
     */
    virtual IMS_BOOL IsVilteToVolteRetryFailureResponseCode(IN IMS_SINT32 nCode) const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsUseEmergencyNumberTranslationInRoamingStatus() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsIgnorePrackDeliveryFailure() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsSupportVideoCallOnlyInVopsOffStatus() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsBlockWifiEmergencyCallIfNotProvisioned() const = 0;

    /**
     * @brief Checks
     *
     * @param nReason
     * @param AoS
     * @return
     */
    virtual IMS_BOOL IsRegistrationDisconnectReasonToTerminateOngoingCall(
            IN IMS_SINT32 nReason) const = 0;  // AoS

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetWifiEmergency18xTimer() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsSupportCanidInfo() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsUseCarrierSpecificContactHeaderForOptionsResponse() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration()
            const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsEnableRegistrationRecoveryWhenCallRejectedByServerError() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsEnableRegistrationRecoveryWhenCallRetryUnavailable() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsRejectVowifiVoiceCallWhenVowifiSettingOff() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsCheckServerOutageReasonForVxlteCall() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetMaximumWaitTimerForGeolocationPidfInfo() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsMaintainMultipleEarlySessionsByForking() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsStopRingbackTimerBy183WithSdpBody() const = 0;

    /**
     * @brief Gets
     *
     * @param bEmergency
     * @param bWifi
     * @return
     */
    virtual IMS_SINT32 GetInformationLevelOfGeolocationPidf(
            IN IMS_BOOL bEmergency, IN IMS_BOOL bWifi, IN IMS_BOOL bShortCode) const = 0;

    /**
     * @brief Checks
     *
     * @param eType
     * @return
     */
    virtual IMS_BOOL IsMessageTypeSupportGeolocationPidf(
            IN MessageTypeForGeolocationPidf eType) const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsInitializePemWhenNoHeader() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyForLocalRingbackToneWith180Response() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetEpsFallbackWatchdogTime() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetSendUdpKeepAliveIntervalTime() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetCallRejectCodeForNotAcceptableCallType() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_BOOL IsReleaseEmergencyPdnWithEmergencyCallFail() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetPolicyForAlertNotUsingPreconditionMechanism() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsRequiredCdmalessFeatureTag() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsEmergencyCallCurrentLocationDiscoverySupported() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsCheckUiConditionForIncomingResume() const = 0;
};

#endif
