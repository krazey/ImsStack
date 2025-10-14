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
#ifndef AOS_ASSET_H_
#define AOS_ASSET_H_

#include "ImsTypeDef.h"
#include "AString.h"
#include "ImsVector.h"
#include "CarrierConfig.h"

struct AosAsset
{
public:
    AosAsset() :
            bB2cCallComposerFeatureTagInRegContact(IMS_TRUE),
            bBlockPcscfOnRegFailure(IMS_TRUE),
            bBlockRegOnCsCall(IMS_TRUE),
            bCallEndAndPdnReactivationByRegTerminated(IMS_FALSE),
            bDestroyUnsecureTcpSocketOnAccomplishingReg(IMS_FALSE),
            bEmcCallBasedOnPAssociatedUriOfNormalReg(IMS_FALSE),
            bEmcRegOnRandomPcscf(IMS_FALSE),
            bERegWithOnlyTcpInRoaming(IMS_FALSE),
            bERegUsingFirstImpuInIsim(IMS_FALSE),
            bHoldRegWithIpcanChangedDuringImsCall(IMS_FALSE),
            bIgnoreVopsForVolteEnable(IMS_FALSE),
            bImsDeregOn3gNetwork(IMS_FALSE),
            bImsiBasedUriPrioritized(IMS_FALSE),
            bInitializeIpsecWithNewPcscf(IMS_FALSE),
            bInitSubUponSubTerminated(IMS_FALSE),
            bKeepEPdnUponPcscfUnavailable(IMS_FALSE),
            bKeepERegRetryOnWlan(IMS_FALSE),
            bKeepExistingPcscfOnPcscfChangeDuringTheCall(IMS_FALSE),
            bKeepRegRetryCntUponPdnReconnect(IMS_FALSE),
            bKeepRegRetryTimerOnAllEnablersDetached(IMS_FALSE),
            bRegTimerForECallTimeoutAsFailure(IMS_TRUE),
            bRegTimerForECallWithRatCheckEnabled(IMS_FALSE),
            bStopERegTimerOnEpdnConnected(IMS_FALSE),
            bPdnReconnectOnAllPcscfsUnavailable(IMS_FALSE),
            bPlmnBlockWithTimeoutOnVoiceCallUnavailable(IMS_FALSE),
            bRegContactValidation(IMS_FALSE),
            bRegRetryWithIpVerFallback(IMS_FALSE),
            bReleaseEPdnOfUnavailableNetwork(IMS_TRUE),
            bReleaseEPdnUponECallEndInFakeMode(IMS_TRUE),
            bRemoveOldSaOnEstablishingSa(IMS_FALSE),
            bRequiredCdmalessFeatureTag(IMS_FALSE),
            bRequiredInitRegAfterImsCallEndOnRegHeld(IMS_FALSE),
            bRequiredInitRegAfterImsECallEndOnRegHeld(IMS_FALSE),
            bRequiredVolteBlockBySsac(IMS_FALSE),
            bRequiredWfcBlockByAirplaneMode(IMS_FALSE),
            bGeolocationPidfInWfcInitReg(IMS_FALSE),
            bPaniHeaderInWfcInitReg(IMS_FALSE),
            bReregWithChangedCountryOnWifi(IMS_FALSE),
            bSipOverIpsecEnabledInRoaming(IMS_TRUE),
            bSmsOverImsAvailableWithoutVoiceCapa(IMS_FALSE),
            bSupportAnonymousECallAction(IMS_FALSE),
            bSupportERegWhenEAttachWithValidSim(IMS_FALSE),
            bSupportEmergencyReregOnIpcanChange(IMS_FALSE),
            bSupportGibaForERegInRoaming(IMS_FALSE),
            bSupportVerstatBasedOnNetworkForReg(IMS_FALSE),
            bSupportVerstatForReg(IMS_FALSE),
            bSupportVideoForEmergencyReg(IMS_FALSE),
            bUpdateOngoingRegRetryTimerOnImsEstTimerExpiry(IMS_TRUE),
            bUseRcsTelephonyFeatureTagAsAvailableVoiceCallType(IMS_FALSE),
            bUseRegInfoContactWithoutUriCheck(IMS_FALSE),
            bUseRetryRuleForEReg(IMS_FALSE),
            bUseSecurityServerPortInInitReg(IMS_FALSE),
            bUseSecurityServerPortInRegContactOfInitReg(IMS_FALSE),
            bUseWfcCountryCodeAvailabilityCheck(IMS_FALSE),
            bVideoOverWifiSupportedWithoutVoice(IMS_FALSE),
            nAuthFailureRetryMaxCnt(0),
            nContactUserInfoPolicyForNonRegMessage(
                    CarrierConfig::Ims::CONTACT_USER_INFO_POLICY_DEFAULT),
            nEmcPreferredIpType(CarrierConfig::Ims::IP_VERSION_6),
            nEmcRegRetryMaxCnt(0),
            nEmcRegRetryTimerMillis(0),
            nGeolocationPidfFormingPolicy(CarrierConfig::Ims::GEOLOCATION_POLICY_WITHOUT_POSITION),
            nImsEstablishmentTimeForLteSec(0),
            nImsEstablishmentTimeForNrSec(180),
            nImsPreferredIpType(CarrierConfig::Ims::IP_VERSION_6),
            nImsSignallingDscp(46),
            nPdnReconnectDelayOnWfcSetupFailAllPcscfsWithCsRoam(0),
            nRegActualWaitTimePolicy(CarrierConfig::Ims::AWT_POLICY_RFC_RULE),
            nRegDefaultWaitTime(0),
            nRegOutOfServicePolicy(CarrierConfig::Ims::REG_OOS_POLICY_DEFAULT),
            nRegPcscfUpdatePolicy(CarrierConfig::Ims::REG_PCSCF_UPDATE_POLICY_DEFAULT),
            nRegRetry305Policy(CarrierConfig::Ims::SIP_305_CODE_POLICY_DEFAULT),
            nRegRetry503Policy(CarrierConfig::Ims::SIP_503_CODE_POLICY_DEFAULT),
            nRegRetryCntOnSinglePcscf(0),
            nRegRetryCntPerPcscf(0),
            nRegRetryCntPerPcscfWithRaTime(0),
            nRegRetryCntResetPolicy(CarrierConfig::Ims::REG_RETRY_CNT_RESET_POLICY_REGISTRATION),
            nRegRetryCntWithIpsecOnAuthFailure(3),
            nRegRetryDefaultPolicy(CarrierConfig::Ims::DEFAULT_RETRY_POLICY_SPEC),
            nRegRetryTimerFPolicy(CarrierConfig::Ims::TIMER_F_POLICY_NONE),
            nRegTimerForEmcCallMillis(0),
            nReleasePdnDelaySecAfterTempPlmnBlock(5),
            nReregRetry305Policy(CarrierConfig::Ims::SIP_305_CODE_POLICY_DEFAULT),
            nRoamingPreferredEmcReg(
                    CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NOT_DEFINED),
            nSipMessageThresholdForTransportChange(200),
            nSubRetry503Policy(CarrierConfig::Ims::SIP_503_CODE_POLICY_3GPP),
            nUsatRegEventDownloadPolicy(CarrierConfig::Ims::USAT_REG_EVENT_NOT_DOWNLOAD),
            nVolteHysTimeSec(0),
            nSubConsecutiveRetryCntForRegForbiddenInWifi(0),
            objKeepRegWithMmtelFeatureTagPolicy(ImsVector<IMS_SINT32>()),
            objRegErrCodeForPcscfDiscovery(ImsVector<IMS_SINT32>()),
            objRegPermanentErrMaxCnt(ImsVector<IMS_SINT32>()),
            objRegRetryErrCodeWithoutIpsec(ImsVector<IMS_SINT32>()),
            objTempPlmnBlockRats(ImsVector<IMS_SINT32>()),
            objReregErrCodeForCallEnd(ImsVector<IMS_SINT32>()),
            objReregErrCodeForImsPdnReactivation(ImsVector<IMS_SINT32>()),
            objReregErrCodeForInitRegWithAvailablePcscf(ImsVector<IMS_SINT32>()),
            objReregRetryErrCodeForInitRegWithSamePcscf(ImsVector<IMS_SINT32>()),
            objSubErrorCodeForInitRegWithNextPcscf(ImsVector<IMS_SINT32>()),
            objSubErrorCodeForStoppingByExpirationTime(ImsVector<IMS_SINT32>()),
            objSupportedRoamingRats(ImsVector<IMS_SINT32>()),
            objTestMode(ImsVector<IMS_SINT32>()),
            objUnavailableFeaturesInLimitedReg(ImsVector<IMS_SINT32>()),
            objERegErrCodeNotSupportedCommonPolicy(ImsVector<IMS_SINT32>()),
            objNetworkAttachRejectCausesForCrossStackRedial(ImsVector<IMS_SINT32>()),
            objVowifiSubErrorCodeForInitReg(ImsVector<IMS_SINT32>())
    {
    }

