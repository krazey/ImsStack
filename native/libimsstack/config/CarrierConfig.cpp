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
#include "CarrierConfig.h"
#include "ServiceConfig.h"

// Public carrier-config - starts
const IMS_CHAR CarrierConfig::KEY_CARRIER_VOLTE_AVAILABLE_BOOL[] = "carrier_volte_available_bool";
const IMS_CHAR CarrierConfig::KEY_CARRIER_VT_AVAILABLE_BOOL[] = "carrier_vt_available_bool";
const IMS_CHAR CarrierConfig::KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL[] =
        "carrier_wfc_ims_available_bool";
const IMS_CHAR CarrierConfig::KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL[] =
        "carrier_cross_sim_ims_available_bool";
const IMS_CHAR CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL[] =
        "support_emergency_sms_over_ims_bool";
const IMS_CHAR CarrierConfig::KEY_CARRIER_USSD_METHOD_INT[] = "carrier_ussd_method_int";
const IMS_CHAR CarrierConfig::KEY_RTT_SUPPORTED_BOOL[] = "rtt_supported_bool";
const IMS_CHAR CarrierConfig::KEY_RTT_SUPPORTED_WHILE_ROAMING_BOOL[] =
        "rtt_supported_while_roaming_bool";
const IMS_CHAR CarrierConfig::KEY_CARRIER_VOLTE_TTY_SUPPORTED_BOOL[] =
        "carrier_volte_tty_supported_bool";
const IMS_CHAR CarrierConfig::KEY_USE_RCS_SIP_OPTIONS_BOOL[] = "use_rcs_sip_options_bool";
const IMS_CHAR CarrierConfig::KEY_SUPPORT_PAUSE_IMS_VIDEO_CALLS_BOOL[] =
        "support_pause_ims_video_calls_bool";
const IMS_CHAR CarrierConfig::KEY_SUPPORT_ADHOC_CONFERENCE_CALLS_BOOL[] =
        "support_adhoc_conference_calls_bool";
const IMS_CHAR CarrierConfig::KEY_SUPPORT_ADD_CONFERENCE_PARTICIPANTS_BOOL[] =
        "support_add_conference_participants_bool";
const IMS_CHAR CarrierConfig::KEY_IS_IMS_CONFERENCE_SIZE_ENFORCED_BOOL[] =
        "is_ims_conference_size_enforced_bool";
const IMS_CHAR CarrierConfig::KEY_IMS_CONFERENCE_SIZE_LIMIT_INT[] = "ims_conference_size_limit_int";
const IMS_CHAR CarrierConfig::KEY_SUPPORTS_DEVICE_TO_DEVICE_COMMUNICATION_USING_RTP_BOOL[] =
        "supports_device_to_device_communication_using_rtp_bool";
const IMS_CHAR CarrierConfig::KEY_SUPPORTS_DEVICE_TO_DEVICE_COMMUNICATION_USING_DTMF_BOOL[] =
        "supports_device_to_device_communication_using_dtmf_bool";
const IMS_CHAR CarrierConfig::KEY_SUPPORTS_SDP_NEGOTIATION_OF_D2D_RTP_HEADER_EXTENSIONS_BOOL[] =
        "supports_sdp_negotiation_of_d2d_rtp_header_extensions_bool";
const IMS_CHAR CarrierConfig::KEY_CARRIER_NR_AVAILABILITIES_INT_ARRAY[] =
        "carrier_nr_availabilities_int_array";
const IMS_CHAR CarrierConfig::KEY_RTT_AUTO_UPGRADE_BOOL[] = "rtt_auto_upgrade_bool";
const IMS_CHAR CarrierConfig::KEY_RTT_SUPPORTED_FOR_VT_BOOL[] = "rtt_supported_for_vt_bool";
const IMS_CHAR CarrierConfig::KEY_RTT_UPGRADE_SUPPORTED_BOOL[] = "rtt_upgrade_supported_bool";
const IMS_CHAR CarrierConfig::KEY_RTT_DOWNGRADE_SUPPORTED_BOOL[] = "rtt_downgrade_supported_bool";
const IMS_CHAR CarrierConfig::KEY_SUPPORT_IMS_CONFERENCE_EVENT_PACKAGE_BOOL[] =
        "support_ims_conference_event_package_bool";
const IMS_CHAR CarrierConfig::KEY_SUPPORT_IMS_CONFERENCE_EVENT_PACKAGE_ON_PEER_BOOL[] =
        "support_ims_conference_event_package_on_peer_bool";
const IMS_CHAR CarrierConfig::KEY_IWLAN_HANDOVER_POLICY_STRING_ARRAY[] =
        "iwlan_handover_policy_string_array";
// Public carrier-config - ends

//// Ims
#define KEY_IMS_PREFIX "ims."
// Public carrier-config - starts
const IMS_CHAR CarrierConfig::Ims::KEY_IMS_SINGLE_REGISTRATION_REQUIRED_BOOL[] =
        KEY_IMS_PREFIX "ims_single_registration_required_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_ENABLE_PRESENCE_PUBLISH_BOOL[] =
        KEY_IMS_PREFIX "enable_presence_publish_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_ENABLE_PRESENCE_CAPABILITY_EXCHANGE_BOOL[] =
        KEY_IMS_PREFIX "enable_presence_capability_exchange_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_RCS_BULK_CAPABILITY_EXCHANGE_BOOL[] =
        KEY_IMS_PREFIX "rcs_bulk_capability_exchange_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_ENABLE_PRESENCE_GROUP_SUBSCRIBE_BOOL[] =
        KEY_IMS_PREFIX "enable_presence_group_subscribe_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_USE_SIP_URI_FOR_PRESENCE_SUBSCRIBE_BOOL[] =
        KEY_IMS_PREFIX "use_sip_uri_for_presence_subscribe_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_NON_RCS_CAPABILITIES_CACHE_EXPIRATION_SEC_INT[] =
        KEY_IMS_PREFIX "non_rcs_capabilities_cache_expiration_sec_int";
const IMS_CHAR CarrierConfig::Ims::KEY_RCS_FEATURE_TAG_ALLOWED_STRING_ARRAY[] =
        KEY_IMS_PREFIX "rcs_feature_tag_allowed_string_array";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_TIMER_T1_MILLIS_INT[] =
        KEY_IMS_PREFIX "sip_timer_t1_millis_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_TIMER_T2_MILLIS_INT[] =
        KEY_IMS_PREFIX "sip_timer_t2_millis_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_TIMER_T4_MILLIS_INT[] =
        KEY_IMS_PREFIX "sip_timer_t4_millis_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_TIMER_B_MILLIS_INT[] =
        KEY_IMS_PREFIX "sip_timer_b_millis_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_TIMER_C_MILLIS_INT[] =
        KEY_IMS_PREFIX "sip_timer_c_millis_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_TIMER_D_MILLIS_INT[] =
        KEY_IMS_PREFIX "sip_timer_d_millis_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_TIMER_F_MILLIS_INT[] =
        KEY_IMS_PREFIX "sip_timer_f_millis_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_TIMER_H_MILLIS_INT[] =
        KEY_IMS_PREFIX "sip_timer_h_millis_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_TIMER_J_MILLIS_INT[] =
        KEY_IMS_PREFIX "sip_timer_j_millis_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_SERVER_PORT_NUMBER_INT[] =
        KEY_IMS_PREFIX "sip_server_port_number_int";
const IMS_CHAR CarrierConfig::Ims::KEY_PHONE_CONTEXT_DOMAIN_NAME_STRING[] =
        KEY_IMS_PREFIX "phone_context_domain_name_string";
const IMS_CHAR CarrierConfig::Ims::KEY_REQUEST_URI_TYPE_INT[] =
        KEY_IMS_PREFIX "request_uri_type_int";
const IMS_CHAR CarrierConfig::Ims::KEY_GRUU_ENABLED_BOOL[] = KEY_IMS_PREFIX "gruu_enabled_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_KEEP_PDN_UP_IN_NO_VOPS_BOOL[] =
        KEY_IMS_PREFIX "keep_pdn_up_in_no_vops_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_PREFERRED_TRANSPORT_INT[] =
        KEY_IMS_PREFIX "sip_preferred_transport_int";
const IMS_CHAR CarrierConfig::Ims::KEY_IPV4_SIP_MTU_SIZE_CELLULAR_INT[] =
        KEY_IMS_PREFIX "ipv4_sip_mtu_size_cellular_int";
const IMS_CHAR CarrierConfig::Ims::KEY_IPV6_SIP_MTU_SIZE_CELLULAR_INT[] =
        KEY_IMS_PREFIX "ipv6_sip_mtu_size_cellular_int";
const IMS_CHAR CarrierConfig::Ims::KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY[] =
        KEY_IMS_PREFIX "ims_pdn_enabled_in_no_vops_support_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_OVER_IPSEC_ENABLED_BOOL[] =
        KEY_IMS_PREFIX "sip_over_ipsec_enabled_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_IPSEC_AUTHENTICATION_ALGORITHMS_INT_ARRAY[] =
        KEY_IMS_PREFIX "ipsec_authentication_algorithms_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_IPSEC_ENCRYPTION_ALGORITHMS_INT_ARRAY[] =
        KEY_IMS_PREFIX "ipsec_encryption_algorithms_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT[] =
        KEY_IMS_PREFIX "registration_expiry_timer_sec_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REGISTRATION_RETRY_BASE_TIMER_MILLIS_INT[] =
        KEY_IMS_PREFIX "registration_retry_base_timer_millis_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REGISTRATION_RETRY_MAX_TIMER_MILLIS_INT[] =
        KEY_IMS_PREFIX "registration_retry_max_timer_millis_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REGISTRATION_EVENT_PACKAGE_SUPPORTED_BOOL[] =
        KEY_IMS_PREFIX "registration_event_package_supported_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT[] =
        KEY_IMS_PREFIX "registration_subscribe_expiry_timer_sec_int";
const IMS_CHAR CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_IN_SIP_REGISTER_SUPPORT_INT_ARRAY[] =
        KEY_IMS_PREFIX "geolocation_pidf_in_sip_register_support_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY[] =
        KEY_IMS_PREFIX "geolocation_pidf_in_sip_invite_support_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_IMS_USER_AGENT_STRING[] =
        KEY_IMS_PREFIX "ims_user_agent_string";
const IMS_CHAR CarrierConfig::Ims::KEY_SUPPORTED_RATS_INT_ARRAY[] =
        KEY_IMS_PREFIX "supported_rats_int_array";
// Bundle {
const IMS_CHAR CarrierConfig::Ims::KEY_MMTEL_REQUIRES_PROVISIONING_BUNDLE[] =
        KEY_IMS_PREFIX "mmtel_requires_provisioning_bundle";
const IMS_CHAR CarrierConfig::Ims::KEY_CAPABILITY_TYPE_VOICE_INT_ARRAY[] =
        KEY_IMS_PREFIX "capability_type_voice_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_CAPABILITY_TYPE_VIDEO_INT_ARRAY[] =
        KEY_IMS_PREFIX "capability_type_video_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_CAPABILITY_TYPE_UT_INT_ARRAY[] =
        KEY_IMS_PREFIX "capability_type_ut_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_CAPABILITY_TYPE_SMS_INT_ARRAY[] =
        KEY_IMS_PREFIX "capability_type_sms_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_CAPABILITY_TYPE_CALL_COMPOSER_INT_ARRAY[] =
        KEY_IMS_PREFIX "capability_type_call_composer_int_array";
// }
// Bundle {
const IMS_CHAR CarrierConfig::Ims::KEY_RCS_REQUIRES_PROVISIONING_BUNDLE[] =
        KEY_IMS_PREFIX "rcs_requires_provisioning_bundle";
const IMS_CHAR CarrierConfig::Ims::KEY_CAPABILITY_TYPE_OPTIONS_UCE_INT_ARRAY[] =
        KEY_IMS_PREFIX "capability_type_options_uce_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_CAPABILITY_TYPE_PRESENCE_UCE_INT_ARRAY[] =
        KEY_IMS_PREFIX "capability_type_presence_uce_int_array";
// }
// Public carrier-config - ends

// Ims General
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_COMPACT_FORM_ENABLED_BOOL[] =
        KEY_IMS_PREFIX "sip_compact_form_enabled_bool";
