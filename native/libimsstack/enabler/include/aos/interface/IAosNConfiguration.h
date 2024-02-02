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
#ifndef INTERFACE_AOS_NCONFIGURATION_H_
#define INTERFACE_AOS_NCONFIGURATION_H_

#include "ImsTypeDef.h"

#define GET_N_CONFIG(SLOT) (AosProvider::GetInstance()->GetNConfiguration(SLOT))

class IAosNConfigurationListener;

/**
 * @brief This class provids an interface to access the configuration information related to Aos.
 *
 */
class IAosNConfiguration
{
public:
    virtual ~IAosNConfiguration(){};

    /**
     * @brief Get the slot id to be retrieved.
     *
     * @return IMS_SINT32 Return slot id to be retrieved
     */
    virtual IMS_SINT32 GetSlotId() const = 0;

    /**
     * @brief Set the listener for monitoring the configuration.
     *
     * @param piListener Indicate the configuration listener.
     */
    virtual void SetListener(IN IAosNConfigurationListener* piListener) = 0;

    /**
     * @brief Remove the listener for monitoring the configuration.
     *
     * @param piListener Indicate the configuration listener.
     */
    virtual void RemoveListener(IN IAosNConfigurationListener* piListener) = 0;

    /**
     * @brief Returns whether subscription is initiated after registration.
     *
     * @return IMS_BOOL Return whether to be applied or not
     */
    virtual IMS_BOOL IsSubscription() const = 0;
    /**
     * @brief Returns whether un-subscription is initiated before de-registration.
     *
     * @return IMS_BOOL Return whether to be applied or not
     */
    virtual IMS_BOOL IsUnSubscription() const = 0;

    /**
     * @brief Check if VoLTE is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     */
    virtual IMS_BOOL IsVoLteAvailable() const = 0;

    /**
     * @brief Check if VoLTE roaming is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     */
    virtual IMS_BOOL IsVoLteRoamingAvailable() const = 0;

    /**
     * @brief Check if Video calling is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     */
    virtual IMS_BOOL IsVtAvailable() const = 0;

    /**
     * @brief Check if WFC is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     */
    virtual IMS_BOOL IsWfcImsAvailable() const = 0;

    /**
     * @brief Check if single registration is required.
     *
     * @return IMS_TRUE if required, IMS_FALSE if not required.
     */
    virtual IMS_BOOL IsImsSingleRegistrationRequired() const = 0;

    /**
     * @brief Check if RTT is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     */
    virtual IMS_BOOL IsRttSupported() const = 0;

    /**
     * @brief Check if limited admin sms mode is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     */
    virtual IMS_BOOL IsSupportLimitedAdminSmsMode() const = 0;

    /**
     * @brief Check if TTY is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     */
    virtual IMS_BOOL IsTtySupported() const = 0;

    /**
     * @brief Check if VOPS is ignored for VoLTE enable.
     *
     * @return IMS_TRUE if ignored, IMS_FALSE if considered.
     */
    virtual IMS_BOOL IsVopsIgnoredForVolteEnabled() const = 0;

    /**
     * @brief Check if Sms Over Ims is available without voice capability.
     *
     * @return IMS_TRUE if available, IMS_FALSE if not available.
     */
    virtual IMS_BOOL IsSmsOverImsAvailableWithoutVoiceCapability() const = 0;

    /**
     * @brief Check if VoLTE service block by SSAC is required.
     *
     * @return IMS_TRUE if required, IMS_FALSE if not required.
     */
    virtual IMS_BOOL IsRequiredVolteBlockBySsac() const = 0;

    /**
     * @brief Check if WFC service block by Airplane mode is required.
     *
     * @return IMS_TRUE if required, IMS_FALSE if not required.
     */
    virtual IMS_BOOL IsRequiredWfcBlockByAirplaneMode() const = 0;

    /**
     * @brief Returns whether re-registration is sent on Wifi when the country is changed.
     *
     * @return IMS_BOOL Return whether to be sent re-registration or not
     */
    virtual IMS_BOOL IsReregRetryWithChangedCountryOnWifi() const = 0;

    /**
     * @brief Returns whether IPSec is enabled for SIP messages in roaming networks.
     *
     * @return IMS_BOOL Return whether to be applied or not
     */
    virtual IMS_BOOL IsSipOverIpsecInRoamingEnabled() const = 0;

    /**
     * @brief Check if conuntry code availability check for wfc is required.
     *
     * @return IMS_TRUE if required, IMS_FALSE if not required.
     */
    virtual IMS_BOOL UseWfcCountryCodeAvailabilityCheck() const = 0;

    /**
     * @brief Returns whether subscription also use the registration retry intervals.
     *
     *        This function relates to GetRegRetryIntervals() and
     *        GetRegRandomRetryIntervals()
     *        Subscription decides whether to follow the mentioned function by this return value.
     *
     * @return IMS_BOOL Return whether to be applied or not
     */
    virtual IMS_BOOL IsRegRetryIntervalsUsedForSub() const = 0;

    /**
     * @brief Check if SmsOverIp is enabled.
     *
     * @return IMS_TRUE if enabled, IMS_FALSE if not enabled.
     */
    virtual IMS_BOOL IsSmsOverIpEnabled() const = 0;

    /**
     * @brief Returns whether the ipsec is supported or not.
     *
     * @return IMS_BOOL Return wherther to be applied or not
     */
    virtual IMS_BOOL IsIpsecEnabled() const = 0;

    /**
     * @brief Returns whether the security server port is used in reg contact of
     *        initial registration.
     *
     * @return IMS_BOOL Return wherther to be applied or not
     */
    virtual IMS_BOOL IsSecurityServerPortInRegContactOfInitRegUsed() const = 0;

    /**
     * @brief Returns whether the security server port is used in initial registration.
     *
     * @return IMS_BOOL Return wherther to be applied or not
     */
    virtual IMS_BOOL IsSecurityServerPortInInitRegUsed() const = 0;

    /**
     * @brief Returns whether the old security association is removed on establishing
     *        the security association
     *
     * @return IMS_BOOL Return wherther to be applied or not
     */
    virtual IMS_BOOL IsOldSaOnEstablishingSaRemoved() const = 0;

    /**
     * @brief Returns whether a call should be terminated due to expiration of registration.
     *
     * @return IMS_BOOL Return wherther to be applied or not
     */
    virtual IMS_BOOL IsCallEndAndPdnReactivationByRegTerminated() const = 0;

