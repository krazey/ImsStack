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
#ifndef CARRIER_CONFIG_H_
#define CARRIER_CONFIG_H_

#include "ImsTypeDef.h"

class CarrierConfig
{
public:
    CarrierConfig() = delete;
    ~CarrierConfig() = delete;

public:
    // Public carrier-config - starts
    static const IMS_CHAR KEY_CARRIER_VOLTE_AVAILABLE_BOOL[];
    static const IMS_CHAR KEY_CARRIER_VT_AVAILABLE_BOOL[];
    static const IMS_CHAR KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL[];
    static const IMS_CHAR KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL[];
    static const IMS_CHAR KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL[];

    static const IMS_CHAR KEY_CARRIER_USSD_METHOD_INT[];
    enum
    {
        USSD_OVER_CS_PREFERRED = 0,
        USSD_OVER_IMS_PREFERRED,
        USSD_OVER_CS_ONLY,
        USSD_OVER_IMS_ONLY
    };

    static const IMS_CHAR KEY_RTT_SUPPORTED_BOOL[];
    static const IMS_CHAR KEY_RTT_SUPPORTED_WHILE_ROAMING_BOOL[];
    static const IMS_CHAR KEY_CARRIER_VOLTE_TTY_SUPPORTED_BOOL[];
    static const IMS_CHAR KEY_USE_RCS_SIP_OPTIONS_BOOL[];
    static const IMS_CHAR KEY_SUPPORT_PAUSE_IMS_VIDEO_CALLS_BOOL[];
    static const IMS_CHAR KEY_SUPPORT_ADHOC_CONFERENCE_CALLS_BOOL[];
    static const IMS_CHAR KEY_SUPPORT_ADD_CONFERENCE_PARTICIPANTS_BOOL[];
    static const IMS_CHAR KEY_IS_IMS_CONFERENCE_SIZE_ENFORCED_BOOL[];
    static const IMS_CHAR KEY_IMS_CONFERENCE_SIZE_LIMIT_INT[];
    static const IMS_CHAR KEY_SUPPORTS_DEVICE_TO_DEVICE_COMMUNICATION_USING_RTP_BOOL[];
    static const IMS_CHAR KEY_SUPPORTS_DEVICE_TO_DEVICE_COMMUNICATION_USING_DTMF_BOOL[];
    static const IMS_CHAR KEY_SUPPORTS_SDP_NEGOTIATION_OF_D2D_RTP_HEADER_EXTENSIONS_BOOL[];

    static const IMS_CHAR KEY_CARRIER_NR_AVAILABILITIES_INT_ARRAY[];
    enum
    {
        CARRIER_NR_AVAILABILITY_NSA = 1,
        CARRIER_NR_AVAILABILITY_SA = 2
    };

    static const IMS_CHAR KEY_RTT_AUTO_UPGRADE_BOOL[];
    static const IMS_CHAR KEY_RTT_SUPPORTED_FOR_VT_BOOL[];
    static const IMS_CHAR KEY_RTT_UPGRADE_SUPPORTED_BOOL[];
    static const IMS_CHAR KEY_RTT_DOWNGRADE_SUPPORTED_BOOL[];
    static const IMS_CHAR KEY_SUPPORT_IMS_CONFERENCE_EVENT_PACKAGE_BOOL[];
    static const IMS_CHAR KEY_SUPPORT_IMS_CONFERENCE_EVENT_PACKAGE_ON_PEER_BOOL[];
    static const IMS_CHAR KEY_IWLAN_HANDOVER_POLICY_STRING_ARRAY[];
    // Public carrier-config - ends

    class Ims
    {
    public:
        // Public carrier-config - starts
        static const IMS_CHAR KEY_IMS_SINGLE_REGISTRATION_REQUIRED_BOOL[];
        static const IMS_CHAR KEY_ENABLE_PRESENCE_PUBLISH_BOOL[];
        static const IMS_CHAR KEY_ENABLE_PRESENCE_CAPABILITY_EXCHANGE_BOOL[];
        static const IMS_CHAR KEY_RCS_BULK_CAPABILITY_EXCHANGE_BOOL[];
        static const IMS_CHAR KEY_ENABLE_PRESENCE_GROUP_SUBSCRIBE_BOOL[];
        static const IMS_CHAR KEY_USE_SIP_URI_FOR_PRESENCE_SUBSCRIBE_BOOL[];
        static const IMS_CHAR KEY_NON_RCS_CAPABILITIES_CACHE_EXPIRATION_SEC_INT[];
        static const IMS_CHAR KEY_RCS_FEATURE_TAG_ALLOWED_STRING_ARRAY[];
        static const IMS_CHAR KEY_SIP_TIMER_T1_MILLIS_INT[];
        static const IMS_CHAR KEY_SIP_TIMER_T2_MILLIS_INT[];
        static const IMS_CHAR KEY_SIP_TIMER_T4_MILLIS_INT[];
        static const IMS_CHAR KEY_SIP_TIMER_B_MILLIS_INT[];
        static const IMS_CHAR KEY_SIP_TIMER_C_MILLIS_INT[];
        static const IMS_CHAR KEY_SIP_TIMER_D_MILLIS_INT[];
        static const IMS_CHAR KEY_SIP_TIMER_F_MILLIS_INT[];
        static const IMS_CHAR KEY_SIP_TIMER_H_MILLIS_INT[];
        static const IMS_CHAR KEY_SIP_TIMER_J_MILLIS_INT[];
        static const IMS_CHAR KEY_SIP_SERVER_PORT_NUMBER_INT[];
        static const IMS_CHAR KEY_PHONE_CONTEXT_DOMAIN_NAME_STRING[];

        static const IMS_CHAR KEY_REQUEST_URI_TYPE_INT[];
        enum
        {
            REQUEST_URI_FORMAT_TEL = 0,
            REQUEST_URI_FORMAT_SIP = 1
        };

        static const IMS_CHAR KEY_GRUU_ENABLED_BOOL[];
        static const IMS_CHAR KEY_KEEP_PDN_UP_IN_NO_VOPS_BOOL[];

        static const IMS_CHAR KEY_SIP_PREFERRED_TRANSPORT_INT[];
        enum
        {
            PREFERRED_TRANSPORT_UDP = 0,
            PREFERRED_TRANSPORT_TCP = 1,
            PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP = 2,
            PREFERRED_TRANSPORT_TLS = 3
        };

        static const IMS_CHAR KEY_IPV4_SIP_MTU_SIZE_CELLULAR_INT[];
        static const IMS_CHAR KEY_IPV6_SIP_MTU_SIZE_CELLULAR_INT[];
        static const IMS_CHAR KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY[];
        static const IMS_CHAR KEY_SIP_OVER_IPSEC_ENABLED_BOOL[];

        static const IMS_CHAR KEY_IPSEC_AUTHENTICATION_ALGORITHMS_INT_ARRAY[];
        enum
        {
            IPSEC_AUTHENTICATION_ALGORITHM_HMAC_MD5 = 0,
            IPSEC_AUTHENTICATION_ALGORITHM_HMAC_SHA1 = 1
        };

        static const IMS_CHAR KEY_IPSEC_ENCRYPTION_ALGORITHMS_INT_ARRAY[];
        enum
        {
            IPSEC_ENCRYPTION_ALGORITHM_NULL = 0,
            IPSEC_ENCRYPTION_ALGORITHM_DES_EDE3_CBC = 1,
            IPSEC_ENCRYPTION_ALGORITHM_AES_CBC = 2
        };

        static const IMS_CHAR KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT[];
        static const IMS_CHAR KEY_REGISTRATION_RETRY_BASE_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_REGISTRATION_RETRY_MAX_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_REGISTRATION_EVENT_PACKAGE_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT[];

        static const IMS_CHAR KEY_GEOLOCATION_PIDF_IN_SIP_REGISTER_SUPPORT_INT_ARRAY[];
        static const IMS_CHAR KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY[];
        enum
        {
            GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI = 1,
            GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI = 2,
            GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR = 3,
            GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR = 4,
            GEOLOCATION_PIDF_FOR_NORMAL_ROUTING_EMERGENCY_ON_WIFI = 5,
            GEOLOCATION_PIDF_FOR_NORMAL_ROUTING_EMERGENCY_ON_CELLULAR = 6
        };

        static const IMS_CHAR KEY_IMS_USER_AGENT_STRING[];

        static const IMS_CHAR KEY_SUPPORTED_RATS_INT_ARRAY[];
        enum
        {
            ACCESS_NETWORK_TYPE_GERAN = 1,
            ACCESS_NETWORK_TYPE_UTRAN = 2,
            ACCESS_NETWORK_TYPE_EUTRAN = 3,
            ACCESS_NETWORK_TYPE_IWLAN = 5,
            ACCESS_NETWORK_TYPE_NGRAN = 6
        };

        // Bundle {
        static const IMS_CHAR KEY_MMTEL_REQUIRES_PROVISIONING_BUNDLE[];
        static const IMS_CHAR KEY_CAPABILITY_TYPE_VOICE_INT_ARRAY[];
        static const IMS_CHAR KEY_CAPABILITY_TYPE_VIDEO_INT_ARRAY[];
        static const IMS_CHAR KEY_CAPABILITY_TYPE_UT_INT_ARRAY[];
        static const IMS_CHAR KEY_CAPABILITY_TYPE_SMS_INT_ARRAY[];
        static const IMS_CHAR KEY_CAPABILITY_TYPE_CALL_COMPOSER_INT_ARRAY[];
        // }
        // Bundle {
        static const IMS_CHAR KEY_RCS_REQUIRES_PROVISIONING_BUNDLE[];
        static const IMS_CHAR KEY_CAPABILITY_TYPE_OPTIONS_UCE_INT_ARRAY[];
        static const IMS_CHAR KEY_CAPABILITY_TYPE_PRESENCE_UCE_INT_ARRAY[];
        // }
        // Public carrier-config - ends

        // General
        static const IMS_CHAR KEY_SIP_COMPACT_FORM_ENABLED_BOOL[];
        static const IMS_CHAR KEY_ALLOW_SIP_UDP_FALLBACK_ON_TCP_CONNECTION_SETUP_FAILED_BOOL[];
        static const IMS_CHAR KEY_ALLOW_ALGORITHM_PARAM_IN_SIP_AUTHORIZATION_HEADER_BOOL[];
        static const IMS_CHAR KEY_USE_SIP_USER_AGENT_HEADER_IN_UA_STRING_BOOL[];
        static const IMS_CHAR KEY_ALLOW_SIP_P_ACCESS_NETWORK_INFO_HEADER_IN_INITIAL_REGISTER_BOOL[];
        static const IMS_CHAR KEY_REQUIRE_SIP_EXPIRES_HEADER_IN_REGISTER_BOOL[];
        static const IMS_CHAR KEY_SUPPORT_SIP_SESSION_ID_HEADER_BOOL[];

        static const IMS_CHAR KEY_REGISTRATION_CONTACT_USER_INFO_PART_INT[];
        enum
        {
            /// Default, use time-based UUID
            REGISTRATION_CONTACT_USER_INFO_PART_UUID = 0,
            /// Use user-info part from IMPU
            REGISTRATION_CONTACT_USER_INFO_PART_IMPU = 1,
            /// No user-info part
            REGISTRATION_CONTACT_USER_INFO_PART_EMPTY = 2,
        };

        static const IMS_CHAR KEY_SIP_DEVICE_ID_TYPE_INT[];

        static const IMS_CHAR KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY[];
        enum
        {
            PCSCF_DISCOVERY_METHOD_PCO = 0,
            PCSCF_DISCOVERY_METHOD_CONFIG = 1,
            PCSCF_DISCOVERY_METHOD_ISIM = 2
        };

        static const IMS_CHAR KEY_SDP_NEGOTIATION_REQUIRED_FOR_NON_RPR_BOOL[];
        static const IMS_CHAR KEY_REQUEST_URI_VALIDATION_REQUIRED_IN_MID_DIALOG_BOOL[];
        static const IMS_CHAR
                KEY_SESSION_TIMER_UPDATE_REQUIRED_IN_SESSION_UPDATE_BY_REINVITE_BOOL[];
        static const IMS_CHAR KEY_ALLOW_SESSION_TIMER_TURN_OFF_BOOL[];
        static const IMS_CHAR
                KEY_ALLOW_SIP_INSTANCE_PARAM_IN_CONTACT_FOR_NON_REGISTER_REQUEST_BOOL[];
        static const IMS_CHAR KEY_IGNORE_UDP_TRANSPORT_PARAMETER_FOR_OUTGOING_REQUEST_BOOL[];
        static const IMS_CHAR KEY_SIP_TIMER_100_TRYING_MILLIS_INT[];
        static const IMS_CHAR KEY_USE_RESET_WHEN_CLOSING_SIP_TCP_CONNECTION_BOOL[];
        static const IMS_CHAR KEY_USE_TUPLE_ELEMENT_IN_GEOLOCATION_PIDF_BOOL[];
        static const IMS_CHAR KEY_ALLOW_UNKNOWN_COUNTRY_ELEMENT_IN_GEOLOCATION_PIDF_BOOL[];
        static const IMS_CHAR KEY_ALLOW_NO_POSITION_IN_GEOLOCATION_PIDF_BOOL[];
        static const IMS_CHAR KEY_SET_SDP_DIRECTION_ATTRIBUTE_FOR_REMOVED_MEDIA_BOOL[];
        static const IMS_CHAR KEY_SUPPORT_SDP_PRECONDITION_BOOL[];
        static const IMS_CHAR KEY_HIDE_MAC_ADDRESS_IN_PANI_HEADER_INT[];
        static const IMS_CHAR KEY_SUPPORT_COUNTRY_PARAM_IN_PANI_HEADER_BOOL[];
        static const IMS_CHAR KEY_SUPPORT_LOCAL_SESSION_TIMER_BOOL[];

        static const IMS_CHAR KEY_SUPPORT_MULTIPLE_REGISTRATION_INT[];
        enum
        {
            MULTIPLE_REGISTRATION_NONE = 0,
            MULTIPLE_REGISTRATION_REG_ID_ONLY = 1,
            MULTIPLE_REGISTRATION_FULL = 2
        };

        static const IMS_CHAR KEY_RETRANSMISSION_ALLOWED_OF_GEOLOCATION_PIDF_STRING[];
        static const IMS_CHAR KEY_ALLOW_CELLULAR_NETWORK_INFO_HEADER_BOOL[];
        static const IMS_CHAR KEY_CELLULAR_NETWORK_INFO_CACHE_EXPIRATION_SEC_INT[];
        static const IMS_CHAR KEY_IPSEC_UE_CLIENT_PORT_RANGE_INT_ARRAY[];
        static const IMS_CHAR KEY_IPSEC_UE_SERVER_PORT_RANGE_INT_ARRAY[];
        static const IMS_CHAR KEY_SIP_TCP_CLIENT_PORT_RANGE_INT_ARRAY[];

        // Aos

        static const IMS_CHAR KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY[];
        enum
        {
            IMS_IDENTITY_PRIORITY_ISIM = 0,
            IMS_IDENTITY_PRIORITY_USIM = 1,
            IMS_IDENTITY_PRIORITY_ISIM_IMSI = 2,
            IMS_IDENTITY_PRIORITY_CONF = 3
        };

        static const IMS_CHAR KEY_ISIM_INDEX_FOR_IMPU_INT[];
        static const IMS_CHAR KEY_UPDATE_REGISTRATION_WITH_RAT_CHANGE_INT_ARRAY[];
        static const IMS_CHAR KEY_UNSUBSCRIBE_REGISTRATION_EVENT_PACKAGE_BOOL[];

        /**
         * Specifies whether SIP REGISTER message should be transmitted by TCP or not.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         *
         */
        static const IMS_CHAR KEY_USE_TCP_TRANSPORT_FOR_REGISTER_BOOL[];

        static const IMS_CHAR KEY_PREFERRED_IMS_DSCP_INT[];
        enum
        {
            PREFERRED_DSCP_NONE = 0,
            PREFERRED_DSCP_CELLULAR = 1,
            PREFERRED_DSCP_WIFI = 2,
            PREFERRED_DSCP_CELLULAR_WIFI = 3
        };

        static const IMS_CHAR KEY_REGISTRATION_PREFERRED_ACCESSTYPE_FEATURE_TAG_INT[];
        enum
        {
            PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED = 0,
            PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED = 1,
            PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED_WITHOUT_NUMERICAL_VALUE = 2
        };

        static const IMS_CHAR KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY[];

