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

package com.android.imsstack.core.config;

import android.annotation.NonNull;
import android.os.Build;
import android.os.Parcel;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.text.TextUtils;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.TelephonyManagerProxy;

import java.util.Arrays;
import java.util.List;
import java.util.Set;
import java.util.StringTokenizer;

/** A class for providing the key/value pair configuration. */
public class CarrierConfig {
    // Assets path / files
    public static final String CARRIER_CONFIG = "carrier_config";
    // A default configuration file.
    public static final String DEFAULT_CARRIER_CONFIG_FILE =
            CARRIER_CONFIG + "/carrier_config.xml";
    // Non-Assets file
    public static final String TEST_CARRIER_CONFIG_FILE =
            "test_carrier_config.xml";

    /** Configuration items for IMS common. */
    @SuppressWarnings("deprecation")
    public static final String[] IMS_COMMON_KEYS = {
        CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL,
        CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL,
        CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL,
        CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS,
        CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL,
        CarrierConfigManager.KEY_CARRIER_SUPPORTS_SS_OVER_UT_BOOL,
        CarrierConfigManager.KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL,
        CarrierConfigManager.KEY_CARRIER_USSD_METHOD_INT,
        CarrierConfigManager.KEY_RTT_SUPPORTED_BOOL,
        CarrierConfigManager.KEY_RTT_SUPPORTED_WHILE_ROAMING_BOOL,
        CarrierConfigManager.KEY_CARRIER_VOLTE_TTY_SUPPORTED_BOOL,
        CarrierConfigManager.KEY_CARRIER_VOLTE_OVERRIDE_WFC_PROVISIONING_BOOL,
        CarrierConfigManager.KEY_CARRIER_RCS_PROVISIONING_REQUIRED_BOOL,
        CarrierConfigManager.KEY_USE_RCS_SIP_OPTIONS_BOOL,

        CarrierConfigManager.KEY_SUPPORT_PAUSE_IMS_VIDEO_CALLS_BOOL,
        CarrierConfigManager.KEY_SUPPORT_ADHOC_CONFERENCE_CALLS_BOOL,
        CarrierConfigManager.KEY_SUPPORT_ADD_CONFERENCE_PARTICIPANTS_BOOL,
        CarrierConfigManager.KEY_IS_IMS_CONFERENCE_SIZE_ENFORCED_BOOL,
        CarrierConfigManager.KEY_IMS_CONFERENCE_SIZE_LIMIT_INT,
        CarrierConfigManager.KEY_SUPPORTS_DEVICE_TO_DEVICE_COMMUNICATION_USING_RTP_BOOL,
        CarrierConfigManager.KEY_SUPPORTS_DEVICE_TO_DEVICE_COMMUNICATION_USING_DTMF_BOOL,
        CarrierConfigManager.KEY_SUPPORTS_SDP_NEGOTIATION_OF_D2D_RTP_HEADER_EXTENSIONS_BOOL,
        CarrierConfigManager.KEY_CARRIER_NR_AVAILABILITIES_INT_ARRAY,
        CarrierConfigManager.KEY_RTT_AUTO_UPGRADE_BOOL,
        CarrierConfigManager.KEY_RTT_SUPPORTED_FOR_VT_BOOL,
        CarrierConfigManager.KEY_RTT_UPGRADE_SUPPORTED_BOOL,
        CarrierConfigManager.KEY_RTT_DOWNGRADE_SUPPORTED_BOOL,
        CarrierConfigManager.KEY_RCS_CONFIG_SERVER_URL_STRING,
        CarrierConfigManager.KEY_SUPPORT_MANAGE_IMS_CONFERENCE_CALL_BOOL,
        CarrierConfigManager.KEY_SUPPORT_IMS_CONFERENCE_EVENT_PACKAGE_BOOL,
        CarrierConfigManager.KEY_SUPPORT_IMS_CONFERENCE_EVENT_PACKAGE_ON_PEER_BOOL,
        CarrierConfigManager.KEY_GBA_MODE_INT,

        // Internal usage temporarily
        CarrierConfigManager.KEY_CARRIER_VOLTE_PROVISIONING_REQUIRED_BOOL,
        CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_ENABLED_BOOL,
        CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_MODE_INT,
        CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_ROAMING_ENABLED_BOOL,
        CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_ROAMING_MODE_INT,
        CarrierConfigManager.KEY_EDITABLE_WFC_MODE_BOOL
    };

    public static final String[] IMS_PREFIX_KEYS = {
        CarrierConfigManager.Ims.KEY_WIFI_OFF_DEFERRING_TIME_MILLIS_INT,
        CarrierConfigManager.Ims.KEY_IMS_SINGLE_REGISTRATION_REQUIRED_BOOL,
        CarrierConfigManager.Ims.KEY_ENABLE_PRESENCE_PUBLISH_BOOL,
        CarrierConfigManager.Ims.KEY_PUBLISH_SERVICE_DESC_FEATURE_TAG_MAP_OVERRIDE_STRING_ARRAY,
        CarrierConfigManager.Ims.KEY_ENABLE_PRESENCE_CAPABILITY_EXCHANGE_BOOL,
        CarrierConfigManager.Ims.KEY_RCS_BULK_CAPABILITY_EXCHANGE_BOOL,
        CarrierConfigManager.Ims.KEY_ENABLE_PRESENCE_GROUP_SUBSCRIBE_BOOL,
        CarrierConfigManager.Ims.KEY_SUBSCRIBE_RETRY_DURATION_MILLIS_LONG,
        CarrierConfigManager.Ims.KEY_USE_SIP_URI_FOR_PRESENCE_SUBSCRIBE_BOOL,
        CarrierConfigManager.Ims.KEY_USE_TEL_URI_FOR_PIDF_XML_BOOL,
        CarrierConfigManager.Ims.KEY_NON_RCS_CAPABILITIES_CACHE_EXPIRATION_SEC_INT,
        CarrierConfigManager.Ims.KEY_RCS_FEATURE_TAG_ALLOWED_STRING_ARRAY,
        CarrierConfigManager.Ims.KEY_RCS_REQUEST_FORBIDDEN_BY_SIP_489_BOOL,
        CarrierConfigManager.Ims.KEY_RCS_REQUEST_RETRY_INTERVAL_MILLIS_LONG,
        CarrierConfigManager.Ims.KEY_SIP_TIMER_T1_MILLIS_INT,
        CarrierConfigManager.Ims.KEY_SIP_TIMER_T2_MILLIS_INT,
        CarrierConfigManager.Ims.KEY_SIP_TIMER_T4_MILLIS_INT,
        CarrierConfigManager.Ims.KEY_SIP_TIMER_B_MILLIS_INT,
        CarrierConfigManager.Ims.KEY_SIP_TIMER_C_MILLIS_INT,
        CarrierConfigManager.Ims.KEY_SIP_TIMER_D_MILLIS_INT,
        CarrierConfigManager.Ims.KEY_SIP_TIMER_F_MILLIS_INT,
        CarrierConfigManager.Ims.KEY_SIP_TIMER_H_MILLIS_INT,
        CarrierConfigManager.Ims.KEY_SIP_TIMER_J_MILLIS_INT,
        CarrierConfigManager.Ims.KEY_SIP_SERVER_PORT_NUMBER_INT,
        CarrierConfigManager.Ims.KEY_PHONE_CONTEXT_DOMAIN_NAME_STRING,
        CarrierConfigManager.Ims.KEY_REQUEST_URI_TYPE_INT,
        CarrierConfigManager.Ims.KEY_GRUU_ENABLED_BOOL,
        CarrierConfigManager.Ims.KEY_KEEP_PDN_UP_IN_NO_VOPS_BOOL,
        CarrierConfigManager.Ims.KEY_SIP_PREFERRED_TRANSPORT_INT,
        CarrierConfigManager.Ims.KEY_IPV4_SIP_MTU_SIZE_CELLULAR_INT,
        CarrierConfigManager.Ims.KEY_IPV6_SIP_MTU_SIZE_CELLULAR_INT,
        CarrierConfigManager.Ims.KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY,
        CarrierConfigManager.Ims.KEY_SIP_OVER_IPSEC_ENABLED_BOOL,
        CarrierConfigManager.Ims.KEY_IPSEC_AUTHENTICATION_ALGORITHMS_INT_ARRAY,
        CarrierConfigManager.Ims.KEY_IPSEC_ENCRYPTION_ALGORITHMS_INT_ARRAY,
        CarrierConfigManager.Ims.KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT,
        CarrierConfigManager.Ims.KEY_REGISTRATION_RETRY_BASE_TIMER_MILLIS_INT,
        CarrierConfigManager.Ims.KEY_REGISTRATION_RETRY_MAX_TIMER_MILLIS_INT,
        CarrierConfigManager.Ims.KEY_REGISTRATION_EVENT_PACKAGE_SUPPORTED_BOOL,
        CarrierConfigManager.Ims.KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT,
        CarrierConfigManager.Ims.KEY_GEOLOCATION_PIDF_IN_SIP_REGISTER_SUPPORT_INT_ARRAY,
        CarrierConfigManager.Ims.KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
        CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING,
        CarrierConfigManager.Ims.KEY_SUPPORTED_RATS_INT_ARRAY,
        CarrierConfigManager.Ims.KEY_MMTEL_REQUIRES_PROVISIONING_BUNDLE,
        CarrierConfigManager.Ims.KEY_CAPABILITY_TYPE_VOICE_INT_ARRAY,
        CarrierConfigManager.Ims.KEY_CAPABILITY_TYPE_VIDEO_INT_ARRAY,
        CarrierConfigManager.Ims.KEY_CAPABILITY_TYPE_UT_INT_ARRAY,
        CarrierConfigManager.Ims.KEY_CAPABILITY_TYPE_SMS_INT_ARRAY,
        CarrierConfigManager.Ims.KEY_CAPABILITY_TYPE_CALL_COMPOSER_INT_ARRAY,
        CarrierConfigManager.Ims.KEY_RCS_REQUIRES_PROVISIONING_BUNDLE,
        CarrierConfigManager.Ims.KEY_CAPABILITY_TYPE_OPTIONS_UCE_INT_ARRAY,
        CarrierConfigManager.Ims.KEY_CAPABILITY_TYPE_PRESENCE_UCE_INT_ARRAY,
        CarrierConfigManager.Ims.KEY_NR_SA_DISABLE_POLICY_INT,
        CarrierConfigManager.Ims.KEY_ALLOW_NON_GLOBAL_PHONE_NUMBER_FORMAT_BOOL
    };

    public static final String[] IMS_VOICE_PREFIX_KEYS = {
        CarrierConfigManager.ImsVoice.KEY_CARRIER_VOLTE_ROAMING_AVAILABLE_BOOL,
        CarrierConfigManager.ImsVoice.KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL,
        CarrierConfigManager.ImsVoice.KEY_MULTIENDPOINT_SUPPORTED_BOOL,
        CarrierConfigManager.ImsVoice.KEY_SESSION_TIMER_SUPPORTED_BOOL,
        CarrierConfigManager.ImsVoice.KEY_SESSION_EXPIRES_TIMER_SEC_INT,
        CarrierConfigManager.ImsVoice.KEY_MINIMUM_SESSION_EXPIRES_TIMER_SEC_INT,
        CarrierConfigManager.ImsVoice.KEY_SESSION_REFRESHER_TYPE_INT,
        CarrierConfigManager.ImsVoice.KEY_SESSION_PRIVACY_TYPE_INT,
        CarrierConfigManager.ImsVoice.KEY_PRACK_SUPPORTED_FOR_18X_BOOL,
        CarrierConfigManager.ImsVoice.KEY_CONFERENCE_SUBSCRIBE_TYPE_INT,
        CarrierConfigManager.ImsVoice.KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL,
        CarrierConfigManager.ImsVoice.KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL,
        CarrierConfigManager.ImsVoice.KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT,
        CarrierConfigManager.ImsVoice.KEY_SRVCC_TYPE_INT_ARRAY,
        CarrierConfigManager.ImsVoice.KEY_RINGING_TIMER_MILLIS_INT,
        CarrierConfigManager.ImsVoice.KEY_RINGBACK_TIMER_MILLIS_INT,
        CarrierConfigManager.ImsVoice.KEY_AUDIO_RTP_INACTIVITY_TIMER_MILLIS_INT,
        CarrierConfigManager.ImsVoice.KEY_AUDIO_RTCP_INACTIVITY_TIMER_MILLIS_INT,
        CarrierConfigManager.ImsVoice.KEY_CONFERENCE_FACTORY_URI_STRING,
        CarrierConfigManager.ImsVoice.KEY_SESSION_REFRESH_METHOD_INT,
        CarrierConfigManager.ImsVoice.KEY_OIP_SOURCE_FROM_HEADER_BOOL,
        CarrierConfigManager.ImsVoice.KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT,
        CarrierConfigManager.ImsVoice.KEY_AUDIO_INACTIVITY_CALL_END_REASONS_INT_ARRAY,
        CarrierConfigManager.ImsVoice.KEY_AUDIO_AS_BANDWIDTH_KBPS_INT,
        CarrierConfigManager.ImsVoice.KEY_AUDIO_RS_BANDWIDTH_BPS_INT,
        CarrierConfigManager.ImsVoice.KEY_AUDIO_RR_BANDWIDTH_BPS_INT,
        CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
        CarrierConfigManager.ImsVoice.KEY_EVS_PAYLOAD_TYPE_INT_ARRAY,
        CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY,
        CarrierConfigManager.ImsVoice.KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY,
        CarrierConfigManager.ImsVoice.KEY_DTMFWB_PAYLOAD_TYPE_INT_ARRAY,
        CarrierConfigManager.ImsVoice.KEY_DTMFNB_PAYLOAD_TYPE_INT_ARRAY,
        CarrierConfigManager.ImsVoice.KEY_VOICE_RTP_PACKET_LOSS_RATE_THRESHOLD_INT,
        CarrierConfigManager.ImsVoice.KEY_VOICE_RTP_JITTER_THRESHOLD_MILLIS_INT,
        CarrierConfigManager.ImsVoice.KEY_VOICE_RTP_INACTIVITY_TIME_THRESHOLD_MILLIS_LONG,
        CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
        CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY,
        CarrierConfigManager.ImsVoice.KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE,
        CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE,
        CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT,
        CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT,
        CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY,
        CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT,
        CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT,
        CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_DTX_BOOL,
        CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL,
        CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_CHANNELS_INT,
        CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_CMR_INT,
        CarrierConfigManager.ImsVoice.KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT,
        CarrierConfigManager.ImsVoice.KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT,
        CarrierConfigManager.ImsVoice.KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT,
        CarrierConfigManager.ImsVoice.KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE
    };