    /**
     * @brief Returns whether unsecure tcp socket is destroyed on accomplishing the registration
     *
     * @return IMS_BOOL Return wherther to be applied or not
     */
    virtual IMS_BOOL IsUnsecureTcpSocketOnAccomplishingRegDestroyed() const = 0;

    /**
     * @brief Returns whether emergency PDN shall be released after the E911 call is ended.
     *
     * @return IMS_BOOL Return wherther to be applied or not
     */
    virtual IMS_BOOL IsEmergencyPdnWithEmergencyCallEndReleased() const = 0;

    /**
     * @brief Flag specifying if SMS over IMS support is available or not.
     *
     * @return IMS_BOOL Return wherther to be available or not
     */
    virtual IMS_BOOL IsSmsOverImsSupported() const = 0;

    /**
     * @brief Flag specifying if IMS over NR is enabled or not.
     *
     * @return IMS_BOOL Return wherther to be enabled or not
     */
    virtual IMS_BOOL IsImsOverNrEnabled() const = 0;

    /**
     * @brief Flag indicating whether the authorized IMPU from P-Associated-URI header in 200 OK
     *        for IMS registration is used in emergency call.
     *
     * @return IMS_BOOL Return wherther to be supported or not
     */
    virtual IMS_BOOL IsEmergencyCallBasedOnPauOfNormalRegistrationSupported() const = 0;

    /**
     * @brief Flag specifying if UE tries emergency registration on a random pcscf.
     *
     *        If this is set as TRUE, UE will choose P-CSCF randomly for emergency registration if
     *        UE receives multiple P-CSCF addresses from P-CSCF discovery for emergency.
     *
     * @return IMS_BOOL Return wherther UE tries emergency registration on random pcscf.
     */
    virtual IMS_BOOL IsEmcRegOnRandomPcscf() const = 0;

    /**
     * @brief Flag specifying whether the re-registration is held when IPCAN is changed
     *        during IMS calls and performed immediately after they are released.
     *
     * @return IMS_BOOL Return wherther to be held or not
     */
    virtual IMS_BOOL IsRegWithIpcanChangedDuringImsCallHeld() const = 0;

    /**
     * @brief Flag specifying whether IMS de-registers if the network is changed to 3G
     *        while IMS registered.
     *
     * @return IMS_BOOL Return wherther IMS de-registers or not.
     */
    virtual IMS_BOOL IsDeregOn3gNetwork() const = 0;

    /**
     * @brief Flag indicating whether ipsec setting is initialized with new pcscf.
     *
     * @return IMS_BOOL Return whether initialize ipsec wetting with new pcscf or not.
     */
    virtual IMS_BOOL IsIpsecInitializedWithNewPcscf() const = 0;

    /**
     * @brief Returns whether UE doesn't send initial registration due to pcscf change
     *
     *        If this is set as TRUE, UE doesn't send initial registration due to pcscf change.
     *
     * @return IMS_BOOL Return whether the logic is applied or not.
     */
    virtual IMS_BOOL IsNoInitRegOnPcscfChange() const = 0;

    /**
     * @brief Flag specifying if video call is supported over wifi when voice call is unavailable.
     *
     * @return IMS_BOOL Return wherther video call is supported over wifi
     *         when voice call is unavailable.
     */
    virtual IMS_BOOL IsVideoOverWifiSupportedWithoutVoice() const = 0;

    /**
     * @brief Flag specifying if Geolocation-PIDF is supported.
     *
     * @param IMS_SINT32 The Geolocation-PIDF type to be evaluated.\n
     *        #CarrierConfig#Ims#GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI\n
     *        #CarrierConfig#Ims#GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI\n
     *        #CarrierConfig#Ims#GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR\n
     *        #CarrierConfig#Ims#GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR
     *
     * @return IMS_TRUE if the specified Geolocation-PIDF type is supported, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsGeolocationPidfSupported(IN IMS_SINT32 nGeolocationPidfType) const = 0;

    /**
     * @brief Flag specifying if g.gsma.rcs.telephony feature tag is used
     *        to indicate available voice call type ("cs", "volte" or "cs,volte").
     *        Example) g.gsma.rcs.telephony = "cs,volte"
     *
     * @return IMS_BOOL Return wherther the feature tag is used
     *                  to indicate available voice call type.
     */
    virtual IMS_BOOL IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType() const = 0;

    /**
     * @brief Flag specifying if "+cdmaless" feature tag is required.
     *
     * @return IMS_BOOL Return wherther the feature tag is required.
     */
    virtual IMS_BOOL IsCdmalessFeatureTagRequired() const = 0;

    /**
     * @brief Flag indicating whether the defined error codes are only applied with Retry-After
     *        header value.
     *
     *        This function relates to GetRegErrCodeWithRetryAfterTime() and
     *        GetReregErrCodeWithRetryAfterTime()
     *
     * @return IMS_BOOL Return wherther the defined error codes is used.
     */
    virtual IMS_BOOL IsRegErrCodeWithRetryAfterTimeOnlyDefined() const = 0;

    /**
     * @brief Returns whether the registration is handled as a failure when the UE receives
     *        an error response against re-registration in roaming
     *
     * @return IMS_BOOL Return wherther to be handled as a failure or not.
     */
    virtual IMS_BOOL IsExtraReregErrInRoamingAsFailureHandled() const = 0;

    /**
     * @brief Flag indicating whether the retry counter should be shared between REGISTER and
     *        SUBSCRIBE for reg event package.
     *
     * @return IMS_BOOL Return whether to be suppoted retry count shared for registration and
     *         reg event package
     */
    virtual IMS_BOOL IsExtraRegErrRetryCntSharedForRegAndSubRequired() const = 0;

    /**
     * @brief Returns Flag specifying whether UE should enter Emergency CallBack Mode (ECBM)
     *        after emergency call is ended.
     *
     * @return IMS_BOOL Return whether to be supported or not
     */
    virtual IMS_BOOL IsEmergencyCallbackModeSupported() const = 0;

    /**
     * @brief Returns Flag indicating whether or not sending emergency SMS messages over IMS
     *        is supported when in LTE/limited LTE (Emergency only) service mode.
     *
     * @return IMS_BOOL Return whether to be supported or not
     */
    virtual IMS_BOOL IsEmergencySmsOverImsSupported() const = 0;

