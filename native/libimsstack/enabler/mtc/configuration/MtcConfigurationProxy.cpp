#include "configuration/MtcConfigurationProxy.h"
#include "ServiceTrace.h"
#include "configuration/ConfigDef.h"
#include "configuration/ConfigCache.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcConfigurationProxy::MtcConfigurationProxy() :
        m_objManager(MtcConfigurationManager()),
        m_pCache(IMS_NULL)
{
}

PUBLIC
MtcConfigurationProxy::~MtcConfigurationProxy()
{
    if (m_pCache)  // just to skip unnecessary operation in normal case.
    {
        delete m_pCache;
    }
}

PUBLIC
void MtcConfigurationProxy::Init()
{
    IMS_TRACE_I("Init", 0, 0, 0);
    // TODO: this is to adjust when carrier config is ready to be loaded.
    m_objManager.Init();
}

PUBLIC
IMS_BOOL MtcConfigurationProxy::Is(IN Feature eFeature) const
{
    if (m_pCache && m_pCache->HasBooleanCache(eFeature))
    {
        return m_pCache->GetBooleanCache(eFeature);
    }

    switch (eFeature)
    {
        case Feature::SUPPORT_SIP_SESSION_ID_HEADER:
            return m_objManager.IsSupportSipSessionIdHeader();
        case Feature::INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE:
            return m_objManager.IsIncludeCallerIdServiceCodesInSipInvite();
        case Feature::MULTIENDPOINT_SUPPORTED:
            return m_objManager.IsMultiendpointSupported();
        case Feature::SESSION_TIMER_SUPPORTED:
            return m_objManager.IsSessionTimerSupported();
        case Feature::PRACK_SUPPORTED_FOR_18X:
            return m_objManager.IsPrackSupportedFor18x();
        case Feature::VOICE_QOS_PRECONDITION_SUPPORTED:
            return m_objManager.IsVoiceQosPreconditionSupported();
        case Feature::VOICE_ON_DEFAULT_BEARER_SUPPORTED:
            return m_objManager.IsVoiceOnDefaultBearerSupported();
        case Feature::OIP_SOURCE_FROM_HEADER:
            return m_objManager.IsOipSourceFromHeader();
        case Feature::SUPPORT_CONFERENCE_REFER_SUBSCRIBE:
            return m_objManager.IsSupportConferenceReferSubscribe();
        case Feature::ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT:
            return m_objManager.IsEnableConferenceSubscribeByParticipant();
        case Feature::ENABLE_SEND_REINVITE_ON_RAT_CHANGE:
            return m_objManager.IsEnableSendReinviteOnRatChange();
        case Feature::DEFAULT_EPS_BEARER_CONTEXT_USAGE_RESTRICTION_ON_CELLULAR:
            return m_objManager.IsDefaultEpsBearerContextUsageRestrictionOnCellular();
        case Feature::VALIDATE_VERSTAT_FEATURE_IN_REGISTRATION_TO_CHECK_NETWORK_CAPABILITY:
            return m_objManager.IsValidateVerstatFeatureInRegistrationToCheckNetworkCapability();
        case Feature::ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL:
            return m_objManager.IsAllowMultipleCallIncludingVideoCall();
        case Feature::REJECT_OFFERLESS_INVITE:
            return m_objManager.IsRejectOfferlessInvite();
        case Feature::VIDEO_ON_DEFAULT_BEARER_SUPPORTED:
            return m_objManager.IsVideoOnDefaultBearerSupported();
        case Feature::VIDEO_QOS_PRECONDITION_SUPPORTED:
            return m_objManager.IsVideoQosPreconditionSupported();
        case Feature::SUPPORT_EARLY_SESSION:
            return m_objManager.IsSupportEarlySession();
        case Feature::ALLOW_TEXT_WITH_VIDEO:
            return m_objManager.IsAllowTextWithVideo();
        case Feature::SUPPORT_VIDEO_TEXT_FEATURE_IN_CONTACT_HEADER_SIMULTANEOUSLY:
            return m_objManager.IsSupportVideoTextFeatureInContactHeaderSimultaneously();
        case Feature::TEXT_ON_DEFAULT_BEARER_SUPPORTED:
            return m_objManager.IsTextOnDefaultBearerSupported();
        case Feature::TEXT_QOS_PRECONDITION_SUPPORTED:
            return m_objManager.IsTextQosPreconditionSupported();
        case Feature::EMERGENCY_CALL_OVER_EMERGENCY_PDN:
            return m_objManager.IsEmergencyCallOverEmergencyPdn();
        case Feature::RETRY_EMERGENCY_ON_IMS_PDN_BOOL:
            return m_objManager.IsRetryEmergencyOnImsPdnBool();
        case Feature::EMERGENCY_QOS_PRECONDITION_SUPPORTED:
            return m_objManager.IsEmergencyQosPreconditionSupported();
        case Feature::EMERGENCY_CALL_OVER_EMERGENCY_PDN_ON_CELLULAR:
            return m_objManager.IsEmergencyCallOverEmergencyPdnOnCellular();
        case Feature::
                EMERGENCY_RETRY_WITHOUT_CHECKING380_CONTENT_FOR_NON_UE_DETECTABLE_EMERGENCY_CALL:
            return m_objManager
                    .IsEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall();
        case Feature::CHECK_CONFERENCE_EVENT_PACKAGE_VERSION:
            return m_objManager.IsCheckConferenceEventPackageVersion();
        case Feature::CONFERENCE_REFER_TO_URI_SOURCE_PAID:
            return m_objManager.IsConferenceReferToUriSourcePaid();
        case Feature::ENABLE_FAKE_QOS_CALL_FLOW_ON_WIFI:
            return m_objManager.IsEnableFakeQosCallFlowOnWifi();
        case Feature::SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS:
            return m_objManager.IsSupportVideoCallUpgradeRegardlessOfFeatureTags();
        case Feature::RETRY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_WITH_NEXT_PCSCF:
            return m_objManager.IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf();
        case Feature::CHECK_AVCHANGE_FEATURE_FOR_CALL_CONVERTING_CAPABILITY:
            return m_objManager.IsCheckAvchangeFeatureForCallConvertingCapability();
        case Feature::SUPPORT_REGISTRATION_RECOVERY_FOR_FAILURE_OF_SESSION_REFRESH:
            return m_objManager.IsSupportRegistrationRecoveryForFailureOfSessionRefresh();
        case Feature::USE_MCID_SUPPLEMENTARY_SERVICE:
            return m_objManager.IsUseMcidSupplementaryService();
        case Feature::USE_MMC_SUPPLEMENTARY_SERVICE:
            return m_objManager.IsUseMmcSupplementaryService();
        case Feature::USE_LTE_PREFERRED_STATUS_FOR_SERVICE_CAPABILITY:
            return m_objManager.IsUseLtePreferredStatusForServiceCapability();
        case Feature::ALLOW_INCOMING_HOLD_REQUEST_DURING_CONFERENCE_CALL:
            return m_objManager.IsAllowIncomingHoldRequestDuringConferenceCall();
        case Feature::IGNORE_180_AFTER_183_RESPONSE:
            return m_objManager.IsIgnore180After183Response();
        case Feature::ADD_REPLACE_HEADER_FOR_CONFERENCE:
            return m_objManager.IsAddReplaceHeaderForConference();
        case Feature::USE_EMERGENCY_NUMBER_TRANSLATION_IN_ROAMING_STATUS:
            return m_objManager.IsUseEmergencyNumberTranslationInRoamingStatus();
        case Feature::IGNORE_PRACK_DELIVERY_FAILURE:
            return m_objManager.IsIgnorePrackDeliveryFailure();
        case Feature::SUPPORT_VIDEO_CALL_ONLY_IN_VOPS_OFF_STATUS:
            return m_objManager.IsSupportVideoCallOnlyInVopsOffStatus();
        case Feature::BLOCK_WIFI_EMERGENCY_CALL_IF_NOT_PROVISIONED:
            return m_objManager.IsBlockWifiEmergencyCallIfNotProvisioned();
        case Feature::SUPPORT_CANID_INFO:
            return m_objManager.IsSupportCanidInfo();
        case Feature::USE_CARRIER_SPECIFIC_CONTACT_HEADER_FOR_OPTIONS_RESPONSE:
            return m_objManager.IsUseCarrierSpecificContactHeaderForOptionsResponse();
        case Feature::USE_CARRIER_SPECIFIC_REJECT_PHRASE_FOR_INCOMING_CALL_DURING_NO_REGISTRATION:
            return m_objManager
                    .IsUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration();
        case Feature::ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_REJECTED_BY_SERVER_ERROR:
            return m_objManager.IsEnableRegistrationRecoveryWhenCallRejectedByServerError();
        case Feature::ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_RETRY_UNAVAILABLE:
            return m_objManager.IsEnableRegistrationRecoveryWhenCallRetryUnavailable();
        case Feature::REJECT_VOWIFI_VOICE_CALL_WHEN_VOWIFI_SETTING_OFF:
            return m_objManager.IsRejectVowifiVoiceCallWhenVowifiSettingOff();
        case Feature::CHECK_SERVER_OUTAGE_REASON_FOR_VXLTE_CALL:
            return m_objManager.IsCheckServerOutageReasonForVxlteCall();
        case Feature::SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE:
            return m_objManager.IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType();
        case Feature::MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING:
            return m_objManager.IsMaintainMultipleEarlySessionsByForking();
        case Feature::STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY:
            return m_objManager.IsStopRingbackTimerBy183WithSdpBody();
        case Feature::ENABLE_VOICE_MAIL_SERVICE_BY_PAID_HEADER:
            return m_objManager.IsEnableVoiceMailServiceByPaidHeader();
        case Feature::INITIALIZE_PEM_WHEN_NO_HEADER:
            return m_objManager.IsInitializePemWhenNoHeader();
        default:
            IMS_TRACE_E(0, "invalid feature [%d]", eFeature, 0, 0);
            return IMS_FALSE;
    }
}