    public static final String[] IMS_SMS_PREFIX_KEYS = {
        CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL,
        CarrierConfigManager.ImsSms.KEY_SMS_CSFB_RETRY_ON_FAILURE_BOOL,
        CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_FORMAT_INT,
        CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY,
        CarrierConfigManager.ImsSms.KEY_SMS_MAX_RETRY_COUNT_INT,
        CarrierConfigManager.ImsSms.KEY_SMS_MAX_RETRY_OVER_IMS_COUNT_INT,
        CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SEND_RETRY_DELAY_MILLIS_INT,
        CarrierConfigManager.ImsSms.KEY_SMS_TR1_TIMER_MILLIS_INT,
        CarrierConfigManager.ImsSms.KEY_SMS_TR2_TIMER_MILLIS_INT,
        CarrierConfigManager.ImsSms.KEY_SMS_RP_CAUSE_VALUES_TO_RETRY_OVER_IMS_INT_ARRAY,
        CarrierConfigManager.ImsSms.KEY_SMS_RP_CAUSE_VALUES_TO_FALLBACK_INT_ARRAY
    };

    public static final String[] IMS_RTT_PREFIX_KEYS = {
        CarrierConfigManager.ImsRtt.KEY_TEXT_ON_DEFAULT_BEARER_SUPPORTED_BOOL,
        CarrierConfigManager.ImsRtt.KEY_TEXT_QOS_PRECONDITION_SUPPORTED_BOOL,
        CarrierConfigManager.ImsRtt.KEY_TEXT_AS_BANDWIDTH_KBPS_INT,
        CarrierConfigManager.ImsRtt.KEY_TEXT_RS_BANDWIDTH_BPS_INT,
        CarrierConfigManager.ImsRtt.KEY_TEXT_RR_BANDWIDTH_BPS_INT,
        CarrierConfigManager.ImsRtt.KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
        CarrierConfigManager.ImsRtt.KEY_T140_PAYLOAD_TYPE_INT,
        CarrierConfigManager.ImsRtt.KEY_RED_PAYLOAD_TYPE_INT
    };

    public static final String[] IMS_EMERGENCY_PREFIX_KEYS = {
        CarrierConfigManager.ImsEmergency.KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL,
        CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_CALLBACK_MODE_SUPPORTED_BOOL,
        CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_QOS_PRECONDITION_SUPPORTED_BOOL,
        CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_OVER_IMS_SUPPORTED_RATS_INT_ARRAY,
        CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT,
        CarrierConfigManager.ImsEmergency.KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT,
        CarrierConfigManager.ImsEmergency
                .KEY_EMERGENCY_OVER_IMS_SUPPORTED_3GPP_NETWORK_TYPES_INT_ARRAY,
        CarrierConfigManager.ImsEmergency
                .KEY_EMERGENCY_OVER_IMS_ROAMING_SUPPORTED_3GPP_NETWORK_TYPES_INT_ARRAY,
        CarrierConfigManager.ImsEmergency
                .KEY_EMERGENCY_OVER_CS_SUPPORTED_ACCESS_NETWORK_TYPES_INT_ARRAY,
        CarrierConfigManager.ImsEmergency
                .KEY_EMERGENCY_OVER_CS_ROAMING_SUPPORTED_ACCESS_NETWORK_TYPES_INT_ARRAY,
        CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_DOMAIN_PREFERENCE_INT_ARRAY,
        CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_DOMAIN_PREFERENCE_ROAMING_INT_ARRAY,
        CarrierConfigManager.ImsEmergency.KEY_PREFER_IMS_EMERGENCY_WHEN_VOICE_CALLS_ON_CS_BOOL,
        CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_VOWIFI_REQUIRES_CONDITION_INT,
        CarrierConfigManager.ImsEmergency.KEY_MAXIMUM_NUMBER_OF_EMERGENCY_TRIES_OVER_VOWIFI_INT,
        CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_SCAN_TIMER_SEC_INT,
        CarrierConfigManager.ImsEmergency.KEY_MAXIMUM_CELLULAR_SEARCH_TIMER_SEC_INT,
        CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_NETWORK_SCAN_TYPE_INT,
        CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_CALL_SETUP_TIMER_ON_CURRENT_NETWORK_SEC_INT,
        CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_REQUIRES_IMS_REGISTRATION_BOOL,
        CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_LTE_PREFERRED_AFTER_NR_FAILED_BOOL,
        CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_CDMA_PREFERRED_NUMBERS_STRING_ARRAY,
        CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_REQUIRES_VOLTE_ENABLED_BOOL,
        CarrierConfigManager.ImsEmergency.KEY_CROSS_STACK_REDIAL_TIMER_SEC_INT,
        CarrierConfigManager.ImsEmergency.KEY_QUICK_CROSS_STACK_REDIAL_TIMER_SEC_INT,
        CarrierConfigManager.ImsEmergency
                .KEY_START_QUICK_CROSS_STACK_REDIAL_TIMER_WHEN_REGISTERED_BOOL,
        CarrierConfigManager.ImsEmergency.KEY_SCAN_LIMITED_SERVICE_AFTER_VOLTE_FAILURE_BOOL,
        CarrierConfigManager.ImsEmergency.KEY_IMS_REASONINFO_CODE_TO_RETRY_EMERGENCY_INT_ARRAY
    };

    public static final String[] IMS_VT_PREFIX_KEYS = {
        CarrierConfigManager.ImsVt.KEY_VIDEO_ON_DEFAULT_BEARER_SUPPORTED_BOOL,
        CarrierConfigManager.ImsVt.KEY_VIDEO_RTP_INACTIVITY_TIMER_MILLIS_INT,
        CarrierConfigManager.ImsVt.KEY_VIDEO_RTCP_INACTIVITY_TIMER_MILLIS_INT,
        CarrierConfigManager.ImsVt.KEY_VIDEO_AS_BANDWIDTH_KBPS_INT,
        CarrierConfigManager.ImsVt.KEY_VIDEO_RS_BANDWIDTH_BPS_INT,
        CarrierConfigManager.ImsVt.KEY_VIDEO_RR_BANDWIDTH_BPS_INT,
        CarrierConfigManager.ImsVt.KEY_VIDEO_RTP_DSCP_INT,
        CarrierConfigManager.ImsVt.KEY_VIDEO_QOS_PRECONDITION_SUPPORTED_BOOL,
        CarrierConfigManager.ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
        CarrierConfigManager.ImsVt.KEY_H264_PAYLOAD_TYPE_INT_ARRAY,
        CarrierConfigManager.ImsVt.KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE,
        CarrierConfigManager.ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT,
        CarrierConfigManager.ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT,
        CarrierConfigManager.ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY,
        CarrierConfigManager.ImsVt.KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING
    };

    public static final String[] IMS_WFC_PREFIX_KEYS = {
        CarrierConfigManager.ImsWfc.KEY_PIDF_SHORT_CODE_STRING_ARRAY,
        CarrierConfigManager.ImsWfc.KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_BOOL
    };

    public static final String[] IMS_SS_PREFIX_KEYS = {
        CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL,
        CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_ROAMING_BOOL,
        CarrierConfigManager.ImsSs.KEY_USE_CSFB_ON_XCAP_OVER_UT_FAILURE_BOOL,
        CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_PS_DATA_OFF_BOOL,
        CarrierConfigManager.ImsSs.KEY_NETWORK_INITIATED_USSD_OVER_IMS_SUPPORTED_BOOL,
        CarrierConfigManager.ImsSs.KEY_UT_IPTYPE_HOME_INT,
        CarrierConfigManager.ImsSs.KEY_UT_IPTYPE_ROAMING_INT,
        CarrierConfigManager.ImsSs.KEY_UT_AS_SERVER_FQDN_STRING,
        CarrierConfigManager.ImsSs.KEY_UT_AS_SERVER_PORT_INT,
        CarrierConfigManager.ImsSs.KEY_UT_TRANSPORT_TYPE_INT,
        CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY,
        CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY,
        CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY,
        CarrierConfigManager.ImsSs.KEY_TERMINAL_BASED_CALL_WAITING_SYNC_TYPE_INT,
        CarrierConfigManager.ImsSs.KEY_TERMINAL_BASED_CALL_WAITING_DEFAULT_ENABLED_BOOL
    };

    /** Configuration items for generic IMS. */
    public static class Ims {
        public static final String KEY_PREFIX = "ims.";
        public static final String KEY_SIP_COMPACT_FORM_ENABLED_BOOL =
                KEY_PREFIX + "sip_compact_form_enabled_bool";
        public static final String KEY_ALLOW_SIP_UDP_FALLBACK_ON_TCP_CONNECTION_SETUP_FAILED_BOOL =
                KEY_PREFIX + "allow_sip_udp_fallback_on_tcp_connection_setup_failed_bool";
        public static final String KEY_ALLOW_ALGORITHM_PARAMETER_IN_SIP_AUTHORIZATION_HEADER_BOOL =
                KEY_PREFIX + "allow_algorithm_parameter_in_sip_authorization_header_bool";
        public static final String KEY_USE_SIP_USER_AGENT_HEADER_ONLY_FOR_UA_STRING_BOOL =
                KEY_PREFIX + "use_sip_user_agent_header_only_for_ua_string_bool";
        public static final String KEY_ALLOW_SIP_P_ACCESS_NETWORK_INFO_HEADER_IN_INITIAL_REGISTER_BOOL =
                KEY_PREFIX + "allow_sip_p_access_network_info_header_in_initial_register_bool";
        public static final String KEY_SUPPORT_SIP_SESSION_ID_HEADER_BOOL =
                KEY_PREFIX + "support_sip_session_id_header_bool";
        public static final String KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY =
                KEY_PREFIX + "pcscf_discovery_method_int_array";
        // Aos
        public static final String KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY =
                KEY_PREFIX + "ims_identity_priority_int_array";
        public static final String KEY_ISIM_INDEX_FOR_IMPU_INT =
                KEY_PREFIX + "isim_index_for_impu_int";
        public static final String KEY_UPDATE_REGISTRATION_WITH_RAT_CHANGE_INT_ARRAY =
                KEY_PREFIX + "update_registration_with_rat_change_int_array";
        public static final String KEY_UNSUBSCRIBE_REGISTRATION_EVENT_PACKAGE_BOOL =
                KEY_PREFIX + "unsubscribe_registration_event_package_bool";
        public static final String KEY_PREFERRED_IMS_DSCP_INT =
                KEY_PREFIX + "preferred_ims_dscp_int";
        public static final String KEY_REGISTRATION_PREFERRED_ACCESSTYPE_FEATURE_TAG_INT =
                KEY_PREFIX + "registration_preferred_accesstype_feature_tag_int";
        public static final String KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY =
                KEY_PREFIX + "registration_permanent_error_code_int_array";

        private Ims() {}
    }

