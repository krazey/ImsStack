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

#include "AString.h"
#include "CarrierConfig.h"
#include "ICarrierConfig.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ServiceTrace.h"
#include "configuration/CarrierConfigItems.h"
#include "configuration/MtcConfigurationUpdater.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC GLOBAL void MtcConfigurationUpdater::Update(
        IN ICarrierConfig* piCc, IN CarrierConfigItems& objCarrierConfigItems)
{
    IMS_TRACE_I("Update", 0, 0, 0);
    ClearArrays(objCarrierConfigItems);
    UpdateByCarrierConfig(piCc, objCarrierConfigItems);
}

PRIVATE GLOBAL void MtcConfigurationUpdater::ClearArrays(
        IN CarrierConfigItems& objCarrierConfigItems)
{
    objCarrierConfigItems.objCallRejectReasonPhrases.Clear();
    objCarrierConfigItems.objCallTerminateReasonHeaders.Clear();
}

PRIVATE GLOBAL void MtcConfigurationUpdater::UpdateByCarrierConfig(
        IN ICarrierConfig* piCc, IN CarrierConfigItems& objItems)
{
    IMS_TRACE_I("UpdateByCarrierConfig", 0, 0, 0);

    objItems.nRequestUriType = piCc->GetInt(CarrierConfig::Ims::KEY_REQUEST_URI_TYPE_INT);
    objItems.objSupportGeolocationPidfInSipInvite = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY);
    objItems.bSupportSipSessionIdHeader =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_SUPPORT_SIP_SESSION_ID_HEADER_BOOL);

    // audio configurations
    objItems.bIncludeCallerIdServiceCodesInSipInvite = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL);
    objItems.bMultiendpointSupported =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_MULTIENDPOINT_SUPPORTED_BOOL);
    objItems.bSessionTimerSupported =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_SESSION_TIMER_SUPPORTED_BOOL);
    objItems.nSessionPrivacyType =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_SESSION_PRIVACY_TYPE_INT);
    objItems.bPrackSupportedFor18x =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_PRACK_SUPPORTED_FOR_18X_BOOL);
    objItems.nConferenceSubscribeType =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_CONFERENCE_SUBSCRIBE_TYPE_INT);
    objItems.bVoiceQosPreconditionSupported =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL);
    objItems.bVoiceOnDefaultBearerSupported =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL);
    objItems.nDedicatedBearerWaitTimer =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT);
    objItems.objSrvccTypes = piCc->GetIntArray(CarrierConfig::ImsVoice::KEY_SRVCC_TYPE_INT_ARRAY);
    objItems.nRingingTimer = piCc->GetInt(CarrierConfig::ImsVoice::KEY_RINGING_TIMER_MILLIS_INT);
    objItems.nRingbackTimer = piCc->GetInt(CarrierConfig::ImsVoice::KEY_RINGBACK_TIMER_MILLIS_INT);
    objItems.strConferenceFactoryUri =
            piCc->GetString(CarrierConfig::ImsVoice::KEY_CONFERENCE_FACTORY_URI_STRING);
    objItems.bOipSourceFromHeader =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_OIP_SOURCE_FROM_HEADER_BOOL);
    objItems.nMoCallRequestTimeout =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT);
    objItems.objAudioInactivityCallEndReasons = piCc->GetIntArray(
            CarrierConfig::ImsVoice::KEY_AUDIO_INACTIVITY_CALL_END_REASONS_INT_ARRAY);
    objItems.n18xTimer = piCc->GetInt(CarrierConfig::ImsVoice::KEY_18X_TIMER_MILLIS_INT);
    objItems.bSupportConferenceReferSubscribe =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_SUPPORT_CONFERENCE_REFER_SUBSCRIBE_BOOL);
    objItems.bEnableConferenceSubscribeByParticipant = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT_BOOL);
    objItems.objEmergencyRegistrationTo18xTimer = piCc->GetIntArray(
            CarrierConfig::ImsVoice::KEY_REGISTRATION_TO_18X_TIMER_MILLIS_INT_ARRAY);
    // KEY_SUPPORT_IMS_CONFERENCE_EVENT_PACKAGE_ON_PEER_BOOL
    objItems.nConferenceSipFlowOrder =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_CONFERENCE_SIP_FLOW_ORDER_INT);
    objItems.nConferenceInvitingReferType =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT);
    objItems.nPolicyQosPreconditionMechanismWhileCallModification = piCc->GetInt(CarrierConfig::
                    ImsVoice::KEY_POLICY_QOS_PRECONDITION_MECHANISM_WHILE_CALL_MODIFICATION_INT);
    objItems.nIncomingCallRejectCodeForUserDecline = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_INCOMING_CALL_REJECT_CODE_FOR_USER_DECLINE_INT);
    objItems.nIncomingCallRejectCodeForNoAnswer =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_INCOMING_CALL_REJECT_CODE_FOR_NO_ANSWER_INT);
    objItems.nPrackUpdateResponseWaitTimer =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_PRACK_UPDATE_RESPONSE_WAIT_TIMER_MILLIS_INT);
    objItems.nSessionRefreshTriggerInterval =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_SESSION_REFRESH_TRIGGER_INTERVAL_SEC_INT);
    objItems.nRegistrationRestorationModeOn504ForInvite = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_REGISTRATION_RESTORATION_MODE_ON_504_FOR_INVITE_INT);
    objItems.nPolicyOnAudioQosDeactivation =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_POLICY_ON_AUDIO_QOS_DEACTIVATION_INT);
    objItems.bEnableSendReinviteOnRatChange =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_ENABLE_SEND_REINVITE_ON_RAT_CHANGE_BOOL);
    objItems.nPolicyForMediaTypeRestrictionOnCellular = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_INT);
    objItems.nPolicyForMediaTypeRestrictionOnCellularInRoaming = piCc->GetInt(CarrierConfig::
                    ImsVoice::KEY_POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_IN_ROAMING_INT);
    objItems.nPolicyOfLocalNumbers =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_POLICY_OF_LOCAL_NUMBERS_INT);
    objItems.nSilentRedialInterval =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT);
    objItems.nCallTypeAfterAudioAndVideoCallMerged = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_CALL_TYPE_AFTER_AUDIO_AND_VIDEO_CALL_MERGED_INT);
    objItems.objShortCallCodes =
            piCc->GetIntArray(CarrierConfig::ImsVoice::KEY_SHORT_CALL_CODE_INT_ARRAY);
    objItems.bAllowMultipleCallIncludingVideoCall = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL_BOOL);
    objItems.objRejectCodeForCsfbs =
            piCc->GetIntArray(CarrierConfig::ImsVoice::KEY_REJECT_CODE_FOR_CSFB_INT_ARRAY);
    objItems.nSilentRedialMaxRetryCount =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT);
    objItems.nPolicyFor403ResponseForInvite =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_POLICY_FOR_403_RESPONSE_FOR_INVITE_INT);
    objItems.nPolicyForCheckingQosWhileCallUpgrading = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT);
    objItems.bRejectOfferlessInvite =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL);
    objItems.nCallMaxCount = piCc->GetInt(CarrierConfig::ImsVoice::KEY_CALL_MAX_COUNT_INT);
    // termiate reason
    objItems.objCallTerminateReasonHeaders.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING));
    objItems.objCallTerminateReasonHeaders.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_RTP_TIMEOUT_STRING));
    objItems.objCallTerminateReasonHeaders.Push(piCc->GetString(CarrierConfig::ImsVoice::
                    KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_AND_RTP_TIMEOUT_STRING));
    objItems.objCallTerminateReasonHeaders.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_MEDIA_BEARER_LOSS_STRING));
    objItems.objCallTerminateReasonHeaders.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_SIP_TIMEOUT_STRING));
    objItems.objCallTerminateReasonHeaders.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_SIP_RESPONSE_TIMEOUT_STRING));
    objItems.objCallTerminateReasonHeaders.Push(piCc->GetString(CarrierConfig::ImsVoice::
                    KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_AND_SIP_RESPONSE_TIMEOUT_STRING));
    objItems.objCallTerminateReasonHeaders.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_CALL_SETUP_TIMEOUT_STRING));
    objItems.objCallTerminateReasonHeaders.Push(piCc->GetString(CarrierConfig::ImsVoice::
                    KEY_CALL_TERMINATE_REASON_HEADER_TERMINATING_EARLYDIALOG_STRING));
    objItems.objCallTerminateReasonHeaders.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_VOPS_OFF_STRING));
    objItems.objCallTerminateReasonHeaders.Push(piCc->GetString(CarrierConfig::ImsVoice::
                    KEY_CALL_TERMINATE_REASON_HEADER_SESSION_REFRESH_FAILURE_STRING));
    objItems.objCallTerminateReasonHeaders.Push(piCc->GetString(CarrierConfig::ImsVoice::
                    KEY_CALL_TERMINATE_REASON_HEADER_CONFERENCE_CALL_JOINED_STRING));
    // reject reason
    objItems.objCallRejectReasonPhrases.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_CSCALL_STRING));
    objItems.objCallRejectReasonPhrases.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_VILTE_AND_NO_LTE_STRING));
    objItems.objCallRejectReasonPhrases.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_CONNECTING_CALL_STRING));
    objItems.objCallRejectReasonPhrases.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_EXCEEDS_MAX_CALL_COUNT_STRING));
    objItems.objCallRejectReasonPhrases.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_CONVERTING_STRING));
    objItems.objCallRejectReasonPhrases.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_NEGOTIATION_FAILURE_STRING));
    objItems.objCallRejectReasonPhrases.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_NO_ANSWER_BY_USER_STRING));
    objItems.objCallRejectReasonPhrases.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_VOWIFI_OFF_STRING));
    objItems.objCallRejectReasonPhrases.Push(piCc->GetString(
            CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_USER_REJECT_STRING));

    // vt configurations
    objItems.bVideoOnDefaultBearerSupported =
            piCc->GetBoolean(CarrierConfig::ImsVt::KEY_VIDEO_ON_DEFAULT_BEARER_SUPPORTED_BOOL);
    objItems.bVideoQosPreconditionSupported =
            piCc->GetBoolean(CarrierConfig::ImsVt::KEY_VIDEO_QOS_PRECONDITION_SUPPORTED_BOOL);
    objItems.nConvertRemoteResponseTimer =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_CONVERT_REMOTE_RESPONSE_TIMER_MILLIS_INT);
    objItems.nConvertUserResponseTimer =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_CONVERT_USER_RESPONSE_TIMER_MILLIS_INT);
    objItems.nPolicyOnVideoQosDeactivation =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_POLICY_ON_VIDEO_QOS_DEACTIVATION_INT);
    objItems.bSupportEarlySession =
            piCc->GetBoolean(CarrierConfig::ImsVt::KEY_SUPPORT_EARLY_SESSION_BOOL);
    objItems.nPolicyForTextWithVideo =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT);
    objItems.nMinimumBatteryLevelForLimitVideoCall =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_MINIMUM_BATTERY_LEVEL_FOR_LIMIT_VIDEO_CALL_INT);

    // rtt configurations
    objItems.bTextOnDefaultBearerSupported =
            piCc->GetBoolean(CarrierConfig::ImsRtt::KEY_TEXT_ON_DEFAULT_BEARER_SUPPORTED_BOOL);
    objItems.bTextQosPreconditionSupported =
            piCc->GetBoolean(CarrierConfig::ImsRtt::KEY_TEXT_QOS_PRECONDITION_SUPPORTED_BOOL);
    objItems.nPolicyOnTextQosDeactivation =
            piCc->GetInt(CarrierConfig::ImsRtt::KEY_POLICY_ON_TEXT_QOS_DEACTIVATION_INT);

    // wfc configurations
    objItems.objPidfShortCodes =
            piCc->GetStringArray(CarrierConfig::ImsWfc::KEY_PIDF_SHORT_CODE_STRING_ARRAY);
    objItems.bEmergencyCallOverEmergencyPdn =
            piCc->GetBoolean(CarrierConfig::ImsWfc::KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_BOOL);
    objItems.nCountryCode = piCc->GetInt(CarrierConfig::ImsWfc::KEY_COUNTRY_CODE_INT);

    // emergency configurations
    objItems.bRetryEmergencyOnImsPdnBool =
            piCc->GetBoolean(CarrierConfig::ImsEmergency::KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL);
    objItems.bEmergencyQosPreconditionSupported = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_EMERGENCY_QOS_PRECONDITION_SUPPORTED_BOOL);
    objItems.bEmergencyCallOverEmergencyPdnOnCellular = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_ON_CELLULAR_BOOL);
    objItems.bEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::
                    KEY_EMERGENCY_RETRY_WITHOUT_CHECKING_380_CONTENT_FOR_NON_UE_DETECTABLE_EMERGENCY_CALL_BOOL);
    objItems.nEmergencyTCallTimer =
            piCc->GetInt(CarrierConfig::ImsEmergency::KEY_EMERGENCY_TCALL_TIMER_MILLIS_INT);
    objItems.nEmergencyRingbackTimer =
            piCc->GetInt(CarrierConfig::ImsEmergency::KEY_EMERGENCY_RINGBACK_TIMER_MILLIS_INT);
    objItems.nEmergency18xTimer =
            piCc->GetInt(CarrierConfig::ImsEmergency::KEY_EMERGENCY_18X_TIMER_MILLIS_INT);
    objItems.nPolicyForEmergencyUrnEscvMapping = piCc->GetInt(
            CarrierConfig::ImsEmergency::KEY_POLICY_FOR_EMERGENCY_URN_ESCV_MAPPING_INT);

    objItems.bCheckConferenceEventPackageVersion = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_CHECK_CONFERENCE_EVENT_PACKAGE_VERSION_BOOL);
    objItems.bConferenceReferToUriSourcePaid =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_CONFERENCE_REFER_TO_URI_SOURCE_PAID_BOOL);
    objItems.nConferenceDropReferToUriSourceType =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_CONFERENCE_DROP_REFER_TO_URI_SOURCE_TYPE_INT);
    objItems.bEnableFakeQosCallFlowOnWifi =
            piCc->GetBoolean(CarrierConfig::ImsWfc::KEY_ENABLE_FAKE_QOS_CALL_FLOW_ON_WIFI_BOOL);
    objItems.nMediaTypeForOfferlessInvite =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_INVITE_INT);
    objItems.nMediaTypeForOfferlessReinvite =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_REINVITE_INT);
    objItems.bSupportVideoCallUpgradeRegardlessOfFeatureTags = piCc->GetBoolean(
            CarrierConfig::ImsVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL);
    objItems.nOipTypeForUnavailable =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_OIP_TYPE_FOR_UNAVAILABLE_INT);
    objItems.bEnableOipHeaderPolicyFallBack =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_ENABLE_OIP_HEADER_POLICY_FALLBACK_BOOL);
    objItems.nDelayUpdateAfterConnectedTimer = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_DELAY_UPDATE_AFTER_CONNECTED_TIMER_MILLIS_INT);
    objItems.nEmergencyRttGuardTimer =
            piCc->GetInt(CarrierConfig::ImsEmergency::KEY_EMERGENCY_RTT_GUARD_TIMER_MILLIS_INT);
    objItems.bRetryEmergencyCallOverEmergencyPdnWithNextPcscf = piCc->GetBoolean(CarrierConfig::
                    ImsEmergency::KEY_RETRY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_WITH_NEXT_PCSCF_BOOL);
    objItems.nPreAlertingTimer =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_PREALERTING_TIMER_MILLIS_INT);
    objItems.nPolicyForTcallTimerExpiryOfVolteCall = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL_INT);
    objItems.nPolicyForTcallTimerExpiryOfVolteEmergencyCall = piCc->GetInt(CarrierConfig::
                    ImsEmergency::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_EMERGENCY_CALL_INT);
    objItems.nPolicyForTcallTimerExpiryOfVowifiCall = piCc->GetInt(
            CarrierConfig::ImsWfc::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL_INT);
    objItems.objCarrierSpecificSipHeaders = piCc->GetStringArray(
            CarrierConfig::ImsVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY);
    objItems.bCheckAvchangeFeatureForCallConvertingCapability = piCc->GetBoolean(
            CarrierConfig::ImsVt::KEY_CHECK_AVCHANGE_FEATURE_FOR_CALL_CONVERTING_CAPABILITY_BOOL);
    objItems.bSupportRegistrationRecoveryForFailureOfSessionRefresh =
            piCc->GetBoolean(CarrierConfig::ImsVoice::
                            KEY_SUPPORT_REGISTRATION_RECOVERY_FOR_FAILURE_OF_SESSION_REFRESH_BOOL);
    objItems.objCallMaintainingOnRegistrationSuspendeds = piCc->GetIntArray(CarrierConfig::
                    ImsVoice::KEY_POLICY_FOR_CALL_MAINTAINING_ON_REGISTRATION_SUSPENDED_INT_ARRAY);
    objItems.objRequiringEmergencyCallWhenVideoEmergencyCallFaileds = piCc->GetIntArray(
            CarrierConfig::ImsEmergency::
                    KEY_POLICY_FOR_REQUIRING_EMERGENCY_CALL_WHEN_VIDEO_EMERGENCY_CALL_FAILED_INT_ARRAY);
    objItems.bUseMcidSupplementaryService =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_USE_MCID_SUPPLEMENTARY_SERVICE_BOOL);
    objItems.bUseMmcSupplementaryService =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_USE_MMC_SUPPLEMENTARY_SERVICE_BOOL);
    objItems.bUseLtePreferredStatusForServiceCapability = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_USE_LTE_PREFERRED_STATUS_FOR_SERVICE_CAPABILITY_BOOL);
    objItems.bAllowIncomingHoldRequestDuringConferenceCall = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_ALLOW_INCOMING_HOLD_REQUEST_DURING_CONFERENCE_CALL_BOOL);
    objItems.bIgnore180After183Response =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_IGNORE_180_AFTER_183_RESPONSE_BOOL);
    objItems.bAddReplaceHeaderForConference =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_ADD_REPLACE_HEADER_FOR_CONFERENCE_BOOL);
    objItems.objVilteToVolteRetryFailureResponseCodes = piCc->GetIntArray(
            CarrierConfig::ImsVt::KEY_VILTE_TO_VOLTE_RETRY_FAILURE_RESPONSE_CODE_INT_ARRAY);
    objItems.bUseEmergencyNumberTranslationInRoamingStatus = piCc->GetBoolean(CarrierConfig::
                    ImsEmergency::KEY_USE_EMERGENCY_NUMBER_TRANSLATION_IN_ROAMING_STATUS_BOOL);
    objItems.bIgnorePrackDeliveryFailure =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_IGNORE_PRACK_DELIVERY_FAILURE_BOOL);
    objItems.bSupportVideoCallOnlyInVopsOffStatus = piCc->GetBoolean(
            CarrierConfig::ImsVt::KEY_SUPPORT_VIDEO_CALL_ONLY_IN_VOPS_OFF_STATUS_BOOL);
    objItems.bBlockWifiEmergencyCallIfNotProvisioned = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_BLOCK_WIFI_EMERGENCY_CALL_IF_NOT_PROVISIONED_BOOL);
    objItems.objRegistrationDisconnectReasonToIgnore = piCc->GetIntArray(
            CarrierConfig::ImsVoice::KEY_REGISTRATION_DISCONNECT_REASON_TO_IGNORE_INT_ARRAY);
    objItems.nWifiEmergency18xTimer =
            piCc->GetInt(CarrierConfig::ImsEmergency::KEY_WIFI_EMERGENCY_18X_TIMER_MILLIS_INT);
    objItems.bUseCarrierSpecificContactHeaderForOptionsResponse = piCc->GetBoolean(CarrierConfig::
                    ImsVoice::KEY_USE_CARRIER_SPECIFIC_CONTACT_HEADER_FOR_OPTIONS_RESPONSE_BOOL);
    objItems.bUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration = piCc->GetBoolean(
            CarrierConfig::ImsVoice::
                    KEY_USE_CARRIER_SPECIFIC_REJECT_PHRASE_FOR_INCOMING_CALL_DURING_NO_REGISTRATION_BOOL);
    objItems.bEnableRegistrationRecoveryWhenCallRejectedByServerError = piCc->GetBoolean(
            CarrierConfig::ImsVoice::
                    KEY_ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_REJECTED_BY_SERVER_ERROR_BOOL);
    objItems.bEnableRegistrationRecoveryWhenCallRetryUnavailable = piCc->GetBoolean(CarrierConfig::
                    ImsVoice::KEY_ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_RETRY_UNAVAILABLE_BOOL);
    objItems.bRejectVowifiVoiceCallWhenVowifiSettingOff = piCc->GetBoolean(
            CarrierConfig::ImsWfc::KEY_REJECT_VOWIFI_VOICE_CALL_WHEN_VOWIFI_SETTING_OFF_BOOL);
    objItems.bCheckServerOutageReasonForVxlteCall = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_CHECK_SERVER_OUTAGE_REASON_FOR_VXLTE_CALL_BOOL);
    objItems.bSetVideoTextFeatureExclusivelyInContactHeaderBySessionType = piCc->GetBoolean(
            CarrierConfig::ImsVt::
                    KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL);
    objItems.nMaximumWaitTimerForGeolocationPidfInfo = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_MAXIMUM_WAIT_TIMER_FOR_GEOLOCATION_PIDF_INFO_MILLIS_INT);
    objItems.bMaintainMultipleEarlySessionsByForking = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING_BOOL);
    objItems.bStopRingbackTimerBy183WithSdpBody = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY_BOOL);
    objItems.objInformationLevelOfGeolocationPidfs = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_INFORMATION_LEVEL_OF_GEOLOCATION_PIDF_INT_ARRAY);
    objItems.objMessageTypesSupportGeolocationPidf = piCc->GetIntArray(
            CarrierConfig::ImsVoice::KEY_MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF_INT_ARRAY);
    objItems.bInitializePemWhenNoHeader = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_INITIALIZE_P_EARLY_MEDIA_WHEN_NO_HEADER_BOOL);
    objItems.nPolicyForLocalRingbackToneWith180Response = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT);
    objItems.nEpsFallbackWatchDogTime =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT);
    objItems.nSendUdpKeepAliveIntervalTime =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT);
    objItems.nCallRejectCodeForNotAcceptableCallType = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_CALL_REJECT_CODE_FOR_NOT_ACCEPTABLE_CALL_TYPE_INT);
    objItems.bReleaseEmergencyPdnWithEmergencyCallFail = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_RELEASE_EMERGENCY_PDN_WITH_EMERGENCY_CALL_FAIL_BOOL);
    objItems.bRequiredCdmalessFeatureTag =
            piCc->GetBoolean(CarrierConfig::Assets::KEY_REQUIRED_CDMALESS_FEATURE_TAG_BOOL);
    objItems.bEmergencyCallCurrentLocationDiscoverySupported = piCc->GetBoolean(CarrierConfig::
                    ImsEmergency::KEY_EMERGENCY_CALL_CURRENT_LOCATION_DISCOVERY_SUPPORTED_BOOL);
    objItems.bCheckUiConditionForIncomingResume = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_CHECK_UI_CONDITION_FOR_INCOMING_RESUME_BOOL);
    objItems.strPEmergencyInfoHeaderInInvite = piCc->GetString(
            CarrierConfig::ImsEmergency::KEY_P_EMERGENCY_INFO_HEADER_IN_INVITE_STRING);
}
