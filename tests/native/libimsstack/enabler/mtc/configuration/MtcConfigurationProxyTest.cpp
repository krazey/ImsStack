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

#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include <gtest/gtest.h>

using ::testing::Return;

class MtcConfigurationProxyTest : public ::testing::Test
{
public:
    MockIMtcConfigurationManager* pConfigManager;
    MtcConfigurationProxy* pConfig;

protected:
    virtual void SetUp() override
    {
        pConfigManager = new MockIMtcConfigurationManager();
        pConfig = new MtcConfigurationProxy(pConfigManager);
    }

    virtual void TearDown() override { delete pConfig; }
};

TEST_F(MtcConfigurationProxyTest, Init)
{
    EXPECT_CALL(*pConfigManager, Init).Times(1);

    pConfig->Init();
}

TEST_F(MtcConfigurationProxyTest, IsReturnsFromConfigManager)
{
    const IMS_BOOL bValue = IMS_TRUE;

    EXPECT_CALL(*pConfigManager, IsSupportSipSessionIdHeader).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::SUPPORT_SIP_SESSION_ID_HEADER));

    EXPECT_CALL(*pConfigManager, IsIncludeCallerIdServiceCodesInSipInvite).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE));

    EXPECT_CALL(*pConfigManager, IsMultiendpointSupported).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::MULTIENDPOINT_SUPPORTED));

    EXPECT_CALL(*pConfigManager, IsSessionTimerSupported).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::SESSION_TIMER_SUPPORTED));

    EXPECT_CALL(*pConfigManager, IsPrackSupportedFor18x).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::PRACK_SUPPORTED_FOR_18X));

    EXPECT_CALL(*pConfigManager, IsVoiceQosPreconditionSupported).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::VOICE_QOS_PRECONDITION_SUPPORTED));

    EXPECT_CALL(*pConfigManager, IsVoiceOnDefaultBearerSupported).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::VOICE_ON_DEFAULT_BEARER_SUPPORTED));

    EXPECT_CALL(*pConfigManager, IsOipSourceFromHeader).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::OIP_SOURCE_FROM_HEADER));

    EXPECT_CALL(*pConfigManager, IsSupportConferenceReferSubscribe).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::SUPPORT_CONFERENCE_REFER_SUBSCRIBE));

    EXPECT_CALL(*pConfigManager, IsEnableConferenceSubscribeByParticipant).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT));

    EXPECT_CALL(*pConfigManager, IsEnableSendReinviteOnRatChange).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::ENABLE_SEND_REINVITE_ON_RAT_CHANGE));

    EXPECT_CALL(*pConfigManager, IsAllowMultipleCallIncludingVideoCall).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL));

    EXPECT_CALL(*pConfigManager, IsRejectOfferlessInvite).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::REJECT_OFFERLESS_INVITE));

    EXPECT_CALL(*pConfigManager, IsVideoOnDefaultBearerSupported).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::VIDEO_ON_DEFAULT_BEARER_SUPPORTED));

    EXPECT_CALL(*pConfigManager, IsVideoQosPreconditionSupported).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::VIDEO_QOS_PRECONDITION_SUPPORTED));

    EXPECT_CALL(*pConfigManager, IsSupportEarlySession).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::SUPPORT_EARLY_SESSION));

    EXPECT_CALL(*pConfigManager, IsTextOnDefaultBearerSupported).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::TEXT_ON_DEFAULT_BEARER_SUPPORTED));

    EXPECT_CALL(*pConfigManager, IsTextQosPreconditionSupported).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::TEXT_QOS_PRECONDITION_SUPPORTED));

    EXPECT_CALL(*pConfigManager, IsEmergencyCallOverEmergencyPdn).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::EMERGENCY_CALL_OVER_EMERGENCY_PDN));

    EXPECT_CALL(*pConfigManager, IsRetryEmergencyOnImsPdnBool).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::RETRY_EMERGENCY_ON_IMS_PDN_BOOL));

    EXPECT_CALL(*pConfigManager, IsEmergencyQosPreconditionSupported).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::EMERGENCY_QOS_PRECONDITION_SUPPORTED));

    EXPECT_CALL(*pConfigManager, IsEmergencyCallOverEmergencyPdnOnCellular)
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::EMERGENCY_CALL_OVER_EMERGENCY_PDN_ON_CELLULAR));

    EXPECT_CALL(*pConfigManager,
            IsEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall)
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue,
            pConfig->Is(Feature::
                            EMERGENCY_RETRY_WITHOUT_CHECKING380_CONTENT_FOR_NON_UE_DETECTABLE_EMERGENCY_CALL));

    EXPECT_CALL(*pConfigManager, IsCheckConferenceEventPackageVersion).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::CHECK_CONFERENCE_EVENT_PACKAGE_VERSION));

    EXPECT_CALL(*pConfigManager, IsConferenceReferToUriSourcePaid).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::CONFERENCE_REFER_TO_URI_SOURCE_PAID));

    EXPECT_CALL(*pConfigManager, IsEnableFakeQosCallFlowOnWifi).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::ENABLE_FAKE_QOS_CALL_FLOW_ON_WIFI));

    EXPECT_CALL(*pConfigManager, IsSupportVideoCallUpgradeRegardlessOfFeatureTags)
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS));

    EXPECT_CALL(*pConfigManager, IsEnableOipHeaderPolicyFallBack).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::ENABLE_OIP_HEADER_POLICY_FALLBACK));

    EXPECT_CALL(*pConfigManager, IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf)
            .WillOnce(Return(bValue));
    EXPECT_EQ(
            bValue, pConfig->Is(Feature::RETRY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_WITH_NEXT_PCSCF));

    EXPECT_CALL(*pConfigManager, IsCheckAvchangeFeatureForCallConvertingCapability)
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::CHECK_AVCHANGE_FEATURE_FOR_CALL_CONVERTING_CAPABILITY));

    EXPECT_CALL(*pConfigManager, IsSupportRegistrationRecoveryForFailureOfSessionRefresh)
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue,
            pConfig->Is(Feature::SUPPORT_REGISTRATION_RECOVERY_FOR_FAILURE_OF_SESSION_REFRESH));

    EXPECT_CALL(*pConfigManager, IsUseMcidSupplementaryService).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::USE_MCID_SUPPLEMENTARY_SERVICE));

    EXPECT_CALL(*pConfigManager, IsUseMmcSupplementaryService).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::USE_MMC_SUPPLEMENTARY_SERVICE));

    EXPECT_CALL(*pConfigManager, IsUseLtePreferredStatusForServiceCapability)
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::USE_LTE_PREFERRED_STATUS_FOR_SERVICE_CAPABILITY));

    EXPECT_CALL(*pConfigManager, IsAllowIncomingHoldRequestDuringConferenceCall)
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::ALLOW_INCOMING_HOLD_REQUEST_DURING_CONFERENCE_CALL));

    EXPECT_CALL(*pConfigManager, IsIgnore180After183Response).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::IGNORE_180_AFTER_183_RESPONSE));

    EXPECT_CALL(*pConfigManager, IsAddReplaceHeaderForConference).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::ADD_REPLACE_HEADER_FOR_CONFERENCE));

    EXPECT_CALL(*pConfigManager, IsUseEmergencyNumberTranslationInRoamingStatus)
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::USE_EMERGENCY_NUMBER_TRANSLATION_IN_ROAMING_STATUS));

    EXPECT_CALL(*pConfigManager, IsIgnorePrackDeliveryFailure).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::IGNORE_PRACK_DELIVERY_FAILURE));

    EXPECT_CALL(*pConfigManager, IsSupportVideoCallOnlyInVopsOffStatus).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::SUPPORT_VIDEO_CALL_ONLY_IN_VOPS_OFF_STATUS));

    EXPECT_CALL(*pConfigManager, IsBlockWifiEmergencyCallIfNotProvisioned).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::BLOCK_WIFI_EMERGENCY_CALL_IF_NOT_PROVISIONED));

    EXPECT_CALL(*pConfigManager, IsUseCarrierSpecificContactHeaderForOptionsResponse)
            .WillOnce(Return(bValue));
    EXPECT_EQ(
            bValue, pConfig->Is(Feature::USE_CARRIER_SPECIFIC_CONTACT_HEADER_FOR_OPTIONS_RESPONSE));

    EXPECT_CALL(
            *pConfigManager, IsUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration)
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue,
            pConfig->Is(Feature::
                            USE_CARRIER_SPECIFIC_REJECT_PHRASE_FOR_INCOMING_CALL_DURING_NO_REGISTRATION));

    EXPECT_CALL(*pConfigManager, IsEnableRegistrationRecoveryWhenCallRejectedByServerError)
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue,
            pConfig->Is(Feature::ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_REJECTED_BY_SERVER_ERROR));

    EXPECT_CALL(*pConfigManager, IsEnableRegistrationRecoveryWhenCallRetryUnavailable)
            .WillOnce(Return(bValue));
    EXPECT_EQ(
            bValue, pConfig->Is(Feature::ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_RETRY_UNAVAILABLE));

    EXPECT_CALL(*pConfigManager, IsRejectVowifiVoiceCallWhenVowifiSettingOff)
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::REJECT_VOWIFI_VOICE_CALL_WHEN_VOWIFI_SETTING_OFF));

    EXPECT_CALL(*pConfigManager, IsCheckServerOutageReasonForVxlteCall).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::CHECK_SERVER_OUTAGE_REASON_FOR_VXLTE_CALL));

    EXPECT_CALL(*pConfigManager, IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType)
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue,
            pConfig->Is(
                    Feature::SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE));

    EXPECT_CALL(*pConfigManager, IsMaintainMultipleEarlySessionsByForking).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING));

    EXPECT_CALL(*pConfigManager, IsStopRingbackTimerBy183WithSdpBody).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY));

    EXPECT_CALL(*pConfigManager, IsInitializePemWhenNoHeader).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::INITIALIZE_PEM_WHEN_NO_HEADER));

    EXPECT_CALL(*pConfigManager, IsReleaseEmergencyPdnWithEmergencyCallFail)
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::RELEASE_EMERGENCY_PDN_WITH_EMERGENCY_CALL_FAIL));

    EXPECT_CALL(*pConfigManager, IsRequiredCdmalessFeatureTag).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::REQUIRED_CDMALESS_FEATURE_TAG));

    EXPECT_CALL(*pConfigManager, IsEmergencyCallCurrentLocationDiscoverySupported)
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::EMERGENCY_CALL_CURRENT_LOCATION_DISCOVERY_SUPPORTED));

    EXPECT_CALL(*pConfigManager, IsCheckUiConditionForIncomingResume).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::CHECK_UI_CONDITION_FOR_INCOMING_RESUME));
}