    /**
     * @brief Returns flag indicating whether the Contact URI validation is checked
     *        when the 200 OK response for SIP REGISTER is received.
     *        If the Contact URI is not matched, the 200 OK response is disregarded.
     *
     * @return IMS_BOOL Return whether to be checked or not
     */
    virtual IMS_BOOL IsContactUriValidationChecked() const = 0;

    /**
     * @brief Returns whether IMS registration is retried based on IP version fallback
     *        from IPv6 to IPv4
     *
     * @return IMS_BOOL Return whether to be applied or not
     */
    virtual IMS_BOOL IsRegRetryWithIpVerFallback() const = 0;

    /**
     * @brief Flag indicating whether the User Info is included or not in contact
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     */
    virtual IMS_BOOL IsUserInfoInContactSupported() const = 0;

    /**
     * @brief Flag indicating the initial registration is tried on not
     *        right after IMS call is ended while registration is held bacuase re-registration is
     *        failed during active call.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     */
    virtual IMS_BOOL IsRegRequiredAfterImsCallEndOnRegHeld() const = 0;

    /**
     * @brief Flag indicating reg includes feature tag even though the feature is not available.
     *
     * @return IMS_BOOL Return wherther the feature tag is included.
     */
    virtual IMS_BOOL IsRegWithFeatureTagUnavailableSupported() const = 0;

    /**
     * @brief Flag specifying if verstat is supported for registration or not
     *
     * @return IMS_BOOL Return wherther to be supported or not
     */
    virtual IMS_BOOL IsVerstatForRegistrationSupported() const = 0;

    /**
     * @brief Flag specifying if service fallback is required when voice call is unavailable.
     *
     * @return IMS_TRUE if required, else IMS_FALSE
     */
    virtual IMS_BOOL IsPlmnBlockWithTimeoutOnVoiceCallUnavailable() const = 0;

    /**
     * @brief Flag specifying if WFC error message is support.
     *
     * @return IMS_TRUE if support, else IMS_FALSE
     */
    virtual IMS_BOOL IsWfcErrorMessageSupported(IN IMS_SINT32 nError) const = 0;

    /**
     * @brief Get the registration retry base-time
     *
     *        This value defines as per RFC 5626 section 4.5
     *        It defines in millisecond.
     *
     * @return IMS_UINT32 Return retry base-time value
     */
    virtual IMS_UINT32 GetRegistrationRetryBaseTime() = 0;

    /**
     * @brief Get the registration retry max-time
     *
     *         This value defines as per RFC 5626 section 4.5
     *         It defines in millisecond.
     *
     * @return IMS_UINT32 Return retry max-time value
     */
    virtual IMS_UINT32 GetRegistrationRetryMaxTime() = 0;

    /**
     * @brief Get the ISIM index for IMPU.
     *
     * @return IMS_UINT32 Returns ISIM index for IMPU.
     */
    virtual IMS_UINT32 GetIsimIndexForImpu() = 0;

    /**
     * @brief Get the IMS establishment time.
     *
     * @return IMS_SINT32 Returns IMS establishment time.
     */
    virtual IMS_SINT32 GetImsEstablishmentTime() const = 0;

    /**
     * @brief Indicate whether DSCP(Differentiated Services (Diffserv) Codepoint)
     *        for SIP packets is set or not.
     *
     *        Specify the preferred ipcan for SIP Packets.
     *        Possible values are,
     *        CarrierConfig::Ims::PREFERRED_DSCP_NONE
     *        CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR
     *        CarrierConfig::Ims::PREFERRED_DSCP_WIFI
     *        CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR_WIFI
     *
     * @return IMS_SINT32 Return the preferred IP-CAN type for SIP Packets
     */
    virtual IMS_SINT32 GetPreferredImsDscp() const = 0;

    /**
     * @brief Indicate whether the g.3gpp.accesstype feature tag indicating the access type
     *        in contact header of SIP REGISTER is included based on 3GPP TS 24.292.
     *
     *        Specify the preferred access feature tag of SIP REGISTER.
     *        Possible values are,
     *        CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED
     *        CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED
     *        CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED_WITHOUT_NUMERICAL_VALUE
     *
     * @return IMS_SINT32 Return the preferred IP-CAN type for SIP Packets
     */
    virtual IMS_SINT32 GetRegistrationPreferredAccessTypeFeatureTag() const = 0;

    /**
     * @brief Get USSD preference method.
     *
     * @return IMS_SINT32 Returns USSD preference value.
     * @see CarrierConfig::USSD_OVER_CS_PREFERRED
     *      CarrierConfig::USSD_OVER_IMS_PREFERRED
     *      CarrierConfig::USSD_OVER_CS_ONLY
     *      CarrierConfig::USSD_OVER_IMS_ONLY
     */
    virtual IMS_SINT32 GetUssdMethod() const = 0;

    /**
     * @brief Get the preferred IP address type
     *
     *        If both IPv4 and IPv6 addresses are assigned by the network to the UE,
     *        the UE use preferred IP address type of each operator's requirement.
     *
     * @return IMS_SINT32
     * @see CarrierConfig::Assets::IP_VERSION_4
     */
    virtual IMS_SINT32 GetPreferredIpType() const = 0;

    /**
     * @brief Get the preferred IP address type for emergency
     *
     *        If both IPv4 and IPv6 addresses are assigned by the network to the UE,
     *        the UE use preferred IP address type of each operator's requirement.
     *
     * @return IMS_SINT32
     * @see CarrierConfig::Assets::IP_VERSION_4
     */
    virtual IMS_SINT32 GetEmergencyPreferredIpType() const = 0;

    /**
     * @brief Get the retry attempt count about pcscfs discovered during emergency PDN/PDU setup.
     *
     *       Specify the number of emergency registration retry attempt to P-CSCFs. UE will try
     *       emergency registration with specified number of P-CSCFs when
     *       CarrierConfig::Assets::KEY_EMC_REG_RETRY_TIMER_MILLIS_INT timer has expired. If the
     *       number is zero, UE will try registration on every P-CSCFs once. If the number of
     *       P-CSCF is less than a given number and UE's default retry policy is a
     *       CarrierConfig::Assets::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF, UE will try
     *       registration from the first P-CSCF again after attempting on all P-CSCFs.
     *       If UE doesn't support emerg-reg-retry defined in 3GPP 24.229, which is configured by
     *       CarrierConfig::Assets::KEY_EMC_REG_RETRY_TIMER_MILLIS_INT, this configuration is
     *       discarded.
     *
     * @return IMS_SINT32 Return the retry attempt count
     */
    virtual IMS_SINT32 GetEmcRegRetryMaxCnt() const = 0;