PUBLIC
IMS_BOOL MtcConfigurationProxy::Is(IN Feature eFeature, IN const AString& strAdditionalInfo) const
{
    switch (eFeature)
    {
        case Feature::PIDF_SHORT_CODE:
            return m_objManager.IsPidfShortCode(strAdditionalInfo);
        case Feature::CARRIER_SPECIFIC_SIP_HEADER:
            return m_objManager.IsCarrierSpecificSipHeader(strAdditionalInfo);
        default:
            IMS_TRACE_E(0, "invalid feature [%d]", eFeature, 0, 0);
            return IMS_FALSE;
    }
}

PUBLIC
IMS_BOOL MtcConfigurationProxy::Is(IN Feature eFeature, IN IMS_SINT32 nAdditionalInfo) const
{
    switch (eFeature)
    {
        case Feature::SUPPORT_GEOLOCATION_PIDF_IN_SIP_INVITE:
            return m_objManager.IsSupportGeolocationPidfInSipInvite(nAdditionalInfo);
        case Feature::SRVCC_TYPE:
            return m_objManager.IsSrvccType(nAdditionalInfo);
        case Feature::SHORT_CALL_CODE:
            return m_objManager.IsShortCallCode(nAdditionalInfo);
        case Feature::REJECT_CODE_FOR_CSFB:
            return m_objManager.IsRejectCodeForCsfb(nAdditionalInfo);
        case Feature::CALL_MAINTAINING_ON_REGISTRATION_SUSPENDED:
            return m_objManager.IsCallMaintainingOnRegistrationSuspended(nAdditionalInfo);
        case Feature::REQUIRING_EMERGENCY_CALL_WHEN_VIDEO_EMERGENCY_CALL_FAILED:
            return m_objManager.IsRequiringEmergencyCallWhenVideoEmergencyCallFailed(
                    nAdditionalInfo);
        case Feature::VILTE_TO_VOLTE_RETRY_FAILURE_RESPONSE_CODE:
            return m_objManager.IsVilteToVolteRetryFailureResponseCode(nAdditionalInfo);
        case Feature::REGISTRATION_DISCONNECT_REASON_TO_TERMINATE_ONGOING_CALL:
            return m_objManager.IsRegistrationDisconnectReasonToTerminateOngoingCall(
                    nAdditionalInfo);
        default:
            IMS_TRACE_E(0, "invalid feature [%d]", eFeature, 0, 0);
            return IMS_FALSE;
    }
}