const IMS_CHAR
        CarrierConfig::Ims::KEY_ALLOW_SIP_UDP_FALLBACK_ON_TCP_CONNECTION_SETUP_FAILED_BOOL[] =
                KEY_IMS_PREFIX "allow_sip_udp_fallback_on_tcp_connection_setup_failed_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_ALLOW_ALGORITHM_PARAM_IN_SIP_AUTHORIZATION_HEADER_BOOL[] =
        KEY_IMS_PREFIX "allow_algorithm_param_in_sip_authorization_header_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_USE_SIP_USER_AGENT_HEADER_IN_UA_STRING_BOOL[] =
        KEY_IMS_PREFIX "use_sip_user_agent_header_in_ua_string_bool";
const IMS_CHAR
        CarrierConfig::Ims::KEY_ALLOW_SIP_P_ACCESS_NETWORK_INFO_HEADER_IN_INITIAL_REGISTER_BOOL[] =
                KEY_IMS_PREFIX "allow_sip_p_access_network_info_header_in_initial_register_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_REQUIRE_SIP_EXPIRES_HEADER_IN_REGISTER_BOOL[] =
        KEY_IMS_PREFIX "require_sip_expires_header_in_register_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_SUPPORT_SIP_SESSION_ID_HEADER_BOOL[] =
        KEY_IMS_PREFIX "support_sip_session_id_header_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_REGISTRATION_CONTACT_USER_INFO_PART_INT[] =
        KEY_IMS_PREFIX "registration_contact_user_info_part_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_DEVICE_ID_TYPE_INT[] =
        KEY_IMS_PREFIX "sip_device_id_type_int";
const IMS_CHAR CarrierConfig::Ims::KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY[] =
        KEY_IMS_PREFIX "pcscf_discovery_method_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_SDP_NEGOTIATION_REQUIRED_FOR_NON_RPR_BOOL[] =
        KEY_IMS_PREFIX "sdp_negotiation_required_for_non_rpr_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_REQUEST_URI_VALIDATION_REQUIRED_IN_MID_DIALOG_BOOL[] =
        KEY_IMS_PREFIX "request_uri_validation_required_in_mid_dialog_bool";
const IMS_CHAR
        CarrierConfig::Ims::KEY_SESSION_TIMER_UPDATE_REQUIRED_IN_SESSION_UPDATE_BY_REINVITE_BOOL[] =
                KEY_IMS_PREFIX "session_timer_update_required_in_session_update_by_reinvite_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_ALLOW_SESSION_TIMER_TURN_OFF_BOOL[] =
        KEY_IMS_PREFIX "allow_session_timer_turn_off_bool";
const IMS_CHAR CarrierConfig::Ims::
        KEY_ALLOW_SIP_INSTANCE_PARAM_IN_CONTACT_FOR_NON_REGISTER_REQUEST_BOOL[] =
                KEY_IMS_PREFIX "allow_sip_instance_param_in_contact_for_non_register_request_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_TIMER_100_TRYING_MILLIS_INT[] =
        KEY_IMS_PREFIX "sip_timer_100_trying_millis_int";
const IMS_CHAR CarrierConfig::Ims::KEY_USE_RESET_WHEN_CLOSING_SIP_TCP_CONNECTION_BOOL[] =
        KEY_IMS_PREFIX "use_reset_when_closing_sip_tcp_connection_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_USE_TUPLE_ELEMENT_IN_GEOLOCATION_PIDF_BOOL[] =
        KEY_IMS_PREFIX "use_tuple_element_in_geolocation_pidf_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_ALLOW_UNKNOWN_COUNTRY_ELEMENT_IN_GEOLOCATION_PIDF_BOOL[] =
        KEY_IMS_PREFIX "allow_unknown_country_element_in_geolocation_pidf_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_ALLOW_NO_POSITION_IN_GEOLOCATION_PIDF_BOOL[] =
        KEY_IMS_PREFIX "allow_no_position_in_geolocation_pidf";
const IMS_CHAR CarrierConfig::Ims::KEY_SET_SDP_DIRECTION_ATTRIBUTE_FOR_REMOVED_MEDIA_BOOL[] =
        KEY_IMS_PREFIX "set_sdp_direction_attribute_for_removed_media_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_SUPPORT_SDP_PRECONDITION_BOOL[] =
        KEY_IMS_PREFIX "support_sdp_precondition_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_HIDE_MAC_ADDRESS_IN_PANI_HEADER_INT[] =
        KEY_IMS_PREFIX "hide_mac_address_in_pani_header_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SUPPORT_COUNTRY_PARAM_IN_PANI_HEADER_BOOL[] =
        KEY_IMS_PREFIX "support_country_param_in_pani_header_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_SUPPORT_LOCAL_SESSION_TIMER_BOOL[] =
        KEY_IMS_PREFIX "support_local_session_timer_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_SUPPORT_MULTIPLE_REGISTRATION_INT[] =
        KEY_IMS_PREFIX "support_multiple_registration_int";
const IMS_CHAR CarrierConfig::Ims::KEY_RETRANSMISSION_ALLOWED_OF_GEOLOCATION_PIDF_STRING[] =
        KEY_IMS_PREFIX "retransmission_allowed_of_geolocation_pidf_string";

// Aos
const IMS_CHAR CarrierConfig::Ims::KEY_UNSUBSCRIBE_REGISTRATION_EVENT_PACKAGE_BOOL[] =
        KEY_IMS_PREFIX "unsubscribe_registration_event_package_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_USE_TCP_TRANSPORT_FOR_REGISTER_BOOL[] =
        KEY_IMS_PREFIX "use_tcp_transport_for_register_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_ISIM_INDEX_FOR_IMPU_INT[] =
        KEY_IMS_PREFIX "isim_index_for_impu_int";
const IMS_CHAR CarrierConfig::Ims::KEY_PREFERRED_IMS_DSCP_INT[] =
        KEY_IMS_PREFIX "preferred_ims_dscp_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REGISTRATION_PREFERRED_ACCESSTYPE_FEATURE_TAG_INT[] =
        KEY_IMS_PREFIX "registration_preferred_accesstype_feature_tag_int";
const IMS_CHAR CarrierConfig::Ims::KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY[] =
        KEY_IMS_PREFIX "ims_identity_priority_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY[] =
        KEY_IMS_PREFIX "registration_permanent_error_code_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_UPDATE_REGISTRATION_WITH_RAT_CHANGE_INT_ARRAY[] =
        KEY_IMS_PREFIX "update_registration_with_rat_change_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_B2C_CALL_COMPOSER_FEATURE_TAG_IN_REG_CONTACT_BOOL[] =
        KEY_IMS_PREFIX "b2c_call_composer_feature_tag_in_reg_contact_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_BLOCK_PCSCF_ON_REG_FAILURE_BOOL[] =
        KEY_IMS_PREFIX "block_pcscf_on_reg_failure_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_CALL_END_AND_PDN_REACTIVATION_BY_REG_TERMINATED_BOOL[] =
        KEY_IMS_PREFIX "call_end_and_pdn_reactivation_by_reg_terminated_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_DESTROY_UNSECURE_TCP_SOCKET_ON_ACCOMPLISHING_REG_BOOL[] =
        KEY_IMS_PREFIX "destroy_unsecure_tcp_socket_on_accomplishing_reg_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_HOLD_REG_WITH_IPCAN_CHANGED_DURING_IMS_CALL_BOOL[] =
        KEY_IMS_PREFIX "hold_reg_with_ipcan_changed_during_ims_call_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_IMS_DEREG_ON_3G_NETWORK_BOOL[] =
        KEY_IMS_PREFIX "ims_dereg_on_3g_network_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_IMSI_BASED_URI_PRIORITIZED_BOOL[] =
        KEY_IMS_PREFIX "imsi_based_uri_prioritized_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_INIT_IPSEC_SETTING_WITH_NEW_PCSCF_BOOL[] =
        KEY_IMS_PREFIX "init_ipsec_setting_with_new_pcscf_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_NO_INIT_REG_ON_PCSCF_CHANGE_BOOL[] =
        KEY_IMS_PREFIX "no_init_reg_on_pcscf_change_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_CONTACT_VALIDATION_BOOL[] =
        KEY_IMS_PREFIX "reg_contact_validation_bool";
const IMS_CHAR
        CarrierConfig::Ims::KEY_REG_PLMN_BLOCK_WITH_TIMEOUT_ON_FAILURE_WITH_ALL_PCSCFS_BOOL[] =
                KEY_IMS_PREFIX "reg_plmn_block_with_timeout_on_failure_with_all_pcscfs_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_RETRY_WITH_IP_VER_FALLBACK_BOOL[] =
        KEY_IMS_PREFIX "reg_retry_with_ip_ver_fallback_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_REMOVE_OLD_SA_ON_ESTABLISHING_SA_BOOL[] =
        KEY_IMS_PREFIX "remove_old_sa_on_establishing_sa_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_REQUIRED_CDMALESS_FEATURE_TAG_BOOL[] =
        KEY_IMS_PREFIX "required_cdmaless_feature_tag_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_REQUIRED_INIT_REG_AFTER_IMS_CALL_END_ON_REG_HELD_BOOL[] =
        KEY_IMS_PREFIX "required_init_reg_after_ims_call_end_on_reg_held_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_OVER_IPSEC_ENABLED_IN_ROAMING_BOOL[] =
        KEY_IMS_PREFIX "sip_over_ipsec_enabled_in_roaming_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_SUPPORT_REG_WITH_FEATURE_TAG_UNAVAILABLE_BOOL[] =
        KEY_IMS_PREFIX "support_reg_with_feature_tag_unavailable_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_SUPPORT_VERSTAT_BASED_ON_NETWORK_FOR_REG_BOOL[] =
        KEY_IMS_PREFIX "support_verstat_based_on_network_for_reg_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_SUPPORT_VERSTAT_FOR_REG_BOOL[] =
        KEY_IMS_PREFIX "support_verstat_for_reg_bool";
const IMS_CHAR
        CarrierConfig::Ims::KEY_USE_RCS_TELEPHONY_FEATURE_TAG_AS_AVAILABLE_VOICE_CALL_TYPE_BOOL[] =
                KEY_IMS_PREFIX "use_rcs_telephony_feature_tag_as_available_voice_call_type_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_USE_SECURITY_SERVER_PORT_IN_INIT_REG_BOOL[] =
        KEY_IMS_PREFIX "use_security_server_port_in_init_reg_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_USE_SECURITY_SERVER_PORT_IN_REG_CONTACT_OF_INIT_REG_BOOL[] =
        KEY_IMS_PREFIX "use_security_server_port_in_reg_contact_of_init_reg_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_CONTACT_USER_INFO_POLICY_FOR_NON_REG_MESSAGE_INT[] =
        KEY_IMS_PREFIX "contact_user_info_policy_for_non_reg_message_int";
const IMS_CHAR CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_FORMING_POLICY_INT[] =
        KEY_IMS_PREFIX "geolocation_pidf_forming_policy_int";
const IMS_CHAR CarrierConfig::Ims::KEY_IMS_ESTABLISHMENT_TIME_SEC_INT[] =
        KEY_IMS_PREFIX "ims_establishment_time_sec_int";
const IMS_CHAR CarrierConfig::Ims::KEY_IMS_PREFERRED_IPTYPE_INT[] =
        KEY_IMS_PREFIX "ims_preferred_iptype_int";
const IMS_CHAR CarrierConfig::Ims::KEY_IMS_SIGNALLING_DSCP_INT[] =
        KEY_IMS_PREFIX "ims_signalling_dscp_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_ACTUAL_WAIT_TIME_POLICY_INT[] =
        KEY_IMS_PREFIX "reg_actual_wait_time_policy_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_DEFAULT_WAIT_TIME_INT[] =
        KEY_IMS_PREFIX "reg_default_wait_time_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_OUT_OF_SERVICE_POLICY_INT[] =
        KEY_IMS_PREFIX "reg_out_of_service_policy_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_PCSCF_UPDATE_POLICY_INT[] =
        KEY_IMS_PREFIX "reg_pcscf_update_policy_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_RETRY_305_POLICY_INT[] =
        KEY_IMS_PREFIX "reg_retry_305_policy_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_RETRY_503_POLICY_INT[] =
        KEY_IMS_PREFIX "reg_retry_503_policy_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_RETRY_CNT_ON_SINGLE_PCSCF_INT[] =
        KEY_IMS_PREFIX "reg_retry_cnt_on_single_pcscf_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_RETRY_CNT_PER_PCSCF_INT[] =
        KEY_IMS_PREFIX "reg_retry_cnt_per_pcscf_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_RETRY_CNT_RESET_POLICY_INT[] =
        KEY_IMS_PREFIX "reg_retry_cnt_reset_policy_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_RETRY_CNT_WITH_IPSEC_ON_AUTH_FAILURE_INT[] =
        KEY_IMS_PREFIX "reg_retry_cnt_with_ipsec_on_auth_failure_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_RETRY_DEFAULT_POLICY_INT[] =
        KEY_IMS_PREFIX "reg_retry_default_policy_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_RETRY_TIMER_F_POLICY_INT[] =
        KEY_IMS_PREFIX "reg_retry_timer_f_policy_int";