    /** Configuration items for ACS. */
    public static class ImsAcs {
        public static final String KEY_PREFIX = "imsacs.";
        public static final String KEY_REFRESH_RATIO_INT =
                KEY_PREFIX + "refresh_ratio_int";
        public static final String KEY_THROTTLING_TIME_SEC_INT =
                KEY_PREFIX + "throttling_time_sec_int";
        public static final String KEY_INITIAL_REQUEST_HTTPS_BOOL =
                KEY_PREFIX + "initial_request_https_bool";
        public static final String KEY_ALWAYS_USE_HTTP_PARAMS_STRING_ARRAY =
                KEY_PREFIX + "always_use_http_params_string_array";
        public static final String KEY_HTTP_PARAMS_STRING_ARRAY =
                KEY_PREFIX + "http_params_string_array";
        public static final String KEY_PERMANENT_ERROR_CODE_INT_ARRAY =
                KEY_PREFIX + "permanent_error_code_int_array";
        public static final String KEY_PDN_TYPE_STRING =
                KEY_PREFIX + "pdn_type_string";
        public static final String KEY_ALWAYS_WORKING_BOOL =
                KEY_PREFIX + "always_working_bool";
        public static final String KEY_USE_PRE_PROVISIONING_BOOL =
                KEY_PREFIX + "use_pre_provisioning_bool";

        private ImsAcs() {}
    }

    /** Configuration items for emergency calling. */
    public static class ImsEmergency {
        public static final String KEY_PREFIX = "imsemergency.";
        public static final String KEY_EMERGENCY_CELLULAR_SCAN_TIMER_MILLIS_INT =
                KEY_PREFIX + "emergency_cellular_scan_timer_millis_int";
        public static final String KEY_EMERGENCY_INVITE_18X_TIMER_MILLIS_INT =
                KEY_PREFIX + "emergency_invite_18x_timer_millis_int";
        public static final String KEY_EMERGENCY_PROVISIONAL_TO_FINAL_RESPONSE_TIMER_MILLIS_INT =
                KEY_PREFIX + "emergency_provisional_to_final_response_timer_millis_int";
        public static final String KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT =
                KEY_PREFIX + "refresh_geolocation_timeout_millis_int";
        // Aos
        public static final String KEY_RELEASE_EMERGENCY_PDN_WITH_EMERGENCY_CALL_END_BOOL =
                KEY_PREFIX + "release_emergency_pdn_with_emergency_call_end_bool";
        public static final String KEY_PREFERRED_EMERGENCY_REGISTRATION_INT =
                KEY_PREFIX + "preferred_emergency_registration_int";
        // Mtc
        public static final String KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_ON_CELLULAR_BOOL =
                KEY_PREFIX + "emergency_call_over_emergency_pdn_on_cellular_bool";
        public static final String KEY_EMERGENCY_RETRY_WITHOUT_CHECKING_380_CONTENT_FOR_NON_UE_DETECTABLE_EMERGENCY_CALL_BOOL =
                KEY_PREFIX + "emergency_retry_without_checking_380_content_for_non_ue_detectable_emergency_call_bool";
        public static final String KEY_EMERGENCY_TCALL_TIMER_MILLIS_INT =
                KEY_PREFIX + "emergency_tcall_timer_millis_int";
        public static final String KEY_EMERGENCY_RINGBACK_TIMER_MILLIS_INT =
                KEY_PREFIX + "emergency_ringback_timer_millis_int";
        public static final String KEY_EMERGENCY_18X_TIMER_MILLIS_INT =
                KEY_PREFIX + "emergency_18x_timer_millis_int";
        public static final String KEY_POLICY_FOR_EMERGENCY_URN_ESCV_MAPPING_INT =
                KEY_PREFIX + "policy_for_emergency_urn_escv_mapping_int";

        private ImsEmergency() {}
    }

    /** Configuration items for RTT. */
    public static class ImsRtt {
        public static final String KEY_PREFIX = "imsrtt.";
        public static final String KEY_POLICY_ON_TEXT_QOS_DEACTIVATION_INT =
                KEY_PREFIX + "policy_on_text_qos_deactivation_int";
        public static final String KEY_TEXT_RTCP_INTERVAL_INT_ARRAY =
                KEY_PREFIX + "text_rtcp_interval_int_array";

        private ImsRtt() {}
    }

    /** Configuration items for SMS over IP. */
    public static class ImsSms {
        public static final String KEY_PREFIX = "imssms.";
        public static final String KEY_SUPPORT_LIMITED_ADMIN_SMS_MODE_BOOL =
                KEY_PREFIX + "support_limited_admin_sms_mode_bool";

        private ImsSms() {}
    }

    /** Configuration items for supplementary service settings. */
    public static class ImsSs {
        public static final String KEY_PREFIX = "imsss.";
        public static final String KEY_XCAP_AUID_PREFIX_STRING =
                KEY_PREFIX + "xcap_auid_prefix_string";
        public static final String KEY_UT_SM_CAUSE_PERMANENT_BLOCK_INT_ARRAY =
                KEY_PREFIX + "ut_sm_cause_permanent_block_int_array";
        public static final String KEY_UT_HTTP_PERMANENT_ERROR_CODE_INT_ARRAY =
                KEY_PREFIX + "ut_http_permanent_error_code_int_array";
        public static final String KEY_TERMINAL_BASED_CALL_WAIT_SYNC_INT =
                KEY_PREFIX + "terminal_based_call_wait_sync_int";
        public static final String KEY_TERMINAL_BASED_CALL_WAIT_DEFAULT_ACTIVATED_BOOL =
                KEY_PREFIX + "terminal_based_call_wait_default_activated_bool";

        private ImsSs() {}
    }

    /** Configuration items for UCE. */
    public static class ImsUce {
        public static final String KEY_PREFIX = "imsuce.";
        public static final String KEY_EXPIRE_VALUE_PUBLISH_SEC_INT =
                KEY_PREFIX + "expire_value_publish_sec_int";
        public static final String KEY_EXTENDED_EXPIRE_VALUE_PUBLISH_SEC_INT =
                KEY_PREFIX + "extended_expire_value_publish_sec_int";
        public static final String KEY_PUBLISH_REFRESH_RATIO_INT =
                KEY_PREFIX + "publish_refresh_ratio_int";
        public static final String KEY_EXPIRE_VALUE_LIST_SUBSCRIBE_SEC_INT =
                KEY_PREFIX + "expire_value_list_subscribe_sec_int";
        public static final String KEY_RLS_URI_STRING =
                KEY_PREFIX + "rls_uri_string";
        public static final String KEY_SUBSCRIBE_INDEPENDENT_OF_PUBLISH_BOOL =
                KEY_PREFIX + "subscribe_independent_of_publish_bool";
        public static final String KEY_ANONYMOUS_FETCH_METHOD_INT =
                KEY_PREFIX + "anonymous_fetch_method_int";
        public static final String KEY_ENCODE_PUBLISH_BODY_BOOL =
                KEY_PREFIX + "encode_publish_body_bool";
        public static final String KEY_ENCODE_SUBSCRIBE_BODY_BOOL =
                KEY_PREFIX + "encode_subscribe_body_bool";
        public static final String KEY_SUPPORT_OPTIONS_BOOL =
                KEY_PREFIX + "support_options_bool";
        public static final String KEY_USE_SIP_URI_FOR_PRESENCE_SUBSCRIBE_BOOL =
                KEY_PREFIX + "use_sip_uri_for_presence_subscribe_bool";
        public static final String KEY_USE_EXPIRED_ETAG_BOOL =
                KEY_PREFIX + "use_expired_etag_bool";
        public static final String KEY_USE_CONTACT_HEADER_IN_PUBLISH_BOOL =
                KEY_PREFIX + "use_contact_header_in_publish_bool";
        public static final String KEY_USE_CONTACT_HEADER_IN_SUBSCRIBE_BOOL =
                KEY_PREFIX + "use_contact_header_in_subscribe_bool";
        public static final String KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE_INT_ARRAY =
                KEY_PREFIX + "immediately_retry_publish_response_int_array";
        public static final String KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE_MAX_COUNT_INT =
                KEY_PREFIX + "immediately_retry_publish_response_max_count_int";
        public static final String KEY_FIXED_TIME_RETRY_PUBLISH_RESPONSE_INT_ARRAY =
                KEY_PREFIX + "fixed_time_retry_publish_response_int_array";
        public static final String KEY_FIXED_TIME_RETRY_PUBLISH_RESPONSE_TIME_SEC_INT =
                KEY_PREFIX + "fixed_time_retry_publish_response_time_sec_int";
        public static final String KEY_VARIABLE_TIME_RETRY_PUBLISH_RESPONSE_INT_ARRAY =
                KEY_PREFIX + "variable_time_retry_publish_response_int_array";
        public static final String KEY_VARIABLE_TIME_RETRY_PUBLISH_RESPONSE_MAX_COUNT_INT =
                KEY_PREFIX + "variable_time_retry_publish_response_max_count_int";
        public static final String KEY_VARIABLE_TIME_RETRY_PUBLISH_RESPONSE_TIME_SEC_INT_ARRAY =
                KEY_PREFIX + "variable_time_retry_publish_response_time_sec_int_array";
        public static final String KEY_REATTEMPT_REGISTRATION_PUBLISH_RESPONSE_INT_ARRAY =
                KEY_PREFIX + "reattempt_registration_publish_response_int_array";
        public static final String KEY_REATTEMPT_REGISTRATION_SUBSCRIBE_RESPONSE_INT_ARRAY =
                KEY_PREFIX + "reattempt_registration_subscribe_response_int_array";

        private ImsUce() {}
    }