PUBLIC
IMS_SINT32 MtcConfigurationProxy::GetInt(IN Feature eFeature) const
{
    if (m_pCache && m_pCache->HasIntegerCache(eFeature))
    {
        return m_pCache->GetIntegerCache(eFeature);
    }

    switch (eFeature)
    {
        case Feature::REQUEST_URI_TYPE:
            return m_objManager.GetRequestUriType();
        case Feature::SESSION_PRIVACY_TYPE:
            return m_objManager.GetSessionPrivacyType();
        case Feature::CONFERENCE_SUBSCRIBE_TYPE:
            return m_objManager.GetConferenceSubscribeType();
        case Feature::DEDICATED_BEARER_WAIT_TIMER:
            return m_objManager.GetDedicatedBearerWaitTimer();
        case Feature::RINGING_TIMER:
            return m_objManager.GetRingingTimer();
        case Feature::RINGBACK_TIMER:
            return m_objManager.GetRingbackTimer();
        case Feature::MO_CALL_REQUEST_TIMEOUT:
            return m_objManager.GetMoCallRequestTimeout();
        case Feature::TIMER_18X:
            return m_objManager.Get18xTimer();
        case Feature::CONFERENCE_SIP_FLOW_ORDER:
            return m_objManager.GetConferenceSipFlowOrder();
        case Feature::CONFERENCE_INVITING_REFER_TYPE:
            return m_objManager.GetConferenceInvitingReferType();
        case Feature::POLICY_QOS_PRECONDITION_MECHANISM_WHILE_CALL_MODIFICATION:
            return m_objManager.GetPolicyQosPreconditionMechanismWhileCallModification();
        case Feature::INCOMING_CALL_REJECT_CODE_FOR_USER_DECLINE:
            return m_objManager.GetIncomingCallRejectCodeForUserDecline();
        case Feature::INCOMING_CALL_REJECT_CODE_FOR_NO_ANSWER:
            return m_objManager.GetIncomingCallRejectCodeForNoAnswer();
        case Feature::PRACK_UPDATE_RESPONSE_WAIT_TIMER:
            return m_objManager.GetPrackUpdateResponseWaitTimer();
        case Feature::SESSION_REFRESH_TRIGGER_INTERVAL:
            return m_objManager.GetSessionRefreshTriggerInterval();
        case Feature::REGISTRATION_RESTORATION_MODE_ON_504_FOR_INVITE:
            return m_objManager.GetRegistrationRestorationModeOn504ForInvite();
        case Feature::POLICY_ON_AUDIO_QOS_DEACTIVATION:
            return m_objManager.GetPolicyOnAudioQosDeactivation();
        case Feature::POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR:
            return m_objManager.GetPolicyForMediaTypeRestrictionOnCellular();
        case Feature::POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_IN_ROAMING:
            return m_objManager.GetPolicyForMediaTypeRestrictionOnCellularInRoaming();
        case Feature::POLICY_OF_LOCAL_NUMBERS:
            return m_objManager.GetPolicyOfLocalNumbers();
        case Feature::SILENT_REDIAL_INTERVAL:
            return m_objManager.GetSilentRedialInterval();
        case Feature::CALL_TYPE_AFTER_AUDIO_AND_VIDEO_CALL_MERGED:
            return m_objManager.GetCallTypeAfterAudioAndVideoCallMerged();
        case Feature::SILENT_REDIAL_MAX_RETRY_COUNT:
            return m_objManager.GetSilentRedialMaxRetryCount();
        case Feature::POLICY_FOR_403_RESPONSE_FOR_INVITE:
            return m_objManager.GetPolicyFor403ResponseForInvite();
        case Feature::POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING:
            return m_objManager.GetPolicyForCheckingQosWhileCallUpgrading();
        case Feature::CALL_MAX_COUNT:
            return m_objManager.GetCallMaxCount();
        case Feature::CONVERT_REMOTE_RESPONSE_TIMER:
            return m_objManager.GetConvertRemoteResponseTimer();
        case Feature::CONVERT_USER_RESPONSE_TIMER:
            return m_objManager.GetConvertUserResponseTimer();
        case Feature::POLICY_ON_VIDEO_QOS_DEACTIVATION:
            return m_objManager.GetPolicyOnVideoQosDeactivation();
        case Feature::MINIMUM_BATTERY_LEVEL_FOR_LIMIT_VIDEO_CALL:
            return m_objManager.GetMinimumBatteryLevelForLimitVideoCall();
        case Feature::POLICY_ON_TEXT_QOS_DEACTIVATION:
            return m_objManager.GetPolicyOnTextQosDeactivation();
        case Feature::EMERGENCY_T_CALL_TIMER:
            return m_objManager.GetEmergencyTCallTimer();
        case Feature::EMERGENCY_RINGBACK_TIMER:
            return m_objManager.GetEmergencyRingbackTimer();
        case Feature::EMERGENCY_18X_TIMER:
            return m_objManager.GetEmergency18xTimer();
        case Feature::POLICY_FOR_EMERGENCY_URN_ESCV_MAPPING:
            return m_objManager.GetPolicyForEmergencyUrnEscvMapping();
        case Feature::CONFERENCE_DROP_REFER_TO_URI_SOURCE_TYPE:
            return m_objManager.GetConferenceDropReferToUriSourceType();
        case Feature::MEDIA_TYPE_FOR_OFFERLESS_REINVITE:
            return m_objManager.GetMediaTypeForOfferlessReinvite();
        case Feature::OIP_TYPE_FOR_UNAVAILABLE:
            return m_objManager.GetOipTypeForUnavailable();
        case Feature::EMERGENCY_RTT_GUARD_TIMER:
            return m_objManager.GetEmergencyRttGuardTimer();
        case Feature::PRE_ALERTING_TIMER:
            return m_objManager.GetPreAlertingTimer();
        case Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL:
            return m_objManager.GetPolicyForTcallTimerExpiryOfVolteCall();
        case Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_EMERGENCY_CALL:
            return m_objManager.GetPolicyForTcallTimerExpiryOfVolteEmergencyCall();
        case Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL:
            return m_objManager.GetPolicyForTcallTimerExpiryOfVowifiCall();
        case Feature::WIFI_EMERGENCY_18X_TIMER:
            return m_objManager.GetWifiEmergency18xTimer();
        case Feature::MAXIMUM_WAIT_TIMER_FOR_GEOLOCATION_PIDF_INFO:
            return m_objManager.GetMaximumWaitTimerForGeolocationPidfInfo();
        case Feature::POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE:
            return m_objManager.GetPolicyForLocalRingbackToneWith180Response();
        default:
            IMS_TRACE_E(0, "invalid feature [%d]", eFeature, 0, 0);
            return IMS_FALSE;
    }
}