    /**
     * @brief Get the maximum time waiting for emergency registration
     *
     *        Specify the maximum time from sending a SIP REGISTER for an emergency registration
     *        until UE receives any final response from this P-CSCF. It will be stopped when the
     *        CarrierConfig::ImsEmergency::KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT timer has
     *        been stopped or expired. Upon this timer expiry, the UE considers that the emergency
     *        registration attempt for this P-CSCF has failed. The UE may retry registration on
     *        a different P-CSCF if available and restart the
     *        CarrierConfig::Assets::KEY_EMC_REG_RETRY_TIMER_MILLIS_INT timer. If the UE has no
     *        more available P-CSCFs, the UE shall stop the
     *        CarrierConfig::ImsEmergency::KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT timer by
     *        considering the emergency registration has failed. If the value is zero, it considers
     *        that the UE doesn't support the emerg-reg-retry timer defined in 3GPP 24.229.
     *
     * @return IMS_SINT32 Return the milli-second time
     */
    virtual IMS_SINT32 GetEmcRegRetryTimerMillis() const = 0;

    /**
     * @brief Get the IMS Server default port as per operator
     *
     * @return IMS_SINT32 Return default P-CSCF port number
     */
    virtual IMS_SINT32 GetPcscfPort() const = 0;

    /**
     * @brief Get the preferred transport protocol for SIP messages.
     *        Possible values are,
     *        #PREFERRED_TRANSPORT_UDP,
     *        #PREFERRED_TRANSPORT_TCP,
     *        #PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP,
     *        #PREFERRED_TRANSPORT_TLS
     *
     * @return IMS_SINT32
     * @see CarrierConfig::Ims::PREFERRED_TRANSPORT_UDP
     */
    virtual IMS_SINT32 GetSipPreferredTransport() const = 0;

    /**
     * @brief Get the maximum IPV4 MTU size of SIP message on Cellular.
     *
     *        If GetSipPreferredTransport() is #PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP and
     *        IPv4 SIP message size is more than this value, then SIP transport will be TCP,
     *        else the SIP transport is UDP.
     *
     * @return IMS_SINT32 Return IPV4 MTU size of SIP message
     */
    virtual IMS_SINT32 GetIpv4MtuSize() const = 0;

    /**
     * @brief Get the maximum IPV6 MTU size of SIP message on Cellular.
     *
     *        If GetSipPreferredTransport() is #PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP and
     *        IPv6 SIP message size is more than this value, then SIP transport will be TCP,
     *        else the SIP transport is UDP.
     *
     * @return IMS_SINT32 Return IPV6 MTU size of SIP message
     */
    virtual IMS_SINT32 GetIpv6MtuSize() const = 0;

    /**
     * @brief Indicate whether emergency call is tried without emergency registration
     *
     *        Specify the preferred policy for emergency registration.
     *        Possible values are,
     *        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_SKIP
     *        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL
     *        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK
     * @return IMS_SINT32 Return the preferred policy for emergency registration
     */
    virtual IMS_SINT32 GetPreferredEmergencyRegistration() const = 0;

    /**
     * @brief Get the maximum time waiting for emergency service
     *
     *        Specify the maximum time from deciding that an emergency service is to
     *        be established until completion of the emergency registration procedure.
     *        Upon timer expiry, the UE considers the emergency REGISTER request or
     *        the emergency call attempt as failed, and stop the
     *        CarrierConfig::Assets::KEY_EMC_REG_RETRY_TIMER_MILLIS_INT timer, if running.
     *
     * @return IMS_SINT32 Return the milli-second time
     */
    virtual IMS_SINT32 GetEmergencyRegistrationTimerMillis() const = 0;

    /**
     * @brief Indicate the DSCP for IMS signalling
     *
     *        If GetPreferredImsDscp() is not PREFERRED_DSCP_NONE, it will be set
     *        according to IP connectivity access network.
     *        DSCP integer must be between 8 and 56.
     *
     * @return IMS_SINT32 Return the DSCP value for IMS signalling
     */
    virtual IMS_SINT32 GetImsSignallingDscp() const = 0;

    /**
     * @brief Indicate whether private header like P-Cellular-Network-Info or
     *        P-Last-Access-Network-Info is supported during the registration over WiFi
     *
     *        Possible values are,
     *        CarrierConfig::ImsWfc::REGISTRATION_P_NOT_SUPPORTED
     *        CarrierConfig::ImsWfc::REGISTRATION_P_CELLULAR_NETWORK_INFO
     *        CarrierConfig::ImsWfc::REGISTRATION_P_LAST_ACCESS_NETWORK_INFO
     *
     * @return IMS_SINT32 Return the preferred private hader value
     */
    virtual IMS_SINT32 GetRegistrationPrivateHeader() const = 0;

    /**
     * @brief Indicate the actual wait time policy
     *
     *        Possible values are,
     *        CarrierConfig::Assets::AWT_POLICY_RFC_RULE
     *            Follow RFC 5626 section 4.5.
     *        CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EVERY_PCSCF
     *            Indicate whether it shall calculate a wait time based on RFC 5626 4.5
     *            if the registration to every known PCSCF is attempted and handled
     *            as temporary failure without Retry-After header.
     *        CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EACH_PCSCF
     *            Indicate whether it shall calculate a wait time based on RFC 5626 4.5
     *            if the registration to each known PCSCF is attempted and handled
     *            as temporary failure without Retry-After header.
     *        CarrierConfig::Assets::AWT_POLICY_SPECIFIED_INTERVAL
     *            @see KEY_REG_RETRY_INTERVAL_BUNDLE
     *
     * @return IMS_SINT32 Return the actual wait time policy
     */
    virtual IMS_SINT32 GetRegActualWaitTimePolicy() const = 0;

    /**
     * @brief Get the out of service policy object
     *
     *        CarrierConfig::Assets::REG_OOS_POLICY_DEFAULT
     *            Indicate that reregistration is not tried during OOS and
     *            reregistratioin is attempted after network service state is changed to in service
     *            and registration is not expired.
     *        CarrierConfig::Assets::REG_OOS_POLICY_DESTROY
     *            Indicate that registration is terminated if registration is refreshed during OOS.
     *            When network service state is changed to in service,
     *            the initial registration is tried.
     *
     * @return IMS_SINT32 Return the out of service policy
     */
    virtual IMS_SINT32 GetRegOutOfServicePolicy() const = 0;

