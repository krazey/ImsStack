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
    static const IMS_CHAR KEY_RTT_AUTO_UPGRADE_BOOL[];
    static const IMS_CHAR KEY_RTT_SUPPORTED_FOR_VT_BOOL[];
    static const IMS_CHAR KEY_RTT_UPGRADE_SUPPORTED_BOOL[];
    static const IMS_CHAR KEY_RTT_DOWNGRADE_SUPPORTED_BOOL[];
    static const IMS_CHAR KEY_SUPPORT_IMS_CONFERENCE_EVENT_PACKAGE_BOOL[];
    static const IMS_CHAR KEY_SUPPORT_IMS_CONFERENCE_EVENT_PACKAGE_ON_PEER_BOOL[];
    static const IMS_CHAR KEY_IWLAN_HANDOVER_POLICY_STRING_ARRAY[];
    // Public carrier-config - ends

    // USSD Method
    enum
    {
        USSD_OVER_CS_PREFERRED = 0,
        USSD_OVER_IMS_PREFERRED,
        USSD_OVER_CS_ONLY,
        USSD_OVER_IMS_ONLY
    };

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
        static const IMS_CHAR KEY_GRUU_ENABLED_BOOL[];
        static const IMS_CHAR KEY_KEEP_PDN_UP_IN_NO_VOPS_BOOL[];
        static const IMS_CHAR KEY_SIP_PREFERRED_TRANSPORT_INT[];
        static const IMS_CHAR KEY_IPV4_SIP_MTU_SIZE_CELLULAR_INT[];
        static const IMS_CHAR KEY_IPV6_SIP_MTU_SIZE_CELLULAR_INT[];
        static const IMS_CHAR KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY[];
        static const IMS_CHAR KEY_SIP_OVER_IPSEC_ENABLED_BOOL[];
        static const IMS_CHAR KEY_IPSEC_AUTHENTICATION_ALGORITHMS_INT_ARRAY[];
        static const IMS_CHAR KEY_IPSEC_ENCRYPTION_ALGORITHMS_INT_ARRAY[];
        static const IMS_CHAR KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT[];
        static const IMS_CHAR KEY_REGISTRATION_RETRY_BASE_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_REGISTRATION_RETRY_MAX_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_REGISTRATION_EVENT_PACKAGE_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT[];
        static const IMS_CHAR KEY_GEOLOCATION_PIDF_IN_SIP_REGISTER_SUPPORT_INT_ARRAY[];
        static const IMS_CHAR KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY[];
        static const IMS_CHAR KEY_IMS_USER_AGENT_STRING[];
        static const IMS_CHAR KEY_SUPPORTED_RATS_INT_ARRAY[];
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
        static const IMS_CHAR KEY_SIP_DEVICE_ID_TYPE_INT[];
        static const IMS_CHAR KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY[];
        static const IMS_CHAR KEY_SDP_NEGOTIATION_REQUIRED_FOR_NON_RPR_BOOL[];
        static const IMS_CHAR KEY_REQUEST_URI_VALIDATION_REQUIRED_IN_MID_DIALOG_BOOL[];
        static const IMS_CHAR
                KEY_SESSION_TIMER_UPDATE_REQUIRED_IN_SESSION_UPDATE_BY_REINVITE_BOOL[];
        static const IMS_CHAR KEY_ALLOW_SESSION_TIMER_TURN_OFF_BOOL[];
        static const IMS_CHAR
                KEY_ALLOW_SIP_INSTANCE_PARAM_IN_CONTACT_FOR_NON_REGISTER_REQUEST_BOOL[];
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
        static const IMS_CHAR KEY_RETRANSMISSION_ALLOWED_OF_GEOLOCATION_PIDF_STRING[];
        static const IMS_CHAR KEY_ALLOW_CELLULAR_NETWORK_INFO_HEADER_BOOL[];
        static const IMS_CHAR KEY_CELLULAR_NETWORK_INFO_CACHE_EXPIRATION_SEC_INT[];
        // Aos
        static const IMS_CHAR KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY[];
        static const IMS_CHAR KEY_ISIM_INDEX_FOR_IMPU_INT[];
        static const IMS_CHAR KEY_UPDATE_REGISTRATION_WITH_RAT_CHANGE_INT_ARRAY[];
        static const IMS_CHAR KEY_UNSUBSCRIBE_REGISTRATION_EVENT_PACKAGE_BOOL[];
        static const IMS_CHAR KEY_USE_TCP_TRANSPORT_FOR_REGISTER_BOOL[];
        static const IMS_CHAR KEY_PREFERRED_IMS_DSCP_INT[];
        static const IMS_CHAR KEY_REGISTRATION_PREFERRED_ACCESSTYPE_FEATURE_TAG_INT[];
        static const IMS_CHAR KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY[];
        static const IMS_CHAR KEY_B2C_CALL_COMPOSER_FEATURE_TAG_IN_REG_CONTACT_BOOL[];
        static const IMS_CHAR KEY_BLOCK_PCSCF_ON_REG_FAILURE_BOOL[];
        static const IMS_CHAR KEY_BLOCK_REG_ON_CS_CALL_BOOL[];
        static const IMS_CHAR KEY_CALL_END_AND_PDN_REACTIVATION_BY_REG_TERMINATED_BOOL[];
        static const IMS_CHAR KEY_DESTROY_UNSECURE_TCP_SOCKET_ON_ACCOMPLISHING_REG_BOOL[];
        static const IMS_CHAR KEY_HOLD_REG_WITH_IPCAN_CHANGED_DURING_IMS_CALL_BOOL[];
        static const IMS_CHAR KEY_IMS_DEREG_ON_3G_NETWORK_BOOL[];
        static const IMS_CHAR KEY_IMSI_BASED_URI_PRIORITIZED_BOOL[];
        static const IMS_CHAR KEY_INIT_IPSEC_SETTING_WITH_NEW_PCSCF_BOOL[];
        static const IMS_CHAR KEY_INIT_SUB_UPON_SUB_TERMINATED_BOOL[];
        static const IMS_CHAR KEY_KEEP_REG_RETRY_CNT_UPON_PDN_RECONNECT_BOOL[];
        static const IMS_CHAR KEY_KEEP_REG_RETRY_TIMER_ON_ALL_ENABLERS_DETACHED_BOOL[];
        static const IMS_CHAR KEY_NO_INIT_REG_ON_PCSCF_CHANGE_BOOL[];
        static const IMS_CHAR KEY_REG_CONTACT_VALIDATION_BOOL[];
        static const IMS_CHAR KEY_REG_PLMN_BLOCK_WITH_TIMEOUT_ON_FAILURE_WITH_ALL_PCSCFS_BOOL[];
        static const IMS_CHAR KEY_REG_RETRY_WITH_IP_VER_FALLBACK_BOOL[];
        static const IMS_CHAR KEY_REMOVE_OLD_SA_ON_ESTABLISHING_SA_BOOL[];
        static const IMS_CHAR KEY_REQUIRED_CDMALESS_FEATURE_TAG_BOOL[];
        static const IMS_CHAR KEY_REQUIRED_INIT_REG_AFTER_IMS_CALL_END_ON_REG_HELD_BOOL[];
        static const IMS_CHAR KEY_REQUIRED_INIT_REG_AFTER_IMS_ECALL_END_ON_REG_HELD_BOOL[];
        static const IMS_CHAR KEY_SIP_OVER_IPSEC_ENABLED_IN_ROAMING_BOOL[];
        static const IMS_CHAR KEY_SUPPORT_REG_WITH_FEATURE_TAG_UNAVAILABLE_BOOL[];
        static const IMS_CHAR KEY_SUPPORT_VERSTAT_BASED_ON_NETWORK_FOR_REG_BOOL[];
        static const IMS_CHAR KEY_SUPPORT_VERSTAT_FOR_REG_BOOL[];
        static const IMS_CHAR KEY_UPDATE_ONGOING_REG_RETRY_TIMER_ON_IMS_EST_TIMER_EXPIRY_BOOL[];
        static const IMS_CHAR KEY_USE_RCS_TELEPHONY_FEATURE_TAG_AS_AVAILABLE_VOICE_CALL_TYPE_BOOL[];
        static const IMS_CHAR KEY_USE_REGINFO_CONTACT_WITHOUT_URI_CHECK_BOOL[];
        static const IMS_CHAR KEY_USE_SECURITY_SERVER_PORT_IN_INIT_REG_BOOL[];
        static const IMS_CHAR KEY_USE_SECURITY_SERVER_PORT_IN_REG_CONTACT_OF_INIT_REG_BOOL[];
        static const IMS_CHAR KEY_CONTACT_USER_INFO_POLICY_FOR_NON_REG_MESSAGE_INT[];
        static const IMS_CHAR KEY_GEOLOCATION_PIDF_FORMING_POLICY_INT[];
        static const IMS_CHAR KEY_IMS_ESTABLISHMENT_TIME_FOR_LTE_SEC_INT[];
        static const IMS_CHAR KEY_IMS_ESTABLISHMENT_TIME_FOR_NR_SEC_INT[];
        static const IMS_CHAR KEY_IMS_PREFERRED_IPTYPE_INT[];
        static const IMS_CHAR KEY_IMS_SIGNALLING_DSCP_INT[];
        static const IMS_CHAR
                KEY_PDN_RECONNECT_DELAY_ON_WFC_SETUP_FAIL_ALL_PCSCFS_WITH_CS_ROAM_SEC_INT[];
        static const IMS_CHAR KEY_REG_ACTUAL_WAIT_TIME_POLICY_INT[];
        static const IMS_CHAR KEY_REG_DEFAULT_WAIT_TIME_INT[];
        static const IMS_CHAR KEY_REG_OUT_OF_SERVICE_POLICY_INT[];
        static const IMS_CHAR KEY_REG_PCSCF_UPDATE_POLICY_INT[];
        static const IMS_CHAR KEY_REG_RETRY_305_POLICY_INT[];
        static const IMS_CHAR KEY_REG_RETRY_503_POLICY_INT[];
        static const IMS_CHAR KEY_REG_RETRY_CNT_ON_SINGLE_PCSCF_INT[];
        static const IMS_CHAR KEY_REG_RETRY_CNT_PER_PCSCF_INT[];
        static const IMS_CHAR KEY_REG_RETRY_CNT_RESET_POLICY_INT[];
        static const IMS_CHAR KEY_REG_RETRY_CNT_WITH_IPSEC_ON_AUTH_FAILURE_INT[];
        static const IMS_CHAR KEY_REG_RETRY_DEFAULT_POLICY_INT[];
        static const IMS_CHAR KEY_REG_RETRY_TIMER_F_POLICY_INT[];
        static const IMS_CHAR KEY_REREG_RETRY_305_POLICY_INT[];
        static const IMS_CHAR KEY_SIP_MESSAGE_THRESHOLD_FOR_TRANSPORT_CHANGE_INT[];
        static const IMS_CHAR KEY_SUB_RETRY_503_POLICY_INT[];
        static const IMS_CHAR KEY_USAT_REG_EVENT_DOWNLOAD_POLICY_INT[];
        static const IMS_CHAR KEY_PERMANENT_PDN_FAILURE_INT_ARRAY[];
        static const IMS_CHAR KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY[];
        static const IMS_CHAR KEY_REG_PERMANENT_ERR_MAX_CNT_INT_ARRAY[];
        static const IMS_CHAR KEY_REG_RETRY_ERR_CODE_WITHOUT_IPSEC_INT_ARRAY[];
        static const IMS_CHAR KEY_REREG_ERR_CODE_FOR_CALL_END_INT_ARRAY[];
        static const IMS_CHAR KEY_REREG_ERR_CODE_FOR_IMS_PDN_REACTIVATION_INT_ARRAY[];
        static const IMS_CHAR KEY_REREG_ERR_CODE_FOR_INIT_REG_WITH_AVAILABLE_PCSCF_INT_ARRAY[];
        static const IMS_CHAR KEY_REREG_RETRY_ERR_CODE_FOR_INIT_REG_WITH_SAME_PCSCF_INT_ARRAY[];
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_NEXT_PCSCF_INT_ARRAY[];
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_STOPPING_BY_EXPIRATION_TIME_INT_ARRAY[];
        static const IMS_CHAR KEY_SUPPORTED_ROAMING_RATS_INT_ARRAY[];
        static const IMS_CHAR KEY_TEST_MODE_INT_ARRAY[];
        static const IMS_CHAR KEY_UNAVAILABLE_FEATURES_IN_LIMITED_REG_INT_ARRAY[];
        // Bundle {
        static const IMS_CHAR KEY_EXTRA_REG_ERR_BUNDLE[];
        static const IMS_CHAR KEY_EXTRA_REG_ERR_CODE_AS_FAILURE_IN_ROAMING_FOR_UPDATE_BOOL[];
        static const IMS_CHAR KEY_EXTRA_REG_ERR_RETRY_CNT_SHARED_FOR_REG_AND_SUB_BOOL[];
        static const IMS_CHAR KEY_EXTRA_REG_ERR_FINAL_TYPE_INT[];
        static const IMS_CHAR KEY_EXTRA_REG_ERR_MAX_CNT_INT[];
        static const IMS_CHAR KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_EPS_5GS_ONLY_ATTACHED_INT[];
        static const IMS_CHAR KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_LTE_COMBINED_ATTACHED_INT[];
        static const IMS_CHAR KEY_EXTRA_REG_ERR_POLICY_INT[];
        static const IMS_CHAR KEY_EXTRA_REG_ERR_CODE_INT_ARRAY[];
        static const IMS_CHAR KEY_EXTRA_REG_ERR_CODE_FOR_UPDATE_INT_ARRAY[];
        static const IMS_CHAR KEY_EXTRA_REG_ERR_WAIT_TIME_SEC_INT_ARRAY[];
        // }
        // Bundle {
        static const IMS_CHAR KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE[];
        static const IMS_CHAR KEY_NOTIFY_TERMINATED_FOR_INIT_REG_WITH_WAIT_TIME_INT[];
        static const IMS_CHAR KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY[];
        static const IMS_CHAR
                KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_WITH_WAIT_TIME_INT_ARRAY[];
        // }
        // Bundle {
        static const IMS_CHAR KEY_PCSCF_RECOVERY_CONDITIONS_BUNDLE[];
        static const IMS_CHAR KEY_PCSCF_RECOVERY_MAX_CNT_INT[];
        static const IMS_CHAR KEY_PCSCF_RECOVERY_WAIT_TIME_SEC_INT[];
        static const IMS_CHAR KEY_PCSCF_RECOVERY_BASE_TIME_SEC_INT[];
        static const IMS_CHAR KEY_PCSCF_RECOVERY_MAX_TIME_SEC_INT[];
        // }
        // Bundle {
        static const IMS_CHAR KEY_REG_ERR_CODE_WITH_RA_TIME_BUNDLE[];
        static const IMS_CHAR KEY_REG_ERR_CODE_WITH_RA_TIME_ONLY_DEFINED_BOOL[];
        static const IMS_CHAR KEY_REG_ERR_CODE_WITH_RA_TIME_INT_ARRAY[];
        static const IMS_CHAR KEY_REG_ERR_CODE_WITH_RA_TIME_FOR_UPDATE_INT_ARRAY[];
        // }
        // Bundle {
        static const IMS_CHAR KEY_REG_RETRY_INTERVAL_BUNDLE[];
        static const IMS_CHAR KEY_REG_RETRY_INTERVAL_USED_FOR_SUB_BOOL[];
        static const IMS_CHAR KEY_REG_RETRY_INTERVAL_RANDOM_UPPER_VALUE_SEC_INT_ARRAY[];
        static const IMS_CHAR KEY_REG_RETRY_INTERVAL_SEC_INT_ARRAY[];
        // }
        // Bundle {
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_INIT_REG_BUNDLE[];
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_RETRY_MAX_CNT_INT[];
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY[];
        // }
        // Bundle {
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_TERMINATED_BUNDLE[];
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_TERMINATED_WITH_RETRY_MAX_CNT_INT[];
        static const IMS_CHAR KEY_SUB_ERR_CODE_FOR_TERMINATED_INT_ARRAY[];
        // }

        // Mtc
        static const IMS_CHAR KEY_INFORMATION_LEVEL_OF_GEOLOCATION_PIDF_INT_ARRAY[];

        // Carrier NR availabilities
        enum
        {
            CARRIER_NR_AVAILABILITY_NSA = 1,
            CARRIER_NR_AVAILABILITY_SA = 2
        };

        // Request URI formats
        enum
        {
            REQUEST_URI_FORMAT_TEL = 0,
            REQUEST_URI_FORMAT_SIP = 1
        };

        // Preferred SIP transport types
        enum
        {
            PREFERRED_TRANSPORT_UDP = 0,
            PREFERRED_TRANSPORT_TCP = 1,
            PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP = 2,
            PREFERRED_TRANSPORT_TLS = 3
        };

        // IpSec authentication algorithms
        enum
        {
            IPSEC_AUTHENTICATION_ALGORITHM_HMAC_MD5 = 0,
            IPSEC_AUTHENTICATION_ALGORITHM_HMAC_SHA1 = 1
        };

        // IpSec encryption algorithms
        enum
        {
            IPSEC_ENCRYPTION_ALGORITHM_NULL = 0,
            IPSEC_ENCRYPTION_ALGORITHM_DES_EDE3_CBC = 1,
            IPSEC_ENCRYPTION_ALGORITHM_AES_CBC = 2
        };

        // Geolocation-PIDF allowed types
        enum
        {
            GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI = 1,
            GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI = 2,
            GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR = 3,
            GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR = 4
        };

        // Network types
        enum
        {
            NETWORK_TYPE_HOME = 0,
            NETWORK_TYPE_ROAMING = 1
        };

        // Media inactivity reasons
        enum
        {
            RTCP_INACTIVITY_ON_HOLD = 0,
            RTCP_INACTIVITY_ON_CONNECTED = 1,
            RTP_INACTIVITY_ON_CONNECTED = 2,
            E911_RTCP_INACTIVITY_ON_CONNECTED = 3,
            E911_RTP_INACTIVITY_ON_CONNECTED = 4
        };

        // Access network types
        enum
        {
            ACCESS_NETWORK_TYPE_GERAN = 1,
            ACCESS_NETWORK_TYPE_UTRAN = 2,
            ACCESS_NETWORK_TYPE_EUTRAN = 3,
            ACCESS_NETWORK_TYPE_IWLAN = 5,
            ACCESS_NETWORK_TYPE_NGRAN = 6
        };

        // P-CSCF discovery methods
        enum
        {
            PCSCF_DISCOVERY_METHOD_PCO = 0,
            PCSCF_DISCOVERY_METHOD_CONFIG = 1,
            PCSCF_DISCOVERY_METHOD_ISIM = 2
        };

        // IMS identity priority types
        enum
        {
            IMS_IDENTITY_PRIORITY_ISIM = 0,
            IMS_IDENTITY_PRIORITY_USIM = 1,
            IMS_IDENTITY_PRIORITY_ISIM_IMSI = 2,
            IMS_IDENTITY_PRIORITY_CONF = 3
        };

        // Preferred IMS DSCP types
        enum
        {
            PREFERRED_DSCP_NONE = 0,
            PREFERRED_DSCP_CELLULAR = 1,
            PREFERRED_DSCP_WIFI = 2,
            PREFERRED_DSCP_CELLULAR_WIFI = 3
        };

        // Registration Preferred Accesstype feature tag
        enum
        {
            PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED = 0,
            PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED = 1,
            PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED_WITHOUT_NUMERICAL_VALUE = 2
        };

        // Registration retry sip 305 policy
        enum
        {
            SIP_305_CODE_POLICY_DEFAULT = 0,
            SIP_305_CODE_POLICY_3GPP = 1,
            SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF = 2,
            SIP_305_CODE_POLICY_USE_CONTACT_VALUE = 3
        };

        // Registration Retry Timer F Policy
        enum
        {
            TIMER_F_POLICY_NONE = 0,
            TIMER_F_POLICY_SPEC = 1,
            TIMER_F_POLICY_SPEC_WITH_AWT = 2
        };

        // Registration Retry Default Policy
        enum
        {
            DEFAULT_RETRY_POLICY_SPEC = 0,
            DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF = 1,
            DEFAULT_RETRY_POLICY_NEXT_PCSCF = 2
        };

        // Registration and Subscription(reg event package) retry sip 503 policy
        enum
        {
            SIP_503_CODE_POLICY_DEFAULT = 0,
            SIP_503_CODE_POLICY_3GPP = 1
        };

        // specific registration error final type
        enum
        {
            ERROR_TYPE_NOT_SPECIFIED = 0,
            ERROR_TYPE_REPEATED = 1,
            ERROR_TYPE_CRITICAL = 2,
            ERROR_TYPE_ROAMING = 3,
            ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK = 4,
            ERROR_TYPE_RAT_BLOCK = 5
        };

        // specific registration error policy
        enum
        {
            ERROR_POLICY_NOT_SPECIFIED = 0,
            ERROR_POLICY_PCSCF_FAILED = 1,
            ERROR_POLICY_SUBSCRIBER_FAILED = 2,
            ERROR_POLICY_PDN_REACTIVATED = 3
        };

        // error code mapping for registration
        // specific registration error policy
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

        // registration retry count reset policy
        enum
        {
            REG_RETRY_CNT_RESET_POLICY_REGISTRATION = 0,
            REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION = 1,
            REG_RETRY_CNT_RESET_POLICY_NOTIFY = 2
        };

        // IP verion types
        enum
        {
            IP_VERSION_4 = 0,
            IP_VERSION_6 = 1
        };

        // Reasons for terminated state of "reg" event package
        enum
        {
            REG_EVENT_TERMINATED_REASON_EXPIRED = 1,
            REG_EVENT_TERMINATED_REASON_DEACTIVATED = 2,
            REG_EVENT_TERMINATED_REASON_PROBATION = 3,
            REG_EVENT_TERMINATED_REASON_UNREGITERED = 4,
            REG_EVENT_TERMINATED_REASON_REJECTED = 5
        };

        // Registration Actual Wait Time policy
        enum
        {
            AWT_POLICY_RFC_RULE = 0,
            AWT_POLICY_FAILURE_TO_EVERY_PCSCF = 1,
            AWT_POLICY_FAILURE_TO_EACH_PCSCF = 2,
            AWT_POLICY_SPECIFIED_INTERVAL = 3
        };

        // Registration out-of-service policy
        enum
        {
            REG_OOS_POLICY_DEFAULT = 0,
            REG_OOS_POLICY_DESTROY = 1
        };

        // Registration With Feature Tag Policy Unavailable
        enum
        {
            UNAVAILABLE_FEATURE_POLICY_VOPS = 1,
            UNAVAILABLE_FEATURE_POLICY_CAPABILITY = 2,
            UNAVAILABLE_FEATURE_POLICY_3G = 3
        };

        // Update Pcscf Policy for registration
        enum
        {
            REG_PCSCF_UPDATE_POLICY_DEFAULT = 0,
            REG_PCSCF_UPDATE_POLICY_ALL_THE_TIME = 1
        };

        // User Info Policy for non register message
        enum
        {
            CONTACT_USER_INFO_POLICY_DEFAULT = 1,
            CONTACT_USER_INFO_POLICY_NONE = 2,
            CONTACT_USER_INFO_POLICY_NO_IMSI = 3
        };

        // Geolocation pidf Forming policy
        enum
        {
            GEOLOCATION_POLICY_WITHOUT_POSITION = 1,
            GEOLOCATION_POLICY_WITH_POSITION = 2,
            GEOLOCATION_POLICY_WITH_POSITION_AND_COUNTRY = 3,
            GEOLOCATION_POLICY_WITHOUT_CIVIC = 4
        };

        // USAT Registration event download policy
        enum
        {
            USAT_REG_EVENT_NOT_DOWNLOAD = 0,
            USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD = 1,
            USAT_REG_EVENT_CONDITIONAL_DOWNLOAD = 2
        };

        // Registration Features
        enum
        {
            REG_FEATURE_MMTEL = 0,
            REG_FEATURE_VIDEO = 1,
            REG_FEATURE_TEXT = 2,
            REG_FEATURE_SMS = 3
        };

        // Test Mode
        enum
        {
            TEST_MODE_PERMANENT_FAILURE_WITHOUT_IMS_PDN_DEACTIVATION = 1
        };

        // Feature: Multiple Registration
        enum
        {
            MULTIPLE_REGISTRATION_NONE = 0,
            MULTIPLE_REGISTRATION_REG_ID_ONLY = 1,
            MULTIPLE_REGISTRATION_FULL = 2
        };

        // Rule for forming the user-info part of Contact header in REGISTER request
        // - KEY_REGISTRATION_CONTACT_USER_INFO_PART_INT
        enum
        {
            /// Default, use time-based UUID
            REGISTRATION_CONTACT_USER_INFO_PART_UUID = 0,
            /// Use user-info part from IMPU
            REGISTRATION_CONTACT_USER_INFO_PART_IMPU = 1,
            /// No user-info part
            REGISTRATION_CONTACT_USER_INFO_PART_EMPTY = 2,
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
        static const IMS_CHAR KEY_RELEASE_EPDN_UPON_ECALL_END_IN_FAKE_MODE_BOOL[];
        static const IMS_CHAR KEY_IPCAN_RELEASE_EMERGENCY_PDN_UPON_EMERGENCY_CALL_END_INT[];
        static const IMS_CHAR KEY_PREFERRED_EMERGENCY_REGISTRATION_INT[];
        static const IMS_CHAR KEY_WAIT_TIME_MILLIS_FOR_RELEASE_EPDN_AFTER_ECALL_END_INT[];
        static const IMS_CHAR KEY_ECALL_BASED_ON_P_ASSOCIATED_URI_OF_NORMAL_REG_BOOL[];
        static const IMS_CHAR KEY_EREG_ON_RANDOM_PCSCF_BOOL[];
        static const IMS_CHAR KEY_EREG_SET_TCP_ONLY_IN_ROAMING_BOOL[];
        static const IMS_CHAR KEY_EREG_USING_FIRST_IMPU_IN_ISIM_BOOL[];
        static const IMS_CHAR KEY_KEEP_EPDN_UPON_PCSCF_UNAVAILABLE_BOOL[];
        static const IMS_CHAR KEY_KEEP_EREG_RETRY_ON_WLAN_BOOL[];
        static const IMS_CHAR KEY_REG_TIMER_FOR_ECALL_TIMEOUT_AS_FAILURE_BOOL[];
        static const IMS_CHAR KEY_REG_TIMER_FOR_ECALL_WITH_RAT_CHECK_ENABLED_BOOL[];
        static const IMS_CHAR KEY_RELEASE_EPDN_OF_UNAVAILABLE_NETWORK_BOOL[];
        static const IMS_CHAR KEY_STOP_EREG_TIMER_ON_EPDN_CONNECTED_BOOL[];
        static const IMS_CHAR KEY_SUPPORT_ANONYMOUS_ECALL_ACTION_BOOL[];
        static const IMS_CHAR KEY_SUPPORT_EREG_WHEN_EATTACH_WITH_VALID_SIM_BOOL[];
        static const IMS_CHAR KEY_SUPPORT_EREREG_ON_IPCAN_CHANGE_BOOL[];
        static const IMS_CHAR KEY_SUPPORT_GIBA_FOR_EREG_IN_ROAMING_BOOL[];
        static const IMS_CHAR KEY_SUPPORT_VIDEO_FOR_EREG_BOOL[];
        static const IMS_CHAR KEY_USE_REG_RETRY_RULE_FOR_EREG_BOOL[];
        static const IMS_CHAR KEY_EPDN_PREFERRED_IPTYPE_INT[];
        static const IMS_CHAR KEY_EREG_RETRY_MAX_CNT_INT[];
        static const IMS_CHAR KEY_EREG_RETRY_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_REG_TIMER_FOR_ECALL_MILLIS_INT[];
        static const IMS_CHAR KEY_ROAMING_PREFERRED_EREG_INT[];
        static const IMS_CHAR KEY_EREG_ERR_CODE_NOT_SUPPORTED_COMMON_POLICY_INT_ARRAY[];
        static const IMS_CHAR KEY_EPDN_REJECT_CAUSES_FOR_CROSS_STACK_REDIAL_INT_ARRAY[];
        static const IMS_CHAR KEY_NETWORK_ATTACH_REJECT_CAUSES_FOR_CROSS_STACK_REDIAL_INT_ARRAY[];

        // Mtc
        static const IMS_CHAR KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_ON_CELLULAR_BOOL[];
        static const IMS_CHAR
                KEY_EMERGENCY_RETRY_WITHOUT_CHECKING_380_CONTENT_FOR_NON_UE_DETECTABLE_EMERGENCY_CALL_BOOL
                        [];
        static const IMS_CHAR KEY_EMERGENCY_TCALL_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_EMERGENCY_RINGBACK_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_EMERGENCY_18X_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_EMERGENCY_RTT_GUARD_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_RETRY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_WITH_NEXT_PCSCF_BOOL[];
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
        static const IMS_CHAR KEY_CALL_PERIODIC_LOCATION_DISCOVERY_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_REJECT_CODE_REQUIRE_IMMEDIATE_TERMINATION_STRING_ARRAY[];
        static const IMS_CHAR
                KEY_REJECT_CODE_AND_REASON_REQUIRE_IMMEDIATE_TERMINATION_STRING_ARRAY[];
        static const IMS_CHAR KEY_REJECT_CODE_REQUIRE_TEMP_FAILURE_INT_ARRAY[];
        static const IMS_CHAR KEY_REJECT_CODE_REQUIRE_PERM_FAILURE_INT_ARRAY[];
        static const IMS_CHAR KEY_POLICY_FOR_EMERGENCY_URN_INT_ARRAY[];
        static const IMS_CHAR KEY_SILENT_REDIAL_WITH_VOIP_BY_RTT_REJECTION_BOOL[];
        static const IMS_CHAR KEY_NUMBER_NEED_OIP_STRING_ARRAY[];
        static const IMS_CHAR KEY_NUMBER_NEED_OIR_STRING_ARRAY[];
        static const IMS_CHAR
                KEY_PLMN_ALLOWING_GEOLOCATION_PIDF_IN_SIP_INVITE_NO_UICC_STRING_ARRAY[];
        static const IMS_CHAR KEY_DYNAMIC_ROUTING_NUMBER_PER_PLMN_STRING_ARRAY[];

        // Preferred Emergency Registration Type
        enum
        {
            PREFERRED_EMERGENCY_REGISTRATION_NOT_DEFINED = -1,
            PREFERRED_EMERGENCY_REGISTRATION_SKIP = 0,
            PREFERRED_EMERGENCY_REGISTRATION_NORMAL = 1,
            PREFERRED_EMERGENCY_REGISTRATION_FALLBACK = 2
        };

        enum
        {
            CALL_PERIODIC_LOCATION_DISCOVERY_METHOD_NONE = 0,
            CALL_PERIODIC_LOCATION_DISCOVERY_METHOD_UPDATE = 1
        };

        enum
        {
            IPCAN_NONE = 0,
            IPCAN_CELLULAR = 1,
            IPCAN_WLAN = 2,
            IPCAN_ALL = 3
        };

        // Specified Define for KEY_EREG_RETRY_MAX_CNT_INT
        enum
        {
            EREG_RETRY_MAX_CNT_NO_RETRY = -1,
            EREG_RETRY_MAX_CNT_EVERY_PCSCF_RETRY = 0
        };
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
    };

    class ImsSms
    {
    public:
        // Public carrier-config - starts
        static const IMS_CHAR KEY_SMS_OVER_IMS_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_SMS_CSFB_RETRY_ON_FAILURE_BOOL[];
        static const IMS_CHAR KEY_SMS_OVER_IMS_FORMAT_INT[];
        static const IMS_CHAR KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY[];
        // Public carrier-config - ends

        // Aos
        static const IMS_CHAR KEY_SUPPORT_LIMITED_ADMIN_SMS_MODE_BOOL[];
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
        static const IMS_CHAR KEY_SMS_GEOLOCATION_PIDF_FOR_EMERGENCY_BOOL[];
        static const IMS_CHAR KEY_SMS_RETRY_AFTER_MAX_TIME_SEC_INT[];
        static const IMS_CHAR KEY_SMS_RETRY_AFTER_MAX_COUNT_INT[];
        static const IMS_CHAR KEY_SMS_REPORT_GENERIC_ERROR_WHEN_RETRY_AFTER_NOT_POSSIBLE_BOOL[];
        static const IMS_CHAR KEY_SMS_MESSAGE_RESPONSE_WAIT_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_SMS_RETRY_POLICY_FOR_EXPIRY_TIMER_F_INT[];
        static const IMS_CHAR KEY_SMS_IN_REPLY_TO_VALIDATION_BOOL[];
        static const IMS_CHAR KEY_SMS_SUPPORT_CONTENT_TRANSFER_ENCODING_HEADER_BOOL[];

        // SMS formats
        enum
        {
            SMS_FORMAT_3GPP = 0,
            SMS_FORMAT_3GPP2 = 1
        };
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
        static const IMS_CHAR KEY_SESSION_PRIVACY_TYPE_INT[];
        static const IMS_CHAR KEY_PRACK_SUPPORTED_FOR_18X_BOOL[];
        static const IMS_CHAR KEY_CONFERENCE_SUBSCRIBE_TYPE_INT[];
        static const IMS_CHAR KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL[];
        static const IMS_CHAR KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_SRVCC_TYPE_INT_ARRAY[];
        static const IMS_CHAR KEY_RINGING_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_RINGBACK_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_AUDIO_RTP_INACTIVITY_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_AUDIO_RTCP_INACTIVITY_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_CONFERENCE_FACTORY_URI_STRING[];
        static const IMS_CHAR KEY_SESSION_REFRESH_METHOD_INT[];
        static const IMS_CHAR KEY_OIP_SOURCE_FROM_HEADER_BOOL[];
        static const IMS_CHAR KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT[];
        static const IMS_CHAR KEY_AUDIO_INACTIVITY_CALL_END_REASONS_INT_ARRAY[];
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

        // Aos
        static const IMS_CHAR KEY_IGNORE_VOPS_FOR_VOLTE_ENABLE_BOOL[];
        static const IMS_CHAR KEY_PLMN_BLOCK_WITH_TIMEOUT_ON_VOICE_CALL_UNAVAILABLE_BOOL[];
        static const IMS_CHAR KEY_REQUIRED_VOLTE_BLOCK_BY_SSAC_BOOL[];
        static const IMS_CHAR KEY_VOLTE_HYS_TIME_SEC_INT[];
        // Mtc
        static const IMS_CHAR KEY_18X_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_SUPPORT_CONFERENCE_REFER_SUBSCRIBE_BOOL[];
        static const IMS_CHAR KEY_ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT_BOOL[];
        static const IMS_CHAR KEY_CONFERENCE_SIP_FLOW_ORDER_INT[];
        static const IMS_CHAR KEY_CONFERENCE_INVITING_REFER_TYPE_INT[];
        static const IMS_CHAR KEY_POLICY_QOS_PRECONDITION_MECHANISM_WHILE_CALL_MODIFICATION_INT[];
        static const IMS_CHAR KEY_INCOMING_CALL_REJECT_CODE_FOR_USER_DECLINE_INT[];
        static const IMS_CHAR KEY_INCOMING_CALL_REJECT_CODE_FOR_NO_ANSWER_INT[];
        static const IMS_CHAR KEY_PRACK_WAIT_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_PRACK_UPDATE_RESPONSE_WAIT_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_SESSION_REFRESH_TRIGGER_INTERVAL_SEC_INT[];
        static const IMS_CHAR KEY_REGISTRATION_RESTORATION_MODE_ON_504_FOR_INVITE_INT[];
        static const IMS_CHAR
                KEY_REGISTRATION_RESTORATION_FOR_INVITE_REQUIRE_HEADER_VALIDATION_BOOL[];
        static const IMS_CHAR KEY_POLICY_ON_AUDIO_QOS_DEACTIVATION_INT[];
        static const IMS_CHAR KEY_ENABLE_SEND_REINVITE_ON_RAT_CHANGE_BOOL[];
        static const IMS_CHAR KEY_ALLOW_SDP_IN_PRACK_BOOL[];
        static const IMS_CHAR KEY_POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_INT[];
        static const IMS_CHAR KEY_POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_IN_ROAMING_INT[];
        static const IMS_CHAR KEY_POLICY_OF_LOCAL_NUMBERS_INT[];
        static const IMS_CHAR KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT[];
        static const IMS_CHAR KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT[];
        static const IMS_CHAR KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT[];
        static const IMS_CHAR KEY_SILENT_REDIAL_ULTIMATE_FAILURE_ACTION_INT[];
        static const IMS_CHAR KEY_SILENT_REDIAL_REGISTRATION_WAIT_TIME_MILLIS_INT[];
        static const IMS_CHAR KEY_CALL_TYPE_AFTER_AUDIO_AND_VIDEO_CALL_MERGED_INT[];
        static const IMS_CHAR KEY_CALLER_ID_SERVICE_CODES_FOR_RESTRICTION_STRING_ARRAY[];
        static const IMS_CHAR KEY_CALLER_ID_SERVICE_CODES_FOR_IDENTITY_STRING_ARRAY[];
        static const IMS_CHAR KEY_SHORT_CALL_CODE_INT_ARRAY[];
        static const IMS_CHAR KEY_LOCAL_NUMBER_PRESENTATION_SET_STRING[];
        static const IMS_CHAR KEY_ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL_BOOL[];
        static const IMS_CHAR KEY_REJECT_CODE_AND_REASON_CODE_SET_STRING_ARRAY[];
        static const IMS_CHAR KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY[];
        static const IMS_CHAR KEY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY[];
        static const IMS_CHAR KEY_EARLY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY[];
        static const IMS_CHAR KEY_CSFB_BLOCK_CONDITION_INT_ARRAY[];
        static const IMS_CHAR KEY_CSFB_WHEN_ALL_PCSCF_UNAVAILABLE_BOOL[];
        static const IMS_CHAR KEY_POLICY_FOR_403_RESPONSE_FOR_INVITE_INT[];
        static const IMS_CHAR KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT[];
        static const IMS_CHAR KEY_WAIT_QOS_WHEN_LOCAL_PRECONDITION_NOT_SUPPORTED_BOOL[];
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
        static const IMS_CHAR KEY_MEDIA_TYPE_FOR_OFFERLESS_INVITE_INT[];
        static const IMS_CHAR KEY_MEDIA_TYPE_FOR_OFFERLESS_REINVITE_INT[];
        static const IMS_CHAR KEY_OIP_TYPE_FOR_UNAVAILABLE_INT[];
        static const IMS_CHAR KEY_PREALERTING_TIMER_MILLIS_INT[];
        static const IMS_CHAR KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL_INT[];
        static const IMS_CHAR KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY[];
        static const IMS_CHAR
                KEY_SUPPORT_REGISTRATION_RECOVERY_FOR_FAILURE_OF_SESSION_REFRESH_BOOL[];
        static const IMS_CHAR KEY_POLICY_FOR_CALL_MAINTAINING_ON_REGISTRATION_SUSPENDED_INT_ARRAY[];
        static const IMS_CHAR KEY_USE_MCID_SUPPLEMENTARY_SERVICE_BOOL[];
        static const IMS_CHAR KEY_USE_MMC_SUPPLEMENTARY_SERVICE_BOOL[];
        static const IMS_CHAR KEY_USE_LTE_PREFERRED_STATUS_FOR_SERVICE_CAPABILITY_BOOL[];
        static const IMS_CHAR KEY_ALLOW_INCOMING_HOLD_REQUEST_DURING_CONFERENCE_CALL_BOOL[];
        static const IMS_CHAR KEY_IGNORE_180_AFTER_183_RESPONSE_BOOL[];
        static const IMS_CHAR KEY_ADD_REPLACE_HEADER_FOR_CONFERENCE_BOOL[];
        static const IMS_CHAR KEY_POLICY_FOR_PRACK_DELIVERY_FAILURE_INT[];
        static const IMS_CHAR KEY_USE_CARRIER_SPECIFIC_CONTACT_HEADER_FOR_OPTIONS_RESPONSE_BOOL[];
        static const IMS_CHAR
                KEY_USE_CARRIER_SPECIFIC_REJECT_PHRASE_FOR_INCOMING_CALL_DURING_NO_REGISTRATION_BOOL
                        [];
        static const IMS_CHAR
                KEY_ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_REJECTED_BY_SERVER_ERROR_BOOL[];
        static const IMS_CHAR KEY_ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_RETRY_UNAVAILABLE_BOOL[];
        static const IMS_CHAR KEY_CHECK_SERVER_OUTAGE_REASON_FOR_VXLTE_CALL_BOOL[];
        static const IMS_CHAR KEY_MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING_BOOL[];
        static const IMS_CHAR KEY_STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY_BOOL[];
        static const IMS_CHAR KEY_MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF_INT_ARRAY[];
        static const IMS_CHAR KEY_GEOLOCATION_BLOCK_CONDITION_INT_ARRAY[];
        static const IMS_CHAR KEY_INCOMING_RESUME_EVENT_SUPPORT_BOOL[];
        static const IMS_CHAR KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT[];
        static const IMS_CHAR KEY_INITIALIZE_P_EARLY_MEDIA_WHEN_NO_HEADER_BOOL[];
        static const IMS_CHAR KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT[];
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
        static const IMS_CHAR KEY_RAT_CONDITION_FOR_NOT_WAITING_DEDICATED_BEARER_INT_ARRAY[];
        static const IMS_CHAR
                KEY_TRIGGER_DEDICATED_BEARER_WAIT_TIMER_BY_SENDING_INITIAL_INVITE_BOOL[];
        static const IMS_CHAR KEY_RESTART_DEDICATED_BEARER_WAIT_TIMER_BY_EPS_FALLBACK_BOOL[];
        static const IMS_CHAR KEY_RESTART_RINGING_TIMER_BY_SENDING_180_BOOL[];
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
        static const IMS_CHAR KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_DTX_BOOL[];
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
        static const IMS_CHAR KEY_AMR_CODEC_ATTRIBUTE_DTX_BOOL[];

        // Session refresher types
        enum
        {
            SESSION_REFRESHER_TYPE_UNKNOWN = 0,
            SESSION_REFRESHER_TYPE_UAC = 1,
            SESSION_REFRESHER_TYPE_UAS = 2
        };

        // Session refresh methods
        enum
        {
            SESSION_REFRESH_METHOD_INVITE = 0,
            SESSION_REFRESH_METHOD_UPDATE_PREFERRED = 1
        };

        // Session privacy types
        enum
        {
            SESSION_PRIVACY_TYPE_HEADER = 0,
            SESSION_PRIVACY_TYPE_NONE = 1,
            SESSION_PRIVACY_TYPE_ID = 2
        };

        // Conference subscribe types
        enum
        {
            CONFERENCE_SUBSCRIBE_NOT_SUPPORT = -1,
            CONFERENCE_SUBSCRIBE_TYPE_IN_DIALOG = 0,
            CONFERENCE_SUBSCRIBE_TYPE_OUT_OF_DIALOG = 1
        };

        // SRVCC types
        enum
        {
            BASIC_SRVCC_SUPPORT = 0,
            ALERTING_SRVCC_SUPPORT = 1,
            PREALERTING_SRVCC_SUPPORT = 2,
            MIDCALL_SRVCC_SUPPORT = 3
        };

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

        // Conference SIP flow order types
        enum
        {
            CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_REFER = 0,
            CONFERENCE_SIP_FLOW_REFER_AND_SUBSCRIBE = 1,
            CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_NOTIFY_REFER = 2
        };

        // Conference invite types
        enum
        {
            CONFERENCE_INVITE_COPYCONTROL = 0,
            CONFERENCE_INVITE_REFER_SINGLE = 1,
            CONFERENCE_INVITE_REFER_MULTIPLE = 2
        };

        // Policy of using QoS precondition mechanism after call established
        enum
        {
            QOS_PRECONDITION_NOT_AVAILABLE = 0,
            QOS_PRECONDITION_BY_MESSAGE_CONTEXT = 1
        };

        // Registration restoration methods (INVITE/504)
        enum
        {
            REGISTRATION_RESTORATION_NOT_AVAILABLE = 0,
            REGISTRATION_RESTORATION_INITIAL_REGISTER_WITH_NEXT_PCSCF = 1,
            REGISTRATION_RESTORATION_RECOVER_REGISTRATION = 2,
            REGISTRATION_RESTORATION_RECOVER_REGISTRATION_WITHOUT_PDN_RECONNECT = 3,
            REGISTRATION_RESTORATION_RECOVER_BY_NETWORK_CONTEXT = 4
        };

        // KEY_SILENT_REDIAL_ULTIMATE_FAILURE_ACTION_INT
        enum
        {
            SILENT_REDIAL_FAILURE_ACTION_TERMINATE = 0,
            SILENT_REDIAL_FAILURE_ACTION_REGISTRATION = 1,
            SILENT_REDIAL_FAILURE_ACTION_CSFB = 2
        };

        // KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY
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
            START_ERROR_ACTION_SILENT_REINVITE_TO_ALTERNATE_PCSCF = 15
        };

        // KEY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY
        enum
        {
            UPDATE_ERROR_ACTION_TERMINATE = 0,
            UPDATE_ERROR_ACTION_RETRY = 1,
            UPDATE_ERROR_ACTION_GLARE_CONDITION = 2,
            UPDATE_ERROR_ACTION_BLOCK_CALL_BY_TIMER = 3
        };

        // KEY_EARLY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY
        enum
        {
            EARLY_UPDATE_ERROR_ACTION_TERMINATE_DIALOG = 0,
            EARLY_UPDATE_ERROR_ACTION_TERMINATE_CALL = 1,
            EARLY_UPDATE_ERROR_ACTION_GLARE_CONDITION = 2,
            EARLY_UPDATE_ERROR_ACTION_BLOCK_CALL_BY_TIMER = 3,
            EARLY_UPDATE_ERROR_ACTION_TIMEOUT = 4
        };

        // KEY_CSFB_BLOCK_CONDITION_INT_ARRAY
        enum
        {
            CSFB_BLOCK_CONDITION_IF_EPS_ONLY_ATTACH = 0,
            CSFB_BLOCK_CONDITION_IN_NR = 1,
            CSFB_BLOCK_CONDITION_IN_WIFI = 2,
            CSFB_BLOCK_CONDITION_IN_ROAMING = 3,
            CSFB_BLOCK_CONDITION_IN_HOME = 4
        };

        // Call handling when QoS is deactivated
        enum
        {
            QOS_DEACTIVATION_POLICY_TERMINATE_CALL = 0,
            QOS_DEACTIVATION_POLICY_MAINTAIN_CALL = 1,
            QOS_DEACTIVATION_POLICY_MODIFY_CALL = 2
        };

        // Media type restriction on cellular
        enum
        {
            MEDIA_TYPE_RESTRICTION_POLICY_RESTRICT_NONE = 0,
            MEDIA_TYPE_RESTRICTION_POLICY_RESTRICT_AUDIO = 1,
            MEDIA_TYPE_RESTRICTION_POLICY_RESTRICT_AUDIO_AND_VIDEO = 2
        };

        // Local number formats
        enum
        {
            NUMBER_FORMAT_HOME_LOCAL = 0,
            NUMBER_FORMAT_GEO_LOCAL = 1,
            NUMBER_FORMAT_GEO_LOCAL_ONLY_IN_ROAMING = 2
        };

        // Call types after merging between the different calls
        enum
        {
            CALL_MERGE_AS_AUDIO = 0,
            CALL_MERGE_AS_AUDIO_VIDEO = 1
        };

        // Policy when 403 response is received
        enum
        {
            SIP_403_POLICY_TERMINATE_CALL = 0,
            SIP_403_POLICY_TERMINATE_CALL_AND_RECOVER_REGISTRATION = 1,
            SIP_403_POLICY_TERMINATE_CALL_AND_REFRESH_REGISTRATION = 2,
            SIP_403_POLICY_CSFB = 3,
            SIP_403_POLICY_CSFB_AND_RECOVER_REGISTRATION = 4
        };

        // Policy for QoS checking while upgrading call
        enum
        {
            QOS_CHECK_POLICY_ON_UPGRADING_CALL_NOT_AVAILABLE = 0,
            QOS_CHECK_POLICY_ON_UPGRADING_CALL_AFTER_UPGRADE = 1,
            QOS_CHECK_POLICY_ON_UPGRADING_CALL_DURING_UPGRADING = 2
        };

        // Conference drop Refer-To URI source type
        enum
        {
            CONFERENCE_DROP_REFER_TO_URI_SOURCE_REFER_TO_URI_FOR_INVITE = 0,
            CONFERENCE_DROP_REFER_TO_URI_SOURCE_USER_ENTITY_IN_CONFERENCE_EVENT_PACKAGE = 1
        };

        // Media type for an incoming INVITE without SDP
        enum
        {
            OFFERLESS_INVITE_MEDIA_TYPE_FULL_CAPABILITY = 0,
            OFFERLESS_INVITE_MEDIA_TYPE_AUDIO = 1
        };

        // Media type for an incoming re-INVITE without SDP
        enum
        {
            OFFERLESS_REINVITE_MEDIA_TYPE_FULL = 0,
            OFFERLESS_REINVITE_MEDIA_TYPE_AUDIO = 1,
            OFFERLESS_REINVITE_MEDIA_TYPE_CURRENT = 2,
            OFFERLESS_REINVITE_MEDIA_TYPE_BY_HISTORY = 3,
            OFFERLESS_REINVITE_MEDIA_TYPE_INITIALLY_OFFERED = 4,
        };

        // Policy for MO call timeout
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

        // Policy for maintaining the call on registration suspended
        enum
        {
            REGISTRATION_SUSPENDED_POLICY_MAINTAIN_NONE = 0,
            REGISTRATION_SUSPENDED_POLICY_MAINTAIN_AUDIO_ONLY = 1,
            REGISTRATION_SUSPENDED_POLICY_MAINTAIN_ALL = 2
        };

        // Index for Geolocation-PIDF information level
        enum
        {
            GEOLOCATION_PIDF_INFO_INDEX_EMERGENCY_CELLULAR = 0,
            GEOLOCATION_PIDF_INFO_INDEX_EMERGENCY_WIFI = 1,
            GEOLOCATION_PIDF_INFO_INDEX_CELLULAR = 2,
            GEOLOCATION_PIDF_INFO_INDEX_WIFI = 3
        };

        // Geolocation-PIDF information level
        enum
        {
            GEOLOCATION_PIDF_INFO_LAT_AND_LONG = 0,
            GEOLOCATION_PIDF_INFO_LAT_AND_LONG_AND_CIVIC = 1,
            GEOLOCATION_PIDF_INFO_COUNTRY_CODE_ONLY = 2,
            GEOLOCATION_PIDF_INFO_COUNTRY_CODE_AND_STATE = 3,
        };

        // Conditions to block geolocation
        enum
        {
            GEOLOCATION_BLOCK_CONDITION_IN_ROAMING = 0,
            GEOLOCATION_BLOCK_CONDITION_FOR_NORMAL_ROUTING_EMERGENCY_CALL = 1,
        };

        // KEY_RAT_CONDITION_FOR_NOT_WAITING_DEDICATED_BEARER_INT_ARRAY
        enum
        {
            NO_WAIT_DEDICATED_BEARER_IN_NR = 0,
            NO_WAIT_DEDICATED_BEARER_IN_EPS_FALLBACK = 1,
            NO_WAIT_DEDICATED_BEARER_IN_EPS_ONLY_ATTACH = 2,
        };

        enum
        {
            PRACK_DELIVERY_FAILURE_POLICY_TERMINATE_DIALOG = 0,
            PRACK_DELIVERY_FAILURE_POLICY_TERMINATE_CALL = 1,
            PRACK_DELIVERY_FAILURE_POLICY_IGNORE = 2,
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
        static const IMS_CHAR KEY_MINIMUM_BATTERY_LEVEL_FOR_LIMIT_VIDEO_CALL_INT[];
        static const IMS_CHAR KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL[];
        static const IMS_CHAR KEY_CHECK_AVCHANGE_FEATURE_FOR_CALL_CONVERTING_CAPABILITY_BOOL[];
        static const IMS_CHAR KEY_VILTE_TO_VOLTE_RETRY_FAILURE_RESPONSE_CODE_INT_ARRAY[];
        static const IMS_CHAR KEY_SUPPORT_VIDEO_CALL_ONLY_IN_VOPS_OFF_STATUS_BOOL[];
        static const IMS_CHAR
                KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL[];
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

        // Policy of supporting text and video media in a call
        enum
        {
            TEXT_VIDEO_NOT_ALLOWED = 0,
            TEXT_VIDEO_NOT_ALLOWED_IF_ACTIVE = 1,
            TEXT_VIDEO_ALLOWED = 2,
        };
    };

    class ImsWfc
    {
    public:
        // Public carrier-config - starts
        static const IMS_CHAR KEY_PIDF_SHORT_CODE_STRING_ARRAY[];
        static const IMS_CHAR KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_BOOL[];
        // Public carrier-config - ends

        // Aos
        static const IMS_CHAR KEY_REGISTRATION_PRIVATE_HEADER_INT[];
        static const IMS_CHAR KEY_COUNTRY_CODE_INT[];
        static const IMS_CHAR KEY_REREG_WITH_CHANGED_COUNTRY_ON_WIFI_BOOL[];
        static const IMS_CHAR KEY_REQUIRED_WFC_BLOCK_BY_AIRPLANE_MODE_BOOL[];
        static const IMS_CHAR KEY_USE_WFC_COUNTRY_CODE_AVAILABILITY_CHECK_BOOL[];
        static const IMS_CHAR KEY_VIDEO_OVER_WIFI_SUPPORTED_WITHOUT_VOICE_BOOL[];
        static const IMS_CHAR KEY_SUB_CONSECUTIVE_RETRY_CNT_FOR_REG_FORBIDDEN_IN_WIFI_INT[];
        static const IMS_CHAR KEY_VOWIFI_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY[];
        // Bundle {
        static const IMS_CHAR KEY_WFC_ERR_MESSAGE_BUNDLE[];
        static const IMS_CHAR KEY_WFC_ERR_REG_403_STRING[];
        static const IMS_CHAR KEY_WFC_ERR_REG_500_STRING[];
        static const IMS_CHAR KEY_WFC_ERR_NOT_SUPPORTED_COUNTRY_STRING[];
        static const IMS_CHAR KEY_WFC_ERR_SUB_403_STRING[];
        static const IMS_CHAR KEY_WFC_ERR_NOTIFY_TERMINATED_STRING[];
        static const IMS_CHAR KEY_WFC_ERR_OTHER_FAILURES_STRING[];
        // }
        // Mtc
        static const IMS_CHAR KEY_ENABLE_FAKE_QOS_CALL_FLOW_ON_WIFI_BOOL[];
        static const IMS_CHAR KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL_INT[];
        static const IMS_CHAR KEY_REJECT_VOWIFI_VOICE_CALL_WHEN_VOWIFI_SETTING_OFF_BOOL[];
        static const IMS_CHAR KEY_OVERRIDE_MEDIA_INACTIVITY_TO_WIFI_LOST_BOOL[];

        // registration private header
        enum
        {
            REGISTRATION_P_NOT_SUPPORTED = 0,
            REGISTRATION_P_CELLULAR_NETWORK_INFO = 1,
            REGISTRATION_P_LAST_ACCESS_NETWORK_INFO = 2
        };

        // WFC error
        enum
        {
            WFC_ERROR_REG_403 = 1,
            WFC_ERROR_REG_500 = 2,
            WFC_ERROR_NOT_SUPPORTED_COUNTRY = 3,
            WFC_ERROR_SUB_403 = 4,
            WFC_ERROR_NOTIFY_TERMINATED = 5,
            WFC_ERROR_OTHER_FAILURES = 6
        };
    };

    class ImsSs
    {
    public:
        // Public carrier-config - starts
        static const IMS_CHAR KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY[];
        static const IMS_CHAR KEY_NETWORK_INITIATED_USSD_OVER_IMS_SUPPORTED_BOOL[];
        // Public carrier-config - ends

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