    /** Configuration items for voice calling. */
    public static class ImsVoice {
        public static final String KEY_PREFIX = "imsvoice.";
        public static final String KEY_SIP_18X_TIMER_MILLIS_INT =
                KEY_PREFIX + "18x_timer_millis_int";
        public static final String KEY_SUPPORT_CONFERENCE_REFER_SUBSCRIBE_BOOL =
                KEY_PREFIX + "support_conference_refer_subscribe_bool";
        public static final String KEY_ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT_BOOL =
                KEY_PREFIX + "enable_conference_subscribe_by_participant_bool";
        public static final String KEY_CONFERENCE_SIP_FLOW_ORDER_INT =
                KEY_PREFIX + "conference_sip_flow_order_int";
        public static final String KEY_CONFERENCE_INVITING_REFER_TYPE_INT =
                KEY_PREFIX + "conference_inviting_refer_type_int";
        public static final String KEY_POLICY_QOS_PRECONDITION_MECHANISM_WHILE_CALL_MODIFICATION_INT =
                KEY_PREFIX + "policy_qos_precondition_mechanism_while_call_modification_int";
        public static final String KEY_INCOMING_CALL_REJECT_CODE_FOR_USER_DECLINE_INT =
                KEY_PREFIX + "incoming_call_reject_code_for_user_decline_int";
        public static final String KEY_INCOMING_CALL_REJECT_CODE_FOR_NO_ANSWER_INT =
                KEY_PREFIX + "incoming_call_reject_code_for_no_answer_int";
        public static final String KEY_PRACK_UPDATE_RESPONSE_WAIT_TIMER_MILLIS_INT =
                KEY_PREFIX + "prack_update_response_wait_timer_millis_int";
        public static final String KEY_SESSION_REFRESH_TRIGGER_INTERVAL_SEC_INT =
                KEY_PREFIX + "session_refresh_trigger_interval_sec_int";
        public static final String KEY_REGISTRATION_RESTORATION_MODE_ON_504_FOR_INVITE_INT =
                KEY_PREFIX + "registration_restoration_mode_on_504_for_invite_int";
        public static final String KEY_POLICY_ON_AUDIO_QOS_DEACTIVATION_INT =
                KEY_PREFIX + "policy_on_audio_qos_deactivation_int";
        public static final String KEY_ENABLE_SEND_REINVITE_ON_RAT_CHANGE_BOOL =
                KEY_PREFIX + "enable_send_reinvite_on_rat_change_bool";
        public static final String KEY_POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_INT =
                KEY_PREFIX + "policy_for_media_type_restriction_on_cellular_int";
        public static final String KEY_POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_IN_ROAMING_INT =
                KEY_PREFIX + "policy_for_media_type_restriction_on_cellular_in_roaming_int";
        public static final String KEY_POLICY_OF_LOCAL_NUMBERS_INT =
                KEY_PREFIX + "policy_of_local_numbers_int";
        public static final String KEY_DEFAULT_EPS_BEARER_CONTEXT_USAGE_RESTRICTION_ON_CELLULAR_BOOL =
                KEY_PREFIX + "default_eps_bearer_context_usage_restriction_on_cellular_bool";
        public static final String KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT =
                KEY_PREFIX + "silent_redial_interval_millis_int";
        public static final String KEY_CALL_TYPE_AFTER_AUDIO_AND_VIDEO_CALL_MERGED_INT =
                KEY_PREFIX + "call_type_after_audio_and_video_call_merged_int";
        public static final String KEY_SHORT_CALL_CODE_INT_ARRAY =
                KEY_PREFIX + "short_call_code_int_array";
        public static final String KEY_ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL_BOOL =
                KEY_PREFIX + "allow_multiple_call_including_video_call_bool";
        public static final String KEY_REJECT_CODE_FOR_CSFB_INT_ARRAY =
                KEY_PREFIX + "reject_code_for_csfb_int_array";
        public static final String KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT =
                KEY_PREFIX + "silent_redial_max_retry_count_int";
        public static final String KEY_POLICY_FOR_403_RESPONSE_FOR_INVITE_INT =
                KEY_PREFIX + "policy_for_403_response_for_invite_int";
        public static final String KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT =
                KEY_PREFIX + "policy_for_checking_qos_while_call_upgrading_int";
        public static final String KEY_REJECT_OFFERLESS_INVITE_BOOL =
                KEY_PREFIX + "reject_offerless_invite_bool";
        public static final String KEY_CALL_MAX_COUNT_INT = KEY_PREFIX + "call_max_count_int";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING =
                KEY_PREFIX + "call_terminate_reason_header_user_ends_call_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_RTP_TIMEOUT_STRING =
                KEY_PREFIX + "call_terminate_reason_header_rtp_timeout_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_AND_RTP_TIMEOUT_STRING =
                KEY_PREFIX + "call_terminate_reason_header_user_ends_and_rtp_timeout_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_MEDIA_BEARER_LOSS_STRING =
                KEY_PREFIX + "call_terminate_reason_header_media_bearer_loss_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_SIP_TIMEOUT_STRING =
                KEY_PREFIX + "call_terminate_reason_header_sip_timeout_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_SIP_RESPONSE_TIMEOUT_STRING =
                KEY_PREFIX + "call_terminate_reason_header_sip_response_timeout_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_AND_SIP_RESPONSE_TIMEOUT_STRING =
                KEY_PREFIX + "call_terminate_reason_header_user_ends_and_sip_response_timeout_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_CALL_SETUP_TIMEOUT_STRING =
                KEY_PREFIX + "call_terminate_reason_header_call_setup_timeout_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_TERMINATING_EARLYDIALOG_STRING =
                KEY_PREFIX + "call_terminate_reason_header_terminating_earlydialog_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_VOPS_OFF_STRING =
                KEY_PREFIX + "call_terminate_reason_header_vops_off_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_SESSION_REFRESH_FAILURE_STRING =
                KEY_PREFIX + "call_terminate_reason_header_session_refresh_failure_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_CONFERENCE_CALL_JOINED_STRING =
                KEY_PREFIX + "call_terminate_reason_header_conference_call_joined_string";
        public static final String KEY_CALL_REJECT_REASON_PHRASE_ON_CSCALL_STRING =
                KEY_PREFIX + "call_reject_reason_phrase_on_cscall_string";
        public static final String KEY_CALL_REJECT_REASON_PHRASE_ON_VILTE_AND_NO_LTE_STRING =
                KEY_PREFIX + "call_reject_reason_phrase_on_vilte_and_no_lte_string";
        public static final String KEY_CALL_REJECT_REASON_PHRASE_ON_CONNECTING_CALL_STRING =
                KEY_PREFIX + "call_reject_reason_phrase_on_connecting_call_string";
        public static final String KEY_CALL_REJECT_REASON_PHRASE_EXCEEDS_MAX_CALL_COUNT_STRING =
                KEY_PREFIX + "call_reject_reason_phrase_exceeds_max_call_count_string";
        public static final String KEY_CALL_REJECT_REASON_PHRASE_ON_CONVERTING_STRING =
                KEY_PREFIX + "call_reject_reason_phrase_on_converting_string";
        public static final String KEY_CALL_REJECT_REASON_PHRASE_NEGOTIATION_FAILURE_STRING =
                KEY_PREFIX + "call_reject_reason_phrase_negotiation_failure_string";
        public static final String KEY_CALL_REJECT_REASON_PHRASE_NO_ANSWER_BY_USER_STRING =
                KEY_PREFIX + "call_reject_reason_phrase_no_answer_by_user_string";
        public static final String KEY_CALL_REJECT_REASON_PHRASE_VOWIFI_OFF_STRING =
                KEY_PREFIX + "call_reject_reason_phrase_vowifi_off_string";
        public static final String KEY_CALL_REJECT_REASON_PHRASE_USER_REJECT_STRING =
                KEY_PREFIX + "call_reject_reason_phrase_user_reject_string";
        // Media
        public static final String KEY_AUDIO_JITTER_BUFFER_SIZE_INT_ARRAY =
                KEY_PREFIX + "audio_jitter_buffer_size_int_array";
        public static final String KEY_AUDIO_RTCP_INTERVAL_INT_ARRAY =
                KEY_PREFIX + "audio_rtcp_interval_int_array";
        public static final String KEY_EVS_AMRWB_IO_MODE_SET_INT =
                KEY_PREFIX + "evs_amrwb_io_mode_set_int";

        private ImsVoice() {}
    }

    /** Configuration items for video calling. */
    public static class ImsVt {
        public static final String KEY_PREFIX = "imsvt.";
        public static final String KEY_CONVERT_REMOTE_RESPONSE_TIMER_MILLIS_INT =
                KEY_PREFIX + "convert_remote_response_timer_millis_int";
        public static final String KEY_CONVERT_USER_RESPONSE_TIMER_MILLIS_INT =
                KEY_PREFIX + "convert_user_response_timer_millis_int";
        public static final String KEY_POLICY_ON_VIDEO_QOS_DEACTIVATION_INT =
                KEY_PREFIX + "policy_on_video_qos_deactivation_int";
        public static final String KEY_SUPPORT_EARLY_SESSION_BOOL =
                KEY_PREFIX + "support_early_session_bool";
        public static final String KEY_ALLOW_TEXT_WITH_VIDEO_BOOL =
                KEY_PREFIX + "allow_text_with_video_bool";
        public static final String KEY_MINIMUM_BATTERY_LEVEL_FOR_LIMIT_VIDEO_CALL_INT =
                KEY_PREFIX + "minimum_battery_level_for_limit_video_call_int";
        // Media
        public static final String KEY_VIDEO_RTCP_INTERVAL_INT_ARRAY =
                KEY_PREFIX + "video_rtcp_interval_int_array";
        public static final String KEY_VIDEO_AVPF_FEATURE_INT =
                KEY_PREFIX + "video_avpf_feature_int";  // TODO Media - need to update
        public static final String KEY_VIDEO_CODEC_BITRATE_INT_ARRAY =
                KEY_PREFIX + "video_codec_bitrate_int_array";
        public static final String KEY_VIDEO_CODEC_IMAGE_ATTR_STRING_ARRAY =
                KEY_PREFIX + "video_codec_image_attr_string_array";
        public static final String KEY_VIDEO_CODEC_FRAME_SIZE_STRING_ARRAY =
                KEY_PREFIX + "video_codec_frame_size_string_array";
        public static final String KEY_VIDEO_CODEC_HEVC_PROFILE_INT_ARRAY =
                KEY_PREFIX + "video_codec_hevc_profile_int_array";
        public static final String KEY_VIDEO_CODEC_HEVC_LEVEL_INT_ARRAY =
                KEY_PREFIX + "video_codec_hevc_level_int_array";
        public static final String KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING =
                KEY_PREFIX + "h264_video_codec_attribute_profile_level_id_string";
                // TODO Media - item in Bundle - added to change the internal xml for test

        private ImsVt() {}
    }

    /** Configuration items for Wi-Fi calling. */
    public static class ImsWfc {
        public static final String KEY_PREFIX = "imswfc.";
        public static final String KEY_REGISTRATION_PRIVATE_HEADER_INT =
                KEY_PREFIX + "registration_private_header_int";
        public static final String KEY_COUNTRY_CODE_INT =
                KEY_PREFIX + "country_code_int";

        private ImsWfc() {}
    }

    /** Configuration items for internal assets. */
    public static class Assets {
        // Ims General
        public static final String KEY_SDP_NEGOTIATION_REQUIRED_FOR_NON_RPR_BOOL =
                "sdp_negotiation_required_for_non_rpr_bool";
        public static final String KEY_REQUEST_URI_VALIDATION_REQUIRED_IN_MID_DIALOG_BOOL =
                "request_uri_validation_required_in_mid_dialog_bool";
        public static final String KEY_SESSION_TIMER_UPDATE_REQUIRED_IN_SESSION_UPDATE_BY_REINVITE_BOOL =
                "session_timer_update_required_in_session_update_by_reinvite_bool";
        public static final String KEY_ALLOW_SIP_INSTANCE_PARAMETER_IN_CONTACT_FOR_NON_REGISTER_REQUEST_BOOL =
                "allow_sip_instance_parameter_in_contact_for_non_register_request_bool";
        public static final String KEY_SIP_TIMER_100_TRYING_MILLIS_INT =
                "sip_timer_100_trying_millis_int";
        public static final String KEY_USE_RESET_WHEN_CLOSING_SIP_TCP_CONNECTION_BOOL =
                "use_reset_when_closing_sip_tcp_connection_bool";
        public static final String KEY_USE_TUPLE_ELEMENT_FOR_GEOLOCATION_PIDF_BOOL =
                "use_tuple_element_for_geolocation_pidf_bool";
        public static final String KEY_ALLOW_UNKNOWN_COUNTRY_ELEMENT_FOR_GEOLOCATION_PIDF_BOOL =
                "allow_unknown_country_element_for_geolocation_pidf_bool";
        public static final String KEY_SET_SDP_DIRECTION_ATTRIBUTE_FOR_REMOVED_MEDIA_BOOL =
                "set_sdp_direction_attribute_for_removed_media_bool";
        public static final String KEY_SUPPORT_SDP_PRECONDITION_BOOL =
                "support_sdp_precondition_bool";
        public static final String KEY_HIDE_MAC_ADDRESS_IN_PANI_HEADER_BOOL =
                "hide_mac_address_in_pani_header_bool";
        public static final String KEY_SUPPORT_COUNTRY_PARAM_IN_PANI_HEADER_BOOL =
                "support_country_param_in_pani_header_bool";
        public static final String KEY_SUPPORT_LOCAL_SESSION_TIMER_BOOL =
                "support_local_session_timer_bool";
        public static final String KEY_SUPPORT_ECBM_FOR_VOLTE_BOOL =
                "support_ecbm_for_volte_bool";
        public static final String KEY_SUPPORT_ECBM_FOR_VOWIFI_BOOL =
                "support_ecbm_for_vowifi_bool";
        public static final String KEY_LOCATION_POLICY_UPDATE_TYPE_INT =
                "location_policy_update_type_int";
        public static final String KEY_LOCATION_ALLOW_MOCK_LOCATION_UPDATE_BOOL =
                "location_allow_mock_location_update_bool";
        public static final String KEY_LOCATION_ACQUISITION_POLICY_INT =
                "location_acquisition_policy_int";
        public static final String KEY_LOCATION_ADDRESS_RESOLUTION_TIME_MILLIS_INT =
                "location_address_resolution_time_millis_int";
        public static final String KEY_LOCATION_VALIDITY_PERIOD_MIN_INT =
                "location_validity_period_min_int";
        public static final String KEY_LOCATION_ADDRESS_VALIDITY_PERIOD_MIN_INT =
                "location_address_validity_period_min_int";
        public static final String KEY_LOCATION_TOLERABLE_DISTANCE_INT =
                "location_tolerable_distance_int";
        public static final String KEY_LOCATION_GPS_SEARCHING_DURATION_SEC_INT =
                "location_gps_searching_duration_sec_int";
        public static final String KEY_LOCATION_GEODETIC_SHAPE_INT =
                "location_geodetic_shape_int";
        public static final String KEY_CELLULAR_NETWORK_INFO_UTC_OFFSET_ENABLED_BOOL =
                "cellular_network_info_utc_offset_enabled_bool";