TEST_F(MtcConfigurationProxyTest, IsWithStringArgReturnsFromConfigManager)
{
    const IMS_BOOL bValue = IMS_TRUE;
    const AString strArg = "some_arg";

    EXPECT_CALL(*pConfigManager, IsPidfShortCode(strArg)).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::PIDF_SHORT_CODE, strArg));

    EXPECT_CALL(*pConfigManager, IsCarrierSpecificSipHeader(strArg)).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::CARRIER_SPECIFIC_SIP_HEADER, strArg));
}

TEST_F(MtcConfigurationProxyTest, IsWithIntArgReturnsFromConfigManager)
{
    const IMS_BOOL bValue = IMS_TRUE;
    const IMS_SINT32 nArg = 1;

    EXPECT_CALL(*pConfigManager, IsSupportGeolocationPidfInSipInvite(nArg))
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::SUPPORT_GEOLOCATION_PIDF_IN_SIP_INVITE, nArg));

    EXPECT_CALL(*pConfigManager, IsSrvccType(nArg)).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::SRVCC_TYPE, nArg));

    EXPECT_CALL(*pConfigManager, IsAudioInactivityCallEndReason(nArg)).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::AUDIO_INACTIVITY_CALL_END_REASON, nArg));

    EXPECT_CALL(*pConfigManager, IsShortCallCode(nArg)).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::SHORT_CALL_CODE, nArg));

    EXPECT_CALL(*pConfigManager, IsRejectCodeForCsfb(nArg)).WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::REJECT_CODE_FOR_CSFB, nArg));

    EXPECT_CALL(*pConfigManager, IsCallMaintainingOnRegistrationSuspended(nArg))
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::CALL_MAINTAINING_ON_REGISTRATION_SUSPENDED, nArg));

    EXPECT_CALL(*pConfigManager, IsRequiringEmergencyCallWhenVideoEmergencyCallFailed(nArg))
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue,
            pConfig->Is(Feature::REQUIRING_EMERGENCY_CALL_WHEN_VIDEO_EMERGENCY_CALL_FAILED, nArg));

    EXPECT_CALL(*pConfigManager, IsVilteToVolteRetryFailureResponseCode(nArg))
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::VILTE_TO_VOLTE_RETRY_FAILURE_RESPONSE_CODE, nArg));

    EXPECT_CALL(*pConfigManager, IsRegistrationDisconnectReasonToIgnore(nArg))
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::REGISTRATION_DISCONNECT_REASON_TO_IGNORE, nArg));

    EXPECT_CALL(*pConfigManager,
            IsMessageTypeSupportGeolocationPidf(static_cast<MessageTypeForGeolocationPidf>(nArg)))
            .WillOnce(Return(bValue));
    EXPECT_EQ(bValue, pConfig->Is(Feature::MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF, nArg));
}

