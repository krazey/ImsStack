#ifndef _INTERFACE_AOS_NCONFIGURATION_H_
#define _INTERFACE_AOS_NCONFIGURATION_H_

#include "IMSTypeDef.h"

#define GET_N_CONFIG(SLOT) (AosProvider::GetInstance()->GetNConfiguration(SLOT))

/**
 * @brief This class provids an interface to access the configuration information related to Aos.
 *
 */
class IAosNConfiguration
{
public:
    /**
     * @brief Get the slot id to be retrieved.
     *
     * @return IMS_SINT32 Return slot id to be retrieved
     */
    virtual IMS_SINT32 GetSlotId() const = 0;

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
     * @brief Check if Mobile data setting is required for Video calling enable.
     *
     * @return IMS_TRUE if not required, IMS_FALSE if required.
     */
    virtual IMS_BOOL IsDataEnableChangeIgnoredForVideoCalls() const = 0;

    /**
     * @brief Check if WFC is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     */
    virtual IMS_BOOL IsWfcImsAvailable() const = 0;

    /**
     * @brief Check if WFC roaming is enabled.
     *
     * @return IMS_TRUE if enabled, IMS_FALSE if not enabled.
     */
    virtual IMS_BOOL IsWfcRoamingEnabled() const = 0;

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
     * @brief Check if emergency registration is required in roaming
     *
     * @return IMS_TRUE if required, IMS_FALSE if not required.
     * @note If 'true' is returned,
     *       emergency registration should be performed in the roaming network
     *       regardless of whether emergency registration is used in the home network.
     */
    virtual IMS_BOOL IsRequiredEmergencyRegistrationInRoaming() const = 0;

    /**
     * @brief Check if VoLTE service block by setting is required.
     *
     * @return IMS_TRUE if required, IMS_FALSE if not required.
     */
    virtual IMS_BOOL IsRequiredVolteBlockBySetting() const = 0;

    /**
     * @brief Check if VoLTE service block by Airplane mode is required.
     *
     * @return IMS_TRUE if required, IMS_FALSE if not required.
     */
    virtual IMS_BOOL IsRequiredVolteBlockByAirplaneMode() const = 0;

    /**
     * @brief Check if WFC service block by Airplane mode is required.
     *
     * @return IMS_TRUE if required, IMS_FALSE if not required.
     */
    virtual IMS_BOOL IsRequiredWfcBlockByAirplaneMode() const = 0;

    /**
     * @brief Check if conuntry code availability check for wfc is required.
     *
     * @return IMS_TRUE if required, IMS_FALSE if not required.
     */
    virtual IMS_BOOL UseWfcCountryCodeAvailabilityCheck() const = 0;

    /**
     * @brief Returns whether subscription also use the registration retry intervals.
     *
     * @return IMS_BOOL Return whether to be applied or not
     */
    virtual IMS_BOOL IsRegistrationRetryIntervalsUsedForSubscription() const = 0;

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
    virtual IMS_BOOL IsSecurityServerPortInRegContactOfInitialRegistrationUsed() const = 0;

    /**
     * @brief Returns whether the security server port is used in initial registration.
     *
     * @return IMS_BOOL Return wherther to be applied or not
     */
    virtual IMS_BOOL IsSecurityServerPortInInitialRegistrationUsed() const = 0;

    /**
     * @brief Returns whether the old security association is removed on establishing
     *        the security association
     *
     * @return IMS_BOOL Return wherther to be applied or not
     */
    virtual IMS_BOOL IsOldSaOnEstablishingSaRemoved() const = 0;

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
     * @brief Flag specifying if verstat is supported for registration or not
     *
     * @return IMS_BOOL Return wherther to be supported or not
     */
    virtual IMS_BOOL IsVerstatForRegistrationSupported() const = 0;

    /**
     * @brief Flag indicating whether the authorized IMPU from P-Associated-URI header in 200 OK
     *        for IMS registration is used in emergency call.
     *
     * @return IMS_BOOL Return wherther to be supported or not
     */
    virtual IMS_BOOL IsEmergencyCallBasedOnPauOfNormalRegistrationSupported() const = 0;