        // Aos
        public static final String KEY_CALL_END_AND_PDN_REACTIVATION_BY_REG_TERMINATED_BOOL =
                "call_end_and_pdn_reactivation_by_reg_terminated_bool";
        public static final String KEY_DESTROY_UNSECURE_TCP_SOCKET_ON_ACCOMPLISHING_REG_BOOL =
                "destroy_unsecure_tcp_socket_on_accomplishing_reg_bool";
        public static final String KEY_ECALL_BASED_ON_P_ASSOCIATED_URI_OF_NORMAL_REG_BOOL =
                "ecall_based_on_p_associated_uri_of_normal_reg_bool";
        public static final String KEY_EREG_ON_RANDOM_PCSCF_BOOL =
                "ereg_on_random_pcscf_bool";
        public static final String KEY_EREG_SET_TCP_ONLY_IN_ROAMING_BOOL =
                "ereg_set_tcp_only_in_roaming_bool";
        public static final String KEY_HOLD_REG_WITH_IPCAN_CHANGED_DURING_IMS_CALL_BOOL =
                "hold_reg_with_ipcan_changed_during_ims_call_bool";
        public static final String KEY_IGNORE_VOPS_FOR_VOLTE_ENABLE_BOOL =
                "ignore_vops_for_volte_enable_bool";
        public static final String KEY_IMS_DEREG_ON_3G_NETWORK_BOOL =
                "ims_dereg_on_3g_network_bool";
        public static final String KEY_INIT_IPSEC_SETTING_WITH_NEW_PCSCF_BOOL =
                "init_ipsec_setting_with_new_pcscf_bool";
        public static final String KEY_NO_INIT_REG_ON_PCSCF_CHANGE_BOOL =
                "no_init_reg_on_pcscf_change_bool";
        public static final String KEY_PLMN_BLOCK_WITH_TIMEOUT_ON_VOICE_CALL_UNAVAILABLE_BOOL =
                "plmn_block_with_timeout_on_voice_call_unavailable_bool";
        public static final String KEY_REG_CONTACT_VALIDATION_BOOL =
                "reg_contact_validation_bool";
        public static final String KEY_REG_RETRY_WITH_IP_VER_FALLBACK_BOOL =
                "reg_retry_with_ip_ver_fallback_bool";
        public static final String KEY_REMOVE_OLD_SA_ON_ESTABLISHING_SA_BOOL =
                "remove_old_sa_on_establishing_sa_bool";
        public static final String KEY_REQUIRED_CDMALESS_FEATURE_TAG_BOOL =
                "required_cdmaless_feature_tag_bool";
        public static final String KEY_REQUIRED_INIT_REG_AFTER_IMS_CALL_END_ON_REG_HELD_BOOL =
                "required_init_reg_after_ims_call_end_on_reg_held_bool";
        public static final String KEY_REQUIRED_VOLTE_BLOCK_BY_SSAC_BOOL =
                "required_volte_block_by_ssac_bool";
        public static final String KEY_REQUIRED_WFC_BLOCK_BY_AIRPLANE_MODE_BOOL =
                "required_wfc_block_by_airplane_mode_bool";
        public static final String KEY_REREG_WITH_CHANGED_COUNTRY_ON_WIFI_BOOL =
                "rereg_with_changed_country_on_wifi_bool";
        public static final String KEY_SIP_OVER_IPSEC_ENABLED_IN_ROAMING_BOOL =
                "sip_over_ipsec_enabled_in_roaming_bool";
        public static final String KEY_SMS_OVER_IMS_AVAILABLE_WITHOUT_VOICE_CAPA_BOOL =
                "sms_over_ims_available_without_voice_capa_bool";
        public static final String KEY_SUPPORT_CONTACT_USER_INFO_BOOL =
                "support_contact_user_info_bool";
        public static final String KEY_SUPPORT_EREREG_ON_IPCAN_CHANGE_BOOL =
                "support_erereg_on_ipcan_change_bool";
        public static final String KEY_SUPPORT_REG_WITH_FEATURE_TAG_UNAVAILABLE_BOOL =
                "support_reg_with_feature_tag_unavailable_bool";
        public static final String KEY_SUPPORT_VERSTAT_FOR_REG_BOOL =
                "support_verstat_for_reg_bool";
        public static final String
                KEY_SUPPORT_VOWIFI_CAPABILITY_WHEN_WIFI_ONLY_OR_PREFERRED_IN_ROAMING_BOOL =
                        "support_vowifi_capability_when_wifi_only_or_preferred_in_roaming_bool";
        public static final String KEY_USE_AWT_WHEN_INIT_REG_WITH_NEXT_PCSCF_BOOL =
                "use_awt_when_init_reg_with_next_pcscf_bool";
        public static final String
                KEY_USE_RCS_TELEPHONY_FEATURE_TAG_AS_AVAILABLE_VOICE_CALL_TYPE_BOOL =
                        "use_rcs_telephony_feature_tag_as_available_voice_call_type_bool";
        public static final String KEY_USE_SECURITY_SERVER_PORT_IN_INIT_REG_BOOL =
                "use_security_server_port_in_init_reg_bool";
        public static final String KEY_USE_SECURITY_SERVER_PORT_IN_REG_CONTACT_OF_INIT_REG_BOOL =
                "use_security_server_port_in_reg_contact_of_init_reg_bool";
        public static final String KEY_USE_WFC_COUNTRY_CODE_AVAILABILITY_CHECK_BOOL =
                "use_wfc_country_code_availability_check_bool";
        public static final String KEY_VIDEO_OVER_WIFI_SUPPORTED_WITHOUT_VOICE_BOOL =
                "video_over_wifi_supported_without_voice_bool";

        public static final String KEY_CONTACT_USER_INFO_POLICY_FOR_NON_REG_MESSAGE_INT =
                "contact_user_info_policy_for_non_reg_message_int";
        public static final String KEY_EPDN_PREFERRED_IPTYPE_INT =
                "epdn_preferred_iptype_int";
        public static final String KEY_EREG_RETRY_MAX_CNT_INT = "ereg_retry_max_cnt_int";
        public static final String KEY_EREG_RETRY_TIMER_MILLIS_INT =
                "ereg_retry_timer_millis_int";
        public static final String KEY_GEOLOCATION_PIDF_FORMING_POLICY_INT =
                "geolocation_pidf_forming_policy_int";
        public static final String KEY_IMS_ESTABLISHMENT_TIME_SEC_INT =
                "ims_establishment_time_sec_int";
        public static final String KEY_IMS_PREFERRED_IPTYPE_INT = "ims_preferred_iptype_int";
        public static final String KEY_IMS_SIGNALLING_DSCP_INT = "ims_signalling_dscp_int";
        public static final String KEY_REG_ACTUAL_WAIT_TIME_POLICY_INT =
                "reg_actual_wait_time_policy_int";
        public static final String KEY_REG_OUT_OF_SERVICE_POLICY_INT =
                "reg_out_of_service_policy_int";
        public static final String KEY_REG_PCSCF_UPDATE_POLICY_INT =
                "reg_pcscf_update_policy_int";
        public static final String KEY_REG_RETRY_305_POLICY_INT = "reg_retry_305_policy_int";
        public static final String KEY_REG_RETRY_503_POLICY_INT = "reg_retry_503_policy_int";
        public static final String KEY_REG_RETRY_CNT_ON_SINGLE_PCSCF_INT =
                "reg_retry_cnt_on_single_pcscf_int";
        public static final String KEY_REG_RETRY_CNT_PER_PCSCF_INT = "reg_retry_cnt_per_pcscf_int";
        public static final String KEY_REG_RETRY_CNT_RESET_POLICY_INT =
                "reg_retry_cnt_reset_policy_int";
        public static final String KEY_REG_RETRY_CNT_WITH_IPSEC_ON_AUTH_FAILURE_INT =
                "reg_retry_cnt_with_ipsec_on_auth_failure_int";
        public static final String KEY_REG_RETRY_DEFAULT_POLICY_INT =
                "reg_retry_default_policy_int";
        public static final String KEY_REG_RETRY_TIMER_F_POLICY_INT =
                "reg_retry_timer_f_policy_int";
        public static final String KEY_REG_TIMER_FOR_ECALL_MILLIS_INT =
                "reg_timer_for_ecall_millis_int";
        public static final String KEY_REREG_RETRY_305_POLICY_INT =
                "rereg_retry_305_policy_int";
        public static final String KEY_ROAMING_PREFERRED_EREG_INT = "roaming_preferred_ereg_int";
        public static final String KEY_SIP_MESSAGE_THRESHOLD_FOR_TRANSPORT_CHANGE_INT =
                "sip_message_threshold_for_transport_change_int";
        public static final String KEY_USAT_REG_EVENT_DOWNLOAD_POLICY_INT =
                "usat_reg_event_download_policy_int";
        public static final String KEY_VOLTE_HYS_TIME_INT = "volte_hys_time_sec_int";