const IMS_CHAR CarrierConfig::Ims::KEY_REREG_RETRY_305_POLICY_INT[] =
        KEY_IMS_PREFIX "rereg_retry_305_policy_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SIP_MESSAGE_THRESHOLD_FOR_TRANSPORT_CHANGE_INT[] =
        KEY_IMS_PREFIX "sip_message_threshold_for_transport_change_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SUB_RETRY_503_POLICY_INT[] =
        KEY_IMS_PREFIX "sub_retry_503_policy_int";
const IMS_CHAR CarrierConfig::Ims::KEY_USAT_REG_EVENT_DOWNLOAD_POLICY_INT[] =
        KEY_IMS_PREFIX "usat_reg_event_download_policy_int";
const IMS_CHAR CarrierConfig::Ims::KEY_PERMANENT_PDN_FAILURE_INT_ARRAY[] =
        KEY_IMS_PREFIX "permanent_pdn_failure_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY[] =
        KEY_IMS_PREFIX "reg_err_code_for_pcscf_discovery_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_PERMANENT_ERR_MAX_CNT_INT_ARRAY[] =
        KEY_IMS_PREFIX "reg_permanent_err_max_cnt_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_RETRY_ERR_CODE_WITHOUT_IPSEC_INT_ARRAY[] =
        KEY_IMS_PREFIX "reg_retry_err_code_without_ipsec_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_REREG_ERR_CODE_FOR_CALL_END_INT_ARRAY[] =
        KEY_IMS_PREFIX "rereg_err_code_for_call_end_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_REREG_ERR_CODE_FOR_IMS_PDN_REACTIVATION_INT_ARRAY[] =
        KEY_IMS_PREFIX "rereg_err_code_for_ims_pdn_reactivation_int_array";
const IMS_CHAR
        CarrierConfig::Ims::KEY_REREG_ERR_CODE_FOR_INIT_REG_WITH_AVAILABLE_PCSCF_INT_ARRAY[] =
                KEY_IMS_PREFIX "rereg_err_code_for_init_reg_with_available_pcscf_int_array";
const IMS_CHAR
        CarrierConfig::Ims::KEY_REREG_RETRY_ERR_CODE_FOR_INIT_REG_WITH_SAME_PCSCF_INT_ARRAY[] =
                KEY_IMS_PREFIX "rereg_retry_err_code_for_init_reg_with_same_pcscf_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_NEXT_PCSCF_INT_ARRAY[] =
        KEY_IMS_PREFIX "sub_err_code_for_init_reg_with_next_pcscf_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_STOPPING_BY_EXPIRATION_TIME_INT_ARRAY[] =
        KEY_IMS_PREFIX "sub_err_code_for_stopping_by_expiration_time_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_SUPPORTED_ROAMING_RATS_INT_ARRAY[] =
        KEY_IMS_PREFIX "supported_roaming_rats_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_TEST_MODE_INT_ARRAY[] = KEY_IMS_PREFIX "test_mode_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_UNAVAILABLE_FEATURES_IN_LIMITED_REG_INT_ARRAY[] =
        KEY_IMS_PREFIX "unavailable_features_in_limited_reg_int_array";
// Bundle {
const IMS_CHAR CarrierConfig::Ims::KEY_EXTRA_REG_ERR_BUNDLE[] =
        KEY_IMS_PREFIX "extra_reg_err_bundle";
const IMS_CHAR CarrierConfig::Ims::KEY_EXTRA_REG_ERR_CODE_AS_FAILURE_IN_ROAMING_FOR_UPDATE_BOOL[] =
        KEY_IMS_PREFIX "extra_reg_err_code_as_failure_in_roaming_for_update_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_EXTRA_REG_ERR_RETRY_CNT_SHARED_FOR_REG_AND_SUB_BOOL[] =
        KEY_IMS_PREFIX "extra_reg_err_retry_cnt_shared_for_reg_and_sub_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_EXTRA_REG_ERR_FINAL_TYPE_INT[] =
        KEY_IMS_PREFIX "extra_reg_err_final_type_int";
const IMS_CHAR CarrierConfig::Ims::KEY_EXTRA_REG_ERR_MAX_CNT_INT[] =
        KEY_IMS_PREFIX "extra_reg_err_max_cnt_int";
const IMS_CHAR
        CarrierConfig::Ims::KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_EPS_5GS_ONLY_ATTACHED_INT[] =
                KEY_IMS_PREFIX "extra_reg_err_pcscfs_repeated_cnt_for_eps_5gs_only_attached_int";
const IMS_CHAR
        CarrierConfig::Ims::KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_LTE_COMBINED_ATTACHED_INT[] =
                KEY_IMS_PREFIX "extra_reg_err_pcscfs_repeated_cnt_for_lte_combined_attached_int";
const IMS_CHAR CarrierConfig::Ims::KEY_EXTRA_REG_ERR_POLICY_INT[] =
        KEY_IMS_PREFIX "extra_reg_err_policy_int";
const IMS_CHAR CarrierConfig::Ims::KEY_EXTRA_REG_ERR_CODE_INT_ARRAY[] =
        KEY_IMS_PREFIX "extra_reg_err_code_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_EXTRA_REG_ERR_CODE_FOR_UPDATE_INT_ARRAY[] =
        KEY_IMS_PREFIX "extra_reg_err_code_for_update_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_EXTRA_REG_ERR_WAIT_TIME_SEC_INT_ARRAY[] =
        KEY_IMS_PREFIX "extra_reg_err_wait_time_sec_int_array";

// }
// Bundle {
const IMS_CHAR CarrierConfig::Ims::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE[] =
        KEY_IMS_PREFIX "notify_terminated_for_init_reg_bundle";
const IMS_CHAR CarrierConfig::Ims::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_WITH_WAIT_TIME_INT[] =
        KEY_IMS_PREFIX "notify_terminated_for_init_reg_with_wait_time_int";
const IMS_CHAR CarrierConfig::Ims::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY[] =
        KEY_IMS_PREFIX "notify_terminated_for_init_reg_used_event_int_array";
const IMS_CHAR CarrierConfig::Ims::
        KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_WITH_WAIT_TIME_INT_ARRAY[] =
                KEY_IMS_PREFIX "notify_terminated_for_init_reg_used_event_with_wait_time_int_array";
// }
// Bundle {
const IMS_CHAR CarrierConfig::Ims::KEY_PCSCF_RECOVERY_CONDITIONS_BUNDLE[] =
        KEY_IMS_PREFIX "pcscf_recovery_conditions_bundle";
const IMS_CHAR CarrierConfig::Ims::KEY_PCSCF_RECOVERY_MAX_CNT_INT[] =
        KEY_IMS_PREFIX "pcscf_recovery_max_cnt_int";
const IMS_CHAR CarrierConfig::Ims::KEY_PCSCF_RECOVERY_WAIT_TIME_SEC_INT[] =
        KEY_IMS_PREFIX "pcscf_recovery_wait_time_sec_int";
const IMS_CHAR CarrierConfig::Ims::KEY_PCSCF_RECOVERY_BASE_TIME_SEC_INT[] =
        KEY_IMS_PREFIX "pcscf_recovery_base_time_sec_int";
const IMS_CHAR CarrierConfig::Ims::KEY_PCSCF_RECOVERY_MAX_TIME_SEC_INT[] =
        KEY_IMS_PREFIX "pcscf_recovery_max_time_sec_int";
// }
// Bundle {
const IMS_CHAR CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_BUNDLE[] =
        KEY_IMS_PREFIX "reg_err_code_with_ra_time_bundle";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_ONLY_DEFINED_BOOL[] =
        KEY_IMS_PREFIX "reg_err_code_with_ra_time_only_defined_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_INT_ARRAY[] =
        KEY_IMS_PREFIX "reg_err_code_with_ra_time_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_FOR_UPDATE_INT_ARRAY[] =
        KEY_IMS_PREFIX "reg_err_code_with_ra_time_for_update_int_array";
// }
// Bundle {
const IMS_CHAR CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_BUNDLE[] =
        KEY_IMS_PREFIX "reg_retry_interval_bundle";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_USED_FOR_SUB_BOOL[] =
        KEY_IMS_PREFIX "reg_retry_interval_used_for_sub_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_RANDOM_UPPER_VALUE_SEC_INT_ARRAY[] =
        KEY_IMS_PREFIX "reg_retry_interval_random_upper_value_sec_int_array";
const IMS_CHAR CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_SEC_INT_ARRAY[] =
        KEY_IMS_PREFIX "reg_retry_interval_sec_int_array";
// }
// Bundle {
const IMS_CHAR CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_BUNDLE[] =
        KEY_IMS_PREFIX "sub_err_code_for_init_reg_bundle";
const IMS_CHAR CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_RETRY_MAX_CNT_INT[] =
        KEY_IMS_PREFIX "sub_err_code_for_init_reg_with_retry_max_cnt_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY[] =
        KEY_IMS_PREFIX "sub_err_code_for_init_reg_int_array";
// }
// Bundle {
const IMS_CHAR CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_TERMINATED_BUNDLE[] =
        KEY_IMS_PREFIX "sub_err_code_for_terminated_bundle";
const IMS_CHAR CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_TERMINATED_WITH_RETRY_MAX_CNT_INT[] =
        KEY_IMS_PREFIX "sub_err_code_for_terminated_with_retry_max_cnt_int";
const IMS_CHAR CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_TERMINATED_INT_ARRAY[] =
        KEY_IMS_PREFIX "sub_err_code_for_terminated_int_array";
// }

//// ImsEmergency
#define KEY_IMS_EMERGENCY_PREFIX "imsemergency."
// Public carrier-config - starts
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "retry_emergency_on_ims_pdn_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EMERGENCY_CALLBACK_MODE_SUPPORTED_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "emergency_callback_mode_supported_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EMERGENCY_QOS_PRECONDITION_SUPPORTED_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "emergency_qos_precondition_supported_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EMERGENCY_OVER_IMS_SUPPORTED_RATS_INT_ARRAY[] =
        KEY_IMS_EMERGENCY_PREFIX "emergency_over_ims_supported_rats_int_array";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "emergency_registration_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "refresh_geolocation_timeout_millis_int";
// Public carrier-config - ends
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EMERGENCY_INVITE_18X_TIMER_MILLIS_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "emergency_invite_18x_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsEmergency::
        KEY_EMERGENCY_PROVISIONAL_TO_FINAL_RESPONSE_TIMER_MILLIS_INT[] =
                KEY_IMS_EMERGENCY_PREFIX "emergency_provisional_to_final_response_timer_millis_int";

// Aos
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_RELEASE_EPDN_UPON_ECALL_END_IN_FAKE_MODE_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "release_epdn_upon_ecall_end_in_fake_mode_bool";
const IMS_CHAR
        CarrierConfig::ImsEmergency::KEY_IPCAN_RELEASE_EMERGENCY_PDN_UPON_EMERGENCY_CALL_END_INT[] =
                KEY_IMS_EMERGENCY_PREFIX "ipcan_release_emergency_pdn_upon_emergency_call_end_int";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_PREFERRED_EMERGENCY_REGISTRATION_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "preferred_emergency_registration_int";
const IMS_CHAR
        CarrierConfig::ImsEmergency::KEY_WAIT_TIME_MILLIS_FOR_RELEASE_EPDN_AFTER_ECALL_END_INT[] =
                KEY_IMS_EMERGENCY_PREFIX "wait_time_millis_for_release_epdn_after_ecall_end_int";