    /**
     * @brief Flag specifying whether the re-registration is held when IPCAN is changed
     *        during IMS calls and performed immediately after they are released.
     *
     * @return IMS_BOOL Return wherther to be held or not
     */
    virtual IMS_BOOL IsRegistrationWhenIpcanChangedWithImsActiveCallHeld() const = 0;

    /**
     * @brief Flag specifying whether IMS de-registers if the network is changed to 3G
     *        while IMS registered.
     *
     * @return IMS_BOOL Return wherther IMS de-registers or not.
     */
    virtual IMS_BOOL IsDeregisterOn3gNetworks() const = 0;

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
     * @param nGeolocationPidfType The Geolocation-PIDF type to be evaluated.\n
     *                         #CarrierConfig#Ims#GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI\n
     *                         #CarrierConfig#Ims#GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI\n
     *                         #CarrierConfig#Ims#GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR\n
     *                         #CarrierConfig#Ims#GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR
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
     * @brief Get the preferred IP address type
     *
     *        If both IPv4 and IPv6 addresses are assigned by the network to the UE,
     *        the UE use preferred IP address type of each operator's requirement.
     *
     * @return IMS_SINT32
     * @see CarrierConfig::Ims::IP_VERSION_4
     */
    virtual IMS_SINT32 GetPreferredIpType() const = 0;

    /**
     * @brief Get the preferred IP address type for emergency
     *
     *        If both IPv4 and IPv6 addresses are assigned by the network to the UE,
     *        the UE use preferred IP address type of each operator's requirement.
     *
     * @return IMS_SINT32
     * @see CarrierConfig::Ims::IP_VERSION_4
     */
    virtual IMS_SINT32 GetEmergencyPreferredIpType() const = 0;

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
     *        the emergency call attempt as failed.
     *
     * @return IMS_SINT32 Return the milli-second time
     */
    virtual IMS_SINT32 GetEmergencyRegistrationTimerMillis() const = 0;

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
    virtual IMS_SINT32 GetRegistrationRetryDefaultPolicy() const = 0;

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
     *        CarrierConfig::Ims::AWT_POLICY_RFC_RULE
     *            Follow RFC 5626 section 4.5.
     *        CarrierConfig::Ims::AWT_POLICY_FAILURE_TO_EVERY_PCSCF
     *            Indicate whether it shall calculate a wait time based on RFC 5626 4.5
     *            if the registration to every known PCSCF is attempted and handled
     *            as temporary failure without Retry-After header.
     *        CarrierConfig::Ims::AWT_POLICY_FAILURE_TO_EACH_PCSCF
     *            Indicate whether it shall calculate a wait time based on RFC 5626 4.5
     *            if the registration to each known PCSCF is attempted and handled
     *            as temporary failure without Retry-After header.
     *        CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL
     *            @see KEY_REGISTRATION_RETRY_INTERVAL_BUNDLE
     *
     * @return IMS_SINT32 Return the actual wait time policy
     */
    virtual IMS_SINT32 GetRegistrationActualWaitTimePolicy() const = 0;

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
    virtual IMS_SINT32 GetRegistrationRetrySip305CodePolicy() const = 0;

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
    virtual IMS_SINT32 GetReregistrationRetrySip305CodePolicy() const = 0;

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
    virtual IMS_SINT32 GetRegistrationRetrySip503CodePolicy() const = 0;

    /**
     * @brief Indicate the specific error type for registration
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
     * @return IMS_SINT32 Return the specific error type
     */
    virtual IMS_SINT32 GetSpecificRegistrationErrorFinalType() const = 0;

    /**
     * @brief Indicate the specific error policy for registration
     *
     *        Possible values are,
     *        CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED
     *        CarrierConfig::Assets::ERROR_POLICY_PCSCF_FAILED
     *        CarrierConfig::Assets::ERROR_POLICY_SUBSCRIBER_FAILED
     *        CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED
     *
     * @return IMS_SINT32 Return the specific error policy
     */
    virtual IMS_SINT32 GetSpecificRegistrationErrorPolicy() const = 0;

