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
#ifndef AOS_NCONFIGURATION_H_
#define AOS_NCONFIGURATION_H_

#include "AString.h"

#include "ICarrierConfigListener.h"

#include "interface/IAosNConfiguration.h"
#include "provider/AosAsset.h"
#include "provider/AosAssetBundle.h"
#include "provider/AosCarrierConfig.h"
#include "provider/AosCarrierConfigBundle.h"

class ICarrierConfig;

/**
 * @brief This class is defined as the configuration information allows classes related Aos take.
 *
 *        This configuration is based on carrier and asset configuration
 */
class AosNConfiguration : public ICarrierConfigListener, public IAosNConfiguration
{
public:
    AosNConfiguration();
    ~AosNConfiguration() override;

    IMS_SINT32 GetSlotId() const override;

    void SetListener(IN IAosNConfigurationListener* piListener) override;
    void RemoveListener(IN IAosNConfigurationListener* piListener) override;

    IMS_BOOL IsTcpRequiredForReg() const override;
    IMS_BOOL IsSubscription() const override;
    IMS_BOOL IsUnSubscription() const override;
    IMS_BOOL IsVoLteAvailable() const override;
    IMS_BOOL IsVoLteRoamingAvailable() const override;
    IMS_BOOL IsVtAvailable() const override;
    IMS_BOOL IsWfcImsAvailable() const override;
    IMS_BOOL IsImsSingleRegistrationRequired() const override;
    IMS_BOOL IsRttSupported() const override;
    IMS_BOOL IsRttSupportedWhileRoaming() const override;
    IMS_BOOL IsSupportLimitedAdminSmsMode() const override;
    IMS_BOOL IsNetworkInitiatedUssdOverImsSupported() const override;
    IMS_BOOL IsVolteTtySupported() const override;
    IMS_BOOL IsVopsIgnoredForVolteEnabled() const override;
    IMS_BOOL IsSmsOverImsAvailableWithoutVoiceCapability() const override;
    IMS_BOOL IsAnonymousECallActionSupported() const override;
    IMS_BOOL IsRequiredVolteBlockBySsac() const override;
    IMS_BOOL IsRequiredWfcBlockByAirplaneMode() const override;
    IMS_BOOL IsReregRetryWithChangedCountryOnWifi() const override;
    IMS_BOOL IsSipOverIpsecInRoamingEnabled() const override;
    IMS_BOOL UseWfcCountryCodeAvailabilityCheck() const override;
    IMS_BOOL IsRegRetryIntervalsUsedForSub() const override;
    IMS_BOOL IsSmsOverIpEnabled() const override;
    IMS_BOOL IsIpsecEnabled() const override;
    IMS_BOOL IsRegRetryRuleForERegUsed() const override;
    IMS_BOOL IsSecurityServerPortInRegContactOfInitRegUsed() const override;
    IMS_BOOL IsSecurityServerPortInInitRegUsed() const override;
    IMS_BOOL IsOldSaOnEstablishingSaRemoved() const override;
    IMS_BOOL IsB2cCallComposerFeatureTagInRegContact() const override;
    IMS_BOOL IsBlockNrRatWhenReceive403ForReg() const override;
    IMS_BOOL IsBlockPcscfOnRegFailure() const override;
    IMS_BOOL IsBlockRegOnCsCall() const override;
    IMS_BOOL IsCallEndAndPdnReactivationByRegTerminated() const override;
    IMS_BOOL IsUnsecureTcpSocketOnAccomplishingRegDestroyed() const override;
    IMS_BOOL IsSmsOverImsSupported() const override;
    IMS_BOOL IsImsOverNrEnabled() const override;
    IMS_BOOL IsEmergencyCallBasedOnPauOfNormalRegistrationSupported() const override;
    IMS_BOOL IsEmcRegOnRandomPcscf() const override;
    IMS_BOOL IsERegWithOnlyTcpInRoaming() const override;
    IMS_BOOL IsERegUsingFirstImpuInIsim() const override;
    IMS_BOOL IsEmergencyReregSupportedOnIpcanChange() const override;
    IMS_BOOL IsGibaSupportedForERegInRoaming() const override;
    IMS_BOOL IsRegWithIpcanChangedDuringImsCallHeld() const override;
    IMS_BOOL IsDeregOn3gNetwork() const override;
    IMS_BOOL IsImsiBasedUriPrioritized() const override;
    IMS_BOOL IsIpsecInitializedWithNewPcscf() const override;
    IMS_BOOL IsKeepEPdnUponPcscfUnavailable() const override;
    IMS_BOOL IsKeepERegRetryOnWlanRequired() const override;
    IMS_BOOL IsStopERegTimerOnEpdnConnected() const override;
    IMS_BOOL IsNoInitRegOnPcscfChange() const override;
    IMS_BOOL IsVideoOverWifiSupportedWithoutVoice() const override;
    IMS_BOOL IsGeolocationPidfSupported(IN IMS_SINT32 nGeolocationPidfType) const override;
    IMS_BOOL IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType() const override;
    IMS_BOOL IsCdmalessFeatureTagRequired() const override;
    IMS_BOOL IsRegErrCodeWithRetryAfterTimeOnlyDefined() const override;
    IMS_BOOL IsExtraReregErrInRoamingAsFailureHandled() const override;
    IMS_BOOL IsExtraRegErrRetryCntSharedForRegAndSubRequired() const override;
    IMS_BOOL IsEmergencyCallbackModeSupported() const override;
    IMS_BOOL IsEmergencySmsOverImsSupported() const override;
    IMS_BOOL IsContactUriValidationChecked() const override;
    IMS_BOOL IsPlmnBlockWithTimeoutOnFailureWithAllPcscfsSupported() const override;
    IMS_BOOL IsRegRetryWithIpVerFallback() const override;
    IMS_BOOL IsRegRequiredAfterImsCallEndOnRegHeld() const override;
    IMS_BOOL IsRegWithFeatureTagUnavailableSupported() const override;
    IMS_BOOL IsVerstatForRegistrationSupported() const override;
    IMS_BOOL IsVerstatSupportedBasedOnNetworkForReg() const override;
    IMS_BOOL IsPlmnBlockWithTimeoutOnVoiceCallUnavailable() const override;
    IMS_BOOL IsWfcErrorMessageSupported(IN IMS_SINT32 nError) const override;
    IMS_BOOL IsVideoSupportedForEmergencyReg() const override;
    IMS_BOOL IsTestModeEnabled(IN IMS_SINT32 nType) const override;
    IMS_BOOL IsReleaseEPdnOfUnavailableNetwork() const override;
    IMS_BOOL IsReleaseEPdnUponECallEndInFakeMode() const override;