    /**
     * @brief Check what registration rule is required in making an emergency call in roaming.
     *
     *       Specify the preferred policy for emergency registration in roaming.
     *       Return value is to follow used configuration in GetPreferredEmergencyRegistration().
     *       (Additionally, PREFERRED_EMERGENCY_REGISTRATION_NOT_DEFINED config is added
     *        for this function.)
     *        Possible values are,
     *        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NOT_DEFINED
     *        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_SKIP
     *        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL
     *        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK
     * @note  If 'PREFERRED_EMERGENCY_REGISTRATION_NOT_DEFINED' is returned,
     *        the UE in roaming follows the same rule as the home.
     *
     * @return IMS_SINT32 Return the emerrgency registration policy in roaming
     */
    virtual IMS_SINT32 GetRoamingPreferredEmcReg() const = 0;

    /**
     * @brief Get the SIP message threshold size caused by the transport change
     *
     *        If GetSipPreferredTransport() is #PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP and
     *        SIP message size is more than MTU size - this value, then SIP transport will be TCP,
     *        else the SIP transport is UDP.
     *
     * @return IMS_SINT32 Return the threshold size of SIP message
     */
    virtual IMS_SINT32 GetSipMessageThresholdForTransportChange() const = 0;

    /**
     * @brief Indicate the USAT IMS registration event download policy.
     *
     *        Specify the support policy of USAT IMS registration event download
     *        TS 131.111 7.5.21 describes about the USAT IMS registration event download
     *        and the conditions to be notified to UICC when an IMS registration event occurs.
     *        This asset indicates whether USAT IMS registration event download is supported,
     *        and if so, whether to check the precondition before notifying.
     *        Possible values are,
     *        CarrierConfig::Assets::USAT_REG_EVENT_NOT_DOWNLOAD
     *        CarrierConfig::Assets::USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD
     *        CarrierConfig::Assets::USAT_REG_EVENT_CONDITIONAL_DOWNLOAD
     *
     * @return IMS_SINT32 Returns registration event download policy for USAT.
     */
    virtual IMS_SINT32 GetUsatRegEventDownloadPolicy() const = 0;

    /**
     * @brief Get the VoLTE Hysteresis time.
     *
     * @return IMS_SINT32 Returns VoLTE Hysteresis time.
     */
    virtual IMS_SINT32 GetVolteHysTime() const = 0;

    /**
     * @brief Indicate the SIP 305 response policy for registration
     *
     *        Possible values are,
     *        CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT
     *            Follow KEY_DEFAULT_RETRY_POLICY_INT operation if 305 code is not configured
     *            from other configurations of this bundle.
     *        CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP
     *            Follow 3GPP 24.229.
     *        CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF
     *            Follow 3GPP 24.229 starting from the top of the existing PCSCF list.
     *        CarrierConfig::Assets::SIP_305_CODE_POLICY_USE_CONTACT_VALUE
     *            Flag indicating whether a new IMS registration is tried
     *            using the contact header field value as specified in RFC3261
     *            after a 305 response for registration is received.
     *
     * @return IMS_SINT32 Return the SIP 305 response policy
     */
    virtual IMS_SINT32 GetRegRetrySip305CodePolicy() const = 0;

    /**
     * @brief Indicate the SIP 305 response policy for reregistration
     *
     *        Possible values are,
     *        CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT
     *            Follow KEY_DEFAULT_RETRY_POLICY_INT operation if 305 code is not configured
     *            from other configurations of this bundle.
     *        CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP
     *            Follow 3GPP 24.229.
     *        CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF
     *            Follow 3GPP 24.229 starting from the top of the existing PCSCF list.
     *        CarrierConfig::Assets::SIP_305_CODE_POLICY_USE_CONTACT_VALUE
     *            Flag indicating whether a new IMS registration is tried
     *            using the contact header field value as specified in RFC3261
     *            after a 305 response for registration is received.
     *
     * @return IMS_SINT32 Return the SIP 305 response policy
     */
    virtual IMS_SINT32 GetReregRetrySip305CodePolicy() const = 0;

    /**
     * @brief Indicate the SIP 503 response policy for registration
     *
     *        Possible values are,
     *        CarrierConfig::Assets::SIP_503_CODE_POLICY_DEFAULT
     *            Follow KEY_DEFAULT_RETRY_POLICY_INT operation if 305 code is not configured
     *            from other configurations of this bundle.
     *        CarrierConfig::Assets::SIP_503_CODE_POLICY_3GPP
     *            Follow 3GPP 24.229.
     *
     * @return IMS_SINT32 Return the SIP 503 response policy
     */
    virtual IMS_SINT32 GetRegRetrySip503CodePolicy() const = 0;

    /**
     * @brief Get the number of registration retry when there is only one PCSCF.
     *
     *        Indicate the number of retry for the PCSCF when there is only one PCSCF.
     *        If the value is 0, it is not applicable.
     *
     * @return IMS_SINT32 Return the number of retry
     */
    virtual IMS_SINT32 GetRegRetryCountOnSinglePcscf() const = 0;

    /**
     * @brief Get the number of registration retry for each PCSCF
     *
     *        Indicate the number of retry for each PCSCF.
     *        If the value is 0, it follows default scheme.
     *
     * @return IMS_SINT32 Return the number of retry
     */
    virtual IMS_SINT32 GetRegRetryCountPerPcscf() const = 0;

    /**
     * @brief Indicate the policy for clearing the registration retry count
     *
     *        Possible values are,
     *        CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION
     *        CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION
     *        CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_NOTIFY
     *
     * @return IMS_SINT32 Return the policy of clearing the retry count for registration
     */
    virtual IMS_SINT32 GetRegRetryCountResetPolicy() const = 0;

    /**
     * @brief Get the numbner of registration retry to maintain ipsec setting on authentication
     *        failure.
     *
     * @return IMS_SINT32 Return the number of retry
     */
    virtual IMS_SINT32 GetRegRetryCountWithIpsecOnAuthFailure() const = 0;

    /**
     * @brief Get the default policy for the registration retry with pcscf selection.
     *
     *        Specify the default retry policy about how to use the PCSCF address selection.
     *        Possible values are,
     *        CarrierConfig::Assets::DEFAULT_RETRY_POLICY_SPEC
     *        CarrierConfig::Assets::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF
     *        CarrierConfig::Assets::DEFAULT_RETRY_POLICY_NEXT_PCSCF
     *
     * @return IMS_SINT32 Return the default policy for the registration retry
     */
    virtual IMS_SINT32 GetRegRetryDefaultPolicy() const = 0;

