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
package com.android.imsstack.imsservice.mmtel.config.base;

import android.telephony.CarrierConfigManager;
import android.telephony.ims.ProvisioningManager;
import android.util.SparseArray;

public class ConfigUtils {
    public static final int UNKNOWN_VALUE = (-1);
    private static SparseArray<ProvisioningItem> sProvisioningItems;

    public static SparseArray<ProvisioningItem> getProvisioningItems() {
        return sProvisioningItems;
    }

    public static ProvisioningItem getProvisioningItem(int item) {
        return sProvisioningItems.get(item);
    }

    public static String getNameFromItem(int item) {
        ProvisioningItem provisioningItem = sProvisioningItems.get(item);

        return (provisioningItem != null) ? provisioningItem.getName() : null;
    }

    public static int getItemFromName(String itemName) {
        for (int i = 0; i < sProvisioningItems.size(); i++) {
            int provisioningKey = sProvisioningItems.keyAt(i);
            ProvisioningItem item = sProvisioningItems.get(provisioningKey);

            if (item.getName().equals(itemName)) {
                return provisioningKey;
            }
        }
        return UNKNOWN_VALUE;
    }

    static {
        sProvisioningItems = new SparseArray<>();

        sProvisioningItems.put(ProvisioningManager.KEY_AMR_CODEC_MODE_SET_VALUES,
                new ProvisioningItem("AMR_CODEC_MODE_SET_VALUES", String.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_AMR_WB_CODEC_MODE_SET_VALUES,
                new ProvisioningItem("AMR_WB_CODEC_MODE_SET_VALUES", String.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_SIP_SESSION_TIMER_SEC,
                new ProvisioningItem("SIP_SESSION_TIMER_SEC", Integer.class,
                CarrierConfigManager.ImsVoice.KEY_SESSION_EXPIRES_TIMER_SEC_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_MINIMUM_SIP_SESSION_EXPIRATION_TIMER_SEC,
                new ProvisioningItem("MINIMUM_SIP_SESSION_EXPIRATION_TIMER_SEC", Integer.class,
                CarrierConfigManager.ImsVoice.KEY_MINIMUM_SESSION_EXPIRES_TIMER_SEC_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_SIP_INVITE_CANCELLATION_TIMER_MS,
            new ProvisioningItem("SIP_INVITE_CANCELLATION_TIMER_MS", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_TRANSITION_TO_LTE_DELAY_MS,
            new ProvisioningItem("TRANSITION_TO_LTE_DELAY_MS", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_ENABLE_SILENT_REDIAL,
            new ProvisioningItem("ENABLE_SILENT_REDIAL", Boolean.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_T1_TIMER_VALUE_MS,
            new ProvisioningItem("T1_TIMER_VALUE_MS", Integer.class,
            CarrierConfigManager.Ims.KEY_SIP_TIMER_T1_MILLIS_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_T2_TIMER_VALUE_MS,
            new ProvisioningItem("T2_TIMER_VALUE_MS", Integer.class,
            CarrierConfigManager.Ims.KEY_SIP_TIMER_T2_MILLIS_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_TF_TIMER_VALUE_MS,
            new ProvisioningItem("TF_TIMER_VALUE_MS", Integer.class,
            CarrierConfigManager.Ims.KEY_SIP_TIMER_F_MILLIS_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_VOLTE_PROVISIONING_STATUS,
            new ProvisioningItem("VOLTE_PROVISIONING_STATUS", Boolean.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_VT_PROVISIONING_STATUS,
            new ProvisioningItem("VT_PROVISIONING_STATUS", Boolean.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_REGISTRATION_DOMAIN_NAME,
            new ProvisioningItem("REGISTRATION_DOMAIN_NAME", String.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_SMS_FORMAT,
            new ProvisioningItem("SMS_FORMAT", Integer.class,
            CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_FORMAT_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_SMS_OVER_IP_ENABLED,
            new ProvisioningItem("SMS_OVER_IP_ENABLED", Boolean.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_RCS_PUBLISH_TIMER_SEC,
            new ProvisioningItem("RCS_PUBLISH_TIMER_SEC", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_RCS_PUBLISH_OFFLINE_AVAILABILITY_TIMER_SEC,
            new ProvisioningItem("RCS_PUBLISH_OFFLINE_AVAILABILITY_TIMER_SEC",
            Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_RCS_CAPABILITY_DISCOVERY_ENABLED,
            new ProvisioningItem("RCS_CAPABILITY_DISCOVERY_ENABLED", Boolean.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_RCS_CAPABILITIES_CACHE_EXPIRATION_SEC,
            new ProvisioningItem("RCS_CAPABILITIES_CACHE_EXPIRATION_SEC", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_RCS_AVAILABILITY_CACHE_EXPIRATION_SEC,
            new ProvisioningItem("RCS_AVAILABILITY_CACHE_EXPIRATION_SEC", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_RCS_CAPABILITIES_POLL_INTERVAL_SEC,
            new ProvisioningItem("RCS_CAPABILITIES_POLL_INTERVAL_SEC", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_RCS_PUBLISH_SOURCE_THROTTLE_MS,
            new ProvisioningItem("RCS_PUBLISH_SOURCE_THROTTLE_MS", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_RCS_MAX_NUM_ENTRIES_IN_RCL,
            new ProvisioningItem("RCS_MAX_NUM_ENTRIES_IN_RCL", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_RCS_CAPABILITY_POLL_LIST_SUB_EXP_SEC,
            new ProvisioningItem("RCS_CAPABILITY_POLL_LIST_SUB_EXP_SEC", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_USE_GZIP_FOR_LIST_SUBSCRIPTION,
            new ProvisioningItem("USE_GZIP_FOR_LIST_SUBSCRIPTION", Boolean.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_EAB_PROVISIONING_STATUS,
            new ProvisioningItem("EAB_PROVISIONING_STATUS", Boolean.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE,
            new ProvisioningItem("VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE", Boolean.class,
            CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_ROAMING_ENABLED_BOOL));

        sProvisioningItems.put(ProvisioningManager.KEY_VOICE_OVER_WIFI_MODE_OVERRIDE,
            new ProvisioningItem("VOICE_OVER_WIFI_MODE_OVERRIDE", Integer.class,
            CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_MODE_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_VOICE_OVER_WIFI_ENABLED_OVERRIDE,
            new ProvisioningItem("VOICE_OVER_WIFI_ENABLED_OVERRIDE", Boolean.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_MOBILE_DATA_ENABLED,
            new ProvisioningItem("MOBILE_DATA_ENABLED", Boolean.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_VOLTE_USER_OPT_IN_STATUS,
            new ProvisioningItem("VOLTE_USER_OPT_IN_STATUS", Boolean.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_LOCAL_BREAKOUT_PCSCF_ADDRESS,
            new ProvisioningItem("LOCAL_BREAKOUT_PCSCF_ADDRESS", String.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_SIP_KEEP_ALIVE_ENABLED,
            new ProvisioningItem("SIP_KEEP_ALIVE_ENABLED", Boolean.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_REGISTRATION_RETRY_BASE_TIME_SEC,
            new ProvisioningItem("REGISTRATION_RETRY_BASE_TIME_SEC",
            Integer.class, CarrierConfigManager.Ims.KEY_REGISTRATION_RETRY_BASE_TIMER_MILLIS_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_REGISTRATION_RETRY_MAX_TIME_SEC,
            new ProvisioningItem("REGISTRATION_RETRY_MAX_TIME_SEC", Integer.class,
            CarrierConfigManager.Ims.KEY_REGISTRATION_RETRY_MAX_TIMER_MILLIS_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_RTP_SPEECH_START_PORT,
            new ProvisioningItem("RTP_SPEECH_START_PORT", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_RTP_SPEECH_END_PORT,
            new ProvisioningItem("RTP_SPEECH_END_PORT", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_SIP_INVITE_REQUEST_TRANSMIT_INTERVAL_MS,
            new ProvisioningItem("SIP_INVITE_REQUEST_TRANSMIT_INTERVAL_MS",
            Integer.class, CarrierConfigManager.Ims.KEY_SIP_TIMER_T1_MILLIS_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_SIP_INVITE_ACK_WAIT_TIME_MS,
            new ProvisioningItem("SIP_INVITE_ACK_WAIT_TIME_MS", Integer.class,
            CarrierConfigManager.Ims.KEY_SIP_TIMER_B_MILLIS_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_SIP_INVITE_RESPONSE_RETRANSMIT_WAIT_TIME_MS,
            new ProvisioningItem("SIP_INVITE_RESPONSE_RETRANSMIT_WAIT_TIME_MS", Integer.class,
            CarrierConfigManager.Ims.KEY_SIP_TIMER_D_MILLIS_INT));

        sProvisioningItems.put(
            ProvisioningManager.KEY_SIP_NON_INVITE_REQUEST_RETRANSMIT_INTERVAL_MS,
            new ProvisioningItem("SIP_NON_INVITE_REQUEST_RETRANSMIT_INTERVAL_MS", Integer.class,
            CarrierConfigManager.Ims.KEY_SIP_TIMER_D_MILLIS_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_SIP_NON_INVITE_TRANSACTION_TIMEOUT_TIMER_MS,
            new ProvisioningItem("SIP_NON_INVITE_TRANSACTION_TIMEOUT_TIMER_MS", Integer.class,
            CarrierConfigManager.Ims.KEY_SIP_TIMER_F_MILLIS_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_SIP_INVITE_RESPONSE_RETRANSMIT_INTERVAL_MS,
            new ProvisioningItem("SIP_INVITE_RESPONSE_RETRANSMIT_INTERVAL_MS", Integer.class,
            CarrierConfigManager.Ims.KEY_SIP_TIMER_T1_MILLIS_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_SIP_ACK_RECEIPT_WAIT_TIME_MS,
            new ProvisioningItem("SIP_ACK_RECEIPT_WAIT_TIME_MS", Integer.class,
            CarrierConfigManager.Ims.KEY_SIP_TIMER_H_MILLIS_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_SIP_ACK_RETRANSMIT_WAIT_TIME_MS,
            new ProvisioningItem("SIP_ACK_RETRANSMIT_WAIT_TIME_MS", Integer.class,
            CarrierConfigManager.Ims.KEY_SIP_TIMER_T4_MILLIS_INT));

        sProvisioningItems.put(
            ProvisioningManager.KEY_SIP_NON_INVITE_REQUEST_RETRANSMISSION_WAIT_TIME_MS,
            new ProvisioningItem("SIP_NON_INVITE_REQUEST_RETRANSMISSION_WAIT_TIME_MS",
            Integer.class, CarrierConfigManager.Ims.KEY_SIP_TIMER_J_MILLIS_INT));

        sProvisioningItems.put(
            ProvisioningManager.KEY_SIP_NON_INVITE_RESPONSE_RETRANSMISSION_WAIT_TIME_MS,
            new ProvisioningItem("SIP_NON_INVITE_RESPONSE_RETRANSMISSION_WAIT_TIME_MS",
            Integer.class, CarrierConfigManager.Ims.KEY_SIP_TIMER_T4_MILLIS_INT));

        sProvisioningItems.put(ProvisioningManager.KEY_AMR_WB_OCTET_ALIGNED_PAYLOAD_TYPE,
            new ProvisioningItem("AMR_WB_OCTET_ALIGNED_PAYLOAD_TYPE", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_AMR_WB_BANDWIDTH_EFFICIENT_PAYLOAD_TYPE,
            new ProvisioningItem("AMR_WB_BANDWIDTH_EFFICIENT_PAYLOAD_TYPE", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_AMR_OCTET_ALIGNED_PAYLOAD_TYPE,
            new ProvisioningItem("AMR_OCTET_ALIGNED_PAYLOAD_TYPE", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_AMR_BANDWIDTH_EFFICIENT_PAYLOAD_TYPE,
            new ProvisioningItem("AMR_BANDWIDTH_EFFICIENT_PAYLOAD_TYPE", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_DTMF_WB_PAYLOAD_TYPE,
            new ProvisioningItem("DTMF_WB_PAYLOAD_TYPE", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_DTMF_NB_PAYLOAD_TYPE,
            new ProvisioningItem("DTMF_NB_PAYLOAD_TYPE", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_AMR_DEFAULT_ENCODING_MODE,
            new ProvisioningItem("AMR_DEFAULT_ENCODING_MODE", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_SMS_PUBLIC_SERVICE_IDENTITY,
            new ProvisioningItem("SMS_PUBLIC_SERVICE_IDENTITY", String.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_VIDEO_QUALITY,
            new ProvisioningItem("VIDEO_QUALITY", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_LTE_THRESHOLD_1,
            new ProvisioningItem("LTE_THRESHOLD_1", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_LTE_THRESHOLD_2,
            new ProvisioningItem("LTE_THRESHOLD_2", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_LTE_THRESHOLD_3,
            new ProvisioningItem("LTE_THRESHOLD_3", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_1X_THRESHOLD,
            new ProvisioningItem("1X_THRESHOLD", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_WIFI_THRESHOLD_A,
            new ProvisioningItem("WIFI_THRESHOLD_A", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_WIFI_THRESHOLD_B,
            new ProvisioningItem("WIFI_THRESHOLD_B", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_LTE_EPDG_TIMER_SEC,
            new ProvisioningItem("LTE_EPDG_TIMER_SEC", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_WIFI_EPDG_TIMER_SEC,
            new ProvisioningItem("WIFI_EPDG_TIMER_SEC", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_1X_EPDG_TIMER_SEC,
            new ProvisioningItem("1X_EPDG_TIMER_SEC", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_MULTIENDPOINT_ENABLED,
            new ProvisioningItem("MULTIENDPOINT_ENABLED", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_RTT_ENABLED,
            new ProvisioningItem("RTT_ENABLED", Integer.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_VOICE_OVER_WIFI_ENTITLEMENT_ID,
            new ProvisioningItem("VOICE_OVER_WIFI_ENTITLEMENT_ID", String.class, null));

        sProvisioningItems.put(ProvisioningManager.KEY_VOIMS_OPT_IN_STATUS,
            new ProvisioningItem("VOIMS_OPT_IN_STATUS", Boolean.class, null));
    }
}