    IMS_SINT32 GetSipTimerT1() override;
    IMS_UINT32 GetRegistrationRetryBaseTime() override;
    IMS_UINT32 GetRegistrationRetryMaxTime() override;
    IMS_UINT32 GetIsimIndexForImpu() override;
    IMS_SINT32 GetImsEstablishmentTime() const override;
    IMS_SINT32 GetPreferredImsDscp() const override;
    IMS_SINT32 GetRegistrationPreferredAccessTypeFeatureTag() const override;
    IMS_SINT32 GetUssdMethod() const override;
    IMS_SINT32 GetPreferredIpType() const override;
    IMS_SINT32 GetEmergencyPreferredIpType() const override;
    IMS_SINT32 GetEmcRegRetryMaxCnt() const override;
    IMS_SINT32 GetEmcRegRetryTimerMillis() const override;
    IMS_SINT32 GetPcscfPort() const override;
    IMS_SINT32 GetSipPreferredTransport() const override;
    IMS_SINT32 GetIpv4MtuSize() const override;
    IMS_SINT32 GetIpv6MtuSize() const override;
    IMS_SINT32 GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd() const override;
    IMS_SINT32 GetPreferredEmergencyRegistration() const override;
    IMS_SINT32 GetWaitTimeMillisForReleaseEPdnAfterECallEnd() const override;
    IMS_SINT32 GetEmergencyRegistrationTimerMillis() const override;
    IMS_SINT32 GetImsSignallingDscp() const override;
    IMS_SINT32 GetRegistrationPrivateHeader() const override;
    IMS_SINT32 GetRegActualWaitTimePolicy() const override;
    IMS_SINT32 GetRegDefaultWaitTime() const override;
    IMS_SINT32 GetRegOutOfServicePolicy() const override;
    IMS_SINT32 GetRoamingPreferredEmcReg() const override;
    IMS_SINT32 GetSipMessageThresholdForTransportChange() const override;
    IMS_SINT32 GetSubRetrySip503CodePolicy() const override;
    IMS_SINT32 GetUsatRegEventDownloadPolicy() const override;
    IMS_SINT32 GetVolteHysTime() const override;
    IMS_SINT32 GetRegRetrySip305CodePolicy() const override;
    IMS_SINT32 GetReregRetrySip305CodePolicy() const override;
    IMS_SINT32 GetRegRetrySip503CodePolicy() const override;
    IMS_SINT32 GetRegRetryCountOnSinglePcscf() const override;
    IMS_SINT32 GetRegRetryCountPerPcscf() const override;
    IMS_SINT32 GetRegRetryCountResetPolicy() const override;
    IMS_SINT32 GetRegRetryCountWithIpsecOnAuthFailure() const override;
    IMS_SINT32 GetRegRetryDefaultPolicy() const override;
    IMS_SINT32 GetRegRetryTimerFPolicy() const override;
    IMS_SINT32 GetRegTimerForEmcCall() const override;
    IMS_SINT32 GetExtraRegErrFinalType() const override;
    IMS_SINT32 GetExtraRegErrPolicy() const override;
    IMS_SINT32 GetExtraRegErrMaxCount() const override;
    IMS_SINT32 GetRegistrationPcscfUpdatePolicy() const override;
    IMS_SINT32 GetUserInfoPolicyForNonRegisterMessage() const override;
    IMS_SINT32 GetGeolocationPidfFormingPolicy() const override;
    IMS_SINT32 GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached() const override;
    IMS_SINT32 GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached() const override;

