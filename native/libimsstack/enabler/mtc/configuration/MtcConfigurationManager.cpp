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

#include "CarrierConfig.h"
#include "ICarrierConfig.h"
#include "ImsVector.h"
#include "ServiceConfig.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "configuration/MtcConfigurationManager.h"
#include "configuration/MtcConfigurationUpdater.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcConfigurationManager::MtcConfigurationManager() :
        m_objCarrierConfig(CarrierConfigItems())
{
}

PUBLIC VIRTUAL MtcConfigurationManager::~MtcConfigurationManager()
{
    ICarrierConfig* piCc =
            ConfigService::GetConfigService()->GetCarrierConfig(ThreadService::GetCurrentSlotId(0));
    piCc->RemoveListener(this);
}

PUBLIC
void MtcConfigurationManager::Init()
{
    IMS_TRACE_I("Init", 0, 0, 0);
    ICarrierConfig* piCc =
            ConfigService::GetConfigService()->GetCarrierConfig(ThreadService::GetCurrentSlotId(0));
    piCc->AddListener(this);

    UpdateFullConfig(piCc);
}

PUBLIC
void MtcConfigurationManager::UpdateFullConfig(ICarrierConfig* piCc)
{
    IMS_TRACE_I("UpdateFullConfig", 0, 0, 0);
    MtcConfigurationUpdater::Update(piCc, m_objCarrierConfig);
}