TEST_F(MtcConfigurationProxyTest, GetIntReturnsFromConfigManager)
{
    const IMS_SINT32 nValue = 1;

    EXPECT_CALL(*pConfigManager, GetRequestUriType).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::REQUEST_URI_TYPE));

    EXPECT_CALL(*pConfigManager, GetSessionPrivacyType).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::SESSION_PRIVACY_TYPE));

    EXPECT_CALL(*pConfigManager, GetConferenceSubscribeType).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::CONFERENCE_SUBSCRIBE_TYPE));

    EXPECT_CALL(*pConfigManager, GetDedicatedBearerWaitTimer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::DEDICATED_BEARER_WAIT_TIMER));

    EXPECT_CALL(*pConfigManager, GetRingingTimer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::RINGING_TIMER));

    EXPECT_CALL(*pConfigManager, GetRingbackTimer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::RINGBACK_TIMER));

    EXPECT_CALL(*pConfigManager, GetMoCallRequestTimeout).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::MO_CALL_REQUEST_TIMEOUT));

    EXPECT_CALL(*pConfigManager, Get18xTimer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::TIMER_18X));

    EXPECT_CALL(*pConfigManager, GetConferenceSipFlowOrder).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::CONFERENCE_SIP_FLOW_ORDER));

    EXPECT_CALL(*pConfigManager, GetConferenceInvitingReferType).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::CONFERENCE_INVITING_REFER_TYPE));

    EXPECT_CALL(*pConfigManager, GetPolicyQosPreconditionMechanismWhileCallModification)
            .WillOnce(Return(nValue));
    EXPECT_EQ(nValue,
            pConfig->GetInt(Feature::POLICY_QOS_PRECONDITION_MECHANISM_WHILE_CALL_MODIFICATION));

    EXPECT_CALL(*pConfigManager, GetIncomingCallRejectCodeForUserDecline).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::INCOMING_CALL_REJECT_CODE_FOR_USER_DECLINE));

    EXPECT_CALL(*pConfigManager, GetIncomingCallRejectCodeForNoAnswer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::INCOMING_CALL_REJECT_CODE_FOR_NO_ANSWER));

    EXPECT_CALL(*pConfigManager, GetPrackUpdateResponseWaitTimer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::PRACK_UPDATE_RESPONSE_WAIT_TIMER));

    EXPECT_CALL(*pConfigManager, GetSessionRefreshTriggerInterval).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::SESSION_REFRESH_TRIGGER_INTERVAL));

    EXPECT_CALL(*pConfigManager, GetRegistrationRestorationModeOn504ForInvite)
            .WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::REGISTRATION_RESTORATION_MODE_ON_504_FOR_INVITE));

    EXPECT_CALL(*pConfigManager, GetPolicyOnAudioQosDeactivation).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::POLICY_ON_AUDIO_QOS_DEACTIVATION));

    EXPECT_CALL(*pConfigManager, GetPolicyForMediaTypeRestrictionOnCellular)
            .WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR));

    EXPECT_CALL(*pConfigManager, GetPolicyForMediaTypeRestrictionOnCellularInRoaming)
            .WillOnce(Return(nValue));
    EXPECT_EQ(nValue,
            pConfig->GetInt(Feature::POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_IN_ROAMING));

    EXPECT_CALL(*pConfigManager, GetPolicyOfLocalNumbers).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::POLICY_OF_LOCAL_NUMBERS));

    EXPECT_CALL(*pConfigManager, GetSilentRedialInterval).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::SILENT_REDIAL_INTERVAL));

    EXPECT_CALL(*pConfigManager, GetCallTypeAfterAudioAndVideoCallMerged).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::CALL_TYPE_AFTER_AUDIO_AND_VIDEO_CALL_MERGED));

    EXPECT_CALL(*pConfigManager, GetSilentRedialMaxRetryCount).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::SILENT_REDIAL_MAX_RETRY_COUNT));

    EXPECT_CALL(*pConfigManager, GetPolicyFor403ResponseForInvite).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::POLICY_FOR_403_RESPONSE_FOR_INVITE));

    EXPECT_CALL(*pConfigManager, GetPolicyForCheckingQosWhileCallUpgrading)
            .WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING));

    EXPECT_CALL(*pConfigManager, GetCallMaxCount).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::CALL_MAX_COUNT));

    EXPECT_CALL(*pConfigManager, GetConvertRemoteResponseTimer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::CONVERT_REMOTE_RESPONSE_TIMER));

    EXPECT_CALL(*pConfigManager, GetConvertUserResponseTimer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::CONVERT_USER_RESPONSE_TIMER));

    EXPECT_CALL(*pConfigManager, GetPolicyOnVideoQosDeactivation).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::POLICY_ON_VIDEO_QOS_DEACTIVATION));

    EXPECT_CALL(*pConfigManager, GetPolicyForTextWithVideo).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::POLICY_FOR_TEXT_WITH_VIDEO));

    EXPECT_CALL(*pConfigManager, GetMinimumBatteryLevelForLimitVideoCall).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::MINIMUM_BATTERY_LEVEL_FOR_LIMIT_VIDEO_CALL));

    EXPECT_CALL(*pConfigManager, GetPolicyOnTextQosDeactivation).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::POLICY_ON_TEXT_QOS_DEACTIVATION));

    EXPECT_CALL(*pConfigManager, GetEmergencyTCallTimer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::EMERGENCY_T_CALL_TIMER));

    EXPECT_CALL(*pConfigManager, GetEmergencyRingbackTimer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::EMERGENCY_RINGBACK_TIMER));

    EXPECT_CALL(*pConfigManager, GetEmergency18xTimer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::EMERGENCY_18X_TIMER));

    EXPECT_CALL(*pConfigManager, GetPolicyForEmergencyUrnEscvMapping).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::POLICY_FOR_EMERGENCY_URN_ESCV_MAPPING));

    EXPECT_CALL(*pConfigManager, GetConferenceDropReferToUriSourceType).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::CONFERENCE_DROP_REFER_TO_URI_SOURCE_TYPE));

    EXPECT_CALL(*pConfigManager, GetMediaTypeForOfferlessInvite).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::MEDIA_TYPE_FOR_OFFERLESS_INVITE));

    EXPECT_CALL(*pConfigManager, GetMediaTypeForOfferlessReinvite).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::MEDIA_TYPE_FOR_OFFERLESS_REINVITE));

    EXPECT_CALL(*pConfigManager, GetOipTypeForUnavailable).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::OIP_TYPE_FOR_UNAVAILABLE));

    EXPECT_CALL(*pConfigManager, GetDelayUpdateAfterConnectedTimer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::DELAY_UPDATE_AFTER_CONNECTED_TIMER));

    EXPECT_CALL(*pConfigManager, GetEmergencyRttGuardTimer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::EMERGENCY_RTT_GUARD_TIMER));

    EXPECT_CALL(*pConfigManager, GetPreAlertingTimer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::PRE_ALERTING_TIMER));

    EXPECT_CALL(*pConfigManager, GetPolicyForTcallTimerExpiryOfVolteCall).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL));

    EXPECT_CALL(*pConfigManager, GetPolicyForTcallTimerExpiryOfVolteEmergencyCall)
            .WillOnce(Return(nValue));
    EXPECT_EQ(nValue,
            pConfig->GetInt(Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_EMERGENCY_CALL));

    EXPECT_CALL(*pConfigManager, GetPolicyForTcallTimerExpiryOfVowifiCall).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL));

    EXPECT_CALL(*pConfigManager, GetWifiEmergency18xTimer).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::WIFI_EMERGENCY_18X_TIMER));

    EXPECT_CALL(*pConfigManager, GetMaximumWaitTimerForGeolocationPidfInfo)
            .WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::MAXIMUM_WAIT_TIMER_FOR_GEOLOCATION_PIDF_INFO));

    EXPECT_CALL(*pConfigManager, GetPolicyForLocalRingbackToneWith180Response)
            .WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE));

    EXPECT_CALL(*pConfigManager, GetEpsFallbackWatchdogTime).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::EPS_FALLBACK_WATCHDOG_TIME));

    EXPECT_CALL(*pConfigManager, GetSendUdpKeepAliveIntervalTime).WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::SEND_UDP_KEEP_ALIVE_INTERVAL_TIME));

    EXPECT_CALL(*pConfigManager, GetCallRejectCodeForNotAcceptableCallType)
            .WillOnce(Return(nValue));
    EXPECT_EQ(nValue, pConfig->GetInt(Feature::CALL_REJECT_CODE_FOR_NOT_ACCEPTABLE_CALL_TYPE));
}