    /**
     * @brief Get the registration retry intervals for using when registration is failed
     *        as general codes.
     *
     *        These values represent wait times for next registration for retry.
     *        It defines in second.
     *
     * @return IMSVector<IMS_SINT32>& Return registration retry intervals.
     */
    virtual IMSVector<IMS_SINT32>& GetRegistrationRetryIntervals() = 0;

    /**
     * @brief Get the registration retry random intervals following the registration retry intervals
     *
     *        These values represent plus random value upper bound of the registration retry
     *        intervals by GetRegistrationRetryIntervals()
     *        So the size of the return value has to be the same as the return value of
     *        GetRegistrationRetryIntervals()
     *        It defines in second.
     *
     * @return IMSVector<IMS_SINT32>& Return random value for registration retry intervals
     */
    virtual IMSVector<IMS_SINT32>& GetRegistrationRandomRetryIntervals() = 0;

    /**
     * @brief Get the ipsec authentication algorithms which will be used.
     *
     *        These algorithms consist of the IPSecType class
     *
     * @return IMSVector<IMS_SINT32>& Return what authentication algorithms will be used
     */
    virtual IMSVector<IMS_SINT32>& GetIpsecAuthenticationAlgorithms() = 0;

    /**
     * @brief Get the ipsec encryption algorithms which will be used.
     *
     *        These algorithms consist of the IPSecType class
     *
     * @return IMSVector<IMS_SINT32>& Return what encryption algorithms will be used
     */
    virtual IMSVector<IMS_SINT32>& GetIpsecEncryptionAlgorithms() = 0;

    /**
     * @brief Get what NOTIFY event that is condition to perform initial registration.
     *
     *        This function is called when it receives a NOTIFY message with a terminated state.
     *        And decide whether to perform initial registration depending on the event.
     *        These events consist of bit masking.
     *        It defines as like NOTIFY_TERMINATED_EXPIRED = 0x01
     *
     * @return IMS_UINT32 Return value whether to perform initial registration
     */
    virtual IMS_UINT32 GetNotifyEventForInitialRegistration() const = 0;
    /**
     * @brief Get the set wait time to perform initial registration by NOTIFY message.
     *
     *        If there is 0, perform initial registration immediately.
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
     *
     * @return IMS_UINT32 Return value whether to perform initial registration with wait time
     */
    virtual IMS_UINT32 GetNotifyEventForInitialRegWithWaitTime() const = 0;

    /**
     * @brief Get error response information against SUBSCRIBE msg that is condition to perform
     *        initial registration
     *
     * @return IMSVector<IMS_SINT32>& return array list for error response information
     */
    virtual IMSVector<IMS_SINT32>& GetSubErrorRegRequired() = 0;

    /**
     * @brief Get the number of error responses against SUBSCRIBE message that is condition to
     *        perform initial registration.
     *
     * @return IMS_SINT32 Return retry count for initial registration
     */
    virtual IMS_SINT32 GetRetryCountSubErrorRegRequired() const = 0;

    /**
     * @brief Get error response information against SUBSCRIBE msg that is condition to perform
     *        initial registration with next pcscf.
     *
     * @return IMSVector<IMS_SINT32>& return array list for error response information
     */
    virtual IMSVector<IMS_SINT32>& GetSubErrorRegRequiredWithNextPcscf() = 0;

    /**
     * @brief Get error response information against SUBSCRIBE msg that is condition to terminate
     *        its subscription
     *
     * @return IMSVector<IMS_SINT32>& return array list for error response information
     */
    virtual IMSVector<IMS_SINT32>& GetSubErrorSubTerminated() = 0;

    /**
     * @brief Get the number of error responses against SUBSCRIBE message that is condition to
     *        terminate its subscription.
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
     * @return IMSVector<IMS_SINT32>& return array list for error response information
     */
    virtual IMSVector<IMS_SINT32>& GetSubErrorStoppingResub() = 0;

    /**
     * @brief Get error response information against SUBSCRIBE msg in Wifi that is condition to
     *        perform initial registration.
     *
     * @return IMSVector<IMS_SINT32>& return array list for error response information
     */
    virtual IMSVector<IMS_SINT32>& GetVowifiSubErrorRegRequired() = 0;