PUBLIC VIRTUAL void MtcConfigurationManager::CarrierConfig_NotifyConfigChanged(
        IN IMS_SINT32 nSlotId)
{
    if (nSlotId == ThreadService::GetCurrentSlotId())
    {
        ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(
                ThreadService::GetCurrentSlotId());
        UpdateFullConfig(piCc);
    }
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetRequestUriType() const
{
    return m_objCarrierConfig.nRequestUriType;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSupportGeolocationPidfInSipInvite(IN IMS_SINT32 nType) const
{
    return ContainsValue(m_objCarrierConfig.objSupportGeolocationPidfInSipInvite, nType);
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSupportSipSessionIdHeader() const
{
    return m_objCarrierConfig.bSupportSipSessionIdHeader;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsIncludeCallerIdServiceCodesInSipInvite() const
{
    return m_objCarrierConfig.bIncludeCallerIdServiceCodesInSipInvite;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsMultiendpointSupported() const
{
    return m_objCarrierConfig.bMultiendpointSupported;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSessionTimerSupported() const
{
    return m_objCarrierConfig.bSessionTimerSupported;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetSessionPrivacyType() const
{
    return m_objCarrierConfig.nSessionPrivacyType;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsPrackSupportedFor18x() const
{
    return m_objCarrierConfig.bPrackSupportedFor18x;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetConferenceSubscribeType() const
{
    // + KEY_SUPPORT_IMS_CONFERENCE_EVENT_PACKAGE_BOOL
    return m_objCarrierConfig.nConferenceSubscribeType;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsVoiceQosPreconditionSupported() const
{
    return m_objCarrierConfig.bVoiceQosPreconditionSupported;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsVoiceOnDefaultBearerSupported() const
{
    return m_objCarrierConfig.bVoiceOnDefaultBearerSupported;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetDedicatedBearerWaitTimer() const
{
    return m_objCarrierConfig.nDedicatedBearerWaitTimer;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSrvccType(IN IMS_SINT32 nType) const
{
    return ContainsValue(m_objCarrierConfig.objSrvccTypes, nType);
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetRingingTimer() const
{
    return m_objCarrierConfig.nRingingTimer;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetRingbackTimer() const
{
    return m_objCarrierConfig.nRingbackTimer;
}

PUBLIC
AString MtcConfigurationManager::GetConferenceFactoryUri() const
{
    return m_objCarrierConfig.strConferenceFactoryUri;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsOipSourceFromHeader() const
{
    return m_objCarrierConfig.bOipSourceFromHeader;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetMoCallRequestTimeout() const
{
    return m_objCarrierConfig.nMoCallRequestTimeout;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsAudioInactivityCallEndReason(IN IMS_SINT32 nReason) const
{
    return ContainsValue(m_objCarrierConfig.objAudioInactivityCallEndReasons, nReason);
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::Get18xTimer() const
{
    return m_objCarrierConfig.n18xTimer;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSupportConferenceReferSubscribe() const
{
    return m_objCarrierConfig.bSupportConferenceReferSubscribe;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsEnableConferenceSubscribeByParticipant() const
{
    // KEY_SUPPORT_IMS_CONFERENCE_EVENT_PACKAGE_ON_PEER_BOOL
    return m_objCarrierConfig.bEnableConferenceSubscribeByParticipant;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetConferenceSipFlowOrder() const
{
    return m_objCarrierConfig.nConferenceSipFlowOrder;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetConferenceInvitingReferType() const
{
    return m_objCarrierConfig.nConferenceInvitingReferType;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyQosPreconditionMechanismWhileCallModification() const
{
    return m_objCarrierConfig.nPolicyQosPreconditionMechanismWhileCallModification;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetIncomingCallRejectCodeForUserDecline() const
{
    return m_objCarrierConfig.nIncomingCallRejectCodeForUserDecline;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetIncomingCallRejectCodeForNoAnswer() const
{
    return m_objCarrierConfig.nIncomingCallRejectCodeForNoAnswer;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPrackUpdateResponseWaitTimer() const
{
    return m_objCarrierConfig.nPrackUpdateResponseWaitTimer;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetSessionRefreshTriggerInterval() const
{
    return m_objCarrierConfig.nSessionRefreshTriggerInterval;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetRegistrationRestorationModeOn504ForInvite() const
{
    return m_objCarrierConfig.nRegistrationRestorationModeOn504ForInvite;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyOnAudioQosDeactivation() const
{
    return m_objCarrierConfig.nPolicyOnAudioQosDeactivation;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsEnableSendReinviteOnRatChange() const
{
    return m_objCarrierConfig.bEnableSendReinviteOnRatChange;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyForMediaTypeRestrictionOnCellular() const
{
    return m_objCarrierConfig.nPolicyForMediaTypeRestrictionOnCellular;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyForMediaTypeRestrictionOnCellularInRoaming() const
{
    return m_objCarrierConfig.nPolicyForMediaTypeRestrictionOnCellularInRoaming;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyOfLocalNumbers() const
{
    return m_objCarrierConfig.nPolicyOfLocalNumbers;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsDefaultEpsBearerContextUsageRestrictionOnCellular() const
{
    return m_objCarrierConfig.bDefaultEpsBearerContextUsageRestrictionOnCellular;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetSilentRedialInterval() const
{
    return m_objCarrierConfig.nSilentRedialInterval;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetCallTypeAfterAudioAndVideoCallMerged() const
{
    return m_objCarrierConfig.nCallTypeAfterAudioAndVideoCallMerged;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsShortCallCode(IN IMS_SINT32 nCode) const
{
    return ContainsValue(m_objCarrierConfig.objShortCallCodes, nCode);
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsAllowMultipleCallIncludingVideoCall() const
{
    return m_objCarrierConfig.bAllowMultipleCallIncludingVideoCall;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsRejectCodeForCsfb(IN IMS_SINT32 nCode) const
{
    return ContainsValue(m_objCarrierConfig.objRejectCodeForCsfbs, nCode);
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetSilentRedialMaxRetryCount() const
{
    return m_objCarrierConfig.nSilentRedialMaxRetryCount;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyFor403ResponseForInvite() const
{
    return m_objCarrierConfig.nPolicyFor403ResponseForInvite;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyForCheckingQosWhileCallUpgrading() const
{
    return m_objCarrierConfig.nPolicyForCheckingQosWhileCallUpgrading;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsRejectOfferlessInvite() const
{
    return m_objCarrierConfig.bRejectOfferlessInvite;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetCallMaxCount() const
{
    return m_objCarrierConfig.nCallMaxCount;
}

PUBLIC
AString MtcConfigurationManager::GetCallTerminateReasonHeader(IN TerminateType eType) const
{
    IMS_UINT32 nIndex = static_cast<IMS_UINT32>(eType);
    if (nIndex >= m_objCarrierConfig.objCallTerminateReasonHeaders.GetSize())
    {
        IMS_TRACE_E(0, "invalid type", 0, 0, 0);
        return AString::ConstNull();
    }

    return m_objCarrierConfig.objCallTerminateReasonHeaders.GetAt(nIndex);
}

PUBLIC
AString MtcConfigurationManager::GetCallRejectReasonPhrase(IN RejectType eType) const
{
    IMS_UINT32 nIndex = static_cast<IMS_UINT32>(eType);
    if (nIndex >= m_objCarrierConfig.objCallRejectReasonPhrases.GetSize())
    {
        IMS_TRACE_E(0, "invalid type", 0, 0, 0);
        return AString::ConstNull();
    }

    return m_objCarrierConfig.objCallRejectReasonPhrases.GetAt(nIndex);
}

// vt configurations
PUBLIC
IMS_BOOL MtcConfigurationManager::IsVideoOnDefaultBearerSupported() const
{
    return m_objCarrierConfig.bVideoOnDefaultBearerSupported;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsVideoQosPreconditionSupported() const
{
    return m_objCarrierConfig.bVideoQosPreconditionSupported;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetConvertRemoteResponseTimer() const
{
    return m_objCarrierConfig.nConvertRemoteResponseTimer;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetConvertUserResponseTimer() const
{
    return m_objCarrierConfig.nConvertUserResponseTimer;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyOnVideoQosDeactivation() const
{
    return m_objCarrierConfig.nPolicyOnVideoQosDeactivation;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSupportEarlySession() const
{
    return m_objCarrierConfig.bSupportEarlySession;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyForTextWithVideo() const
{
    return m_objCarrierConfig.nPolicyForTextWithVideo;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetMinimumBatteryLevelForLimitVideoCall() const
{
    return m_objCarrierConfig.nMinimumBatteryLevelForLimitVideoCall;
}

// rtt configurations
PUBLIC
IMS_BOOL MtcConfigurationManager::IsTextOnDefaultBearerSupported() const
{
    return m_objCarrierConfig.bTextOnDefaultBearerSupported;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsTextQosPreconditionSupported() const
{
    return m_objCarrierConfig.bTextQosPreconditionSupported;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyOnTextQosDeactivation() const
{
    return m_objCarrierConfig.nPolicyOnTextQosDeactivation;
}

// wfc configurations
PUBLIC
IMS_BOOL MtcConfigurationManager::IsPidfShortCode(IN const AString& strCode) const
{
    return ContainsValue(m_objCarrierConfig.objPidfShortCodes, strCode);
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsEmergencyCallOverEmergencyPdn() const  // wifi
{
    return m_objCarrierConfig.bEmergencyCallOverEmergencyPdn;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetCountryCode() const
{
    return m_objCarrierConfig.nCountryCode;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsRetryEmergencyOnImsPdnBool() const
{
    return m_objCarrierConfig.bRetryEmergencyOnImsPdnBool;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsEmergencyQosPreconditionSupported() const
{
    return m_objCarrierConfig.bEmergencyQosPreconditionSupported;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsEmergencyCallOverEmergencyPdnOnCellular() const
{
    return m_objCarrierConfig.bEmergencyCallOverEmergencyPdnOnCellular;
}

PUBLIC
IMS_BOOL
MtcConfigurationManager::IsEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall()
        const
{
    return m_objCarrierConfig
            .bEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetEmergencyTCallTimer() const
{
    return m_objCarrierConfig.nEmergencyTCallTimer;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetEmergencyRingbackTimer() const
{
    return m_objCarrierConfig.nEmergencyRingbackTimer;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetEmergency18xTimer() const
{
    return m_objCarrierConfig.nEmergency18xTimer;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyForEmergencyUrnEscvMapping() const
{
    return m_objCarrierConfig.nPolicyForEmergencyUrnEscvMapping;
}

// ASSETs
PUBLIC
IMS_BOOL MtcConfigurationManager::IsCheckConferenceEventPackageVersion() const
{
    return m_objCarrierConfig.bCheckConferenceEventPackageVersion;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsConferenceReferToUriSourcePaid() const
{
    return m_objCarrierConfig.bConferenceReferToUriSourcePaid;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetConferenceDropReferToUriSourceType() const
{
    return m_objCarrierConfig.nConferenceDropReferToUriSourceType;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsEnableFakeQosCallFlowOnWifi() const
{
    return m_objCarrierConfig.bEnableFakeQosCallFlowOnWifi;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetMediaTypeForOfferlessReinvite() const
{
    return m_objCarrierConfig.nMediaTypeForOfferlessReinvite;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSupportVideoCallUpgradeRegardlessOfFeatureTags() const
{
    return m_objCarrierConfig.bSupportVideoCallUpgradeRegardlessOfFeatureTags;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetOipTypeForUnavailable() const
{
    return m_objCarrierConfig.nOipTypeForUnavailable;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsEnableOipHeaderPolicyFallBack() const
{
    return m_objCarrierConfig.bEnableOipHeaderPolicyFallBack;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetEmergencyRttGuardTimer() const
{
    return m_objCarrierConfig.nEmergencyRttGuardTimer;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf() const
{
    return m_objCarrierConfig.bRetryEmergencyCallOverEmergencyPdnWithNextPcscf;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPreAlertingTimer() const
{
    return m_objCarrierConfig.nPreAlertingTimer;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyForTcallTimerExpiryOfVolteCall() const
{
    return m_objCarrierConfig.nPolicyForTcallTimerExpiryOfVolteCall;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyForTcallTimerExpiryOfVolteEmergencyCall() const
{
    return m_objCarrierConfig.nPolicyForTcallTimerExpiryOfVolteEmergencyCall;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyForTcallTimerExpiryOfVowifiCall() const
{
    return m_objCarrierConfig.nPolicyForTcallTimerExpiryOfVowifiCall;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsCarrierSpecificSipHeader(IN const AString& strHeader) const
{
    return ContainsValue(m_objCarrierConfig.objCarrierSpecificSipHeaders, strHeader);
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsCheckAvchangeFeatureForCallConvertingCapability() const
{
    return m_objCarrierConfig.bCheckAvchangeFeatureForCallConvertingCapability;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSupportRegistrationRecoveryForFailureOfSessionRefresh() const
{
    return m_objCarrierConfig.bSupportRegistrationRecoveryForFailureOfSessionRefresh;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsCallMaintainingOnRegistrationSuspended(
        IN IMS_SINT32 nSuspendType) const
{
    return ContainsValue(
            m_objCarrierConfig.objCallMaintainingOnRegistrationSuspendeds, nSuspendType);
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsRequiringEmergencyCallWhenVideoEmergencyCallFailed(
        IN IMS_SINT32 nCode) const
{
    return ContainsValue(
            m_objCarrierConfig.objRequiringEmergencyCallWhenVideoEmergencyCallFaileds, nCode);
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsUseMcidSupplementaryService() const
{
    return m_objCarrierConfig.bUseMcidSupplementaryService;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsUseMmcSupplementaryService() const
{
    return m_objCarrierConfig.bUseMmcSupplementaryService;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsUseLtePreferredStatusForServiceCapability() const
{
    return m_objCarrierConfig.bUseLtePreferredStatusForServiceCapability;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsAllowIncomingHoldRequestDuringConferenceCall() const
{
    return m_objCarrierConfig.bAllowIncomingHoldRequestDuringConferenceCall;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsIgnore180After183Response() const
{
    return m_objCarrierConfig.bIgnore180After183Response;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsAddReplaceHeaderForConference() const
{
    return m_objCarrierConfig.bAddReplaceHeaderForConference;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsVilteToVolteRetryFailureResponseCode(IN IMS_SINT32 nCode) const
{
    return ContainsValue(m_objCarrierConfig.objVilteToVolteRetryFailureResponseCodes, nCode);
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsUseEmergencyNumberTranslationInRoamingStatus() const
{
    return m_objCarrierConfig.bUseEmergencyNumberTranslationInRoamingStatus;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsIgnorePrackDeliveryFailure() const
{
    return m_objCarrierConfig.bIgnorePrackDeliveryFailure;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSupportVideoCallOnlyInVopsOffStatus() const
{
    return m_objCarrierConfig.bSupportVideoCallOnlyInVopsOffStatus;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsBlockWifiEmergencyCallIfNotProvisioned() const
{
    return m_objCarrierConfig.bBlockWifiEmergencyCallIfNotProvisioned;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsRegistrationDisconnectReasonToIgnore(
        IN IMS_SINT32 nReason) const
{
    return ContainsValue(m_objCarrierConfig.objRegistrationDisconnectReasonToIgnore, nReason);
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetWifiEmergency18xTimer() const
{
    return m_objCarrierConfig.nWifiEmergency18xTimer;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSupportCanidInfo() const
{
    return m_objCarrierConfig.bSupportCanidInfo;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsUseCarrierSpecificContactHeaderForOptionsResponse() const
{
    return m_objCarrierConfig.bUseCarrierSpecificContactHeaderForOptionsResponse;
}

PUBLIC
IMS_BOOL
MtcConfigurationManager::IsUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration() const
{
    return m_objCarrierConfig.bUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsEnableRegistrationRecoveryWhenCallRejectedByServerError() const
{
    return m_objCarrierConfig.bEnableRegistrationRecoveryWhenCallRejectedByServerError;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsEnableRegistrationRecoveryWhenCallRetryUnavailable() const
{
    return m_objCarrierConfig.bEnableRegistrationRecoveryWhenCallRetryUnavailable;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsRejectVowifiVoiceCallWhenVowifiSettingOff() const
{
    return m_objCarrierConfig.bRejectVowifiVoiceCallWhenVowifiSettingOff;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsCheckServerOutageReasonForVxlteCall() const
{
    return m_objCarrierConfig.bCheckServerOutageReasonForVxlteCall;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType()
        const
{
    return m_objCarrierConfig.bSetVideoTextFeatureExclusivelyInContactHeaderBySessionType;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetMaximumWaitTimerForGeolocationPidfInfo() const
{
    return m_objCarrierConfig.nMaximumWaitTimerForGeolocationPidfInfo;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsMaintainMultipleEarlySessionsByForking() const
{
    return m_objCarrierConfig.bMaintainMultipleEarlySessionsByForking;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsStopRingbackTimerBy183WithSdpBody() const
{
    return m_objCarrierConfig.bStopRingbackTimerBy183WithSdpBody;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetInformationLevelOfGeolocationPidf(
        IN IMS_BOOL bEmergency, IN IMS_BOOL bWifi, IN IMS_BOOL bShortCode) const
{
    if (bEmergency && !bWifi)
    {
        return m_objCarrierConfig.objInformationLevelOfGeolocationPidfs.GetAt(0);
    }
    else if (bEmergency && bWifi)
    {
        return m_objCarrierConfig.objInformationLevelOfGeolocationPidfs.GetAt(1);
    }
    else if (!bEmergency && !bWifi)
    {
        return m_objCarrierConfig.objInformationLevelOfGeolocationPidfs.GetAt(2);
    }
    else  // if (!bEmergency && bWifi)
    {
        if (bShortCode)
        {
            return CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_COUNTRY_CODE_AND_STATE;
        }
        return m_objCarrierConfig.objInformationLevelOfGeolocationPidfs.GetAt(3);
    }
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsMessageTypeSupportGeolocationPidf(
        IN MessageTypeForGeolocationPidf eType) const
{
    IMS_UINT32 nType = static_cast<IMS_UINT32>(eType);
    return ContainsValue(m_objCarrierConfig.objMessageTypesSupportGeolocationPidf, nType);
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsInitializePemWhenNoHeader() const
{
    return m_objCarrierConfig.bInitializePemWhenNoHeader;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyForLocalRingbackToneWith180Response() const
{
    return m_objCarrierConfig.nPolicyForLocalRingbackToneWith180Response;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetEpsFallbackWatchdogTime() const
{
    return m_objCarrierConfig.nEpsFallbackWatchDogTime;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetSendUdpKeepAliveIntervalTime() const
{
    return m_objCarrierConfig.nSendUdpKeepAliveIntervalTime;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetCallRejectCodeForNotAcceptableCallType() const
{
    return m_objCarrierConfig.nCallRejectCodeForNotAcceptableCallType;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsReleaseEmergencyPdnWithEmergencyCallFail() const
{
    return m_objCarrierConfig.bReleaseEmergencyPdnWithEmergencyCallFail;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyForAlertNotUsingPreconditionMechanism() const
{
    return m_objCarrierConfig.nPolicyForAlertNotUsingPreconditionMechanism;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsRequiredCdmalessFeatureTag() const
{
    return m_objCarrierConfig.bRequiredCdmalessFeatureTag;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsEmergencyCallCurrentLocationDiscoverySupported() const
{
    return m_objCarrierConfig.bEmergencyCallCurrentLocationDiscoverySupported;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsCheckUiConditionForIncomingResume() const
{
    return m_objCarrierConfig.bCheckUiConditionForIncomingResume;
}

PRIVATE
IMS_BOOL MtcConfigurationManager::ContainsValue(
        IN const ImsVector<IMS_SINT32>& lstList, IN IMS_SINT32 nValue)
{
    for (IMS_UINT32 i = 0; i < lstList.GetSize(); i++)
    {
        if (lstList.GetAt(i) == nValue)
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MtcConfigurationManager::ContainsValue(
        IN const ImsVector<AString>& lstList, IN const AString& strValue)
{
    for (IMS_UINT32 i = 0; i < lstList.GetSize(); i++)
    {
        if (lstList.GetAt(i).Equals(strValue))
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}