TEST_F(MtcConfigurationProxyTest, GetIntWithBoolArgReturnsFromConfigManager)
{
    const IMS_SINT32 nValue = 1;
    const IMS_BOOL bArg = IMS_TRUE;

    EXPECT_CALL(*pConfigManager, GetEmergencyRegistrationTo18xTimer(bArg)).WillOnce(Return(nValue));
    EXPECT_EQ(
            nValue, pConfig->GetInt(Feature::EMERGENCY_REGISTRATION_TO_18X_TIMER_MILLIS_INT, bArg));
}

TEST_F(MtcConfigurationProxyTest, GetIntWith3BoolArgReturnsFromConfigManager)
{
    const IMS_SINT32 nValue = 1;
    const IMS_BOOL bArg1 = IMS_FALSE;
    const IMS_BOOL bArg2 = IMS_TRUE;
    const IMS_BOOL bArg3 = IMS_TRUE;

    EXPECT_CALL(*pConfigManager, GetInformationLevelOfGeolocationPidf(bArg1, bArg2, bArg3))
            .WillOnce(Return(nValue));
    EXPECT_EQ(nValue,
            pConfig->GetInt(Feature::INFORMATION_LEVEL_OF_GEOLOCATION_PIDF, bArg1, bArg2, bArg3));
}