        /**
         * Specifies whether +g.gsma.callcomposer feature tag is included in contact header of
         * REGISTER messages when call composer is for B2C only.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_B2C_CALL_COMPOSER_FEATURE_TAG_IN_REG_CONTACT_BOOL[];

        /**
         * Specifies whether PCSCFs that UE fails to register will be blocked.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_BLOCK_PCSCF_ON_REG_FAILURE_BOOL[];

        /**
         * Specifies whether to block IMS registration on CS call.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_BLOCK_REG_ON_CS_CALL_BOOL[];

        /**
         * Specifies whether a call should be terminated due to expiration of registration.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_CALL_END_AND_PDN_REACTIVATION_BY_REG_TERMINATED_BOOL[];

        /**
         * Specifies whether or not to destroy unsecure TCP socket when the registration is
         * accomplished.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_DESTROY_UNSECURE_TCP_SOCKET_ON_ACCOMPLISHING_REG_BOOL[];

        /**
         * Specifies whether to disable N1 mode capability when failing to establish IMS PDU Session
         * for IMS.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_DISABLE_N1_MODE_ON_IMS_PDU_ESTABLISH_FAILURE_BOOL[];

        /**
         * Specifies whether the re-registration is held when IPCAN is changed during IMS calls
         * and performed immediately after they are released.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_HOLD_REG_WITH_IPCAN_CHANGED_DURING_IMS_CALL_BOOL[];

        /**
         * Specifies whether IMS de-registers if the network is changed to 3G while IMS
         * registered.
         *
         * If {@code true}: IMS de-registers if the network is changed to 3G
         * while IMS registered. If {@code false}: IMS re-registers according to the service
         * availability.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_IMS_DEREG_ON_3G_NETWORK_BOOL[];

        /**
         * Specifies if IMSI-based URI should be prioritized.
         *
         * IMS_TRUE if IMSI-based URI should be prioritized, IMS_FALSE otherwise.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_IMSI_BASED_URI_PRIORITIZED_BOOL[];

        /**
         * Specifies whether ipsec setting is initialized with new pcscf.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_INIT_IPSEC_SETTING_WITH_NEW_PCSCF_BOOL[];

        /**
         * Specifies whether to send initial subscription when the subscription is terminated.
         * (reg event package).
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_INIT_SUB_UPON_SUB_TERMINATED_BOOL[];

        /**
         * Specifies whether to keep using the existing pcscf to perform re-reg when handover
         * between cellular and wifi during the call.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_KEEP_EXISTING_PCSCF_ON_PCSCF_CHANGE_DURING_THE_CALL_BOOL[];

        /**
         * Specifies whether to keep the count of IMS registrations failures even when PDN is
         * reconnected.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_KEEP_REG_RETRY_CNT_UPON_PDN_RECONNECT_BOOL[];

        /**
         * Specifies whether Aos will keep the registration retry timer running even if all
         * enablers are detached.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_KEEP_REG_RETRY_TIMER_ON_ALL_ENABLERS_DETACHED_BOOL[];

        /**
         * Specifies whether UE will reconnect PDN when all pcscfs are unavailable.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_PDN_RECONNECT_ON_ALL_PCSCFS_UNAVAILABLE_BOOL[];

        /**
         * Specifies whether the Contact URI validation is checked when the 200 OK response for
         * SIP REGISTER is received.
         *
         * If the Contact URI is not matched, the 200 OK response is disregarded.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_REG_CONTACT_VALIDATION_BOOL[];

        /**
         * Specifies whether IMS registration is retried based on IP version fallback from IPv6
         * to IPv4.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_REG_RETRY_WITH_IP_VER_FALLBACK_BOOL[];

        /**
         * Specifies whether it should be removed old security association information during
         * establishing security association.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_REMOVE_OLD_SA_ON_ESTABLISHING_SA_BOOL[];

        /**
         * Specifies if "+cdmaless" feature tag is required.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_REQUIRED_CDMALESS_FEATURE_TAG_BOOL[];

        /**
         * Specifies if the initial registration is tried on not right after IMS call is ended
         * while registration is held because re-registration is failed during active call.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_REQUIRED_INIT_REG_AFTER_IMS_CALL_END_ON_REG_HELD_BOOL[];

        /**
         * Specifies the initial registration is tried on not right after IMS emergency call is
         * ended while registration is held because re-registration is failed during active
         * emergency call.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_REQUIRED_INIT_REG_AFTER_IMS_ECALL_END_ON_REG_HELD_BOOL[];

        /**
         * Specifies whether IPSec enabled for SIP messages in roaming network only when
         * {@code KEY_SIP_OVER_IPSEC_ENABLED_BOOL} is true.
         *
         * Reference: 3GPP TS 33.203 and RFC 3329.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_SIP_OVER_IPSEC_ENABLED_IN_ROAMING_BOOL[];

        /**
         * Specifies if verstat is supported only when +g.3gpp.verstat feature tag is present in
         * the 200 OK response to REGISTER message.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_SUPPORT_VERSTAT_BASED_ON_NETWORK_FOR_REG_BOOL[];

        /**
         * Specifies whether It is to include the feature tag with "+g.3gpp.verstat" for calling
         * number verification status determination.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_SUPPORT_VERSTAT_FOR_REG_BOOL[];

        /**
         * Specifies whether to update ongoing reg retry timer when ims establishment timer is
         * expired and the data is maintained.
         *
         * When PLMN_BLOCK_WITH_TIMEOUT is sent due to ims establishment timer
         * expiry. If the value is true, the UE updates ongoing reg retry timer If the value is
         * false, the UE follows ongoing reg retry timer.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_UPDATE_ONGOING_REG_RETRY_TIMER_ON_IMS_EST_TIMER_EXPIRY_BOOL[];

        /**
         * Specifies this item adds a customized +g.gsma.rcs.telephony feature tag in contact
         * header of SIP REGISTER message with "cs,volte" value.
         *
         * The value is changed to "cs" at runtime according to the specific carrier's VoWiFi
         * requirement.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_USE_RCS_TELEPHONY_FEATURE_TAG_AS_AVAILABLE_VOICE_CALL_TYPE_BOOL[];

        /**
         * Specifies whether to allow use of contact information in reginfo XML without checking
         * for URI matching.
         *
         * If it is allowed, URI verification will be skipped during retrieving the
         * "contact" element from Reginfo XML in the SIP Notify message.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_USE_REGINFO_CONTACT_WITHOUT_URI_CHECK_BOOL[];

        /**
         * Specifies whether it should be use security server port in initial registration.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_USE_SECURITY_SERVER_PORT_IN_INIT_REG_BOOL[];

        /**
         * Specifies whether it should be set the protected port to the reg contact before
         * establishing a security association.
         *
         * In other words, whether it should be set the protected port to the reg
         * contact of 1st REGISTER msg.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_USE_SECURITY_SERVER_PORT_IN_REG_CONTACT_OF_INIT_REG_BOOL[];

        /**
         * Specifies retry count to consider as a failure when receiving 401 response to the
         * REGISTER message repeatedly and AKA result is successful.
         *
         * Possible Values:
         *   {@code 2}
         */
        static const IMS_CHAR KEY_AUTH_FAILURE_RETRY_MAX_CNT_INT[];

        /**
         * Specifies policy indicating which policy for user info is applied for non-register
         * message.
         *
         * It will be applied in all the outgoing SIP request and response except for
         * register request.
         *
         * Possible Values:
         *   {@code CONTACT_USER_INFO_POLICY_DEFAULT} (1)
         *   {@code CONTACT_USER_INFO_POLICY_NONE} (2)
         *   {@code CONTACT_USER_INFO_POLICY_AUTHORIZED_IMPU} (3)
         */
        static const IMS_CHAR KEY_CONTACT_USER_INFO_POLICY_FOR_NON_REG_MESSAGE_INT[];
        enum
        {
            CONTACT_USER_INFO_POLICY_DEFAULT = 1,
            CONTACT_USER_INFO_POLICY_NONE = 2,
            CONTACT_USER_INFO_POLICY_AUTHORIZED_IMPU = 3
        };

        /**
         * Specifies policy indicating which policy is applied for creating geolocation pidf.
         *
         * Possible Values:
         *   {@code GEOLOCATION_POLICY_WITHOUT_POSITION} (1)
         *   {@code GEOLOCATION_POLICY_WITH_POSITION} (2)
         *   {@code GEOLOCATION_POLICY_WITH_POSITION_AND_COUNTRY} (3)
         *   {@code GEOLOCATION_POLICY_WITHOUT_CIVIC} (4)
         */
        static const IMS_CHAR KEY_GEOLOCATION_PIDF_FORMING_POLICY_INT[];
        enum
        {
            GEOLOCATION_POLICY_WITHOUT_POSITION = 1,
            GEOLOCATION_POLICY_WITH_POSITION = 2,
            GEOLOCATION_POLICY_WITH_POSITION_AND_COUNTRY = 3,
            GEOLOCATION_POLICY_WITHOUT_CIVIC = 4
        };

        static const IMS_CHAR KEY_IMS_ESTABLISHMENT_TIME_FOR_LTE_SEC_INT[];

        /**
         * Specifies IMS establishment time for NR.
         *
         * IMS establishment time is supported when a new IMS registration is
         * initiated.
         *
         * Possible Values:
         *   {@code 300}
         */
        static const IMS_CHAR KEY_IMS_ESTABLISHMENT_TIME_FOR_NR_SEC_INT[];

        /**
         * Specifies the preferred IP version priority for connection.
         *
         * Possible Values:
         *   {@code IP_VERSION_4} (0)
         *   {@code IP_VERSION_6} (1)
         */
        static const IMS_CHAR KEY_IMS_PREFERRED_IPTYPE_INT[];
        enum
        {
            IP_VERSION_4 = 0,
            IP_VERSION_6 = 1
        };

        /**
         * Specifies the IMS signalling DSCP value.
         *
         * If KEY_PREFERRED_IMS_DSCP_INT is not PREFERRED_DSCP_NONE, it will be set according to
         * IP connectivity access network.
         * DSCP integer must be between 8 and 56. This key is considered invalid if the format is
         * violated. If the key is invalid or not configured, the DSCP will not apply.
         *
         * Possible Values:
         *   {@code IP_VERSION_4} (0)
         *   {@code IP_VERSION_6} (1)
         */
        static const IMS_CHAR KEY_IMS_SIGNALLING_DSCP_INT[];

        /**
         * Specifies the max allowed network MTU.
         *
         * If Network assigns MTU large than KEY_MAX_ALLOWED_NETWORK_MTU_INT, it will
         * be KEY_MAX_ALLOWED_NETWORK_MTU_INT to avoid UE use wrong transport type to send SIP
         * message.
         *
         * Possible Values:
         *   {@code 1500}
         */
        static const IMS_CHAR KEY_MAX_ALLOWED_NETWORK_MTU_INT[];

        /**
         * Specifies the delay time before reconnecting IMS PDN when WFC setup is failed for all
         * P-CSCFs while UE is on CS roaming network.
         *
         * Possible Values:
         *   {@code 120}
         */
        static const IMS_CHAR
                KEY_PDN_RECONNECT_DELAY_ON_WFC_SETUP_FAIL_ALL_PCSCFS_WITH_CS_ROAM_SEC_INT[];

        /**
         * Specifies policy indicating which policy is applied for registration retry interval.
         *
         * Possible Values:
         *   {@code AWT_POLICY_RFC_RULE} (0)
         *   {@code AWT_POLICY_FAILURE_TO_EVERY_PCSCF} (1)
         *   {@code AWT_POLICY_FAILURE_TO_EACH_PCSCF} (2)
         *   {@code AWT_POLICY_SPECIFIED_INTERVAL} (3)
         *   {@code AWT_POLICY_ONLY_RETRY_AFTER} (4)
         */
        static const IMS_CHAR KEY_REG_ACTUAL_WAIT_TIME_POLICY_INT[];
        enum
        {
            AWT_POLICY_RFC_RULE = 0,
            AWT_POLICY_FAILURE_TO_EVERY_PCSCF = 1,
            AWT_POLICY_FAILURE_TO_EACH_PCSCF = 2,
            AWT_POLICY_SPECIFIED_INTERVAL = 3,
            AWT_POLICY_ONLY_RETRY_AFTER = 4
        };

        /**
         * Specifies the default wait time prior to proceed to the next PCSCF.
         *
         * If this value is greater than 0, this will overlay the actual wait time.
         *
         * Possible Values:
         *   {@code 0}
         */
        static const IMS_CHAR KEY_REG_DEFAULT_WAIT_TIME_INT[];

        /**
         * Specifies how to handle the registration when registration is refresh during out of
         * service.
         *
         * {@code REG_OOS_POLICY_DEFAULT} (0) Indicate that reregistration is not tried during
         * OOS and reregistration is attempted after network service state is changed to in
         * service and registration is not expired.
         *
         * Possible Values:
         *   {@code REG_OOS_POLICY_DEFAULT} (0)
         *   {@code REG_OOS_POLICY_DESTROY} (1)
         */
        static const IMS_CHAR KEY_REG_OUT_OF_SERVICE_POLICY_INT[];
        enum
        {
            REG_OOS_POLICY_DEFAULT = 0,
            REG_OOS_POLICY_DESTROY = 1
        };

        /**
         * Specifies which the PCSCF address to use when the PCSCF address are changed with
         * address and order.
         *
         * Possible Values:
         *   {@code REG_PCSCF_UPDATE_POLICY_DEFAULT} (0)
         *   {@code REG_PCSCF_UPDATE_POLICY_ALL_THE_TIME} (1)
         */
        static const IMS_CHAR KEY_REG_PCSCF_UPDATE_POLICY_INT[];
        enum
        {
            REG_PCSCF_UPDATE_POLICY_DEFAULT = 0,
            REG_PCSCF_UPDATE_POLICY_ALL_THE_TIME = 1
        };

        /**
         * {@code SIP_305_CODE_POLICY_DEFAULT} (0) follow KEY_DEFAULT_RETRY_POLICY_INT
         * operation if 305 code is not configured from other configurations of this bundle.
         *
         * {@code SIP_305_CODE_POLICY_3GPP} (1) follow 3GPP 24.229
         * {@code SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF} (2) follow 3GPP 24.229 starting from
         * the top of the existing PCSCF list
         * {@code SIP_305_CODE_POLICY_USE_CONTACT_VALUE} (3) Flag indicating
         * whether a new IMS registration is tried using the contact header field value as
         * specified in RFC3261 after a 305 response for registration is received.
         *
         * Possible Values:
         *   {@code 0}
         */
        static const IMS_CHAR KEY_REG_RETRY_305_POLICY_INT[];
        enum
        {
            SIP_305_CODE_POLICY_DEFAULT = 0,
            SIP_305_CODE_POLICY_3GPP = 1,
            SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF = 2,
            SIP_305_CODE_POLICY_USE_CONTACT_VALUE = 3
        };

        /**
         * {@code SIP_503_CODE_POLICY_DEFAULT} (0) Follow KEY_DEFAULT_RETRY_POLICY_INT
         * operation if 305 code is not configured from other configurations of this bundle.
         *
         * {@code SIP_503_CODE_POLICY_3GPP} (1) Follow 3GPP 24.229.
         *
         * Possible Values:
         *   {@code None}
         */
        static const IMS_CHAR KEY_REG_RETRY_503_POLICY_INT[];
        enum
        {
            SIP_503_CODE_POLICY_DEFAULT = 0,
            SIP_503_CODE_POLICY_3GPP = 1
        };

        /**
         * Specifies the number of retry for each PCSCF.
         *
         * If value is 0, it follows standard(=1).
         *
         * Possible Values:
         *   {@code 3}
         */
        static const IMS_CHAR KEY_REG_RETRY_CNT_ON_SINGLE_PCSCF_INT[];

        static const IMS_CHAR KEY_REG_RETRY_CNT_PER_PCSCF_INT[];

        /**
         * Specifies retry count to try to register with that P-CSCF if retry-after header is
         * present in the response.
         *
         * If retry-after header is present in the response, try to register with the
         * same P-CSCF as many retries as defined here. It's applied if
         * CarrierConfig::Ims::KEY_REG_ACTUAL_WAIT_TIME_POLICY_INT is
         * CarrierConfig::Ims::AWT_POLICY_ONLY_RETRY_AFTER.
         *
         * Possible Values:
         *   {@code 5}
         */
        static const IMS_CHAR KEY_REG_RETRY_CNT_PER_PCSCF_WITH_RA_TIME_INT[];

        /**
         * {@code REG_RETRY_CNT_RESET_POLICY_REGISTRATION} (0)
         * {@code REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION} (1)
         * {@code REG_RETRY_CNT_RESET_POLICY_NOTIFY} (2).
         *
         * Possible Values:
         *   {@code 0} ~ 2
         */
        static const IMS_CHAR KEY_REG_RETRY_CNT_RESET_POLICY_INT[];
        enum
        {
            REG_RETRY_CNT_RESET_POLICY_REGISTRATION = 0,
            REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION = 1,
            REG_RETRY_CNT_RESET_POLICY_NOTIFY = 2
        };

        /**
         * Specifies the number of retry count of registration with IPSEC on authentication
         * failure.
         *
         * Possible Values:
         *   {@code 0}
         */
        static const IMS_CHAR KEY_REG_RETRY_CNT_WITH_IPSEC_ON_AUTH_FAILURE_INT[];

        /**
         * Specifies the default retry policy about how to use the PCSCF address
         * selection.
         *
         * {@code DEFAULT_RETRY_POLICY_SPEC} (0)
         * {@code DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF} (1)
         * {@code DEFAULT_RETRY_POLICY_NEXT_PCSCF} (2).
         *
         * Possible Values:
         *   {@code DEFAULT_RETRY_POLICY_SPEC} (0)
         *   {@code DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF} (1)
         *   {@code DEFAULT_RETRY_POLICY_NEXT_PCSCF} (2)
         */
        static const IMS_CHAR KEY_REG_RETRY_DEFAULT_POLICY_INT[];
        enum
        {
            DEFAULT_RETRY_POLICY_SPEC = 0,
            DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF = 1,
            DEFAULT_RETRY_POLICY_NEXT_PCSCF = 2
        };