    /**
     * @brief Get the Reg Retry Timer F Policy object
     *
     *        This is related to managing pcscf. In case the UE uses this config.
     *        A value of KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY and
     *        KEY_REREG_ERR_CODE_FOR_INIT_REG_WITH_AVAILABLE_PCSCF_INT_ARRAY must be set to 0
     *        as well. (0: TimerF)
     *
     * @return IMS_SINT32 Return retry policy
     */
    virtual IMS_SINT32 GetRegRetryTimerFPolicy() const = 0;

    /**
     * @brief Get a waiting max timer to proceed IMS registration for an emergency call.
     *
     *        After the timer expires, the emergency call is handled as failure.
     *        Indicate the number as millisecond.
     *
     * @return IMS_SINT32 Return a waiting reg timer for an emergency call
     */
    virtual IMS_SINT32 GetRegTimerForEmcCall() const = 0;

    /**
     * @brief Indicate the extra error type for registration
     *
     *        Possible values are,
     *        CarrierConfig::Assets::ERROR_TYPE_NOT_SPECIFIED
     *        CarrierConfig::Assets::ERROR_TYPE_REPEATED
     *            Indicate that it results in blocking PLMN with the specific protocol timer.
     *            like T3402.
     *        CarrierConfig::Assets::ERROR_TYPE_CRITICAL
     *            Indicate that it results in blocking PLMN.
     *        CarrierConfig::Assets::ERROR_TYPE_ROAMING
     *            Indicate that it results in blocking PLMN basd on the attached network type.
     *        CarrierConfig::Assets::ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK
     *            Indicate that it results in blocking PLMN with the specific protocol timer
     *            like T3402.
     *
     * @return IMS_SINT32 Return the extra error type
     */
    virtual IMS_SINT32 GetExtraRegErrFinalType() const = 0;

    /**
     * @brief Indicate the extra error policy for registration
     *
     *        Possible values are,
     *        CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED
     *        CarrierConfig::Assets::ERROR_POLICY_PCSCF_FAILED
     *        CarrierConfig::Assets::ERROR_POLICY_SUBSCRIBER_FAILED
     *        CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED
     *
     * @return IMS_SINT32 Return the extra error policy
     */
    virtual IMS_SINT32 GetExtraRegErrPolicy() const = 0;

    /**
     * @brief Indicate max of retry count the extra error for registration
     *
     *      CarrierConfig::Assets::ERROR_TYPE_REPEATED
     *          Indicate the maximum retry count with the same PCSCF.
     *          If the retry count reaches the maximum count, initial registration is tried
     *          with other PCSCF.
     *      CarrierConfig::Assets::ERROR_TYPE_CRITICAL
     *          Indicate the number of error response that is included in KEY_ERROR_CODE_INT_ARRAY.
     *          If this number reaches, it is handled as a critical error.
     *
     * @return IMS_SINT32 Return max of the extra error
     */
    virtual IMS_SINT32 GetExtraRegErrMaxCount() const = 0;

    /**
     * @brief Indicate the minimum number for retries when registration or reregistration fails.
     *
     *        In case of initial registration, it is retried with the next PCSCF that is available.
     *        But in case of reregistration, it is retried with the same PCSCF according to
     *        the minimum number of times.
     *
     * @return IMS_SINT32 Return min of the extra error
     */
    virtual IMS_SINT32 GetExtraRegErrMinCount() const = 0;

    /**
     * @brief Indicate which the PCSCF address  to use when the PCSCF address are changed
     *        with address and order.
     *
     *        Possible values are,
     *        CarrierConfig::Assets::REG_PCSCF_UPDATE_POLICY_DEFAULT
     *            Indicate that registration is tried with new PCSCF address
     *            only when the current PCSCF address that is used for registration
     *            is not contained in the new PCSCF list.
     *        CarrierConfig::Assets::REG_PCSCF_UPDATE_POLICY_ALL_THE_TIME
     *            Indicate that registration or reregistration is always tried
     *            when the PCSCF addresses are changed regardless of the existence
     *            of the current PCSCF address in the new PCSCF list.
     *
     * @return IMS_SINT32 Return the policy for which the pcscf address to use
     */
    virtual IMS_SINT32 GetRegistrationPcscfUpdatePolicy() const = 0;

    /**
     * @brief Indicate which policy is applied to set user info for none register message.
     *        It will be applied in all the outgoing SIP request adn response
     *        except for register request.
     *
     *        Possible values are,
     *        CarrierConfig::Assets::CONTACT_USER_INFO_POLICY_DEFAULT
     *        CarrierConfig::Assets::CONTACT_USER_INFO_POLICY_NONE
     *        CarrierConfig::Assets::CONTACT_USER_INFO_POLICY_NO_IMSI
     *
     * @return IMS_SINT32 Return the policy of setting the user info
     */
    virtual IMS_SINT32 GetUserInfoPolicyForNonRegisterMessage() const = 0;

    /**
     * @brief Indicate which policy is applied for creating geolocation pidf.
     *
     *        Possible values are,
     *        CarrierConfig::Assets::GEOLOCATION_POLICY_WITHOUT_POSITION
     *        CarrierConfig::Assets::GEOLOCATION_POLICY_WITH_POSITION
     *        CarrierConfig::Assets::GEOLOCATION_POLICY_WITH_POSITION_AND_COUNTRY
     *        CarrierConfig::Assets::GEOLOCATION_POLICY_WITHOUT_CIVIC
     *
     * @return IMS_SINT32 Return the policy of setting the user info
     */
    virtual IMS_SINT32 GetGeolocationPidfFormingPolicy() const = 0;

    /**
     * @brief Get the retry attempt count about pcscfs being discovered in combined network
     *
     * @return IMS_SINT32 Return the retry attempt count
     */
    virtual IMS_SINT32 GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached() const = 0;

    /**
     * @brief Get the retry attempt count about pcscfs being discovered in only attached network
     *
     * @return IMS_SINT32 Retrurn the retry attempt count
     */
    virtual IMS_SINT32 GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached() const = 0;

    /**
     * @brief Get the registration retry intervals for using when registration is failed
     *        as general codes.
     *
     *        These values represent wait times for next registration for retry.
     *        It defines in second.
     *
     * @return ImsVector<IMS_SINT32>& Return registration retry intervals.
     */
    virtual ImsVector<IMS_SINT32>& GetRegRetryIntervals() = 0;