const IMS_CHAR
        CarrierConfig::ImsEmergency::KEY_ECALL_BASED_ON_P_ASSOCIATED_URI_OF_NORMAL_REG_BOOL[] =
                KEY_IMS_EMERGENCY_PREFIX "ecall_based_on_p_associated_uri_of_normal_reg_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EREG_ON_RANDOM_PCSCF_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "ereg_on_random_pcscf_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EREG_SET_TCP_ONLY_IN_ROAMING_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "ereg_set_tcp_only_in_roaming_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EREG_USING_FIRST_IMPU_IN_ISIM_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "ereg_using_first_impu_in_isim_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_KEEP_EPDN_UPON_PCSCF_UNAVAILABLE_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "keep_epdn_upon_pcscf_unavailable_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_KEEP_EREG_RETRY_ON_WLAN_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "keep_ereg_retry_on_wlan_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_RELEASE_EPDN_OF_UNAVAILABLE_NETWORK_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "release_epdn_of_unavailable_network_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_STOP_EREG_TIMER_ON_EPDN_CONNECTED_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "stop_ereg_timer_on_epdn_connected_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_SUPPORT_EREREG_ON_IPCAN_CHANGE_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "support_erereg_on_ipcan_change_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_SUPPORT_GIBA_FOR_EREG_IN_ROAMING_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "support_giba_for_ereg_in_roaming_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_SUPPORT_VIDEO_FOR_EREG_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "support_video_for_ereg_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_USE_REG_RETRY_RULE_FOR_EREG_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "use_reg_retry_rule_for_ereg_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EPDN_PREFERRED_IPTYPE_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "epdn_preferred_iptype_int";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EREG_RETRY_MAX_CNT_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "ereg_retry_max_cnt_int";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EREG_RETRY_TIMER_MILLIS_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "ereg_retry_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_REG_TIMER_FOR_ECALL_MILLIS_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "reg_timer_for_ecall_millis_int";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_ROAMING_PREFERRED_EREG_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "roaming_preferred_ereg_int";
const IMS_CHAR
        CarrierConfig::ImsEmergency::KEY_EREG_ERR_CODE_NOT_SUPPORTED_COMMON_POLICY_INT_ARRAY[] =
                KEY_IMS_EMERGENCY_PREFIX "ereg_err_code_not_supported_common_policy_int_array";

// Mtc
const IMS_CHAR
        CarrierConfig::ImsEmergency::KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_ON_CELLULAR_BOOL[] =
                KEY_IMS_EMERGENCY_PREFIX "emergency_call_over_emergency_pdn_on_cellular_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::
        KEY_EMERGENCY_RETRY_WITHOUT_CHECKING_380_CONTENT_FOR_NON_UE_DETECTABLE_EMERGENCY_CALL_BOOL
                [] = KEY_IMS_EMERGENCY_PREFIX
        "emergency_retry_without_checking_380_content_for_non_ue_detectable_emergency_call_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EMERGENCY_TCALL_TIMER_MILLIS_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "emergency_tcall_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EMERGENCY_RINGBACK_TIMER_MILLIS_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "emergency_ringback_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EMERGENCY_18X_TIMER_MILLIS_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "emergency_18x_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_POLICY_FOR_EMERGENCY_URN_ESCV_MAPPING_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "policy_for_emergency_urn_escv_mapping_int";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_CALL_PERIODIC_LOCATION_DISCOVERY_METHOD_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "call_periodic_location_discovery_method_int";
const IMS_CHAR
        CarrierConfig::ImsEmergency::KEY_CALL_PERIODIC_LOCATION_DISCOVERY_TIMER_MILLIS_INT[] =
                KEY_IMS_EMERGENCY_PREFIX "call_periodic_location_discovery_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_NUMBER_NEED_OIR_STRING_ARRAY[] =
        KEY_IMS_EMERGENCY_PREFIX "number_need_oir_string_array";
const IMS_CHAR CarrierConfig::ImsEmergency::
        KEY_PLMN_ALLOWING_GEOLOCATION_PIDF_IN_SIP_INVITE_NO_UICC_STRING_ARRAY[] =
                KEY_IMS_EMERGENCY_PREFIX
        "plmn_allowing_geolocation_pidf_in_sip_invite_no_uicc_string_array";

//// ImsRtt
#define KEY_IMS_RTT_PREFIX "imsrtt."
// Public carrier-config - starts
const IMS_CHAR CarrierConfig::ImsRtt::KEY_TEXT_ON_DEFAULT_BEARER_SUPPORTED_BOOL[] =
        KEY_IMS_RTT_PREFIX "text_on_default_bearer_supported_bool";
const IMS_CHAR CarrierConfig::ImsRtt::KEY_TEXT_QOS_PRECONDITION_SUPPORTED_BOOL[] =
        KEY_IMS_RTT_PREFIX "text_qos_precondition_supported_bool";
const IMS_CHAR CarrierConfig::ImsRtt::KEY_TEXT_AS_BANDWIDTH_KBPS_INT[] =
        KEY_IMS_RTT_PREFIX "text_as_bandwidth_kbps_int";
const IMS_CHAR CarrierConfig::ImsRtt::KEY_TEXT_RS_BANDWIDTH_BPS_INT[] =
        KEY_IMS_RTT_PREFIX "text_rs_bandwidth_bps_int";
const IMS_CHAR CarrierConfig::ImsRtt::KEY_TEXT_RR_BANDWIDTH_BPS_INT[] =
        KEY_IMS_RTT_PREFIX "text_rr_bandwidth_bps_int";
// Bundle {
const IMS_CHAR CarrierConfig::ImsRtt::KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE[] =
        KEY_IMS_RTT_PREFIX "text_codec_capability_payload_types_bundle";
const IMS_CHAR CarrierConfig::ImsRtt::KEY_T140_PAYLOAD_TYPE_INT[] =
        KEY_IMS_RTT_PREFIX "t140_payload_type_int";
const IMS_CHAR CarrierConfig::ImsRtt::KEY_RED_PAYLOAD_TYPE_INT[] =
        KEY_IMS_RTT_PREFIX "red_payload_type_int";
// }
// Public carrier-config - ends
const IMS_CHAR CarrierConfig::ImsRtt::KEY_POLICY_ON_TEXT_QOS_DEACTIVATION_INT[] =
        KEY_IMS_RTT_PREFIX "policy_on_text_qos_deactivation_int";
const IMS_CHAR CarrierConfig::ImsRtt::KEY_TEXT_RTP_PORT_RANGE_INT_ARRAY[] =
        KEY_IMS_RTT_PREFIX "text_rtp_port_range_int_array";
const IMS_CHAR CarrierConfig::ImsRtt::KEY_TEXT_CODEC_EMPTY_REDUNDANT_BOOL[] =
        KEY_IMS_RTT_PREFIX "text_codec_empty_redundant_bool";
const IMS_CHAR CarrierConfig::ImsRtt::KEY_TEXT_RTP_DSCP_INT[] =
        KEY_IMS_RTT_PREFIX "text_rtp_dscp_int";
const IMS_CHAR CarrierConfig::ImsRtt::KEY_TEXT_CODEC_REDUNDANCY_LEVEL_INT[] =
        KEY_IMS_RTT_PREFIX "text_codec_redundancy_level_int";
const IMS_CHAR CarrierConfig::ImsRtt::KEY_TEXT_RTP_INACTIVITY_TIMER_MILLIS_INT[] =
        KEY_IMS_RTT_PREFIX "text_rtp_inactivity_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsRtt::KEY_TEXT_RTCP_INACTIVITY_TIMER_MILLIS_INT[] =
        KEY_IMS_RTT_PREFIX "text_rtcp_inactivity_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsRtt::KEY_TEXT_RTCP_INTERVAL_INT_ARRAY[] =
        KEY_IMS_RTT_PREFIX "text_rtcp_interval_int_array";

//// ImsSms
#define KEY_IMS_SMS_PREFIX "imssms."
// Public carrier-config - starts
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL[] =
        KEY_IMS_SMS_PREFIX "sms_over_ims_supported_bool";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_CSFB_RETRY_ON_FAILURE_BOOL[] =
        KEY_IMS_SMS_PREFIX "sms_csfb_retry_on_failure_bool";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_FORMAT_INT[] =
        KEY_IMS_SMS_PREFIX "sms_over_ims_format_int";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY[] =
        KEY_IMS_SMS_PREFIX "sms_over_ims_supported_rats_int_array";
// Public carrier-config - ends
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_AVAILABLE_WITHOUT_VOICE_CAPA_BOOL[] =
        KEY_IMS_SMS_PREFIX "sms_over_ims_available_without_voice_capa_bool";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SUPPORT_LIMITED_ADMIN_SMS_MODE_BOOL[] =
        KEY_IMS_SMS_PREFIX "support_limited_admin_sms_mode_bool";

//// ImsUce
#define KEY_IMS_UCE_PREFIX "imsuce."
const IMS_CHAR CarrierConfig::ImsUce::KEY_EXPIRE_VALUE_PUBLISH_SEC_INT[] =
        KEY_IMS_UCE_PREFIX "expire_value_publish_sec_int";
const IMS_CHAR CarrierConfig::ImsUce::KEY_EXTENDED_EXPIRE_VALUE_PUBLISH_SEC_INT[] =
        KEY_IMS_UCE_PREFIX "extended_expire_value_publish_sec_int";
const IMS_CHAR CarrierConfig::ImsUce::KEY_PUBLISH_REFRESH_RATIO_INT[] =
        KEY_IMS_UCE_PREFIX "publish_refresh_ratio_int";
const IMS_CHAR CarrierConfig::ImsUce::KEY_EXPIRE_VALUE_LIST_SUBSCRIBE_SEC_INT[] =
        KEY_IMS_UCE_PREFIX "expire_value_list_subscribe_sec_int";
const IMS_CHAR CarrierConfig::ImsUce::KEY_RLS_URI_STRING[] = KEY_IMS_UCE_PREFIX "rls_uri_string";
const IMS_CHAR CarrierConfig::ImsUce::KEY_SUBSCRIBE_INDEPENDENT_OF_PUBLISH_BOOL[] =
        KEY_IMS_UCE_PREFIX "subscribe_independent_of_publish_bool";
const IMS_CHAR CarrierConfig::ImsUce::KEY_ANONYMOUS_FETCH_METHOD_INT[] =
        KEY_IMS_UCE_PREFIX "anonymous_fetch_method_int";
const IMS_CHAR CarrierConfig::ImsUce::KEY_ENCODE_PUBLISH_BODY_BOOL[] =
        KEY_IMS_UCE_PREFIX "encode_publish_body_bool";
const IMS_CHAR CarrierConfig::ImsUce::KEY_ENCODE_SUBSCRIBE_BODY_BOOL[] =
        KEY_IMS_UCE_PREFIX "encode_subscribe_body_bool";
const IMS_CHAR CarrierConfig::ImsUce::KEY_SUPPORT_OPTIONS_BOOL[] =
        KEY_IMS_UCE_PREFIX "support_options_bool";
const IMS_CHAR CarrierConfig::ImsUce::KEY_USE_SIP_URI_FOR_PRESENCE_SUBSCRIBE_BOOL[] =
        KEY_IMS_UCE_PREFIX "use_sip_uri_for_presence_subscribe_bool";
const IMS_CHAR CarrierConfig::ImsUce::KEY_USE_EXPIRED_ETAG_BOOL[] =
        KEY_IMS_UCE_PREFIX "use_expired_etag_bool";
const IMS_CHAR CarrierConfig::ImsUce::KEY_USE_CONTACT_HEADER_IN_PUBLISH_BOOL[] =
        KEY_IMS_UCE_PREFIX "use_contact_header_in_publish_bool";
const IMS_CHAR CarrierConfig::ImsUce::KEY_USE_CONTACT_HEADER_IN_SUBSCRIBE_BOOL[] =
        KEY_IMS_UCE_PREFIX "use_contact_header_in_subscribe_bool";
const IMS_CHAR CarrierConfig::ImsUce::KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE_INT_ARRAY[] =
        KEY_IMS_UCE_PREFIX "immediately_retry_publish_response_int_array";
const IMS_CHAR CarrierConfig::ImsUce::KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE_MAX_COUNT_INT[] =
        KEY_IMS_UCE_PREFIX "immediately_retry_publish_response_max_count_int";
const IMS_CHAR CarrierConfig::ImsUce::KEY_FIXED_TIME_RETRY_PUBLISH_RESPONSE_INT_ARRAY[] =
        KEY_IMS_UCE_PREFIX "fixed_time_retry_publish_response_int_array";
const IMS_CHAR CarrierConfig::ImsUce::KEY_FIXED_TIME_RETRY_PUBLISH_RESPONSE_MAX_COUNT_INT[] =
        KEY_IMS_UCE_PREFIX "fixed_time_retry_publish_response_max_count_int";