        /**
         * Specifies the policy of handling the failure with TIME_F expire.
         *
         * (0) It is not applicable. {@code TIMER_F_POLICY_SPEC} (1) Indicate whether
         * registration is tried based on 3GPP 24.229 when registration is failed because the
         * timer F expires.
         *
         * Possible Values:
         *   {@code 1}
         *   {@code 2}
         */
        static const IMS_CHAR KEY_REG_RETRY_TIMER_F_POLICY_INT[];
        enum
        {
            TIMER_F_POLICY_NONE = 0,
            TIMER_F_POLICY_SPEC = 1,
            TIMER_F_POLICY_SPEC_WITH_AWT = 2
        };

        static const IMS_CHAR KEY_REG_TRANSACTION_TIMEOUT_ON_PCSCF_RESTORATION_SEC_INT[];

        /**
         * Specifies the delay time to release pdn after temporary plmn block.
         *
         * (Seconds) If the value is 0, release pdn immediately after temporary plmn
         * block. If the value is greater than 0, release ims pdn in the value(delay time) after
         * temporary plmn block.
         *
         * Possible Values:
         *   {@code 0}
         */
        static const IMS_CHAR KEY_RELEASE_PDN_DELAY_SEC_AFTER_TEMP_PLMN_BLOCK_INT[];

        /**
         * reuse {@code KEY_REGISTRATION_RETRY_SIP_305_CODE_POLICY_INT} definition
         * {@code SIP_305_CODE_POLICY_NONE} (0) It is not applicable.
         *
         * {@code SIP_305_CODE_POLICY_3GPP} (1) follow 3GPP 24.229
         * {@code SIP_305_CODE_POLICY_USE_CONTACT_VALUE} (2) Flag indicating whether a new IMS
         * registration is tried using the contact header field value as specified in RFC3261
         * after a 305 response for registration is received.
         *
         * Possible Values:
         *   {@code None}
         */
        static const IMS_CHAR KEY_REREG_RETRY_305_POLICY_INT[];

        /**
         * Specifies the SIP message threshold size caused by the transport change If
         * #PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP is set, this config is used.
         *
         * @see KEY_SIP_PREFERRED_TRANSPORT_INT SIP message size is more than MTU
         * size - this value, then SIP transport will be TCP, else the SIP transport is UDP.
         *
         * Possible Values:
         *   {@code 340}
         */
        static const IMS_CHAR KEY_SIP_MESSAGE_THRESHOLD_FOR_TRANSPORT_CHANGE_INT[];

        /**
         * Specifies the policy of handling the failure when receiving 503 error response to the
         * SUBSCRIBE message.
         *
         * {@code SIP_503_CODE_POLICY_DEFAULT} (0) Follow default retry operation. (Retry
         * SUBSCRIBE message after retry-after or AWT) {@code SIP_503_CODE_POLICY_3GPP} (1) Follow
         * 3GPP 24.229.
         *
         * Possible Values:
         *   {@code 0}
         */
        static const IMS_CHAR KEY_SUB_RETRY_503_POLICY_INT[];

        /**
         * Specifies the USAT IMS registration event download policy Specify the support policy
         * of USAT IMS registration event download TS 131.111 7.5.21 describes about the USAT
         * IMS registration event download and the conditions to be notified to UICC when an
         * IMS registration event occurs.
         *
         * THIS asset indicates whether USAT IMS registration event download is
         * supported, and if so, whether to check the precondition before notifying.
         *
         * Possible Values:
         *   {@code USAT_REG_EVENT_NOT_DOWNLOAD} (0)
         *   {@code :} Not support IMS Registration Event.
         *   {@code USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD} (1)
         *   {@code :} Support IMS Registration Event without checking EF-UICCIARI.
         *   {@code USAT_REG_EVENT_CONDITIONAL_DOWNLOAD} (2)
         *   {@code :} Support IMS Registration Event if there is EF-UICCIARI.
         *   {@code (b/279986730)}
         */
        static const IMS_CHAR KEY_USAT_REG_EVENT_DOWNLOAD_POLICY_INT[];
        enum
        {
            USAT_REG_EVENT_NOT_DOWNLOAD = 0,
            USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD = 1,
            USAT_REG_EVENT_CONDITIONAL_DOWNLOAD = 2
        };

        /**
         * Specifies the policies that are needed to keep the IMS registration with mmtel
         * feature tag.
         *
         * Possible Values:
         *   {@code UNAVAILABLE_FEATURE_POLICY_VOPS} (1)
         *   {@code UNAVAILABLE_FEATURE_POLICY_SSAC} (2)
         *   {@code UNAVAILABLE_FEATURE_POLICY_3G} (3)
         */
        static const IMS_CHAR KEY_KEEP_REG_WITH_MMTEL_FEATURE_TAG_POLICY_INT_ARRAY[];
        enum
        {
            UNAVAILABLE_FEATURE_POLICY_VOPS = 1,
            UNAVAILABLE_FEATURE_POLICY_SSAC = 2,
            UNAVAILABLE_FEATURE_POLICY_3G = 3
        };

        /**
         * If the PDN connection request is rejected with an operator specific error cause
         * (ex. 28 or 33), UE should treat these cause codes as permanent errors and
         * make no additional attempts to establish a PDN connection to the IMS APN
         * This configuration specify these error cause list for each operators.
         *
         * Possible Values:
         *   {@code 27} (MISSING_UNKNOWN_APN)
         *   {@code 28} (UNKNOWN_PDP_ADDRESS_TYPE)
         *   {@code 33} (SERVICE_OPTION_NOT_SUBSCRIBED)
         */
        static const IMS_CHAR KEY_PERMANENT_PDN_FAILURE_INT_ARRAY[];

        /**
         * Specifies the error codes of the registration followed by PCSCF discovery
         * when PCSCF is unavailable.
         *
         * Possible Values:
         *   {@code 504}
         */
        static const IMS_CHAR KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY[];

        /**
         * Specifies the number of error code considered the final result.
         * (This value is indicated by two numbers if the value is different
         * in the home and roaming network.
         * This value is indicated by a single number if the value is the same
         * in the home and roaming network.)
         *
         * Possible Values:
         *   {@code {2}}
         */
        static const IMS_CHAR KEY_REG_PERMANENT_ERR_MAX_CNT_INT_ARRAY[];

        /**
         * Specifies the list of error codes whether the initial registration
         * without IPSEC is tried or not.
         * IPSEC is enabled when the initial registration is tried with other PCSCFs.
         * When it is updated, please check the configs below (b/260172173, ag/21457244)
         *   -  KEY_INIT_IPSEC_SETTING_WITH_NEW_PCSCF_BOOL
         *   -  KEY_REG_RETRY_CNT_WITH_IPSEC_ON_AUTH_FAILURE_INT
         *
         * Possible Values:
         *   {@code {406}}
         */
        static const IMS_CHAR KEY_REG_RETRY_ERR_CODE_WITHOUT_IPSEC_INT_ARRAY[];

        /**
         * Specifies the list of RAT(Radio Access Technology) network types for which PLMN block
         * should be applied when IMS registration fails on all P-CSCFs.
         * This configuration specifies whether to block the PLMN in certain network types
         * if registration fails across all P-CSCFs.
         *
         * Possible Values:
         *   {@code CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN} (6)
         *   {@code CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN} (3)
         */
        static const IMS_CHAR KEY_REG_TEMP_PLMN_BLOCK_RATS_ON_ALL_PCSCFS_FAIL_INT_ARRAY[];

        /**
         * Specifies the list of error codes that result in terminating the IMS call
         * when reregistration fails.
         *
         * Possible Values:
         *   {@code {403}}
         */
        static const IMS_CHAR KEY_REREG_ERR_CODE_FOR_CALL_END_INT_ARRAY[];

        /**
         * Specifies the error codes of the reregistration followed by IMS PDN reactivation.
         *
         * Possible Values:
         *   {@code {403}}
         */
        static const IMS_CHAR KEY_REREG_ERR_CODE_FOR_IMS_PDN_REACTIVATION_INT_ARRAY[];

        /**
         * Specifies the error codes of the reregistration followed by intital registration
         * with available PCSCF. If no available PCSCF, IMS PDN is re-activated.
         *
         * Possible Values:
         *   {@code {0}} REG_ERROR_CODE_TIMER_F
         */
        static const IMS_CHAR KEY_REREG_ERR_CODE_FOR_INIT_REG_WITH_AVAILABLE_PCSCF_INT_ARRAY[];

        /**
         * Specifies the list of error cause that results in new registration
         * with same PCSCF when reregistration is failed.
         *
         * Possible Values:
         *   {@code {0}} TIMER_F
         */
        static const IMS_CHAR KEY_REREG_RETRY_ERR_CODE_FOR_INIT_REG_WITH_SAME_PCSCF_INT_ARRAY[];

        /**
         * Specifies information trying initial ims registration with next pcscf address
         * when it receives what error responses against SUBSCRIBE msg for the reg event package.
         *
         * ("Timer F" should be replaced to 0 when error response argument is represented)
         *
         * Possible Values:
         *   {@code {420}}
         *   {@code {421}}
         *   {@code {422}}
         */
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_NEXT_PCSCF_INT_ARRAY[];

        /**
         * Specifies information not trying send ims subscription by ims subscription
         * expiration time when it receives what kind of error responses against re-SUBSCRIBE
         * msg for the reg event package.
         *
         * ("4xx" should be replaced to "4" if it proceeds when it receives 400s
         * responses like 404, 403, and so on.).
         *
         * Possible Values:
         *   {@code {4, 503}}
         *   {@code {4, 5, 6}}
         */
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_STOPPING_BY_EXPIRATION_TIME_INT_ARRAY[];

        /**
         * Specifies list of different RAT technologies on which IMS is supported
         * in roaming network.
         *
         * Possible Values:
         *   {@code ACCESS_NETWORK_TYPE_GERAN} (1)
         *   {@code ACCESS_NETWORK_TYPE_UTRAN} (2)
         *   {@code ACCESS_NETWORK_TYPE_EUTRAN} (3)
         *   {@code ACCESS_NETWORK_TYPE_IWLAN} (5)
         *   {@code ACCESS_NETWORK_TYPE_NGRAN} (6)
         */
        static const IMS_CHAR KEY_SUPPORTED_ROAMING_RATS_INT_ARRAY[];

        static const IMS_CHAR KEY_TEST_MODE_INT_ARRAY[];
        enum
        {
            TEST_MODE_PERMANENT_FAILURE_WITHOUT_IMS_PDN_DEACTIVATION = 1
        };

        /**
         * Specifies list of features that unavailable in limited registration.
         *
         * Possible Values:
         *   {@code FEATURE_TYPE_MMTEL} (0)
         *   {@code FEATURE_TYPE_VIDEO} (1)
         *   {@code FEATURE_TYPE_TEXT} (2)
         *   {@code FEATURE_TYPE_SMS} (3)
         */
        static const IMS_CHAR KEY_UNAVAILABLE_FEATURES_IN_LIMITED_REG_INT_ARRAY[];
        enum
        {
            REG_FEATURE_MMTEL = 0,
            REG_FEATURE_VIDEO = 1,
            REG_FEATURE_TEXT = 2,
            REG_FEATURE_SMS = 3
        };

        // Bundle {
        /**
         * It can be made with multiple objects.
         */
        static const IMS_CHAR KEY_EXTRA_REG_ERR_BUNDLE[];

        /**
         * Specifies It handle as registration failure immediately in roaming
         * when reregistation fails once.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_EXTRA_REG_ERR_CODE_AS_FAILURE_IN_ROAMING_FOR_UPDATE_BOOL[];

        /**
         * Specifies whether the retry counter should be shared between REGISTER and SUBSCRIBE
         * for reg event package.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_EXTRA_REG_ERR_RETRY_CNT_SHARED_FOR_REG_AND_SUB_BOOL[];

        /**
         * ERROR_TYPE_NOT_SPECIFIED (0)
         * ERROR_TYPE_REPEATED (1) Indicate that it results in blocking PLMN
         *                         with the specific protocol timer like T3402.
         * ERROR_TYPE_CRITICAL (2) Indicate that it results in blocking PLMN.
         * ERROR_TYPE_ROAMING(3) Indicate that it results in blocking PLMN basd on
         *                       the attached network type.
         * ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK (4) Indicate that it results in blocking
         *                                                    PLMN with the specific protocol timer
         *                                                    like T3402.
         */
        static const IMS_CHAR KEY_EXTRA_REG_ERR_FINAL_TYPE_INT[];
        enum
        {
            ERROR_TYPE_NOT_SPECIFIED = 0,
            ERROR_TYPE_REPEATED = 1,
            ERROR_TYPE_CRITICAL = 2,
            ERROR_TYPE_ROAMING = 3,
            ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK = 4,
            ERROR_TYPE_RAT_BLOCK = 5
        };

        static const IMS_CHAR KEY_EXTRA_REG_ERR_MAX_CNT_INT[];

        /**
         * Specifies the retry attempt count about pcscfs being discovered in combined network.
         *
         * Possible Values:
         *   {@code 1}
         */
        static const IMS_CHAR KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_EPS_5GS_ONLY_ATTACHED_INT[];

        /**
         * Specifies the retry attempt count about pcscfs being discovered in only attached network.
         *
         * Possible Values:
         *   {@code 2}
         */
        static const IMS_CHAR KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_LTE_COMBINED_ATTACHED_INT[];

        /**
         * ERROR_TYPE_REPEATED in KEY_FINAL_ERROR_TYPE : Indicate the maximum retry count with same
         *                                               PCSCF. If the retry count reaches the
         *                                               maximum count, registration is tried with
         *                                               other PCSCF.
         * ERROR_TYPE_CRITICAL in KEY_FINAL_ERROR_TYPE : Indicate the number of error response that
         *                                               is included in KEY_ERROR_CODE_INT_ARRAY.
         *                                               If this number reaches, it handles as
         *                                               critical error.
         */
        static const IMS_CHAR KEY_EXTRA_REG_ERR_POLICY_INT[];
        enum
        {
            ERROR_POLICY_NOT_SPECIFIED = 0,
            ERROR_POLICY_PCSCF_FAILED = 1,
            ERROR_POLICY_SUBSCRIBER_FAILED = 2,
            ERROR_POLICY_PDN_REACTIVATED = 3,
            ERROR_POLICY_SUBSCRIBER_FAILED_NO_IMSI_FALLBACK = 4
        };

        /**
         * ERROR_POLICY_PCSCF in KEY_ERROR_POLICY : Indicate the error codes that result in initial
         *                                          registration with other PCSCFs available.
         * ERROR_POLICY_SUBSCRIBER_FAILED in KEY_ERROR_POLICY : Indicate the list of error codes
         *                                                      that result in initial registration
         *                                                      with another PCSCF after wait-time.
         *                                                      If initial registration fails with
         *                                                      all PCSCFs and another IMPU is
         *                                                      available, registration is retried
         *                                                      with changed IMPU.
         *                                                      After it fails with both IMPUs
         *                                                      based on MSISDN and IMSI,
         *                                                      registration is handled as critcal
         *                                                      error with PLMN block
         *
         * wait time @see KEY_WAIT_TIME_SEC_INT
         *
         * ERROR_POLICY_PDN_REACTIVATED in KEY_ERROR_POLICY : Indicate the error causes that result
         *                                                    in new PCSCF discovery when the
         *                                                    calculated retry number reaches.
         *
         * Possible Values:
         *   TIMER_F - 0
         *   TRANSPORT_ERROR - 1
         *   {305, 403, 404}
         */
        static const IMS_CHAR KEY_EXTRA_REG_ERR_CODE_INT_ARRAY[];
        enum
        {
            REG_ERROR_CODE_TIMER_F = 0,
            REG_ERROR_CODE_TRANSPORT = 1,
            REG_ERROR_CODE_OTHER = 2,
            REG_ERROR_CODE_3XX = 3,
            REG_ERROR_CODE_4XX = 4,
            REG_ERROR_CODE_5XX = 5,
            REG_ERROR_CODE_6XX = 6,
            REG_ERROR_CODE_ALL_RESP = 9,
            REG_ERROR_CODE_USIM_AUTHENTICATION = 10
        };

        /**
         * Specifies the list of error codes for reregistration that is followed by
         * initial registration with other PCSCFs available.
         *
         * Possible Values:
         *   TIMER_F - 0
         *   {@code 403, 404}
         */
        static const IMS_CHAR KEY_EXTRA_REG_ERR_CODE_FOR_UPDATE_INT_ARRAY[];

        /**
         * Specifies the list of wait-time seconds when registration is retried.
         *
         * Possible Values:
         *   {@code 30}
         */
        static const IMS_CHAR KEY_EXTRA_REG_ERR_WAIT_TIME_SEC_INT_ARRAY[];
        // }

        // Bundle {
        /**
         * Specifies information whether trying initial ims registration when it receives
         * NOTIFY msg with "terminated" state.
         */
        static const IMS_CHAR KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE[];

        /**
         * Specifies information what time needed for trying initial ims registration when it
         * receives NOTIFY msg with "terminated" state (It is represented in seconds.)
         *
         * Possible Values:
         *   {@code 60}
         */
        static const IMS_CHAR KEY_NOTIFY_TERMINATED_FOR_INIT_REG_WITH_WAIT_TIME_INT[];