    /**
     * @brief Get the registration retry random intervals following the registration retry intervals
     *
     *        These values represent plus random value upper bound of the registration retry
     *        intervals by GetRegRetryIntervals()
     *        So the size of the return value has to be the same as the return value of
     *        GetRegRetryIntervals()
     *        It defines in second.
     *
     * @return ImsVector<IMS_SINT32>& Return random value for registration retry intervals
     */
    virtual ImsVector<IMS_SINT32>& GetRegRandomRetryIntervals() = 0;

    /**
     * @brief Get the ipsec authentication algorithms which will be used.
     *
     *        These algorithms consist of the IpSecType class
     *
     * @return ImsVector<IMS_SINT32>& Return what authentication algorithms will be used
     */
    virtual ImsVector<IMS_SINT32>& GetIpsecAuthenticationAlgorithms() = 0;

    /**
     * @brief Get the ipsec encryption algorithms which will be used.
     *
     *        These algorithms consist of the IpSecType class
     *
     * @return ImsVector<IMS_SINT32>& Return what encryption algorithms will be used
     */
    virtual ImsVector<IMS_SINT32>& GetIpsecEncryptionAlgorithms() = 0;

    /**
     * @brief Get what NOTIFY event that is condition to perform initial registration.
     *
     *        This function is called when it receives a NOTIFY message with a terminated state.
     *        And decide whether to perform initial registration depending on the event.
     *        These events consist of bit masking.
     *        It defines as like NOTIFY_TERMINATED_EXPIRED = 0x01
     *        This function relates to GetNotifyEventForInitialRegWithWaitTime() if there are values
     *        in GetNotifyEventForInitialRegWithWaitTime(), use the values.
     *
     * @return IMS_UINT32 Return value whether to perform initial registration
     */
    virtual IMS_UINT32 GetNotifyEventForInitialRegistration() const = 0;
    /**
     * @brief Get the set wait time to perform initial registration by NOTIFY message.
     *
     *        If there is 0, perform initial registration immediately.
     *        This function relates to GetNotifyEventForInitialRegWithWaitTime() and when it's an
     *        event taken from GetNotifyEventForInitialRegWithWaitTime() this return value is used
     *
     * @return IMS_SINT32 Return set wait time value
     */
    virtual IMS_SINT32 GetNotifyWaitTime() const = 0;
    /**
     * @brief Get what NOTIFY event that is condition to perform initial registration
     *        after set wait time..
     *
     *        This function is called when it receives a NOTIFY message with a terminated state.
     *        And decide whether to perform initial registration after the set wait time depending
     *        on the event. These events consist of bit masking.
     *        It defines as like NOTIFY_TERMINATED_EXPIRED = 0x01
     *        This function relates to GetNotifyWaitTime() and use wait time taken from
     *        GetNotifyWaitTime() at these return value
     *
     * @return IMS_UINT32 Return value whether to perform initial registration with wait time
     */
    virtual IMS_UINT32 GetNotifyEventForInitialRegWithWaitTime() const = 0;

    /**
     * @brief Get error response information against SUBSCRIBE msg that is condition to perform
     *        initial registration.
     *
     *        This function relates to GetRetryCountSubErrorRegRequired()
     *
     * @return ImsVector<IMS_SINT32>& return array list for error response information
     */
    virtual ImsVector<IMS_SINT32>& GetSubErrorRegRequired() = 0;

    /**
     * @brief Get the number of error responses against SUBSCRIBE message that is condition to
     *        perform initial registration.
     *
     *        This function relates to GetSubErrorRegRequired()
     *
     * @return IMS_SINT32 Return retry count for initial registration
     */
    virtual IMS_SINT32 GetRetryCountSubErrorRegRequired() const = 0;

    /**
     * @brief Get error response information against SUBSCRIBE msg that is condition to perform
     *        initial registration with next pcscf.
     *
     * @return ImsVector<IMS_SINT32>& return array list for error response information
     */
    virtual ImsVector<IMS_SINT32>& GetSubErrorRegRequiredWithNextPcscf() = 0;

    /**
     * @brief Get error response information against SUBSCRIBE msg that is condition to terminate
     *        its subscription.
     *
     *        This function relates to GetRetryCountSubErrorSubTerminated()
     *
     * @return ImsVector<IMS_SINT32>& return array list for error response information
     */
    virtual ImsVector<IMS_SINT32>& GetSubErrorSubTerminated() = 0;

    /**
     * @brief Get the number of error responses against SUBSCRIBE message that is condition to
     *        terminate its subscription.
     *
     *        This function relates to GetSubErrorSubTerminated()
     *
     * @return IMS_SINT32 Return retry count for terminating subscription
     */
    virtual IMS_SINT32 GetRetryCountSubErrorSubTerminated() const = 0;

    /**
     * @brief Get error response information against SUBSCRIBE msg that is condition to stop
     *        resending SUBSCRIBE msg.
     *
     *        When there are error responses, it stops retrying the current subscription and keeps
     *        its subscription until its expiration time
     *
     * @return ImsVector<IMS_SINT32>& return array list for error response information
     */
    virtual ImsVector<IMS_SINT32>& GetSubErrorStoppingResub() = 0;

    /**
     * @brief Get error response information against SUBSCRIBE msg in Wifi that is condition to
     *        perform initial registration.
     *
     * @return ImsVector<IMS_SINT32>& return array list for error response information
     */
    virtual ImsVector<IMS_SINT32>& GetVowifiSubErrorRegRequired() = 0;

    /**
     * @brief Get a priority of ISIM and USIM provisioning to obtain IMS Identity.
     *
     * @return vector list
     * @see {0} CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM
            {1} CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM
            {2} CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI
            {3} CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_CONF
     */
    virtual ImsVector<IMS_SINT32>& GetImsIdentityPriority() = 0;

    /**
     * @brief Get a P-CSCF address discovery methods and its preference order.
     *        For example, {0(PCO), 1(CONF)} means it supports two P-CSCF discovery methods.
     *        It uses P-CSCF address in PCO field if present.
     *        otherwise, it uses the pre-configured P-CSCF address.
     *
     * @return vector list
     * @see CarrierConfig::Ims::PCSCF_DISCOVERY_METHOD_PCO
     */
    virtual ImsVector<IMS_SINT32>& GetPcscfDiscoveryMethod() = 0;