TEST_F(MtcConfigurationProxyTest, GetStrWithIntArgReturnsFromConfigManager)
{
    const AString strValue = "some_value";
    const IMS_SINT32 nArg = 1;

    EXPECT_CALL(*pConfigManager, GetConferenceFactoryUri).WillOnce(Return(strValue));
    EXPECT_EQ(strValue, pConfig->GetStr(Feature::CONFERENCE_FACTORY_URI, nArg));

    EXPECT_CALL(*pConfigManager, GetCallTerminateReasonHeader(static_cast<TerminateType>(nArg)))
            .WillOnce(Return(strValue));
    EXPECT_EQ(strValue, pConfig->GetStr(Feature::CALL_TERMINATE_REASON_HEADER, nArg));

    EXPECT_CALL(*pConfigManager, GetCallRejectReasonPhrase(static_cast<RejectType>(nArg)))
            .WillOnce(Return(strValue));
    EXPECT_EQ(strValue, pConfig->GetStr(Feature::CALL_REJECT_REASON_PHRASE, nArg));

    EXPECT_CALL(*pConfigManager, GetPEmergencyInfoHeaderInInvite()).WillOnce(Return(strValue));
    EXPECT_EQ(strValue, pConfig->GetStr(Feature::P_EMERGENCY_INFO_HEADER_IN_INVITE, nArg));
}

