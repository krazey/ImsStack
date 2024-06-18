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

#include "ServiceTrace.h"
#include "configuration/ConfigCache.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcConfigurationProxy::MtcConfigurationProxy(IN IMtcConfigurationManager* pManager) :
        m_pManager(pManager),
        m_pCache(IMS_NULL)
{
    IMS_ASSERT(pManager != IMS_NULL);
}

PUBLIC
MtcConfigurationProxy::~MtcConfigurationProxy()
{
    delete m_pCache;
}

PUBLIC
void MtcConfigurationProxy::Init()
{
    IMS_TRACE_I("Init", 0, 0, 0);
    // TODO: this is to adjust when carrier config is ready to be loaded.
    m_pManager->Init();
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
            return m_pManager->IsSupportSipSessionIdHeader();
        case Feature::INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE:
            return m_pManager->IsIncludeCallerIdServiceCodesInSipInvite();
        case Feature::MULTIENDPOINT_SUPPORTED:
            return m_pManager->IsMultiendpointSupported();
        case Feature::SESSION_TIMER_SUPPORTED:
            return m_pManager->IsSessionTimerSupported();
        case Feature::PRACK_SUPPORTED_FOR_18X:
            return m_pManager->IsPrackSupportedFor18x();
        case Feature::VOICE_QOS_PRECONDITION_SUPPORTED:
            return m_pManager->IsVoiceQosPreconditionSupported();
        case Feature::VOICE_ON_DEFAULT_BEARER_SUPPORTED:
            return m_pManager->IsVoiceOnDefaultBearerSupported();
        case Feature::OIP_SOURCE_FROM_HEADER:
            return m_pManager->IsOipSourceFromHeader();
        case Feature::SUPPORT_CONFERENCE_REFER_SUBSCRIBE:
            return m_pManager->IsSupportConferenceReferSubscribe();
        case Feature::ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT:
            return m_pManager->IsEnableConferenceSubscribeByParticipant();
        case Feature::ENABLE_SEND_REINVITE_ON_RAT_CHANGE:
            return m_pManager->IsEnableSendReinviteOnRatChange();
        case Feature::DEFAULT_EPS_BEARER_CONTEXT_USAGE_RESTRICTION_ON_CELLULAR:
            return m_pManager->IsDefaultEpsBearerContextUsageRestrictionOnCellular();
        case Feature::ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL:
            return m_pManager->IsAllowMultipleCallIncludingVideoCall();
        case Feature::REJECT_OFFERLESS_INVITE:
            return m_pManager->IsRejectOfferlessInvite();
        case Feature::VIDEO_ON_DEFAULT_BEARER_SUPPORTED:
            return m_pManager->IsVideoOnDefaultBearerSupported();
        case Feature::VIDEO_QOS_PRECONDITION_SUPPORTED:
            return m_pManager->IsVideoQosPreconditionSupported();
        case Feature::SUPPORT_EARLY_SESSION:
            return m_pManager->IsSupportEarlySession();
        case Feature::TEXT_ON_DEFAULT_BEARER_SUPPORTED:
            return m_pManager->IsTextOnDefaultBearerSupported();
        case Feature::TEXT_QOS_PRECONDITION_SUPPORTED:
            return m_pManager->IsTextQosPreconditionSupported();
        case Feature::EMERGENCY_CALL_OVER_EMERGENCY_PDN:
            return m_pManager->IsEmergencyCallOverEmergencyPdn();
        case Feature::RETRY_EMERGENCY_ON_IMS_PDN_BOOL:
            return m_pManager->IsRetryEmergencyOnImsPdnBool();
        case Feature::EMERGENCY_QOS_PRECONDITION_SUPPORTED:
            return m_pManager->IsEmergencyQosPreconditionSupported();
        case Feature::EMERGENCY_CALL_OVER_EMERGENCY_PDN_ON_CELLULAR:
            return m_pManager->IsEmergencyCallOverEmergencyPdnOnCellular();
        case Feature::
                EMERGENCY_RETRY_WITHOUT_CHECKING380_CONTENT_FOR_NON_UE_DETECTABLE_EMERGENCY_CALL:
            return m_pManager
                    ->IsEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall();
        case Feature::CHECK_CONFERENCE_EVENT_PACKAGE_VERSION:
            return m_pManager->IsCheckConferenceEventPackageVersion();
        case Feature::CONFERENCE_REFER_TO_URI_SOURCE_PAID:
            return m_pManager->IsConferenceReferToUriSourcePaid();
        case Feature::ENABLE_FAKE_QOS_CALL_FLOW_ON_WIFI:
            return m_pManager->IsEnableFakeQosCallFlowOnWifi();
        case Feature::SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS:
            return m_pManager->IsSupportVideoCallUpgradeRegardlessOfFeatureTags();
        case Feature::ENABLE_OIP_HEADER_POLICY_FALLBACK:
            return m_pManager->IsEnableOipHeaderPolicyFallBack();
        case Feature::RETRY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_WITH_NEXT_PCSCF:
            return m_pManager->IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf();
        case Feature::CHECK_AVCHANGE_FEATURE_FOR_CALL_CONVERTING_CAPABILITY:
            return m_pManager->IsCheckAvchangeFeatureForCallConvertingCapability();
        case Feature::SUPPORT_REGISTRATION_RECOVERY_FOR_FAILURE_OF_SESSION_REFRESH:
            return m_pManager->IsSupportRegistrationRecoveryForFailureOfSessionRefresh();
        case Feature::USE_MCID_SUPPLEMENTARY_SERVICE:
            return m_pManager->IsUseMcidSupplementaryService();
        case Feature::USE_MMC_SUPPLEMENTARY_SERVICE:
            return m_pManager->IsUseMmcSupplementaryService();
        case Feature::USE_LTE_PREFERRED_STATUS_FOR_SERVICE_CAPABILITY:
            return m_pManager->IsUseLtePreferredStatusForServiceCapability();
        case Feature::ALLOW_INCOMING_HOLD_REQUEST_DURING_CONFERENCE_CALL:
            return m_pManager->IsAllowIncomingHoldRequestDuringConferenceCall();
        case Feature::IGNORE_180_AFTER_183_RESPONSE:
            return m_pManager->IsIgnore180After183Response();
        case Feature::ADD_REPLACE_HEADER_FOR_CONFERENCE:
            return m_pManager->IsAddReplaceHeaderForConference();
        case Feature::USE_EMERGENCY_NUMBER_TRANSLATION_IN_ROAMING_STATUS:
            return m_pManager->IsUseEmergencyNumberTranslationInRoamingStatus();
        case Feature::IGNORE_PRACK_DELIVERY_FAILURE:
            return m_pManager->IsIgnorePrackDeliveryFailure();
        case Feature::SUPPORT_VIDEO_CALL_ONLY_IN_VOPS_OFF_STATUS:
            return m_pManager->IsSupportVideoCallOnlyInVopsOffStatus();
        case Feature::BLOCK_WIFI_EMERGENCY_CALL_IF_NOT_PROVISIONED:
            return m_pManager->IsBlockWifiEmergencyCallIfNotProvisioned();
        case Feature::SUPPORT_CANID_INFO:
            return m_pManager->IsSupportCanidInfo();
        case Feature::USE_CARRIER_SPECIFIC_CONTACT_HEADER_FOR_OPTIONS_RESPONSE:
            return m_pManager->IsUseCarrierSpecificContactHeaderForOptionsResponse();
        case Feature::USE_CARRIER_SPECIFIC_REJECT_PHRASE_FOR_INCOMING_CALL_DURING_NO_REGISTRATION:
            return m_pManager
                    ->IsUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration();
        case Feature::ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_REJECTED_BY_SERVER_ERROR:
            return m_pManager->IsEnableRegistrationRecoveryWhenCallRejectedByServerError();
        case Feature::ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_RETRY_UNAVAILABLE:
            return m_pManager->IsEnableRegistrationRecoveryWhenCallRetryUnavailable();
        case Feature::REJECT_VOWIFI_VOICE_CALL_WHEN_VOWIFI_SETTING_OFF:
            return m_pManager->IsRejectVowifiVoiceCallWhenVowifiSettingOff();
        case Feature::CHECK_SERVER_OUTAGE_REASON_FOR_VXLTE_CALL:
            return m_pManager->IsCheckServerOutageReasonForVxlteCall();
        case Feature::SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE:
            return m_pManager->IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType();
        case Feature::MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING:
            return m_pManager->IsMaintainMultipleEarlySessionsByForking();
        case Feature::STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY:
            return m_pManager->IsStopRingbackTimerBy183WithSdpBody();
        case Feature::INITIALIZE_PEM_WHEN_NO_HEADER:
            return m_pManager->IsInitializePemWhenNoHeader();
        case Feature::RELEASE_EMERGENCY_PDN_WITH_EMERGENCY_CALL_FAIL:
            return m_pManager->IsReleaseEmergencyPdnWithEmergencyCallFail();
        case Feature::REQUIRED_CDMALESS_FEATURE_TAG:
            return m_pManager->IsRequiredCdmalessFeatureTag();
        case Feature::EMERGENCY_CALL_CURRENT_LOCATION_DISCOVERY_SUPPORTED:
            return m_pManager->IsEmergencyCallCurrentLocationDiscoverySupported();
        case Feature::CHECK_UI_CONDITION_FOR_INCOMING_RESUME:
            return m_pManager->IsCheckUiConditionForIncomingResume();
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
            return m_pManager->IsPidfShortCode(strAdditionalInfo);
        case Feature::CARRIER_SPECIFIC_SIP_HEADER:
            return m_pManager->IsCarrierSpecificSipHeader(strAdditionalInfo);
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
            return m_pManager->IsSupportGeolocationPidfInSipInvite(nAdditionalInfo);
        case Feature::SRVCC_TYPE:
            return m_pManager->IsSrvccType(nAdditionalInfo);
        case Feature::AUDIO_INACTIVITY_CALL_END_REASON:
            return m_pManager->IsAudioInactivityCallEndReason(nAdditionalInfo);
        case Feature::SHORT_CALL_CODE:
            return m_pManager->IsShortCallCode(nAdditionalInfo);
        case Feature::REJECT_CODE_FOR_CSFB:
            return m_pManager->IsRejectCodeForCsfb(nAdditionalInfo);
        case Feature::CALL_MAINTAINING_ON_REGISTRATION_SUSPENDED:
            return m_pManager->IsCallMaintainingOnRegistrationSuspended(nAdditionalInfo);
        case Feature::REQUIRING_EMERGENCY_CALL_WHEN_VIDEO_EMERGENCY_CALL_FAILED:
            return m_pManager->IsRequiringEmergencyCallWhenVideoEmergencyCallFailed(
                    nAdditionalInfo);
        case Feature::VILTE_TO_VOLTE_RETRY_FAILURE_RESPONSE_CODE:
            return m_pManager->IsVilteToVolteRetryFailureResponseCode(nAdditionalInfo);
        case Feature::REGISTRATION_DISCONNECT_REASON_TO_IGNORE:
            return m_pManager->IsRegistrationDisconnectReasonToIgnore(nAdditionalInfo);
        case Feature::MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF:
            return m_pManager->IsMessageTypeSupportGeolocationPidf(
                    static_cast<MessageTypeForGeolocationPidf>(nAdditionalInfo));
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
            return m_pManager->GetRequestUriType();
        case Feature::SESSION_PRIVACY_TYPE:
            return m_pManager->GetSessionPrivacyType();
        case Feature::CONFERENCE_SUBSCRIBE_TYPE:
            return m_pManager->GetConferenceSubscribeType();
        case Feature::DEDICATED_BEARER_WAIT_TIMER:
            return m_pManager->GetDedicatedBearerWaitTimer();
        case Feature::RINGING_TIMER:
            return m_pManager->GetRingingTimer();
        case Feature::RINGBACK_TIMER:
            return m_pManager->GetRingbackTimer();
        case Feature::MO_CALL_REQUEST_TIMEOUT:
            return m_pManager->GetMoCallRequestTimeout();
        case Feature::TIMER_18X:
            return m_pManager->Get18xTimer();
        case Feature::CONFERENCE_SIP_FLOW_ORDER:
            return m_pManager->GetConferenceSipFlowOrder();
        case Feature::CONFERENCE_INVITING_REFER_TYPE:
            return m_pManager->GetConferenceInvitingReferType();
        case Feature::POLICY_QOS_PRECONDITION_MECHANISM_WHILE_CALL_MODIFICATION:
            return m_pManager->GetPolicyQosPreconditionMechanismWhileCallModification();
        case Feature::INCOMING_CALL_REJECT_CODE_FOR_USER_DECLINE:
            return m_pManager->GetIncomingCallRejectCodeForUserDecline();
        case Feature::INCOMING_CALL_REJECT_CODE_FOR_NO_ANSWER:
            return m_pManager->GetIncomingCallRejectCodeForNoAnswer();
        case Feature::PRACK_UPDATE_RESPONSE_WAIT_TIMER:
            return m_pManager->GetPrackUpdateResponseWaitTimer();
        case Feature::SESSION_REFRESH_TRIGGER_INTERVAL:
            return m_pManager->GetSessionRefreshTriggerInterval();
        case Feature::REGISTRATION_RESTORATION_MODE_ON_504_FOR_INVITE:
            return m_pManager->GetRegistrationRestorationModeOn504ForInvite();
        case Feature::POLICY_ON_AUDIO_QOS_DEACTIVATION:
            return m_pManager->GetPolicyOnAudioQosDeactivation();
        case Feature::POLICY_FOR_TEXT_WITH_VIDEO:
            return m_pManager->GetPolicyForTextWithVideo();
        case Feature::POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR:
            return m_pManager->GetPolicyForMediaTypeRestrictionOnCellular();
        case Feature::POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_IN_ROAMING:
            return m_pManager->GetPolicyForMediaTypeRestrictionOnCellularInRoaming();
        case Feature::POLICY_OF_LOCAL_NUMBERS:
            return m_pManager->GetPolicyOfLocalNumbers();
        case Feature::SILENT_REDIAL_INTERVAL:
            return m_pManager->GetSilentRedialInterval();
        case Feature::CALL_TYPE_AFTER_AUDIO_AND_VIDEO_CALL_MERGED:
            return m_pManager->GetCallTypeAfterAudioAndVideoCallMerged();
        case Feature::SILENT_REDIAL_MAX_RETRY_COUNT:
            return m_pManager->GetSilentRedialMaxRetryCount();
        case Feature::POLICY_FOR_403_RESPONSE_FOR_INVITE:
            return m_pManager->GetPolicyFor403ResponseForInvite();
        case Feature::POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING:
            return m_pManager->GetPolicyForCheckingQosWhileCallUpgrading();
        case Feature::CALL_MAX_COUNT:
            return m_pManager->GetCallMaxCount();
        case Feature::CONVERT_REMOTE_RESPONSE_TIMER:
            return m_pManager->GetConvertRemoteResponseTimer();
        case Feature::CONVERT_USER_RESPONSE_TIMER:
            return m_pManager->GetConvertUserResponseTimer();
        case Feature::POLICY_ON_VIDEO_QOS_DEACTIVATION:
            return m_pManager->GetPolicyOnVideoQosDeactivation();
        case Feature::MINIMUM_BATTERY_LEVEL_FOR_LIMIT_VIDEO_CALL:
            return m_pManager->GetMinimumBatteryLevelForLimitVideoCall();
        case Feature::POLICY_ON_TEXT_QOS_DEACTIVATION:
            return m_pManager->GetPolicyOnTextQosDeactivation();
        case Feature::EMERGENCY_T_CALL_TIMER:
            return m_pManager->GetEmergencyTCallTimer();
        case Feature::EMERGENCY_RINGBACK_TIMER:
            return m_pManager->GetEmergencyRingbackTimer();
        case Feature::EMERGENCY_18X_TIMER:
            return m_pManager->GetEmergency18xTimer();
        case Feature::POLICY_FOR_EMERGENCY_URN_ESCV_MAPPING:
            return m_pManager->GetPolicyForEmergencyUrnEscvMapping();
        case Feature::CONFERENCE_DROP_REFER_TO_URI_SOURCE_TYPE:
            return m_pManager->GetConferenceDropReferToUriSourceType();
        case Feature::MEDIA_TYPE_FOR_OFFERLESS_REINVITE:
            return m_pManager->GetMediaTypeForOfferlessReinvite();
        case Feature::OIP_TYPE_FOR_UNAVAILABLE:
            return m_pManager->GetOipTypeForUnavailable();
        case Feature::EMERGENCY_RTT_GUARD_TIMER:
            return m_pManager->GetEmergencyRttGuardTimer();
        case Feature::PRE_ALERTING_TIMER:
            return m_pManager->GetPreAlertingTimer();
        case Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL:
            return m_pManager->GetPolicyForTcallTimerExpiryOfVolteCall();
        case Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_EMERGENCY_CALL:
            return m_pManager->GetPolicyForTcallTimerExpiryOfVolteEmergencyCall();
        case Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL:
            return m_pManager->GetPolicyForTcallTimerExpiryOfVowifiCall();
        case Feature::WIFI_EMERGENCY_18X_TIMER:
            return m_pManager->GetWifiEmergency18xTimer();
        case Feature::MAXIMUM_WAIT_TIMER_FOR_GEOLOCATION_PIDF_INFO:
            return m_pManager->GetMaximumWaitTimerForGeolocationPidfInfo();
        case Feature::POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE:
            return m_pManager->GetPolicyForLocalRingbackToneWith180Response();
        case Feature::EPS_FALLBACK_WATCHDOG_TIME:
            return m_pManager->GetEpsFallbackWatchdogTime();
        case Feature::SEND_UDP_KEEP_ALIVE_INTERVAL_TIME:
            return m_pManager->GetSendUdpKeepAliveIntervalTime();
        case Feature::CALL_REJECT_CODE_FOR_NOT_ACCEPTABLE_CALL_TYPE:
            return m_pManager->GetCallRejectCodeForNotAcceptableCallType();
        case Feature::POLICY_FOR_ALERT_NOT_USING_PRECONDITION_MECHANISM:
            return m_pManager->GetPolicyForAlertNotUsingPreconditionMechanism();
        default:
            IMS_TRACE_E(0, "invalid feature [%d]", eFeature, 0, 0);
            return 0;
    }
}

PUBLIC
IMS_SINT32 MtcConfigurationProxy::GetInt(
        IN Feature eFeature, IN IMS_BOOL bParam1, IN IMS_BOOL bParam2, IN IMS_BOOL bParam3) const
{
    switch (eFeature)
    {
        case Feature::INFORMATION_LEVEL_OF_GEOLOCATION_PIDF:
            return m_pManager->GetInformationLevelOfGeolocationPidf(bParam1, bParam2, bParam3);
        default:
            IMS_TRACE_E(0, "invalid feature [%d]", eFeature, 0, 0);
            return 0;
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
            return m_pManager->GetConferenceFactoryUri();
        case Feature::CALL_TERMINATE_REASON_HEADER:
            return m_pManager->GetCallTerminateReasonHeader(
                    static_cast<TerminateType>(nAdditionalInfo));
        case Feature::CALL_REJECT_REASON_PHRASE:
            return m_pManager->GetCallRejectReasonPhrase(static_cast<RejectType>(nAdditionalInfo));
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