        /**
         * Specifies information what event received  when it receives NOTIFY msg with "terminated"
         * state for trying initial ims registration.
         *
         * Possible Values:
         *   NOTIFY_TERMINATED_EXPIRED (1)
         *   NOTIFY_TERMINATED_DEACTIVATED (2)
         *   NOTIFY_TERMINATED_PROBATION (3)
         *   NOTIFY_TERMINATED_UNREGITERED (4)
         *   NOTIFY_TERMINATED_REJECTED (5)
         */
        static const IMS_CHAR KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY[];

        /**
         * Specifies information what event received when it receives NOTIFY msg with "terminated"
         * state for trying initial ims registration to follow the wait time.
         * (wait time @see KEY_WAIT_TIME_INT)
         *
         * Possible Values:
         *   NOTIFY_TERMINATED_EXPIRED (1)
         *   NOTIFY_TERMINATED_DEACTIVATED (2)
         *   NOTIFY_TERMINATED_PROBATION (3)
         *   NOTIFY_TERMINATED_UNREGITERED (4)
         *   NOTIFY_TERMINATED_REJECTED (5)
         */
        static const IMS_CHAR
                KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_WITH_WAIT_TIME_INT_ARRAY[];
        // }

        // Bundle {
        /**
         * Specifies include information to control PSCSF recovery condition of each carrier
         * with mutable value.
         */
        static const IMS_CHAR KEY_PCSCF_RECOVERY_CONDITIONS_BUNDLE[];

        /**
         * Specifies max count to retry with fixed wait time.
         * When the fail count does not reach this value yet, it waits during fixed time
         * before retrying PCSCF recovery.
         *
         * Possible Values:
         *   {@code 3}
         */
        static const IMS_CHAR KEY_PCSCF_RECOVERY_MAX_CNT_INT[];

        /**
         * Specifies wait time in seconds before retrying PCSCF recovery.
         * This is used as waiting time for PCSCF recovery during fail count is unter max count.
         * If valid PCSCF acquisition fails during this time, IMS PDN reestablishment will be
         * requested for PCSCF recovery.
         *
         * Possible Values:
         *   {@code 20}
         */
        static const IMS_CHAR KEY_PCSCF_RECOVERY_WAIT_TIME_SEC_INT[];

        /**
         * Specifies base time in seconds that is used to calculate upper-bound wait time described
         * in RFC5626. When the fail count reaches max count, waiting time for PCSCF recovery is
         * determined as a random value between the upper-bound wait time and half of it.
         *
         * Possible Values:
         *   {@code 20}
         */
        static const IMS_CHAR KEY_PCSCF_RECOVERY_BASE_TIME_SEC_INT[];

        /**
         * Specifies max time in seconds that is used to calculate upper-bound wait time described
         * in RFC5626. When the fail count reaches max count, waiting time for PCSCF recovery is
         * determined as a random value between the upper-bound wait time and half of it.
         *
         * Possible Values:
         *   {@code 1800}
         */
        static const IMS_CHAR KEY_PCSCF_RECOVERY_MAX_TIME_SEC_INT[];
        // }

        // Bundle {
        /**
         * TBD.
         */
        static const IMS_CHAR KEY_REG_ERR_CODE_WITH_RA_TIME_BUNDLE[];

        /**
         * Specifies whether the defined error codes are only applied with Retry-After header value.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false} : In case the error codes that are not defined here are applied
         *                   according to the standard.
         */
        static const IMS_CHAR KEY_REG_ERR_CODE_WITH_RA_TIME_ONLY_DEFINED_BOOL[];

        /**
         * Specifies the list of the error response with time value containing Retry-After header
         * for registration retry.
         *
         * This config is related to KEY_REG_ERR_CODE_WITH_RA_TIME_ONLY_DEFINED_BOOL.
         *
         * Possible Values:
         *   {@code 4xx}, {@code 5xx}, {@code 6xx}
         */
        static const IMS_CHAR KEY_REG_ERR_CODE_WITH_RA_TIME_INT_ARRAY[];

        /**
         * Specifies the list of the error response with time value containing Retry-After header
         * for reregistration retry.
         *
         * This config is related to KEY_REG_ERR_CODE_WITH_RA_TIME_ONLY_DEFINED_BOOL.
         *
         * Possible Values:
         *   {@code 4xx}, {@code 5xx}, {@code 6xx}
         */
        static const IMS_CHAR KEY_REG_ERR_CODE_WITH_RA_TIME_FOR_UPDATE_INT_ARRAY[];
        // }

        // Bundle {
        /**
         * TBD
         */
        static const IMS_CHAR KEY_REG_RETRY_INTERVAL_BUNDLE[];

        /**
         * Specifies whether subscription for reg event package is retried or not
         * with this interval that is same as registration.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_REG_RETRY_INTERVAL_USED_FOR_SUB_BOOL[];

        /**
         * Specifies the length of the timer shall be any seconds plus random value
         * that shall have an uper bound of the value.
         *
         * Possible Values:
         *   {@code 0, 0, 15, 0, 0}
         */
        static const IMS_CHAR KEY_REG_RETRY_INTERVAL_RANDOM_UPPER_VALUE_SEC_INT_ARRAY[];

        /**
         * Specifies the retry interval when registration is failed as general codes.
         *
         * Possible Values:
         *   {@code 30, 30, 60, 120, 480, 900}
         *   {@code 10}
         */
        static const IMS_CHAR KEY_REG_RETRY_INTERVAL_SEC_INT_ARRAY[];
        // }

        // Bundle {
        /**
         * Specifies information trying initial ims registration when it receives what kind of
         * error responses against SUBSCRIBE msg for the reg event package one time or several
         * times.
         */
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_INIT_REG_BUNDLE[];

        /**
         * Specifies information how many should be received error response to handle
         * trying new ims registration.
         *
         * Possible Values:
         *   {@code 2}
         *   {@code 1}
         */
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_RETRY_MAX_CNT_INT[];

        /**
         * Specifies information what error responses against SUBSCRIBE mag for the reg event
         * package to handle trying initial ims registration.
         *
         * ("Timer F" should be replaced to 0 when error response argument is represented)
         *
         * Possible Values:
         *   {@code 403, 504, 0}
         *   {@code 500, 503, 600}
         */
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY[];
        // }

        // Bundle {
        /**
         * Specifies information not trying ims subscription before ims registration when it
         * receives what error responses against SUBSCRIBE msg for the reg event package one
         * time or several times.
         */
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_TERMINATED_BUNDLE[];

        /**
         * Specifies information how many should be received error response to handle
         * not trying ims subscription.
         *
         * Possible Values:
         *   {@code 1}
         *   {@code 2}
         */
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_TERMINATED_WITH_RETRY_MAX_CNT_INT[];

        /**
         * Specifies information what error responses against SUBSCRIBE mag for the reg event
         * package to handle not trying ims subscription.
         *
         * ("Timer F" should be replaced to 0 when error response argument is represented)
         *
         * Possible Values:
         *   {@code 489}
         *   {@code 400, 403, 404, 420}
         */
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_TERMINATED_INT_ARRAY[];
        // }

        // Mtc
        static const IMS_CHAR KEY_INFORMATION_LEVEL_OF_GEOLOCATION_PIDF_INT_ARRAY[];
        enum
        {
            GEOLOCATION_PIDF_INFO_LAT_AND_LONG = 0,
            GEOLOCATION_PIDF_INFO_LAT_AND_LONG_AND_CIVIC = 1,
            GEOLOCATION_PIDF_INFO_COUNTRY_CODE_ONLY = 2,
            GEOLOCATION_PIDF_INFO_COUNTRY_CODE_AND_STATE = 3,
        };

        /**
         * Specifies the mode for including the Geolocation-Routing header.
         *
         * Possible Values:
         *   {@code GEOLOCATION_HEADER_MODE_NOT_PRESENT} (0) : The Geolocation-Routing header is
         *                                                     not included.
         *   {@code GEOLOCATION_HEADER_MODE_INCLUDE_YES_ON_IWLAN} (1) : Include "yes" in the
         *                                                              Geolocation-Routing header
         *                                                              only when on IWLAN.
         *   {@code GEOLOCATION_HEADER_MODE_INCLUDE_YES_ALWAYS} (2) : Always include "yes" in the
         *                                                            Geolocation-Routing header.
         *   {@code GEOLOCATION_HEADER_MODE_INCLUDE_NO_ALWAYS} (3) : Always include "no" in the
         *                                                           Geolocation-Routing header.
         */
        static const IMS_CHAR KEY_GEOLOCATION_ROUTING_HEADER_MODE_INT[];
        enum
        {
            GEOLOCATION_HEADER_MODE_NOT_PRESENT = 0,
            GEOLOCATION_HEADER_MODE_INCLUDE_YES_ON_IWLAN = 1,
            GEOLOCATION_HEADER_MODE_INCLUDE_YES_ALWAYS = 2,
            GEOLOCATION_HEADER_MODE_INCLUDE_NO_ALWAYS = 3
        };

        // Unused.
        // Network types
        enum
        {
            NETWORK_TYPE_HOME = 0,
            NETWORK_TYPE_ROAMING = 1
        };

        // Unused.
        // Reasons for terminated state of "reg" event package
        enum
        {
            REG_EVENT_TERMINATED_REASON_EXPIRED = 1,
            REG_EVENT_TERMINATED_REASON_DEACTIVATED = 2,
            REG_EVENT_TERMINATED_REASON_PROBATION = 3,
            REG_EVENT_TERMINATED_REASON_UNREGITERED = 4,
            REG_EVENT_TERMINATED_REASON_REJECTED = 5
        };
    };

    class ImsEmergency
    {
    public:
        // Public carrier-config - starts
        static const IMS_CHAR KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL[];
        static const IMS_CHAR KEY_EMERGENCY_CALLBACK_MODE_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_EMERGENCY_QOS_PRECONDITION_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_EMERGENCY_OVER_IMS_SUPPORTED_RATS_INT_ARRAY[];
        static const IMS_CHAR KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT[];
        // Public carrier-config - ends
        static const IMS_CHAR KEY_EMERGENCY_INVITE_18X_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_EMERGENCY_PROVISIONAL_TO_FINAL_RESPONSE_TIMER_MILLIS_INT[];

        // Aos
        static const IMS_CHAR KEY_RELEASE_EPDN_UPON_ECALL_END_IF_EATTACH_BOOL[];

        /**
         * Specifies if emergency PDN would be released upon emergency call is terminated in
         * fake mode.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_RELEASE_EPDN_UPON_ECALL_END_IN_FAKE_MODE_BOOL[];

        /**
         * Specifies IPCAN that emergency PDN shall be released after the emergency call is
         * ended.
         *
         * Possible Values:
         *   {@code IPCAN_NONE} (0)
         *   {@code IPCAN_CELLULAR} (1)
         *   {@code IPCAN_WLAN} (2)
         *   {@code IPCAN_ALL} (3)
         */
        static const IMS_CHAR KEY_IPCAN_RELEASE_EMERGENCY_PDN_UPON_EMERGENCY_CALL_END_INT[];
        enum
        {
            IPCAN_NONE = 0,
            IPCAN_CELLULAR = 1,
            IPCAN_WLAN = 2,
            IPCAN_ALL = 3
        };

        /**
         * Specifies whether emergency call is tried without emergency registration
         *
         * Specify the preferred policy for emergency registration
         * Possible values:
         *   {@code PREFERRED_EMERGENCY_REGISTRATION_SKIP} 0 : The emergency call is tried without
         *                                                     the emergency registration.
         *   {@code PREFERRED_EMERGENCY_REGISTRATION_NORMAL} 1 : The emergency call is handled as
         *                                                       failure when the emergency
         *                                                       registration is failed.
         *   {@code PREFERRED_EMERGENCY_REGISTRATION_FALLBACK} 2 : The emergency call is tried
         *                                                         without the emergency
         *                                                         registration after the emergency
         *                                                         registration is failed.
         */
        static const IMS_CHAR KEY_PREFERRED_EMERGENCY_REGISTRATION_INT[];
        enum
        {
            PREFERRED_EMERGENCY_REGISTRATION_NOT_DEFINED = -1,
            PREFERRED_EMERGENCY_REGISTRATION_SKIP = 0,
            PREFERRED_EMERGENCY_REGISTRATION_NORMAL = 1,
            PREFERRED_EMERGENCY_REGISTRATION_FALLBACK = 2
        };

        /**
         * Specifies the delay time from the end of the call to the start of the emergency
         * PDN release. If this is set to zero, it means not set the delay time to release
         * emergency PDN when call ends.
         *
         * And this configuration can have a dependency with following configurations.
         * Those configuration is used to release emergency PDN just after the emergency call ends.
         * So, if you want to set the emergency PDN to be released after a specific amount of
         * time after the emergency call ends, you should not set the following configurations.
         * Because if that is set, it would release the emergency PDN just after the emergency
         * call ends.
         *
         * #KEY_IPCAN_RELEASE_EMERGENCY_PDN_UPON_EMERGENCY_CALL_END_INT
         * #KEY_RELEASE_EPDN_UPON_ECALL_END_IN_FAKE_MODE_BOOL
         *
         * Possible Values:
         *   {@code 240000}
         */
        static const IMS_CHAR KEY_WAIT_TIME_MILLIS_FOR_RELEASE_EPDN_AFTER_ECALL_END_INT[];

        /**
         * Specifies whether the authorized IMPU from P-Associated-URI header in 200 OK for IMS
         * registration is used in emergency call.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_ECALL_BASED_ON_P_ASSOCIATED_URI_OF_NORMAL_REG_BOOL[];

        /**
         * Specifies if UE tries emergency registration on a random pcscf.
         *
         * If this is set as TRUE, UE will choose P-CSCF randomly for emergency
         * registration if UE receives multiple P-CSCF addresses from P-CSCF discovery for
         * emergency.
         *
         * Possible Values:
         *   {@code None}
         */
        static const IMS_CHAR KEY_EREG_ON_RANDOM_PCSCF_BOOL[];

        /**
         * Specifies if emergency registration is transmitted only over TCP in roaming network.
         *
         * It will be applied in all the SIP messages which are sent via this
         * emergency IMS registration.
         *
         * Possible Values:
         *   {@code None}
         */
        static const IMS_CHAR KEY_EREG_SET_TCP_ONLY_IN_ROAMING_BOOL[];

        /**
         * Specifies if the first public user identity to the list stored in the ISIM is used in
         * emergency registration requests.
         *
         * If this is set as TRUE, the first public user identity in the ISIM will be
         * used for emergency IMS registration.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_EREG_USING_FIRST_IMPU_IN_ISIM_BOOL[];

        /**
         * Specifies whether to keep emergency pdn when there is no pcscf available by
         * requesting fake registration with the next pcscf.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_KEEP_EPDN_UPON_PCSCF_UNAVAILABLE_BOOL[];

        /**
         * Specifies whether keep on retrying emergency registration while the emergency PDN is
         * connected on WLAN.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_KEEP_EREG_RETRY_ON_WLAN_BOOL[];

        /**
         * Specifies how to handle the emergency call as failure or to proceed it without
         * completing the normal registration after the timer as
         * CarrierConfig::ImsEmergency::KEY_REG_TIMER_FOR_ECALL_MILLIS_INT setting value is expired.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_REG_TIMER_FOR_ECALL_TIMEOUT_AS_FAILURE_BOOL[];

        /**
         * Specifies whether RAT is checked or not when E-PDN is requested. If RAT is not
         * IMS supported, the timer as
         * CarrierConfig::ImsEmergency::KEY_REG_TIMER_FOR_ECALL_MILLIS_INT setting value is not
         * started and the emergency call is proceeded without that timer waiting for the normal
         * registration.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_REG_TIMER_FOR_ECALL_WITH_RAT_CHECK_ENABLED_BOOL[];

        /**
         * Specifies if emergency PDN would be released when the network is unavailable.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_RELEASE_EPDN_OF_UNAVAILABLE_NETWORK_BOOL[];

        /**
         * Specifies whether to stop emergency registration timer on E-PDN connection.
         *
         * After the timer is expired, the emergency call is handled as failure.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_STOP_EREG_TIMER_ON_EPDN_CONNECTED_BOOL[];

        static const IMS_CHAR KEY_SUPPORT_ANONYMOUS_ECALL_ACTION_BOOL[];

        static const IMS_CHAR KEY_SUPPORT_EREG_WHEN_EATTACH_WITH_VALID_SIM_BOOL[];

        /**
         * Specifies if emergency re-registration is supported after EPDN handover between
         * cellular and IWLAN.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_SUPPORT_EREREG_ON_IPCAN_CHANGE_BOOL[];

        /**
         * Specifies if GIBA(GPRS-IMS-Bundled authentication) is support for emergency
         * registration in roaming.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_SUPPORT_GIBA_FOR_EREG_IN_ROAMING_BOOL[];

        /**
         * Specifies if video feature tag is supported for emergency registration.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_SUPPORT_VIDEO_FOR_EREG_BOOL[];

        /**
         * Specifies whether to follow reg retry rule when emergency registration failed.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_USE_REG_RETRY_RULE_FOR_EREG_BOOL[];

        /**
         * Specifies the preferred IP version priority for emergency connection.
         *
         * Possible Values:
         *   {@code IPV4} (0)
         *   {@code IPV6} (1)
         */
        static const IMS_CHAR KEY_EPDN_PREFERRED_IPTYPE_INT[];