        public static final String KEY_PERMANENT_PDN_FAILURE_INT_ARRAY =
                "permanent_pdn_failure_int_array";
        public static final String KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY =
                "reg_err_code_for_pcscf_discovery_int_array";
        public static final String KEY_REG_PERMANENT_ERR_MAX_CNT_INT_ARRAY =
                "reg_permanent_err_max_cnt_int_array";
        public static final String KEY_REG_RETRY_ERR_CODE_WITHOUT_IPSEC_INT_ARRAY =
                "reg_retry_err_code_without_ipsec_int_array";
        public static final String KEY_REREG_ERR_CODE_FOR_CALL_END_INT_ARRAY =
                "rereg_err_code_for_call_end_int_array";
        public static final String KEY_REREG_ERR_CODE_FOR_IMS_PDN_REACTIVATION_INT_ARRAY =
                "rereg_err_code_for_ims_pdn_reactivation_int_array";
        public static final String
                KEY_REREG_ERR_CODE_FOR_INIT_REG_WITH_AVAILABLE_PCSCF_INT_ARRAY =
                        "rereg_err_code_for_init_reg_with_available_pcscf_int_array";
        public static final String KEY_REREG_RETRY_ERR_CODE_FOR_INIT_REG_WITH_SAME_PCSCF_INT_ARRAY =
                "rereg_retry_err_code_for_init_reg_with_same_pcscf_int_array";
        public static final String KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_NEXT_PCSCF_INT_ARRAY =
                "sub_err_code_for_init_reg_with_next_pcscf_int_array";
        public static final String KEY_SUB_ERR_CODE_FOR_STOPPING_BY_EXPIRATION_TIME_INT_ARRAY =
                "sub_err_code_for_stopping_by_expiration_time_int_array";
        public static final String KEY_SUPPORTED_ROAMING_RATS_INT_ARRAY =
                "supported_roaming_rats_int_array";
        public static final String KEY_VOWIFI_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY =
                "vowifi_sub_err_code_for_init_reg_int_array";
        // Bundle {
        public static final String KEY_EXTRA_REG_ERR_BUNDLE =
                "extra_reg_err_bundle";
        public static final String KEY_EXTRA_REG_ERR_CODE_AS_FAILURE_IN_ROAMING_FOR_UPDATE_BOOL =
                "extra_reg_err_code_as_failure_in_roaming_for_update_bool";
        public static final String KEY_EXTRA_REG_ERR_RETRY_CNT_SHARED_FOR_REG_AND_SUB_BOOL =
                "extra_reg_err_retry_cnt_shared_for_reg_and_sub_bool";
        public static final String KEY_EXTRA_REG_ERR_FINAL_TYPE_INT =
                "extra_reg_err_final_type_int";
        public static final String KEY_EXTRA_REG_ERR_MAX_CNT_INT =
                "extra_reg_err_max_cnt_int";
        public static final String
                KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_EPS_5GS_ONLY_ATTACHED_INT =
                        "extra_reg_err_pcscfs_repeated_cnt_for_eps_5gs_only_attached_int";
        public static final String
                KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_LTE_COMBINDED_ATTACHED_INT =
                        "extra_reg_err_pcscfs_repeated_cnt_for_lte_combined_attached_int";
        public static final String KEY_EXTRA_REG_ERR_POLICY_INT =
                "extra_reg_err_policy_int";
        public static final String KEY_EXTRA_REG_ERR_CODE_INT_ARRAY =
                "extra_reg_err_code_int_array";
        public static final String KEY_EXTRA_REG_ERR_CODE_FOR_UPDATE_INT_ARRAY =
                "extra_reg_err_code_for_update_int_array";
        public static final String KEY_EXTRA_REG_ERR_WAIT_TIME_SEC_INT_ARRAY =
                "extra_reg_err_wait_time_sec_int_array";
        // }
        // Bundle {
        public static final String KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE =
                "notify_terminated_for_init_reg_bundle";
        public static final String KEY_NOTIFY_TERMINATED_FOR_INIT_REG_WITH_WAIT_TIME_INT =
                "notify_terminated_for_init_reg_with_wait_time_int";
        public static final String KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY =
                "notify_terminated_for_init_reg_used_event_int_array";
        public static final String
                KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_WITH_WAIT_TIME_INT_ARRAY =
                        "notify_terminated_for_init_reg_used_event_with_wait_time_int_array";
        // }
        // Bundle {
        public static final String KEY_PCSCF_RECOVERY_CONDITIONS_BUNDLE =
                "pcscf_recovery_conditions_bundle";
        public static final String KEY_PCSCF_RECOVERY_MAX_CNT_INT =
                "pcscf_recovery_max_cnt_int";
        public static final String KEY_PCSCF_RECOVERY_WAIT_TIME_SEC_INT =
                "pcscf_recovery_wait_time_sec_int";
        public static final String KEY_PCSCF_RECOVERY_BASE_TIME_SEC_INT =
                "pcscf_recovery_base_time_sec_int";
        public static final String KEY_PCSCF_RECOVERY_MAX_TIME_SEC_INT =
                "pcscf_recovery_max_time_sec_int";
        // }
        // Bundle {
        public static final String KEY_REG_ERR_CODE_WITH_RA_TIME_BUNDLE =
                "reg_err_code_with_ra_time_bundle";
        public static final String KEY_REG_ERR_CODE_WITH_RA_TIME_ONLY_DEFINED_BOOL =
                "reg_err_code_with_ra_time_only_defined_bool";
        public static final String KEY_REG_ERR_CODE_WITH_RA_TIME_INT_ARRAY =
                "reg_err_code_with_ra_time_int_array";
        public static final String KEY_REG_ERR_CODE_WITH_RA_TIME_FOR_UPDATE_INT_ARRAY =
                "reg_err_code_with_ra_time_for_update_int_array";
        // }
        // Bundle {
        public static final String KEY_REG_RETRY_INTERVAL_BUNDLE =
                "reg_retry_interval_bundle";
        public static final String KEY_REG_RETRY_INTERVAL_USED_FOR_SUB_BOOL =
                "reg_retry_interval_used_for_sub_bool";
        public static final String KEY_REG_RETRY_INTERVAL_RANDOM_UPPER_VALUE_SEC_INT_ARRAY =
                "reg_retry_interval_random_upper_value_sec_int_array";
        public static final String KEY_REG_RETRY_INTERVAL_SEC_INT_ARRAY =
                "reg_retry_interval_sec_int_array";
        // }
        // Bundle {
        public static final String KEY_SUB_ERR_CODE_FOR_INIT_REG_BUNDLE =
                "sub_err_code_for_init_reg_bundle";
        public static final String KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_RETRY_MAX_CNT_INT =
                "sub_err_code_for_init_reg_with_retry_max_cnt_int";
        public static final String KEY_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY =
                "sub_err_code_for_init_reg_int_array";
        // }
        // Bundle {
        public static final String KEY_SUB_ERR_CODE_FOR_TERMINATED_BUNDLE =
                "sub_err_code_for_terminated_bundle";
        public static final String KEY_SUB_ERR_CODE_FOR_TERMINATED_WITH_RETRY_MAX_CNT_INT =
                "sub_err_code_for_terminated_with_retry_max_cnt_int";
        public static final String KEY_SUB_ERR_CODE_FOR_TERMINATED_INT_ARRAY =
                "sub_err_code_for_terminated_int_array";
        // }
        // Bundle {
        public static final String KEY_WFC_ERR_MESSAGE_BUNDLE =
                "wfc_err_message_bundle";
        public static final String KEY_WFC_ERR_REG_403_STRING =
                "wfc_err_reg_403_string";
        public static final String KEY_WFC_ERR_REG_500_STRING =
                "wfc_err_reg_500_string";
        public static final String KEY_WFC_ERR_NOT_SUPPORTED_COUNTRY_STRING =
                "wfc_err_not_supported_country_string";
        public static final String KEY_WFC_ERR_SUB_403_STRING =
                "wfc_err_sub_403_string";
        public static final String KEY_WFC_ERR_NOTIFY_TERMINATED_STRING =
                "wfc_err_notify_terminated_string";
        public static final String KEY_WFC_ERR_OTHER_FAILURES_STRING =
                "wfc_err_other_failures_string";
        // }
        // Mtc
        public static final String KEY_CHECK_CONFERENCE_EVENT_PACKAGE_VERSION_BOOL =
                "check_conference_event_package_version_bool";
        public static final String KEY_CONFERENCE_REFER_TO_URI_SOURCE_PAID_BOOL =
                "conference_refer_to_uri_source_paid_bool";
        public static final String KEY_CONFERENCE_DROP_REFER_TO_URI_SOURCE_TYPE_INT =
                "conference_drop_refer_to_uri_source_type_int";
        public static final String KEY_ENABLE_FAKE_QOS_CALL_FLOW_ON_WIFI_BOOL =
                "enable_fake_qos_call_flow_on_wifi_bool";
        public static final String KEY_MEDIA_TYPE_FOR_OFFERLESS_REINVITE_INT =
                "media_type_for_offerless_reinvite_int";
        public static final String KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL =
                "support_video_call_upgrade_regardless_of_feature_tags_bool";
        public static final String KEY_OIP_TYPE_FOR_UNAVAILABLE_INT =
                "oip_type_for_unavailable_int";
        public static final String KEY_ENABLE_OIP_HEADER_POLICY_FALLBACK_BOOL =
                "enable_oip_header_policy_fallback_bool";
        public static final String KEY_EMERGENCY_RTT_GUARD_TIMER_MILLIS_INT =
                "emergency_rtt_guard_timer_millis_int";
        public static final String KEY_RETRY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_WITH_NEXT_PCSCF_BOOL =
                "retry_emergency_call_over_emergency_pdn_with_next_pcscf_bool";
        public static final String KEY_PREALERTING_TIMER_MILLIS_INT =
                "prealerting_timer_millis_int";
        public static final String KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL_INT =
                "policy_for_tcall_timer_expiry_of_volte_call_int";
        public static final String KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_EMERGENCY_CALL_INT =
                "policy_for_tcall_timer_expiry_of_volte_emergency_call_int";
        public static final String KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL_INT =
                "policy_for_tcall_timer_expiry_of_vowifi_call_int";
        public static final String KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY =
                "carrier_specific_sip_headers_string_array";
        public static final String KEY_CHECK_AVCHANGE_FEATURE_FOR_CALL_CONVERTING_CAPABILITY_BOOL =
                "check_avchange_feature_for_call_converting_capability_bool";
        public static final String KEY_SUPPORT_REGISTRATION_RECOVERY_FOR_FAILURE_OF_SESSION_REFRESH_BOOL =
                "support_registration_recovery_for_failure_of_session_refresh_bool";
        public static final String KEY_POLICY_FOR_CALL_MAINTAINING_ON_REGISTRATION_SUSPENDED_INT_ARRAY =
                "policy_for_call_maintaining_on_registration_suspended_int_array";
        public static final String KEY_POLICY_FOR_REQUIRING_EMERGENCY_CALL_WHEN_VIDEO_EMERGENCY_CALL_FAILED_INT_ARRAY =
                "policy_for_requiring_emergency_call_when_video_emergency_call_failed_int_array";
        public static final String KEY_USE_MCID_SUPPLEMENTARY_SERVICE_BOOL =
                "use_mcid_supplementary_service_bool";
        public static final String KEY_USE_MMC_SUPPLEMENTARY_SERVICE_BOOL =
                "use_mmc_supplementary_service_bool";
        public static final String KEY_USE_LTE_PREFERRED_STATUS_FOR_SERVICE_CAPABILITY_BOOL =
                "use_lte_preferred_status_for_service_capability_bool";
        public static final String KEY_ALLOW_INCOMING_HOLD_REQUEST_DURING_CONFERENCE_CALL_BOOL =
                "allow_incoming_hold_request_during_conference_call_bool";
        public static final String KEY_IGNORE_180_AFTER_183_RESPONSE_BOOL =
                "ignore_180_after_183_response_bool";
        public static final String KEY_ADD_REPLACE_HEADER_FOR_CONFERENCE_BOOL =
                "add_replace_header_for_conference_bool";
        public static final String KEY_VILTE_TO_VOLTE_RETRY_FAILURE_RESPONSE_CODE_INT_ARRAY =
                "vilte_to_volte_retry_failure_response_code_int_array";
        public static final String KEY_USE_EMERGENCY_NUMBER_TRANSLATION_IN_ROAMING_STATUS_BOOL =
                "use_emergency_number_translation_in_roaming_status_bool";
        public static final String KEY_IGNORE_PRACK_DELIVERY_FAILURE_BOOL =
                "ignore_prack_delivery_failure_bool";
        public static final String KEY_SUPPORT_VIDEO_CALL_ONLY_IN_VOPS_OFF_STATUS_BOOL =
                "support_video_call_only_in_vops_off_status_bool";
        public static final String KEY_BLOCK_WIFI_EMERGENCY_CALL_IF_NOT_PROVISIONED_BOOL =
                "block_wifi_emergency_call_if_not_provisioned_bool";
        public static final String KEY_REGISTRATION_DISCONNECT_REASON_TO_IGNORE_INT_ARRAY =
                "registration_disconnect_reason_to_ignore_int_array";
        public static final String KEY_WIFI_EMERGENCY_18X_TIMER_MILLIS_INT =
                "wifi_emergency_18x_timer_millis_int";
        public static final String KEY_SUPPORT_CANID_INFO_BOOL =
                "support_canid_info_bool";
        public static final String KEY_USE_CARRIER_SPECIFIC_CONTACT_HEADER_FOR_OPTIONS_RESPONSE_BOOL =
                "use_carrier_specific_contact_header_for_options_response_bool";
        public static final String KEY_USE_CARRIER_SPECIFIC_REJECT_PHRASE_FOR_INCOMING_CALL_DURING_NO_REGISTRATION_BOOL =
                "use_carrier_specific_reject_phrase_for_incoming_call_during_no_registration_bool";
        public static final String KEY_ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_REJECTED_BY_SERVER_ERROR_BOOL =
                "enable_registration_recovery_when_call_rejected_by_server_error_bool";
        public static final String KEY_ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_RETRY_UNAVAILABLE_BOOL =
                "enable_registration_recovery_when_call_retry_unavailable_bool";
        public static final String KEY_REJECT_VOWIFI_VOICE_CALL_WHEN_VOWIFI_SETTING_OFF_BOOL =
                "reject_vowifi_voice_call_when_vowifi_setting_off_bool";
        public static final String KEY_CHECK_SERVER_OUTAGE_REASON_FOR_VXLTE_CALL_BOOL =
                "check_server_outage_reason_for_vxlte_call_bool";
        public static final String KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL =
                "set_video_text_feature_exclusively_in_contact_header_by_session_type_bool";
        public static final String KEY_MAXIMUM_WAIT_TIMER_FOR_GEOLOCATION_PIDF_INFO_MILLIS_INT =
                "maximum_wait_timer_for_geolocation_pidf_info_millis_int";
        public static final String KEY_MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING_BOOL =
                "maintain_multiple_early_sessions_by_forking_bool";
        public static final String KEY_STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY_BOOL =
                "stop_ringback_timer_by_183_with_sdp_body_bool";
        public static final String KEY_INFORMATION_LEVEL_OF_GEOLOCATION_PIDF_INT_ARRAY =
                "information_level_of_geolocation_pidf_int_array";
        public static final String KEY_MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF_INT_ARRAY =
                "message_type_support_geolocation_pidf_int_array";
        public static final String KEY_INCOMING_RESUME_EVENT_SUPPORT_BOOL =
                "incoming_resume_event_support_bool";
        public static final String KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT =
                "sip_status_code_for_rejecting_call_type_change_int";
        public static final String KEY_INITIALIZE_P_EARLY_MEDIA_WHEN_NO_HEADER_BOOL =
                "initialize_p_early_media_when_no_header_bool";
        public static final String KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT =
                "policy_for_local_ringback_tone_with_180_response_int";
        public static final String KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT =
                "eps_fallback_watchdog_time_millis_int";
        public static final String KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT =
                "send_udp_keep_alive_interval_time_millis_int";
        public static final String KEY_CALL_REJECT_CODE_FOR_NOT_ACCEPTABLE_CALL_TYPE_INT =
                "call_reject_code_for_not_acceptable_call_type_int";
        public static final String KEY_SUPPINFO_CDIV_CAUSE_REQUIRED_BOOL =
                "suppinfo_cdiv_cause_required_bool";
        public static final String KEY_ONE_WAY_VIDEO_BY_LOCAL_END_SUPPORTED_BOOL =
                "one_way_video_call_by_local_end_supported_bool";
        public static final String KEY_ONE_WAY_VIDEO_BY_REMOTE_END_SUPPORTED_BOOL =
                "one_way_video_call_by_remote_end_supported_bool";
        public static final String KEY_NOTIFY_CONF_STATE_WHEN_ANONYMOUS_USER_BOOL =
                "notify_conf_state_when_anonymous_user_bool";
        public static final String KEY_GEOLOCATION_POLICY_FOR_LOCATION_BASED_CALL_INT =
                "geolocation_policy_for_location_based_call_int";
        public static final String KEY_LOCATION_BASED_NUMBER_LIST_INT_ARRAY =
                "location_based_number_list_int_array";
        public static final String KEY_CALL_MERGEABLE_ON_CONFERENCE_ON_HOLD_BOOL =
                "call_mergeable_on_conference_on_hold_bool";
        public static final String KEY_RELEASE_EMERGENCY_PDN_WITH_EMERGENCY_CALL_FAIL_BOOL =
                "release_emergency_pdn_with_emergency_call_fail_bool";
        public static final String KEY_POLICY_FOR_ALERT_NOT_USING_PRECONDITION_MECHANISM_INT =
                "policy_for_alert_not_using_precondition_mechanism_int";
        public static final String KEY_EMERGENCY_CALL_CURRENT_LOCATION_DISCOVERY_SUPPORTED_BOOL =
                "emergency_call_current_location_discovery_supported_bool";
        public static final String KEY_CHECK_UI_CONDITION_FOR_INCOMING_RESUME_BOOL =
                "check_ui_condition_for_incoming_resume_bool";

