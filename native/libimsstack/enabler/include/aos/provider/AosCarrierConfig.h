#ifndef _AOS_CARRIER_CONFIG_H_
#define _AOS_CARRIER_CONFIG_H_

#include "IMSTypeDef.h"
#include "AString.h"
#include "IMSVector.h"
#include "CarrierConfig.h"

struct AosCarrierConfig
{
public:
    AosCarrierConfig()
        : bSupportEmergencySmsOverIms(IMS_FALSE)
        , bCarrierVolteAvailable(IMS_FALSE)
        , bCarrierVtAvailable(IMS_FALSE)
        , bCarrierWfcImsAvailable(IMS_FALSE)
        , bRttSupported(IMS_FALSE)
        , bCarrierCrossSimImsAvailable(IMS_FALSE)
        , bCarrierVolteTtySupported(IMS_FALSE)
        , objCarrierNrAvailabilities(IMSVector<IMS_SINT32>())
        , nCarrierUssdMethod(CarrierConfig::USSD_OVER_CS_PREFERRED)
        , objPcscfDiscoveryMethod(IMSVector<IMS_SINT32>())
        , bImsSingleRegistrationRequired(IMS_FALSE)
        , nSipServerPortNumber(5060)
        , bKeepPdnUpInNoVops(IMS_FALSE)
        , nSipPreferredTransport(CarrierConfig::Ims::PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP)
        , nIpv4SipMtuSizeCellular(1500)
        , nIpv6SipMtuSizeCellular(1500)
        , objImsPdnEnabledInNoVopsSupport(IMSVector<IMS_SINT32>())
        , bSipOverIpsecEnabled(IMS_FALSE) // default - IMS_TRUE
        , objIpsecAuthenticationAlgorithms(IMSVector<IMS_SINT32>())
        , objIpsecEncryptionAlgorithms(IMSVector<IMS_SINT32>())
        , nRegistrationExpiryTimerSec(600000)
        , nRegistrationRetryBaseTimerMillis(30000)
        , nRegistrationRetryMaxTimerMillis(1800000)
        , bRegistrationEventPackageSupported(IMS_TRUE)
        , nRegistrationSubscribeExpiryTimerSec(600000)
        , objGeolocationPidfInSipRegisterSupport(IMSVector<IMS_SINT32>())
        , objSupportedRats(IMSVector<IMS_SINT32>())
        , bCarrierVolteRoamingAvailable(IMS_TRUE)
        , bSmsOverImsSupported(IMS_TRUE)
        , objSmsOverImsSupportedRats(IMSVector<IMS_SINT32>())
        , bEmergencyCallbackModeSupported(IMS_FALSE)
        , objEmergencyOverImsSupportedRats(IMSVector<IMS_SINT32>())
        , nEmergencyRegistrationTimerMillis(10000)
        , nRefreshGeolocationTimeoutMillis(5000)
        , nImsPreferredIpType(CarrierConfig::Ims::IP_VERSION_6)
        , objImsIdentityPriority(IMSVector<IMS_SINT32>())
        , nIsimIndexForImpu(1)
        , objUpdateRegistrationWithRatChange(IMSVector<IMS_SINT32>())
        , bUnsubscribeRegistrationEventPackage(IMS_FALSE)
        , bRegistrationEventForCatRequired(IMS_FALSE)
        , nPreferredImsDscp(CarrierConfig::Ims::PREFERRED_DSCP_NONE)
        , nImsSignallingDscp(46)
        , nRegistrationPreferredAccesstypeFeatureTag(
                CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED)
        , nRegistrationActualWaitTimePolicy(CarrierConfig::Ims::AWT_POLICY_RFC_RULE)
        , bSupportLimitedAdminSmsMode(IMS_FALSE)
        , bReleaseEmergencyPdnWithEmergencyCallEnd(IMS_FALSE)
        , bDisableT3482ForEmergency(IMS_FALSE)
        , nRegistrationTimerForEmergencyCallMillis(0)
        , nPreferredEmergencyRegistration(
                CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK)
        , bUpdateRegistrationWithCountryChange(IMS_FALSE)
        , nRegistrationPrivateHeader(0)
    {
        // temp setting
        objSupportedRats.Push(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN);
        objSupportedRats.Push(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
    }

    AosCarrierConfig(IN const AosCarrierConfig&) = delete;
    AosCarrierConfig& operator=(IN const AosCarrierConfig&) = delete;

public:
    /* aosp_carrier_config */
    /// no prefix
    IMS_BOOL bSupportEmergencySmsOverIms;
    IMS_BOOL bCarrierVolteAvailable;
    IMS_BOOL bCarrierVtAvailable;
    IMS_BOOL bCarrierWfcImsAvailable;
    IMS_BOOL bRttSupported;
    IMS_BOOL bCarrierCrossSimImsAvailable;
    IMS_BOOL bCarrierVolteTtySupported;
    IMSVector<IMS_SINT32> objCarrierNrAvailabilities;
    IMS_SINT32 nCarrierUssdMethod;

    /// ims.
    IMSVector<IMS_SINT32> objPcscfDiscoveryMethod;
    IMS_BOOL bImsSingleRegistrationRequired;
    IMS_SINT32 nSipServerPortNumber;
    IMS_BOOL bKeepPdnUpInNoVops;
    IMS_SINT32 nSipPreferredTransport;
    IMS_SINT32 nIpv4SipMtuSizeCellular;
    IMS_SINT32 nIpv6SipMtuSizeCellular;
    IMSVector<IMS_SINT32> objImsPdnEnabledInNoVopsSupport;
    IMS_BOOL bSipOverIpsecEnabled;
    IMSVector<IMS_SINT32> objIpsecAuthenticationAlgorithms;
    IMSVector<IMS_SINT32> objIpsecEncryptionAlgorithms;
    IMS_SINT32 nRegistrationExpiryTimerSec;
    IMS_SINT32 nRegistrationRetryBaseTimerMillis;
    IMS_SINT32 nRegistrationRetryMaxTimerMillis;
    IMS_BOOL bRegistrationEventPackageSupported;
    IMS_SINT32 nRegistrationSubscribeExpiryTimerSec;
    IMSVector<IMS_SINT32> objGeolocationPidfInSipRegisterSupport;
    IMSVector<IMS_SINT32> objSupportedRats;

    /// imsvoice.
    IMS_BOOL bCarrierVolteRoamingAvailable;

    /// imssms.
    IMS_BOOL bSmsOverImsSupported;
    IMSVector<IMS_SINT32> objSmsOverImsSupportedRats;

    /// imsrtt.

    /// imsemergency.
    IMS_BOOL bEmergencyCallbackModeSupported;
    IMSVector<IMS_SINT32> objEmergencyOverImsSupportedRats;
    IMS_SINT32 nEmergencyRegistrationTimerMillis;
    IMS_SINT32 nRefreshGeolocationTimeoutMillis;

    /// imsvt.

    /// imswfc.

    /* carrier_config */
    /// ims
    IMS_SINT32 nImsPreferredIpType;
    IMSVector<IMS_SINT32> objImsIdentityPriority;
    IMS_SINT32 nIsimIndexForImpu;
    IMSVector<IMS_SINT32> objUpdateRegistrationWithRatChange;
    IMS_BOOL bUnsubscribeRegistrationEventPackage;
    IMS_BOOL bRegistrationEventForCatRequired;
    IMS_SINT32 nPreferredImsDscp;
    IMS_SINT32 nImsSignallingDscp;
    IMS_SINT32 nRegistrationPreferredAccesstypeFeatureTag;
    IMS_SINT32 nRegistrationActualWaitTimePolicy;

    /// imssms.
    IMS_BOOL bSupportLimitedAdminSmsMode;

    /// imsemergency.
    IMS_BOOL bReleaseEmergencyPdnWithEmergencyCallEnd;
    IMS_BOOL bDisableT3482ForEmergency;
    IMS_SINT32 nRegistrationTimerForEmergencyCallMillis;
    IMS_SINT32 nPreferredEmergencyRegistration;

    /// imswfc.
    IMS_BOOL bUpdateRegistrationWithCountryChange;
    IMS_SINT32 nRegistrationPrivateHeader;
};
#endif // _AOS_CARRIER_CONFIG_H_