        /**
         * Specifies the number of emergency registration retry attempt to P-CSCFs.
         *
         * UE will try emergency registration with specified number of P-CSCFs when
         * CarrierConfig::Assets::KEY_EREG_RETRY_TIMER_MILLIS_INT timer has expired. If the
         * number is zero, UE will try registration on every P-CSCFs once. If the number of
         * P-CSCF is less than a given number and UE's default retry policy is a
         * CarrierConfig::Assets::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF, UE will try
         * registration from the first P-CSCF again after attempting on all P-CSCFs. If UE
         * doesn't support emerg-reg-retry defined in 3GPP 24.229, which is configured by
         * CarrierConfig::Assets::KEY_EMC_REG_RETRY_TIMER_MILLIS_INT, this configuration is
         * discarded. As an exception, if this value is set to -1, the emergency registration
         * retry will not be performed and will be treated as a failure. The subsequent action
         * will be determined by the
         * CarrierConfig::ImsEmergency::KEY_PREFERRED_EMERGENCY_REGISTRATION_INT setting value.
         *
         * Possible Values:
         *   {@code 0}
         */
        static const IMS_CHAR KEY_EREG_RETRY_MAX_CNT_INT[];
        enum
        {
            EREG_RETRY_MAX_CNT_NO_RETRY = -1,
            EREG_RETRY_MAX_CNT_EVERY_PCSCF_RETRY = 0
        };

        /**
         * Specifies the maximum time from sending a SIP REGISTER for an emergency registration
         * until UE receives any final response from this P-CSCF. It will be stopped when the
         * CarrierConfig::ImsEmergency::KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT timer has been
         * stopped or expired. Upon this timer expiry, the UE considers that the emergency
         * registration attempt for this P-CSCF has failed. The UE may retry registration on
         * a different P-CSCF if available and restart the
         * CarrierConfig::Assets::KEY_EREG_RETRY_TIMER_MILLIS_INT timer.
         * If the UE has no more available P-CSCFs, the UE shall stop the
         * CarrierConfig::ImsEmergency::KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT timer
         * by considering the emergency registration has failed. If the value is zero,
         * it considers that the UE doesn't support the emerg-reg-retry timer defined
         * in 3GPP 24.229.
         *
         * Possible Values:
         *   {@code 0}
         */

        static const IMS_CHAR KEY_EREG_RETRY_TIMER_MILLIS_INT[];

        /**
         * Specifies waiting max timer for completing normal IMS registration before the emergency
         * call is performed. After the timer is expired, the emergency call is handled as failure.
         *
         * Possible Values:
         *   {@code 10000}
         */
        static const IMS_CHAR KEY_REG_TIMER_FOR_ECALL_MILLIS_INT[];

        /**
         * Specifies the preferred policy for emergency registration in roaming.
         *
         * If the value is ‘'PREFERRED_EMERGENCY_REGISTRATION_NOT_DEFINED', the UE in roaming
         * follows the same rule as the home.
         *
         * Possible Values:
         *   {@code PREFERRED_EMERGENCY_REGISTRATION_NOT_DEFINED} (-1) : The emergency call in
         *                                                               roaming  is tried in the
         *                                                               same way as at home.
         *   {@code PREFERRED_EMERGENCY_REGISTRATION_SKIP} (0) : The emergency call is tried
         *                                                       without the emergency
         *                                                       registration.
         *   {@code PREFERRED_EMERGENCY_REGISTRATION_NORMAL} (1) : The emergency call is handled as
         *                                                         failure when the emergency
         *                                                         registration is failed.
         *   {@code PREFERRED_EMERGENCY_REGISTRATION_FALLBACK} (2) : The emergency call is tried
         *                                                           without the emergency
         *                                                           registration after the
         *                                                           emergency registration is
         *                                                           failed.
         */
        static const IMS_CHAR KEY_ROAMING_PREFERRED_EREG_INT[];

        /**
         * Specifies the error codes of emergency registration which does not supported the
         * common policy.
         *
         * Possible Values:
         *   {@code {423}}
         */
        static const IMS_CHAR KEY_EREG_ERR_CODE_NOT_SUPPORTED_COMMON_POLICY_INT_ARRAY[];

        /**
         * Specifies a list of reject causes for emergency PDN/PDU connection requests.
         * If a request fails with one of these causes, the emergency call should be retried over
         * the cross stack.
         *
         * Possible Values:
         *   {@code {5}}
         */
        static const IMS_CHAR KEY_EPDN_REJECT_CAUSES_FOR_CROSS_STACK_REDIAL_INT_ARRAY[];

        /**
         * Specifies a list of reject causes for network attach requests.
         *
         * If a request fails with one of these causes, the emergency call should be retried
         * over the cross stack.
         *
         * Possible Values:
         *   {@code {3}
         *   {@code 6}
         *   {@code 7}
         *   {@code 8}}
         */
        static const IMS_CHAR KEY_NETWORK_ATTACH_REJECT_CAUSES_FOR_CROSS_STACK_REDIAL_INT_ARRAY[];

        /**
         * Specifies the PLMNs that require UE to release an emergency PDN/PDU after an
         * emergency call ends.
         *
         * Possible Values:
         *   {@code 00101}
         *   {@code 310410}
         */
        static const IMS_CHAR KEY_PLMNS_RELEASE_EPDN_UPON_ECALL_END_IN_FAKE_MODE_STRING_ARRAY[];

        // Mtc
        static const IMS_CHAR KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_ON_CELLULAR_BOOL[];
        static const IMS_CHAR
                KEY_EMERGENCY_RETRY_WITHOUT_CHECKING_380_CONTENT_FOR_NON_UE_DETECTABLE_EMERGENCY_CALL_BOOL
                        [];
        static const IMS_CHAR KEY_EMERGENCY_EXCLUDE_URI_PARAMETERS_FOR_EMERGENCY_TEST_NUMBER_BOOL[];
        static const IMS_CHAR KEY_EMERGENCY_TCALL_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_EMERGENCY_RINGBACK_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_EMERGENCY_18X_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_EMERGENCY_RTT_GUARD_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_EMERGENCY_CALL_INT[];
        static const IMS_CHAR
                KEY_POLICY_FOR_REQUIRING_EMERGENCY_CALL_WHEN_VIDEO_EMERGENCY_CALL_FAILED_INT_ARRAY
                        [];
        static const IMS_CHAR KEY_USE_EMERGENCY_NUMBER_TRANSLATION_IN_ROAMING_STATUS_BOOL[];
        static const IMS_CHAR KEY_BLOCK_WIFI_EMERGENCY_CALL_IF_NOT_PROVISIONED_BOOL[];
        static const IMS_CHAR KEY_WIFI_EMERGENCY_18X_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_RELEASE_EMERGENCY_PDN_ON_FAILURE_AFTER_100_BOOL[];
        static const IMS_CHAR KEY_EMERGENCY_CALL_CURRENT_LOCATION_DISCOVERY_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_P_EMERGENCY_INFO_HEADER_IN_INVITE_STRING[];
        static const IMS_CHAR KEY_CONTACT_HEADER_ADDRESS_IN_INVITE_STRING_ARRAY[];
        static const IMS_CHAR KEY_P_PREFERRED_IDENTITY_INFO_HEADER_IN_INVITE_STRING_ARRAY[];

        static const IMS_CHAR KEY_CALL_PERIODIC_LOCATION_DISCOVERY_METHOD_INT[];
        enum
        {
            CALL_PERIODIC_LOCATION_DISCOVERY_METHOD_NONE = 0,
            CALL_PERIODIC_LOCATION_DISCOVERY_METHOD_UPDATE = 1
        };

        static const IMS_CHAR KEY_CALL_PERIODIC_LOCATION_DISCOVERY_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_REJECT_CODE_AND_REASON_CODE_SET_STRING_ARRAY[];

        static const IMS_CHAR KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY[];
        enum
        {
            START_ERROR_ACTION_SILENT_REINVITE_NEXT_PCSCF_IF_EPDN = 0,
            START_ERROR_ACTION_SILENT_REINVITE_BY_RETRY_AFTER = 1,
            START_ERROR_ACTION_SILENT_REINVITE_VOIP_BY_RTT_REJECTION = 2,
            START_ERROR_ACTION_SILENT_REINVITE_ANONYMOUS = 3,
            START_ERROR_ACTION_CROSS_SIM_TEMP_FAILURE = 4,
            START_ERROR_ACTION_CROSS_SIM_PERM_FAILURE = 5,
            START_ERROR_ACTION_TERMINATE = 6,
            START_ERROR_ACTION_SILENT_REINVITE_TO_ALTERNATE_PCSCF_ONCE = 7,
            START_ERROR_ACTION_SILENT_REINVITE_TO_ALTERNATE_PCSCF = 8
        };

        static const IMS_CHAR
                KEY_REJECT_CODE_AND_REASON_REQUIRE_IMMEDIATE_TERMINATION_STRING_ARRAY[];
        static const IMS_CHAR KEY_POLICY_FOR_EMERGENCY_URN_INT_ARRAY[];
        static const IMS_CHAR KEY_CATEGORY_FOR_GENERIC_URN_INT_ARRAY[];
        static const IMS_CHAR KEY_NUMBER_NEED_OIP_STRING_ARRAY[];
        static const IMS_CHAR KEY_NUMBER_NEED_OIR_STRING_ARRAY[];
        static const IMS_CHAR
                KEY_PLMN_ALLOWING_GEOLOCATION_PIDF_IN_SIP_INVITE_NO_UICC_STRING_ARRAY[];
        static const IMS_CHAR KEY_DYNAMIC_ROUTING_NUMBER_PER_PLMN_STRING_ARRAY[];
        static const IMS_CHAR KEY_EMERGENCY_SERVICE_CATEGORY_PER_PLMN_STRING_ARRAY[];
    };

    class ImsRtt
    {
    public:
        // Public carrier-config - starts
        static const IMS_CHAR KEY_TEXT_ON_DEFAULT_BEARER_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_TEXT_QOS_PRECONDITION_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_TEXT_AS_BANDWIDTH_KBPS_INT[];
        static const IMS_CHAR KEY_TEXT_RS_BANDWIDTH_BPS_INT[];
        static const IMS_CHAR KEY_TEXT_RR_BANDWIDTH_BPS_INT[];
        // Bundle {
        static const IMS_CHAR KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE[];
        static const IMS_CHAR KEY_T140_PAYLOAD_TYPE_INT[];
        static const IMS_CHAR KEY_RED_PAYLOAD_TYPE_INT[];
        // }
        // Public carrier-config - ends
        static const IMS_CHAR KEY_POLICY_ON_TEXT_QOS_DEACTIVATION_INT[];
        static const IMS_CHAR KEY_TEXT_RTCP_INTERVAL_INT_ARRAY[];
        static const IMS_CHAR KEY_TEXT_RTP_PORT_RANGE_INT_ARRAY[];
        static const IMS_CHAR KEY_TEXT_CODEC_EMPTY_REDUNDANT_BOOL[];
        static const IMS_CHAR KEY_TEXT_RTP_DSCP_INT[];
        static const IMS_CHAR KEY_TEXT_CODEC_REDUNDANCY_LEVEL_INT[];
        static const IMS_CHAR KEY_TEXT_RTP_INACTIVITY_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_TEXT_RTCP_INACTIVITY_TIMER_MILLIS_INT[];
        /**
         * Specifies whether to check if local resource for text media is reserved after
         * the call is established or updated.
         *
         * If {@code true} and text QoS hasn't activated, the next action will be determined
         * by {@code imsrtt.policy_on_text_qos_deactivation_int} - usually downgrade.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_CHECK_LOCAL_RESOURCE_AFTER_ESTABLISHED_OR_MODIFIED_BOOL[];
    };

    class ImsSms
    {
    public:
        // Public carrier-config - starts
        static const IMS_CHAR KEY_SMS_OVER_IMS_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_SMS_CSFB_RETRY_ON_FAILURE_BOOL[];

        static const IMS_CHAR KEY_SMS_OVER_IMS_FORMAT_INT[];
        enum
        {
            SMS_FORMAT_3GPP = 0,
            SMS_FORMAT_3GPP2 = 1
        };

        static const IMS_CHAR KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY[];
        // Public carrier-config - ends

        // Aos
        /**
         * Specifies whether registration mode to limited mode when ISIM is inactivated.
         *
         * Only sending admin SMS and receiving admin/normal SMS is allowed in the
         * limited mode.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_SUPPORT_LIMITED_ADMIN_SMS_MODE_BOOL[];

        /**
         * Specifies whether SMS over IMS is supported without voice capability.
         *
         * If{@code true}: SMS over IMS is supported regardless of voice
         * capability. If{@code false}: SMS over IMS is not supported if voice is not
         * capable.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_SMS_OVER_IMS_AVAILABLE_WITHOUT_VOICE_CAPA_BOOL[];

        // Mts
        static const IMS_CHAR KEY_SMS_PREFERRED_PSI_URI_TYPE_INT[];
        static const IMS_CHAR KEY_SMS_USE_DIALED_NUMBER_FOR_REQUEST_URI_BOOL[];
        static const IMS_CHAR KEY_SMS_ALLOW_IMSI_BASED_SIP_URI_BOOL[];
        static const IMS_CHAR KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY[];
        static const IMS_CHAR KEY_SMS_FALLBACK_ERROR_CODES_INT_ARRAY[];
        static const IMS_CHAR KEY_SMS_SMMA_GENERIC_ERROR_CODES_INT_ARRAY[];
        static const IMS_CHAR KEY_SMS_REG_POLICY_FOR_EXPIRY_TIMER_F_INT[];
        static const IMS_CHAR KEY_SMS_REG_POLICY_FOR_403_RESPONSE_INT[];
        static const IMS_CHAR KEY_SMS_REG_POLICY_FOR_404_RESPONSE_INT[];
        static const IMS_CHAR KEY_SMS_REG_POLICY_FOR_406_RESPONSE_INT[];
        static const IMS_CHAR KEY_SMS_REG_POLICY_FOR_408_RESPONSE_INT[];
        static const IMS_CHAR KEY_SMS_REG_POLICY_FOR_500_RESPONSE_INT[];
        static const IMS_CHAR KEY_SMS_REG_POLICY_FOR_503_RESPONSE_INT[];
        static const IMS_CHAR KEY_SMS_REG_POLICY_FOR_504_RESPONSE_INT[];
        static const IMS_CHAR KEY_SMS_GEOLOCATION_PIDF_IN_SIP_MESSAGE_SUPPORT_INT_ARRAY[];
        static const IMS_CHAR KEY_SMS_RETRY_AFTER_MAX_TIME_SEC_INT[];
        static const IMS_CHAR KEY_SMS_RETRY_AFTER_MAX_COUNT_INT[];
        static const IMS_CHAR KEY_SMS_REPORT_GENERIC_ERROR_WHEN_RETRY_AFTER_NOT_POSSIBLE_BOOL[];
        static const IMS_CHAR KEY_SMS_MESSAGE_RESPONSE_WAIT_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_SMS_RETRY_POLICY_FOR_EXPIRY_TIMER_F_INT[];
        static const IMS_CHAR KEY_SMS_IN_REPLY_TO_VALIDATION_BOOL[];
        static const IMS_CHAR KEY_SMS_SUPPORT_CONTENT_TRANSFER_ENCODING_HEADER_BOOL[];
        static const IMS_CHAR KEY_SMS_EVALUATE_RADIO_STATUS_FOR_3RD_ATTEMPT_BOOL[];
        static const IMS_CHAR KEY_SMS_ERROR_CODE_WHEN_MT_SMS_BLOCKED_INT[];
        static const IMS_CHAR KEY_SMS_EVALUATE_RADIO_STATUS_FOR_ERROR_CODES_INT_ARRAY[];
        static const IMS_CHAR KEY_SMS_EVALUATE_RADIO_STATUS_FOR_RP_ERROR_CAUSES_INT_ARRAY[];
    };

    class ImsUce
    {
    public:
        static const IMS_CHAR KEY_EXPIRE_VALUE_PUBLISH_SEC_INT[];
        static const IMS_CHAR KEY_EXTENDED_EXPIRE_VALUE_PUBLISH_SEC_INT[];
        static const IMS_CHAR KEY_PUBLISH_REFRESH_RATIO_INT[];
        static const IMS_CHAR KEY_EXPIRE_VALUE_LIST_SUBSCRIBE_SEC_INT[];
        static const IMS_CHAR KEY_RLS_URI_STRING[];
        static const IMS_CHAR KEY_SUBSCRIBE_INDEPENDENT_OF_PUBLISH_BOOL[];
        static const IMS_CHAR KEY_ANONYMOUS_FETCH_METHOD_INT[];
        static const IMS_CHAR KEY_ENCODE_PUBLISH_BODY_BOOL[];
        static const IMS_CHAR KEY_ENCODE_SUBSCRIBE_BODY_BOOL[];
        static const IMS_CHAR KEY_SUPPORT_OPTIONS_BOOL[];
        static const IMS_CHAR KEY_USE_SIP_URI_FOR_PRESENCE_SUBSCRIBE_BOOL[];
        static const IMS_CHAR KEY_USE_EXPIRED_ETAG_BOOL[];
        static const IMS_CHAR KEY_USE_CONTACT_HEADER_IN_PUBLISH_BOOL[];
        static const IMS_CHAR KEY_USE_CONTACT_HEADER_IN_SUBSCRIBE_BOOL[];
        static const IMS_CHAR KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE_INT_ARRAY[];
        static const IMS_CHAR KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE_MAX_COUNT_INT[];
        static const IMS_CHAR KEY_FIXED_TIME_RETRY_PUBLISH_RESPONSE_INT_ARRAY[];
        static const IMS_CHAR KEY_FIXED_TIME_RETRY_PUBLISH_RESPONSE_MAX_COUNT_INT[];
        static const IMS_CHAR KEY_FIXED_TIME_RETRY_PUBLISH_RESPONSE_TIME_SEC_INT[];
        static const IMS_CHAR KEY_VARIABLE_TIME_RETRY_PUBLISH_RESPONSE_INT_ARRAY[];
        static const IMS_CHAR KEY_VARIABLE_TIME_RETRY_PUBLISH_RESPONSE_MAX_COUNT_INT[];
        static const IMS_CHAR KEY_VARIABLE_TIME_RETRY_PUBLISH_RESPONSE_TIME_SEC_INT_ARRAY[];
        static const IMS_CHAR KEY_REATTEMPT_REGISTRATION_PUBLISH_RESPONSE_INT_ARRAY[];
        static const IMS_CHAR KEY_REATTEMPT_REGISTRATION_SUBSCRIBE_RESPONSE_INT_ARRAY[];
        static const IMS_CHAR KEY_ADD_VIDEO_TAG_CONTACT_HEADER_IN_PUBLISH_BOOL[];
    };

    class ImsVoice
    {
    public:
        // Public carrier-config - starts
        static const IMS_CHAR KEY_CARRIER_VOLTE_ROAMING_AVAILABLE_BOOL[];
        static const IMS_CHAR KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL[];
        static const IMS_CHAR KEY_MULTIENDPOINT_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_SESSION_TIMER_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_SESSION_EXPIRES_TIMER_SEC_INT[];
        static const IMS_CHAR KEY_MINIMUM_SESSION_EXPIRES_TIMER_SEC_INT[];

        static const IMS_CHAR KEY_SESSION_REFRESHER_TYPE_INT[];
        enum
        {
            SESSION_REFRESHER_TYPE_UNKNOWN = 0,
            SESSION_REFRESHER_TYPE_UAC = 1,
            SESSION_REFRESHER_TYPE_UAS = 2
        };

        static const IMS_CHAR KEY_SESSION_PRIVACY_TYPE_INT[];
        enum
        {
            SESSION_PRIVACY_TYPE_HEADER = 0,
            SESSION_PRIVACY_TYPE_NONE = 1,
            SESSION_PRIVACY_TYPE_ID = 2
        };

        static const IMS_CHAR KEY_PRACK_SUPPORTED_FOR_18X_BOOL[];

        static const IMS_CHAR KEY_CONFERENCE_SUBSCRIBE_TYPE_INT[];
        enum
        {
            CONFERENCE_SUBSCRIBE_NOT_SUPPORT = -1,
            CONFERENCE_SUBSCRIBE_TYPE_IN_DIALOG = 0,
            CONFERENCE_SUBSCRIBE_TYPE_OUT_OF_DIALOG = 1
        };

        static const IMS_CHAR KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT[];

        static const IMS_CHAR KEY_SRVCC_TYPE_INT_ARRAY[];
        enum
        {
            BASIC_SRVCC_SUPPORT = 0,
            ALERTING_SRVCC_SUPPORT = 1,
            PREALERTING_SRVCC_SUPPORT = 2,
            MIDCALL_SRVCC_SUPPORT = 3
        };

        static const IMS_CHAR KEY_RINGING_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_RINGBACK_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_AUDIO_RTP_INACTIVITY_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_AUDIO_RTCP_INACTIVITY_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_CONFERENCE_FACTORY_URI_STRING[];

        static const IMS_CHAR KEY_SESSION_REFRESH_METHOD_INT[];
        enum
        {
            SESSION_REFRESH_METHOD_INVITE = 0,
            SESSION_REFRESH_METHOD_UPDATE_PREFERRED = 1
        };

        static const IMS_CHAR KEY_OIP_SOURCE_FROM_HEADER_BOOL[];
        static const IMS_CHAR KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT[];

        static const IMS_CHAR KEY_AUDIO_INACTIVITY_CALL_END_REASONS_INT_ARRAY[];
        enum
        {
            RTCP_INACTIVITY_ON_HOLD = 0,
            RTCP_INACTIVITY_ON_CONNECTED = 1,
            RTP_INACTIVITY_ON_CONNECTED = 2,
            E911_RTCP_INACTIVITY_ON_CONNECTED = 3,
            E911_RTP_INACTIVITY_ON_CONNECTED = 4
        };

        static const IMS_CHAR KEY_AUDIO_AS_BANDWIDTH_KBPS_INT[];
        static const IMS_CHAR KEY_AUDIO_RS_BANDWIDTH_BPS_INT[];
        static const IMS_CHAR KEY_AUDIO_RR_BANDWIDTH_BPS_INT[];
        // Bundle {
        static const IMS_CHAR KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE[];
        static const IMS_CHAR KEY_EVS_PAYLOAD_TYPE_INT_ARRAY[];
        static const IMS_CHAR KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY[];
        static const IMS_CHAR KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY[];
        static const IMS_CHAR KEY_DTMFWB_PAYLOAD_TYPE_INT_ARRAY[];
        static const IMS_CHAR KEY_DTMFNB_PAYLOAD_TYPE_INT_ARRAY[];
        // }
        // Bundle {
        static const IMS_CHAR KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE[];
        static const IMS_CHAR KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE[];
        static const IMS_CHAR KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT[];
        static const IMS_CHAR KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY[];
        // }
        // Bundle {
        static const IMS_CHAR KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE[];
        static const IMS_CHAR KEY_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT[];
        static const IMS_CHAR KEY_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT[];
        static const IMS_CHAR KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY[];
        static const IMS_CHAR KEY_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT[];
        static const IMS_CHAR KEY_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT[];
        static const IMS_CHAR KEY_EVS_CODEC_ATTRIBUTE_DTX_BOOL[];
        static const IMS_CHAR KEY_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL[];
        static const IMS_CHAR KEY_EVS_CODEC_ATTRIBUTE_CHANNELS_INT[];
        static const IMS_CHAR KEY_EVS_CODEC_ATTRIBUTE_CMR_INT[];
        static const IMS_CHAR KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT[];
        static const IMS_CHAR KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT[];
        static const IMS_CHAR KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT[];
        // }
        // Public carrier-config - ends

        static const IMS_CHAR KEY_POLICY_FOR_SDP_PREVIEW_MODE_INT[];
        enum
        {
            SDP_PREVIEW_MODE_DISABLED = 0,
            SDP_PREVIEW_MODE_FOR_NORMAL_CALL_ONLY = 1,
            SDP_PREVIEW_MODE_FOR_EMERGENCY_CALL_ONLY = 2,
            SDP_PREVIEW_MODE_FOR_ALL_CALLS = 3,
        };

        static const IMS_CHAR KEY_IGNORE_SUBSEQUENT_SDP_ANSWER_IN_PREVIEW_MODE_BOOL[];

        // Aos
        /**
         * Specifies whether VoPS is not considered for VoLTE availability.
         *
         * Possible Values:
         *   {@code true} : VoLTE can be enabled regardless of VoPS.
         *   {@code false} : VoLTE can be enabled only when VoPS is supported.
         */
        static const IMS_CHAR KEY_IGNORE_VOPS_FOR_VOLTE_ENABLE_BOOL[];

