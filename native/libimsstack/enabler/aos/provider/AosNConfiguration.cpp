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

#include "interface/IAosNConfigurationListener.h"
#include "provider/AosNConfiguration.h"

__IMS_TRACE_TAG_AOS__;

#define LOGTAG m_strLogTag.GetStr()

PUBLIC
AosNConfiguration::AosNConfiguration() :
        m_nSlotId(IMS_SLOT_0),
        m_objAsset(AosAsset()),
        m_objCarrierConfig(AosCarrierConfig()),
        m_objMmtelProvisioning(AosMmtelRequiresProvisioningBundle()),
        m_objExtraRegErr(AosExtraRegErrBundle()),
        m_objNotifyTerminated(AosNotifyTerminatedForInitRegBundle()),
        m_objPcscfRecoveryConditions(AosPcscfRecoveryConditionsBundle()),
        m_objRegErrCodeWithRaTime(AosRegErrCodeWithRaTimeBundle()),
        m_objRegRetryInterval(AosRegRetryIntervalBundle()),
        m_objSubErrCodeForInitReg(AosSubErrCodeForInitRegBundle()),
        m_objSubErrCodeForTerminated(AosSubErrCodeForTerminatedBundle()),
        m_objWfcErrMessage(AosWfcErrMessageBundle()),
        m_nEventForInitRegOnTerminatedState(0),
        m_nEventToFollowWtForInitRegOnTerminatedState(0),
        m_objListeners(ImsList<IAosNConfigurationListener*>())
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosNConfiguration = %" PFLS_u "/%" PFLS_x,
            sizeof(AosNConfiguration), this, 0);
}

PUBLIC VIRTUAL AosNConfiguration::~AosNConfiguration()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : AosNConfiguration = %" PFLS_u "/%" PFLS_x,
            sizeof(AosNConfiguration), this, 0);

    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    if (piCc != IMS_NULL)
    {
        piCc->RemoveListener(this);
    }

    m_objListeners.Clear();
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetSlotId() const
{
    return m_nSlotId;
}

PUBLIC VIRTUAL void AosNConfiguration::SetListener(IN IAosNConfigurationListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        const IAosNConfigurationListener* piCurrListener = m_objListeners.GetAt(i);

        if (piCurrListener == piListener)
        {
            return;
        }
    }

    m_objListeners.Append(piListener);

    A_IMS_TRACE_D(LOGTAG, "SetListener :: Listener (%" PFLS_x ") is set", piListener, 0, 0);
}