        // Media
        public static final String KEY_AUDIO_RTP_PORT_RANGE_INT_ARRAY =
                "audio_rtp_port_range_int_array";
        public static final String KEY_AUDIO_BW_NEGO_OPTION_BOOL =
                "audio_bw_nego_option_bool";
        public static final String KEY_AUDIO_PTIME_MILLIS_INT =
                "audio_ptime_millis_int";
        public static final String KEY_AUDIO_MAXPTIME_MILLIS_INT =
                "audio_maxptime_millis_int";
        public static final String KEY_AUDIO_MAXRED_INT =
                "audio_maxred_int";
        public static final String KEY_AUDIO_RTP_DSCP_INT =
                "audio_rtp_dscp_int";
        public static final String KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_MODESET_BOOL =
                "audio_show_codec_attribute_modeset_bool";
        public static final String KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_DTX_BOOL =
                "audio_show_codec_attribute_dtx_bool";
        public static final String KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_AMRWBIO_MODESET_BOOL =
                "audio_show_codec_attribute_amrwbio_modeset_bool";
        public static final String KEY_AUDIO_AMRWB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY =
                "audio_amrwb_codec_attribute_default_modeset_int_array";
        public static final String KEY_AUDIO_AMRNB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY =
                "audio_amrnb_codec_attribute_default_modeset_int_array";
        public static final String KEY_AUDIO_TELEPHONE_EVENT_DURATION_MILLIS_INT =
                "audio_telephone_event_duration_millis_int";
        public static final String KEY_AUDIO_CANDIDATE_ATTRIBUTE_STRING_ARRAY =
                "audio_candidate_attribute_string_array";
        public static final String KEY_AUDIO_RTCPXR_ENABLE_BOOL =
                "audio_rtcpxr_enable_bool";
        public static final String KEY_AUDIO_RTCPXR_STATISTICS_BOOL =
                "audio_rtcpxr_statistics_bool";
        public static final String KEY_AUDIO_RTCPXR_VOIP_METRICS_BOOL =
                "audio_rtcpxr_voip_metrics_bool";
        public static final String KEY_AUDIO_RTCPXR_PACKET_LOSS_RLE_BOOL =
                "audio_rtcpxr_packet_loss_rle_bool";
        public static final String KEY_AUDIO_RTCPXR_PACKET_DUPLICATE_RLE_BOOL =
                "audio_rtcpxr_packet_duplicate_rle_bool";
        public static final String KEY_AVC_SPROP_PARAMETER_SETS_STRING =
                "avc_sprop_parameter_sets_string";
        public static final String KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY =
                "hevc_payload_type_int_array";
        public static final String KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE =
                "hevc_payload_description_bundle";
        public static final String KEY_HEVC_SPROP_PARAMETER_SETS_STRING =
                "hevc_sprop_parameter_sets_string";
        public static final String KEY_HEVC_PROFILE_INT =
                "hevc_profile_int";
        public static final String KEY_HEVC_LEVEL_INT =
                "hevc_level_int";
        public static final String KEY_VIDEO_RTP_PORT_RANGE_INT_ARRAY =
                "video_rtp_port_range_int_array";
        public static final String KEY_VIDEO_CVO_VALUE_INT =
                "video_cvo_value_int";
        public static final String KEY_VIDEO_SDP_OFFER_CAP_NEGO_FOR_AVPF_INT =
                "video_sdp_offer_cap_nego_for_avpf_int";
        public static final String KEY_VIDEO_AVPF_ENABLE_BOOL =
                "video_avpf_enable_bool";
        public static final String KEY_VIDEO_IFRAME_INTERVAL_SEC_INT =
                "video_iframe_interval_sec_int";
        public static final String KEY_VIDEO_SEND_PERIODIC_SPS_PPS_INT =
                "video_send_periodic_sps_pps_int";
        public static final String KEY_VIDEO_BW_NEGO_OPTION_BOOL =
                "video_bw_nego_option_bool";
        public static final String KEY_VIDEO_LOWEST_BITRATE_BPS_INT =
                "video_lowest_bitrate_bps_int";
        public static final String KEY_TEXT_RTP_PORT_RANGE_INT_ARRAY =
                "text_rtp_port_range_int_array";
        public static final String KEY_TEXT_CODEC_EMPTY_REDUNDANT_BOOL =
                "text_codec_empty_redundant_bool";
        public static final String KEY_TEXT_RTP_DSCP_INT =
                "text_rtp_dscp_int";
        public static final String KEY_SUPPORT_MULTI_CONFIG_IN_EARLY_SESSION_BOOL =
                "support_multi_config_in_early_session_bool";
        public static final String KEY_AUDIO_EVS_SUPPORT_BOOL =
                "audio_evs_support_bool";
        public static final String KEY_MEDIA_ANBR_CAPABILITY_IN_MODEM_BOOL =
                "media_anbr_capability_in_modem_bool";
        public static final String KEY_MEDIA_SESSION_LEVEL_BANDWIDTH_BOOL =
                "media_session_level_bandwidth_bool";
        public static final String KEY_SDP_ANSWER_FULL_CAPABILITY_BOOL =
                "sdp_answer_full_capability_bool";
        public static final String KEY_SDP_REOFFER_FULL_CAPABILITY_BOOL =
                "sdp_reoffer_full_capability_bool";
        public static final String KEY_TEXT_CODEC_REDUNDANCY_LEVEL_INT =
                "text_codec_redundancy_level_int";
        public static final String KEY_TEXT_RTP_INACTIVITY_TIMER_MILLIS_INT =
                "text_rtp_inactivity_timer_millis_int";
        public static final String KEY_TEXT_RTCP_INACTIVITY_TIMER_MILLIS_INT =
                "text_rtcp_inactivity_timer_millis_int";
        public static final String KEY_DYNAMIC_VIDEO_QUALITY_SUPPORTED_BOOL =
                "dynamic_video_quality_supported_bool";

        // TODO Media additional for CallFeature
        public static final String KEY_AUDIO_HOLD_WITH_DIRECTION_INACTIVE_BOOL =
                "audio_hold_with_direction_inactive_bool";
        public static final String KEY_VIDEO_HOLD_WITH_DIRECTION_INACTIVE_BOOL =
                "video_hold_with_direction_inactive_bool";
        public static final String KEY_TEXT_HOLD_WITH_DIRECTION_INACTIVE_BOOL =
                "text_hold_with_direction_inactive_bool";

        // Mts
        public static final String KEY_SMS_REQUEST_URI_TYPE_INT =
                "sms_request_uri_type_int";
        public static final String KEY_SMS_USE_DIALED_NUMBER_FOR_REQUEST_URI_BOOL =
                "sms_use_dialed_number_for_request_uri_bool";
        public static final String KEY_SMS_ALLOW_IMSI_BASED_SIP_URI_BOOL =
                "sms_allow_imsi_based_sip_uri_bool";
        public static final String KEY_SMS_GEOLOCATION_PIDF_FOR_EMERGENCY_BOOL =
                "sms_geolocation_pidf_for_emergency_bool";

        // Ut/Xcap
        public static final String KEY_UT_SM_CAUSE_TEMPORARY_BLOCK_INT_ARRAY =
                "ut_sm_cause_temporary_block_int_array";
        public static final String KEY_UT_HTTP_TEMPORARY_ERROR_CODE_INT_ARRAY =
                "ut_http_temporary_error_code_int_array";
        public static final String KEY_UT_MAX_RETRY_COUNT_INT = "ut_max_retry_count_int";
        public static final String KEY_UT_TEMPORARY_BLOCK_TIMER_MIN_INT =
                "ut_temporary_block_timer_min_int";
        public static final String KEY_UT_SUPPORT_CF_ACTION_ERASURE_BOOL =
                "ut_support_cf_action_erasure_bool";
        public static final String KEY_UT_QUERY_CF_ALL_AND_CF_ALL_CONDITIONAL_SUPPORT_BOOL =
                "ut_query_cf_all_and_cf_all_conditional_support_bool";
        public static final String KEY_UT_OIR_NETWORK_DEFAULT_OPERATION_INT =
                "ut_oir_network_default_operation_int";
        public static final String KEY_UT_OIR_TIR_ALWAYS_TEMPORARY_MODE_BOOL =
                "ut_oir_tir_always_temporary_mode_bool";
        public static final String KEY_UT_TEMPORARY_BLOCK_TIMER_WITH_ANY_REASON_SEC_INT =
                "ut_temporary_block_timer_with_any_reason_sec_int";
        public static final String KEY_UT_TARGET_ADDRESS_PHONE_CONTEXT_STRING =
                "ut_target_address_phone_context_string";
        public static final String KEY_UT_TARGET_ADDRESS_COUNTRY_CODE_REPLACE_TO_ZERO_STRING =
                "ut_target_address_country_code_replace_to_zero_string";
        public static final String KEY_UT_TARGET_ADDRESS_ZERO_REPLACE_TO_COUNTRY_CODE_STRING =
                "ut_target_address_zero_replace_to_country_code_string";
        public static final String KEY_UT_XCAP_APN_INACTIVITY_TIMER_SEC_INT =
                "ut_xcap_apn_inactivity_timer_sec_int";
        public static final String KEY_UT_DISPLAY_ERROR_PHRASE_WITH_409_ERROR_BOOL =
                "ut_display_error_phrase_with_409_error_bool";
        public static final String KEY_UT_OMIT_NAMESPACE_OF_DOCUMENT_ELEMENT_BOOL =
                "ut_omit_namespace_of_document_element_bool";
        public static final String KEY_UT_OMIT_NAMESPACE_SS_BOOL = "ut_omit_namespace_ss_bool";
        public static final String KEY_UT_OMIT_NAMESPACE_CP_BOOL = "ut_omit_namespace_cp_bool";
        public static final String KEY_UT_INSERT_NEW_RULE_BOOL = "ut_insert_new_rule_bool";
        public static final String KEY_UT_URI_TYPE_FOR_CF_TARGET_NUMBER_INT =
                "ut_uri_type_for_cf_target_number_int";
        // Uce
        public static final String KEY_ADD_VIDEO_TAG_CONTACT_HEADER_IN_PUBLISH_BOOL =
                "add_video_tag_contact_header_in_publish_bool";

        // Indicates preferred IP version
        public static final int IPV4_PREFERRED = 0;
        public static final int IPV6_PREFERRED = 1;

        // Indicates whether location information policy need to be updated using carrier config.
        public static final int LOCATION_UPDATE_POLICY_NONE = 0;
        public static final int LOCATION_UPDATE_POLICY_ONLY_WHEN_WFC_ENABLED = 1;
        public static final int LOCATION_UPDATE_POLICY_ALWAYS = 2;

        // Indicates the type of PIDF-LO geodetic shape.
        public static final int GEODETIC_SHAPE_CIRCLE = 0;
        public static final int GEODETIC_SHAPE_ELLIPSOID = 1;

        private Assets() {}
    }

    /**
     * Configuration items for the AP IMS hidden key.
     * The AP IMS hidden key contains internal settings of AP IMS as a nested bundle.
     **/
    public static class ApIms {
        public static final String KEY_PREFIX = "apims.";
        public static final String KEY_CARRIER_CONFIG_BUNDLE =
                KEY_PREFIX + "carrier_config_bundle";

        private ApIms() {
        }
    }

    // PAYLOAD_DESCRIPTION_BUNDLE is excluded from this list because it has a nested bundle.
    private static final List<String> IMS_BUNDLE_KEYS = List.of(
            CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
            CarrierConfigManager.ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
            CarrierConfigManager.ImsRtt.KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
            Assets.KEY_EXTRA_REG_ERR_BUNDLE,
            Assets.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE,
            Assets.KEY_REG_RETRY_INTERVAL_BUNDLE,
            Assets.KEY_REG_ERR_CODE_WITH_RA_TIME_BUNDLE,
            Assets.KEY_SUB_ERR_CODE_FOR_INIT_REG_BUNDLE,
            Assets.KEY_SUB_ERR_CODE_FOR_TERMINATED_BUNDLE,
            Assets.KEY_WFC_ERR_MESSAGE_BUNDLE);