        /**
         * Specifies if UE blocks current plmn when voice call is unavailable.
         *
         * This config is only applicable for VoPS NOT SUPPORTED and SSAC barring
         * cases.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_PLMN_BLOCK_WITH_TIMEOUT_ON_VOICE_CALL_UNAVAILABLE_BOOL[];

        /**
         * Specifies if VoLTE service is blocked by SSAC.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_REQUIRED_VOLTE_BLOCK_BY_SSAC_BOOL[];

        /**
         * Specifies delay time for re-registration when VoLTE service is get available due to
         * the change of VoPS or SSAC.
         *
         * Possible Values:
         *   {@code 60}
         */
        static const IMS_CHAR KEY_VOLTE_HYS_TIME_SEC_INT[];
        // Mtc
        static const IMS_CHAR KEY_18X_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_REQUIRE_PRACK_FOR_ALERT_BOOL[];
        static const IMS_CHAR KEY_FORCE_183_BEFORE_ALERTING_ON_NON_100REL_INVITE_BOOL[];
        static const IMS_CHAR KEY_SUPPORT_CONFERENCE_REFER_SUBSCRIBE_BOOL[];
        static const IMS_CHAR KEY_ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT_BOOL[];

        static const IMS_CHAR KEY_CONFERENCE_SIP_FLOW_ORDER_INT[];
        enum
        {
            CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_REFER = 0,
            CONFERENCE_SIP_FLOW_REFER_AND_SUBSCRIBE = 1,
            CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_NOTIFY_REFER = 2
        };

        static const IMS_CHAR KEY_CONFERENCE_INVITING_REFER_TYPE_INT[];
        enum
        {
            CONFERENCE_INVITE_COPYCONTROL = 0,
            CONFERENCE_INVITE_REFER_SINGLE = 1,
            CONFERENCE_INVITE_REFER_MULTIPLE = 2
        };

        static const IMS_CHAR KEY_DISABLE_PRECONDITION_AFTER_CALL_ESTABLISHED_BOOL[];
        static const IMS_CHAR KEY_INCOMING_CALL_REJECT_CODE_FOR_USER_DECLINE_INT[];
        static const IMS_CHAR KEY_INCOMING_CALL_REJECT_CODE_FOR_NO_ANSWER_INT[];
        static const IMS_CHAR KEY_PRACK_WAIT_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_PRACK_UPDATE_RESPONSE_WAIT_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_SESSION_REFRESH_TRIGGER_INTERVAL_SEC_INT[];

        static const IMS_CHAR KEY_REGISTRATION_RESTORATION_MODE_ON_504_FOR_INVITE_INT[];
        enum
        {
            REGISTRATION_RESTORATION_NOT_APPLICABLE = 0,
            REGISTRATION_RESTORATION_NEXT_PCSCF = 1,
            REGISTRATION_RESTORATION_SAME_PCSCF = 2,
            REGISTRATION_RESTORATION_NEXT_PCSCF_BY_NETWORK_CONTEXT = 3,
            REGISTRATION_RESTORATION_NEXT_PCSCF_WITH_SILENT_REDIAL = 4
        };

        static const IMS_CHAR
                KEY_REGISTRATION_RESTORATION_FOR_INVITE_REQUIRE_HEADER_VALIDATION_BOOL[];
        static const IMS_CHAR KEY_RELEASE_CALL_ON_DEDICATED_BEARER_WAIT_TIMEOUT_BOOL[];

        static const IMS_CHAR KEY_POLICY_ON_AUDIO_QOS_DEACTIVATION_INT[];
        enum
        {
            QOS_DEACTIVATION_POLICY_TERMINATE_CALL = 0,
            QOS_DEACTIVATION_POLICY_MAINTAIN_CALL = 1,
            QOS_DEACTIVATION_POLICY_MODIFY_CALL = 2
        };

        static const IMS_CHAR KEY_ENABLE_SEND_REINVITE_ON_RAT_CHANGE_BOOL[];
        static const IMS_CHAR KEY_ALLOW_SDP_IN_PRACK_BOOL[];

        static const IMS_CHAR KEY_POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_INT[];
        static const IMS_CHAR KEY_POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_IN_ROAMING_INT[];
        enum
        {
            MEDIA_TYPE_RESTRICTION_POLICY_RESTRICT_NONE = 0,
            MEDIA_TYPE_RESTRICTION_POLICY_RESTRICT_AUDIO = 1,
            MEDIA_TYPE_RESTRICTION_POLICY_RESTRICT_AUDIO_AND_VIDEO = 2
        };

        static const IMS_CHAR KEY_POLICY_OF_LOCAL_NUMBERS_INT[];
        enum
        {
            NUMBER_FORMAT_HOME_LOCAL = 0,
            NUMBER_FORMAT_GEO_LOCAL = 1,
            NUMBER_FORMAT_GEO_LOCAL_ONLY_IN_ROAMING = 2
        };

        static const IMS_CHAR KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT[];
        static const IMS_CHAR KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT[];
        static const IMS_CHAR KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT[];

        static const IMS_CHAR KEY_SILENT_REDIAL_ULTIMATE_FAILURE_ACTION_INT[];
        enum
        {
            SILENT_REDIAL_FAILURE_ACTION_TERMINATE = 0,
            SILENT_REDIAL_FAILURE_ACTION_REGISTRATION = 1,
            SILENT_REDIAL_FAILURE_ACTION_CSFB = 2
        };

        static const IMS_CHAR KEY_SILENT_REDIAL_REGISTRATION_WAIT_TIME_MILLIS_INT[];

        static const IMS_CHAR KEY_CALL_TYPE_AFTER_AUDIO_AND_VIDEO_CALL_MERGED_INT[];
        enum
        {
            CALL_MERGE_AS_AUDIO = 0,
            CALL_MERGE_AS_AUDIO_VIDEO = 1
        };

        static const IMS_CHAR KEY_CALLER_ID_SERVICE_CODES_FOR_RESTRICTION_STRING_ARRAY[];
        static const IMS_CHAR KEY_CALLER_ID_SERVICE_CODES_FOR_IDENTITY_STRING_ARRAY[];
        static const IMS_CHAR KEY_SHORT_CALL_CODE_INT_ARRAY[];
        static const IMS_CHAR KEY_LOCAL_NUMBER_PRESENTATION_SET_STRING[];
        static const IMS_CHAR KEY_ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL_BOOL[];
        static const IMS_CHAR KEY_REJECT_CODE_AND_REASON_CODE_SET_STRING_ARRAY[];

        static const IMS_CHAR KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY[];
        enum
        {
            START_ERROR_ACTION_CSFB = 0,
            START_ERROR_ACTION_SILENT_REINVITE = 1,
            START_ERROR_ACTION_SILENT_REINVITE_BY_SDP_CONTENT = 2,
            START_ERROR_ACTION_SILENT_REINVITE_BY_RETRY_AFTER = 3,
            START_ERROR_ACTION_REGISTRATION_RESTORATION_ON_IMS3GPP_BY_POLICY = 4,
            START_ERROR_ACTION_REDIRECTION_BY_CONTACT = 5,
            START_ERROR_ACTION_NON_UE_DETECTABLE_EMERGENCY_CALL = 6,
            START_ERROR_ACTION_HANDLE_FORBIDDEN_BY_POLICY = 7,
            START_ERROR_ACTION_TERMINATE_BY_REASON_PHRASE = 8,
            START_ERROR_ACTION_USSI_CSFB = 9,
            START_ERROR_ACTION_BLOCK_CALL_BY_TIMER = 10,
            START_ERROR_ACTION_TRIGGER_EPSFB = 11,
            START_ERROR_ACTION_TERMINATE_BY_RESPONSE_SOURCE = 12,
            START_ERROR_ACTION_TERMINATE_BY_REASON_HEADER_TEXT = 13,
            START_ERROR_ACTION_REGISTRATION_TO_ALTERNATE_PCSCF = 14,
            START_ERROR_ACTION_SILENT_REINVITE_TO_ALTERNATE_PCSCF = 15,
            START_ERROR_ACTION_SILENT_REINVITE_TO_ALTERNATE_PCSCF_ONCE = 16,
            START_ERROR_ACTION_SILENT_REINVITE_WITH_AUDIO = 17
        };

        static const IMS_CHAR KEY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY[];
        enum
        {
            UPDATE_ERROR_ACTION_TERMINATE = 0,
            UPDATE_ERROR_ACTION_RETRY = 1,
            UPDATE_ERROR_ACTION_GLARE_CONDITION = 2,
            UPDATE_ERROR_ACTION_BLOCK_CALL_BY_TIMER = 3
        };

        static const IMS_CHAR KEY_EARLY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY[];
        enum
        {
            EARLY_UPDATE_ERROR_ACTION_TERMINATE_DIALOG = 0,
            EARLY_UPDATE_ERROR_ACTION_TERMINATE_CALL = 1,
            EARLY_UPDATE_ERROR_ACTION_GLARE_CONDITION = 2,
            EARLY_UPDATE_ERROR_ACTION_BLOCK_CALL_BY_TIMER = 3,
            EARLY_UPDATE_ERROR_ACTION_TIMEOUT = 4,
            EARLY_UPDATE_ERROR_ACTION_REGISTRATION_RESTORATION = 5
        };

        static const IMS_CHAR KEY_CSFB_BLOCK_CONDITION_INT_ARRAY[];
        enum
        {
            CSFB_BLOCK_CONDITION_IF_EPS_ONLY_ATTACH = 0,
            CSFB_BLOCK_CONDITION_IN_NR = 1,
            CSFB_BLOCK_CONDITION_IN_WIFI = 2,
            CSFB_BLOCK_CONDITION_IN_ROAMING = 3,
            CSFB_BLOCK_CONDITION_IN_HOME = 4
        };

        static const IMS_CHAR KEY_CSFB_WHEN_ALL_PCSCF_UNAVAILABLE_BOOL[];

        static const IMS_CHAR KEY_POLICY_FOR_403_RESPONSE_FOR_INVITE_INT[];
        enum
        {
            SIP_403_POLICY_TERMINATE_CALL = 0,
            SIP_403_POLICY_TERMINATE_CALL_AND_RECOVER_REGISTRATION = 1,
            SIP_403_POLICY_TERMINATE_CALL_AND_REFRESH_REGISTRATION = 2,
            SIP_403_POLICY_CSFB = 3,
            SIP_403_POLICY_CSFB_AND_RECOVER_REGISTRATION = 4,
            SIP_403_POLICY_CSFB_AND_RECOVER_REGISTRATION_BY_WARNING = 5
        };