TEST_F(MtcConfigurationProxyTest, ReturnsDefaultValueForInvalidFeature)
{
    const IMS_BOOL bDefaultValue = IMS_FALSE;
    const IMS_SINT32 nDefaultValue = 0;
    const AString strDefaultValue = AString::ConstNull();

    EXPECT_EQ(bDefaultValue, pConfig->Is(Feature::CONFERENCE_FACTORY_URI));
    EXPECT_EQ(bDefaultValue, pConfig->Is(Feature::CONFERENCE_FACTORY_URI, ""));
    EXPECT_EQ(bDefaultValue, pConfig->Is(Feature::CONFERENCE_FACTORY_URI, 0));
    EXPECT_EQ(nDefaultValue, pConfig->GetInt(Feature::SUPPORT_EARLY_SESSION));
    EXPECT_EQ(nDefaultValue,
            pConfig->GetInt(Feature::SUPPORT_EARLY_SESSION, IMS_FALSE, IMS_FALSE, IMS_FALSE));
    EXPECT_EQ(strDefaultValue, pConfig->GetStr(Feature::SUPPORT_EARLY_SESSION, 0));
}

TEST_F(MtcConfigurationProxyTest, GetBooleanCacheIfExists)
{
    const Feature eFeature = Feature::SUPPORT_SIP_SESSION_ID_HEADER;
    const IMS_BOOL bValue = IMS_TRUE;
    pConfig->PutConfigCache(eFeature, bValue);

    EXPECT_EQ(bValue, pConfig->Is(eFeature));
}