const IMS_CHAR CarrierConfig::ImsUce::KEY_FIXED_TIME_RETRY_PUBLISH_RESPONSE_TIME_SEC_INT[] =
        KEY_IMS_UCE_PREFIX "fixed_time_retry_publish_response_time_sec_int";
const IMS_CHAR CarrierConfig::ImsUce::KEY_VARIABLE_TIME_RETRY_PUBLISH_RESPONSE_INT_ARRAY[] =
        KEY_IMS_UCE_PREFIX "variable_time_retry_publish_response_int_array";
const IMS_CHAR CarrierConfig::ImsUce::KEY_VARIABLE_TIME_RETRY_PUBLISH_RESPONSE_MAX_COUNT_INT[] =
        KEY_IMS_UCE_PREFIX "variable_time_retry_publish_response_max_count_int";
const IMS_CHAR
        CarrierConfig::ImsUce::KEY_VARIABLE_TIME_RETRY_PUBLISH_RESPONSE_TIME_SEC_INT_ARRAY[] =
                KEY_IMS_UCE_PREFIX "variable_time_retry_publish_response_time_sec_int_array";
const IMS_CHAR CarrierConfig::ImsUce::KEY_REATTEMPT_REGISTRATION_PUBLISH_RESPONSE_INT_ARRAY[] =
        KEY_IMS_UCE_PREFIX "reattempt_registration_publish_response_int_array";
const IMS_CHAR CarrierConfig::ImsUce::KEY_REATTEMPT_REGISTRATION_SUBSCRIBE_RESPONSE_INT_ARRAY[] =
        KEY_IMS_UCE_PREFIX "reattempt_registration_subscribe_response_int_array";
const IMS_CHAR CarrierConfig::ImsUce::KEY_ADD_VIDEO_TAG_CONTACT_HEADER_IN_PUBLISH_BOOL[] =
        KEY_IMS_UCE_PREFIX "add_video_tag_contact_header_in_publish_bool";

//// ImsVoice
#define KEY_IMS_VOICE_PREFIX "imsvoice."
// Public carrier-config - starts
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CARRIER_VOLTE_ROAMING_AVAILABLE_BOOL[] =
        KEY_IMS_VOICE_PREFIX "carrier_volte_roaming_available_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL[] =
        KEY_IMS_VOICE_PREFIX "include_caller_id_service_codes_in_sip_invite_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_MULTIENDPOINT_SUPPORTED_BOOL[] =
        KEY_IMS_VOICE_PREFIX "multiendpoint_supported_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SESSION_TIMER_SUPPORTED_BOOL[] =
        KEY_IMS_VOICE_PREFIX "session_timer_supported_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SESSION_EXPIRES_TIMER_SEC_INT[] =
        KEY_IMS_VOICE_PREFIX "session_expires_timer_sec_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_MINIMUM_SESSION_EXPIRES_TIMER_SEC_INT[] =
        KEY_IMS_VOICE_PREFIX "minimum_session_expires_timer_sec_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SESSION_REFRESHER_TYPE_INT[] =
        KEY_IMS_VOICE_PREFIX "session_refresher_type_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SESSION_PRIVACY_TYPE_INT[] =
        KEY_IMS_VOICE_PREFIX "session_privacy_type_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_PRACK_SUPPORTED_FOR_18X_BOOL[] =
        KEY_IMS_VOICE_PREFIX "prack_supported_for_18x_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CONFERENCE_SUBSCRIBE_TYPE_INT[] =
        KEY_IMS_VOICE_PREFIX "conference_subscribe_type_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL[] =
        KEY_IMS_VOICE_PREFIX "voice_qos_precondition_supported_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL[] =
        KEY_IMS_VOICE_PREFIX "voice_on_default_bearer_supported_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "dedicated_bearer_wait_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SRVCC_TYPE_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "srvcc_type_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_RINGING_TIMER_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "ringing_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_RINGBACK_TIMER_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "ringback_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_RTP_INACTIVITY_TIMER_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "audio_rtp_inactivity_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_RTCP_INACTIVITY_TIMER_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "audio_rtcp_inactivity_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CONFERENCE_FACTORY_URI_STRING[] =
        KEY_IMS_VOICE_PREFIX "conference_factory_uri_string";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SESSION_REFRESH_METHOD_INT[] =
        KEY_IMS_VOICE_PREFIX "session_refresh_method_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_OIP_SOURCE_FROM_HEADER_BOOL[] =
        KEY_IMS_VOICE_PREFIX "oip_source_from_header_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "mo_call_request_timeout_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_INACTIVITY_CALL_END_REASONS_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "audio_inactivity_call_end_reasons_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_AS_BANDWIDTH_KBPS_INT[] =
        KEY_IMS_VOICE_PREFIX "audio_as_bandwidth_kbps_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_RS_BANDWIDTH_BPS_INT[] =
        KEY_IMS_VOICE_PREFIX "audio_rs_bandwidth_bps_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_RR_BANDWIDTH_BPS_INT[] =
        KEY_IMS_VOICE_PREFIX "audio_rr_bandwidth_bps_int";
// Bundle {
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE[] =
        KEY_IMS_VOICE_PREFIX "audio_codec_capability_payload_types_bundle";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_EVS_PAYLOAD_TYPE_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "evs_payload_type_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "amrwb_payload_type_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "amrnb_payload_type_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_DTMFWB_PAYLOAD_TYPE_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "dtmfwb_payload_type_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_DTMFNB_PAYLOAD_TYPE_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "dtmfnb_payload_type_int_array";
// }

// Bundle {
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE[] =
        KEY_IMS_VOICE_PREFIX "amrnb_payload_description_bundle";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE[] =
        KEY_IMS_VOICE_PREFIX "amrwb_payload_description_bundle";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT[] =
        KEY_IMS_VOICE_PREFIX "amr_codec_attribute_payload_format_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "amr_codec_attribute_modeset_int_array";
// }
// Bundle {
const IMS_CHAR CarrierConfig::ImsVoice::KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE[] =
        KEY_IMS_VOICE_PREFIX "evs_payload_description_bundle";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT[] =
        KEY_IMS_VOICE_PREFIX "evs_codec_attribute_mode_switch_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT[] =
        KEY_IMS_VOICE_PREFIX "evs_codec_attribute_bandwidth_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "evs_codec_attribute_bitrate_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT[] =
        KEY_IMS_VOICE_PREFIX "evs_codec_attribute_ch_aw_recv_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT[] =
        KEY_IMS_VOICE_PREFIX "evs_codec_attribute_hf_only_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_DTX_BOOL[] =
        KEY_IMS_VOICE_PREFIX "evs_codec_attribute_dtx_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL[] =
        KEY_IMS_VOICE_PREFIX "evs_codec_attribute_dtx_recv_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CHANNELS_INT[] =
        KEY_IMS_VOICE_PREFIX "evs_codec_attribute_channels_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CMR_INT[] =
        KEY_IMS_VOICE_PREFIX "codec_attribute_cmr_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT[] =
        KEY_IMS_VOICE_PREFIX "codec_attribute_mode_change_period_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT[] =
        KEY_IMS_VOICE_PREFIX "codec_attribute_mode_change_capability_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT[] =
        KEY_IMS_VOICE_PREFIX "codec_attribute_mode_change_neighbor_int";
// }
// Public carrier-config - ends

// Aos
const IMS_CHAR CarrierConfig::ImsVoice::KEY_IGNORE_VOPS_FOR_VOLTE_ENABLE_BOOL[] =
        KEY_IMS_VOICE_PREFIX "ignore_vops_for_volte_enable_bool";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_PLMN_BLOCK_WITH_TIMEOUT_ON_VOICE_CALL_UNAVAILABLE_BOOL[] =
                KEY_IMS_VOICE_PREFIX "plmn_block_with_timeout_on_voice_call_unavailable_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_REQUIRED_VOLTE_BLOCK_BY_SSAC_BOOL[] =
        KEY_IMS_VOICE_PREFIX "required_volte_block_by_ssac_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_VOLTE_HYS_TIME_SEC_INT[] =
        KEY_IMS_VOICE_PREFIX "volte_hys_time_sec_int";

// Mtc
const IMS_CHAR CarrierConfig::ImsVoice::KEY_18X_TIMER_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "18x_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SUPPORT_CONFERENCE_REFER_SUBSCRIBE_BOOL[] =
        KEY_IMS_VOICE_PREFIX "support_conference_refer_subscribe_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT_BOOL[] =
        KEY_IMS_VOICE_PREFIX "enable_conference_subscribe_by_participant_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CONFERENCE_SIP_FLOW_ORDER_INT[] =
        KEY_IMS_VOICE_PREFIX "conference_sip_flow_order_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT[] =
        KEY_IMS_VOICE_PREFIX "conference_inviting_refer_type_int";
const IMS_CHAR CarrierConfig::ImsVoice::
        KEY_POLICY_QOS_PRECONDITION_MECHANISM_WHILE_CALL_MODIFICATION_INT[] = KEY_IMS_VOICE_PREFIX
        "policy_qos_precondition_mechanism_while_call_modification_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_INCOMING_CALL_REJECT_CODE_FOR_USER_DECLINE_INT[] =
        KEY_IMS_VOICE_PREFIX "incoming_call_reject_code_for_user_decline_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_INCOMING_CALL_REJECT_CODE_FOR_NO_ANSWER_INT[] =
        KEY_IMS_VOICE_PREFIX "incoming_call_reject_code_for_no_answer_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_PRACK_WAIT_TIMER_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "prack_wait_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_PRACK_UPDATE_RESPONSE_WAIT_TIMER_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "prack_update_response_wait_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SESSION_REFRESH_TRIGGER_INTERVAL_SEC_INT[] =
        KEY_IMS_VOICE_PREFIX "session_refresh_trigger_interval_sec_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_REGISTRATION_RESTORATION_MODE_ON_504_FOR_INVITE_INT[] =
        KEY_IMS_VOICE_PREFIX "registration_restoration_mode_on_504_for_invite_int";
const IMS_CHAR CarrierConfig::ImsVoice::
        KEY_REGISTRATION_RESTORATION_FOR_INVITE_REQUIRE_HEADER_VALIDATION_BOOL[] =
                KEY_IMS_VOICE_PREFIX
        "registration_restoration_for_invite_require_header_validation_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_POLICY_ON_AUDIO_QOS_DEACTIVATION_INT[] =
        KEY_IMS_VOICE_PREFIX "policy_on_audio_qos_deactivation_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_ENABLE_SEND_REINVITE_ON_RAT_CHANGE_BOOL[] =
        KEY_IMS_VOICE_PREFIX "enable_send_reinvite_on_rat_change_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_ALLOW_SDP_IN_PRACK_BOOL[] =
        KEY_IMS_VOICE_PREFIX "allow_sdp_in_prack_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_INT[] =
        KEY_IMS_VOICE_PREFIX "policy_for_media_type_restriction_on_cellular_int";
const IMS_CHAR CarrierConfig::ImsVoice::
        KEY_POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_IN_ROAMING_INT[] =
                KEY_IMS_VOICE_PREFIX "policy_for_media_type_restriction_on_cellular_in_roaming_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_POLICY_OF_LOCAL_NUMBERS_INT[] =
        KEY_IMS_VOICE_PREFIX "policy_of_local_numbers_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "silent_redial_max_duration_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "silent_redial_interval_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT[] =
        KEY_IMS_VOICE_PREFIX "silent_redial_max_retry_count_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SILENT_REDIAL_ULTIMATE_FAILURE_ACTION_INT[] =
        KEY_IMS_VOICE_PREFIX "silent_redial_ultimate_failure_action_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_TYPE_AFTER_AUDIO_AND_VIDEO_CALL_MERGED_INT[] =
        KEY_IMS_VOICE_PREFIX "call_type_after_audio_and_video_call_merged_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALLER_ID_SERVICE_CODES_FOR_RESTRICTION_STRING_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "caller_id_service_codes_for_restriction_string_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALLER_ID_SERVICE_CODES_FOR_IDENTITY_STRING_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "caller_id_service_codes_for_identity_string_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SHORT_CALL_CODE_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "short_call_code_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_LOCAL_NUMBER_PRESENTATION_SET_STRING[] =
        KEY_IMS_VOICE_PREFIX "local_number_presentation_set_string";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL_BOOL[] =
        KEY_IMS_VOICE_PREFIX "allow_multiple_call_including_video_call_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_REJECT_CODE_AND_REASON_CODE_SET_STRING_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "reject_code_and_reason_code_set_string_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "reject_code_and_action_set_string_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "update_reject_code_and_action_set_string_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "csfb_block_condition_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_POLICY_FOR_403_RESPONSE_FOR_INVITE_INT[] =
        KEY_IMS_VOICE_PREFIX "policy_for_403_response_for_invite_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT[] =
        KEY_IMS_VOICE_PREFIX "policy_for_checking_qos_while_call_upgrading_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL[] =
        KEY_IMS_VOICE_PREFIX "reject_offerless_invite_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_MAX_COUNT_INT[] =
        KEY_IMS_VOICE_PREFIX "call_max_count_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING[] =
        KEY_IMS_VOICE_PREFIX "call_terminate_reason_header_user_ends_call_string";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_RTP_TIMEOUT_STRING[] =
        KEY_IMS_VOICE_PREFIX "call_terminate_reason_header_rtp_timeout_string";