    /**
     * @brief Indicate the list of RATs where registration is updated with RAT change.
     *        Possible values are,
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN
     *        ...
     *
     * @return vector rat list
     */
    virtual ImsVector<IMS_SINT32>& GetUpdateRegistrationWithRatChange() = 0;

    /**
     * @brief List of different RAT technologies on which IMS is supported
     *        Possible values are,
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN
     *
     * @return vector rat list
     */
    virtual ImsVector<IMS_SINT32>& GetSupportedRats() = 0;

    /**
     * @brief List of different RAT technologies on which IMS is supported in roaming network
     *        Possible values are,
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN
     *
     * @return vector rat list
     */
    virtual ImsVector<IMS_SINT32>& GetSupportedRoamingRats() = 0;

    /**
     * @brief List of different RAT technologies on which SMS over IMS is supported.
     *        Possible values are,
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN
     *
     * @return vector rat list
     */
    virtual ImsVector<IMS_SINT32>& GetSmsOverImsSupportedRats() = 0;

    /**
     * @brief Indicate the error codes for registration
     *        - ERROR_POLICY_PCSCF
     *        Indicate the error codes that result in initial registration
     *        with other PCSCFs available.
     *        - ERROR_POLICY_SUBSCRIBER_FAILED
     *        Indicate the list of error codes that result in initial registration
     *        with another PCSCF after wait-time.
     *        If initial registration fails with all PCSCFs and another IMPU is available,
     *        registration is retried with changed IMPU.
     *        After it fails with both IMPUs based on MSISDN and IMSI,
     *        registration is handled as critcal error with PLMN block
     *        (wait time - KEY_WAIT_TIME_SEC_INT)
     *        - ERROR_POLICY_PDN_REACTIVATED
     *        Indicate the error causes that result in new PCSCF discovery
     *        when the calculated retry number reaches.
     *        - Possible wildcard values except for error codes are,
     *        CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F, etc
     *
     * @return vector error code list
     */
    virtual ImsVector<IMS_SINT32>& GetExtraRegErrCode() = 0;

    /**
     * @brief Indicate the error codes for reregistration
     *        see@GetExtraRegErrCode
     *
     * @return vector error code list
     */
    virtual ImsVector<IMS_SINT32>& GetExtraReregErrCode() = 0;

    /**
     * @brief Indicate the list of wait-time seconds when registration is retried.
     *
     * @return vector wait time list
     */
    virtual ImsVector<IMS_SINT32>& GetExtraRegErrWaitTime() = 0;

    /**
     * @brief Indicate the error codes to attempt the initial registration with same PCSCF
     *
     * @return vector error code list
     */
    virtual ImsVector<IMS_SINT32>& GetReregRetryErrCodeForInitRegWithSamePcscf() = 0;

    /**
     * @brief Indicate the list of error responses. It shall not attempt any more IMS registrations
     *        until power cycled, switched in and out of airplane mode.
     *
     * @return vector permanent error code list
     */
    virtual ImsVector<IMS_SINT32>& GetRegPermanentErrCode() = 0;

    /**
     * @brief Indicate the number of error code considered the final result.
     *
     *        This function relates to GetRegPermanentErrCode()
     *        The UE check if how many registration will be sent through this function  when the UE
     *        receives error message that is in the GetRegPermanentErrCode()
     *
     *        This value is indicated by two numbers if the value is different in the home and
     *        roaming network. This value is indicated by a single number if the value is the same
     *        in the home and roaming network.
     *
     * @return vector max count list
     */
    virtual ImsVector<IMS_SINT32>& GetRegPermanentErrMaxCount() = 0;

    /**
     * @brief Inidicate the list of error codes to attempt initial registration without ipsec.
     *
     * @return vector error code list
     */
    virtual ImsVector<IMS_SINT32>& GetRegErrCodeWithoutIpsec() = 0;

    /**
     * @brief Indicate the list of the error response with time value containing Retry-After header
     *        for registration retry.
     *
     *        This function relates to IsRegErrCodeWithRetryAfterTimeOnlyDefined()
     *
     * @return vector error code list
     */
    virtual ImsVector<IMS_SINT32>& GetRegErrCodeWithRetryAfterTime() = 0;

    /**
     * @brief Indicate the list of the error response with time value containing Retry-After header
     *        for reregistration retry.
     *
     *        This function relates to IsRegErrCodeWithRetryAfterTimeOnlyDefined()
     *
     * @return vector error code list
     */
    virtual ImsVector<IMS_SINT32>& GetReregErrCodeWithRetryAfterTime() = 0;

    /**
     * @brief Indicate the list of the time seconds waiting after the emergency registration is
     *        failed and pcscf is changed
     * @return vector retry wait time
     */
    virtual ImsVector<IMS_SINT32>& GetEmergencyPcscfRetryWaitTime() = 0;

    /**
     * @brief Indicate the error codes of the registration followed by PCSCF discovery
     *        when PCSCF is unavailable.
     * @return vector error code
     */
    virtual ImsVector<IMS_SINT32>& GetRegErrCodeForPcscfDiscovery() = 0;

    /**
     * @brief Indicate the list of error codes that result in terminating the IMS call
     *        when reregistraiton fails.
     *
     * @return vector error code
     */
    virtual ImsVector<IMS_SINT32>& GetReregErrCodeForCallEnd() = 0;

    /**
     * @brief Indicate the error codes of the reregistration followed by intital registration
     *        with available PCSCF. If no available PCSCF, IMS PDN is re-activated.
     * @return vector error code
     */
    virtual ImsVector<IMS_SINT32>& GetReregErrCodeForInitRegWithAvailablePcscf() = 0;

    /**
     * @brief Indicate the error codes of the reregistration followed by IMS PDN reactivation
     *
     * @return vector error code
     */
    virtual ImsVector<IMS_SINT32>& GetReregErrCodeForImsPdnReactivation() = 0;

    enum
    {
        NOTIFY_TERMINATED_EXPIRED = 0x01,
        NOTIFY_TERMINATED_DEACTIVATED = 0x02,
        NOTIFY_TERMINATED_PROBATION = 0x04,
        NOTIFY_TERMINATED_UNREGISTERED = 0x08,
        NOTIFY_TERMINATED_REJECTED = 0x10
    };

private:
    friend class AosBuildDirector;
    virtual void Init(IN IMS_SINT32 nSlotId = IMS_SLOT_0) = 0;
};
#endif  // INTERFACE_AOS_NCONFIGURATION_H_