    private final PersistableBundle mConfig = new PersistableBundle();

    public CarrierConfig() {}

    /**
     * Sets the configuration with the given configuration and slot.
     *
     * @param config The configuration to be set.
     * @param slotId The slot id.
     */
    public void setConfig(PersistableBundle config, int slotId) {
        mConfig.clear();
        mConfig.putAll(config);

        adjustSpecialKeys(slotId);
    }

    /**
     * Returns the string value for a specified key.
     *
     * @param key The config key
     * @return A string value if present. Otherwise, returns null.
     */
    public String getString(String key) {
        return getString(key, null);
    }

    /**
     * Returns the string value for a specified key.
     *
     * @param key The config key
     * @param defaultValue The default value if not present
     * @return A string value if present. Otherwise, returns a default value.
     */
    public String getString(String key, String defaultValue) {
        return mConfig.getString(key, defaultValue);
    }

    /**
     * Returns the boolean value for a specified key.
     *
     * @param key The config key
     * @return A boolean value.
     */
    public boolean getBoolean(String key) {
        return getBoolean(key, false);
    }

    /**
     * Returns the boolean value for a specified key.
     *
     * @param key The config key
     * @param defaultValue The default value if not present
     * @return A boolean value.
     */
    public boolean getBoolean(String key, boolean defaultValue) {
        return mConfig.getBoolean(key, defaultValue);
    }

    /**
     * Returns the integer value for a specified key.
     *
     * @param key The config key
     * @return An integer value if present. Otherwise, returns a negative value.
     */
    public int getInt(String key) {
        return getInt(key, -1);
    }

    /**
     * Returns the integer value for a specified key.
     *
     * @param key The config key
     * @param defaultValue The default value if not present
     * @return An integer value if present. Otherwise, returns a default value.
     */
    public int getInt(String key, int defaultValue) {
        return mConfig.getInt(key, defaultValue);
    }

    /**
     * Returns the long value for a specified key.
     *
     * @param key The config key
     * @return A long value if present. Otherwise, returns a negative value.
     */
    public long getLong(String key) {
        return getLong(key, -1L);
    }

    /**
     * Returns the long value for a specified key.
     *
     * @param key The config key
     * @param defaultValue The default value if not present
     * @return A long value if present. Otherwise, returns a default value.
     */
    public long getLong(String key, long defaultValue) {
        return mConfig.getLong(key, defaultValue);
    }

    /**
     * Returns the boolean-array value for a specified key.
     *
     * @param key The config key
     * @return A boolean-array value if present. Otherwise, returns an empty array.
     */
    public boolean[] getBooleanArray(String key) {
        return mConfig.getBooleanArray(key);
    }

    /**
     * Returns the integer-array value for a specified key.
     *
     * @param key The config key
     * @return An integer-array value if present. Otherwise, returns an empty array.
     */
    public int[] getIntArray(String key) {
        return mConfig.getIntArray(key);
    }

    /**
     * Returns the long-array value for a specified key.
     *
     * @param key The config key
     * @return A long-array value if present. Otherwise, returns an empty array.
     */
    public long[] getLongArray(String key) {
        return mConfig.getLongArray(key);
    }

    /**
     * Returns the string-array value for a specified key.
     *
     * @param key The config key
     * @return A string-array value if present. Otherwise, returns an empty array.
     */
    public String[] getStringArray(String key) {
        return mConfig.getStringArray(key);
    }

    /**
     * Returns the PersistableBundle value for a specified key.
     *
     * @param key The config key
     * @return A PersistableBundle object if present. Otherwise, returns null.
     */
    public PersistableBundle getBundle(String key) {
        return mConfig.getPersistableBundle(key);
    }

    /**
     * Returns the total configuration.
     */
    public PersistableBundle getConfig() {
        return mConfig;
    }

    /**
     * Writes the configuration to the given {@link Parcel} object.
     *
     * @param p The {@link Parcel} object.
     */
    public void writeToParcel(Parcel p) {
        mConfig.writeToParcel(p, 0);
    }

    /**
     * Returns a flag indicating whether the VoLTE provisioning is required or not.
     *
     * @return {@code true} if VoLTE provisioning is required, {@code false} otherwise.
     */
    @SuppressWarnings("deprecation")
    public boolean isVoLteProvisioningRequired() {
        return getBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_PROVISIONING_REQUIRED_BOOL);
    }

    /**
     * Returns the value from the carrier-config that matches with the given key.
     *
     * @param config The {@link PersistableBundle} containing the carrier configuration
     * @param key The key string to find the value
     * @return A string representation of the value for the given key.
     */
    public static String getValue(@NonNull PersistableBundle config, @NonNull String key) {
        if (key.endsWith("_bool") || key.endsWith("_boolean")) {
            return String.valueOf(config.getBoolean(key, false));
        } else if (key.endsWith("_int")) {
            return String.valueOf(config.getInt(key, -1));
        } else if (key.endsWith("_long")) {
            return String.valueOf(config.getLong(key, -1L));
        } else if (key.endsWith("_string")) {
            return config.getString(key);
        } else if (key.endsWith("_double")) {
            return String.valueOf(config.getDouble(key, 0.0));
        } else if (key.endsWith("_bool_array")) {
            return Arrays.toString(config.getBooleanArray(key));
        } else if (key.endsWith("_int_array")) {
            return Arrays.toString(config.getIntArray(key));
        } else if (key.endsWith("_long_array")) {
            return Arrays.toString(config.getLongArray(key));
        } else if (key.endsWith("_string_array")) {
            return Arrays.toString(config.getStringArray(key));
        } else if (key.endsWith("_double_array")) {
            return Arrays.toString(config.getDoubleArray(key));
        } else if (key.endsWith("_bundle") || TextUtils.isDigitsOnly(key)) {
            // isDigitsOnly: codec payload number
            PersistableBundle b = config.getPersistableBundle(key);
            if (b != null) {
                Set<String> keys = b.keySet();
                StringBuilder sb = new StringBuilder();
                sb.append("{ ");
                for (String k : keys) {
                    sb.append(k).append("=").append(getValue(b, k)).append(",");
                }
                sb.append(" }");
                return sb.toString();
            }
            return "null";
        } else if (key.equals(
                CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS)) {
            return String.valueOf(config.getBoolean(key, false));
        }

        return "UnknownKeyType";
    }

    /**
     * Overrides the bundle item - merge two configurations ({@code defaultConfig}: low priority,
     * {@code overrideConfig}: high priority) and puts the updated value to the
     * {@code overrideConfig} argument.
     *
     * @param defaultConfig A configuration with lower priority
     * @param overrideConfig A configuration with higher priority
     */
    public static void overrideNestedBundles(PersistableBundle defaultConfig,
            PersistableBundle overrideConfig) {
        for (int i = 0; i < IMS_BUNDLE_KEYS.size(); ++i) {
            final String key = IMS_BUNDLE_KEYS.get(i);
            PersistableBundle newBundle = new PersistableBundle();
            PersistableBundle defaultBundle = defaultConfig.getPersistableBundle(key);
            PersistableBundle overrideBundle = overrideConfig.getPersistableBundle(key);

            if (defaultBundle != null) {
                newBundle.putAll(defaultBundle);
            }

            if (overrideBundle != null) {
                newBundle.putAll(overrideBundle);
            }

            overrideConfig.putPersistableBundle(key, newBundle);
        }

        overridePayloadDescriptionBundles(defaultConfig, overrideConfig);
    }

    private void adjustSpecialKeys(int slotId) {
        // User-Agent string
        String uaString = mConfig.getString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING);

        if (!TextUtils.isEmpty(uaString) && uaString.contains("#")) {
            StringTokenizer st = new StringTokenizer(uaString, "#");
            StringBuilder sb = new StringBuilder();

            while (st.hasMoreTokens()) {
                String token = st.nextToken();

                switch (token) {
                    case "MANUFACTURER": // FALL-THROUGH
                    case "MANUFACTURE":
                        sb.append(Build.MANUFACTURER);
                        break;
                    case "MODEL":
                        sb.append(Build.MODEL);
                        break;
                    case "AV":
                        sb.append(Build.VERSION.RELEASE);
                        break;
                    case "BUILD":
                        sb.append(Build.ID);
                        break;
                    case "IMEI":
                        sb.append(getImei(slotId));
                        break;
                    case "IMEISV":
                        sb.append(getImeiSv(slotId));
                        break;
                    default:
                        sb.append(token);
                        break;
                }
            }

            mConfig.putString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING, sb.toString());
        }
    }

    private static String getImei(int slotId) {
        final String emptyImei = "000000000000000";
        TelephonyManagerProxy tmp =
                AppContext.getInstance().getSystemServiceProxy(TelephonyManagerProxy.class);
        String imei = tmp.getImei(slotId);
        return TextUtils.isEmpty(imei) ? emptyImei : imei;
    }

    private static String getImeiSv(int slotId) {
        final String emptyImeiWithoutCheckDigit = "00000000000000";
        TelephonyManagerProxy tmp =
                AppContext.getInstance().getSystemServiceProxy(TelephonyManagerProxy.class);
        String imei = tmp.getImei(slotId);

        if (TextUtils.isEmpty(imei)) {
            imei = emptyImeiWithoutCheckDigit;
        } else {
            imei = imei.substring(0, imei.length() - 1);
        }

        String sv = tmp.getDeviceSoftwareVersion(slotId);

        if (TextUtils.isEmpty(sv)) {
            sv = "00";
        } else if (sv.length() == 1) {
            sv = "0" + sv;
        }

        return imei + sv;
    }

    private static void overridePayloadDescriptionBundles(PersistableBundle defaultConfig,
            PersistableBundle overrideConfig) {
        overridePayloadDescriptionBundle(defaultConfig, overrideConfig,
                CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
                CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY,
                CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE);

        overridePayloadDescriptionBundle(defaultConfig, overrideConfig,
                CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
                CarrierConfigManager.ImsVoice.KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY,
                CarrierConfigManager.ImsVoice.KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE);

        overridePayloadDescriptionBundle(defaultConfig, overrideConfig,
                CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
                CarrierConfigManager.ImsVoice.KEY_EVS_PAYLOAD_TYPE_INT_ARRAY,
                CarrierConfigManager.ImsVoice.KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE);

        overridePayloadDescriptionBundle(defaultConfig, overrideConfig,
                CarrierConfigManager.ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
                CarrierConfigManager.ImsVt.KEY_H264_PAYLOAD_TYPE_INT_ARRAY,
                CarrierConfigManager.ImsVt.KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE);
    }

    private static void overridePayloadDescriptionBundle(PersistableBundle defaultConfig,
            PersistableBundle overrideConfig, String payloadTypesBundleKey, String payloadTypesKey,
            String payloadDescriptionBundleKey) {
        PersistableBundle defaultPayloadDescriptionBundle =
                defaultConfig.getPersistableBundle(payloadDescriptionBundleKey);

        if (defaultPayloadDescriptionBundle == null || defaultPayloadDescriptionBundle.isEmpty()) {
            // Use a prioritized configuration (overrideConfig) if present
            return;
        }

        PersistableBundle overridePayloadTypesBundle =
                overrideConfig.getPersistableBundle(payloadTypesBundleKey);
        int[] overridePayloadTypes = (overridePayloadTypesBundle != null)
                ? overridePayloadTypesBundle.getIntArray(payloadTypesKey) : null;

        if (overridePayloadTypes == null || overridePayloadTypes.length == 0) {
            // Use a default configuration
            return;
        }

        PersistableBundle newPayloadDescriptionBundle = new PersistableBundle();
        PersistableBundle overridePayloadDescriptionBundle =
                overrideConfig.getPersistableBundle(payloadDescriptionBundleKey);

        if (overridePayloadDescriptionBundle == null
                || overridePayloadDescriptionBundle.isEmpty()) {
            newPayloadDescriptionBundle.putAll(defaultPayloadDescriptionBundle);
        } else {
            for (int i = 0; i < overridePayloadTypes.length; ++i) {
                final String payloadKey = String.valueOf(overridePayloadTypes[i]);
                PersistableBundle newPayloadBundle = new PersistableBundle();
                PersistableBundle defaultPayloadBundle =
                        defaultPayloadDescriptionBundle.getPersistableBundle(payloadKey);
                PersistableBundle overridePayloadBundle =
                        overridePayloadDescriptionBundle.getPersistableBundle(payloadKey);

                if (defaultPayloadBundle != null) {
                    newPayloadBundle.putAll(defaultPayloadBundle);
                }

                if (overridePayloadBundle != null) {
                    newPayloadBundle.putAll(overridePayloadBundle);
                }

                newPayloadDescriptionBundle.putPersistableBundle(payloadKey, newPayloadBundle);
            }
        }

        overrideConfig.putPersistableBundle(
                payloadDescriptionBundleKey, newPayloadDescriptionBundle);
    }
}