const IMS_CHAR CarrierConfig::ImsVoice::
        KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_AND_RTP_TIMEOUT_STRING[] = KEY_IMS_VOICE_PREFIX
        "call_terminate_reason_header_user_ends_and_rtp_timeout_string";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_MEDIA_BEARER_LOSS_STRING[] =
                KEY_IMS_VOICE_PREFIX "call_terminate_reason_header_media_bearer_loss_string";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_SIP_TIMEOUT_STRING[] =
        KEY_IMS_VOICE_PREFIX "call_terminate_reason_header_sip_timeout_string";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_SIP_RESPONSE_TIMEOUT_STRING[] =
                KEY_IMS_VOICE_PREFIX "call_terminate_reason_header_sip_response_timeout_string";
const IMS_CHAR CarrierConfig::ImsVoice::
        KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_AND_SIP_RESPONSE_TIMEOUT_STRING[] =
                KEY_IMS_VOICE_PREFIX
        "call_terminate_reason_header_user_ends_and_sip_response_timeout_string";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_CALL_SETUP_TIMEOUT_STRING[] =
                KEY_IMS_VOICE_PREFIX "call_terminate_reason_header_call_setup_timeout_string";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_TERMINATING_EARLYDIALOG_STRING[] =
                KEY_IMS_VOICE_PREFIX "call_terminate_reason_header_terminating_earlydialog_string";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_SESSION_REFRESH_FAILURE_STRING[] =
                KEY_IMS_VOICE_PREFIX "call_terminate_reason_header_session_refresh_failure_string";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_CONFERENCE_CALL_JOINED_STRING[] =
                KEY_IMS_VOICE_PREFIX "call_terminate_reason_header_conference_call_joined_string";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_CSCALL_STRING[] =
        KEY_IMS_VOICE_PREFIX "call_reject_reason_phrase_on_cscall_string";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_VILTE_AND_NO_LTE_STRING[] =
        KEY_IMS_VOICE_PREFIX "call_reject_reason_phrase_on_vilte_and_no_lte_string";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_CONNECTING_CALL_STRING[] =
        KEY_IMS_VOICE_PREFIX "call_reject_reason_phrase_on_connecting_call_string";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_EXCEEDS_MAX_CALL_COUNT_STRING[] =
                KEY_IMS_VOICE_PREFIX "call_reject_reason_phrase_exceeds_max_call_count_string";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_CONVERTING_STRING[] =
        KEY_IMS_VOICE_PREFIX "call_reject_reason_phrase_on_converting_string";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_NEGOTIATION_FAILURE_STRING[] =
        KEY_IMS_VOICE_PREFIX "call_reject_reason_phrase_negotiation_failure_string";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_NO_ANSWER_BY_USER_STRING[] =
        KEY_IMS_VOICE_PREFIX "call_reject_reason_phrase_no_answer_by_user_string";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_VOWIFI_OFF_STRING[] =
        KEY_IMS_VOICE_PREFIX "call_reject_reason_phrase_vowifi_off_string";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_USER_REJECT_STRING[] =
        KEY_IMS_VOICE_PREFIX "call_reject_reason_phrase_user_reject_string";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_ACCESS_CLASS_BLOCKED_STRING[] =
                KEY_IMS_VOICE_PREFIX "call_reject_reason_phrase_access_class_blocked_string";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_VOPS_OFF_STRING[] =
        KEY_IMS_VOICE_PREFIX "call_reject_reason_phrase_vops_off_string";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_REGISTRATION_TO_18X_TIMER_MILLIS_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "registration_to_18x_timer_millis_int_array";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_QOS_ACQUISITION_AFTER_W2L_HANDOVER_WAIT_TIMER_MILLIS_INT[] =
                KEY_IMS_VOICE_PREFIX "qos_acquisition_after_w2l_handover_wait_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::
        KEY_WAIT_VIDEO_TEXT_QOS_AFTER_AUDIO_QOS_ACQUISITION_TIMER_MILLIS_INT[] =
                KEY_IMS_VOICE_PREFIX
        "wait_video_text_qos_after_audio_qos_acquisition_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_QOS_LOST_GUARD_TIMER_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "qos_lost_guard_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_QOS_FORCED_ACQUISITION_TIMER_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "qos_forced_acquisition_timer_millis_int";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_RAT_CONDITION_FOR_NOT_WAITING_DEDICATED_BEARER_INT_ARRAY[] =
                KEY_IMS_VOICE_PREFIX "rat_condition_for_not_waiting_dedicated_bearer_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::
        KEY_TRIGGER_DEDICATED_BEARER_WAIT_TIMER_BY_SENDING_INITIAL_INVITE_BOOL[] =
                KEY_IMS_VOICE_PREFIX
        "trigger_dedicated_bearer_wait_timer_by_sending_initial_invite_bool";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_RESTART_DEDICATED_BEARER_WAIT_TIMER_BY_EPS_FALLBACK_BOOL[] =
                KEY_IMS_VOICE_PREFIX "restart_dedicated_bearer_wait_timer_by_eps_fallback_bool";
// Media
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_JITTER_BUFFER_SIZE_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "audio_jitter_buffer_size_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_RTCP_INTERVAL_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "audio_rtcp_interval_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_EVS_AMRWB_IO_MODE_SET_INT[] =
        KEY_IMS_VOICE_PREFIX "evs_amrwb_io_mode_set_int";

//// ImsVt
#define KEY_IMS_VT_PREFIX "imsvt."
// Public carrier-config - starts
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_ON_DEFAULT_BEARER_SUPPORTED_BOOL[] =
        KEY_IMS_VT_PREFIX "video_on_default_bearer_supported_bool";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_RTP_INACTIVITY_TIMER_MILLIS_INT[] =
        KEY_IMS_VT_PREFIX "video_rtp_inactivity_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_RTCP_INACTIVITY_TIMER_MILLIS_INT[] =
        KEY_IMS_VT_PREFIX "video_rtcp_inactivity_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_AS_BANDWIDTH_KBPS_INT[] =
        KEY_IMS_VT_PREFIX "video_as_bandwidth_kbps_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_RS_BANDWIDTH_BPS_INT[] =
        KEY_IMS_VT_PREFIX "video_rs_bandwidth_bps_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_RR_BANDWIDTH_BPS_INT[] =
        KEY_IMS_VT_PREFIX "video_rr_bandwidth_bps_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_RTP_DSCP_INT[] =
        KEY_IMS_VT_PREFIX "video_rtp_dscp_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_QOS_PRECONDITION_SUPPORTED_BOOL[] =
        KEY_IMS_VT_PREFIX "video_qos_precondition_supported_bool";
// Bundle {
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE[] =
        KEY_IMS_VT_PREFIX "video_codec_capability_payload_types_bundle";
const IMS_CHAR CarrierConfig::ImsVt::KEY_H264_PAYLOAD_TYPE_INT_ARRAY[] =
        KEY_IMS_VT_PREFIX "h264_payload_type_int_array";
// }
// Bundle {
const IMS_CHAR CarrierConfig::ImsVt::KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE[] =
        KEY_IMS_VT_PREFIX "h264_payload_description_bundle";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT[] =
        KEY_IMS_VT_PREFIX "video_codec_attribute_packetization_mode_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT[] =
        KEY_IMS_VT_PREFIX "video_codec_attribute_frame_rate_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY[] =
        KEY_IMS_VT_PREFIX "video_codec_attribute_resolution_int_array";
const IMS_CHAR CarrierConfig::ImsVt::KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING[] =
        KEY_IMS_VT_PREFIX "h264_video_codec_attribute_profile_level_id_string";
// }
// Public carrier-config - ends
const IMS_CHAR CarrierConfig::ImsVt::KEY_CONVERT_REMOTE_RESPONSE_TIMER_MILLIS_INT[] =
        KEY_IMS_VT_PREFIX "convert_remote_response_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_CONVERT_USER_RESPONSE_TIMER_MILLIS_INT[] =
        KEY_IMS_VT_PREFIX "convert_user_response_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_POLICY_ON_VIDEO_QOS_DEACTIVATION_INT[] =
        KEY_IMS_VT_PREFIX "policy_on_video_qos_deactivation_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_SUPPORT_EARLY_SESSION_BOOL[] =
        KEY_IMS_VT_PREFIX "support_early_session_bool";
const IMS_CHAR CarrierConfig::ImsVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT[] =
        KEY_IMS_VT_PREFIX "policy_for_text_with_video_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_MINIMUM_BATTERY_LEVEL_FOR_LIMIT_VIDEO_CALL_INT[] =
        KEY_IMS_VT_PREFIX "minimum_battery_level_for_limit_video_call_int";
// Media
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_RTCP_INTERVAL_INT_ARRAY[] =
        KEY_IMS_VT_PREFIX "video_rtcp_interval_int_array";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_AVPF_FEATURE_INT[] =
        KEY_IMS_VT_PREFIX "video_avpf_feature_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_CODEC_BITRATE_INT_ARRAY[] =
        KEY_IMS_VT_PREFIX "video_codec_bitrate_int_array";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_CODEC_IMAGE_ATTR_STRING_ARRAY[] =
        KEY_IMS_VT_PREFIX "video_codec_image_attr_string_array";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_CODEC_FRAME_SIZE_STRING_ARRAY[] =
        KEY_IMS_VT_PREFIX "video_codec_frame_size_string_array";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_CODEC_HEVC_PROFILE_INT_ARRAY[] =
        KEY_IMS_VT_PREFIX "video_codec_hevc_profile_int_array";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_CODEC_HEVC_LEVEL_INT_ARRAY[] =
        KEY_IMS_VT_PREFIX "video_codec_hevc_level_int_array";

//// ImsWfc
#define KEY_IMS_WFC_PREFIX "imswfc."
// Public carrier-config - starts
const IMS_CHAR CarrierConfig::ImsWfc::KEY_PIDF_SHORT_CODE_STRING_ARRAY[] =
        KEY_IMS_WFC_PREFIX "pidf_short_code_string_array";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_BOOL[] =
        KEY_IMS_WFC_PREFIX "emergency_call_over_emergency_pdn_bool";
// Public carrier-config - ends

// Aos
const IMS_CHAR CarrierConfig::ImsWfc::KEY_REGISTRATION_PRIVATE_HEADER_INT[] =
        KEY_IMS_WFC_PREFIX "registration_private_header_int";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_COUNTRY_CODE_INT[] =
        KEY_IMS_WFC_PREFIX "country_code_int";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_REREG_WITH_CHANGED_COUNTRY_ON_WIFI_BOOL[] =
        KEY_IMS_WFC_PREFIX "rereg_with_changed_country_on_wifi_bool";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_REQUIRED_WFC_BLOCK_BY_AIRPLANE_MODE_BOOL[] =
        KEY_IMS_WFC_PREFIX "required_wfc_block_by_airplane_mode_bool";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_USE_WFC_COUNTRY_CODE_AVAILABILITY_CHECK_BOOL[] =
        KEY_IMS_WFC_PREFIX "use_wfc_country_code_availability_check_bool";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_VIDEO_OVER_WIFI_SUPPORTED_WITHOUT_VOICE_BOOL[] =
        KEY_IMS_WFC_PREFIX "video_over_wifi_supported_without_voice_bool";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_VOWIFI_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY[] =
        KEY_IMS_WFC_PREFIX "vowifi_sub_err_code_for_init_reg_int_array";
