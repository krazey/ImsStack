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

import android.os.Build;
import android.os.Parcel;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.imsstack.util.AppContext;

import java.util.StringTokenizer;

public class CarrierConfig {
    // Assets path / files
    public static final String CARRIER_CONFIG = "carrier_config";
    public static final String DEFAULT_CARRIER_CONFIG_FILE =
            CARRIER_CONFIG + "/carrier_config.xml";
    // Non-Assets file
    public static final String TEST_CARRIER_CONFIG_FILE =
            "test_carrier_config.xml";

    public static final String IMS_KEY_PREFIXES[] =
        {
            CarrierConfigManager.Ims.KEY_PREFIX,
            CarrierConfigManager.ImsEmergency.KEY_PREFIX,
            CarrierConfigManager.ImsRtt.KEY_PREFIX,
            CarrierConfigManager.ImsSms.KEY_PREFIX,
            CarrierConfigManager.ImsSs.KEY_PREFIX,
            CarrierConfigManager.ImsVoice.KEY_PREFIX,
            CarrierConfigManager.ImsVt.KEY_PREFIX,
            CarrierConfigManager.ImsWfc.KEY_PREFIX
        };

    public static final String IMS_COMMON_KEYS[] =
        {
            CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL,
            CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL,
            CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL,
            CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL,
            CarrierConfigManager.KEY_CARRIER_SUPPORTS_SS_OVER_UT_BOOL,
            CarrierConfigManager.KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL,
            CarrierConfigManager.KEY_CARRIER_USSD_METHOD_INT,
            CarrierConfigManager.KEY_RTT_SUPPORTED_BOOL,
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
        };

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
        public static final String KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT =
                KEY_PREFIX + "refresh_geolocation_timeout_millis_int";
        public static final String KEY_UNSUBSCRIBE_REGISTRATION_EVENT_PACKAGE_BOOL =
                KEY_PREFIX + "unsubscribe_registration_event_package_bool";
        public static final String KEY_REGISTRATION_EVENT_FOR_CAT_REQUIRED_BOOL =
                KEY_PREFIX + "registration_event_for_cat_required_bool";
        public static final String KEY_PREFERRED_IMS_DSCP_INT =
                KEY_PREFIX + "preferred_ims_dscp_int";
        public static final String KEY_REGISTRATION_PREFERRED_ACCESSTYPE_FEATURE_TAG_INT =
                KEY_PREFIX + "registration_preferred_accesstype_feature_tag_int";
        public static final String KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY =
                KEY_PREFIX + "registration_permanent_error_code_int_array";
    }

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
    }

    public static class ImsEmergency {
        public static final String KEY_PREFIX = "imsemergency.";
        public static final String KEY_EMERGENCY_CELLULAR_SCAN_TIMER_MILLIS_INT =
                KEY_PREFIX + "emergency_cellular_scan_timer_millis_int";
        public static final String KEY_EMERGENCY_INVITE_18X_TIMER_MILLIS_INT =
                KEY_PREFIX + "emergency_invite_18x_timer_millis_int";
        public static final String KEY_EMERGENCY_PROVISIONAL_TO_FINAL_RESPONSE_TIMER_MILLIS_INT =
                KEY_PREFIX + "emergency_provisional_to_final_response_timer_millis_int";
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
    }

    public static class ImsRtt {
        public static final String KEY_PREFIX = "imsrtt.";
        public static final String KEY_POLICY_ON_TEXT_QOS_DEACTIVATION_INT =
                KEY_PREFIX + "policy_on_text_qos_deactivation_int";
        public static final String KEY_TEXT_RTCP_INTERVAL_INT_ARRAY =
                KEY_PREFIX + "text_rtcp_interval_int_array";
    }

    public static class ImsSms {
        public static final String KEY_PREFIX = "imssms.";
        public static final String KEY_SUPPORT_LIMITED_ADMIN_SMS_MODE_BOOL =
                KEY_PREFIX + "support_limited_admin_sms_mode_bool";
    }

    public static class ImsSs {
        public static final String KEY_PREFIX = "imsss.";
        public static final String KEY_XCAP_ROOT_URI_REQUIRES_SRV_QUERY_BOOL =
                KEY_PREFIX + "xcap_root_uri_requires_srv_query_bool";
        public static final String KEY_XCAP_AUID_PREFIX_STRING =
                KEY_PREFIX + "xcap_auid_prefix_string";
        public static final String KEY_UT_SM_CAUSE_PERMANENT_BLOCK_INT_ARRAY =
                KEY_PREFIX + "ut_sm_cause_permanent_block_int_array";
        public static final String KEY_UT_SM_CAUSE_TEMPORARY_BLOCK_INT_ARRAY =
                KEY_PREFIX + "ut_sm_cause_temporary_block_int_array";
        public static final String KEY_UT_HTTP_PERMANENT_ERROR_CODE_INT_ARRAY =
                KEY_PREFIX + "ut_http_permanent_error_code_int_array";
        public static final String KEY_UT_HTTP_TEMPORARY_ERROR_CODE_INT_ARRAY =
                KEY_PREFIX + "ut_http_temporary_error_code_int_array";
        public static final String KEY_UT_MAX_RETRY_COUNT_INT =
                KEY_PREFIX + "ut_max_retry_count_int";
        public static final String KEY_UT_TEMPORARY_BLOCK_TIMER_MIN_INT =
                KEY_PREFIX + "ut_temporary_block_timer_min_int";
        public static final String KEY_XCAP_OVER_WIFI_REQUIRES_VOWIFI_REGISTRATION_BOOL =
                KEY_PREFIX + "xcap_over_wifi_requires_vowifi_registration_bool";
        public static final String KEY_TERMINAL_BASED_CALL_WAIT_SYNC_INT =
                KEY_PREFIX + "terminal_based_call_wait_sync_int";
        public static final String KEY_TERMINAL_BASED_CALL_WAIT_DEFAULT_ACTIVATED_BOOL =
                KEY_PREFIX + "terminal_based_call_wait_default_activated_bool";
    }

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
    }

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
        public static final String KEY_SESSION_REFRESH_TRIGGER_INTERVAL_MILLIS_INT =
                KEY_PREFIX + "session_refresh_trigger_interval_millis_int";
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
        public static final String KEY_VALIDATE_VERSTAT_FEATURE_IN_REGISTRATION_TO_CHECK_NETWORK_CAPABILITY_BOOL =
                KEY_PREFIX + "validate_verstat_feature_in_registration_to_check_network_capability_bool";
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
        public static final String KEY_CALL_MAX_COUNT = KEY_PREFIX + "call_max_count";
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
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_REDIRECTION_FAILURE_STRING =
                KEY_PREFIX + "call_terminate_reason_header_redirection_failure_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_TERMINATING_EARLYDIALOG_STRING =
                KEY_PREFIX + "call_terminate_reason_header_terminating_earlydialog_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_VOPS_OFF_STRING =
                KEY_PREFIX + "call_terminate_reason_header_vops_off_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_SESSION_REFRESH_FAILURE_STRING =
                KEY_PREFIX + "call_terminate_reason_header_session_refresh_failure_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_CONFERENCE_CALL_JOINED_STRING =
                KEY_PREFIX + "call_terminate_reason_header_conference_call_joined_string";
        public static final String KEY_CALL_TERMINATE_REASON_HEADER_INTERNAL_ERROR_STRING =
                KEY_PREFIX + "call_terminate_reason_header_internal_error_string";
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
        public static final String KEY_CALL_REJECT_REASON_PHRASE_USER_REJECT =
                KEY_PREFIX + "call_reject_reason_phrase_user_reject";
        // Media
        public static final String KEY_MEDIA_ANBR_CAPABILITY_IN_MODEM_BOOL =
                KEY_PREFIX + "media_anbr_capability_in_modem_bool";
        public static final String KEY_AUDIO_JITTER_BUFFER_SIZE_INT_ARRAY =
                KEY_PREFIX + "audio_jitter_buffer_size_int_array";
        public static final String KEY_AUDIO_RTCP_INTERVAL_INT_ARRAY =
                KEY_PREFIX + "audio_rtcp_interval_int_array";
        public static final String KEY_EVS_AMRWB_IO_MODE_SET_INT =
                KEY_PREFIX + "evs_amrwb_io_mode_set_int";
    }

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
    }

    public static class ImsWfc {
        public static final String KEY_PREFIX = "imswfc.";
        public static final String KEY_REGISTRATION_PRIVATE_HEADER_INT =
                KEY_PREFIX + "registration_private_header_int";
        public static final String KEY_COUNTRY_CODE_INT =
                KEY_PREFIX + "country_code_int";
    }

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
        // Aos
        public static final String KEY_USE_SECURITY_SERVER_PORT_IN_REG_CONTACT_OF_INITIAL_REGISTRATION_BOOL =
                "use_security_server_port_in_reg_contact_of_initial_registration_bool";
        public static final String KEY_USE_SECURITY_SERVER_PORT_IN_INITIAL_REGISTRATION_BOOL =
                "use_security_server_port_in_initial_registration_bool";
        public static final String KEY_REMOVE_OLD_SA_ON_ESTABLISHING_SA_BOOL =
                "remove_old_sa_on_establishing_sa_bool";
        public static final String KEY_USE_G_GSMA_RCS_TELEPHONY_FEATURE_TAG_AS_AVAILABLE_VOICE_CALL_TYPE_BOOL =
                "use_g_gsma_rcs_telephony_feature_tag_as_available_voice_call_type_bool";
        public static final String KEY_PCSCF_DISCOVERY_METHOD_ROAMING_INT_ARRAY =
                "pcscf_discovery_method_roaming_int_array";
        public static final String KEY_HOLD_REGISTRATION_WHEN_IPCAN_CHANGED_WITH_IMS_ACTIVE_CALL_BOOL =
                "hold_registration_when_ipcan_changed_with_ims_active_call_bool";
        public static final String KEY_SUPPORT_VERSTAT_FOR_REGISTRATION_BOOL =
                "support_verstat_for_registration_bool";
        public static final String KEY_NO_INITIAL_REGISTRATION_ON_PCSCF_CHANGE_BOOL =
                "no_initial_registration_on_pcscf_change_bool";
        public static final String KEY_EMERGENCY_CALL_BASED_ON_P_ASSOCIATED_URI_OF_NORMAL_REGISTRATION_BOOL =
                "emergency_call_based_on_p_associated_uri_of_normal_registration_bool";
        public static final String KEY_SIP_OVER_IPSEC_ENABLED_IN_ROAMING_BOOL =
                "sip_over_ipsec_enabled_in_roaming_bool";
        public static final String KEY_REGISTRATION_CONTACT_VALIDATION_BOOL =
                "registration_contact_validation_bool";
        public static final String KEY_REGISTRATION_OUT_OF_SERVICE_POLICY_INT =
                "registration_out_of_service_policy_int";
        public static final String KEY_VIDEO_OVER_WIFI_SUPPORTED_WITHOUT_VOICE_BOOL =
                "video_over_wifi_supported_without_voice_bool";
        public static final String KEY_CDMALESS_FEATURE_TAG_REQUIRED_BOOL =
                "cdmaless_feature_tag_required_bool";
        public static final String KEY_REGISTRATION_RETRY_COUNT_RESET_POLICY_INT =
                "registration_retry_count_reset_policy_int";
        public static final String KEY_REGISTRATION_PERMANENT_ERROR_MAX_COUNT_INT_ARRAY =
                "registration_permanent_error_max_count_int_array";
        public static final String
                KEY_VOWIFI_SUBSCRIPTION_ERROR_CODE_WITH_INITIAL_REGISTRATION_INT_ARRAY =
                        "vowifi_subscription_error_code_with_initial_registration_int_array";
        public static final String
                KEY_SUBSCRIPTION_ERROR_CODE_FOR_STOPPING_BY_EXPIRATION_TIME_INT_ARRAY =
                        "subscription_error_code_for_stopping_by_expiration_time_int_array";
        public static final String
                KEY_SUB_ERR_CODE_FOR_REG_EVENT_WITH_INITIAL_REG_WITH_NEXT_PCSCF_INT_ARRAY =
                        "subscription_error_code_for_reg_event_"
                                + "with_initial_registration_with_next_pcscf_int_array";
        public static final String KEY_IGNORE_VOPS_FOR_VOLTE_ENABLE_BOOL =
                "ignore_vops_for_volte_enable_bool";
        public static final String KEY_SMS_OVER_IMS_AVAILABLE_WITHOUT_VOICE_CAPABILITY_BOOL =
                "sms_over_ims_available_without_voice_capability_bool";
        public static final String KEY_IMS_DEREGISTER_ON_3G_NETWORKS_BOOL =
                "ims_deregister_on_3g_networks_bool";
        public static final String KEY_REQUIRED_EMERGENCY_REGISTRATION_IN_ROAMING_BOOL =
                "required_emergency_registration_in_roaming_bool";
        public static final String KEY_CLEAR_PERMANENT_PDN_FAILURE_INT_ARRAY =
                "clear_permanent_pdn_failure_int_array";
        public static final String KEY_USE_WFC_COUNTRY_CODE_AVAILABILITY_CHECK_BOOL =
                "use_wfc_country_code_availability_check_bool";
        public static final String KEY_REQUIRED_VOLTE_BLOCK_BY_SETTING_BOOL =
                "required_volte_block_by_setting_bool";
        public static final String KEY_REQUIRED_VOLTE_BLOCK_BY_AIRPLANE_MODE_BOOL =
                "required_volte_block_by_airplane_mode_bool";
        public static final String KEY_REQUIRED_WFC_BLOCK_BY_AIRPLANE_MODE_BOOL =
                "required_wfc_block_by_airplane_mode_bool";
        public static final String KEY_SUPPORTED_ROAMING_RATS_INT_ARRAY =
                "supported_roaming_rats_int_array";
        public static final String KEY_EMERGENCY_PREFERRED_IPTYPE_INT =
                "emergency_preferred_iptype_int";
        public static final String KEY_SIP_MESSAGE_THRESHOLD_FOR_TRANSPORT_CHANGE_INT =
                "sip_message_threshold_for_transport_change_int";
        public static final String KEY_PERMANENT_PDN_FAILURE_INT_ARRAY =
                "permanent_pdn_failure_int_array";
        public static final String KEY_EMERGENCY_PCSCF_RETRY_WAIT_TIME_SEC_INT_ARRAY =
                "emergency_pcscf_retry_wait_time_sec_int_array";
        public static final String KEY_IMS_PREFERRED_IPTYPE_INT =
                "ims_preferred_iptype_int";
        public static final String KEY_IMS_SIGNALLING_DSCP_INT =
                "ims_signalling_dscp_int";
        public static final String KEY_REGISTRATION_ACTUAL_WAIT_TIME_POLICY_INT =
                "registration_actual_wait_time_policy_int";
        public static final String KEY_DISABLE_T3482_FOR_EMERGENCY_BOOL =
                "disable_t3482_for_emergency_bool";
        public static final String KEY_REGISTRATION_TIMER_FOR_EMERGENCY_CALL_MILLIS_INT =
                "registration_timer_for_emergency_call_millis_int";
        public static final String KEY_UPDATE_REGISTRATION_WITH_COUNTRY_CHANGE_BOOL =
                "update_registration_with_country_change_bool";
        public static final String KEY_REGISTRATION_PCSCF_UPDATE_POLICY_INT =
                "registration_pcscf_update_policy_int";
        public static final String KEY_SUPPORT_CONTACT_USER_INFO_BOOL =
                "support_contact_user_info_bool";
        public static final String KEY_CONTACT_USER_INFO_POLICY_FOR_NON_REGISTER_MESSAGE_INT =
                "contact_user_info_policy_for_non_register_message_int";
        public static final String KEY_GEOLOCATION_PIDF_FORMING_POLICY_INT =
                "geolocation_pidf_forming_policy_int";
        public static final String KEY_SPECIFIC_REGISTRATION_ERROR_BUNDLE =
                "specific_registration_error_bundle";
        public static final String KEY_SPECIFIC_REGISTRATION_ERROR_FINAL_TYPE_INT =
                "specific_registration_error_final_type_int";
        public static final String KEY_SPECIFIC_REGISTRATION_ERROR_POLICY_INT =
                "specific_registration_error_policy_int";
        public static final String KEY_SPECIFIC_REGISTRATION_ERROR_MAX_COUNT_INT =
                "specific_registration_error_max_count_int";
        public static final String KEY_SPECIFIC_REGISTRATION_ERROR_MIN_COUNT_INT =
                "specific_registration_error_min_count_int";
        public static final String KEY_SPECIFIC_REGISTRATION_ERROR_RETRY_COUNT_SHARED_FOR_REGISTRATION_AND_REG_EVENT_BOOL =
                "specific_registration_error_retry_count_shared_for_registration_and_reg_event_bool";
        public static final String KEY_SPECIFIC_REGISTRATION_ERROR_NUMBER_MULTIPLIED_BY_PCSCF_NUMBER_INT_ARRAY =
                "specific_registration_error_number_multiplied_by_pcscf_number_int_array";
        public static final String KEY_SPECIFIC_REGISTRATION_ERROR_CODE_INT_ARRAY =
                "specific_registration_error_code_int_array";
        public static final String KEY_SPECIFIC_REREGISTRATION_ERROR_CODE_INT_ARRAY =
                "specific_reregistration_error_code_int_array";
        public static final String KEY_SPECIFIC_REGISTRATION_ERROR_WAIT_TIME_SEC_INT_ARRAY =
                "specific_registration_error_wait_time_sec_int_array";
        public static final String KEY_SPECIFIC_REREGISTRATION_FAILURE_WITH_ERROR_CODE_IN_ROAMING_BOOL =
                "specific_reregistration_failure_with_error_code_in_roaming_bool";
        // }
        // Bundle {
        public static final String KEY_REGISTRATION_RETRY_BUNDLE =
                "registration_retry_bundle";
        public static final String KEY_REGISTRATION_RETRY_MIN_COUNT_INT =
                "registration_retry_min_count_int";
        public static final String KEY_REGISTRATION_RETRY_SIP_305_CODE_POLICY_INT =
                "registration_retry_sip_305_code_policy_int";
        public static final String KEY_REGISTRATION_RETRY_ERROR_CODE_WITHOUT_IPSEC_INT_ARRAY =
                "registration_retry_error_code_without_ipsec_int_array";
        public static final String KEY_REGISTRATION_RETRY_TIMER_F_POLICY_INT =
                "registration_retry_timer_f_policy_int";
        public static final String KEY_REGISTRATION_RETRY_ERROR_CODE_WITH_DIFFERENT_PCSCF_INT_ARRAY =
                "registration_retry_error_code_with_different_pcscf_int_array";
        public static final String KEY_REGISTRATION_RETRY_WITH_IP_VERSION_FALLBACK_BOOL =
                "registration_retry_with_ip_version_fallback_bool";
        public static final String KEY_REGISTRATION_RETRY_DEFAULT_POLICY_INT =
                "registration_retry_default_policy_int";
        public static final String KEY_REGISTRATION_RETRY_SIP_503_CODE_POLICY_INT =
                "registration_retry_sip_503_code_policy_int";
        // }
        // Bundle {
        public static final String KEY_REREGISTRATION_RETRY_BUNDLE =
                "reregistration_retry_bundle";
        public static final String KEY_REREGISTRATION_RETRY_ERROR_CODE_WITH_INITIAL_REGISTRATION_INT_ARRAY =
                "reregistration_retry_error_code_with_initial_registration_int_array";
        public static final String KEY_REREGISTRATION_RETRY_EXPIRE_TIME_CHECKED_BOOL =
                "reregistration_retry_expire_time_checked_bool";
        public static final String KEY_REREGISTRATION_RETRY_MAX_COUNT_KEPT_REGISTRATION_INT =
                "reregistration_retry_max_count_kept_registration_int";
        public static final String KEY_REREGISTRATION_RETRY_ERROR_CODE_WITH_INITIAL_REGISTRATION_WITH_SAME_PCSCF_INT_ARRAY =
                "reregistration_retry_error_code_with_initial_registration_with_same_pcscf_int_array";
        public static final String KEY_REREGISTRATION_RETRY_SIP_305_CODE_POLICY_INT =
                "reregistration_retry_sip_305_code_policy_int";
        // }
        // Bundle {
        public static final String KEY_REREGISTRATION_ERROR_POLICY_DURING_CALL_BUNDLE =
                "reregistration_error_policy_during_call_bundle";
        public static final String KEY_REREGISTRATION_ERROR_CODE_WITH_CALL_END_INT_ARRAY =
                "reregistration_error_code_with_call_end_int_array";
        public static final String KEY_REREGISTRATION_ERROR_CAUSE_WITH_PDN_REACTIVATION_AFTER_CALL_END_INT_ARRAY =
                "reregistration_error_cause_with_pdn_reactivation_after_call_end_int_array";
        // }
        // Bundle {
        public static final String KEY_SUBSCRIPTION_ERROR_CODE_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_BUNDLE =
                "subscription_error_code_for_reg_event_with_initial_registration_bundle";
        public static final String KEY_SUBSCRIPTION_ERROR_RETRY_MAX_COUNT_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_INT =
                "subscription_error_retry_max_count_for_reg_event_with_initial_registration_int";
        public static final String KEY_SUBSCRIPTION_ERROR_CODE_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_INT_ARRAY =
                "subscription_error_code_for_reg_event_with_initial_registration_int_array";
        // }
        // Bundle {
        public static final String KEY_SUBSCRIPTION_TERMINATED_ERROR_CODE_FOR_REG_EVENT_BUNDLE =
                "subscription_terminated_error_code_for_reg_event_bundle";
        public static final String KEY_SUBSCRIPTION_TERMINATED_ERROR_CODE_FOR_REG_EVENT_RETRY_MAX_COUNT_INT =
                "subscription_terminated_error_code_for_reg_event_retry_max_count_int";
        public static final String KEY_SUBSCRIPTION_TERMINATED_ERROR_CODE_FOR_REG_EVENT_INT_ARRAY =
                "subscription_terminated_error_code_for_reg_event_int_array";
        // }
        // Bundle {
        public static final String KEY_REGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_BUNDLE =
                "registration_error_code_with_retry_after_time_bundle";
        public static final String KEY_REGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_ONLY_DEFINED_BOOL =
                "registration_error_code_with_retry_after_time_only_defined_bool";
        public static final String KEY_REGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_INT_ARRAY =
                "registration_error_code_with_retry_after_time_int_array";
        public static final String KEY_REREGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_INT_ARRAY =
                "reregistration_error_code_with_retry_after_time_int_array";
        // }
        // Bundle {
        public static final String KEY_REGISTRATION_WITH_FEATURE_TAG_UNAVAILABLE_BUNDLE =
                "registration_with_feature_tag_unavailable_bundle";
        public static final String KEY_REGISTRATION_WITH_FEATURE_TAG_UNAVAILABLE_INT_ARRAY =
                "registration_with_feature_tag_unavailable_int_array";
        public static final String KEY_REGISTRATION_WITH_FEATURE_TAG_UNAVAILABLE_POLICY_INT_ARRAY =
                "registration_with_feature_tag_unavailable_policy_int_array";
        // }
        // Bundle {
        public static final String
                KEY_NOTIFY_TERMINATED_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_BUNDLE =
                        "notify_terminated_for_reg_event_with_initial_registration_bundle";
        public static final String
                KEY_WAIT_TIME_FOR_INITIAL_REGISTRATION_ON_TERMINATED_STATE_OF_REG_EVENT_INT =
                        "wait_time_for_initial_registration_on_terminated_state_of_reg_event_int";
        public static final String
                KEY_EVT_FOR_INIT_REG_ON_TERMINATED_STATE_OF_REG_EVENT_INT_ARRAY =
                        "event_for_initial_registration_on_terminated_state_of_reg_event_int_array";
        public static final String
                KEY_EVT_TO_FOLLOW_WAIT_TIME_FOR_INIT_REG_ON_TERM_STATE_OF_REG_EVENT_INT_ARRAY =
                        "event_to_follow_wait_time_for_initial_registration_"
                                + "on_terminated_state_of_reg_event_int_array";
        // }
        // Bundle {
        public static final String KEY_REGISTRATION_RETRY_INTERVAL_BUNDLE =
                "registration_retry_interval_bundle";
        public static final String KEY_REGISTRATION_RETRY_RANDOM_UPPER_VALUE_SEC_INT_ARRAY =
                "registration_retry_random_upper_value_sec_int_array";
        public static final String KEY_REGISTRATION_RETRY_INTERVAL_SEC_INT_ARRAY =
                "registration_retry_interval_sec_int_array";
        public static final String KEY_USE_REGISTRATION_RETRY_INTERVAL_FOR_SUBSCRIPTION_RETRY_BOOL =
                "use_registration_retry_interval_for_subscription_retry_bool";
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
        public static final String KEY_REGISTRATION_DISCONNECT_REASON_TO_TERMINATE_ONGOING_CALL_INT_ARRAY =
                "registration_disconnect_reason_to_terminate_ongoing_call_int_array";
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
        public static final String KEY_SEND_180_FOR_INITIAL_INVITE_BOOL =
                "send_180_for_initial_invite_bool";
        // Media
        public static final String KEY_AUDIO_RTP_PORT_RANGE_INT_ARRAY =
                "audio_rtp_port_range_int_array";
        public static final String KEY_AUDIO_BW_NEGO_OPTION_BOOL =
                "audio_bw_nego_option_bool";
        public static final String KEY_AUDIO_MAXPTIME_MILLIS_INT =
                "audio_maxptime_millis_int";
        public static final String KEY_AUDIO_MAXRED_INT =
                "audio_maxred_int";
        public static final String KEY_AUDIO_RTP_DSCP_INT =
                "audio_rtp_dscp_int";
        public static final String KEY_AUDIO_TELEPHONE_EVENT_DURATION_INT =
                "audio_telephone_event_duration_int";
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
        public static final String KEY_TEXT_RTP_PORT_RANGE_INT_ARRAY =
                "text_rtp_port_range_int_array";
        public static final String KEY_TEXT_CODEC_EMPTY_REDUNDANT_BOOL =
                "text_codec_empty_redundant_bool";
        public static final String KEY_SUPPORT_MULTI_CONFIG_IN_EARLY_SESSION_BOOL =
                "support_multi_config_in_early_session_bool";
        public static final String KEY_MEDIA_SESSION_LEVEL_BANDWIDTH_BOOL =
                "media_session_level_bandwidth_bool";
        public static final String KEY_SDP_ANSWER_FULL_CAPABILITY_BOOL =
                "sdp_answer_full_capability_bool";
        public static final String KEY_SDP_REOFFER_FULL_CAPABILITY_BOOL =
                "sdp_reoffer_full_capability_bool";

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

        // Ut/Xcap
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
        public static final String KEY_UT_HOST_HEADER_REQUIRES_PORT_NUMBER_BOOL =
                "ut_host_header_requires_port_number_bool";
        public static final String KEY_UT_XCAP_APN_INACTIVITY_TIMER_SEC_INT =
                "ut_xcap_apn_inactivity_timer_sec_int";
        public static final String KEY_UT_DISPLAY_ERROR_PHRASE_WITH_409_ERROR_BOOL =
                "ut_display_error_phrase_with_409_error_bool";
        public static final String KEY_UT_OMIT_NAMESPACE_OF_DOCUMENT_ELEMENT_BOOL =
                "ut_omit_namespace_of_document_element_bool";
        public static final String KEY_UT_OMIT_NAMESPACE_SS_BOOL = "ut_omit_namespace_ss_bool";
        public static final String KEY_UT_OMIT_NAMESPACE_CP_BOOL = "ut_omit_namespace_cp_bool";
        public static final String KEY_UT_INSERT_NEW_RULE_BOOL = "ut_insert_new_rule_bool";
        // Uce
        public static final String KEY_ADD_VIDEO_TAG_CONTACT_HEADER_IN_PUBLISH_BOOL =
                "add_video_tag_contact_header_in_publish_bool";

        // Indicates preferred IP version
        public static final int IPV4_PREFERRED = 0;
        public static final int IPV6_PREFERRED = 1;
    }

    private static final int TYPE_BOOLEAN = 0;
    private static final int TYPE_INT = 1;
    private static final int TYPE_LONG = 2;
    private static final int TYPE_STRING = 3;
    private static final int TYPE_BOOLEAN_ARRAY = 4;
    private static final int TYPE_INT_ARRAY = 5;
    private static final int TYPE_LONG_ARRAY = 6;
    private static final int TYPE_STRING_ARRAY = 7;

    private final PersistableBundle mConfig = new PersistableBundle();

    public CarrierConfig() {
    }

    public void setConfig(PersistableBundle config, int slotId) {
        mConfig.clear();
        mConfig.putAll(config);

        refineSpecialKeys(slotId);
        refineBundles();
    }

    /**
     * @brief Returns the string value for a specified key.
     *
     * @param key The config key
     * @return A string value if present. Otherwise, returns null.
     */
    public String getString(String key) {
        return getString(key, null);
    }

    /**
     * @brief Returns the string value for a specified key.
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
     * @brief Returns the long value for a specified key.
     *
     * @param key The config key
     * @return A long value if present. Otherwise, returns a negative value.
     */
    public long getLong(String key) {
        return getLong(key, -1L);
    }

    /**
     * @brief Returns the long value for a specified key.
     *
     * @param key The config key
     * @param defaultValue The default value if not present
     * @return A long value if present. Otherwise, returns a default value.
     */
    public long getLong(String key, long defaultValue) {
        return mConfig.getLong(key, defaultValue);
    }

    /**
     * @brief Returns the boolean-array value for a specified key.
     *
     * @param key The config key
     * @return An boolean-array value if present. Otherwise, returns an empty array.
     */
    public boolean[] getBooleanArray(String key) {
        return mConfig.getBooleanArray(key);
    }

    /**
     * @brief Returns the integer-array value for a specified key.
     *
     * @param key The config key
     * @return An integer-array value if present. Otherwise, returns an empty array.
     */
    public int[] getIntArray(String key) {
        return mConfig.getIntArray(key);
    }

    /**
     * @brief Returns the long-array value for a specified key.
     *
     * @param key The config key
     * @return A long-array value if present. Otherwise, returns an empty array.
     */
    public long[] getLongArray(String key) {
        return mConfig.getLongArray(key);
    }

    /**
     * @brief Returns the string-array value for a specified key.
     *
     * @param key The config key
     * @return A string-array value if present. Otherwise, returns an empty array.
     */
    public String[] getStringArray(String key) {
        return mConfig.getStringArray(key);
    }

    /**
     * @brief Returns the PersistableBundle value for a specified key.
     *
     * @param key The config key
     * @return A PersistableBundle object if present. Otherwise, returns null.
     */
    public PersistableBundle getBundle(String key) {
        return mConfig.getPersistableBundle(key);
    }

    public PersistableBundle getConfig() {
        return mConfig;
    }

    public void writeToParcel(Parcel p) {
        mConfig.writeToParcel(p, 0);
    }

    private void refineSpecialKeys(int slotId) {
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
        TelephonyManager tm = AppContext.getTelephonyManager();
        String imei = (tm != null) ? tm.getImei(slotId) : null;
        return TextUtils.isEmpty(imei) ? emptyImei : imei;
    }

    private static String getImeiSv(int slotId) {
        final String emptyImeiWithoutCheckDigit = "00000000000000";
        TelephonyManager tm = AppContext.getTelephonyManager();
        String imei = (tm != null) ? tm.getImei(slotId) : null;

        if (TextUtils.isEmpty(imei)) {
            imei = emptyImeiWithoutCheckDigit;
        } else {
            imei = imei.substring(0, imei.length() - 1);
        }

        String sv = (tm != null) ? tm.getDeviceSoftwareVersion(slotId) : null;

        if (TextUtils.isEmpty(sv)) {
            sv = "00";
        } else if (sv.length() == 1) {
            sv = "0" + sv;
        }

        return imei + sv;
    }

    private void refineBundles() {
        refineBundlesForIms();
        refineBundlesForImsRtt();
        refineBundlesForImsVoice();
        //refineBundlesForImsVt(); // TODO_MEDIA temp block waiting for bundle
        refineBundlesForAssets();
    }

    private void refineBundlesForIms() {
        // These bundles are set by the Telephony Service,
        // so we don't need to adjust these bundles.
        // KEY_MMTEL_REQUIRES_PROVISIONING_BUNDLE
        // KEY_RCS_REQUIRES_PROVISIONING_BUNDLE
    }

    private void refineBundlesForImsRtt() {
        // Check the following keys:
        // KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE
        final String[] TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE_KEYS =
            {
                CarrierConfigManager.ImsRtt.KEY_T140_PAYLOAD_TYPE_INT,
                CarrierConfigManager.ImsRtt.KEY_RED_PAYLOAD_TYPE_INT
            };

        setBundle(mConfig,
                CarrierConfigManager.ImsRtt.KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
                TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE_KEYS);
    }

    private void refineBundlesForImsVoice() {
        // Check the following keys:
        // KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE
        // KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE
        // KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE
        // KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE

        final String[] AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE_KEYS =
            {
                CarrierConfigManager.ImsVoice.KEY_EVS_PAYLOAD_TYPE_INT_ARRAY,
                CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY,
                CarrierConfigManager.ImsVoice.KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY,
                CarrierConfigManager.ImsVoice.KEY_DTMFWB_PAYLOAD_TYPE_INT_ARRAY,
                CarrierConfigManager.ImsVoice.KEY_DTMFNB_PAYLOAD_TYPE_INT_ARRAY
            };

        setBundle(mConfig,
                CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
                AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE_KEYS);

        PersistableBundle audioCodecCapabilityPayloadTypes = getBundle(
                CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);

        if (audioCodecCapabilityPayloadTypes != null) {
            int[] payloadTypesForAmrNb = audioCodecCapabilityPayloadTypes.getIntArray(
                    CarrierConfigManager.ImsVoice.KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY);

            if (payloadTypesForAmrNb != null) {
                PersistableBundle payloadDescForAmrNb = new PersistableBundle();

                if (payloadTypesForAmrNb.length >= 1) {
                    // Bandwidth-efficient mode
                    payloadDescForAmrNb.putPersistableBundle(
                            String.valueOf(payloadTypesForAmrNb[0]), new PersistableBundle());
                }

                if (payloadTypesForAmrNb.length >= 2) {
                    PersistableBundle codecAttrForAmrNb = new PersistableBundle();

                    codecAttrForAmrNb.putInt(
                            CarrierConfigManager.ImsVoice.
                                    KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                            CarrierConfigManager.ImsVoice.OCTET_ALIGNED);

                    payloadDescForAmrNb.putPersistableBundle(
                            String.valueOf(payloadTypesForAmrNb[1]), codecAttrForAmrNb);
                }

                // How to handle this when the count of payload type is greater than two ?
                // How to configure the mode-set?

                mConfig.putPersistableBundle(
                        CarrierConfigManager.ImsVoice.KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE,
                        payloadDescForAmrNb);
            }

            int[] payloadTypesForAmrWb = audioCodecCapabilityPayloadTypes.getIntArray(
                    CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY);

            if (payloadTypesForAmrWb != null) {
                PersistableBundle payloadDescForAmrWb = new PersistableBundle();
                final String[] AMRWB_CODEC_ATTRIBUTE_KEYS =
                    {
                        CarrierConfigManager.ImsVoice.
                                KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT,
                        CarrierConfigManager.ImsVoice.
                                KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT,
                        CarrierConfigManager.ImsVoice.
                                KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT
                    };

                if (payloadTypesForAmrWb.length >= 1) {
                    // Bandwidth-efficient mode
                    PersistableBundle codecAttrForAmrWb = new PersistableBundle();

                    for (String key : AMRWB_CODEC_ATTRIBUTE_KEYS) {
                        moveItemToBundle(mConfig, codecAttrForAmrWb, key, false);
                    }

                    payloadDescForAmrWb.putPersistableBundle(
                            String.valueOf(payloadTypesForAmrWb[0]), codecAttrForAmrWb);
                }

                if (payloadTypesForAmrWb.length >= 2) {
                    PersistableBundle codecAttrForAmrWb = new PersistableBundle();

                    codecAttrForAmrWb.putInt(
                            CarrierConfigManager.ImsVoice.
                                    KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                            CarrierConfigManager.ImsVoice.OCTET_ALIGNED);

                    for (String key : AMRWB_CODEC_ATTRIBUTE_KEYS) {
                        moveItemToBundle(mConfig, codecAttrForAmrWb, key, false);
                    }

                    payloadDescForAmrWb.putPersistableBundle(
                            String.valueOf(payloadTypesForAmrWb[1]), codecAttrForAmrWb);
                }

                // How to handle this when the count of payload type is greater than two ?
                // How to configure the mode-set?

                mConfig.putPersistableBundle(
                        CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE,
                        payloadDescForAmrWb);
            }

            int[] payloadTypesForEvs = audioCodecCapabilityPayloadTypes.getIntArray(
                    CarrierConfigManager.ImsVoice.KEY_EVS_PAYLOAD_TYPE_INT_ARRAY);

            if (payloadTypesForEvs != null) {
                PersistableBundle payloadDescForEvs = new PersistableBundle();

                if (payloadTypesForEvs.length >= 1) {
                    PersistableBundle codecAttrForEvs = new PersistableBundle();

                    final String[] EVS_CODEC_ATTRIBUTE_KEYS =
                        {
                            CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT,
                            CarrierConfigManager.ImsVoice.
                                    KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY,
                            CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT,
                            CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT,
                            CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_DTX_BOOL,
                            CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL,
                            CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT,
                            CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_CMR_INT,
                            CarrierConfigManager.ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_CHANNELS_INT,
                            CarrierConfigManager.ImsVoice.
                                    KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT,
                            CarrierConfigManager.ImsVoice.
                                    KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT,
                            CarrierConfigManager.ImsVoice.
                                    KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT
                        };

                    for (String key : EVS_CODEC_ATTRIBUTE_KEYS) {
                        moveItemToBundle(mConfig, codecAttrForEvs, key, false);
                    }

                    payloadDescForEvs.putPersistableBundle(
                            String.valueOf(payloadTypesForEvs[0]), codecAttrForEvs);
                }

                // How to handle this when the count of payload type is greater than one ?

                mConfig.putPersistableBundle(
                        CarrierConfigManager.ImsVoice.KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE,
                        payloadDescForEvs);
            }
        }
    }

    private void refineBundlesForImsVt() {
        // Check the following keys:
        // KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE
        // KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE

        final String[] VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE_KEYS =
            {
                CarrierConfigManager.ImsVt.KEY_H264_PAYLOAD_TYPE_INT_ARRAY
            };

        setBundle(mConfig,
                CarrierConfigManager.ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
                VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE_KEYS);

        final String[] H264_PAYLOAD_DESCRIPTION_BUNDLE_KEYS =
            {
                CarrierConfigManager.ImsVt.KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING,
                CarrierConfigManager.ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT,
                CarrierConfigManager.ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT,
                CarrierConfigManager.ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY
            };

        setBundle(mConfig,
                CarrierConfigManager.ImsVt.KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE,
                H264_PAYLOAD_DESCRIPTION_BUNDLE_KEYS);
    }

    private void refineBundlesForAssets() {
        // Check the following keys:
        // KEY_SPECIFIC_REGISTRATION_ERROR_BUNDLE
        // KEY_REGISTRATION_RETRY_BUNDLE
        // KEY_REREGISTRATION_RETRY_BUNDLE
        // KEY_REREGISTRATION_ERROR_POLICY_DURING_CALL_BUNDLE
        // KEY_SUBSCRIPTION_ERROR_CODE_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_BUNDLE
        // KEY_SUBSCRIPTION_TERMINATED_ERROR_CODE_FOR_REG_EVENT_BUNDLE
        // KEY_REGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_BUNDLE
        // KEY_REGISTRATION_WITH_FEATURE_TAG_UNAVAILABLE_BUNDLE
        // KEY_NOTIFY_TERMINATED_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_BUNDLE
        // KEY_REGISTRATION_RETRY_INTERVAL_BUNDLE

        final String[] SPECIFIC_REGISTRATION_ERROR_BUNDLE_KEYS =
            {
                Assets.KEY_SPECIFIC_REGISTRATION_ERROR_FINAL_TYPE_INT,
                Assets.KEY_SPECIFIC_REGISTRATION_ERROR_POLICY_INT,
                Assets.KEY_SPECIFIC_REGISTRATION_ERROR_MAX_COUNT_INT,
                Assets.KEY_SPECIFIC_REGISTRATION_ERROR_MIN_COUNT_INT,
                Assets.KEY_SPECIFIC_REGISTRATION_ERROR_RETRY_COUNT_SHARED_FOR_REGISTRATION_AND_REG_EVENT_BOOL,
                Assets.KEY_SPECIFIC_REGISTRATION_ERROR_NUMBER_MULTIPLIED_BY_PCSCF_NUMBER_INT_ARRAY,
                Assets.KEY_SPECIFIC_REGISTRATION_ERROR_CODE_INT_ARRAY,
                Assets.KEY_SPECIFIC_REREGISTRATION_ERROR_CODE_INT_ARRAY,
                Assets.KEY_SPECIFIC_REGISTRATION_ERROR_WAIT_TIME_SEC_INT_ARRAY,
                Assets.KEY_SPECIFIC_REREGISTRATION_FAILURE_WITH_ERROR_CODE_IN_ROAMING_BOOL
            };

        setBundle(mConfig,
                Assets.KEY_SPECIFIC_REGISTRATION_ERROR_BUNDLE,
                SPECIFIC_REGISTRATION_ERROR_BUNDLE_KEYS);

        final String[] REGISTRATION_RETRY_BUNDLE_KEYS =
            {
                Assets.KEY_REGISTRATION_RETRY_MIN_COUNT_INT,
                Assets.KEY_REGISTRATION_RETRY_SIP_305_CODE_POLICY_INT,
                Assets.KEY_REGISTRATION_RETRY_ERROR_CODE_WITHOUT_IPSEC_INT_ARRAY,
                Assets.KEY_REGISTRATION_RETRY_TIMER_F_POLICY_INT,
                Assets.KEY_REGISTRATION_RETRY_ERROR_CODE_WITH_DIFFERENT_PCSCF_INT_ARRAY,
                Assets.KEY_REGISTRATION_RETRY_WITH_IP_VERSION_FALLBACK_BOOL,
                Assets.KEY_REGISTRATION_RETRY_DEFAULT_POLICY_INT,
                Assets.KEY_REGISTRATION_RETRY_SIP_503_CODE_POLICY_INT
            };

        setBundle(mConfig,
                Assets.KEY_REGISTRATION_RETRY_BUNDLE,
                REGISTRATION_RETRY_BUNDLE_KEYS);

        final String[] REREGISTRATION_RETRY_BUNDLE_KEYS =
            {
                Assets.KEY_REREGISTRATION_RETRY_ERROR_CODE_WITH_INITIAL_REGISTRATION_INT_ARRAY,
                Assets.KEY_REREGISTRATION_RETRY_EXPIRE_TIME_CHECKED_BOOL,
                Assets.KEY_REREGISTRATION_RETRY_MAX_COUNT_KEPT_REGISTRATION_INT,
                Assets.KEY_REREGISTRATION_RETRY_ERROR_CODE_WITH_INITIAL_REGISTRATION_WITH_SAME_PCSCF_INT_ARRAY,
                Assets.KEY_REREGISTRATION_RETRY_SIP_305_CODE_POLICY_INT
            };

        setBundle(mConfig,
                Assets.KEY_REREGISTRATION_RETRY_BUNDLE,
                REREGISTRATION_RETRY_BUNDLE_KEYS);

        final String[] REREGISTRATION_ERROR_POLICY_DURING_CALL_BUNDLE_KEYS =
            {
                Assets.KEY_REREGISTRATION_ERROR_CODE_WITH_CALL_END_INT_ARRAY,
                Assets.KEY_REREGISTRATION_ERROR_CAUSE_WITH_PDN_REACTIVATION_AFTER_CALL_END_INT_ARRAY
            };

        setBundle(mConfig,
                Assets.KEY_REREGISTRATION_ERROR_POLICY_DURING_CALL_BUNDLE,
                REREGISTRATION_ERROR_POLICY_DURING_CALL_BUNDLE_KEYS);

        final String[] SUBSCRIPTION_ERROR_CODE_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_BUNDLE_KEYS =
            {
                Assets.KEY_SUBSCRIPTION_ERROR_RETRY_MAX_COUNT_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_INT,
                Assets.KEY_SUBSCRIPTION_ERROR_CODE_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_INT_ARRAY
            };

        setBundle(mConfig,
                Assets.KEY_SUBSCRIPTION_ERROR_CODE_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_BUNDLE,
                SUBSCRIPTION_ERROR_CODE_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_BUNDLE_KEYS);

        final String[] SUBSCRIPTION_TERMINATED_ERROR_CODE_FOR_REG_EVENT_BUNDLE_KEYS =
            {
                Assets.KEY_SUBSCRIPTION_TERMINATED_ERROR_CODE_FOR_REG_EVENT_RETRY_MAX_COUNT_INT,
                Assets.KEY_SUBSCRIPTION_TERMINATED_ERROR_CODE_FOR_REG_EVENT_INT_ARRAY
            };

        setBundle(mConfig,
                Assets.KEY_SUBSCRIPTION_TERMINATED_ERROR_CODE_FOR_REG_EVENT_BUNDLE,
                SUBSCRIPTION_TERMINATED_ERROR_CODE_FOR_REG_EVENT_BUNDLE_KEYS);

        final String[] REGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_BUNDLE_KEYS =
            {
                Assets.KEY_REGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_ONLY_DEFINED_BOOL,
                Assets.KEY_REGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_INT_ARRAY,
                Assets.KEY_REREGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_INT_ARRAY
            };

        setBundle(mConfig,
                Assets.KEY_REGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_BUNDLE,
                REGISTRATION_ERROR_CODE_WITH_RETRY_AFTER_TIME_BUNDLE_KEYS);

        final String[] REGISTRATION_WITH_FEATURE_TAG_UNAVAILABLE_BUNDLE_KEYS =
            {
                Assets.KEY_REGISTRATION_WITH_FEATURE_TAG_UNAVAILABLE_INT_ARRAY,
                Assets.KEY_REGISTRATION_WITH_FEATURE_TAG_UNAVAILABLE_POLICY_INT_ARRAY
            };

        setBundle(mConfig,
                Assets.KEY_REGISTRATION_WITH_FEATURE_TAG_UNAVAILABLE_BUNDLE,
                REGISTRATION_WITH_FEATURE_TAG_UNAVAILABLE_BUNDLE_KEYS);

        final String[] notifyTerminatedForRegEventWithInitRegBundleKeys = {
                Assets.KEY_WAIT_TIME_FOR_INITIAL_REGISTRATION_ON_TERMINATED_STATE_OF_REG_EVENT_INT,
                Assets.KEY_EVT_FOR_INIT_REG_ON_TERMINATED_STATE_OF_REG_EVENT_INT_ARRAY,
                Assets.KEY_EVT_TO_FOLLOW_WAIT_TIME_FOR_INIT_REG_ON_TERM_STATE_OF_REG_EVENT_INT_ARRAY
        };

        setBundle(mConfig,
                Assets.KEY_NOTIFY_TERMINATED_FOR_REG_EVENT_WITH_INITIAL_REGISTRATION_BUNDLE,
                        notifyTerminatedForRegEventWithInitRegBundleKeys);

        final String[] registrationRetryIntervalBundleKeys = {
                Assets.KEY_REGISTRATION_RETRY_RANDOM_UPPER_VALUE_SEC_INT_ARRAY,
                Assets.KEY_REGISTRATION_RETRY_INTERVAL_SEC_INT_ARRAY,
                Assets.KEY_USE_REGISTRATION_RETRY_INTERVAL_FOR_SUBSCRIPTION_RETRY_BOOL
        };

        setBundle(mConfig,
                Assets.KEY_REGISTRATION_RETRY_INTERVAL_BUNDLE,
                        registrationRetryIntervalBundleKeys);
    }

    private static void setBundle(PersistableBundle config,
            String bundleKey, String[] itemKeys) {
        PersistableBundle bundle = config.getPersistableBundle(bundleKey);

        if (bundle == null) {
            bundle = new PersistableBundle();
        }

        int itemCount = 0;

        for (String itemKey : itemKeys) {
            itemCount += moveItemToBundle(config, bundle, itemKey, true);
        }

        if (itemCount > 0) {
            config.putPersistableBundle(bundleKey, bundle);
        }
    }

    private static int moveItemToBundle(PersistableBundle src,
            PersistableBundle dst, String key, boolean removeKeyFromSrc) {
        int count = 0;

        if (src.containsKey(key)) {
            if (key.endsWith("_bool") || key.endsWith("_boolean")) {
                dst.putBoolean(key, src.getBoolean(key));
                count++;
            } else if (key.endsWith("_int")) {
                dst.putInt(key, src.getInt(key));
                count++;
            } else if (key.endsWith("_long")) {
                dst.putLong(key, src.getLong(key));
                count++;
            } else if (key.endsWith("_string")) {
                dst.putString(key, src.getString(key));
                count++;
            } else if (key.endsWith("_bool_array")) {
                dst.putBooleanArray(key, src.getBooleanArray(key));
                count++;
            } else if (key.endsWith("_int_array")) {
                dst.putIntArray(key, src.getIntArray(key));
                count++;
            } else if (key.endsWith("_long_array")) {
                dst.putLongArray(key, src.getLongArray(key));
                count++;
            } else if (key.endsWith("_string_array")) {
                dst.putStringArray(key, src.getStringArray(key));
                count++;
            }

            if (count > 0 && removeKeyFromSrc) {
                src.remove(key);
            }
        }

        return count;
    }
}
