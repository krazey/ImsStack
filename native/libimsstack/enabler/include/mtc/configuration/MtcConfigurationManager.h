#ifndef MTC_CONFIGURATION_MANAGER_H_
#define MTC_CONFIGURATION_MANAGER_H_

#include "IMSTypeDef.h"
#include "AString.h"
#include "configuration/CarrierConfigItems.h"
#include "configuration/AssetItems.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationUpdater.h"
#include "ICarrierConfigListener.h"

class ICarrierConfig;

class MtcConfigurationManager final : public ICarrierConfigListener
{
public:
    MtcConfigurationManager();
    ~MtcConfigurationManager();
    MtcConfigurationManager(IN const MtcConfigurationManager&) = delete;
    MtcConfigurationManager& operator=(IN const MtcConfigurationManager&) = delete;

    void Init();
    void UpdateFullConfig(ICarrierConfig* piCc);

    void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId) override;

    // ims public carrier-configs ==> can be obtained by engine config.
    IMS_SINT32 GetRequestUriType() const;          // KEY_REQUEST_URI_TYPE_INT // tel = 0, sip = 1
    IMS_BOOL IsSupportSipSessionIdHeader() const;  // KEY_SUPPORT_SIP_SESSION_ID_HEADER_BOOL

    // voice configurations
    IMS_BOOL IsIncludeCallerIdServiceCodesInSipInvite()
            const;  // KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL
    IMS_BOOL IsMultiendpointSupported() const;         // KEY_MULTIENDPOINT_SUPPORTED_BOOL
    IMS_BOOL IsSessionTimerSupported() const;          // KEY_SESSION_TIMER_SUPPORTED_BOOL
    IMS_SINT32 GetSessionPrivacyType() const;          // KEY_SESSION_PRIVACY_TYPE_INT
    IMS_BOOL IsPrackSupportedFor18x() const;           // KEY_PRACK_SUPPORTED_FOR_18X_BOOL
    IMS_SINT32 GetConferenceSubscribeType() const;     // KEY_CONFERENCE_SUBSCRIBE_TYPE_INT
    IMS_BOOL IsVoiceQosPreconditionSupported() const;  // KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL
    IMS_BOOL IsVoiceOnDefaultBearerSupported() const;  // KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL
    IMS_SINT32 GetDedicatedBearerWaitTimer() const;    // KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT
    IMS_BOOL IsSrvccType(IN IMS_SINT32 nType) const;   // KEY_SRVCC_TYPE_INT_ARRAY
    IMS_SINT32 GetRingingTimer() const;                // KEY_RINGING_TIMER_MILLIS_INT
    IMS_SINT32 GetRingbackTimer() const;               // KEY_RINGBACK_TIMER_MILLIS_INT
    const AString GetConferenceFactoryUri() const;     // KEY_CONFERENCE_FACTORY_URI_STRING
    IMS_BOOL IsOipSourceFromHeader() const;            // KEY_OIP_SOURCE_FROM_HEADER_BOOL
    IMS_SINT32 GetMoCallRequestTimeout() const;        // KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT
    IMS_SINT32 Get18xTimer() const;
    IMS_BOOL IsSupportConferenceReferSubscribe() const;
    IMS_BOOL IsEnableConferenceSubscribeByParticipant() const;
    IMS_SINT32 GetConferenceSipFlowOrder() const;
    IMS_SINT32 GetConferenceInvitingReferType() const;
    IMS_SINT32 GetPolicyQosPreconditionMechanismWhileCallModification() const;
    IMS_SINT32 GetIncomingCallRejectCodeForUserDecline() const;
    IMS_SINT32 GetIncomingCallRejectCodeForNoAnswer() const;
    IMS_SINT32 GetPrackUpdateResponseWaitTimer() const;
    IMS_SINT32 GetSessionRefreshTriggerInterval() const;
    IMS_SINT32 GetRegistrationRestorationModeOn504ForInvite() const;
    IMS_SINT32 GetPolicyOnAudioQosDeactivation() const;
    IMS_BOOL IsEnableSendReinviteOnRatChange() const;
    IMS_SINT32 GetPolicyForMediaTypeRestrictionOnCellular() const;
    IMS_SINT32 GetPolicyForMediaTypeRestrictionOnCellularInRoaming() const;
    IMS_SINT32 GetPolicyOfLocalNumbers() const;
    IMS_BOOL IsDefaultEpsBearerContextUsageRestrictionOnCellular() const;
    IMS_SINT32 GetSilentRedialInterval() const;
    IMS_SINT32 GetCallTypeAfterAudioAndVideoCallMerged() const;
    IMS_BOOL IsShortCallCode(IN IMS_SINT32 nCode) const;  // int array parsing.
    IMS_BOOL IsValidateVerstatFeatureInRegistrationToCheckNetworkCapability() const;
    IMS_BOOL IsAllowMultipleCallIncludingVideoCall() const;
    IMS_BOOL IsRejectCodeForCsfb(IN IMS_SINT32 nCode) const;  // int array parsing.
    IMS_SINT32 GetSilentRedialMaxRetryCount() const;
    IMS_SINT32 GetPolicyFor403ResponseForInvite() const;
    IMS_SINT32 GetPolicyForCheckingQosWhileCallUpgrading() const;
    IMS_BOOL IsRejectOfferlessInvite() const;
    const AString GetCallTerminateReasonHeader(IN TerminateType eType) const;
    const AString GetCallRejectReasonPhrase(IN RejectType eType) const;

    // vt configurations
    IMS_BOOL IsVideoOnDefaultBearerSupported() const;  // KEY_VIDEO_ON_DEFAULT_BEARER_SUPPORTED_BOOL
    IMS_BOOL IsVideoQosPreconditionSupported() const;  // KEY_VIDEO_QOS_PRECONDITION_SUPPORTED_BOOL
    IMS_SINT32 GetConvertRemoteResponseTimer() const;
    IMS_SINT32 GetConvertUserResponseTimer() const;
    IMS_SINT32 GetPolicyOnVideoQosDeactivation() const;
    IMS_BOOL IsSupportEarlySession() const;
    IMS_BOOL IsAllowTextWithVideo() const;
    IMS_SINT32 GetMinimumBatteryLevelForLimitVideoCall() const;
    IMS_BOOL IsSupportVideoTextFeatureInContactHeaderSimultaneously() const;

    // rtt configurations
    IMS_BOOL IsTextOnDefaultBearerSupported() const;  // KEY_TEXT_ON_DEFAULT_BEARER_SUPPORTED_BOOL
    IMS_BOOL IsTextQosPreconditionSupported() const;  // KEY_TEXT_QOS_PRECONDITION_SUPPORTED_BOOL
    IMS_SINT32 GetPolicyOnTextQosDeactivation() const;

    // wfc configurations
    IMS_BOOL IsPidfShortCode(const AString& strCode) const;  // KEY_PIDF_SHORT_CODE_STRING_ARRAY
    IMS_BOOL IsEmergencyCallOverEmergencyPdn() const;  // KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_BOOL
    IMS_SINT32 GetCountryCode() const;

    // emergency configurations
    IMS_BOOL IsRetryEmergencyOnImsPdnBool() const;
    IMS_BOOL IsEmergencyQosPreconditionSupported()
            const;  // KEY_EMERGENCY_QOS_PRECONDITION_SUPPORTED_BOOL
    IMS_BOOL IsEmergencyCallOverEmergencyPdnOnCellular() const;
    IMS_BOOL IsEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall() const;
    IMS_SINT32 GetEmergencyTCallTimer() const;
    IMS_SINT32 GetEmergencyRingbackTimer() const;
    IMS_SINT32 GetEmergency18xTimer() const;
    IMS_SINT32 GetPolicyForEmergencyUrnEscvMapping() const;

    // asset - separated?
    IMS_BOOL IsCheckConferenceEventPackageVersion() const;
    IMS_BOOL IsConferenceReferToUriSourcePaid() const;
    IMS_SINT32 GetConferenceDropReferToUriSourceType() const;
    IMS_BOOL IsEnableFakeQosCallFlowOnWifi() const;
    IMS_SINT32 GetMediaTypeForOfferlessReinvite() const;
    IMS_BOOL IsSupportVideoCallUpgradeRegardlessOfFeatureTags() const;
    IMS_SINT32 GetOipTypeForUnavailable() const;
    IMS_SINT32 GetEmergencyRttGuardTimer() const;
    IMS_BOOL IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf() const;
    IMS_SINT32 GetPreAlertingTimer() const;
    IMS_SINT32 GetPolicyForTcallTimerExpiryOfVolteCall() const;
    IMS_SINT32 GetPolicyForTcallTimerExpiryOfVolteEmergencyCall() const;
    IMS_SINT32 GetPolicyForTcallTimerExpiryOfVowifiCall() const;
    IMS_BOOL IsCarrierSpecificSipHeader(IN const AString& strHeader) const;
    IMS_BOOL IsCheckAvchangeFeatureForCallConvertingCapability() const;
    IMS_BOOL IsSupportRegistrationRecoveryForFailureOfSessionRefresh() const;
    IMS_BOOL IsCallMaintainingOnRegistrationSuspended(
            IN IMS_SINT32 nSuspendType) const;  // AoSReason?
    IMS_BOOL IsRequiringEmergencyCallWhenVideoEmergencyCallFailed(IN IMS_SINT32 nCode) const;
    IMS_BOOL IsUseMcidSupplementaryService() const;
    IMS_BOOL IsUseMmcSupplementaryService() const;
    IMS_BOOL IsUseLtePreferredStatusForServiceCapability() const;
    IMS_BOOL IsAllowIncomingHoldRequestDuringConferenceCall() const;
    IMS_BOOL IsIgnore180After183Response() const;
    IMS_BOOL IsAddReplaceHeaderForConference() const;
    IMS_BOOL IsVilteToVolteRetryFailureResponseCode(IN IMS_SINT32 nCode) const;
    IMS_BOOL IsUseEmergencyNumberTranslationInRoamingStatus() const;
    IMS_BOOL IsIgnorePrackDeliveryFailure() const;
    IMS_BOOL IsSupportVideoCallOnlyInVopsOffStatus() const;
    IMS_BOOL IsBlockWifiEmergencyCallIfNotProvisioned() const;
    IMS_BOOL IsRegistrationDisconnectReasonToTerminateOngoingCall(
            IN IMS_SINT32 nReason) const;  // AoS
    IMS_SINT32 GetWifiEmergency18xTimer() const;
    IMS_BOOL IsSupportCanidInfo() const;
    IMS_BOOL IsUseCarrierSpecificContactHeaderForOptionsResponse() const;
    IMS_BOOL IsUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration() const;
    IMS_BOOL IsEnableRegistrationRecoveryWhenCallRejectedByServerError() const;
    IMS_BOOL IsEnableRegistrationRecoveryWhenCallRetryUnavailable() const;
    IMS_BOOL IsRejectVowifiVoiceCallWhenVowifiSettingOff() const;
    IMS_BOOL IsCheckServerOutageReasonForVxlteCall() const;
    IMS_BOOL IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType() const;
    IMS_SINT32 GetMaximumWaitTimerForGeolocationPidfInfo() const;
    IMS_BOOL IsMaintainMultipleEarlySessionsByForking() const;
    IMS_BOOL IsStopRingbackTimerBy183WithSdpBody() const;
    IMS_BOOL IsEnableVoiceMailServiceByPaidHeader() const;
    IMS_BOOL IsIgnorePemHeader() const;
    IMS_SINT32 GetInformationLevelOfGeolocationPidf(
            IN IMS_BOOL bEmergency, IN IMS_BOOL bWifi) const;

private:
    CarrierConfigItems m_objCarrierConfig;
    AssetItems m_objAsset;
};

#endif
