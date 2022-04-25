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
#include "ServiceConfig.h"

#include "ICarrierConfig.h"
#include "ImsIpSecType.h"

#include "provider/AosNConfiguration.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define LOGTAG m_strLogTag.GetStr()

PUBLIC
AosNConfiguration::AosNConfiguration()
    : m_nSlotId(IMS_SLOT_0)
    , m_objAsset(AosAsset())
    , m_objCarrierConfig(AosCarrierConfig())
    , m_objMmtelProvisioning(AosMmtelRequiresProvisioningBundle())
    , m_objNotifyTerminated(AosNotifyTerminatedForRegEventWithInitialRegistrationBundle())
    , m_objRegPermanentErrCode(AosRegistrationPermanentErrorCodeBundle())
    , m_objRegErrCodeWithRetryAfterTime(AosRegistrationErrorCodeWithRetryAfterTimeBundle())
    , m_objRegWithFeatureTagUnavailable(AosRegistrationWithFeatureTagUnavailableBundle())
    , m_objRegRetry(AosRegistrationRetryBundle())
    , m_objRegRetryInterval(AosRegistrationRetryIntervalBundle())
    , m_objReregErrPolicyCall(AosReregistrationErrorPolicyDuringCallBundle())
    , m_objReregRetry(AosReregistrationRetryBundle())
    , m_objSpecificRegErr(AosSpecificRegistrationErrorBundle())
    , m_objSubErrCodeWithInitReg(AosSubscriptionErrorCodeForRegEventWithInitialRegistrationBundle())
    , m_objSubTerminatedErrCode(AosSubscriptionTerminatedErrorCodeForRegEventBundle())
    , m_nEventForInitRegOnTerminatedState(0)
    , m_nEventToFollowWtForInitRegOnTerminatedState(0)
    , m_nClearPermanentPdnFailure(0)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosNConfiguration = %" PFLS_u "/%" PFLS_x,
            sizeof(AosNConfiguration), this, 0);
}