TEST_F(MtcConfigurationProxyTest, GetIntegerCacheIfExists)
{
    const Feature eFeature = Feature::SUPPORT_SIP_SESSION_ID_HEADER;
    const IMS_SINT32 nValue = 1;
    pConfig->PutConfigCache(eFeature, nValue);

    EXPECT_EQ(nValue, pConfig->GetInt(eFeature));
}

TEST_F(MtcConfigurationProxyTest, GetStringCacheIfExists)
{
    const Feature eFeature = Feature::SUPPORT_SIP_SESSION_ID_HEADER;
    const AString strValue("some_str");
    pConfig->PutConfigCache(eFeature, strValue);

    EXPECT_EQ(strValue, pConfig->GetStr(eFeature, 0));
}

TEST_F(MtcConfigurationProxyTest, OnRegistrationRefreshedResetsCache)
{
    const Feature eFeature = Feature::REQUEST_URI_TYPE;
    const IMS_SINT32 nValue = 2;
    ON_CALL(*pConfigManager, GetRequestUriType).WillByDefault(Return(nValue));

    const IMS_SINT32 nCachedValue = 1;
    pConfig->PutConfigCache(eFeature, nCachedValue);

    pConfig->OnRegistrationRefreshed();
    EXPECT_EQ(nValue, pConfig->GetInt(eFeature));
}