    /**
     * @brief Get a bit for reasons to clear service block for permanent PDN failures.
     *
     * @return IMS_UINT32 return bit value to clear service blocks.
     *         {0x01} : IMSVector{0} SIM state is changed
     *         {0x02} : IMSVector{1} Airplane mode is changed
     *         {0x04} : IMSVector{2} Network operator is changed
     *         {0x08} : IMSVector{3} Radio access technology is changed
     *         {0x10} : IMSVector{4} WiFi connection state is changed
     *         {0x20} : IMSVector{5} VoLTE settings is changed
     *         {0x40} : IMSVector{6} VoWiFi settings is changed
     */
    virtual IMS_UINT32 GetClearReasonForPermanentPdnFailure() const = 0;

    /**
     * @brief Get a priority of ISIM and USIM provisioning to obtain IMS Identity.
     *
     * @return vector list
     * @see {0} CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM
            {1} CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM
            {2} CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI
            {3} CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_CONF
     */
    virtual IMSVector<IMS_SINT32>& GetImsIdentityPriority() = 0;

    /**
     * @brief Get a P-CSCF address discovery methods and its preference order.
     *        For example, {0(PCO), 1(CONF)} means it supports two P-CSCF discovery methods.
     *        It uses P-CSCF address in PCO field if present.
     *        otherwise, it uses the pre-configured P-CSCF address.
     *
     * @return vector list
     * @see CarrierConfig::Ims::PCSCF_DISCOVERY_METHOD_PCO
     */
    virtual IMSVector<IMS_SINT32>& GetPcscfDiscoveryMethod() = 0;

    /**
     * @brief Get a P-CSCF address discovery methods in roaming area and its preference order.
     *        For example, {0(PCO), 1(CONF)} means it supports two P-CSCF discovery methods.
     *        It uses P-CSCF address in PCO field if present.
     *        otherwise, it uses the pre-configured P-CSCF address.
     *
     * @return vector list
     * @see CarrierConfig::Ims::PCSCF_DISCOVERY_METHOD_PCO
     */
    virtual IMSVector<IMS_SINT32>& GetRoamingPcscfDiscoveryMethod() = 0;

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
    virtual IMSVector<IMS_SINT32>& GetUpdateRegistrationWithRatChange() = 0;

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
    virtual IMSVector<IMS_SINT32>& GetSupportedRats() = 0;

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
    virtual IMSVector<IMS_SINT32>& GetSupportedRoamingRats() = 0;

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
    virtual IMSVector<IMS_SINT32>& GetSmsOverImsSupportedRats() = 0;

    /**
     * @brief Indicate the retry attempt number in normal and only attached network formula
     *        is “n (this integer) * <# of PCSCFs being discovered>
     *
     * @return vector list
     */
    virtual IMSVector<IMS_SINT32>& GetSpecificRegErrNumMultipliedByPcscfNum() = 0;

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
     * @return vector list
     */
    virtual IMSVector<IMS_SINT32>& GetSpecificRegistrationErrorCode() = 0;

    /**
     * @brief Indicate the error codes to attempt the initial registration with same PCSCF
     *
     * @return vector list
     */
    virtual IMSVector<IMS_SINT32>& GetReregRetryErrCodeWithInitialRegWithSamePcscf() = 0;

    enum
    {
        NOTIFY_TERMINATED_EXPIRED = 0x01,
        NOTIFY_TERMINATED_DEACTIVATED = 0x02,
        NOTIFY_TERMINATED_PROBATION = 0x04,
        NOTIFY_TERMINATED_UNREGITERED = 0x08,
        NOTIFY_TERMINATED_REJECTED = 0x10
    };

    enum class ClearReason
    {
        SIM_STATE     = 0x01,
        AIRPLANE      = 0x02,
        PLMN_CHANGED  = 0x04,
        RAT_CHANGED   = 0x08,
        WIFI_CHANGED  = 0x10,
        VOLTE_SETTING = 0x20,
        WFC_SETTING   = 0x40
    };

private:
    friend class AosBuildDirector;
    virtual void Init(IN IMS_SINT32 nSlotId = IMS_SLOT_0) = 0;
};
#endif // _INTERFACE_AOS_NCONFIGURATION_H_