PUBLIC VIRTUAL
AosNConfiguration::~AosNConfiguration()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : AosNConfiguration = %" PFLS_u "/%" PFLS_x,
            sizeof(AosNConfiguration), this, 0);

    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    if (piCc != IMS_NULL)
    {
        piCc->RemoveListener(this);
    }
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetSlotId() const
{
    return m_nSlotId;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsSubscription() const
{
    return m_objCarrierConfig.bRegistrationEventPackageSupported;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsUnSubscription() const
{
    return m_objCarrierConfig.bUnsubscribeRegistrationEventPackage;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsVoLteAvailable() const
{
    return m_objCarrierConfig.bCarrierVolteAvailable;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsVoLteRoamingAvailable() const
{
    return m_objCarrierConfig.bCarrierVolteRoamingAvailable;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsVtAvailable() const
{
    return m_objCarrierConfig.bCarrierVtAvailable;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsDataEnableChangeIgnoredForVideoCalls() const
{
    // TODO: KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS
    return IMS_FALSE;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsWfcImsAvailable() const
{
    return m_objCarrierConfig.bCarrierWfcImsAvailable;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsWfcRoamingEnabled() const
{
    // TODO: KEY_CARRIER_WFC_ROAMING_ENABLED_BOOL (?)
    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsImsSingleRegistrationRequired() const
{
    return m_objCarrierConfig.bImsSingleRegistrationRequired;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsRttSupported() const
{
    return m_objCarrierConfig.bRttSupported;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsSupportLimitedAdminSmsMode() const
{
    return m_objCarrierConfig.bSupportLimitedAdminSmsMode;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsTtySupported() const
{
    // TODO: KEY_TTY_SUPPORTED_BOOL (?)
    return m_objCarrierConfig.bCarrierVolteTtySupported;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsVopsIgnoredForVolteEnabled() const
{
    return m_objAsset.bIgnoreVopsForVolteEnable;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsRequiredEmergencyRegistrationInRoaming() const
{
    return m_objAsset.bRequiredEmergencyRegistrationInRoaming;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsRequiredVolteBlockBySetting() const
{
    return m_objAsset.bRequiredVolteBlockBySetting;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsRequiredVolteBlockByAirplaneMode() const
{
    return m_objAsset.bRequiredVolteBlockByAirplaneMode;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsRequiredWfcBlockByAirplaneMode() const
{
    return m_objAsset.bRequiredWfcBlockByAirplaneMode;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::UseWfcCountryCodeAvailabilityCheck() const
{
    return m_objAsset.bUseWfcCountryCodeAvailabilityCheck;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsRegistrationRetryIntervalsUsedForSubscription() const
{
    return m_objRegRetryInterval.bUseRegistrationRetryIntervalForSubscriptionRetry;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsSmsOverIpEnabled() const
{
    // KEY_SMS_OVER_IP_ENABLED (ProvisioningManager)
    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsIpsecEnabled() const
{
    return m_objCarrierConfig.bSipOverIpsecEnabled;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsSecurityServerPortInRegContactOfInitialRegistrationUsed() const
{
    return m_objAsset.bUseSecurityServerPortInRegContactOfInitialRegistration;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsSecurityServerPortInInitialRegistrationUsed() const
{
    return m_objAsset.bUseSecurityServerPortInInitialRegistration;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsOldSaOnEstablishingSaRemoved() const
{
    return m_objAsset.bRemoveOldSaOnEstablishingSa;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsEmergencyPdnWithEmergencyCallEndReleased() const
{
    return m_objCarrierConfig.bReleaseEmergencyPdnWithEmergencyCallEnd;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsSmsOverImsSupported() const
{
    return m_objCarrierConfig.bSmsOverImsSupported;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsImsOverNrEnabled() const
{
    for (int i = 0; i < m_objCarrierConfig.objCarrierNrAvailabilities.GetSize(); i++)
    {
        if (m_objCarrierConfig.objCarrierNrAvailabilities.GetAt(i) ==
                CarrierConfig::Ims::CARRIER_NR_AVAILABILITY_SA)
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsVerstatForRegistrationSupported() const
{
    return m_objAsset.bSupportVerstatForRegistration;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsEmergencyCallBasedOnPauOfNormalRegistrationSupported() const
{
    return m_objAsset.bEmergencyCallBasedOnPAssociatedUriOfNormalRegistration;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsRegistrationWhenIpcanChangedWithImsActiveCallHeld() const
{
    return m_objAsset.bHoldRegistrationWhenIpcanChangedWithImsActiveCall;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsDeregisterOn3gNetworks() const
{
    return m_objAsset.bImsDeregisterOn3gNetworks;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsVideoOverWifiSupportedWithoutVoice() const
{
    return m_objAsset.bVideoOverWifiSupportedWithoutVoice;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsGeolocationPidfSupported(IN IMS_SINT32 nGeolocationPidfType) const
{
    const IMSVector<IMS_SINT32>& objGeolocationPidfTypes =
            m_objCarrierConfig.objGeolocationPidfInSipRegisterSupport;

    for (IMS_UINT32 i = 0; i < objGeolocationPidfTypes.GetSize(); ++i)
    {
        if (nGeolocationPidfType == objGeolocationPidfTypes.GetAt(i))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType() const
{
    return m_objAsset.bUseGGsmaRcsTelephonyFeatureTagAsAvailableVoiceCallType;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsCdmalessFeatureTagRequired() const
{
    return m_objAsset.bCdmalessFeatureTagRequired;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsRegErrCodeWithRetryAfterTimeOnlyDeifined() const
{
    return m_objRegErrCodeWithRetryAfterTime.bRegistrationErrorCodeWithRetryAfterTimeOnlyDefined;
}

PUBLIC VIRTUAL
IMS_BOOL AosNConfiguration::IsSpecificRegErrRetryCountSharedForRegAndRegEventRequired() const
{
    return m_objSpecificRegErr.bSpecificRegErrRetryCountSharedForRegAndRegEvent;
}

PUBLIC VIRTUAL
IMS_UINT32 AosNConfiguration::GetRegistrationRetryBaseTime()
{
    return static_cast<IMS_UINT32>(m_objCarrierConfig.nRegistrationRetryBaseTimerMillis);
}

PUBLIC VIRTUAL
IMS_UINT32 AosNConfiguration::GetRegistrationRetryMaxTime()
{
    return static_cast<IMS_UINT32>(m_objCarrierConfig.nRegistrationRetryMaxTimerMillis);
}

PUBLIC VIRTUAL
IMS_UINT32 AosNConfiguration::GetIsimIndexForImpu()
{
    return static_cast<IMS_UINT32>(m_objCarrierConfig.nIsimIndexForImpu);
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetUssdMethod() const
{
    return m_objCarrierConfig.nCarrierUssdMethod;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetPreferredIpType() const
{
    return m_objCarrierConfig.nImsPreferredIpType;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetEmergencyPreferredIpType() const
{
    return m_objAsset.nEmergencyPreferredIpType;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetPcscfPort() const
{
    return m_objCarrierConfig.nSipServerPortNumber;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetSipPreferredTransport() const
{
    return m_objCarrierConfig.nSipPreferredTransport;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetIpv4MtuSize() const
{
    return m_objCarrierConfig.nIpv4SipMtuSizeCellular;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetIpv6MtuSize() const
{
    return m_objCarrierConfig.nIpv6SipMtuSizeCellular;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetPreferredEmergencyRegistration() const
{
    return m_objCarrierConfig.nPreferredEmergencyRegistration;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetEmergencyRegistrationTimerMillis() const
{
    return m_objCarrierConfig.nEmergencyRegistrationTimerMillis;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetRegistrationRetryDefaultPolicy() const
{
    return m_objRegRetry.nRegistrationRetryDefaultPolicy;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetPreferredImsDscp() const
{
    return m_objCarrierConfig.nPreferredImsDscp;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetImsSignallingDscp() const
{
    return m_objCarrierConfig.nImsSignallingDscp;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetRegistrationPreferredAccessTypeFeatureTag() const
{
    return m_objCarrierConfig.nRegistrationPreferredAccesstypeFeatureTag;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetRegistrationPrivateHeader() const
{
    return m_objCarrierConfig.nRegistrationPrivateHeader;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetRegistrationActualWaitTimePolicy() const
{
    return m_objCarrierConfig.nRegistrationActualWaitTimePolicy;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetSipMessageThresholdForTransportChange() const
{
    return m_objAsset.nSipMessageThresholdForTransportChange;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetRegistrationRetrySip305CodePolicy() const
{
    return m_objRegRetry.nRegistrationRetrySip305CodePolicy;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetReregistrationRetrySip305CodePolicy() const
{
    return m_objReregRetry.nReregistrationRetrySip305CodePolicy;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetRegistrationRetrySip503CodePolicy() const
{
    return m_objRegRetry.nRegistrationRetrySip503CodePolicy;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetSpecificRegistrationErrorFinalType() const
{
    return m_objSpecificRegErr.nSpecificRegErrFinalType;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetSpecificRegistrationErrorPolicy() const
{
    return m_objSpecificRegErr.nSpecificRegErrPolicy;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetSpecificRegistrationErrorMaxCount() const
{
    return m_objSpecificRegErr.nSpecificRegErrMaxCount;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetRegistrationRetryIntervals()
{
    return m_objRegRetryInterval.objRegistrationRetryIntervalSec;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetRegistrationRandomRetryIntervals()
{
    return m_objRegRetryInterval.objRegistrationRetryRandomUpperValueSec;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetIpsecAuthenticationAlgorithms()
{
    return m_objCarrierConfig.objIpsecAuthenticationAlgorithms;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetIpsecEncryptionAlgorithms()
{
    return m_objCarrierConfig.objIpsecEncryptionAlgorithms;
}

PUBLIC VIRTUAL
IMS_UINT32 AosNConfiguration::GetNotifyEventForInitialRegistration() const
{
    return m_nEventForInitRegOnTerminatedState;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetNotifyWaitTime() const
{
    return m_objNotifyTerminated.nWaitTimeForInitRegOnTerminatedstate;
}

PUBLIC VIRTUAL
IMS_UINT32 AosNConfiguration::GetNotifyEventForInitialRegWithWaitTime() const
{
    return m_nEventToFollowWtForInitRegOnTerminatedState;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetRetryCountSubErrorRegRequired() const
{
    return m_objSubErrCodeWithInitReg.nSubErrRetryMaxCountWithInitReg;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetSubErrorRegRequired()
{
    return m_objSubErrCodeWithInitReg.objSubErrCodeWithInitReg;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetSubErrorRegRequiredWithNextPcscf()
{
    return m_objAsset.objSubscriptionErrorCodeForRegEventWithInitialRegistrationWithNextPcscf;
}

PUBLIC VIRTUAL
IMS_SINT32 AosNConfiguration::GetRetryCountSubErrorSubTerminated() const
{
    return m_objSubTerminatedErrCode.nSubTerminatedErrCodeRetryMaxCount;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetSubErrorSubTerminated()
{
    return m_objSubTerminatedErrCode.objSubscriptionTerminatedErrorCodeForRegEvent;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetSubErrorStoppingResub()
{
    return m_objAsset.objSubscriptionErrorCodeForStoppingByExpirationTime;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetVowifiSubErrorRegRequired()
{
    return m_objAsset.objVowifiSubscriptionErrorCodeWithInitialRegistration;
}

PUBLIC VIRTUAL
IMS_UINT32 AosNConfiguration::GetClearReasonForPermanentPdnFailure() const
{
    return m_nClearPermanentPdnFailure;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetImsIdentityPriority()
{
    return m_objCarrierConfig.objImsIdentityPriority;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetPcscfDiscoveryMethod()
{
    return m_objCarrierConfig.objPcscfDiscoveryMethod;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetRoamingPcscfDiscoveryMethod()
{
    return m_objAsset.objPcscfDiscoveryMethodRoaming;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetUpdateRegistrationWithRatChange()
{
    return m_objCarrierConfig.objUpdateRegistrationWithRatChange;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetSupportedRats()
{
    return m_objCarrierConfig.objSupportedRats;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetSupportedRoamingRats()
{
    return m_objAsset.objSupportedRoamingRats;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetSmsOverImsSupportedRats()
{
    return m_objCarrierConfig.objSmsOverImsSupportedRats;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetSpecificRegErrNumMultipliedByPcscfNum()
{
    return m_objSpecificRegErr.objSpecificRegErrNumMultipliedByPcscfNum;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetSpecificRegistrationErrorCode()
{
    return m_objSpecificRegErr.objSpecificRegErrCode;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetSpecificReregistrationErrorCode()
{
    return m_objSpecificRegErr.objSpecificReregErrCode;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetSpecificRegErrWaitTime()
{
    return m_objSpecificRegErr.objSpecificRegErrWaitTimeSec;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetReregRetryErrCodeWithInitialRegWithSamePcscf()
{
    return m_objReregRetry.objReregistrationRetryErrorCodeWithInitialRegistrationWithSamePcscf;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetRegPermanentErrCode()
{
    return m_objRegPermanentErrCode.objRegistrationPermanentErrorCode;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetRegPermanentErrMaxCount()
{
    return m_objRegPermanentErrCode.objRegistrationPermanentErrorMaxCount;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetRegErrCodeWithRetryAfterTime()
{
    return m_objRegErrCodeWithRetryAfterTime.objRegistrationErrorCodeWithRetryAfterTime;
}

PUBLIC VIRTUAL
IMSVector<IMS_SINT32>& AosNConfiguration::GetReregErrCodeWithRetryAfterTime()
{
    return m_objRegErrCodeWithRetryAfterTime.objReregistrationErrorCodeWithRetryAfterTime;
}

PRIVATE VIRTUAL
void AosNConfiguration::CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId)
{
    if (m_nSlotId != nSlotId)
    {
        return;
    }

    // TODO: implement
}

PRIVATE VIRTUAL
void AosNConfiguration::Init(IN IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    m_strLogTag.Sprintf("%d:aos", nSlotId);

    A_IMS_TRACE_D(LOGTAG, "Init", 0, 0, 0);

    m_nSlotId = nSlotId;

    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    if (piCc == IMS_NULL)
    {
        A_IMS_TRACE_I(LOGTAG, "Init :: config failed", 0, 0, 0);

    }

    piCc->AddListener(this);

    InitConfig(piCc);
    InitBundle(piCc);
}

PRIVATE
void AosNConfiguration::InitBundle(IN const ICarrierConfig* piCc)
{
    // AosSpecificRegistrationErrorBundle
    ICarrierConfig* piCcBundle =
                piCc->GetBundle(CarrierConfig::Assets::KEY_SPECIFIC_REGISTRATION_ERROR_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objSpecificRegErr.nSpecificRegErrFinalType = piCcBundle->GetInt(
                CarrierConfig::Assets::KEY_SPECIFIC_REGISTRATION_ERROR_FINAL_TYPE_INT);
        m_objSpecificRegErr.nSpecificRegErrPolicy = piCcBundle->GetInt(
                CarrierConfig::Assets::KEY_SPECIFIC_REGISTRATION_ERROR_POLICY_INT);
        m_objSpecificRegErr.nSpecificRegErrMaxCount = piCcBundle->GetInt(
                CarrierConfig::Assets::KEY_SPECIFIC_REGISTRATION_ERROR_MAX_COUNT_INT);
        m_objSpecificRegErr.nSpecificRegErrMinCount = piCcBundle->GetInt(
                CarrierConfig::Assets::KEY_SPECIFIC_REGISTRATION_ERROR_MIN_COUNT_INT);
        m_objSpecificRegErr.bSpecificRegErrRetryCountSharedForRegAndRegEvent =
                piCcBundle->GetBoolean(
                CarrierConfig::Assets::KEY_SPECIFIC_REGISTRATION_ERROR_RETRY_COUNT_SHARED_FOR_REGISTRATION_AND_REG_EVENT_BOOL);
        m_objSpecificRegErr.objSpecificRegErrNumMultipliedByPcscfNum =
                piCcBundle->GetIntArray(
                CarrierConfig::Assets::KEY_SPECIFIC_REGISTRATION_ERROR_NUMBER_MULTIPLIED_BY_PCSCF_NUMBER_INT_ARRAY);
        m_objSpecificRegErr.objSpecificRegErrCode =
                piCcBundle->GetIntArray(
                CarrierConfig::Assets::KEY_SPECIFIC_REGISTRATION_ERROR_CODE_INT_ARRAY);
        m_objSpecificRegErr.objSpecificReregErrCode =
                piCcBundle->GetIntArray(
                CarrierConfig::Assets::KEY_SPECIFIC_REREGISTRATION_ERROR_CODE_INT_ARRAY);
        m_objSpecificRegErr.objSpecificRegErrWaitTimeSec =
                piCcBundle->GetIntArray(
                CarrierConfig::Assets::KEY_SPECIFIC_REGISTRATION_ERROR_WAIT_TIME_SEC_INT_ARRAY);
        m_objSpecificRegErr.bSpecificReregFailureWithErrCodeInRoaming = piCcBundle->GetBoolean(
                CarrierConfig::Assets::KEY_SPECIFIC_REREGISTRATION_FAILURE_WITH_ERROR_CODE_IN_ROAMING_BOOL);
        piCcBundle->ReleaseBundle();
        piCcBundle = IMS_NULL;
    }

    // AosRegistrationRetryBundle
    piCcBundle = piCc->GetBundle(CarrierConfig::Assets::KEY_REGISTRATION_RETRY_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objRegRetry.nRegistrationRetryMinCount = piCcBundle->GetInt(
                CarrierConfig::Assets::KEY_REGISTRATION_RETRY_MIN_COUNT_INT);
        m_objRegRetry.nRegistrationRetrySip305CodePolicy = piCcBundle->GetInt(
                CarrierConfig::Assets::KEY_REGISTRATION_RETRY_SIP_305_CODE_POLICY_INT);
        m_objRegRetry.objRegistrationRetryErrorCodeWithoutIpsec =
                piCcBundle->GetIntArray(
                CarrierConfig::Assets::KEY_REGISTRATION_RETRY_ERROR_CODE_WITHOUT_IPSEC_INT_ARRAY);
        m_objRegRetry.nRegistrationRetryTimerFPolicy = piCcBundle->GetInt(
                CarrierConfig::Assets::KEY_REGISTRATION_RETRY_TIMER_F_POLICY_INT);
        m_objRegRetry.objRegistrationRetryErrorCodeWithDifferentPcscf =
                piCcBundle->GetIntArray(
                CarrierConfig::Assets::KEY_REGISTRATION_RETRY_ERROR_CODE_WITH_DIFFERENT_PCSCF_INT_ARRAY);
        m_objRegRetry.bRegistrationRetryWithIpVersionFallback = piCcBundle->GetBoolean(
                CarrierConfig::Assets::KEY_REGISTRATION_RETRY_WITH_IP_VERSION_FALLBACK_BOOL);
        m_objRegRetry.nRegistrationRetryDefaultPolicy = piCcBundle->GetInt(
                CarrierConfig::Assets::KEY_REGISTRATION_RETRY_DEFAULT_POLICY_INT);
        m_objRegRetry.nRegistrationRetrySip503CodePolicy = piCcBundle->GetInt(
                CarrierConfig::Assets::KEY_REGISTRATION_RETRY_SIP_503_CODE_POLICY_INT);
        piCcBundle->ReleaseBundle();
        piCcBundle = IMS_NULL;
    }

    // AosReregistrationRetryBundle
    piCcBundle = piCc->GetBundle(CarrierConfig::Assets::KEY_REREGISTRATION_RETRY_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objReregRetry.objReregistrationRetryErrorCodeWithInitialRegistration =
                piCcBundle->GetIntArray(
                CarrierConfig::Assets::KEY_REREGISTRATION_RETRY_ERROR_CODE_WITH_INITIAL_REGISTRATION_INT_ARRAY);
        m_objReregRetry.bReregistrationRetryExpireTimeChecked = piCcBundle->GetBoolean(
                CarrierConfig::Assets::KEY_REREGISTRATION_RETRY_EXPIRE_TIME_CHECKED_BOOL);
        m_objReregRetry.nReregistrationRetryMaxCountKeptRegistration = piCcBundle->GetInt(
                CarrierConfig::Assets::KEY_REREGISTRATION_RETRY_MAX_COUNT_KEPT_REGISTRATION_INT);
        m_objReregRetry.objReregistrationRetryErrorCodeWithInitialRegistrationWithSamePcscf =
                piCcBundle->GetIntArray(
                CarrierConfig::Assets::KEY_REREGISTRATION_RETRY_ERROR_CODE_WITH_INITIAL_REGISTRATION_WITH_SAME_PCSCF_INT_ARRAY);
        m_objReregRetry.nReregistrationRetrySip305CodePolicy = piCcBundle->GetInt(
                CarrierConfig::Assets::KEY_REREGISTRATION_RETRY_SIP_305_CODE_POLICY_INT);
        piCcBundle->ReleaseBundle();
        piCcBundle = IMS_NULL;
    }

    // AosRegistrationRetryIntervalBundle
    piCcBundle = piCc->GetBundle(
            CarrierConfig::Ims::KEY_REGISTRATION_RETRY_INTERVAL_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objRegRetryInterval.objRegistrationRetryRandomUpperValueSec = piCcBundle->GetIntArray(
                CarrierConfig::Ims::KEY_REGISTRATION_RETRY_RANDOM_UPPER_VALUE_SEC_INT_ARRAY);
        m_objRegRetryInterval.objRegistrationRetryIntervalSec = piCcBundle->GetIntArray(
                CarrierConfig::Ims::KEY_REGISTRATION_RETRY_INTERVAL_SEC_INT_ARRAY);
        m_objRegRetryInterval.bUseRegistrationRetryIntervalForSubscriptionRetry =
                piCcBundle->GetBoolean(
                CarrierConfig::Ims::KEY_USE_REGISTRATION_RETRY_INTERVAL_FOR_SUBSCRIPTION_RETRY_BOOL);
        piCcBundle->ReleaseBundle();
        piCcBundle = IMS_NULL;
    }

    // AosRegistrationPermanentErrorCodeBundle
    piCcBundle = piCc->GetBundle(
            CarrierConfig::Ims::KEY_REGISTRATION_PERMANENT_ERROR_CODE_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objRegPermanentErrCode.objRegistrationPermanentErrorCode = piCcBundle->GetIntArray(
                CarrierConfig::Ims::KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY);
        m_objRegPermanentErrCode.objRegistrationPermanentErrorMaxCount = piCcBundle->GetIntArray(
                CarrierConfig::Ims::KEY_REGISTRATION_PERMANENT_ERROR_MAX_COUNT_INT_ARRAY);
        piCcBundle->ReleaseBundle();
        piCcBundle = IMS_NULL;
    }

    // AosRegistrationErrorCodeWithRetryAfterTimeBundle
    piCcBundle = piCc->GetBundle(
            CarrierConfig::Assets::KEY_REGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objRegErrCodeWithRetryAfterTime.bRegistrationErrorCodeWithRetryAfterTimeOnlyDefined =
                piCcBundle->GetBoolean(
                CarrierConfig::Assets::KEY_REGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_ONLY_DEFINED_BOOL);
        m_objRegErrCodeWithRetryAfterTime.objRegistrationErrorCodeWithRetryAfterTime =
                piCcBundle->GetIntArray(
                CarrierConfig::Assets::KEY_REGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_INT_ARRAY);
        m_objRegErrCodeWithRetryAfterTime.objReregistrationErrorCodeWithRetryAfterTime =
                piCcBundle->GetIntArray(
                CarrierConfig::Assets::KEY_REREGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_INT_ARRAY);
        piCcBundle->ReleaseBundle();
        piCcBundle = IMS_NULL;
    }

    // AosNotifyTerminatedForRegEventWithInitialRegistrationBundle
    piCcBundle = piCc->GetBundle(
            CarrierConfig::Ims::KEY_NOTIFY_TERMINATED_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objNotifyTerminated.nWaitTimeForInitRegOnTerminatedstate = piCcBundle->GetInt(
                CarrierConfig::Ims::KEY_WAIT_TIME_FOR_INITIAL_REGISTRATION_ON_TERMINATED_STATE_OF_REG_EVENT_INT);
        m_objNotifyTerminated.objEventForInitRegOnTerminatedState = piCcBundle->GetIntArray(
                CarrierConfig::Ims::KEY_EVENT_FOR_INITIAL_REGISTRATION_ON_TERMINATED_STATE_OF_REG_EVENT_INT_ARRAY);
        m_objNotifyTerminated.objEventToFollowWtForInitRegOnTerminatedState =
                piCcBundle->GetIntArray(
                CarrierConfig::Ims::KEY_EVENT_TO_FOLLOW_WAIT_TIME_FOR_INITIAL_REGISTRATION_ON_TERMINATED_STATE_OF_REG_EVENT_INT_ARRAY);
        piCcBundle->ReleaseBundle();
        piCcBundle = IMS_NULL;

        IMSVector<IMS_SINT32>& objNotifyEvents =
                m_objNotifyTerminated.objEventForInitRegOnTerminatedState;
        m_nEventForInitRegOnTerminatedState = 0;
        for (int i = 0; i < objNotifyEvents.GetSize(); i++)
        {
            m_nEventForInitRegOnTerminatedState |= 0x1 << (objNotifyEvents.GetAt(i) - 1);
        }

        IMSVector<IMS_SINT32>& objEventToFollow =
                m_objNotifyTerminated.objEventToFollowWtForInitRegOnTerminatedState;
        m_nEventToFollowWtForInitRegOnTerminatedState = 0;
        for (int i = 0; i < objEventToFollow.GetSize(); i++)
        {
            m_nEventToFollowWtForInitRegOnTerminatedState |= 0x1 << (objEventToFollow.GetAt(i) - 1);
        }
    }

    // AosSubscriptionErrorCodeForRegEventWithInitialRegistrationBundle
    piCcBundle = piCc->GetBundle(
            CarrierConfig::Assets::KEY_SUBSCRIPTION_ERROR_CODE_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objSubErrCodeWithInitReg.nSubErrRetryMaxCountWithInitReg = piCcBundle->GetInt(
                CarrierConfig::Assets::KEY_SUBSCRIPTION_ERROR_RETRY_MAX_COUNT_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_INT);
        m_objSubErrCodeWithInitReg.objSubErrCodeWithInitReg = piCcBundle->GetIntArray(
                CarrierConfig::Assets::KEY_SUBSCRIPTION_ERROR_CODE_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_INT_ARRAY);
        piCcBundle->ReleaseBundle();
        piCcBundle = IMS_NULL;
    }

    // AosSubscriptionTerminatedErrorCodeForRegEventBundle
    piCcBundle = piCc->GetBundle(
            CarrierConfig::Assets::KEY_SUBSCRIPTION_TERMINATED_ERROR_CODE_FOR_REG_EVENT_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objSubTerminatedErrCode.nSubTerminatedErrCodeRetryMaxCount = piCcBundle->GetInt(
                CarrierConfig::Assets::KEY_SUBSCRIPTION_TERMINATED_ERROR_CODE_FOR_REG_EVENT_RETRY_MAX_COUNT_INT);
        m_objSubTerminatedErrCode.objSubscriptionTerminatedErrorCodeForRegEvent =
                piCcBundle->GetIntArray(
                CarrierConfig::Assets::KEY_SUBSCRIPTION_TERMINATED_ERROR_CODE_FOR_REG_EVENT_INT_ARRAY);
        piCcBundle->ReleaseBundle();
        piCcBundle = IMS_NULL;
    }
}

PRIVATE
void AosNConfiguration::InitConfig(IN const ICarrierConfig* piCc)
{
    /* aosp_carrier_config */
    /// no prefix
    m_objCarrierConfig.bSupportEmergencySmsOverIms = piCc->GetBoolean(
            CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL);
    m_objCarrierConfig.bCarrierVolteAvailable = piCc->GetBoolean(
            CarrierConfig::KEY_CARRIER_VOLTE_AVAILABLE_BOOL);
    m_objCarrierConfig.bCarrierVtAvailable = piCc->GetBoolean(
            CarrierConfig::KEY_CARRIER_VT_AVAILABLE_BOOL);
    m_objCarrierConfig.bCarrierWfcImsAvailable = piCc->GetBoolean(
            CarrierConfig::KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL);
    m_objCarrierConfig.bRttSupported = piCc->GetBoolean(
            CarrierConfig::KEY_RTT_SUPPORTED_BOOL);
    m_objCarrierConfig.bCarrierCrossSimImsAvailable = piCc->GetBoolean(
            CarrierConfig::KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL);
    m_objCarrierConfig.bCarrierVolteTtySupported = piCc->GetBoolean(
            CarrierConfig::KEY_CARRIER_VOLTE_TTY_SUPPORTED_BOOL);
    m_objCarrierConfig.objCarrierNrAvailabilities = piCc->GetIntArray(
            CarrierConfig::KEY_CARRIER_NR_AVAILABILITIES_INT_ARRAY);
    m_objCarrierConfig.nCarrierUssdMethod = piCc->GetInt(
            CarrierConfig::KEY_CARRIER_USSD_METHOD_INT);
    /// ims.
    m_objCarrierConfig.objPcscfDiscoveryMethod = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY);
    m_objCarrierConfig.bImsSingleRegistrationRequired = piCc->GetBoolean(
            CarrierConfig::Ims::KEY_IMS_SINGLE_REGISTRATION_REQUIRED_BOOL);
     m_objCarrierConfig.nSipServerPortNumber = piCc->GetInt(
            CarrierConfig::Ims::KEY_SIP_SERVER_PORT_NUMBER_INT);
    m_objCarrierConfig.bKeepPdnUpInNoVops = piCc->GetBoolean(
            CarrierConfig::Ims::KEY_KEEP_PDN_UP_IN_NO_VOPS_BOOL);
    m_objCarrierConfig.nSipPreferredTransport = piCc->GetInt(
            CarrierConfig::Ims::KEY_SIP_PREFERRED_TRANSPORT_INT);
    m_objCarrierConfig.nIpv4SipMtuSizeCellular = piCc->GetInt(
            CarrierConfig::Ims::KEY_IPV4_SIP_MTU_SIZE_CELLULAR_INT);
    m_objCarrierConfig.nIpv6SipMtuSizeCellular = piCc->GetInt(
            CarrierConfig::Ims::KEY_IPV6_SIP_MTU_SIZE_CELLULAR_INT);
    m_objCarrierConfig.objImsPdnEnabledInNoVopsSupport = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY);
    m_objCarrierConfig.bSipOverIpsecEnabled = piCc->GetBoolean(
            CarrierConfig::Ims::KEY_SIP_OVER_IPSEC_ENABLED_BOOL);
    InitIpsecAlgorithm(piCc);
    m_objCarrierConfig.nRegistrationExpiryTimerSec =  piCc->GetInt(
            CarrierConfig::Ims::KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT);
    m_objCarrierConfig.nRegistrationRetryBaseTimerMillis = piCc->GetInt(
            CarrierConfig::Ims::KEY_REGISTRATION_RETRY_BASE_TIMER_MILLIS_INT);
    m_objCarrierConfig.nRegistrationRetryMaxTimerMillis = piCc->GetInt(
            CarrierConfig::Ims::KEY_REGISTRATION_RETRY_MAX_TIMER_MILLIS_INT);
    m_objCarrierConfig.bRegistrationEventPackageSupported = piCc->GetBoolean(
            CarrierConfig::Ims::KEY_REGISTRATION_EVENT_PACKAGE_SUPPORTED_BOOL);
    m_objCarrierConfig.nRegistrationSubscribeExpiryTimerSec = piCc->GetInt(
            CarrierConfig::Ims::KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT);
    m_objCarrierConfig.objGeolocationPidfInSipRegisterSupport = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_IN_SIP_REGISTER_SUPPORT_INT_ARRAY);

    // temp code
    IMSVector<IMS_SINT32> objTemp = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_SUPPORTED_RATS_INT_ARRAY);
    if (objTemp.GetSize() > 0)
    {
        m_objCarrierConfig.objSupportedRats.Clear();
        m_objCarrierConfig.objSupportedRats = piCc->GetIntArray(
                CarrierConfig::Ims::KEY_SUPPORTED_RATS_INT_ARRAY);
    }

    /// imsvoice.
    m_objCarrierConfig.bCarrierVolteRoamingAvailable = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_CARRIER_VOLTE_ROAMING_AVAILABLE_BOOL);
    /// imssms.
    m_objCarrierConfig.bSmsOverImsSupported = piCc->GetBoolean(
            CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL);
    m_objCarrierConfig.objSmsOverImsSupportedRats = piCc->GetIntArray(
            CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY);
    /// imsrtt.
    /// imsemergency.
    m_objCarrierConfig.bEmergencyCallbackModeSupported = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_EMERGENCY_CALLBACK_MODE_SUPPORTED_BOOL);
    m_objCarrierConfig.objEmergencyOverImsSupportedRats = piCc->GetIntArray(
            CarrierConfig::ImsEmergency::KEY_EMERGENCY_OVER_IMS_SUPPORTED_RATS_INT_ARRAY);
    m_objCarrierConfig.nEmergencyRegistrationTimerMillis = piCc->GetInt(
            CarrierConfig::ImsEmergency::KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT);
    m_objCarrierConfig.nRefreshGeolocationTimeoutMillis = piCc->GetInt(
            CarrierConfig::ImsEmergency::KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT);
    /// imsvt.
    /// imswfc.

    /* carrier_config */
    /// ims.
    m_objCarrierConfig.nImsPreferredIpType = piCc->GetInt(
            CarrierConfig::Ims::KEY_IMS_PREFERRED_IPTYPE_INT);
    m_objCarrierConfig.objImsIdentityPriority = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY);
    m_objCarrierConfig.nIsimIndexForImpu = piCc->GetInt(
            CarrierConfig::Ims::KEY_ISIM_INDEX_FOR_IMPU_INT);
    m_objCarrierConfig.objUpdateRegistrationWithRatChange = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_UPDATE_REGISTRATION_WITH_RAT_CHANGE_INT_ARRAY);
    m_objCarrierConfig.bUnsubscribeRegistrationEventPackage = piCc->GetBoolean(
            CarrierConfig::Ims::KEY_UNSUBSCRIBE_REGISTRATION_EVENT_PACKAGE_BOOL);
    m_objCarrierConfig.bRegistrationEventForCatRequired = piCc->GetBoolean(
            CarrierConfig::Ims::KEY_REGISTRATION_EVENT_FOR_CAT_REQUIRED_BOOL);
    m_objCarrierConfig.nPreferredImsDscp = piCc->GetInt(
            CarrierConfig::Ims::KEY_PREFERRED_IMS_DSCP_INT);
    m_objCarrierConfig.nImsSignallingDscp = piCc->GetInt(
            CarrierConfig::Ims::KEY_IMS_SIGNALLING_DSCP_INT);
    m_objCarrierConfig.nRegistrationPreferredAccesstypeFeatureTag = piCc->GetInt(
            CarrierConfig::Ims::KEY_REGISTRATION_PREFERRED_ACCESSTYPE_FEATURE_TAG_INT);
    m_objCarrierConfig.nRegistrationActualWaitTimePolicy = piCc->GetInt(
            CarrierConfig::Ims::KEY_REGISTRATION_ACTUAL_WAIT_TIME_POLICY_INT);
    /// imssms.
    m_objCarrierConfig.bSupportLimitedAdminSmsMode = piCc->GetBoolean(
            CarrierConfig::ImsSms::KEY_SUPPORT_LIMITED_ADMIN_SMS_MODE_BOOL);
    /// imsemergency.
    m_objCarrierConfig.bReleaseEmergencyPdnWithEmergencyCallEnd = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_RELEASE_EMERGENCY_PDN_WITH_EMERGENCY_CALL_END_BOOL);
    m_objCarrierConfig.bDisableT3482ForEmergency = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_DISABLE_T3482_FOR_EMERGENCY_BOOL);
    m_objCarrierConfig.nRegistrationTimerForEmergencyCallMillis = piCc->GetInt(
            CarrierConfig::ImsEmergency::KEY_REGISTRATION_TIMER_FOR_EMERGENCY_CALL_MILLIS_INT);
    m_objCarrierConfig.nPreferredEmergencyRegistration = piCc->GetInt(
            CarrierConfig::ImsEmergency::KEY_PREFERRED_EMERGENCY_REGISTRATION_INT);
    /// imswfc.
    m_objCarrierConfig.bUpdateRegistrationWithCountryChange = piCc->GetBoolean(
            CarrierConfig::ImsWfc::KEY_UPDATE_REGISTRATION_WITH_COUNTRY_CHANGE_BOOL);
    m_objCarrierConfig.nRegistrationPrivateHeader = piCc->GetInt(
            CarrierConfig::ImsWfc::KEY_REGISTRATION_PRIVATE_HEADER_INT);
    /// asset
    m_objAsset.bUseSecurityServerPortInRegContactOfInitialRegistration = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_USE_SECURITY_SERVER_PORT_IN_REG_CONTACT_OF_INITIAL_REGISTRATION_BOOL);
    m_objAsset.bUseSecurityServerPortInInitialRegistration = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_USE_SECURITY_SERVER_PORT_IN_INITIAL_REGISTRATION_BOOL);
    m_objAsset.bRemoveOldSaOnEstablishingSa = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_REMOVE_OLD_SA_ON_ESTABLISHING_SA_BOOL);
    m_objAsset.bUseGGsmaRcsTelephonyFeatureTagAsAvailableVoiceCallType = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_USE_G_GSMA_RCS_TELEPHONY_FEATURE_TAG_AS_AVAILABLE_VOICE_CALL_TYPE_BOOL);
    m_objAsset.objPcscfDiscoveryMethodRoaming = piCc->GetIntArray(
            CarrierConfig::Assets::KEY_PCSCF_DISCOVERY_METHOD_ROAMING_INT_ARRAY);
    m_objAsset.bHoldRegistrationWhenIpcanChangedWithImsActiveCall = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_HOLD_REGISTRATION_WHEN_IPCAN_CHANGED_WITH_IMS_ACTIVE_CALL_BOOL);
    m_objAsset.bSupportVerstatForRegistration = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_SUPPORT_VERSTAT_FOR_REGISTRATION_BOOL);
    m_objAsset.bNoInitialRegistrationOnPcscfChange = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_NO_INITIAL_REGISTRATION_ON_PCSCF_CHANGE_BOOL);
    m_objAsset.bEmergencyCallBasedOnPAssociatedUriOfNormalRegistration = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_EMERGENCY_CALL_BASED_ON_P_ASSOCIATED_URI_OF_NORMAL_REGISTRATION_BOOL);
    m_objAsset.bSipOverIpsecEnabledInRoaming = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_SIP_OVER_IPSEC_ENABLED_IN_ROAMING_BOOL);
    m_objAsset.bRegistrationContactValidation = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_REGISTRATION_CONTACT_VALIDATION_BOOL);
    m_objAsset.nRegistrationOutOfServicePolicy = piCc->GetInt(
            CarrierConfig::Assets::KEY_REGISTRATION_OUT_OF_SERVICE_POLICY_INT);
    m_objAsset.objVowifiSubscriptionErrorCodeWithInitialRegistration = piCc->GetIntArray(
            CarrierConfig::Assets::KEY_VOWIFI_SUBSCRIPTION_ERROR_CODE_WITH_INITIAL_REGISTRATION_INT_ARRAY);
    m_objAsset.objSubscriptionErrorCodeForStoppingByExpirationTime = piCc->GetIntArray(
            CarrierConfig::Assets::KEY_SUBSCRIPTION_ERROR_CODE_FOR_STOPPING_BY_EXPIRATION_TIME_INT_ARRAY);
    m_objAsset.objSubscriptionErrorCodeForRegEventWithInitialRegistrationWithNextPcscf =
            piCc->GetIntArray(
            CarrierConfig::Assets::KEY_SUBSCRIPTION_ERROR_CODE_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_WITH_NEXT_PCSCF_INT_ARRAY);
    m_objAsset.bIgnoreVopsForVolteEnable = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_IGNORE_VOPS_FOR_VOLTE_ENABLE_BOOL);
    m_objAsset.bSmsOverImsAvailableInNoVops = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_SMS_OVER_IMS_AVAILABLE_IN_NO_VOPS_BOOL);
    m_objAsset.bImsDeregisterOn3gNetworks = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_IMS_DEREGISTER_ON_3G_NETWORKS_BOOL);
    m_objAsset.bVideoOverWifiSupportedWithoutVoice = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_VIDEO_OVER_WIFI_SUPPORTED_WITHOUT_VOICE_BOOL);
    m_objAsset.bRequiredEmergencyRegistrationInRoaming = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_REQUIRED_EMERGENCY_REGISTRATION_IN_ROAMING_BOOL);
    m_objAsset.objClearPermanentPdnFailure = piCc->GetIntArray(
            CarrierConfig::Assets::KEY_CLEAR_PERMANENT_PDN_FAILURE_INT_ARRAY);
    for (int i = 0; i < m_objAsset.objClearPermanentPdnFailure.GetSize(); i++)
    {
        m_nClearPermanentPdnFailure |= 0x1 << m_objAsset.objClearPermanentPdnFailure.GetAt(i);
    }
    m_objAsset.bUseWfcCountryCodeAvailabilityCheck = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_USE_WFC_COUNTRY_CODE_AVAILABILITY_CHECK_BOOL);
    m_objAsset.bRequiredVolteBlockBySetting = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_REQUIRED_VOLTE_BLOCK_BY_SETTING_BOOL);
    m_objAsset.bRequiredVolteBlockByAirplaneMode = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_REQUIRED_VOLTE_BLOCK_BY_AIRPLANE_MODE_BOOL);
    m_objAsset.bRequiredWfcBlockByAirplaneMode = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_REQUIRED_WFC_BLOCK_BY_AIRPLANE_MODE_BOOL);
    m_objAsset.objSupportedRoamingRats = piCc->GetIntArray(
            CarrierConfig::Assets::KEY_SUPPORTED_ROAMING_RATS_INT_ARRAY);
    m_objAsset.nEmergencyPreferredIpType = piCc->GetInt(
            CarrierConfig::Assets::KEY_EMERGENCY_PREFERRED_IPTYPE_INT);
    m_objAsset.nSipMessageThresholdForTransportChange = piCc->GetInt(
            CarrierConfig::Assets::KEY_SIP_MESSAGE_THRESHOLD_FOR_TRANSPORT_CHANGE_INT);
    m_objAsset.bCdmalessFeatureTagRequired = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_CDMALESS_FEATURE_TAG_REQUIRED_BOOL);
}

PRIVATE
void AosNConfiguration::InitIpsecAlgorithm(IN const ICarrierConfig* piCc)
{
    IMSVector<IMS_SINT32> objAuthAlgo = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_IPSEC_AUTHENTICATION_ALGORITHMS_INT_ARRAY);

    for (int i = 0; i < objAuthAlgo.GetSize(); i++)
    {
        m_objCarrierConfig.objIpsecAuthenticationAlgorithms.Push(objAuthAlgo.GetAt(i));
    }

    IMSVector<IMS_SINT32> objEncryAlgo = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_IPSEC_ENCRYPTION_ALGORITHMS_INT_ARRAY);

    for (int i = 0; i < objEncryAlgo.GetSize(); i++)
    {
        if (objEncryAlgo.GetAt(i) == CarrierConfig::Ims::IPSEC_ENCRYPTION_ALGORITHM_AES_CBC)
        {
            m_objCarrierConfig.objIpsecEncryptionAlgorithms.Push(
                    IpSecType::ENCRYPTION_ALGORITHM_AES_CBC);
        }
        else if (objEncryAlgo.GetAt(i) ==
                CarrierConfig::Ims::IPSEC_ENCRYPTION_ALGORITHM_DES_EDE3_CBC)
        {
            m_objCarrierConfig.objIpsecEncryptionAlgorithms.Push(
                    IpSecType::ENCRYPTION_ALGORITHM_DES_EDE3_CBC);
        }
        else // IPSEC_ENCRYPTION_ALGORITHM_NULL
        {
            m_objCarrierConfig.objIpsecEncryptionAlgorithms.Push(
                    IpSecType::ENCRYPTION_ALGORITHM_NO);
        }

    }
}