    ImsVector<IMS_SINT32>& GetRegRetryIntervals() override;
    ImsVector<IMS_SINT32>& GetRegRandomRetryIntervals() override;
    ImsVector<IMS_SINT32>& GetIpsecAuthenticationAlgorithms() override;
    ImsVector<IMS_SINT32>& GetIpsecEncryptionAlgorithms() override;

    IMS_UINT32 GetNotifyEventForInitialRegistration() const override;
    IMS_SINT32 GetNotifyWaitTime() const override;
    IMS_UINT32 GetNotifyEventForInitialRegWithWaitTime() const override;

    IMS_SINT32 GetPcscfRecoveryMaxRetryCnt() const override;
    IMS_SINT32 GetPcscfRecoveryWaitTime() const override;
    IMS_SINT32 GetPcscfRecoveryBaseTime() const override;
    IMS_SINT32 GetPcscfRecoveryMaxTime() const override;

    ImsVector<IMS_SINT32>& GetSubErrorRegRequired() override;
    IMS_SINT32 GetRetryCountSubErrorRegRequired() const override;
    ImsVector<IMS_SINT32>& GetSubErrorRegRequiredWithNextPcscf() override;
    ImsVector<IMS_SINT32>& GetSubErrorSubTerminated() override;
    IMS_SINT32 GetRetryCountSubErrorSubTerminated() const override;
    ImsVector<IMS_SINT32>& GetSubErrorStoppingResub() override;
    ImsVector<IMS_SINT32>& GetVowifiSubErrorRegRequired() override;
    ImsVector<IMS_SINT32>& GetImsIdentityPriority() override;
    ImsVector<IMS_SINT32>& GetPcscfDiscoveryMethod() override;
    ImsVector<IMS_SINT32>& GetUpdateRegistrationWithRatChange() override;
    ImsVector<IMS_SINT32>& GetSupportedRats() override;
    ImsVector<IMS_SINT32>& GetSupportedRoamingRats() override;
    ImsVector<IMS_SINT32>& GetSmsOverImsSupportedRats() override;
    ImsVector<IMS_SINT32>& GetEmergencyOverImsSupportedRats() override;
    ImsVector<IMS_SINT32>& GetExtraRegErrCode() override;
    ImsVector<IMS_SINT32>& GetExtraReregErrCode() override;
    ImsVector<IMS_SINT32>& GetExtraRegErrWaitTime() override;
    ImsVector<IMS_SINT32>& GetReregRetryErrCodeForInitRegWithSamePcscf() override;
    ImsVector<IMS_SINT32>& GetRegPermanentErrCode() override;
    ImsVector<IMS_SINT32>& GetRegPermanentErrMaxCount() override;
    ImsVector<IMS_SINT32>& GetRegErrCodeWithoutIpsec() override;
    ImsVector<IMS_SINT32>& GetRegErrCodeWithRetryAfterTime() override;
    ImsVector<IMS_SINT32>& GetReregErrCodeWithRetryAfterTime() override;
    ImsVector<IMS_SINT32>& GetRegErrCodeForPcscfDiscovery() override;
    ImsVector<IMS_SINT32>& GetReregErrCodeForCallEnd() override;
    ImsVector<IMS_SINT32>& GetReregErrCodeForInitRegWithAvailablePcscf() override;
    ImsVector<IMS_SINT32>& GetReregErrCodeForImsPdnReactivation() override;
    ImsVector<IMS_SINT32>& GetUnavailableFeaturesInLimitedReg() override;
    ImsVector<IMS_SINT32>& GetERegErrCodeNotSupportedCommonPolicy() override;

private:
    friend class AosBuildDirector;

    void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId) override;
    void Init(IN IMS_SINT32 nSlotId = IMS_SLOT_0) override;

protected:
    void InitBundle(IN const ICarrierConfig* piCc);
    void InitConfig(IN const ICarrierConfig* piCc);
    void InitAssetsConfig(IN const ICarrierConfig* piCc);
    void InitIpsecAlgorithm(IN const ICarrierConfig* piCc);

private:
    IMS_SINT32 m_nSlotId;

    AosAsset m_objAsset;
    AosCarrierConfig m_objCarrierConfig;

    AosMmtelRequiresProvisioningBundle m_objMmtelProvisioning;

    AosExtraRegErrBundle m_objExtraRegErr;
    AosNotifyTerminatedForInitRegBundle m_objNotifyTerminated;
    AosPcscfRecoveryConditionsBundle m_objPcscfRecoveryConditions;
    AosRegErrCodeWithRaTimeBundle m_objRegErrCodeWithRaTime;
    AosRegRetryIntervalBundle m_objRegRetryInterval;
    AosSubErrCodeForInitRegBundle m_objSubErrCodeForInitReg;
    AosSubErrCodeForTerminatedBundle m_objSubErrCodeForTerminated;
    AosWfcErrMessageBundle m_objWfcErrMessage;

    IMS_UINT32 m_nEventForInitRegOnTerminatedState;
    IMS_UINT32 m_nEventToFollowWtForInitRegOnTerminatedState;

    ImsList<IAosNConfigurationListener*> m_objListeners;

    AString m_strLogTag;
};
#endif  // AOS_NCONFIGURATION_H_
