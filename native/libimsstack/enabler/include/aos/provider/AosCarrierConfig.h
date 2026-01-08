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
#ifndef AOS_CARRIER_CONFIG_H_
#define AOS_CARRIER_CONFIG_H_

#include "ImsTypeDef.h"
#include "AString.h"
#include "ImsVector.h"
#include "CarrierConfig.h"

struct AosCarrierConfig
{
public:
    AosCarrierConfig() :
            bSupportEmergencySmsOverIms(IMS_FALSE),
            bCarrierVolteAvailable(IMS_FALSE),
            bCarrierVtAvailable(IMS_FALSE),
            bCarrierWfcImsAvailable(IMS_FALSE),
            bRttSupported(IMS_FALSE),
            bRttSupportedWhileRoaming(IMS_FALSE),
            bCarrierCrossSimImsAvailable(IMS_FALSE),
            bCarrierVolteTtySupported(IMS_FALSE),
            bImsSingleRegistrationRequired(IMS_FALSE),
            bKeepPdnUpInNoVops(IMS_FALSE),
            bSipOverIpsecEnabled(IMS_FALSE),  // default - IMS_TRUE
            bRegistrationEventPackageSupported(IMS_TRUE),
            bCarrierVolteRoamingAvailable(IMS_TRUE),
            bSmsOverImsSupported(IMS_TRUE),
            bEmergencyCallbackModeSupported(IMS_FALSE),
            bNetworkInitiatedUssdOverImsSupported(IMS_TRUE),
            bTcpRequiredForReg(IMS_FALSE),
            bUnsubscribeRegistrationEventPackage(IMS_FALSE),
            bSupportLimitedAdminSmsMode(IMS_FALSE),
            nCarrierUssdMethod(CarrierConfig::USSD_OVER_CS_PREFERRED),
            nSipTimerT1Millis(2000),
            nSipServerPortNumber(5060),
            nSipPreferredTransport(CarrierConfig::Ims::PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP),
            nIpv4SipMtuSizeCellular(1500),
            nIpv6SipMtuSizeCellular(1500),
            nRegistrationExpiryTimerSec(600000),
            nRegistrationRetryBaseTimerMillis(30000),
            nRegistrationRetryMaxTimerMillis(1800000),
            nRegistrationSubscribeExpiryTimerSec(600000),
            nEmergencyRegistrationTimerMillis(10000),
            nRefreshGeolocationTimeoutMillis(5000),
            nIsimIndexForImpu(1),
            nPreferredImsDscp(CarrierConfig::Ims::PREFERRED_DSCP_NONE),
            nRegistrationPreferredAccesstypeFeatureTag(
                    CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED),
            nIpcanReleaseEmergencyPdnUponEmergencyCallEnd(CarrierConfig::ImsEmergency::IPCAN_NONE),
            nPreferredEmergencyRegistration(
                    CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK),
            nWaitTimeMillisForReleaseEPdnAfterECallEnd(0),
            nRegistrationPrivateHeader(0),
            objCarrierNrAvailabilities(ImsVector<IMS_SINT32>()),
            objPcscfDiscoveryMethod(ImsVector<IMS_SINT32>()),
            objImsPdnEnabledInNoVopsSupport(ImsVector<IMS_SINT32>()),
            objIpsecAuthenticationAlgorithms(ImsVector<IMS_SINT32>()),
            objIpsecEncryptionAlgorithms(ImsVector<IMS_SINT32>()),
            objSupportedRats(ImsVector<IMS_SINT32>()),
            objSmsOverImsSupportedRats(ImsVector<IMS_SINT32>()),
            objEmergencyOverImsSupportedRats(ImsVector<IMS_SINT32>()),
            objGeolocationPidfInSipRegisterSupport(ImsVector<IMS_SINT32>()),
            objImsIdentityPriority(ImsVector<IMS_SINT32>()),
            objRegistrationPermanentErrorCode(ImsVector<IMS_SINT32>()),
            objUpdateRegistrationWithRatChange(ImsVector<IMS_SINT32>())
    {
        // temp setting
        objSupportedRats.Push(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN);
        objSupportedRats.Push(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
    }

    AosCarrierConfig(IN const AosCarrierConfig&) = delete;
    AosCarrierConfig& operator=(IN const AosCarrierConfig&) = delete;

public:
    /// aosp_carrier_config - no prefix
    IMS_BOOL bSupportEmergencySmsOverIms;
    IMS_BOOL bCarrierVolteAvailable;
    IMS_BOOL bCarrierVtAvailable;
    IMS_BOOL bCarrierWfcImsAvailable;
    IMS_BOOL bRttSupported;
    IMS_BOOL bRttSupportedWhileRoaming;
    IMS_BOOL bCarrierCrossSimImsAvailable;
    IMS_BOOL bCarrierVolteTtySupported;
    /// aosp_carrier_config - ims.
    IMS_BOOL bImsSingleRegistrationRequired;
    IMS_BOOL bKeepPdnUpInNoVops;
    IMS_BOOL bSipOverIpsecEnabled;
    IMS_BOOL bRegistrationEventPackageSupported;
    /// aosp_carrier_config - imsvoice.
    IMS_BOOL bCarrierVolteRoamingAvailable;
    /// aosp_carrier_config - imssms.
    IMS_BOOL bSmsOverImsSupported;
    /// aosp_carrier_config - imsemergency.
    IMS_BOOL bEmergencyCallbackModeSupported;
    /// aosp_carrier_config - imsss.
    IMS_BOOL bNetworkInitiatedUssdOverImsSupported;

    /// carrier_config - ims
    IMS_BOOL bTcpRequiredForReg;
    IMS_BOOL bUnsubscribeRegistrationEventPackage;
    /// carrier_config - imssms.
    IMS_BOOL bSupportLimitedAdminSmsMode;

    /// aosp_carrier_config - no prefix
    IMS_SINT32 nCarrierUssdMethod;
    /// aosp_carrier_config - ims.
    IMS_SINT32 nSipTimerT1Millis;
    IMS_SINT32 nSipServerPortNumber;
    IMS_SINT32 nSipPreferredTransport;
    IMS_SINT32 nIpv4SipMtuSizeCellular;
    IMS_SINT32 nIpv6SipMtuSizeCellular;
    IMS_SINT32 nRegistrationExpiryTimerSec;
    IMS_SINT32 nRegistrationRetryBaseTimerMillis;
    IMS_SINT32 nRegistrationRetryMaxTimerMillis;
    IMS_SINT32 nRegistrationSubscribeExpiryTimerSec;
    /// aosp_carrier_config - imsemergency.
    IMS_SINT32 nEmergencyRegistrationTimerMillis;
    IMS_SINT32 nRefreshGeolocationTimeoutMillis;

    /// carrier_config - ims
    IMS_SINT32 nIsimIndexForImpu;
    IMS_SINT32 nPreferredImsDscp;
    IMS_SINT32 nRegistrationPreferredAccesstypeFeatureTag;
    /// carrier_config - imsemergency.
    IMS_SINT32 nIpcanReleaseEmergencyPdnUponEmergencyCallEnd;
    IMS_SINT32 nPreferredEmergencyRegistration;
    IMS_SINT32 nWaitTimeMillisForReleaseEPdnAfterECallEnd;
    /// carrier_config - imswfc.
    IMS_SINT32 nRegistrationPrivateHeader;

    /// aosp_carrier_config - no prefix
    ImsVector<IMS_SINT32> objCarrierNrAvailabilities;
    /// aosp_carrier_config - ims.
    ImsVector<IMS_SINT32> objPcscfDiscoveryMethod;
    ImsVector<IMS_SINT32> objImsPdnEnabledInNoVopsSupport;
    ImsVector<IMS_SINT32> objIpsecAuthenticationAlgorithms;
    ImsVector<IMS_SINT32> objIpsecEncryptionAlgorithms;
    ImsVector<IMS_SINT32> objSupportedRats;
    /// aosp_carrier_config - imssms.
    ImsVector<IMS_SINT32> objSmsOverImsSupportedRats;
    /// aosp_carrier_config - imsemergency.
    ImsVector<IMS_SINT32> objEmergencyOverImsSupportedRats;

    /// carrier_config - ims
    ImsVector<IMS_SINT32> objGeolocationPidfInSipRegisterSupport;
    ImsVector<IMS_SINT32> objImsIdentityPriority;
    ImsVector<IMS_SINT32> objRegistrationPermanentErrorCode;
    ImsVector<IMS_SINT32> objUpdateRegistrationWithRatChange;
};
#endif  // AOS_CARRIER_CONFIG_H_