    AosAsset(IN const AosAsset&) = delete;
    AosAsset& operator=(IN const AosAsset&) = delete;

public:
    IMS_BOOL bB2cCallComposerFeatureTagInRegContact;
    IMS_BOOL bBlockPcscfOnRegFailure;
    IMS_BOOL bBlockRegOnCsCall;
    IMS_BOOL bCallEndAndPdnReactivationByRegTerminated;
    IMS_BOOL bDestroyUnsecureTcpSocketOnAccomplishingReg;
    IMS_BOOL bEmcCallBasedOnPAssociatedUriOfNormalReg;
    IMS_BOOL bEmcRegOnRandomPcscf;
    IMS_BOOL bERegWithOnlyTcpInRoaming;
    IMS_BOOL bERegUsingFirstImpuInIsim;
    IMS_BOOL bHoldRegWithIpcanChangedDuringImsCall;
    IMS_BOOL bIgnoreVopsForVolteEnable;
    IMS_BOOL bImsDeregOn3gNetwork;
    IMS_BOOL bImsiBasedUriPrioritized;
    IMS_BOOL bInitializeIpsecWithNewPcscf;
    IMS_BOOL bInitSubUponSubTerminated;
    IMS_BOOL bKeepEPdnUponPcscfUnavailable;
    IMS_BOOL bKeepERegRetryOnWlan;
    IMS_BOOL bKeepExistingPcscfOnPcscfChangeDuringTheCall;
    IMS_BOOL bKeepRegRetryCntUponPdnReconnect;
    IMS_BOOL bKeepRegRetryTimerOnAllEnablersDetached;
    IMS_BOOL bRegTimerForECallTimeoutAsFailure;
    IMS_BOOL bRegTimerForECallWithRatCheckEnabled;
    IMS_BOOL bStopERegTimerOnEpdnConnected;
    IMS_BOOL bPdnReconnectOnAllPcscfsUnavailable;
    IMS_BOOL bPlmnBlockWithTimeoutOnVoiceCallUnavailable;
    IMS_BOOL bRegContactValidation;
    IMS_BOOL bRegRetryWithIpVerFallback;
    IMS_BOOL bReleaseEPdnOfUnavailableNetwork;
    IMS_BOOL bReleaseEPdnUponECallEndInFakeMode;
    IMS_BOOL bRemoveOldSaOnEstablishingSa;
    IMS_BOOL bRequiredCdmalessFeatureTag;
    IMS_BOOL bRequiredInitRegAfterImsCallEndOnRegHeld;
    IMS_BOOL bRequiredInitRegAfterImsECallEndOnRegHeld;
    IMS_BOOL bRequiredVolteBlockBySsac;
    IMS_BOOL bRequiredWfcBlockByAirplaneMode;
    IMS_BOOL bGeolocationPidfInWfcInitReg;
    IMS_BOOL bPaniHeaderInWfcInitReg;
    IMS_BOOL bReregWithChangedCountryOnWifi;
    IMS_BOOL bSipOverIpsecEnabledInRoaming;
    IMS_BOOL bSmsOverImsAvailableWithoutVoiceCapa;
    IMS_BOOL bSupportAnonymousECallAction;
    IMS_BOOL bSupportERegWhenEAttachWithValidSim;
    IMS_BOOL bSupportEmergencyReregOnIpcanChange;
    IMS_BOOL bSupportGibaForERegInRoaming;
    IMS_BOOL bSupportVerstatBasedOnNetworkForReg;
    IMS_BOOL bSupportVerstatForReg;
    IMS_BOOL bSupportVideoForEmergencyReg;
    IMS_BOOL bUpdateOngoingRegRetryTimerOnImsEstTimerExpiry;
    IMS_BOOL bUseRcsTelephonyFeatureTagAsAvailableVoiceCallType;
    IMS_BOOL bUseRegInfoContactWithoutUriCheck;
    IMS_BOOL bUseRetryRuleForEReg;
    IMS_BOOL bUseSecurityServerPortInInitReg;
    IMS_BOOL bUseSecurityServerPortInRegContactOfInitReg;
    IMS_BOOL bUseWfcCountryCodeAvailabilityCheck;
    IMS_BOOL bVideoOverWifiSupportedWithoutVoice;
    IMS_SINT32 nAuthFailureRetryMaxCnt;
    IMS_SINT32 nContactUserInfoPolicyForNonRegMessage;
    IMS_SINT32 nEmcPreferredIpType;
    IMS_SINT32 nEmcRegRetryMaxCnt;
    IMS_SINT32 nEmcRegRetryTimerMillis;
    IMS_SINT32 nGeolocationPidfFormingPolicy;
    IMS_SINT32 nImsEstablishmentTimeForLteSec;
    IMS_SINT32 nImsEstablishmentTimeForNrSec;
    IMS_SINT32 nImsPreferredIpType;
    IMS_SINT32 nImsSignallingDscp;
    IMS_SINT32 nPdnReconnectDelayOnWfcSetupFailAllPcscfsWithCsRoam;
    IMS_SINT32 nRegActualWaitTimePolicy;
    IMS_SINT32 nRegDefaultWaitTime;
    IMS_SINT32 nRegOutOfServicePolicy;
    IMS_SINT32 nRegPcscfUpdatePolicy;
    IMS_SINT32 nRegRetry305Policy;
    IMS_SINT32 nRegRetry503Policy;
    IMS_SINT32 nRegRetryCntOnSinglePcscf;
    IMS_SINT32 nRegRetryCntPerPcscf;
    IMS_SINT32 nRegRetryCntPerPcscfWithRaTime;
    IMS_SINT32 nRegRetryCntResetPolicy;
    IMS_SINT32 nRegRetryCntWithIpsecOnAuthFailure;
    IMS_SINT32 nRegRetryDefaultPolicy;
    IMS_SINT32 nRegRetryTimerFPolicy;
    IMS_SINT32 nRegTimerForEmcCallMillis;
    IMS_SINT32 nReleasePdnDelaySecAfterTempPlmnBlock;
    IMS_SINT32 nReregRetry305Policy;
    IMS_SINT32 nRoamingPreferredEmcReg;
    IMS_SINT32 nSipMessageThresholdForTransportChange;
    IMS_SINT32 nSubRetry503Policy;
    IMS_SINT32 nUsatRegEventDownloadPolicy;
    IMS_SINT32 nVolteHysTimeSec;
    IMS_SINT32 nSubConsecutiveRetryCntForRegForbiddenInWifi;
    ImsVector<IMS_SINT32> objKeepRegWithMmtelFeatureTagPolicy;
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    ImsVector<IMS_SINT32> objRegPermanentErrMaxCnt;
    ImsVector<IMS_SINT32> objRegRetryErrCodeWithoutIpsec;
    ImsVector<IMS_SINT32> objTempPlmnBlockRats;
    ImsVector<IMS_SINT32> objReregErrCodeForCallEnd;
    ImsVector<IMS_SINT32> objReregErrCodeForImsPdnReactivation;
    ImsVector<IMS_SINT32> objReregErrCodeForInitRegWithAvailablePcscf;
    ImsVector<IMS_SINT32> objReregRetryErrCodeForInitRegWithSamePcscf;
    ImsVector<IMS_SINT32> objSubErrorCodeForInitRegWithNextPcscf;
    ImsVector<IMS_SINT32> objSubErrorCodeForStoppingByExpirationTime;
    ImsVector<IMS_SINT32> objSupportedRoamingRats;
    ImsVector<IMS_SINT32> objTestMode;
    ImsVector<IMS_SINT32> objUnavailableFeaturesInLimitedReg;
    ImsVector<IMS_SINT32> objERegErrCodeNotSupportedCommonPolicy;
    ImsVector<IMS_SINT32> objNetworkAttachRejectCausesForCrossStackRedial;
    ImsVector<IMS_SINT32> objVowifiSubErrorCodeForInitReg;
};
#endif  // AOS_ASSET_H_