// Bundle {
const IMS_CHAR CarrierConfig::ImsWfc::KEY_WFC_ERR_MESSAGE_BUNDLE[] =
        KEY_IMS_WFC_PREFIX "wfc_err_message_bundle";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_WFC_ERR_REG_403_STRING[] =
        KEY_IMS_WFC_PREFIX "wfc_err_reg_403_string";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_WFC_ERR_REG_500_STRING[] =
        KEY_IMS_WFC_PREFIX "wfc_err_reg_500_string";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_WFC_ERR_NOT_SUPPORTED_COUNTRY_STRING[] =
        KEY_IMS_WFC_PREFIX "wfc_err_not_supported_country_string";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_WFC_ERR_SUB_403_STRING[] =
        KEY_IMS_WFC_PREFIX "wfc_err_sub_403_string";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_WFC_ERR_NOTIFY_TERMINATED_STRING[] =
        KEY_IMS_WFC_PREFIX "wfc_err_notify_terminated_string";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_WFC_ERR_OTHER_FAILURES_STRING[] =
        KEY_IMS_WFC_PREFIX "wfc_err_other_failures_string";
// }

//// ImsSs
#define KEY_IMS_SS_PREFIX "imsss."
// Public carrier-config - starts
const IMS_CHAR CarrierConfig::ImsSs::KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY[] =
        KEY_IMS_SS_PREFIX "ut_terminal_based_services_int_array";
const IMS_CHAR CarrierConfig::ImsSs::KEY_NETWORK_INITIATED_USSD_OVER_IMS_SUPPORTED_BOOL[] =
        KEY_IMS_SS_PREFIX "network_initiated_ussd_over_ims_supported_bool";
// Public carrier-config - ends

//// Assets
// Mtc
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CHECK_CONFERENCE_EVENT_PACKAGE_VERSION_BOOL[] =
        KEY_IMS_VOICE_PREFIX "check_conference_event_package_version_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CONFERENCE_REFER_TO_URI_SOURCE_PAID_BOOL[] =
        KEY_IMS_VOICE_PREFIX "conference_refer_to_uri_source_paid_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CONFERENCE_DROP_REFER_TO_URI_SOURCE_TYPE_INT[] =
        KEY_IMS_VOICE_PREFIX "conference_drop_refer_to_uri_source_type_int";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_ENABLE_FAKE_QOS_CALL_FLOW_ON_WIFI_BOOL[] =
        KEY_IMS_WFC_PREFIX "enable_fake_qos_call_flow_on_wifi_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_INVITE_INT[] =
        KEY_IMS_VOICE_PREFIX "media_type_for_offerless_invite_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_REINVITE_INT[] =
        KEY_IMS_VOICE_PREFIX "media_type_for_offerless_reinvite_int";
const IMS_CHAR
        CarrierConfig::ImsVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL[] =
                KEY_IMS_VT_PREFIX "support_video_call_upgrade_regardless_of_feature_tags_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_OIP_TYPE_FOR_UNAVAILABLE_INT[] =
        KEY_IMS_VOICE_PREFIX "oip_type_for_unavailable_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_ENABLE_OIP_HEADER_POLICY_FALLBACK_BOOL[] =
        KEY_IMS_VOICE_PREFIX "enable_oip_header_policy_fallback_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_DELAY_UPDATE_AFTER_CONNECTED_TIMER_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "delay_update_after_connected_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_EMERGENCY_RTT_GUARD_TIMER_MILLIS_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "emergency_rtt_guard_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsEmergency::
        KEY_RETRY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_WITH_NEXT_PCSCF_BOOL[] =
                KEY_IMS_EMERGENCY_PREFIX
        "retry_emergency_call_over_emergency_pdn_with_next_pcscf_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_PREALERTING_TIMER_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "prealerting_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL_INT[] =
        KEY_IMS_VOICE_PREFIX "policy_for_tcall_timer_expiry_of_volte_call_int";
const IMS_CHAR CarrierConfig::ImsEmergency::
        KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_EMERGENCY_CALL_INT[] = KEY_IMS_EMERGENCY_PREFIX
        "policy_for_tcall_timer_expiry_of_volte_emergency_call_int";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL_INT[] =
        KEY_IMS_WFC_PREFIX "policy_for_tcall_timer_expiry_of_vowifi_call_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "carrier_specific_sip_headers_string_array";
const IMS_CHAR
        CarrierConfig::ImsVt::KEY_CHECK_AVCHANGE_FEATURE_FOR_CALL_CONVERTING_CAPABILITY_BOOL[] =
                KEY_IMS_VT_PREFIX "check_avchange_feature_for_call_converting_capability_bool";
const IMS_CHAR CarrierConfig::ImsVoice::
        KEY_SUPPORT_REGISTRATION_RECOVERY_FOR_FAILURE_OF_SESSION_REFRESH_BOOL[] =
                KEY_IMS_VOICE_PREFIX
        "support_registration_recovery_for_failure_of_session_refresh_bool";
const IMS_CHAR CarrierConfig::ImsVoice::
        KEY_POLICY_FOR_CALL_MAINTAINING_ON_REGISTRATION_SUSPENDED_INT_ARRAY[] = KEY_IMS_VOICE_PREFIX
        "policy_for_call_maintaining_on_registration_suspended_int_array";
const IMS_CHAR CarrierConfig::ImsEmergency::
        KEY_POLICY_FOR_REQUIRING_EMERGENCY_CALL_WHEN_VIDEO_EMERGENCY_CALL_FAILED_INT_ARRAY[] =
                KEY_IMS_EMERGENCY_PREFIX
        "policy_for_requiring_emergency_call_when_video_emergency_call_failed_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_USE_MCID_SUPPLEMENTARY_SERVICE_BOOL[] =
        KEY_IMS_VOICE_PREFIX "use_mcid_supplementary_service_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_USE_MMC_SUPPLEMENTARY_SERVICE_BOOL[] =
        KEY_IMS_VOICE_PREFIX "use_mmc_supplementary_service_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_USE_LTE_PREFERRED_STATUS_FOR_SERVICE_CAPABILITY_BOOL[] =
        KEY_IMS_VOICE_PREFIX "use_lte_preferred_status_for_service_capability_bool";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_ALLOW_INCOMING_HOLD_REQUEST_DURING_CONFERENCE_CALL_BOOL[] =
                KEY_IMS_VOICE_PREFIX "allow_incoming_hold_request_during_conference_call_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_IGNORE_180_AFTER_183_RESPONSE_BOOL[] =
        KEY_IMS_VOICE_PREFIX "ignore_180_after_183_response_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_ADD_REPLACE_HEADER_FOR_CONFERENCE_BOOL[] =
        KEY_IMS_VOICE_PREFIX "add_replace_header_for_conference_bool";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VILTE_TO_VOLTE_RETRY_FAILURE_RESPONSE_CODE_INT_ARRAY[] =
        KEY_IMS_VT_PREFIX "vilte_to_volte_retry_failure_response_code_int_array";
const IMS_CHAR
        CarrierConfig::ImsEmergency::KEY_USE_EMERGENCY_NUMBER_TRANSLATION_IN_ROAMING_STATUS_BOOL[] =
                KEY_IMS_EMERGENCY_PREFIX "use_emergency_number_translation_in_roaming_status_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_IGNORE_PRACK_DELIVERY_FAILURE_BOOL[] =
        KEY_IMS_VOICE_PREFIX "ignore_prack_delivery_failure_bool";
const IMS_CHAR CarrierConfig::ImsVt::KEY_SUPPORT_VIDEO_CALL_ONLY_IN_VOPS_OFF_STATUS_BOOL[] =
        KEY_IMS_VT_PREFIX "support_video_call_only_in_vops_off_status_bool";
const IMS_CHAR
        CarrierConfig::ImsEmergency::KEY_BLOCK_WIFI_EMERGENCY_CALL_IF_NOT_PROVISIONED_BOOL[] =
                KEY_IMS_EMERGENCY_PREFIX "block_wifi_emergency_call_if_not_provisioned_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_REGISTRATION_DISCONNECT_REASON_TO_IGNORE_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "registration_disconnect_reason_to_ignore_int_array";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_WIFI_EMERGENCY_18X_TIMER_MILLIS_INT[] =
        KEY_IMS_EMERGENCY_PREFIX "wifi_emergency_18x_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::
        KEY_USE_CARRIER_SPECIFIC_CONTACT_HEADER_FOR_OPTIONS_RESPONSE_BOOL[] = KEY_IMS_VOICE_PREFIX
        "use_carrier_specific_contact_header_for_options_response_bool";
const IMS_CHAR CarrierConfig::ImsVoice::
        KEY_USE_CARRIER_SPECIFIC_REJECT_PHRASE_FOR_INCOMING_CALL_DURING_NO_REGISTRATION_BOOL[] =
                KEY_IMS_VOICE_PREFIX
        "use_carrier_specific_reject_phrase_for_incoming_call_during_no_registration_bool";
const IMS_CHAR CarrierConfig::ImsVoice::
        KEY_ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_REJECTED_BY_SERVER_ERROR_BOOL[] =
                KEY_IMS_VOICE_PREFIX
        "enable_registration_recovery_when_call_rejected_by_server_error_bool";
const IMS_CHAR CarrierConfig::ImsVoice::
        KEY_ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_RETRY_UNAVAILABLE_BOOL[] = KEY_IMS_VOICE_PREFIX
        "enable_registration_recovery_when_call_retry_unavailable_bool";
const IMS_CHAR CarrierConfig::ImsWfc::KEY_REJECT_VOWIFI_VOICE_CALL_WHEN_VOWIFI_SETTING_OFF_BOOL[] =
        KEY_IMS_WFC_PREFIX "reject_vowifi_voice_call_when_vowifi_setting_off_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CHECK_SERVER_OUTAGE_REASON_FOR_VXLTE_CALL_BOOL[] =
        KEY_IMS_VOICE_PREFIX "check_server_outage_reason_for_vxlte_call_bool";
const IMS_CHAR CarrierConfig::ImsVt::
        KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL[] =
                KEY_IMS_VT_PREFIX
        "set_video_text_feature_exclusively_in_contact_header_by_session_type_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING_BOOL[] =
        KEY_IMS_VOICE_PREFIX "maintain_multiple_early_sessions_by_forking_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY_BOOL[] =
        KEY_IMS_VOICE_PREFIX "stop_ringback_timer_by_183_with_sdp_body_bool";
const IMS_CHAR CarrierConfig::Ims::KEY_INFORMATION_LEVEL_OF_GEOLOCATION_PIDF_INT_ARRAY[] =
        KEY_IMS_PREFIX "information_level_of_geolocation_pidf_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "message_type_support_geolocation_pidf_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_GEOLOCATION_BLOCK_CONDITION_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "geolocation_block_condition_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_INCOMING_RESUME_EVENT_SUPPORT_BOOL[] =
        KEY_IMS_VOICE_PREFIX "incoming_resume_event_support_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT[] =
        KEY_IMS_VOICE_PREFIX "sip_status_code_for_rejecting_call_type_change_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_INITIALIZE_P_EARLY_MEDIA_WHEN_NO_HEADER_BOOL[] =
        KEY_IMS_VOICE_PREFIX "initialize_p_early_media_when_no_header_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT[] =
        KEY_IMS_VOICE_PREFIX "policy_for_local_ringback_tone_with_180_response_int";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_USER_CANCEL_REASON_AFTER_RESPONSE_TIMEOUT_TIMER_MILLIS_INT[] =
                KEY_IMS_VOICE_PREFIX "user_cancel_reason_after_response_timeout_timer_millis_int";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT[] =
                KEY_IMS_VOICE_PREFIX "mo_call_request_timeout_for_eps_fallback_trigger_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "eps_fallback_watchdog_time_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_EPS_FALLBACK_TRIGGER_BY_RRC_REJECT_WAIT_TIME_BOOL[] =
        KEY_IMS_VOICE_PREFIX "eps_fallback_trigger_by_rrc_reject_wait_time_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "send_udp_keep_alive_interval_time_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CALL_REJECT_CODE_FOR_NOT_ACCEPTABLE_CALL_TYPE_INT[] =
        KEY_IMS_VOICE_PREFIX "call_reject_code_for_not_acceptable_call_type_int";