        static const IMS_CHAR KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT[];
        enum
        {
            QOS_CHECK_POLICY_ON_UPGRADING_CALL_NOT_AVAILABLE = 0,
            QOS_CHECK_POLICY_ON_UPGRADING_CALL_AFTER_UPGRADE = 1,
            QOS_CHECK_POLICY_ON_UPGRADING_CALL_DURING_UPGRADING = 2
        };

        static const IMS_CHAR KEY_WAIT_QOS_WHEN_LOCAL_PRECONDITION_NOT_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_WAIT_QOS_FOR_INCOMING_INVITE_WITHOUT_PRECONDITION_BOOL[];
        static const IMS_CHAR KEY_REJECT_OFFERLESS_INVITE_BOOL[];
        static const IMS_CHAR KEY_CALL_MAX_COUNT_INT[];
        static const IMS_CHAR KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING[];
        static const IMS_CHAR KEY_CALL_TERMINATE_REASON_HEADER_RTP_TIMEOUT_STRING[];
        static const IMS_CHAR KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_AND_RTP_TIMEOUT_STRING[];
        static const IMS_CHAR KEY_CALL_TERMINATE_REASON_HEADER_MEDIA_BEARER_LOSS_STRING[];
        static const IMS_CHAR KEY_CALL_TERMINATE_REASON_HEADER_SIP_TIMEOUT_STRING[];
        static const IMS_CHAR KEY_CALL_TERMINATE_REASON_HEADER_SIP_RESPONSE_TIMEOUT_STRING[];
        static const IMS_CHAR
                KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_AND_SIP_RESPONSE_TIMEOUT_STRING[];
        static const IMS_CHAR KEY_CALL_TERMINATE_REASON_HEADER_CALL_SETUP_TIMEOUT_STRING[];
        static const IMS_CHAR KEY_CALL_TERMINATE_REASON_HEADER_TERMINATING_EARLYDIALOG_STRING[];
        static const IMS_CHAR KEY_CALL_TERMINATE_REASON_HEADER_SESSION_REFRESH_FAILURE_STRING[];
        static const IMS_CHAR KEY_CALL_TERMINATE_REASON_HEADER_CONFERENCE_CALL_JOINED_STRING[];
        static const IMS_CHAR KEY_CALL_TERMINATE_REASON_HEADER_NETWORK_LOST_STRING[];
        static const IMS_CHAR KEY_CALL_TERMINATE_REASON_HEADER_MEDIA_NOT_SUPPORTED_STRING[];
        static const IMS_CHAR KEY_CALL_TERMINATE_REASON_HEADER_MEDIA_BEARER_NOT_MET_STRING[];

        static const IMS_CHAR KEY_CALL_REJECT_REASON_PHRASE_ON_CSCALL_STRING[];
        static const IMS_CHAR KEY_CALL_REJECT_REASON_PHRASE_ON_VILTE_AND_NO_LTE_STRING[];
        static const IMS_CHAR KEY_CALL_REJECT_REASON_PHRASE_ON_CONNECTING_CALL_STRING[];
        static const IMS_CHAR KEY_CALL_REJECT_REASON_PHRASE_EXCEEDS_MAX_CALL_COUNT_STRING[];
        static const IMS_CHAR KEY_CALL_REJECT_REASON_PHRASE_ON_CONVERTING_STRING[];
        static const IMS_CHAR KEY_CALL_REJECT_REASON_PHRASE_NEGOTIATION_FAILURE_STRING[];
        static const IMS_CHAR KEY_CALL_REJECT_REASON_PHRASE_NO_ANSWER_BY_USER_STRING[];
        static const IMS_CHAR KEY_CALL_REJECT_REASON_PHRASE_VOWIFI_OFF_STRING[];
        static const IMS_CHAR KEY_CALL_REJECT_REASON_PHRASE_USER_REJECT_STRING[];
        static const IMS_CHAR KEY_CALL_REJECT_REASON_PHRASE_ACCESS_CLASS_BLOCKED_STRING[];
        static const IMS_CHAR KEY_CALL_REJECT_REASON_PHRASE_VOPS_OFF_STRING[];
        static const IMS_CHAR KEY_ENABLE_OIP_HEADER_POLICY_FALLBACK_BOOL[];
        static const IMS_CHAR KEY_DELAY_UPDATE_AFTER_CONNECTED_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_CHECK_CONFERENCE_EVENT_PACKAGE_VERSION_BOOL[];
        static const IMS_CHAR KEY_CONFERENCE_REFER_TO_URI_SOURCE_PAID_BOOL[];

        static const IMS_CHAR KEY_CONFERENCE_DROP_REFER_TO_URI_SOURCE_TYPE_INT[];
        enum
        {
            CONFERENCE_DROP_REFER_TO_URI_SOURCE_REFER_TO_URI_FOR_INVITE = 0,
            CONFERENCE_DROP_REFER_TO_URI_SOURCE_USER_ENTITY_IN_CONFERENCE_EVENT_PACKAGE = 1
        };

        static const IMS_CHAR KEY_MEDIA_TYPE_FOR_OFFERLESS_INVITE_INT[];
        enum
        {
            OFFERLESS_INVITE_MEDIA_TYPE_FULL_CAPABILITY = 0,
            OFFERLESS_INVITE_MEDIA_TYPE_AUDIO = 1
        };

        static const IMS_CHAR KEY_MEDIA_TYPE_FOR_OFFERLESS_REINVITE_INT[];
        enum
        {
            OFFERLESS_REINVITE_MEDIA_TYPE_FULL = 0,
            OFFERLESS_REINVITE_MEDIA_TYPE_AUDIO = 1,
            OFFERLESS_REINVITE_MEDIA_TYPE_CURRENT = 2,
            OFFERLESS_REINVITE_MEDIA_TYPE_BY_HISTORY = 3,
            OFFERLESS_REINVITE_MEDIA_TYPE_INITIALLY_OFFERED = 4,
        };

        static const IMS_CHAR KEY_OIP_TYPE_FOR_UNAVAILABLE_INT[];
        static const IMS_CHAR KEY_PREALERTING_TIMER_MILLIS_INT[];

        static const IMS_CHAR KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL_INT[];
        enum
        {
            MO_CALL_REQUEST_TIMEOUT_POLICY_CALL_END = 0,
            MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_PCSCF_DISCOVERY_AFTER_CSFB = 1,
            MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB = 2,
            MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_AFTER_CSFB_IF_AVAILBLE = 3,
            MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_CURRENT_PCSCF = 4,
            MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_NEXT_PCSCF = 5,
            MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_WITH_PDN_RECONNECT_AFTER_CSFB = 6,
            MO_CALL_REQUEST_TIMEOUT_POLICY_SILENT_REDIAL = 7,
            MO_CALL_REQUEST_TIMEOUT_POLICY_SILENT_REDIAL_WITH_INITIAL_REGISTER_PCSCF_DISCOVERY = 8
        };

        static const IMS_CHAR KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY[];
        static const IMS_CHAR
                KEY_SUPPORT_REGISTRATION_RECOVERY_FOR_FAILURE_OF_SESSION_REFRESH_BOOL[];

        static const IMS_CHAR KEY_POLICY_FOR_CALL_MAINTAINING_ON_REGISTRATION_SUSPENDED_INT_ARRAY[];
        enum
        {
            REGISTRATION_SUSPENDED_POLICY_MAINTAIN_NONE = 0,
            REGISTRATION_SUSPENDED_POLICY_MAINTAIN_AUDIO_ONLY = 1,
            REGISTRATION_SUSPENDED_POLICY_MAINTAIN_ALL = 2
        };

        static const IMS_CHAR KEY_USE_LTE_PREFERRED_STATUS_FOR_SERVICE_CAPABILITY_BOOL[];
        static const IMS_CHAR KEY_ALLOW_INCOMING_HOLD_REQUEST_DURING_CONFERENCE_CALL_BOOL[];
        static const IMS_CHAR KEY_IGNORE_180_AFTER_183_RESPONSE_BOOL[];
        static const IMS_CHAR KEY_ADD_REPLACE_HEADER_FOR_CONFERENCE_BOOL[];

        static const IMS_CHAR KEY_POLICY_FOR_PRACK_DELIVERY_FAILURE_INT[];
        enum
        {
            PRACK_DELIVERY_FAILURE_POLICY_TERMINATE_DIALOG = 0,
            PRACK_DELIVERY_FAILURE_POLICY_TERMINATE_CALL = 1,
            PRACK_DELIVERY_FAILURE_POLICY_IGNORE = 2,
        };

        static const IMS_CHAR KEY_USE_CARRIER_SPECIFIC_CONTACT_HEADER_FOR_OPTIONS_RESPONSE_BOOL[];
        static const IMS_CHAR
                KEY_USE_CARRIER_SPECIFIC_REJECT_PHRASE_FOR_INCOMING_CALL_DURING_NO_REGISTRATION_BOOL
                        [];
        static const IMS_CHAR KEY_ENABLE_REGISTRATION_RECOVERY_ON_PRACK_TIMEOUT_BOOL[];
        static const IMS_CHAR
                KEY_ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_REJECTED_BY_SERVER_ERROR_BOOL[];
        static const IMS_CHAR KEY_ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_RETRY_UNAVAILABLE_BOOL[];
        static const IMS_CHAR KEY_ENABLE_REGISTRATION_RECOVERY_WHEN_BYE_TRANSACTION_TIMEOUT_BOOL[];
        static const IMS_CHAR KEY_CHECK_SERVER_OUTAGE_REASON_FOR_VXLTE_CALL_BOOL[];
        static const IMS_CHAR KEY_MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING_BOOL[];
        static const IMS_CHAR KEY_DESTROY_INACTIVE_EARLY_SESSIONS_WHEN_ESTABLISHED_BOOL[];
        static const IMS_CHAR KEY_STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY_BOOL[];
        static const IMS_CHAR KEY_MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF_INT_ARRAY[];

        static const IMS_CHAR KEY_GEOLOCATION_BLOCK_CONDITION_INT_ARRAY[];
        enum
        {
            GEOLOCATION_BLOCK_CONDITION_IN_ROAMING = 0,
            GEOLOCATION_BLOCK_CONDITION_FOR_NORMAL_ROUTING_EMERGENCY_CALL = 1,
        };

        static const IMS_CHAR KEY_INCOMING_RESUME_EVENT_SUPPORT_BOOL[];
        static const IMS_CHAR KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT[];

