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
    virtual ~AosNConfiguration();

    virtual IMS_SINT32 GetSlotId() const;

    virtual void SetListener(IN IAosNConfigurationListener* piListener);
    virtual void RemoveListener(IN IAosNConfigurationListener* piListener);

    virtual IMS_BOOL IsSubscription() const;
    virtual IMS_BOOL IsUnSubscription() const;
    virtual IMS_BOOL IsVoLteAvailable() const;
    virtual IMS_BOOL IsVoLteRoamingAvailable() const;
    virtual IMS_BOOL IsVtAvailable() const;
    virtual IMS_BOOL IsWfcImsAvailable() const;
    virtual IMS_BOOL IsWfcRoamingEnabled() const;
    virtual IMS_BOOL IsImsSingleRegistrationRequired() const;
    virtual IMS_BOOL IsRttSupported() const;
    virtual IMS_BOOL IsSupportLimitedAdminSmsMode() const;
    virtual IMS_BOOL IsTtySupported() const;
    virtual IMS_BOOL IsVopsIgnoredForVolteEnabled() const;
    virtual IMS_BOOL IsSmsOverImsAvailableWithoutVoiceCapability() const;
    virtual IMS_BOOL IsRequiredEmcRegInRoaming() const;
    virtual IMS_BOOL IsRequiredVolteBlockByAirplaneMode() const;
    virtual IMS_BOOL IsRequiredWfcBlockByAirplaneMode() const;
    virtual IMS_BOOL IsReregRetryWithChangedCountryOnWifi() const;
    virtual IMS_BOOL IsSipOverIpsecInRoamingEnabled() const;
    virtual IMS_BOOL UseWfcCountryCodeAvailabilityCheck() const;
    virtual IMS_BOOL IsRegRetryIntervalsUsedForSub() const;
    virtual IMS_BOOL IsSmsOverIpEnabled() const;
    virtual IMS_BOOL IsIpsecEnabled() const;
    virtual IMS_BOOL IsSecurityServerPortInRegContactOfInitRegUsed() const;
    virtual IMS_BOOL IsSecurityServerPortInInitRegUsed() const;
    virtual IMS_BOOL IsOldSaOnEstablishingSaRemoved() const;
    virtual IMS_BOOL IsUnsecureTcpSocketOnAccomplishingRegDestroyed() const;
    virtual IMS_BOOL IsEmergencyPdnWithEmergencyCallEndReleased() const;
    virtual IMS_BOOL IsSmsOverImsSupported() const;
    virtual IMS_BOOL IsImsOverNrEnabled() const;
    virtual IMS_BOOL IsEmergencyCallBasedOnPauOfNormalRegistrationSupported() const;
    virtual IMS_BOOL IsRegWithIpcanChangedDuringImsCallHeld() const;
    virtual IMS_BOOL IsDeregOn3gNetwork() const;
    virtual IMS_BOOL IsNoInitRegOnPcscfChange() const;
    virtual IMS_BOOL IsVideoOverWifiSupportedWithoutVoice() const;
    virtual IMS_BOOL IsGeolocationPidfSupported(IN IMS_SINT32 nGeolocationPidfType) const;
    virtual IMS_BOOL IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType() const;
    virtual IMS_BOOL IsCdmalessFeatureTagRequired() const;
    virtual IMS_BOOL IsRegErrCodeWithRetryAfterTimeOnlyDefined() const;
    virtual IMS_BOOL IsExtraReregErrInRoamingAsFailureHandled() const;
    virtual IMS_BOOL IsExtraRegErrRetryCntSharedForRegAndSubRequired() const;
    virtual IMS_BOOL IsRegistrationEventForCatRequired() const;
    virtual IMS_BOOL IsEmergencyCallbackModeSupported() const;
    virtual IMS_BOOL IsEmergencySmsOverImsSupported() const;
    virtual IMS_BOOL IsContactUriValidationChecked() const;
    virtual IMS_BOOL IsRegRetryWithIpVerFallback() const;
    virtual IMS_BOOL IsUserInfoInContactSupported() const;
    virtual IMS_BOOL IsRegRequiredAfterImsCallEndOnRegHeld() const;
    virtual IMS_BOOL IsRegWithFeatureTagUnavailableSupported() const;
    virtual IMS_BOOL IsVerstatForRegistrationSupported() const;

    virtual IMS_UINT32 GetRegistrationRetryBaseTime();
    virtual IMS_UINT32 GetRegistrationRetryMaxTime();
    virtual IMS_UINT32 GetIsimIndexForImpu();
    virtual IMS_SINT32 GetImsEstablishmentTime() const;
    virtual IMS_SINT32 GetPreferredImsDscp() const;
    virtual IMS_SINT32 GetRegistrationPreferredAccessTypeFeatureTag() const;
    virtual IMS_SINT32 GetUssdMethod() const;
    virtual IMS_SINT32 GetPreferredIpType() const;
    virtual IMS_SINT32 GetEmergencyPreferredIpType() const;
    virtual IMS_SINT32 GetPcscfPort() const;
    virtual IMS_SINT32 GetSipPreferredTransport() const;
    virtual IMS_SINT32 GetIpv4MtuSize() const;
    virtual IMS_SINT32 GetIpv6MtuSize() const;
    virtual IMS_SINT32 GetPreferredEmergencyRegistration() const;
    virtual IMS_SINT32 GetEmergencyRegistrationTimerMillis() const;
    virtual IMS_SINT32 GetImsSignallingDscp() const;
    virtual IMS_SINT32 GetRegistrationPrivateHeader() const;
    virtual IMS_SINT32 GetRegActualWaitTimePolicy() const;
    virtual IMS_SINT32 GetRegOutOfServicePolicy() const;
    virtual IMS_SINT32 GetSipMessageThresholdForTransportChange() const;
    virtual IMS_SINT32 GetRegRetrySip305CodePolicy() const;
    virtual IMS_SINT32 GetReregRetrySip305CodePolicy() const;
    virtual IMS_SINT32 GetRegRetrySip503CodePolicy() const;
    virtual IMS_SINT32 GetRegRetryCountResetPolicy() const;
    virtual IMS_SINT32 GetRegRetryDefaultPolicy() const;
    virtual IMS_SINT32 GetRegRetryMinCount() const;
    virtual IMS_SINT32 GetRegRetryTimerFPolicy() const;
    virtual IMS_SINT32 GetRegTimerForEmcCall() const;
    virtual IMS_SINT32 GetExtraRegErrFinalType() const;
    virtual IMS_SINT32 GetExtraRegErrPolicy() const;
    virtual IMS_SINT32 GetExtraRegErrMaxCount() const;
    virtual IMS_SINT32 GetExtraRegErrMinCount() const;
    virtual IMS_SINT32 GetRegistrationPcscfUpdatePolicy() const;
    virtual IMS_SINT32 GetUserInfoPolicyForNonRegisterMessage() const;
    virtual IMS_SINT32 GetGeolocationPidfFormingPolicy() const;
    virtual IMS_SINT32 GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached() const;
    virtual IMS_SINT32 GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached() const;

    virtual IMSVector<IMS_SINT32>& GetRegRetryIntervals();
    virtual IMSVector<IMS_SINT32>& GetRegRandomRetryIntervals();
    virtual IMSVector<IMS_SINT32>& GetIpsecAuthenticationAlgorithms();
    virtual IMSVector<IMS_SINT32>& GetIpsecEncryptionAlgorithms();

    virtual IMS_UINT32 GetNotifyEventForInitialRegistration() const;
    virtual IMS_SINT32 GetNotifyWaitTime() const;
    virtual IMS_UINT32 GetNotifyEventForInitialRegWithWaitTime() const;

    virtual IMSVector<IMS_SINT32>& GetSubErrorRegRequired();
    virtual IMS_SINT32 GetRetryCountSubErrorRegRequired() const;
    virtual IMSVector<IMS_SINT32>& GetSubErrorRegRequiredWithNextPcscf();
    virtual IMSVector<IMS_SINT32>& GetWfcSubErrorByMissing911Address();
    virtual IMSVector<IMS_SINT32>& GetSubErrorSubTerminated();
    virtual IMS_SINT32 GetRetryCountSubErrorSubTerminated() const;
    virtual IMSVector<IMS_SINT32>& GetSubErrorStoppingResub();
    virtual IMSVector<IMS_SINT32>& GetVowifiSubErrorRegRequired();
    virtual IMSVector<IMS_SINT32>& GetImsIdentityPriority();
    virtual IMSVector<IMS_SINT32>& GetPcscfDiscoveryMethod();
    virtual IMSVector<IMS_SINT32>& GetUpdateRegistrationWithRatChange();
    virtual IMSVector<IMS_SINT32>& GetSupportedRats();
    virtual IMSVector<IMS_SINT32>& GetSupportedRoamingRats();
    virtual IMSVector<IMS_SINT32>& GetSmsOverImsSupportedRats();
    virtual IMSVector<IMS_SINT32>& GetExtraRegErrCode();
    virtual IMSVector<IMS_SINT32>& GetExtraReregErrCode();
    virtual IMSVector<IMS_SINT32>& GetExtraRegErrWaitTime();
    virtual IMSVector<IMS_SINT32>& GetReregRetryErrCodeForInitRegWithSamePcscf();
    virtual IMSVector<IMS_SINT32>& GetRegPermanentErrCode();
    virtual IMSVector<IMS_SINT32>& GetRegPermanentErrMaxCount();
    virtual IMSVector<IMS_SINT32>& GetRegErrCodeWithoutIpsec();
    virtual IMSVector<IMS_SINT32>& GetRegErrCodeWithRetryAfterTime();
    virtual IMSVector<IMS_SINT32>& GetReregErrCodeWithRetryAfterTime();
    virtual IMSVector<IMS_SINT32>& GetEmergencyPcscfRetryWaitTime();
    virtual IMSVector<IMS_SINT32>& GetRegErrCodeForPcscfDiscovery();
    virtual IMSVector<IMS_SINT32>& GetReregErrCodeForCallEnd();
    virtual IMSVector<IMS_SINT32>& GetReregErrCodeForPdnReactivationAfterCallEnd();
    virtual IMSVector<IMS_SINT32>& GetReregErrCodeForInitRegWithAvailablePcscf();
    virtual IMSVector<IMS_SINT32>& GetReregErrCodeForImsPdnReactivation();

private:
    friend class AosBuildDirector;

    virtual void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId);
    virtual void Init(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

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
    AosRegErrCodeWithRaTimeBundle m_objRegErrCodeWithRaTime;
    AosRegRetryIntervalBundle m_objRegRetryInterval;
    AosSubErrCodeForInitRegBundle m_objSubErrCodeForInitReg;
    AosSubErrCodeForTerminatedBundle m_objSubErrCodeForTerminated;

    IMS_UINT32 m_nEventForInitRegOnTerminatedState;
    IMS_UINT32 m_nEventToFollowWtForInitRegOnTerminatedState;

    IMSList<IAosNConfigurationListener*> m_objListeners;

    AString m_strLogTag;

private:
    friend class AosNConfigurationTest;
};
#endif  // AOS_NCONFIGURATION_H_
