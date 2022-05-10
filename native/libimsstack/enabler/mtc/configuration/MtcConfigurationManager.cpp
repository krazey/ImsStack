#include "configuration/MtcConfigurationManager.h"
#include "configuration/MtcConfigurationUpdater.h"
#include "ServiceTrace.h"
#include "ServiceThread.h"
#include "ServiceConfig.h"
#include "ICarrierConfig.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcConfigurationManager::MtcConfigurationManager() :
        m_objCarrierConfig(CarrierConfigItems()),
        m_objAsset(AssetItems())
{
}

PUBLIC
MtcConfigurationManager::~MtcConfigurationManager()
{
    ICarrierConfig* piCc =
            ConfigService::GetConfigService()->GetCarrierConfig(ThreadService::GetCurrentSlotId());
    piCc->RemoveListener(this);
}

PUBLIC
void MtcConfigurationManager::Init()
{
    IMS_TRACE_I("Init", 0, 0, 0);
    ICarrierConfig* piCc =
            ConfigService::GetConfigService()->GetCarrierConfig(ThreadService::GetCurrentSlotId());
    piCc->AddListener(this);

    UpdateFullConfig(piCc);
}

PUBLIC
void MtcConfigurationManager::UpdateFullConfig(ICarrierConfig* piCc)
{
    IMS_TRACE_I("UpdateFullConfig", 0, 0, 0);
    MtcConfigurationUpdater::Update(piCc, m_objCarrierConfig, m_objAsset);
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
    for (IMS_UINT32 i = 0; i < m_objCarrierConfig.objSrvccTypes.GetSize(); i++)
    {
        if (m_objCarrierConfig.objSrvccTypes.GetAt(i) == nType)
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
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
const AString MtcConfigurationManager::GetConferenceFactoryUri() const
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
    for (IMS_UINT32 i = 0; i < m_objCarrierConfig.objShortCallCodes.GetSize(); i++)
    {
        if (m_objCarrierConfig.objShortCallCodes.GetAt(i) == nCode)
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsValidateVerstatFeatureInRegistrationToCheckNetworkCapability()
        const
{
    return m_objCarrierConfig.bValidateVerstatFeatureInRegistrationToCheckNetworkCapability;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsAllowMultipleCallIncludingVideoCall() const
{
    return m_objCarrierConfig.bAllowMultipleCallIncludingVideoCall;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsRejectCodeForCsfb(IN IMS_SINT32 nCode) const
{
    for (IMS_UINT32 i = 0; i < m_objCarrierConfig.objRejectCodeForCsfbs.GetSize(); i++)
    {
        if (m_objCarrierConfig.objRejectCodeForCsfbs.GetAt(i) == nCode)
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
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
const AString MtcConfigurationManager::GetCallTerminateReasonHeader(IN TerminateType eType) const
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
const AString MtcConfigurationManager::GetCallRejectReasonPhrase(IN RejectType eType) const
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
IMS_BOOL MtcConfigurationManager::IsAllowTextWithVideo() const
{
    return m_objCarrierConfig.bAllowTextWithVideo;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetMinimumBatteryLevelForLimitVideoCall() const
{
    return m_objCarrierConfig.nMinimumBatteryLevelForLimitVideoCall;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSupportVideoTextFeatureInContactHeaderSimultaneously() const
{
    return m_objCarrierConfig.bSupportVideoTextFeatureInContactHeaderSimultaneously;
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
IMS_BOOL MtcConfigurationManager::IsPidfShortCode(const AString& strCode) const
{
    for (IMS_UINT32 i = 0; i < m_objCarrierConfig.objPidfShortCodes.GetSize(); i++)
    {
        if (m_objCarrierConfig.objPidfShortCodes.GetAt(i).Equals(strCode))
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
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
    return m_objAsset.bCheckConferenceEventPackageVersion;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsConferenceReferToUriSourcePaid() const
{
    return m_objAsset.bConferenceReferToUriSourcePaid;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetConferenceDropReferToUriSourceType() const
{
    return m_objAsset.nConferenceDropReferToUriSourceType;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsEnableFakeQosCallFlowOnWifi() const
{
    return m_objAsset.bEnableFakeQosCallFlowOnWifi;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetMediaTypeForOfferlessReinvite() const
{
    return m_objAsset.nMediaTypeForOfferlessReinvite;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSupportVideoCallUpgradeRegardlessOfFeatureTags() const
{
    return m_objAsset.bSupportVideoCallUpgradeRegardlessOfFeatureTags;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetOipTypeForUnavailable() const
{
    return m_objAsset.nOipTypeForUnavailable;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetEmergencyRttGuardTimer() const
{
    return m_objAsset.nEmergencyRttGuardTimer;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf() const
{
    return m_objAsset.bRetryEmergencyCallOverEmergencyPdnWithNextPcscf;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPreAlertingTimer() const
{
    return m_objAsset.nPreAlertingTimer;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyForTcallTimerExpiryOfVolteCall() const
{
    return m_objAsset.nPolicyForTcallTimerExpiryOfVolteCall;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyForTcallTimerExpiryOfVolteEmergencyCall() const
{
    return m_objAsset.nPolicyForTcallTimerExpiryOfVolteEmergencyCall;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetPolicyForTcallTimerExpiryOfVowifiCall() const
{
    return m_objAsset.nPolicyForTcallTimerExpiryOfVowifiCall;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsCarrierSpecificSipHeader(IN const AString& strHeader) const
{
    for (IMS_UINT32 i = 0; i < m_objAsset.objCarrierSpecificSipHeaders.GetSize(); i++)
    {
        if (m_objAsset.objCarrierSpecificSipHeaders.GetAt(i).Equals(strHeader))
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsCheckAvchangeFeatureForCallConvertingCapability() const
{
    return m_objAsset.bCheckAvchangeFeatureForCallConvertingCapability;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSupportRegistrationRecoveryForFailureOfSessionRefresh() const
{
    return m_objAsset.bSupportRegistrationRecoveryForFailureOfSessionRefresh;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsCallMaintainingOnRegistrationSuspended(
        IN IMS_SINT32 nSuspendType) const
{
    for (IMS_UINT32 i = 0; i < m_objAsset.objCallMaintainingOnRegistrationSuspendeds.GetSize(); i++)
    {
        if (m_objAsset.objCallMaintainingOnRegistrationSuspendeds.GetAt(i) == nSuspendType)
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsRequiringEmergencyCallWhenVideoEmergencyCallFailed(
        IN IMS_SINT32 nCode) const
{
    for (IMS_UINT32 i = 0;
            i < m_objAsset.objRequiringEmergencyCallWhenVideoEmergencyCallFaileds.GetSize(); i++)
    {
        if (m_objAsset.objRequiringEmergencyCallWhenVideoEmergencyCallFaileds.GetAt(i) == nCode)
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsUseMcidSupplementaryService() const
{
    return m_objAsset.bUseMcidSupplementaryService;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsUseMmcSupplementaryService() const
{
    return m_objAsset.bUseMmcSupplementaryService;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsUseLtePreferredStatusForServiceCapability() const
{
    return m_objAsset.bUseLtePreferredStatusForServiceCapability;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsAllowIncomingHoldRequestDuringConferenceCall() const
{
    return m_objAsset.bAllowIncomingHoldRequestDuringConferenceCall;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsIgnore180After183Response() const
{
    return m_objAsset.bIgnore180After183Response;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsAddReplaceHeaderForConference() const
{
    return m_objAsset.bAddReplaceHeaderForConference;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsVilteToVolteRetryFailureResponseCode(IN IMS_SINT32 nCode) const
{
    for (IMS_UINT32 i = 0; i < m_objAsset.objVilteToVolteRetryFailureResponseCodes.GetSize(); i++)
    {
        if (m_objAsset.objVilteToVolteRetryFailureResponseCodes.GetAt(i) == nCode)
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsUseEmergencyNumberTranslationInRoamingStatus() const
{
    return m_objAsset.bUseEmergencyNumberTranslationInRoamingStatus;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsIgnorePrackDeliveryFailure() const
{
    return m_objAsset.bIgnorePrackDeliveryFailure;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSupportVideoCallOnlyInVopsOffStatus() const
{
    return m_objAsset.bSupportVideoCallOnlyInVopsOffStatus;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsBlockWifiEmergencyCallIfNotProvisioned() const
{
    return m_objAsset.bBlockWifiEmergencyCallIfNotProvisioned;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsRegistrationDisconnectReasonToTerminateOngoingCall(
        IN IMS_SINT32 nReason) const
{
    for (IMS_UINT32 i = 0;
            i < m_objAsset.objRegistrationDisconnectReasonToTerminateOngoingCalls.GetSize(); i++)
    {
        if (m_objAsset.objRegistrationDisconnectReasonToTerminateOngoingCalls.GetAt(i) == nReason)
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetWifiEmergency18xTimer() const
{
    return m_objAsset.nWifiEmergency18xTimer;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSupportCanidInfo() const
{
    return m_objAsset.bSupportCanidInfo;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsUseCarrierSpecificContactHeaderForOptionsResponse() const
{
    return m_objAsset.bUseCarrierSpecificContactHeaderForOptionsResponse;
}

PUBLIC
IMS_BOOL
MtcConfigurationManager::IsUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration() const
{
    return m_objAsset.bUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsEnableRegistrationRecoveryWhenCallRejectedByServerError() const
{
    return m_objAsset.bEnableRegistrationRecoveryWhenCallRejectedByServerError;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsEnableRegistrationRecoveryWhenCallRetryUnavailable() const
{
    return m_objAsset.bEnableRegistrationRecoveryWhenCallRetryUnavailable;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsRejectVowifiVoiceCallWhenVowifiSettingOff() const
{
    return m_objAsset.bRejectVowifiVoiceCallWhenVowifiSettingOff;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsCheckServerOutageReasonForVxlteCall() const
{
    return m_objAsset.bCheckServerOutageReasonForVxlteCall;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType()
        const
{
    return m_objAsset.bSetVideoTextFeatureExclusivelyInContactHeaderBySessionType;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetMaximumWaitTimerForGeolocationPidfInfo() const
{
    return m_objAsset.nMaximumWaitTimerForGeolocationPidfInfo;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsMaintainMultipleEarlySessionsByForking() const
{
    return m_objAsset.bMaintainMultipleEarlySessionsByForking;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsStopRingbackTimerBy183WithSdpBody() const
{
    return m_objAsset.bStopRingbackTimerBy183WithSdpBody;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsEnableVoiceMailServiceByPaidHeader() const
{
    return m_objAsset.bEnableVoiceMailServiceByPaidHeader;
}

PUBLIC
IMS_BOOL MtcConfigurationManager::IsIgnorePemHeader() const
{
    return m_objAsset.bIgnorePemHeader;
}

PUBLIC
IMS_SINT32 MtcConfigurationManager::GetInformationLevelOfGeolocationPidf(
        IN IMS_BOOL bEmergency, IN IMS_BOOL bWifi) const
{
    if (bEmergency && !bWifi)
    {
        return m_objAsset.objInformationLevelOfGeolocationPidfs.GetAt(0);
    }
    else if (bEmergency && bWifi)
    {
        return m_objAsset.objInformationLevelOfGeolocationPidfs.GetAt(1);
    }
    else if (!bEmergency && !bWifi)
    {
        return m_objAsset.objInformationLevelOfGeolocationPidfs.GetAt(2);
    }
    else  // if (!bEmergency && bWifi)
    {
        return m_objAsset.objInformationLevelOfGeolocationPidfs.GetAt(3);
    }
}