PUBLIC VIRTUAL void AosNConfiguration::RemoveListener(IN IAosNConfigurationListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        IAosNConfigurationListener* piCurrListener = m_objListeners.GetAt(i);

        if (piCurrListener == piListener)
        {
            m_objListeners.RemoveAt(i);

            A_IMS_TRACE_D(
                    LOGTAG, "RemoveListener - Listener (%" PFLS_x ") is removed", piListener, 0, 0);
            return;
        }
    }
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsTcpRequiredForReg() const
{
    return m_objCarrierConfig.bTcpRequiredForReg;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsSubscription() const
{
    return m_objCarrierConfig.bRegistrationEventPackageSupported;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsUnSubscription() const
{
    return m_objCarrierConfig.bUnsubscribeRegistrationEventPackage;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsVoLteAvailable() const
{
    return m_objCarrierConfig.bCarrierVolteAvailable;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsVoLteRoamingAvailable() const
{
    return m_objCarrierConfig.bCarrierVolteRoamingAvailable;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsVtAvailable() const
{
    return m_objCarrierConfig.bCarrierVtAvailable;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsWfcImsAvailable() const
{
    return m_objCarrierConfig.bCarrierWfcImsAvailable;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsImsSingleRegistrationRequired() const
{
    return m_objCarrierConfig.bImsSingleRegistrationRequired;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsRttSupported() const
{
    return m_objCarrierConfig.bRttSupported;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsRttSupportedWhileRoaming() const
{
    return m_objCarrierConfig.bRttSupportedWhileRoaming;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsSupportLimitedAdminSmsMode() const
{
    return m_objCarrierConfig.bSupportLimitedAdminSmsMode;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsNetworkInitiatedUssdOverImsSupported() const
{
    return m_objCarrierConfig.bNetworkInitiatedUssdOverImsSupported;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsVolteTtySupported() const
{
    return m_objCarrierConfig.bCarrierVolteTtySupported;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsVopsIgnoredForVolteEnabled() const
{
    return m_objAsset.bIgnoreVopsForVolteEnable;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsSmsOverImsAvailableWithoutVoiceCapability() const
{
    return m_objAsset.bSmsOverImsAvailableWithoutVoiceCapa;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsAnonymousECallActionSupported() const
{
    return m_objAsset.bSupportAnonymousECallAction;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsRequiredVolteBlockBySsac() const
{
    return m_objAsset.bRequiredVolteBlockBySsac;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsRequiredWfcBlockByAirplaneMode() const
{
    return m_objAsset.bRequiredWfcBlockByAirplaneMode;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsEmergencyReregSupportedOnIpcanChange() const
{
    return m_objAsset.bSupportEmergencyReregOnIpcanChange;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsGibaSupportedForERegInRoaming() const
{
    return m_objAsset.bSupportGibaForERegInRoaming;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsReregRetryWithChangedCountryOnWifi() const
{
    return m_objAsset.bReregWithChangedCountryOnWifi;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsSipOverIpsecInRoamingEnabled() const
{
    return m_objAsset.bSipOverIpsecEnabledInRoaming;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::UseWfcCountryCodeAvailabilityCheck() const
{
    return m_objAsset.bUseWfcCountryCodeAvailabilityCheck;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsRegRetryIntervalsUsedForSub() const
{
    return m_objRegRetryInterval.bUseRegRetryIntervalForSub;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsSmsOverIpEnabled() const
{
    // KEY_SMS_OVER_IP_ENABLED (ProvisioningManager)
    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsIpsecEnabled() const
{
    return m_objCarrierConfig.bSipOverIpsecEnabled;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsRegRetryRuleForERegUsed() const
{
    return m_objAsset.bUseRetryRuleForEReg;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsSecurityServerPortInRegContactOfInitRegUsed() const
{
    return m_objAsset.bUseSecurityServerPortInRegContactOfInitReg;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsSecurityServerPortInInitRegUsed() const
{
    return m_objAsset.bUseSecurityServerPortInInitReg;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsOldSaOnEstablishingSaRemoved() const
{
    return m_objAsset.bRemoveOldSaOnEstablishingSa;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsB2cCallComposerFeatureTagInRegContact() const
{
    return m_objAsset.bB2cCallComposerFeatureTagInRegContact;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsBlockPcscfOnRegFailure() const
{
    return m_objAsset.bBlockPcscfOnRegFailure;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsBlockRegOnCsCall() const
{
    return m_objAsset.bBlockRegOnCsCall;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsCallEndAndPdnReactivationByRegTerminated() const
{
    return m_objAsset.bCallEndAndPdnReactivationByRegTerminated;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsUnsecureTcpSocketOnAccomplishingRegDestroyed() const
{
    return m_objAsset.bDestroyUnsecureTcpSocketOnAccomplishingReg;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsSmsOverImsSupported() const
{
    return m_objCarrierConfig.bSmsOverImsSupported;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsImsOverNrEnabled() const
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

PUBLIC VIRTUAL IMS_BOOL
AosNConfiguration::IsEmergencyCallBasedOnPauOfNormalRegistrationSupported() const
{
    return m_objAsset.bEmcCallBasedOnPAssociatedUriOfNormalReg;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsEmcRegOnRandomPcscf() const
{
    return m_objAsset.bEmcRegOnRandomPcscf;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsERegWithOnlyTcpInRoaming() const
{
    return m_objAsset.bERegWithOnlyTcpInRoaming;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsERegUsingFirstImpuInIsim() const
{
    return m_objAsset.bERegUsingFirstImpuInIsim;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsRegWithIpcanChangedDuringImsCallHeld() const
{
    return m_objAsset.bHoldRegWithIpcanChangedDuringImsCall;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsDeregOn3gNetwork() const
{
    return m_objAsset.bImsDeregOn3gNetwork;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsImsiBasedUriPrioritized() const
{
    return m_objAsset.bImsiBasedUriPrioritized;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsIpsecInitializedWithNewPcscf() const
{
    return m_objAsset.bInitializeIpsecWithNewPcscf;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsInitSubUponSubTerminated() const
{
    return m_objAsset.bInitSubUponSubTerminated;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsKeepEPdnUponPcscfUnavailable() const
{
    return m_objAsset.bKeepEPdnUponPcscfUnavailable;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsKeepERegRetryOnWlanRequired() const
{
    return m_objAsset.bKeepERegRetryOnWlan;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsKeepRegRetryCntUponPdnReconnect() const
{
    return m_objAsset.bKeepRegRetryCntUponPdnReconnect;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsRegTimerForECallTimeoutAsFailure() const
{
    return m_objAsset.bRegTimerForECallTimeoutAsFailure;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsRegTimerForECallWithRatCheckEnabled() const
{
    return m_objAsset.bRegTimerForECallWithRatCheckEnabled;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsStopERegTimerOnEpdnConnected() const
{
    return m_objAsset.bStopERegTimerOnEpdnConnected;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsNoInitRegOnPcscfChange() const
{
    return m_objAsset.bNoInitRegOnPcscfChange;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsVideoOverWifiSupportedWithoutVoice() const
{
    return m_objAsset.bVideoOverWifiSupportedWithoutVoice;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsGeolocationPidfSupported(
        IN IMS_SINT32 nGeolocationPidfType) const
{
    const ImsVector<IMS_SINT32>& objGeolocationPidfTypes =
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

PUBLIC VIRTUAL IMS_BOOL
AosNConfiguration::IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType() const
{
    return m_objAsset.bUseRcsTelephonyFeatureTagAsAvailableVoiceCallType;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsCdmalessFeatureTagRequired() const
{
    return m_objAsset.bRequiredCdmalessFeatureTag;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsRegErrCodeWithRetryAfterTimeOnlyDefined() const
{
    return m_objRegErrCodeWithRaTime.bRegErrCodeWithRaTimeOnlyDefined;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsExtraReregErrInRoamingAsFailureHandled() const
{
    return m_objExtraRegErr.bExtraReregFailureWithErrCodeInRoaming;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsExtraRegErrRetryCntSharedForRegAndSubRequired() const
{
    return m_objExtraRegErr.bExtraRegErrRetryCntSharedForRegAndSub;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsEmergencyCallbackModeSupported() const
{
    return m_objCarrierConfig.bEmergencyCallbackModeSupported;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsEmergencySmsOverImsSupported() const
{
    return m_objCarrierConfig.bSupportEmergencySmsOverIms;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsContactUriValidationChecked() const
{
    return m_objAsset.bRegContactValidation;
}

PUBLIC VIRTUAL IMS_BOOL
AosNConfiguration::IsPlmnBlockWithTimeoutOnFailureWithAllPcscfsSupported() const
{
    return m_objAsset.bPlmnBlockWithTimeoutOnFailureWithAllPcscfs;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsRegRetryWithIpVerFallback() const
{
    return m_objAsset.bRegRetryWithIpVerFallback;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsReleaseEPdnOfUnavailableNetwork() const
{
    return m_objAsset.bReleaseEPdnOfUnavailableNetwork;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsReleaseEPdnUponECallEndInFakeMode() const
{
    return m_objAsset.bReleaseEPdnUponECallEndInFakeMode;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsRegRequiredAfterImsCallEndOnRegHeld() const
{
    return m_objAsset.bRequiredInitRegAfterImsCallEndOnRegHeld;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsRegRequiredAfterImsECallEndOnRegHeld() const
{
    return m_objAsset.bRequiredInitRegAfterImsECallEndOnRegHeld;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsRegWithFeatureTagUnavailableSupported() const
{
    return m_objAsset.bSupportRegWithFeatureTagUnavailable;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsVerstatForRegistrationSupported() const
{
    return m_objAsset.bSupportVerstatForReg;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsVerstatSupportedBasedOnNetworkForReg() const
{
    return m_objAsset.bSupportVerstatBasedOnNetworkForReg;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsPlmnBlockWithTimeoutOnVoiceCallUnavailable() const
{
    return m_objAsset.bPlmnBlockWithTimeoutOnVoiceCallUnavailable;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsWfcErrorMessageSupported(IN IMS_SINT32 nError) const
{
    switch (nError)
    {
        case CarrierConfig::ImsWfc::WFC_ERROR_REG_403:
            return !m_objWfcErrMessage.strWfcErrorReg403.IsEmpty();
        case CarrierConfig::ImsWfc::WFC_ERROR_REG_500:
            return !m_objWfcErrMessage.strWfcErrorReg500.IsEmpty();
        case CarrierConfig::ImsWfc::WFC_ERROR_NOT_SUPPORTED_COUNTRY:
            return !m_objWfcErrMessage.strWfcErrorNotSupportedCountry.IsEmpty();
        case CarrierConfig::ImsWfc::WFC_ERROR_SUB_403:
            return !m_objWfcErrMessage.strWfcErrorSub403.IsEmpty();
        case CarrierConfig::ImsWfc::WFC_ERROR_NOTIFY_TERMINATED:
            return !m_objWfcErrMessage.strWfcErrorNotifyTerminated.IsEmpty();
        case CarrierConfig::ImsWfc::WFC_ERROR_OTHER_FAILURES:
            return !m_objWfcErrMessage.strWfcErrorOtherFailures.IsEmpty();
        default:
            return IMS_FALSE;
    }
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsVideoSupportedForEmergencyReg() const
{
    return m_objAsset.bSupportVideoForEmergencyReg;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsUseRegInfoContactWithoutUriCheck() const
{
    return m_objAsset.bUseRegInfoContactWithoutUriCheck;
}

PUBLIC VIRTUAL IMS_BOOL AosNConfiguration::IsTestModeEnabled(IN IMS_SINT32 nType) const
{
    return m_objAsset.objTestMode.Contains(nType);
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetSipTimerT1()
{
    return static_cast<IMS_SINT32>(m_objCarrierConfig.nSipTimerT1Millis);
}

PUBLIC VIRTUAL IMS_UINT32 AosNConfiguration::GetRegistrationRetryBaseTime()
{
    return static_cast<IMS_UINT32>(m_objCarrierConfig.nRegistrationRetryBaseTimerMillis);
}

PUBLIC VIRTUAL IMS_UINT32 AosNConfiguration::GetRegistrationRetryMaxTime()
{
    return static_cast<IMS_UINT32>(m_objCarrierConfig.nRegistrationRetryMaxTimerMillis);
}

PUBLIC VIRTUAL IMS_UINT32 AosNConfiguration::GetIsimIndexForImpu()
{
    return static_cast<IMS_UINT32>(m_objCarrierConfig.nIsimIndexForImpu);
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetImsEstablishmentTime() const
{
    return m_objAsset.nImsEstablishmentTimeSec;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetPreferredImsDscp() const
{
    return m_objCarrierConfig.nPreferredImsDscp;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegistrationPreferredAccessTypeFeatureTag() const
{
    return m_objCarrierConfig.nRegistrationPreferredAccesstypeFeatureTag;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetUssdMethod() const
{
    return m_objCarrierConfig.nCarrierUssdMethod;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetPreferredIpType() const
{
    return m_objAsset.nImsPreferredIpType;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetEmergencyPreferredIpType() const
{
    return m_objAsset.nEmcPreferredIpType;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetEmcRegRetryMaxCnt() const
{
    return m_objAsset.nEmcRegRetryMaxCnt;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetEmcRegRetryTimerMillis() const
{
    return m_objAsset.nEmcRegRetryTimerMillis;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetPcscfPort() const
{
    return m_objCarrierConfig.nSipServerPortNumber;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetSipPreferredTransport() const
{
    return m_objCarrierConfig.nSipPreferredTransport;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetIpv4MtuSize() const
{
    return m_objCarrierConfig.nIpv4SipMtuSizeCellular;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetIpv6MtuSize() const
{
    return m_objCarrierConfig.nIpv6SipMtuSizeCellular;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd() const
{
    return m_objCarrierConfig.nIpcanReleaseEmergencyPdnUponEmergencyCallEnd;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetPreferredEmergencyRegistration() const
{
    return m_objCarrierConfig.nPreferredEmergencyRegistration;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetWaitTimeMillisForReleaseEPdnAfterECallEnd() const
{
    return m_objCarrierConfig.nWaitTimeMillisForReleaseEPdnAfterECallEnd;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetEmergencyRegistrationTimerMillis() const
{
    return m_objCarrierConfig.nEmergencyRegistrationTimerMillis;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetImsSignallingDscp() const
{
    return m_objAsset.nImsSignallingDscp;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegistrationPrivateHeader() const
{
    return m_objCarrierConfig.nRegistrationPrivateHeader;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegActualWaitTimePolicy() const
{
    return m_objAsset.nRegActualWaitTimePolicy;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegDefaultWaitTime() const
{
    return m_objAsset.nRegDefaultWaitTime;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegOutOfServicePolicy() const
{
    return m_objAsset.nRegOutOfServicePolicy;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRoamingPreferredEmcReg() const
{
    return m_objAsset.nRoamingPreferredEmcReg;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetSipMessageThresholdForTransportChange() const
{
    return m_objAsset.nSipMessageThresholdForTransportChange;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetSubRetrySip503CodePolicy() const
{
    return m_objAsset.nSubRetry503Policy;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetUsatRegEventDownloadPolicy() const
{
    return m_objAsset.nUsatRegEventDownloadPolicy;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetVolteHysTime() const
{
    return m_objAsset.nVolteHysTimeSec;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegRetrySip305CodePolicy() const
{
    return m_objAsset.nRegRetry305Policy;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetReregRetrySip305CodePolicy() const
{
    return m_objAsset.nReregRetry305Policy;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegRetrySip503CodePolicy() const
{
    return m_objAsset.nRegRetry503Policy;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegRetryCountOnSinglePcscf() const
{
    return m_objAsset.nRegRetryCntOnSinglePcscf;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegRetryCountPerPcscf() const
{
    return m_objAsset.nRegRetryCntPerPcscf;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegRetryCountResetPolicy() const
{
    return m_objAsset.nRegRetryCntResetPolicy;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegRetryCountWithIpsecOnAuthFailure() const
{
    return m_objAsset.nRegRetryCntWithIpsecOnAuthFailure;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegRetryDefaultPolicy() const
{
    return m_objAsset.nRegRetryDefaultPolicy;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegRetryTimerFPolicy() const
{
    return m_objAsset.nRegRetryTimerFPolicy;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegTimerForEmcCall() const
{
    return m_objAsset.nRegTimerForEmcCallMillis;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetExtraRegErrFinalType() const
{
    return m_objExtraRegErr.nExtraRegErrFinalType;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetExtraRegErrPolicy() const
{
    return m_objExtraRegErr.nExtraRegErrPolicy;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetExtraRegErrMaxCount() const
{
    return m_objExtraRegErr.nExtraRegErrMaxCnt;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRegistrationPcscfUpdatePolicy() const
{
    return m_objAsset.nRegPcscfUpdatePolicy;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetUserInfoPolicyForNonRegisterMessage() const
{
    return m_objAsset.nContactUserInfoPolicyForNonRegMessage;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetGeolocationPidfFormingPolicy() const
{
    return m_objAsset.nGeolocationPidfFormingPolicy;
}

PUBLIC VIRTUAL IMS_SINT32
AosNConfiguration::GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached() const
{
    return m_objExtraRegErr.nExtraRegErrPcscfsRepeatedCntForLteCombinedAttached;
}

PUBLIC VIRTUAL IMS_SINT32
AosNConfiguration::GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached() const
{
    return m_objExtraRegErr.nExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetRegRetryIntervals()
{
    return m_objRegRetryInterval.objRegRetryIntervalSec;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetRegRandomRetryIntervals()
{
    return m_objRegRetryInterval.objRegRetryRandomUpperValueSec;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetIpsecAuthenticationAlgorithms()
{
    return m_objCarrierConfig.objIpsecAuthenticationAlgorithms;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetIpsecEncryptionAlgorithms()
{
    return m_objCarrierConfig.objIpsecEncryptionAlgorithms;
}

PUBLIC VIRTUAL IMS_UINT32 AosNConfiguration::GetNotifyEventForInitialRegistration() const
{
    return m_nEventForInitRegOnTerminatedState;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetNotifyWaitTime() const
{
    return m_objNotifyTerminated.nWaitTimeForInitRegOnTerminatedState;
}

PUBLIC VIRTUAL IMS_UINT32 AosNConfiguration::GetNotifyEventForInitialRegWithWaitTime() const
{
    return m_nEventToFollowWtForInitRegOnTerminatedState;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetPcscfRecoveryMaxRetryCnt() const
{
    return m_objPcscfRecoveryConditions.nMaxRetryCnt;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetPcscfRecoveryWaitTime() const
{
    return m_objPcscfRecoveryConditions.nWaitTime;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetPcscfRecoveryBaseTime() const
{
    return m_objPcscfRecoveryConditions.nBaseTime;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetPcscfRecoveryMaxTime() const
{
    return m_objPcscfRecoveryConditions.nMaxTime;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRetryCountSubErrorRegRequired() const
{
    return m_objSubErrCodeForInitReg.nSubErrCodeForInitRegWithRetryMaxCnt;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetSubErrorRegRequired()
{
    return m_objSubErrCodeForInitReg.objSubErrCodeForInitReg;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetSubErrorRegRequiredWithNextPcscf()
{
    return m_objAsset.objSubErrorCodeForInitRegWithNextPcscf;
}

PUBLIC VIRTUAL IMS_SINT32 AosNConfiguration::GetRetryCountSubErrorSubTerminated() const
{
    return m_objSubErrCodeForTerminated.nSubErrCodeForTerminatedRetryMaxCnt;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetSubErrorSubTerminated()
{
    return m_objSubErrCodeForTerminated.objSubErrCodeForTerminated;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetSubErrorStoppingResub()
{
    return m_objAsset.objSubErrorCodeForStoppingByExpirationTime;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetVowifiSubErrorRegRequired()
{
    return m_objAsset.objVowifiSubErrorCodeForInitReg;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetImsIdentityPriority()
{
    return m_objCarrierConfig.objImsIdentityPriority;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetPcscfDiscoveryMethod()
{
    return m_objCarrierConfig.objPcscfDiscoveryMethod;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetUpdateRegistrationWithRatChange()
{
    return m_objCarrierConfig.objUpdateRegistrationWithRatChange;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetSupportedRats()
{
    return m_objCarrierConfig.objSupportedRats;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetSupportedRoamingRats()
{
    return m_objAsset.objSupportedRoamingRats;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetSmsOverImsSupportedRats()
{
    return m_objCarrierConfig.objSmsOverImsSupportedRats;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetEmergencyOverImsSupportedRats()
{
    return m_objCarrierConfig.objEmergencyOverImsSupportedRats;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetExtraRegErrCode()
{
    return m_objExtraRegErr.objExtraRegErrCode;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetExtraReregErrCode()
{
    return m_objExtraRegErr.objExtraReregErrCode;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetExtraRegErrWaitTime()
{
    return m_objExtraRegErr.objExtraRegErrWaitTimeSec;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>&
AosNConfiguration::GetReregRetryErrCodeForInitRegWithSamePcscf()
{
    return m_objAsset.objReregRetryErrCodeForInitRegWithSamePcscf;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetRegPermanentErrCode()
{
    return m_objCarrierConfig.objRegistrationPermanentErrorCode;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetRegPermanentErrMaxCount()
{
    return m_objAsset.objRegPermanentErrMaxCnt;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetRegErrCodeWithoutIpsec()
{
    return m_objAsset.objRegRetryErrCodeWithoutIpsec;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetRegErrCodeWithRetryAfterTime()
{
    return m_objRegErrCodeWithRaTime.objRegErrCodeWithRaTime;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetReregErrCodeWithRetryAfterTime()
{
    return m_objRegErrCodeWithRaTime.objReregErrCodeWithRaTime;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetRegErrCodeForPcscfDiscovery()
{
    return m_objAsset.objRegErrCodeForPcscfDiscovery;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetReregErrCodeForCallEnd()
{
    return m_objAsset.objReregErrCodeForCallEnd;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>&
AosNConfiguration::GetReregErrCodeForInitRegWithAvailablePcscf()
{
    return m_objAsset.objReregErrCodeForInitRegWithAvailablePcscf;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetReregErrCodeForImsPdnReactivation()
{
    return m_objAsset.objReregErrCodeForImsPdnReactivation;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetUnavailableFeaturesInLimitedReg()
{
    return m_objAsset.objUnavailableFeaturesInLimitedReg;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32>& AosNConfiguration::GetERegErrCodeNotSupportedCommonPolicy()
{
    return m_objAsset.objERegErrCodeNotSupportedCommonPolicy;
}

PRIVATE VIRTUAL void AosNConfiguration::CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId)
{
    if (m_nSlotId != nSlotId)
    {
        return;
    }

    A_IMS_TRACE_I(LOGTAG, "CarrierConfig_NotifyConfigChanged :: updated", 0, 0, 0);

    const ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);

    if (piCc == IMS_NULL)
    {
        A_IMS_TRACE_I(LOGTAG, "Init :: config failed", 0, 0, 0);
        return;
    }

    InitConfig(piCc);
    InitAssetsConfig(piCc);
    InitBundle(piCc);

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        IAosNConfigurationListener* piListener = m_objListeners.GetAt(i);

        if (piListener == IMS_NULL)
        {
            continue;
        }

        piListener->NConfiguration_NotifyConfigChanged();
    }
}

PRIVATE VIRTUAL void AosNConfiguration::Init(IN IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    m_strLogTag.Sprintf("%d:aos", nSlotId);

    A_IMS_TRACE_D(LOGTAG, "Init", 0, 0, 0);

    m_nSlotId = nSlotId;

    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    if (piCc == IMS_NULL)
    {
        A_IMS_TRACE_I(LOGTAG, "Init :: config failed", 0, 0, 0);
        return;
    }

    piCc->AddListener(this);

    InitConfig(piCc);
    InitAssetsConfig(piCc);
    InitBundle(piCc);
}

PROTECTED
void AosNConfiguration::InitBundle(IN const ICarrierConfig* piCc)
{
    // AosExtraRegErrBundle
    ICarrierConfig* piCcBundle = piCc->GetBundle(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objExtraRegErr.bExtraReregFailureWithErrCodeInRoaming = piCcBundle->GetBoolean(
                CarrierConfig::Ims::KEY_EXTRA_REG_ERR_CODE_AS_FAILURE_IN_ROAMING_FOR_UPDATE_BOOL);
        m_objExtraRegErr.bExtraRegErrRetryCntSharedForRegAndSub = piCcBundle->GetBoolean(
                CarrierConfig::Ims::KEY_EXTRA_REG_ERR_RETRY_CNT_SHARED_FOR_REG_AND_SUB_BOOL);
        m_objExtraRegErr.nExtraRegErrFinalType =
                piCcBundle->GetInt(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_FINAL_TYPE_INT);
        m_objExtraRegErr.nExtraRegErrMaxCnt =
                piCcBundle->GetInt(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_MAX_CNT_INT);
        m_objExtraRegErr.nExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached = piCcBundle->GetInt(
                CarrierConfig::Ims::
                        KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_EPS_5GS_ONLY_ATTACHED_INT);
        m_objExtraRegErr.nExtraRegErrPcscfsRepeatedCntForLteCombinedAttached = piCcBundle->GetInt(
                CarrierConfig::Ims::
                        KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_LTE_COMBINED_ATTACHED_INT);
        m_objExtraRegErr.nExtraRegErrPolicy =
                piCcBundle->GetInt(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_POLICY_INT);
        m_objExtraRegErr.objExtraRegErrCode =
                piCcBundle->GetIntArray(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_CODE_INT_ARRAY);
        m_objExtraRegErr.objExtraReregErrCode = piCcBundle->GetIntArray(
                CarrierConfig::Ims::KEY_EXTRA_REG_ERR_CODE_FOR_UPDATE_INT_ARRAY);
        m_objExtraRegErr.objExtraRegErrWaitTimeSec = piCcBundle->GetIntArray(
                CarrierConfig::Ims::KEY_EXTRA_REG_ERR_WAIT_TIME_SEC_INT_ARRAY);
        piCcBundle->ReleaseBundle();
#ifdef __IMS_DEBUG__
        A_IMS_TRACE_D(LOGTAG, "KEY_EXTRA_REG_ERR_BUNDLE :: FinalType(%d), Policy(%d), MaxCnt(%d)",
                m_objExtraRegErr.nExtraRegErrFinalType, m_objExtraRegErr.nExtraRegErrPolicy,
                m_objExtraRegErr.nExtraRegErrMaxCnt);
        A_IMS_TRACE_D(LOGTAG, "RetryCntShared(%d), ReregRoaming(%d)",
                m_objExtraRegErr.bExtraRegErrRetryCntSharedForRegAndSub,
                m_objExtraRegErr.bExtraReregFailureWithErrCodeInRoaming, 0);
        A_IMS_TRACE_D(LOGTAG, "Pcscfs Repeated Cnt: EPS 5GS only(%d), LTE Combined(%d)",
                m_objExtraRegErr.nExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached,
                m_objExtraRegErr.nExtraRegErrPcscfsRepeatedCntForLteCombinedAttached, 0);
        IMS_UINT32 nSize = m_objExtraRegErr.objExtraRegErrCode.GetSize();
        for (int i = 0; i < nSize; i++)
        {
            IMS_SINT32 nValue = m_objExtraRegErr.objExtraRegErrCode.GetAt(i);
            A_IMS_TRACE_D(LOGTAG, "RegErrCode(%d), ", nValue, 0, 0);
        }
        nSize = m_objExtraRegErr.objExtraReregErrCode.GetSize();
        for (int i = 0; i < nSize; i++)
        {
            IMS_SINT32 nValue = m_objExtraRegErr.objExtraReregErrCode.GetAt(i);
            A_IMS_TRACE_D(LOGTAG, "ReregErrCode(%d), ", nValue, 0, 0);
        }
        nSize = m_objExtraRegErr.objExtraRegErrWaitTimeSec.GetSize();
        for (int i = 0; i < nSize; i++)
        {
            IMS_SINT32 nValue = m_objExtraRegErr.objExtraRegErrWaitTimeSec.GetAt(i);
            A_IMS_TRACE_D(LOGTAG, "ExtraWaitTimeSec(%d), ", nValue, 0, 0);
        }
#endif
    }

    // AosNotifyTerminatedForInitRegBundle
    piCcBundle = piCc->GetBundle(CarrierConfig::Ims::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objNotifyTerminated.nWaitTimeForInitRegOnTerminatedState = piCcBundle->GetInt(
                CarrierConfig::Ims::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_WITH_WAIT_TIME_INT);
        m_objNotifyTerminated.objEventForInitRegOnTerminatedState = piCcBundle->GetIntArray(
                CarrierConfig::Ims::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY);
        m_objNotifyTerminated.objEventWithWtForInitRegOnTerminatedState = piCcBundle->GetIntArray(
                CarrierConfig::Ims::
                        KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_WITH_WAIT_TIME_INT_ARRAY);
        piCcBundle->ReleaseBundle();
#ifdef __IMS_DEBUG__
        A_IMS_TRACE_D(LOGTAG,
                "KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE :: "
                "WTFIROTS(%d)",
                m_objNotifyTerminated.nWaitTimeForInitRegOnTerminatedState, 0, 0);
        IMS_UINT32 nSize = m_objNotifyTerminated.objEventForInitRegOnTerminatedState.GetSize();
        for (int i = 0; i < nSize; i++)
        {
            IMS_SINT32 nValue = m_objNotifyTerminated.objEventForInitRegOnTerminatedState.GetAt(i);
            A_IMS_TRACE_D(LOGTAG, "EFIROTS(%d), ", nValue, 0, 0);
        }
        nSize = m_objNotifyTerminated.objEventWithWtForInitRegOnTerminatedState.GetSize();
        for (int i = 0; i < nSize; i++)
        {
            IMS_SINT32 nValue =
                    m_objNotifyTerminated.objEventWithWtForInitRegOnTerminatedState.GetAt(i);
            A_IMS_TRACE_D(LOGTAG, "ETFWFIROTS(%d), ", nValue, 0, 0);
        }
#endif
        ImsVector<IMS_SINT32>& objNotifyEvents =
                m_objNotifyTerminated.objEventForInitRegOnTerminatedState;
        m_nEventForInitRegOnTerminatedState = 0;
        for (int i = 0; i < objNotifyEvents.GetSize(); i++)
        {
            m_nEventForInitRegOnTerminatedState |= 0x1 << (objNotifyEvents.GetAt(i) - 1);
        }

        ImsVector<IMS_SINT32>& objEventToFollow =
                m_objNotifyTerminated.objEventWithWtForInitRegOnTerminatedState;
        m_nEventToFollowWtForInitRegOnTerminatedState = 0;
        for (int i = 0; i < objEventToFollow.GetSize(); i++)
        {
            m_nEventToFollowWtForInitRegOnTerminatedState |= 0x1 << (objEventToFollow.GetAt(i) - 1);
        }
    }

    // AosPcscfRecoveryConditionsBundle
    piCcBundle = piCc->GetBundle(CarrierConfig::Ims::KEY_PCSCF_RECOVERY_CONDITIONS_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objPcscfRecoveryConditions.nMaxRetryCnt =
                piCcBundle->GetInt(CarrierConfig::Ims::KEY_PCSCF_RECOVERY_MAX_CNT_INT);
        m_objPcscfRecoveryConditions.nWaitTime =
                piCcBundle->GetInt(CarrierConfig::Ims::KEY_PCSCF_RECOVERY_WAIT_TIME_SEC_INT);
        m_objPcscfRecoveryConditions.nBaseTime =
                piCcBundle->GetInt(CarrierConfig::Ims::KEY_PCSCF_RECOVERY_BASE_TIME_SEC_INT);
        m_objPcscfRecoveryConditions.nMaxTime =
                piCcBundle->GetInt(CarrierConfig::Ims::KEY_PCSCF_RECOVERY_MAX_TIME_SEC_INT);
        piCcBundle->ReleaseBundle();
#ifdef __IMS_DEBUG__
        A_IMS_TRACE_D(LOGTAG, "KEY_PCSCF_RECOVERY_CONDITIONS_BUNDLE :: MRC(%d), WT(%d)",
                m_objPcscfRecoveryConditions.nMaxRetryCnt, m_objPcscfRecoveryConditions.nWaitTime,
                0);
        A_IMS_TRACE_D(LOGTAG, "KEY_PCSCF_RECOVERY_CONDITIONS_BUNDLE :: BT(%d), MT(%d)",
                m_objPcscfRecoveryConditions.nBaseTime, m_objPcscfRecoveryConditions.nMaxTime, 0);
#endif
    }

    // AosRegErrCodeWithRaTimeBundle
    piCcBundle = piCc->GetBundle(CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objRegErrCodeWithRaTime.bRegErrCodeWithRaTimeOnlyDefined = piCcBundle->GetBoolean(
                CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_ONLY_DEFINED_BOOL);
        m_objRegErrCodeWithRaTime.objRegErrCodeWithRaTime = piCcBundle->GetIntArray(
                CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_INT_ARRAY);
        m_objRegErrCodeWithRaTime.objReregErrCodeWithRaTime = piCcBundle->GetIntArray(
                CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_FOR_UPDATE_INT_ARRAY);
        piCcBundle->ReleaseBundle();
#ifdef __IMS_DEBUG__
        A_IMS_TRACE_D(LOGTAG, "KEY_REG_ERR_CODE_WITH_RA_TIME_BUNDLE :: RECWRATOD(%d)",
                m_objRegErrCodeWithRaTime.bRegErrCodeWithRaTimeOnlyDefined, 0, 0);
        IMS_UINT32 nSize = m_objRegErrCodeWithRaTime.objRegErrCodeWithRaTime.GetSize();
        for (int i = 0; i < nSize; i++)
        {
            IMS_SINT32 nValue = m_objRegErrCodeWithRaTime.objRegErrCodeWithRaTime.GetAt(i);
            A_IMS_TRACE_D(LOGTAG, "RECWRAT(%d), ", nValue, 0, 0);
        }
        nSize = m_objRegErrCodeWithRaTime.objReregErrCodeWithRaTime.GetSize();
        for (int i = 0; i < nSize; i++)
        {
            IMS_SINT32 nValue = m_objRegErrCodeWithRaTime.objReregErrCodeWithRaTime.GetAt(i);
            A_IMS_TRACE_D(LOGTAG, "RRECWRAT(%d), ", nValue, 0, 0);
        }
#endif
    }

    // AosRegRetryIntervalBundle
    piCcBundle = piCc->GetBundle(CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objRegRetryInterval.bUseRegRetryIntervalForSub = piCcBundle->GetBoolean(
                CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_USED_FOR_SUB_BOOL);
        m_objRegRetryInterval.objRegRetryRandomUpperValueSec = piCcBundle->GetIntArray(
                CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_RANDOM_UPPER_VALUE_SEC_INT_ARRAY);
        m_objRegRetryInterval.objRegRetryIntervalSec =
                piCcBundle->GetIntArray(CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_SEC_INT_ARRAY);
        piCcBundle->ReleaseBundle();
#ifdef __IMS_DEBUG__
        A_IMS_TRACE_D(LOGTAG, "KEY_REG_RETRY_INTERVAL_BUNDLE :: URRIFSR(%d)",
                m_objRegRetryInterval.bUseRegRetryIntervalForSub, 0, 0);
        IMS_UINT32 nSize = m_objRegRetryInterval.objRegRetryRandomUpperValueSec.GetSize();
        for (int i = 0; i < nSize; i++)
        {
            IMS_SINT32 nValue = m_objRegRetryInterval.objRegRetryRandomUpperValueSec.GetAt(i);
            A_IMS_TRACE_D(LOGTAG, "RRRUVS(%d), ", nValue, 0, 0);
        }
        nSize = m_objRegRetryInterval.objRegRetryIntervalSec.GetSize();
        for (int i = 0; i < nSize; i++)
        {
            IMS_SINT32 nValue = m_objRegRetryInterval.objRegRetryIntervalSec.GetAt(i);
            A_IMS_TRACE_D(LOGTAG, "RRIS(%d), ", nValue, 0, 0);
        }
#endif
    }

    // AosSubErrCodeForInitRegBundle
    piCcBundle = piCc->GetBundle(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objSubErrCodeForInitReg.nSubErrCodeForInitRegWithRetryMaxCnt = piCcBundle->GetInt(
                CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_RETRY_MAX_CNT_INT);
        m_objSubErrCodeForInitReg.objSubErrCodeForInitReg = piCcBundle->GetIntArray(
                CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY);
        piCcBundle->ReleaseBundle();
#ifdef __IMS_DEBUG__
        A_IMS_TRACE_D(LOGTAG,
                "KEY_SUB_ERR_CODE_FOR_INIT_REG_BUNDLE :: "
                "SERMCWIR(%d)",
                m_objSubErrCodeForInitReg.nSubErrCodeForInitRegWithRetryMaxCnt, 0, 0);
        IMS_UINT32 nSize = m_objSubErrCodeForInitReg.objSubErrCodeForInitReg.GetSize();
        for (int i = 0; i < nSize; i++)
        {
            IMS_SINT32 nValue = m_objSubErrCodeForInitReg.objSubErrCodeForInitReg.GetAt(i);
            A_IMS_TRACE_D(LOGTAG, "SECWIR(%d), ", nValue, 0, 0);
        }
#endif
    }

    // AosSubErrCodeForTerminatedBundle
    piCcBundle = piCc->GetBundle(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_TERMINATED_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objSubErrCodeForTerminated.nSubErrCodeForTerminatedRetryMaxCnt = piCcBundle->GetInt(
                CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_TERMINATED_WITH_RETRY_MAX_CNT_INT);
        m_objSubErrCodeForTerminated.objSubErrCodeForTerminated = piCcBundle->GetIntArray(
                CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_TERMINATED_INT_ARRAY);
        piCcBundle->ReleaseBundle();
#ifdef __IMS_DEBUG__
        A_IMS_TRACE_D(LOGTAG, "KEY_SUB_ERR_CODE_FOR_TERMINATED_BUNDLE :: STECRM(%d)",
                m_objSubErrCodeForTerminated.nSubErrCodeForTerminatedRetryMaxCnt, 0, 0);
        IMS_UINT32 nSize = m_objSubErrCodeForTerminated.objSubErrCodeForTerminated.GetSize();
        for (int i = 0; i < nSize; i++)
        {
            IMS_SINT32 nValue = m_objSubErrCodeForTerminated.objSubErrCodeForTerminated.GetAt(i);
            A_IMS_TRACE_D(LOGTAG, "STECFRE(%d), ", nValue, 0, 0);
        }
#endif
    }

    // AosWfcErrMessageBundle
    piCcBundle = piCc->GetBundle(CarrierConfig::ImsWfc::KEY_WFC_ERR_MESSAGE_BUNDLE);
    if (piCcBundle != IMS_NULL)
    {
        m_objWfcErrMessage.strWfcErrorReg403 =
                piCcBundle->GetString(CarrierConfig::ImsWfc::KEY_WFC_ERR_REG_403_STRING);
        m_objWfcErrMessage.strWfcErrorReg500 =
                piCcBundle->GetString(CarrierConfig::ImsWfc::KEY_WFC_ERR_REG_500_STRING);
        m_objWfcErrMessage.strWfcErrorNotSupportedCountry = piCcBundle->GetString(
                CarrierConfig::ImsWfc::KEY_WFC_ERR_NOT_SUPPORTED_COUNTRY_STRING);
        m_objWfcErrMessage.strWfcErrorSub403 =
                piCcBundle->GetString(CarrierConfig::ImsWfc::KEY_WFC_ERR_SUB_403_STRING);
        m_objWfcErrMessage.strWfcErrorNotifyTerminated =
                piCcBundle->GetString(CarrierConfig::ImsWfc::KEY_WFC_ERR_NOTIFY_TERMINATED_STRING);
        m_objWfcErrMessage.strWfcErrorOtherFailures =
                piCcBundle->GetString(CarrierConfig::ImsWfc::KEY_WFC_ERR_OTHER_FAILURES_STRING);

        piCcBundle->ReleaseBundle();
#ifdef __IMS_DEBUG__
        A_IMS_TRACE_D(LOGTAG, "KEY_WFC_ERR_REG_403_STRING :: STECRM(%s)",
                m_objWfcErrMessage.strWfcErrorReg403.GetStr(), 0, 0);
        A_IMS_TRACE_D(LOGTAG, "KEY_WFC_ERR_REG_500_STRING :: STECRM(%s)",
                m_objWfcErrMessage.strWfcErrorReg500.GetStr(), 0, 0);
        A_IMS_TRACE_D(LOGTAG, "KEY_WFC_ERR_NOT_SUPPORTED_COUNTRY_STRING :: STECRM(%s)",
                m_objWfcErrMessage.strWfcErrorNotSupportedCountry.GetStr(), 0, 0);
        A_IMS_TRACE_D(LOGTAG, "KEY_WFC_ERR_SUB_403_STRING :: STECRM(%s)",
                m_objWfcErrMessage.strWfcErrorSub403.GetStr(), 0, 0);
        A_IMS_TRACE_D(LOGTAG, "KEY_WFC_ERR_NOTIFY_TERMINATED_STRING :: STECRM(%s)",
                m_objWfcErrMessage.strWfcErrorNotifyTerminated.GetStr(), 0, 0);
        A_IMS_TRACE_D(LOGTAG, "KEY_WFC_ERR_OTHER_FAILURES_STRING :: STECRM(%s)",
                m_objWfcErrMessage.strWfcErrorOtherFailures.GetStr(), 0, 0);
#endif
    }
}

PROTECTED
void AosNConfiguration::InitConfig(IN const ICarrierConfig* piCc)
{
    /* aosp_carrier_config */
    /// no prefix
    m_objCarrierConfig.bSupportEmergencySmsOverIms =
            piCc->GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL);
    m_objCarrierConfig.bCarrierVolteAvailable =
            piCc->GetBoolean(CarrierConfig::KEY_CARRIER_VOLTE_AVAILABLE_BOOL);
    m_objCarrierConfig.bCarrierVtAvailable =
            piCc->GetBoolean(CarrierConfig::KEY_CARRIER_VT_AVAILABLE_BOOL);
    m_objCarrierConfig.bCarrierWfcImsAvailable =
            piCc->GetBoolean(CarrierConfig::KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL);
    m_objCarrierConfig.bRttSupported = piCc->GetBoolean(CarrierConfig::KEY_RTT_SUPPORTED_BOOL);
    m_objCarrierConfig.bRttSupportedWhileRoaming =
            piCc->GetBoolean(CarrierConfig::KEY_RTT_SUPPORTED_WHILE_ROAMING_BOOL);
    m_objCarrierConfig.bCarrierCrossSimImsAvailable =
            piCc->GetBoolean(CarrierConfig::KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL);
    m_objCarrierConfig.bCarrierVolteTtySupported =
            piCc->GetBoolean(CarrierConfig::KEY_CARRIER_VOLTE_TTY_SUPPORTED_BOOL);
    m_objCarrierConfig.objCarrierNrAvailabilities =
            piCc->GetIntArray(CarrierConfig::KEY_CARRIER_NR_AVAILABILITIES_INT_ARRAY);
    m_objCarrierConfig.nCarrierUssdMethod =
            piCc->GetInt(CarrierConfig::KEY_CARRIER_USSD_METHOD_INT);
    /// ims.
    m_objCarrierConfig.objPcscfDiscoveryMethod =
            piCc->GetIntArray(CarrierConfig::Ims::KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY);
    m_objCarrierConfig.bImsSingleRegistrationRequired =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_IMS_SINGLE_REGISTRATION_REQUIRED_BOOL);
    m_objCarrierConfig.nSipServerPortNumber =
            piCc->GetInt(CarrierConfig::Ims::KEY_SIP_SERVER_PORT_NUMBER_INT);
    m_objCarrierConfig.bKeepPdnUpInNoVops =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_KEEP_PDN_UP_IN_NO_VOPS_BOOL);
    m_objCarrierConfig.nSipPreferredTransport =
            piCc->GetInt(CarrierConfig::Ims::KEY_SIP_PREFERRED_TRANSPORT_INT);
    m_objCarrierConfig.nIpv4SipMtuSizeCellular =
            piCc->GetInt(CarrierConfig::Ims::KEY_IPV4_SIP_MTU_SIZE_CELLULAR_INT);
    m_objCarrierConfig.nIpv6SipMtuSizeCellular =
            piCc->GetInt(CarrierConfig::Ims::KEY_IPV6_SIP_MTU_SIZE_CELLULAR_INT);
    m_objCarrierConfig.objImsPdnEnabledInNoVopsSupport =
            piCc->GetIntArray(CarrierConfig::Ims::KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY);
    m_objCarrierConfig.bSipOverIpsecEnabled =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_SIP_OVER_IPSEC_ENABLED_BOOL);
    InitIpsecAlgorithm(piCc);
    m_objCarrierConfig.nRegistrationExpiryTimerSec =
            piCc->GetInt(CarrierConfig::Ims::KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT);
    m_objCarrierConfig.nSipTimerT1Millis =
            piCc->GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_T1_MILLIS_INT);
    m_objCarrierConfig.nRegistrationRetryBaseTimerMillis =
            piCc->GetInt(CarrierConfig::Ims::KEY_REGISTRATION_RETRY_BASE_TIMER_MILLIS_INT);
    m_objCarrierConfig.nRegistrationRetryMaxTimerMillis =
            piCc->GetInt(CarrierConfig::Ims::KEY_REGISTRATION_RETRY_MAX_TIMER_MILLIS_INT);
    m_objCarrierConfig.bRegistrationEventPackageSupported =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_REGISTRATION_EVENT_PACKAGE_SUPPORTED_BOOL);
    m_objCarrierConfig.nRegistrationSubscribeExpiryTimerSec =
            piCc->GetInt(CarrierConfig::Ims::KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT);

    // temp code
    ImsVector<IMS_SINT32> objTemp =
            piCc->GetIntArray(CarrierConfig::Ims::KEY_SUPPORTED_RATS_INT_ARRAY);
    if (objTemp.GetSize() > 0)
    {
        m_objCarrierConfig.objSupportedRats.Clear();
        m_objCarrierConfig.objSupportedRats =
                piCc->GetIntArray(CarrierConfig::Ims::KEY_SUPPORTED_RATS_INT_ARRAY);
    }

    /// imsvoice.
    m_objCarrierConfig.bCarrierVolteRoamingAvailable =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_CARRIER_VOLTE_ROAMING_AVAILABLE_BOOL);
    /// imssms.
    m_objCarrierConfig.bSmsOverImsSupported =
            piCc->GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL);
    m_objCarrierConfig.objSmsOverImsSupportedRats =
            piCc->GetIntArray(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY);
    /// imsrtt.
    /// imsemergency.
    m_objCarrierConfig.bEmergencyCallbackModeSupported = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_EMERGENCY_CALLBACK_MODE_SUPPORTED_BOOL);
    m_objCarrierConfig.objEmergencyOverImsSupportedRats = piCc->GetIntArray(
            CarrierConfig::ImsEmergency::KEY_EMERGENCY_OVER_IMS_SUPPORTED_RATS_INT_ARRAY);
    m_objCarrierConfig.nEmergencyRegistrationTimerMillis =
            piCc->GetInt(CarrierConfig::ImsEmergency::KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT);
    m_objCarrierConfig.nRefreshGeolocationTimeoutMillis =
            piCc->GetInt(CarrierConfig::ImsEmergency::KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT);
    /// imsss.
    m_objCarrierConfig.bNetworkInitiatedUssdOverImsSupported = piCc->GetBoolean(
            CarrierConfig::ImsSs::KEY_NETWORK_INITIATED_USSD_OVER_IMS_SUPPORTED_BOOL);
    /// imsvt.
    /// imswfc.

    /* carrier_config */
    /// ims.
    m_objCarrierConfig.bTcpRequiredForReg =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_USE_TCP_TRANSPORT_FOR_REGISTER_BOOL);
    m_objCarrierConfig.bUnsubscribeRegistrationEventPackage =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_UNSUBSCRIBE_REGISTRATION_EVENT_PACKAGE_BOOL);
    m_objCarrierConfig.nIsimIndexForImpu =
            piCc->GetInt(CarrierConfig::Ims::KEY_ISIM_INDEX_FOR_IMPU_INT);
    m_objCarrierConfig.nPreferredImsDscp =
            piCc->GetInt(CarrierConfig::Ims::KEY_PREFERRED_IMS_DSCP_INT);
    m_objCarrierConfig.nRegistrationPreferredAccesstypeFeatureTag =
            piCc->GetInt(CarrierConfig::Ims::KEY_REGISTRATION_PREFERRED_ACCESSTYPE_FEATURE_TAG_INT);
    m_objCarrierConfig.objGeolocationPidfInSipRegisterSupport = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_IN_SIP_REGISTER_SUPPORT_INT_ARRAY);
    m_objCarrierConfig.objImsIdentityPriority =
            piCc->GetIntArray(CarrierConfig::Ims::KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY);
    m_objCarrierConfig.objRegistrationPermanentErrorCode =
            piCc->GetIntArray(CarrierConfig::Ims::KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY);
    m_objCarrierConfig.objUpdateRegistrationWithRatChange = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_UPDATE_REGISTRATION_WITH_RAT_CHANGE_INT_ARRAY);

    /// imsemergency.
    m_objCarrierConfig.nIpcanReleaseEmergencyPdnUponEmergencyCallEnd = piCc->GetInt(CarrierConfig::
                    ImsEmergency::KEY_IPCAN_RELEASE_EMERGENCY_PDN_UPON_EMERGENCY_CALL_END_INT);
    m_objCarrierConfig.nPreferredEmergencyRegistration =
            piCc->GetInt(CarrierConfig::ImsEmergency::KEY_PREFERRED_EMERGENCY_REGISTRATION_INT);
    m_objCarrierConfig.nWaitTimeMillisForReleaseEPdnAfterECallEnd = piCc->GetInt(
            CarrierConfig::ImsEmergency::KEY_WAIT_TIME_MILLIS_FOR_RELEASE_EPDN_AFTER_ECALL_END_INT);
    /// imssms.
    m_objCarrierConfig.bSupportLimitedAdminSmsMode =
            piCc->GetBoolean(CarrierConfig::ImsSms::KEY_SUPPORT_LIMITED_ADMIN_SMS_MODE_BOOL);
    /// imswfc.
    m_objCarrierConfig.nRegistrationPrivateHeader =
            piCc->GetInt(CarrierConfig::ImsWfc::KEY_REGISTRATION_PRIVATE_HEADER_INT);
}

PROTECTED
void AosNConfiguration::InitAssetsConfig(IN const ICarrierConfig* piCc)
{
    m_objAsset.bB2cCallComposerFeatureTagInRegContact = piCc->GetBoolean(
            CarrierConfig::Ims::KEY_B2C_CALL_COMPOSER_FEATURE_TAG_IN_REG_CONTACT_BOOL);
    m_objAsset.bBlockPcscfOnRegFailure =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_BLOCK_PCSCF_ON_REG_FAILURE_BOOL);
    m_objAsset.bBlockRegOnCsCall =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_BLOCK_REG_ON_CS_CALL_BOOL);
    m_objAsset.bCallEndAndPdnReactivationByRegTerminated = piCc->GetBoolean(
            CarrierConfig::Ims::KEY_CALL_END_AND_PDN_REACTIVATION_BY_REG_TERMINATED_BOOL);
    m_objAsset.bDestroyUnsecureTcpSocketOnAccomplishingReg = piCc->GetBoolean(
            CarrierConfig::Ims::KEY_DESTROY_UNSECURE_TCP_SOCKET_ON_ACCOMPLISHING_REG_BOOL);
    m_objAsset.bEmcCallBasedOnPAssociatedUriOfNormalReg = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_ECALL_BASED_ON_P_ASSOCIATED_URI_OF_NORMAL_REG_BOOL);
    m_objAsset.bEmcRegOnRandomPcscf =
            piCc->GetBoolean(CarrierConfig::ImsEmergency::KEY_EREG_ON_RANDOM_PCSCF_BOOL);
    m_objAsset.bERegWithOnlyTcpInRoaming =
            piCc->GetBoolean(CarrierConfig::ImsEmergency::KEY_EREG_SET_TCP_ONLY_IN_ROAMING_BOOL);
    m_objAsset.bERegUsingFirstImpuInIsim =
            piCc->GetBoolean(CarrierConfig::ImsEmergency::KEY_EREG_USING_FIRST_IMPU_IN_ISIM_BOOL);
    m_objAsset.bHoldRegWithIpcanChangedDuringImsCall = piCc->GetBoolean(
            CarrierConfig::Ims::KEY_HOLD_REG_WITH_IPCAN_CHANGED_DURING_IMS_CALL_BOOL);
    m_objAsset.bIgnoreVopsForVolteEnable =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_IGNORE_VOPS_FOR_VOLTE_ENABLE_BOOL);
    m_objAsset.bImsDeregOn3gNetwork =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_IMS_DEREG_ON_3G_NETWORK_BOOL);
    m_objAsset.bImsiBasedUriPrioritized =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_IMSI_BASED_URI_PRIORITIZED_BOOL);
    m_objAsset.bInitializeIpsecWithNewPcscf =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_INIT_IPSEC_SETTING_WITH_NEW_PCSCF_BOOL);
    m_objAsset.bInitSubUponSubTerminated =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_INIT_SUB_UPON_SUB_TERMINATED_BOOL);
    m_objAsset.bKeepEPdnUponPcscfUnavailable = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_KEEP_EPDN_UPON_PCSCF_UNAVAILABLE_BOOL);
    m_objAsset.bKeepERegRetryOnWlan =
            piCc->GetBoolean(CarrierConfig::ImsEmergency::KEY_KEEP_EREG_RETRY_ON_WLAN_BOOL);
    m_objAsset.bKeepRegRetryCntUponPdnReconnect =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_KEEP_REG_RETRY_CNT_UPON_PDN_RECONNECT_BOOL);
    m_objAsset.bRegTimerForECallTimeoutAsFailure = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_REG_TIMER_FOR_ECALL_TIMEOUT_AS_FAILURE_BOOL);
    m_objAsset.bRegTimerForECallWithRatCheckEnabled = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_REG_TIMER_FOR_ECALL_WITH_RAT_CHECK_ENABLED_BOOL);
    m_objAsset.bStopERegTimerOnEpdnConnected = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_STOP_EREG_TIMER_ON_EPDN_CONNECTED_BOOL);
    m_objAsset.bNoInitRegOnPcscfChange =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_NO_INIT_REG_ON_PCSCF_CHANGE_BOOL);
    m_objAsset.bPlmnBlockWithTimeoutOnVoiceCallUnavailable = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_PLMN_BLOCK_WITH_TIMEOUT_ON_VOICE_CALL_UNAVAILABLE_BOOL);
    m_objAsset.bRegContactValidation =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_REG_CONTACT_VALIDATION_BOOL);
    m_objAsset.bPlmnBlockWithTimeoutOnFailureWithAllPcscfs = piCc->GetBoolean(
            CarrierConfig::Ims::KEY_REG_PLMN_BLOCK_WITH_TIMEOUT_ON_FAILURE_WITH_ALL_PCSCFS_BOOL);
    m_objAsset.bRegRetryWithIpVerFallback =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_REG_RETRY_WITH_IP_VER_FALLBACK_BOOL);
    m_objAsset.bReleaseEPdnOfUnavailableNetwork = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_RELEASE_EPDN_OF_UNAVAILABLE_NETWORK_BOOL);
    m_objAsset.bReleaseEPdnUponECallEndInFakeMode = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_RELEASE_EPDN_UPON_ECALL_END_IN_FAKE_MODE_BOOL);
    m_objAsset.bRemoveOldSaOnEstablishingSa =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_REMOVE_OLD_SA_ON_ESTABLISHING_SA_BOOL);
    m_objAsset.bRequiredCdmalessFeatureTag =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_REQUIRED_CDMALESS_FEATURE_TAG_BOOL);
    m_objAsset.bRequiredInitRegAfterImsCallEndOnRegHeld = piCc->GetBoolean(
            CarrierConfig::Ims::KEY_REQUIRED_INIT_REG_AFTER_IMS_CALL_END_ON_REG_HELD_BOOL);
    m_objAsset.bRequiredInitRegAfterImsECallEndOnRegHeld = piCc->GetBoolean(
            CarrierConfig::Ims::KEY_REQUIRED_INIT_REG_AFTER_IMS_ECALL_END_ON_REG_HELD_BOOL);
    m_objAsset.bRequiredVolteBlockBySsac =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_REQUIRED_VOLTE_BLOCK_BY_SSAC_BOOL);
    m_objAsset.bRequiredWfcBlockByAirplaneMode =
            piCc->GetBoolean(CarrierConfig::ImsWfc::KEY_REQUIRED_WFC_BLOCK_BY_AIRPLANE_MODE_BOOL);
    m_objAsset.bReregWithChangedCountryOnWifi =
            piCc->GetBoolean(CarrierConfig::ImsWfc::KEY_REREG_WITH_CHANGED_COUNTRY_ON_WIFI_BOOL);
    m_objAsset.bSipOverIpsecEnabledInRoaming =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_SIP_OVER_IPSEC_ENABLED_IN_ROAMING_BOOL);
    m_objAsset.bSmsOverImsAvailableWithoutVoiceCapa = piCc->GetBoolean(
            CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_AVAILABLE_WITHOUT_VOICE_CAPA_BOOL);
    m_objAsset.bSupportAnonymousECallAction =
            piCc->GetBoolean(CarrierConfig::ImsEmergency::KEY_SUPPORT_ANONYMOUS_ECALL_ACTION_BOOL);
    m_objAsset.bSupportEmergencyReregOnIpcanChange =
            piCc->GetBoolean(CarrierConfig::ImsEmergency::KEY_SUPPORT_EREREG_ON_IPCAN_CHANGE_BOOL);
    m_objAsset.bSupportGibaForERegInRoaming = piCc->GetBoolean(
            CarrierConfig::ImsEmergency::KEY_SUPPORT_GIBA_FOR_EREG_IN_ROAMING_BOOL);
    m_objAsset.bSupportRegWithFeatureTagUnavailable =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_SUPPORT_REG_WITH_FEATURE_TAG_UNAVAILABLE_BOOL);
    m_objAsset.bSupportVerstatBasedOnNetworkForReg =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_SUPPORT_VERSTAT_BASED_ON_NETWORK_FOR_REG_BOOL);
    m_objAsset.bSupportVerstatForReg =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_SUPPORT_VERSTAT_FOR_REG_BOOL);
    m_objAsset.bSupportVideoForEmergencyReg =
            piCc->GetBoolean(CarrierConfig::ImsEmergency::KEY_SUPPORT_VIDEO_FOR_EREG_BOOL);
    m_objAsset.bUseRegInfoContactWithoutUriCheck =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_USE_REGINFO_CONTACT_WITHOUT_URI_CHECK_BOOL);
    m_objAsset.bUseRcsTelephonyFeatureTagAsAvailableVoiceCallType = piCc->GetBoolean(CarrierConfig::
                    Ims::KEY_USE_RCS_TELEPHONY_FEATURE_TAG_AS_AVAILABLE_VOICE_CALL_TYPE_BOOL);
    m_objAsset.bUseRetryRuleForEReg =
            piCc->GetBoolean(CarrierConfig::ImsEmergency::KEY_USE_REG_RETRY_RULE_FOR_EREG_BOOL);
    m_objAsset.bUseSecurityServerPortInInitReg =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_USE_SECURITY_SERVER_PORT_IN_INIT_REG_BOOL);
    m_objAsset.bUseSecurityServerPortInRegContactOfInitReg = piCc->GetBoolean(
            CarrierConfig::Ims::KEY_USE_SECURITY_SERVER_PORT_IN_REG_CONTACT_OF_INIT_REG_BOOL);
    m_objAsset.bUseWfcCountryCodeAvailabilityCheck = piCc->GetBoolean(
            CarrierConfig::ImsWfc::KEY_USE_WFC_COUNTRY_CODE_AVAILABILITY_CHECK_BOOL);
    m_objAsset.bVideoOverWifiSupportedWithoutVoice = piCc->GetBoolean(
            CarrierConfig::ImsWfc::KEY_VIDEO_OVER_WIFI_SUPPORTED_WITHOUT_VOICE_BOOL);

    m_objAsset.nContactUserInfoPolicyForNonRegMessage =
            piCc->GetInt(CarrierConfig::Ims::KEY_CONTACT_USER_INFO_POLICY_FOR_NON_REG_MESSAGE_INT);
    m_objAsset.nEmcPreferredIpType =
            piCc->GetInt(CarrierConfig::ImsEmergency::KEY_EPDN_PREFERRED_IPTYPE_INT);
    m_objAsset.nEmcRegRetryMaxCnt =
            piCc->GetInt(CarrierConfig::ImsEmergency::KEY_EREG_RETRY_MAX_CNT_INT);
    m_objAsset.nEmcRegRetryTimerMillis =
            piCc->GetInt(CarrierConfig::ImsEmergency::KEY_EREG_RETRY_TIMER_MILLIS_INT);
    m_objAsset.nGeolocationPidfFormingPolicy =
            piCc->GetInt(CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_FORMING_POLICY_INT);
    m_objAsset.nImsEstablishmentTimeSec =
            piCc->GetInt(CarrierConfig::Ims::KEY_IMS_ESTABLISHMENT_TIME_SEC_INT);
    m_objAsset.nImsPreferredIpType = piCc->GetInt(CarrierConfig::Ims::KEY_IMS_PREFERRED_IPTYPE_INT);
    m_objAsset.nImsSignallingDscp = piCc->GetInt(CarrierConfig::Ims::KEY_IMS_SIGNALLING_DSCP_INT);
    m_objAsset.nRegActualWaitTimePolicy =
            piCc->GetInt(CarrierConfig::Ims::KEY_REG_ACTUAL_WAIT_TIME_POLICY_INT);
    m_objAsset.nRegDefaultWaitTime =
            piCc->GetInt(CarrierConfig::Ims::KEY_REG_DEFAULT_WAIT_TIME_INT);
    m_objAsset.nRegOutOfServicePolicy =
            piCc->GetInt(CarrierConfig::Ims::KEY_REG_OUT_OF_SERVICE_POLICY_INT);
    m_objAsset.nRegPcscfUpdatePolicy =
            piCc->GetInt(CarrierConfig::Ims::KEY_REG_PCSCF_UPDATE_POLICY_INT);
    m_objAsset.nRegRetry305Policy = piCc->GetInt(CarrierConfig::Ims::KEY_REG_RETRY_305_POLICY_INT);
    m_objAsset.nRegRetry503Policy = piCc->GetInt(CarrierConfig::Ims::KEY_REG_RETRY_503_POLICY_INT);
    m_objAsset.nRegRetryCntOnSinglePcscf =
            piCc->GetInt(CarrierConfig::Ims::KEY_REG_RETRY_CNT_ON_SINGLE_PCSCF_INT);
    m_objAsset.nRegRetryCntPerPcscf =
            piCc->GetInt(CarrierConfig::Ims::KEY_REG_RETRY_CNT_PER_PCSCF_INT);
    m_objAsset.nRegRetryCntResetPolicy =
            piCc->GetInt(CarrierConfig::Ims::KEY_REG_RETRY_CNT_RESET_POLICY_INT);
    m_objAsset.nRegRetryCntWithIpsecOnAuthFailure =
            piCc->GetInt(CarrierConfig::Ims::KEY_REG_RETRY_CNT_WITH_IPSEC_ON_AUTH_FAILURE_INT);
    m_objAsset.nRegRetryDefaultPolicy =
            piCc->GetInt(CarrierConfig::Ims::KEY_REG_RETRY_DEFAULT_POLICY_INT);
    m_objAsset.nRegRetryTimerFPolicy =
            piCc->GetInt(CarrierConfig::Ims::KEY_REG_RETRY_TIMER_F_POLICY_INT);
    m_objAsset.nRegTimerForEmcCallMillis =
            piCc->GetInt(CarrierConfig::ImsEmergency::KEY_REG_TIMER_FOR_ECALL_MILLIS_INT);
    m_objAsset.nReregRetry305Policy =
            piCc->GetInt(CarrierConfig::Ims::KEY_REREG_RETRY_305_POLICY_INT);
    m_objAsset.nRoamingPreferredEmcReg =
            piCc->GetInt(CarrierConfig::ImsEmergency::KEY_ROAMING_PREFERRED_EREG_INT);
    m_objAsset.nSipMessageThresholdForTransportChange =
            piCc->GetInt(CarrierConfig::Ims::KEY_SIP_MESSAGE_THRESHOLD_FOR_TRANSPORT_CHANGE_INT);
    m_objAsset.nSubRetry503Policy = piCc->GetInt(CarrierConfig::Ims::KEY_SUB_RETRY_503_POLICY_INT);
    m_objAsset.nUsatRegEventDownloadPolicy =
            piCc->GetInt(CarrierConfig::Ims::KEY_USAT_REG_EVENT_DOWNLOAD_POLICY_INT);
    m_objAsset.nVolteHysTimeSec = piCc->GetInt(CarrierConfig::ImsVoice::KEY_VOLTE_HYS_TIME_SEC_INT);

    m_objAsset.objRegErrCodeForPcscfDiscovery =
            piCc->GetIntArray(CarrierConfig::Ims::KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY);
    m_objAsset.objRegPermanentErrMaxCnt =
            piCc->GetIntArray(CarrierConfig::Ims::KEY_REG_PERMANENT_ERR_MAX_CNT_INT_ARRAY);
    m_objAsset.objRegRetryErrCodeWithoutIpsec =
            piCc->GetIntArray(CarrierConfig::Ims::KEY_REG_RETRY_ERR_CODE_WITHOUT_IPSEC_INT_ARRAY);
    m_objAsset.objReregErrCodeForCallEnd =
            piCc->GetIntArray(CarrierConfig::Ims::KEY_REREG_ERR_CODE_FOR_CALL_END_INT_ARRAY);
    m_objAsset.objReregErrCodeForImsPdnReactivation = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_REREG_ERR_CODE_FOR_IMS_PDN_REACTIVATION_INT_ARRAY);
    m_objAsset.objReregErrCodeForInitRegWithAvailablePcscf = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_REREG_ERR_CODE_FOR_INIT_REG_WITH_AVAILABLE_PCSCF_INT_ARRAY);
    m_objAsset.objReregRetryErrCodeForInitRegWithSamePcscf = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_REREG_RETRY_ERR_CODE_FOR_INIT_REG_WITH_SAME_PCSCF_INT_ARRAY);
    m_objAsset.objSubErrorCodeForInitRegWithNextPcscf = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_NEXT_PCSCF_INT_ARRAY);
    m_objAsset.objSubErrorCodeForStoppingByExpirationTime = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_STOPPING_BY_EXPIRATION_TIME_INT_ARRAY);
    m_objAsset.objSupportedRoamingRats =
            piCc->GetIntArray(CarrierConfig::Ims::KEY_SUPPORTED_ROAMING_RATS_INT_ARRAY);
    m_objAsset.objTestMode = piCc->GetIntArray(CarrierConfig::Ims::KEY_TEST_MODE_INT_ARRAY);
    m_objAsset.objUnavailableFeaturesInLimitedReg = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_UNAVAILABLE_FEATURES_IN_LIMITED_REG_INT_ARRAY);
    m_objAsset.objERegErrCodeNotSupportedCommonPolicy = piCc->GetIntArray(
            CarrierConfig::ImsEmergency::KEY_EREG_ERR_CODE_NOT_SUPPORTED_COMMON_POLICY_INT_ARRAY);
    m_objAsset.objVowifiSubErrorCodeForInitReg = piCc->GetIntArray(
            CarrierConfig::ImsWfc::KEY_VOWIFI_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY);
}

PROTECTED
void AosNConfiguration::InitIpsecAlgorithm(IN const ICarrierConfig* piCc)
{
    ImsVector<IMS_SINT32> objAuthAlgo =
            piCc->GetIntArray(CarrierConfig::Ims::KEY_IPSEC_AUTHENTICATION_ALGORITHMS_INT_ARRAY);

    m_objCarrierConfig.objIpsecAuthenticationAlgorithms.Clear();

    for (int i = 0; i < objAuthAlgo.GetSize(); i++)
    {
        m_objCarrierConfig.objIpsecAuthenticationAlgorithms.Push(objAuthAlgo.GetAt(i));
    }

    ImsVector<IMS_SINT32> objEncryAlgo =
            piCc->GetIntArray(CarrierConfig::Ims::KEY_IPSEC_ENCRYPTION_ALGORITHMS_INT_ARRAY);

    m_objCarrierConfig.objIpsecEncryptionAlgorithms.Clear();

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
        else  // IPSEC_ENCRYPTION_ALGORITHM_NULL
        {
            m_objCarrierConfig.objIpsecEncryptionAlgorithms.Push(
                    IpSecType::ENCRYPTION_ALGORITHM_NO);
        }
    }
}