        /**
         * Specifies whether to initialize P-Early-Media type to 'inactive' when the header is
         * missing in the received message and no P-Early-Media type has been set yet.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_SET_INACTIVE_P_EARLY_MEDIA_WHEN_NO_HEADER_BOOL[];

        static const IMS_CHAR KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT[];
        enum
        {
            DYNAMIC_NW_TONE_WHEN_PEM_NOT_CONTAINS_SEND = 0,
            DYNAMIC_NW_TONE_WHEN_PEM_ALL = 1,
            DYNAMIC_NW_TONE_WHEN_PEM_CONTAINS_SEND = 2,
            NW_TONE_WHEN_PEM_CONTAINS_SEND_AFTER_180 = 3,
        };

        static const IMS_CHAR KEY_USER_CANCEL_REASON_AFTER_RESPONSE_TIMEOUT_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT[];
        static const IMS_CHAR KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT[];
        static const IMS_CHAR KEY_EPS_FALLBACK_TRIGGER_BY_RRC_REJECT_WAIT_TIME_BOOL[];
        static const IMS_CHAR KEY_EPS_FALLBACK_TRIGGER_BY_AC_BARRING_BOOL[];
        static const IMS_CHAR
                KEY_REQUIRE_REGISTRATION_AFTER_EPS_FALLBACK_TRIGGER_FOR_SILENT_REDIAL_BOOL[];
        static const IMS_CHAR KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT[];
        static const IMS_CHAR KEY_CALL_REJECT_CODE_FOR_NOT_ACCEPTABLE_CALL_TYPE_INT[];
        static const IMS_CHAR KEY_CHECK_UI_CONDITION_FOR_INCOMING_RESUME_BOOL[];
        static const IMS_CHAR KEY_REGISTRATION_TO_18X_TIMER_MILLIS_INT_ARRAY[];
        static const IMS_CHAR KEY_CALL_INITIATION_TO_18X_TIMER_MILLIS_INT_ARRAY[];
        static const IMS_CHAR KEY_QOS_ACQUISITION_AFTER_W2L_HANDOVER_WAIT_TIMER_MILLIS_INT[];
        static const IMS_CHAR
                KEY_WAIT_VIDEO_TEXT_QOS_AFTER_AUDIO_QOS_ACQUISITION_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_QOS_LOST_GUARD_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_QOS_FORCED_ACQUISITION_TIMER_MILLIS_INT[];

        static const IMS_CHAR
                KEY_RAT_CONDITION_FOR_NOT_WAITING_DEDICATED_BEARER_BEFORE_ESTABLISHED_INT_ARRAY[];
        enum
        {
            NO_WAIT_DEDICATED_BEARER_IN_NR = 0,
            NO_WAIT_DEDICATED_BEARER_IN_EPS_FALLBACK = 1,
            NO_WAIT_DEDICATED_BEARER_IN_EPS_ONLY_ATTACH = 2,
        };

        static const IMS_CHAR
                KEY_TRIGGER_DEDICATED_BEARER_WAIT_TIMER_BY_SENDING_INITIAL_INVITE_BOOL[];
        static const IMS_CHAR KEY_RESTART_DEDICATED_BEARER_WAIT_TIMER_BY_EPS_FALLBACK_BOOL[];
        static const IMS_CHAR KEY_RESTART_RINGING_TIMER_BY_SENDING_180_BOOL[];
        static const IMS_CHAR KEY_CONTENT_ID_FOR_GEOLOCATION_STRING[];
        static const IMS_CHAR KEY_ENRICH_CALLREASONINFO_WITH_REASON_HEADER_BOOL[];

        static const IMS_CHAR KEY_POLICY_FOR_MULTIPLE_P_ASSERTED_IDENTITY_HEADERS_INT[];
        enum
        {
            PAI_POLICY_PREFER_TOPMOST = 0,
            PAI_POLICY_PREFER_SIP_URI = 1,
        };

        // Media
        static const IMS_CHAR KEY_AUDIO_JITTER_BUFFER_SIZE_INT_ARRAY[];
        static const IMS_CHAR KEY_AUDIO_RTCP_INTERVAL_INT_ARRAY[];
        static const IMS_CHAR KEY_EVS_AMRWB_IO_MODE_SET_INT[];
        static const IMS_CHAR KEY_AUDIO_RTP_PORT_RANGE_INT_ARRAY[];
        static const IMS_CHAR KEY_AUDIO_BW_NEGO_OPTION_BOOL[];
        static const IMS_CHAR KEY_AUDIO_PTIME_MILLIS_INT[];
        static const IMS_CHAR KEY_AUDIO_MAXPTIME_MILLIS_INT[];
        static const IMS_CHAR KEY_AUDIO_MAXRED_INT[];
        static const IMS_CHAR KEY_AUDIO_RTP_DSCP_INT[];
        static const IMS_CHAR KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_MODESET_BOOL[];
        static const IMS_CHAR KEY_AUDIO_AMRWB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY[];
        static const IMS_CHAR KEY_AUDIO_AMRNB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY[];
        static const IMS_CHAR KEY_EVS_CODEC_ATTRIBUTE_VISIBLE_DTX_BOOL[];
        static const IMS_CHAR KEY_CODEC_ATTRIBUTE_VISIBLE_MODE_CHANGE_NEIGHBOR_BOOL[];
        static const IMS_CHAR KEY_CODEC_ATTRIBUTE_VISIBLE_MODE_CHANGE_CAPABILITY_BOOL[];
        static const IMS_CHAR KEY_CODEC_ATTRIBUTE_VISIBLE_MODE_CHANGE_PERIOD_BOOL[];
        static const IMS_CHAR KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_AMRWBIO_MODESET_BOOL[];
        static const IMS_CHAR KEY_AUDIO_TELEPHONE_EVENT_DURATION_MILLIS_INT[];
        static const IMS_CHAR KEY_AUDIO_CANDIDATE_ATTRIBUTE_STRING_ARRAY[];
        static const IMS_CHAR KEY_AUDIO_RTCPXR_ENABLE_BOOL[];
        static const IMS_CHAR KEY_AUDIO_RTCPXR_STATISTICS_BOOL[];
        static const IMS_CHAR KEY_AUDIO_RTCPXR_VOIP_METRICS_BOOL[];
        static const IMS_CHAR KEY_AUDIO_RTCPXR_PACKET_LOSS_RLE_BOOL[];
        static const IMS_CHAR KEY_AUDIO_RTCPXR_PACKET_DUPLICATE_RLE_BOOL[];
        static const IMS_CHAR KEY_SUPPORT_MULTI_CONFIG_IN_EARLY_SESSION_BOOL[];
        static const IMS_CHAR KEY_AUDIO_EVS_SUPPORT_BOOL[];
        static const IMS_CHAR KEY_MEDIA_ANBR_CAPABILITY_IN_MODEM_BOOL[];
        static const IMS_CHAR KEY_MEDIA_SESSION_LEVEL_BANDWIDTH_BOOL[];
        static const IMS_CHAR KEY_SDP_ANSWER_FULL_CAPABILITY_BOOL[];
        static const IMS_CHAR KEY_SDP_REOFFER_FULL_CAPABILITY_BOOL[];
        static const IMS_CHAR KEY_SESSION_REFRESH_SDP_SESSION_VERSION_INCREMENT_INT[];
        static const IMS_CHAR KEY_INCOMING_DTMF_TONE_PLAY_SUPPORT_BOOL[];
        static const IMS_CHAR KEY_MEDIA_RECVONLY_EARLY_SESSION_BOOL[];
        static const IMS_CHAR KEY_EARLY_MEDIA_INACTIVE_DIRECTION_ON_PEM_INACTIVE_BOOL[];
        static const IMS_CHAR KEY_AMR_CODEC_ATTRIBUTE_DTX_BOOL[];
        static const IMS_CHAR KEY_CODEC_BASED_DYNAMIC_AS_ENABLED_BOOL[];
        static const IMS_CHAR KEY_AMR_CODEC_PAYLOAD_FORMAT_RELAXED_MATCHING_BOOL[];

        // AMR payload formats
        enum
        {
            AMR_BANDWIDTH_EFFICIENT = 0,
            AMR_OCTET_ALIGNED = 1
        };

        // EVS operational modes
        enum
        {
            EVS_OPERATIONAL_MODE_PRIMARY = 0,
            EVS_OPERATIONAL_MODE_AMRWB_IO = 1
        };

        // EVS encoded bandwidth types
        enum
        {
            EVS_ENCODED_BW_TYPE_NB = 0,
            EVS_ENCODED_BW_TYPE_WB = 1,
            EVS_ENCODED_BW_TYPE_SWB = 2,
            EVS_ENCODED_BW_TYPE_FB = 3,
            EVS_ENCODED_BW_TYPE_NB_WB = 4,
            EVS_ENCODED_BW_TYPE_NB_WB_SWB = 5,
            EVS_ENCODED_BW_TYPE_NB_WB_SWB_FB = 6
        };

        // EVS primary mode bitrates
        enum
        {
            EVS_PRIMARY_MODE_BITRATE_5_9_KBPS = 0,
            EVS_PRIMARY_MODE_BITRATE_7_2_KBPS = 1,
            EVS_PRIMARY_MODE_BITRATE_8_0_KBPS = 2,
            EVS_PRIMARY_MODE_BITRATE_9_6_KBPS = 3,
            EVS_PRIMARY_MODE_BITRATE_13_2_KBPS = 4,
            EVS_PRIMARY_MODE_BITRATE_16_4_KBPS = 5,
            EVS_PRIMARY_MODE_BITRATE_24_4_KBPS = 6,
            EVS_PRIMARY_MODE_BITRATE_32_0_KBPS = 7,
            EVS_PRIMARY_MODE_BITRATE_48_0_KBPS = 8,
            EVS_PRIMARY_MODE_BITRATE_64_0_KBPS = 9,
            EVS_PRIMARY_MODE_BITRATE_96_0_KBPS = 10,
            EVS_PRIMARY_MODE_BITRATE_128_0_KBPS = 11
        };

        // Unused. Duplicated {@code GEOLOCATION_PIDF_FOR_...}.
        // Index for Geolocation-PIDF information level
        enum
        {
            GEOLOCATION_PIDF_INFO_INDEX_EMERGENCY_CELLULAR = 0,
            GEOLOCATION_PIDF_INFO_INDEX_EMERGENCY_WIFI = 1,
            GEOLOCATION_PIDF_INFO_INDEX_CELLULAR = 2,
            GEOLOCATION_PIDF_INFO_INDEX_WIFI = 3
        };
    };

    class ImsVt
    {
    public:
        // Public carrier-config - starts
        static const IMS_CHAR KEY_VIDEO_ON_DEFAULT_BEARER_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_VIDEO_RTP_INACTIVITY_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_VIDEO_RTCP_INACTIVITY_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_VIDEO_AS_BANDWIDTH_KBPS_INT[];
        static const IMS_CHAR KEY_VIDEO_RS_BANDWIDTH_BPS_INT[];
        static const IMS_CHAR KEY_VIDEO_RR_BANDWIDTH_BPS_INT[];
        static const IMS_CHAR KEY_VIDEO_RTP_DSCP_INT[];
        static const IMS_CHAR KEY_VIDEO_QOS_PRECONDITION_SUPPORTED_BOOL[];
        // Bundle {
        static const IMS_CHAR KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE[];
        static const IMS_CHAR KEY_H264_PAYLOAD_TYPE_INT_ARRAY[];
        // }
        // Bundle {
        static const IMS_CHAR KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE[];
        static const IMS_CHAR KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT[];
        static const IMS_CHAR KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT[];
        static const IMS_CHAR KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY[];
        static const IMS_CHAR KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING[];
        // }
        // Public carrier-config - ends
        static const IMS_CHAR KEY_CONVERT_REMOTE_RESPONSE_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_CONVERT_USER_RESPONSE_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_POLICY_ON_VIDEO_QOS_DEACTIVATION_INT[];
        static const IMS_CHAR KEY_SUPPORT_EARLY_SESSION_BOOL[];

        static const IMS_CHAR KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT[];
        enum
        {
            TEXT_VIDEO_NOT_ALLOWED = 0,
            TEXT_VIDEO_NOT_ALLOWED_IF_ACTIVE = 1,
            TEXT_VIDEO_ALLOWED = 2,
        };

        static const IMS_CHAR KEY_MINIMUM_BATTERY_LEVEL_FOR_LIMIT_VIDEO_CALL_INT[];
        static const IMS_CHAR KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL[];

        /**
         * Specifies whether to determine the capability to upgrade/downgrade voice/video calls
         * based on the "avchange" feature.
         *
         * If set to {@code true}, the IMS Stack considers the "avchange" feature for
         * call converting capability. If set to {@code false}, the IMS Stack does not consider
         * the "avchange" feature for call converting capability.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_CHECK_AVCHANGE_FEATURE_FOR_CALL_CONVERTING_CAPABILITY_BOOL[];
        static const IMS_CHAR KEY_VILTE_TO_VOLTE_RETRY_FAILURE_RESPONSE_CODE_INT_ARRAY[];
        static const IMS_CHAR KEY_SUPPORT_VIDEO_CALL_ONLY_IN_VOPS_OFF_STATUS_BOOL[];
        static const IMS_CHAR
                KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL[];
        /**
         * Specifies whether to include the "video" feature tag in the Accept-Contact header
         * regardless of the call type.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_ADD_VIDEO_FEATURE_TAG_IN_ACCEPT_CONTACT_ALWAYS_BOOL[];
        static const IMS_CHAR KEY_REQUIRE_SIP_SIGNALING_ON_MULTITASKING_BOOL[];
        /**
         * Specifies whether to check if local resource for video media is reserved after
         * the call is established or updated.
         *
         * If {@code true} and video QoS hasn't activated, the next action will be determined
         * by {@code imsvt.policy_on_video_qos_deactivation_int} - usually downgrade.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_CHECK_LOCAL_RESOURCE_AFTER_ESTABLISHED_OR_MODIFIED_BOOL[];
        // Media
        static const IMS_CHAR KEY_VIDEO_RTCP_INTERVAL_INT_ARRAY[];
        static const IMS_CHAR KEY_VIDEO_AVPF_FEATURE_INT[];
        static const IMS_CHAR KEY_VIDEO_SEND_PERIODIC_SPS_PPS_INT[];
        static const IMS_CHAR KEY_VIDEO_AVPF_ENABLE_BOOL[];
        static const IMS_CHAR KEY_VIDEO_CODEC_BITRATE_INT_ARRAY[];
        static const IMS_CHAR KEY_VIDEO_CODEC_IMAGE_ATTR_STRING_ARRAY[];
        static const IMS_CHAR KEY_VIDEO_CODEC_FRAME_SIZE_STRING_ARRAY[];
        static const IMS_CHAR KEY_VIDEO_CODEC_HEVC_PROFILE_INT_ARRAY[];
        static const IMS_CHAR KEY_VIDEO_CODEC_HEVC_LEVEL_INT_ARRAY[];
        static const IMS_CHAR KEY_AVC_SPROP_PARAMETER_SETS_STRING[];
        static const IMS_CHAR KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY[];
        static const IMS_CHAR KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE[];
        static const IMS_CHAR KEY_HEVC_SPROP_PARAMETER_SETS_STRING[];
        static const IMS_CHAR KEY_HEVC_PROFILE_INT[];
        static const IMS_CHAR KEY_HEVC_LEVEL_INT[];
        static const IMS_CHAR KEY_VIDEO_RTP_PORT_RANGE_INT_ARRAY[];
        static const IMS_CHAR KEY_VIDEO_CVO_VALUE_INT[];
        static const IMS_CHAR KEY_VIDEO_SDP_OFFER_CAP_NEGO_FOR_AVPF_INT[];
        static const IMS_CHAR KEY_VIDEO_IFRAME_INTERVAL_SEC_INT[];
        static const IMS_CHAR KEY_VIDEO_BW_NEGO_OPTION_BOOL[];
        static const IMS_CHAR KEY_VIDEO_LOWEST_BITRATE_BPS_INT[];
        static const IMS_CHAR KEY_VIDEO_CODEC_HEVC_PRIORITY_ORDER_BOOL[];
        static const IMS_CHAR KEY_VIDEO_INACTIVE_HOLD_BOOL[];
    };

    class ImsWfc
    {
    public:
        // Public carrier-config - starts
        static const IMS_CHAR KEY_PIDF_SHORT_CODE_STRING_ARRAY[];
        static const IMS_CHAR KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_BOOL[];
        // Public carrier-config - ends

        // Aos
        /**
         * Specifies whether private header like P-Cellular-Network-Info or
         * P-Last-Access-Network-Info is supported during the registration over WiFi.
         *
         * Possible Values:
         *   {@code P_NOT_SUPPORTED} (0)
         *   {@code P_CELLULAR_NETWORK_INFO} (1)
         *   {@code P_LAST_ACCESS_NETWORK_INFO} (2)
         */
        static const IMS_CHAR KEY_REGISTRATION_PRIVATE_HEADER_INT[];
        enum
        {
            REGISTRATION_P_NOT_SUPPORTED = 0,
            REGISTRATION_P_CELLULAR_NETWORK_INFO = 1,
            REGISTRATION_P_LAST_ACCESS_NETWORK_INFO = 2
        };

        static const IMS_CHAR KEY_COUNTRY_CODE_INT[];

        /**
         * Specifies whether to add geolocation pidf in initial registration over wifi.
         *
         * It's applied if CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI is set
         * in the CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_IN_SIP_REGISTER_SUPPORT_INT_ARRAY.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_GEOLOCATION_PIDF_IN_WFC_INIT_REG_BOOL[];

        /**
         * Specifies whether to add PANI header in initial registration over wifi.
         *
         * It's applied if
         * CarrierConfig::Ims::KEY_ALLOW_SIP_P_ACCESS_NETWORK_INFO_HEADER_IN_INITIAL_REGISTER_BOOL
         * is false
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_PANI_HEADER_IN_WFC_INIT_REG_BOOL[];

        /**
         * Specifies whether registration is updated when the country is changed over WiFi.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_REREG_WITH_CHANGED_COUNTRY_ON_WIFI_BOOL[];

        /**
         * Specifies whether or not WFC service block is required when airplane mode on. If this
         * is set to 'true', WFC service is blocked when flight mode on.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_REQUIRED_WFC_BLOCK_BY_AIRPLANE_MODE_BOOL[];

        /**
         * Specifies whether check availability of country code or not for wfc.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_USE_WFC_COUNTRY_CODE_AVAILABILITY_CHECK_BOOL[];

        /**
         * Specifies if video call is supported over wifi when voice call is unavailable. True:
         * Video call is supported over wifi even though voice call is unavailable. False:
         * Video call is not supported over wifi if voice call is unavailable.
         *
         * Possible Values:
         *   {@code true}
         *   {@code false}
         */
        static const IMS_CHAR KEY_VIDEO_OVER_WIFI_SUPPORTED_WITHOUT_VOICE_BOOL[];

        /**
         * Specifies retry count to block wifi registration when receiving error response to the
         * reg event package consecutively The retry count is increased if delivering the wfc
         * error message below.
         *
         * - CarrierConfig::ImsWfc::KEY_WFC_ERR_SUB_403_STRING
         * - CarrierConfig::ImsWfc::KEY_WFC_ERR_NOTIFY_TERMINATED_STRING The retry count is
         * reset if the error response is not received consecutively
         *
         * @see CarrierConfig::ImsWfc::KEY_WFC_ERR_MESSAGE_BUNDLE.
         *
         * Possible Values:
         *   {@code 3}
         */
        static const IMS_CHAR KEY_SUB_CONSECUTIVE_RETRY_CNT_FOR_REG_FORBIDDEN_IN_WIFI_INT[];

        /**
         * Specifies whether initial IMS registration is tried when the device receives any
         * error code to the subscription with reg event package over Wi-Fi.
         *
         * Possible Values:
         *   {@code {403}}
         */
        static const IMS_CHAR KEY_VOWIFI_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY[];

        // Bundle {
        /**
         * Specifies if WFC error message is supported.
         */
        static const IMS_CHAR KEY_WFC_ERR_MESSAGE_BUNDLE[];

        /**
         * This WFC error message is not supported if empty.
         *
         * The information to be displayed are written f it is supported.
         *
         * Possible Values:
         *   REG90 - Unable to Connect
         */
        static const IMS_CHAR KEY_WFC_ERR_REG_403_STRING[];

        /**
         * This WFC error message is not supported if empty.
         *
         * The information to be displayed are written f it is supported.
         *
         * Possible Values:
         *   REG91 - Unable to Connect
         */
        static const IMS_CHAR KEY_WFC_ERR_REG_500_STRING[];

        /**
         * This WFC error message is not supported if empty.
         *
         * The information to be displayed are written f it is supported.
         *
         * Possible Values:
         *   REG92 - Wi-Fi Calling isn't supported in this country
         */
        static const IMS_CHAR KEY_WFC_ERR_NOT_SUPPORTED_COUNTRY_STRING[];

        /**
         * This WFC error message is not supported if empty.
         *
         * The information to be displayed are written f it is supported.
         *
         * Possible Values:
         *   REG09 - Missing 911 Address
         */
        static const IMS_CHAR KEY_WFC_ERR_SUB_403_STRING[];

        /**
         * This WFC error message is not supported if empty.
         *
         * The information to be displayed are written f it is supported.
         *
         * Possible Values:
         *   REG09 - Missing 911 Address
         */
        static const IMS_CHAR KEY_WFC_ERR_NOTIFY_TERMINATED_STRING[];

        /**
         * This WFC error message is not supported if empty.
         *
         * The information to be displayed are written f it is supported.
         *
         * Possible Values:
         *   REG99 - Unable to Connect
         */
        static const IMS_CHAR KEY_WFC_ERR_OTHER_FAILURES_STRING[];

        enum
        {
            WFC_ERROR_REG_403 = 1,
            WFC_ERROR_REG_500 = 2,
            WFC_ERROR_NOT_SUPPORTED_COUNTRY = 3,
            WFC_ERROR_SUB_403 = 4,
            WFC_ERROR_NOTIFY_TERMINATED = 5,
            WFC_ERROR_OTHER_FAILURES = 6
        };
        // }

        // Mtc
        static const IMS_CHAR KEY_ENABLE_FAKE_QOS_CALL_FLOW_ON_WIFI_BOOL[];
        static const IMS_CHAR KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL_INT[];
        static const IMS_CHAR KEY_REJECT_VOWIFI_VOICE_CALL_WHEN_VOWIFI_SETTING_OFF_BOOL[];
        static const IMS_CHAR KEY_OVERRIDE_MEDIA_INACTIVITY_TO_WIFI_LOST_BOOL[];
    };

    class ImsSs
    {
    public:
        // Public carrier-config - starts
        static const IMS_CHAR KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY[];
        static const IMS_CHAR KEY_NETWORK_INITIATED_USSD_OVER_IMS_SUPPORTED_BOOL[];
        // Public carrier-config - ends

        // Unused. Using {@code CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_...}.
        // ut terminal based services
        enum
        {
            SUPPLEMENTARY_SERVICE_CW = 0,
        };
    };

    /**
     * Checks if VoLTE is enabled by the platform or not.
     *
     * @param nSlotId The slot-id to be checked.
     *
     * @return IMS_TRUE if VoLTE is enabled, IMS_FALSE otherwise.
     */
    static IMS_BOOL IsVoLteEnabled(IN IMS_SINT32 nSlotId);

    /**
     * Checks if VT is enabled by the platform or not.
     *
     * @param nSlotId The slot-id to be checked.
     *
     * @return IMS_TRUE if VT is enabled, IMS_FALSE otherwise.
     */
    static IMS_BOOL IsVtEnabled(IN IMS_SINT32 nSlotId);

    /**
     * Checks if Wi-Fi Calling is enabled by the platform or not.
     *
     * @param nSlotId The slot-id to be checked.
     *
     * @return IMS_TRUE if Wi-Fi Calling is enabled, IMS_FALSE otherwise.
     */
    static IMS_BOOL IsWfcEnabled(IN IMS_SINT32 nSlotId);
};

#endif