PUBLIC
IMS_SINT32 MtcConfigurationProxy::GetInt(
        IN Feature eFeature, IN IMS_BOOL bWParam, IN IMS_BOOL bLParam) const
{
    switch (eFeature)
    {
        case Feature::INFORMATION_LEVEL_OF_GEOLOCATION_PIDF:
            return m_objManager.GetInformationLevelOfGeolocationPidf(bWParam, bLParam);
        default:
            IMS_TRACE_E(0, "invalid feature [%d]", eFeature, 0, 0);
            return IMS_FALSE;
    }
}

PUBLIC
const AString MtcConfigurationProxy::GetStr(
        IN Feature eFeature, IN IMS_SINT32 nAdditionalInfo) const
{
    if (m_pCache && m_pCache->HasStringCache(eFeature))
    {
        return m_pCache->GetStringCache(eFeature);
    }

    switch (eFeature)
    {
        case Feature::CONFERENCE_FACTORY_URI:
            return m_objManager.GetConferenceFactoryUri();
        case Feature::CALL_TERMINATE_REASON_HEADER:
            return m_objManager.GetCallTerminateReasonHeader(
                    static_cast<TerminateType>(nAdditionalInfo));
        case Feature::CALL_REJECT_REASON_PHRASE:
            return m_objManager.GetCallRejectReasonPhrase(static_cast<RejectType>(nAdditionalInfo));
        default:
            IMS_TRACE_E(0, "invalid feature [%d]", eFeature, 0, 0);
            return AString::ConstNull();
    }
}

PUBLIC
void MtcConfigurationProxy::PutConfigCache(IN Feature eFeature, IN IMS_SINT32 nValue)
{
    if (m_pCache == IMS_NULL)
    {
        m_pCache = new ConfigCache();
    }
    m_pCache->PutCache(eFeature, nValue);
}

PUBLIC
void MtcConfigurationProxy::PutConfigCache(IN Feature eFeature, IN IMS_BOOL bValue)
{
    if (m_pCache == IMS_NULL)
    {
        m_pCache = new ConfigCache();
    }
    m_pCache->PutCache(eFeature, bValue);
}

PUBLIC
void MtcConfigurationProxy::PutConfigCache(IN Feature eFeature, const AString& strValue)
{
    if (m_pCache == IMS_NULL)
    {
        m_pCache = new ConfigCache();
    }
    m_pCache->PutCache(eFeature, strValue);
}

PUBLIC
void MtcConfigurationProxy::OnRegistrationRefreshed()
{
    if (m_pCache)
    {
        IMS_TRACE_I("OnRegistrationRefreshed", 0, 0, 0);
        // TODO: if there is a cached config of which life cycle is different than registration...
        delete m_pCache;
        m_pCache = IMS_NULL;
    }
}