const IMS_CHAR
        CarrierConfig::ImsEmergency::KEY_RELEASE_EMERGENCY_PDN_WITH_EMERGENCY_CALL_FAIL_BOOL[] =
                KEY_IMS_EMERGENCY_PREFIX "release_emergency_pdn_with_emergency_call_fail_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::
        KEY_EMERGENCY_CALL_CURRENT_LOCATION_DISCOVERY_SUPPORTED_BOOL[] =
                KEY_IMS_EMERGENCY_PREFIX "emergency_call_current_location_discovery_supported_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_CHECK_UI_CONDITION_FOR_INCOMING_RESUME_BOOL[] =
        KEY_IMS_VOICE_PREFIX "check_ui_condition_for_incoming_resume_bool";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_P_EMERGENCY_INFO_HEADER_IN_INVITE_STRING[] =
        KEY_IMS_EMERGENCY_PREFIX "p_emergency_info_header_in_invite_string";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_CONTACT_HEADER_ADDRESS_IN_INVITE_STRING_ARRAY[] =
        KEY_IMS_EMERGENCY_PREFIX "contact_header_address_in_invite_string_array";
const IMS_CHAR
        CarrierConfig::ImsEmergency::KEY_P_PREFERRED_IDENTITY_INFO_HEADER_IN_INVITE_STRING_ARRAY[] =
                KEY_IMS_EMERGENCY_PREFIX "p_preferred_identity_header_in_invite_string_array";
const IMS_CHAR
        CarrierConfig::ImsEmergency::KEY_REJECT_CODE_REQUIRE_IMMEDIATE_TERMINATION_STRING_ARRAY[] =
                KEY_IMS_EMERGENCY_PREFIX "reject_code_require_immediate_termination_string_array";
const IMS_CHAR CarrierConfig::ImsEmergency::
        KEY_REJECT_CODE_AND_REASON_REQUIRE_IMMEDIATE_TERMINATION_STRING_ARRAY[] =
                KEY_IMS_EMERGENCY_PREFIX
        "reject_code_and_reason_require_immediate_termination_string_array";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_REJECT_CODE_REQUIRE_TEMP_FAILURE_INT_ARRAY[] =
        KEY_IMS_EMERGENCY_PREFIX "reject_code_require_temp_failure_int_array";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_REJECT_CODE_REQUIRE_PERM_FAILURE_INT_ARRAY[] =
        KEY_IMS_EMERGENCY_PREFIX "reject_code_require_perm_failure_int_array";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_POLICY_FOR_EMERGENCY_URN_INT_ARRAY[] =
        KEY_IMS_EMERGENCY_PREFIX "policy_for_emergency_urn_int_array";
const IMS_CHAR CarrierConfig::ImsEmergency::KEY_SILENT_REDIAL_WITH_VOIP_BY_RTT_REJECTION_BOOL[] =
        KEY_IMS_EMERGENCY_PREFIX "silent_redial_with_voip_by_rtt_rejection_bool";

// Media
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_RTP_PORT_RANGE_INT_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "audio_rtp_port_range_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_BW_NEGO_OPTION_BOOL[] =
        KEY_IMS_VOICE_PREFIX "audio_bw_nego_option_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_PTIME_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "audio_ptime_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_MAXPTIME_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "audio_maxptime_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_MAXRED_INT[] =
        KEY_IMS_VOICE_PREFIX "audio_maxred_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_RTP_DSCP_INT[] =
        KEY_IMS_VOICE_PREFIX "audio_rtp_dscp_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_MODESET_BOOL[] =
        KEY_IMS_VOICE_PREFIX "audio_show_codec_attribute_modeset_bool";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_AUDIO_AMRWB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY[] =
                KEY_IMS_VOICE_PREFIX "audio_amrwb_codec_attribute_default_modeset_int_array";
const IMS_CHAR
        CarrierConfig::ImsVoice::KEY_AUDIO_AMRNB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY[] =
                KEY_IMS_VOICE_PREFIX "audio_amrnb_codec_attribute_default_modeset_int_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_DTX_BOOL[] =
        KEY_IMS_VOICE_PREFIX "audio_show_codec_attribute_dtx_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_AMRWBIO_MODESET_BOOL[] =
        KEY_IMS_VOICE_PREFIX "audio_show_codec_attribute_amrwbio_modeset_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_TELEPHONE_EVENT_DURATION_MILLIS_INT[] =
        KEY_IMS_VOICE_PREFIX "audio_telephone_event_duration_millis_int";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_CANDIDATE_ATTRIBUTE_STRING_ARRAY[] =
        KEY_IMS_VOICE_PREFIX "audio_candidate_attribute_string_array";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_ENABLE_BOOL[] =
        KEY_IMS_VOICE_PREFIX "audio_rtcpxr_enable_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_STATISTICS_BOOL[] =
        KEY_IMS_VOICE_PREFIX "audio_rtcpxr_statistics_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_VOIP_METRICS_BOOL[] =
        KEY_IMS_VOICE_PREFIX "audio_rtcpxr_voip_metrics_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_PACKET_LOSS_RLE_BOOL[] =
        KEY_IMS_VOICE_PREFIX "audio_rtcpxr_packet_loss_rle_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_PACKET_DUPLICATE_RLE_BOOL[] =
        KEY_IMS_VOICE_PREFIX "audio_rtcpxr_packet_duplicate_rle_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SUPPORT_MULTI_CONFIG_IN_EARLY_SESSION_BOOL[] =
        KEY_IMS_VOICE_PREFIX "support_multi_config_in_early_session_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_AUDIO_EVS_SUPPORT_BOOL[] =
        KEY_IMS_VOICE_PREFIX "audio_evs_support_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_MEDIA_ANBR_CAPABILITY_IN_MODEM_BOOL[] =
        KEY_IMS_VOICE_PREFIX "media_anbr_capability_in_modem_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_MEDIA_SESSION_LEVEL_BANDWIDTH_BOOL[] =
        KEY_IMS_VOICE_PREFIX "media_session_level_bandwidth_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SDP_ANSWER_FULL_CAPABILITY_BOOL[] =
        KEY_IMS_VOICE_PREFIX "sdp_answer_full_capability_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_SDP_REOFFER_FULL_CAPABILITY_BOOL[] =
        KEY_IMS_VOICE_PREFIX "sdp_reoffer_full_capability_bool";
const IMS_CHAR CarrierConfig::ImsVoice::KEY_INCOMING_DTMF_TONE_PLAY_SUPPORT_BOOL[] =
        KEY_IMS_VOICE_PREFIX "incoming_dtmf_tone_play_support_bool";
const IMS_CHAR CarrierConfig::ImsVt::KEY_AVC_SPROP_PARAMETER_SETS_STRING[] =
        KEY_IMS_VT_PREFIX "avc_sprop_parameter_sets_string";
const IMS_CHAR CarrierConfig::ImsVt::KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY[] =
        KEY_IMS_VT_PREFIX "hevc_payload_type_int_array";
const IMS_CHAR CarrierConfig::ImsVt::KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE[] =
        KEY_IMS_VT_PREFIX "hevc_payload_description_bundle";
const IMS_CHAR CarrierConfig::ImsVt::KEY_HEVC_SPROP_PARAMETER_SETS_STRING[] =
        KEY_IMS_VT_PREFIX "hevc_sprop_parameter_sets_string";
const IMS_CHAR CarrierConfig::ImsVt::KEY_HEVC_PROFILE_INT[] = KEY_IMS_VT_PREFIX "hevc_profile_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_HEVC_LEVEL_INT[] = KEY_IMS_VT_PREFIX "hevc_level_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_RTP_PORT_RANGE_INT_ARRAY[] =
        KEY_IMS_VT_PREFIX "video_rtp_port_range_int_array";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_CVO_VALUE_INT[] =
        KEY_IMS_VT_PREFIX "video_cvo_value_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_SDP_OFFER_CAP_NEGO_FOR_AVPF_INT[] =
        KEY_IMS_VT_PREFIX "video_sdp_offer_cap_nego_for_avpf_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_AVPF_ENABLE_BOOL[] =
        KEY_IMS_VT_PREFIX "video_avpf_enable_bool";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_IFRAME_INTERVAL_SEC_INT[] =
        KEY_IMS_VT_PREFIX "video_iframe_interval_sec_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_SEND_PERIODIC_SPS_PPS_INT[] =
        KEY_IMS_VT_PREFIX "video_send_periodic_sps_pps_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_BW_NEGO_OPTION_BOOL[] =
        KEY_IMS_VT_PREFIX "video_bw_nego_option_bool";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_LOWEST_BITRATE_BPS_INT[] =
        KEY_IMS_VT_PREFIX "video_lowest_bitrate_bps_int";
const IMS_CHAR CarrierConfig::ImsVt::KEY_VIDEO_CODEC_HEVC_PRIORITY_ORDER_BOOL[] =
        KEY_IMS_VT_PREFIX "video_codec_hevc_priority_order_bool";

// Mts
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_REQUEST_URI_TYPE_INT[] =
        KEY_IMS_SMS_PREFIX "sms_request_uri_type_int";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_USE_DIALED_NUMBER_FOR_REQUEST_URI_BOOL[] =
        KEY_IMS_SMS_PREFIX "sms_use_dialed_number_for_request_uri_bool";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_ALLOW_IMSI_BASED_SIP_URI_BOOL[] =
        KEY_IMS_SMS_PREFIX "sms_allow_imsi_based_sip_uri_bool";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY[] =
        KEY_IMS_SMS_PREFIX "sms_generic_error_codes_int_array";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_EXPIRY_TIMER_F_INT[] =
        KEY_IMS_SMS_PREFIX "sms_reg_policy_for_expiry_timer_f_int";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_403_RESPONSE_INT[] =
        KEY_IMS_SMS_PREFIX "sms_reg_policy_for_403_response_int";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_404_RESPONSE_INT[] =
        KEY_IMS_SMS_PREFIX "sms_reg_policy_for_404_response_int";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_406_RESPONSE_INT[] =
        KEY_IMS_SMS_PREFIX "sms_reg_policy_for_406_response_int";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_408_RESPONSE_INT[] =
        KEY_IMS_SMS_PREFIX "sms_reg_policy_for_408_response_int";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_500_RESPONSE_INT[] =
        KEY_IMS_SMS_PREFIX "sms_reg_policy_for_500_response_int";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_503_RESPONSE_INT[] =
        KEY_IMS_SMS_PREFIX "sms_reg_policy_for_503_response_int";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_504_RESPONSE_INT[] =
        KEY_IMS_SMS_PREFIX "sms_reg_policy_for_504_response_int";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_GEOLOCATION_PIDF_FOR_EMERGENCY_BOOL[] =
        KEY_IMS_SMS_PREFIX "sms_geolocation_pidf_for_emergency_bool";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_RETRY_AFTER_MAX_TIME_SEC_INT[] =
        KEY_IMS_SMS_PREFIX "sms_retry_after_max_time_sec_int";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_RETRY_AFTER_MAX_COUNT_INT[] =
        KEY_IMS_SMS_PREFIX "sms_retry_after_max_count_int";
const IMS_CHAR
        CarrierConfig::ImsSms::KEY_SMS_REPORT_GENERIC_ERROR_WHEN_RETRY_AFTER_NOT_POSSIBLE_BOOL[] =
                KEY_IMS_SMS_PREFIX "sms_report_generic_error_when_retry_after_not_possible_bool";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_MESSAGE_RESPONSE_WAIT_TIMER_MILLIS_INT[] =
        KEY_IMS_SMS_PREFIX "sms_message_response_wait_timer_millis_int";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_RETRY_POLICY_FOR_EXPIRY_TIMER_F_INT[] =
        KEY_IMS_SMS_PREFIX "sms_retry_policy_for_expiry_timer_f_int";
const IMS_CHAR CarrierConfig::ImsSms::KEY_SMS_IN_REPLY_TO_VALIDATION_BOOL[] =
        KEY_IMS_SMS_PREFIX "sms_in_reply_to_validation_bool";

PUBLIC GLOBAL IMS_BOOL CarrierConfig::IsVoLteEnabled(IN IMS_SINT32 nSlotId)
{
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(nSlotId);
    return (piCc != IMS_NULL) ? piCc->GetBoolean(KEY_CARRIER_VOLTE_AVAILABLE_BOOL) : IMS_FALSE;
}

PUBLIC GLOBAL IMS_BOOL CarrierConfig::IsVtEnabled(IN IMS_SINT32 nSlotId)
{
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(nSlotId);
    return (piCc != IMS_NULL) ? piCc->GetBoolean(KEY_CARRIER_VT_AVAILABLE_BOOL) : IMS_FALSE;
}

PUBLIC GLOBAL IMS_BOOL CarrierConfig::IsWfcEnabled(IN IMS_SINT32 nSlotId)
{
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(nSlotId);
    return (piCc != IMS_NULL) ? piCc->GetBoolean(KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL) : IMS_FALSE;
}
