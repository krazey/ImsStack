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
package com.android.imsstack.enabler.aos;

import android.annotation.NonNull;
import android.net.Uri;

import java.util.Set;

public interface IAosRegistrationListener {

    /**
     * Notify the application that the device is connected to the IMS network.
     *
     * @param regType Type of the registration. See {@link RegistrationType}.
     * @param networkType The radio access technology. See {@link NetworkType}.
     * @param featureTagBits Type of bits an integer. See {@link FeatureTagMask}.
     * @param featureTags Type of {@code Set<String>}.
     */
    void notifyRegistered(int regType, int networkType, int featureTagBits,
            Set<String> featureTags);

    /**
     * Notify the application that the device is trying to connect to the IMS network.
     *
     * @param regType Type of the registration. See {@link RegistrationType}.
     * @param networkType The radio access technology. See {@link NetworkType}.
     * @param featureTagBits Type of bits an integer. See {@link FeatureTagMask}.
     * @param featureTags Type of {@code Set<String>}.
     */
    void notifyRegistering(int regType, int networkType, int featureTagBits,
            Set<String> featureTags);

    /**
     * Notify the application that the device is disconnected from the IMS network.
     *
     * @param networkType The radio access technology. See {@link NetworkType}.
     * @param reason associated with why registration was disconnected. See {@link ReasonCode}.
     * @param message associated with why registration was disconnected.
     */
    void notifyDeregistered(int networkType, int reason, String message);

    /**
     * Notify the framework that the handover from the current radio technology to the other
     * technology has failed.
     *
     * @param regType Type of the registration. See {@link RegistrationType}.
     * @param networkType The technology that has failed to be changed to. See {@link NetworkType}.
     * @param causeCode The handover failure cause. See {@link android.telephony.DataFailCause}.
     * @param message The handover failure message.
     */
    void notifyTechnologyChangeFailed(int regType, int networkType, int causeCode, String message);

    /**
     * This device's subscriber associated {@link Uri}s have changed, which are used to filter out
     * this device's {@link Uri}s during conference calling.
     *
     * @param uris the network provisioned public user identities.
     */
    void notifyAssociatedUriChanged(Uri[] uris);

    /**
     * This method is called when capability update fails after
     * {@link IAosRegistration#changeCapabilities} is called.
     *
     * @param capabilities capabilities that failed to update. See {@link Capability}.
     * @param networkType The radio access technology. See {@link NetworkType}.
     * @param reason Reason for update failure. See {@link CapabilityReason}.
     */
    void notifyCapabilitiesUpdateFailed(int capabilities, int networkType, int reason);

    /**
     * This method is called when capabilities are changed from
     * {@link com.android.imsstack.enabler.aos.service.AosService}.
     *
     * @param pairs An instance of {@link IAosRegistration.CapabilityPairs},
     *              representing a pair of capabilities and network types.
     *              The {@code pairs} contains all enabled capabilities
     *              for each network type.
     * @see IAosRegistrationListener.Capability
     * @see IAosRegistrationListener.NetworkType
     */
    void notifyCapabilitiesUpdated(IAosRegistration.CapabilityPairs pairs);

    /**
     * This method is called to notify changes in the registration event state.
     * The provided {@code impus} is applicable only when the {@code statusCode} is 200.
     *
     * @param statusCode IMS registration status code.
     * @param impus Set of IMPU.
     */
    void notifyRegEventStateChanged(int statusCode, @NonNull Set<Uri> impus);

    /**
     * Registration State
     */
    class RegistrationState {
        public static final int DEREGISTERED = 0;
        public static final int REGISTERING = 1;
        public static final int REGISTERED = 2;
    }

    /**
     * Registration Type
     *
     * This indicates the type of the registration which will be reported to the Telephony.
     * It is not equivalent to AosRegistrationType in native.
     *
     * NORMAL       Used for non-emergency registration status.
     * EMERGENCY    Used for emergency registration status.
     * FAKE         Used when an emergency call is initiated without emergency registration.
     *              It includes both the case that UE never try an emergency registration
     *              and the case that UE fails the emergency registration.
     */
    class RegistrationType {
        public static final int NORMAL = 0;
        public static final int EMERGENCY = 1;
        public static final int FAKE = 2;

        /**
         * This method returns a String for the given registration type.
         *
         * @param regType The registration Type.
         * @return A String for the given registration type.
         */
        public static String toString(int regType) {
            return switch (regType) {
                case NORMAL -> "NORMAL";
                case EMERGENCY -> "EMERGENCY";
                case FAKE -> "FAKE";
                default -> "NONE";
            };
        }
    }

    /**
     * NETWORK_TYPE
     */
    class NetworkType {
        public static final int NONE = -1;
        public static final int LTE = 0;
        public static final int IWLAN = 1;
        public static final int CROSS_SIM = 2;
        public static final int NR = 3;
        public static final int UTRAN = 4;

        public static String toString(int networkType) {
            return switch (networkType) {
                case LTE -> "LTE";
                case IWLAN -> "IWLAN";
                case CROSS_SIM -> "CROSS_SIM";
                case NR -> "NR";
                case UTRAN -> "UTRAN";
                default -> "NONE";
            };
        }
    }

    /**
     * Capability
     */
    class Capability {
        /** MmTelFeature capability
        * {@link android.telephony.ims.feature.MmTelFeature.MmTelCapabilities}.
        */
        public static final int NONE = 0;
        public static final int VOICE = 1;
        public static final int VIDEO = 1 << 1;
        public static final int UT = 1 << 2;
        public static final int SMS = 1 << 3;
        public static final int CALL_COMPOSER = 1 << 4;
        /** RcsFeature capability
        * {@link android.telephony.ims.feature.RcsFeature.RcsImsCapabilities}.
        */
        public static final int OPTIONS_UCE = 1 << 5;
        public static final int PRESENCE_UCE = 1 << 6;
        // Internal capability
        public static final int TEXT = 1 << 11;

        public static String toString(int capabilities) {
            StringBuilder sb = new StringBuilder("[");

            appendToken(sb, capabilities, VOICE, "voice");
            appendToken(sb, capabilities, VIDEO, "video");
            appendToken(sb, capabilities, UT, "ut");
            appendToken(sb, capabilities, SMS, "sms");
            appendToken(sb, capabilities, CALL_COMPOSER, "call_composer");
            appendToken(sb, capabilities, OPTIONS_UCE, "options_uce");
            appendToken(sb, capabilities, PRESENCE_UCE, "presence_uce");
            appendToken(sb, capabilities, TEXT, "text");
            sb.append("]");

            return sb.toString();
        }
    }

    /**
     * FeatureTag Mask
     */
    class FeatureTagMask {

        public static final int NONE = 0;
        /// MTC
        public static final int MMTEL = 0x00000001;
        public static final int VIDEO = 0x00000002;
        public static final int TEXT = 0x00000004;
        public static final int USSI = 0x00000008;
        public static final int VERSTAT = 0x00000010;
        /// MTS
        public static final int SMSIP = 0x00000020;
        /// SIP Controller
        public static final int STANDALONE_MSG = 0x00000040;
        public static final int CHAT_IM = 0x00000080;
        public static final int CHAT_SESSION = 0x00000100;
        public static final int FILE_TRANSFER = 0x00000200;
        public static final int FILE_TRANSFER_VIA_SMS = 0x00000400;
        public static final int CALL_COMPOSER_ENRICHED_CALLING = 0x00000800;
        public static final int CALL_COMPOSER_VIA_TELEPHONY = 0x00001000;
        public static final int POST_CALL = 0x00002000;
        public static final int SHARED_MAP = 0x00004000;
        public static final int SHARED_SKETCH = 0x00008000;
        public static final int GEO_PUSH = 0x00010000;
        public static final int GEO_PUSH_VIA_SMS = 0x00020000;
        public static final int CHATBOT_COMMUNICATION_USING_SESSION = 0x00040000;
        public static final int CHATBOT_COMMUNICATION_USING_STANDALONE_MSG = 0x00080000;
        public static final int CHATBOT_VERSION_SUPPORTED = 0x00100000;
        public static final int CHATBOT_VERSION_V2_SUPPORTED = 0x00200000;
        public static final int CHATBOT_ROLE = 0x00400000;
        public static final int PRESENCE = 0x00800000;

        /// ALL
        public static final int ALL = 0xFFFFFFFF;

        public static String toString(int feature) {
            StringBuilder sb = new StringBuilder("[");

            appendToken(sb, feature, MMTEL, "mmtel");
            appendToken(sb, feature, VIDEO, "video");
            appendToken(sb, feature, TEXT, "text");
            appendToken(sb, feature, USSI, "ussi");
            appendToken(sb, feature, VERSTAT, "verstat");
            appendToken(sb, feature, SMSIP, "smsip");
            appendToken(sb, feature, STANDALONE_MSG, "standalone_msg");
            appendToken(sb, feature, CHAT_IM, "chat_im");
            appendToken(sb, feature, CHAT_SESSION, "chat_session");
            appendToken(sb, feature, FILE_TRANSFER, "file_transfer");
            appendToken(sb, feature, FILE_TRANSFER_VIA_SMS, "file_transfer_via_sms");
            appendToken(sb, feature, CALL_COMPOSER_ENRICHED_CALLING,
                    "call_composer_enriched_calling");
            appendToken(sb, feature, CALL_COMPOSER_VIA_TELEPHONY, "call_composer_via_telephony");
            appendToken(sb, feature, POST_CALL, "post_call");
            appendToken(sb, feature, SHARED_MAP, "shared_map");
            appendToken(sb, feature, SHARED_SKETCH, "shared_sketch");
            appendToken(sb, feature, GEO_PUSH, "geo_push");
            appendToken(sb, feature, GEO_PUSH_VIA_SMS, "geo_push_via_sms");
            appendToken(sb, feature, CHATBOT_COMMUNICATION_USING_SESSION,
                    "chatbot_communication_using_session");
            appendToken(sb, feature, CHATBOT_COMMUNICATION_USING_STANDALONE_MSG,
                    "chatbot_communication_using_standalone_msg");
            appendToken(sb, feature, CHATBOT_VERSION_SUPPORTED, "chatbot_version_supported");
            appendToken(sb, feature, CHATBOT_VERSION_V2_SUPPORTED, "chatbot_version_v2_supported");
            appendToken(sb, feature, CHATBOT_ROLE, "chatbot_role");
            appendToken(sb, feature, PRESENCE, "presence");
            sb.append("]");

            return sb.toString();
        }
    }

    /**
     * Appends the specified token to the StringBuilder if the specified bitmask is set.
     *
     * @param sb       The StringBuilder to which the token should be appended.
     * @param bitmasks The bitmasks to check.
     * @param bitmask  The bitmask to compare against.
     * @param token    The token to append.
     */
    private static void appendToken(StringBuilder sb, int bitmasks, int bitmask, String token) {
        if ((bitmasks & bitmask) != 0) {
            sb.append(token).append(" ");
        }
    }

    // TODO : need to check this class.

    /**
     * ReasonCode
     */
    class ReasonCode {

        /**
         * The Reason is unspecified.
         */
        public static final int CODE_UNSPECIFIED = 0;
        /**
         * Indicates that the IMS registration is failed with fatal error such as 403 or 404
         * on all P-CSCF addresses. The radio shall block the current PLMN or disable
         * the RAT
         */
        public static final int CODE_PLMN_BLOCK = 1;
        /**
         * Indicates that the IMS registration on current PLMN failed multiple times.
         * The radio shall block the current PLMN or disable the RAT during the time
         * based on carrier requirement
         */
        public static final int CODE_PLMN_BLOCK_WITH_TIMEOUT = 2;
        /**
         * IMS Registration error code
         */
        public static final int CODE_REGISTRATION_ERROR = 3;
        /**
         * WFC Registration error code if the network returns 403 Forbidden for Register.
         * The 403 Forbidden case due to non-support for other countries are not included.
         */
        public static final int CODE_REGISTRATION_ERROR_WFC_REG_403 = 4;
        /**
         * WFC Registration error code if the network returns 500 error for Register.
         */
        public static final int CODE_REGISTRATION_ERROR_WFC_REG_500 = 5;
        /**
         * WFC Registration error code if the network returns 403 error
         *  with a different country for register.
         */
        public static final int CODE_REGISTRATION_ERROR_WFC_NOT_SUPPORTED_COUNTRY = 6;
        /**
         * WFC Registration error code if the network returns 403 response for Subscribe.
         */
        public static final int CODE_REGISTRATION_ERROR_WFC_SUB_403 = 7;
        /**
         * WFC Registration error code if the network returns Notify Terminate message.
         */
        public static final int CODE_REGISTRATION_ERROR_WFC_NOTIFY_TERMINATED = 8;
        /**
         * WFC Registration error code for all other failures.
         */
        public static final int CODE_REGISTRATION_ERROR_WFC_OTHER_FAILURES = 9;
        /**
         * Registration error code for USIM authentication failures.
         */
        public static final int CODE_REGISTRATION_ERROR_USIM_AUTHENTICATION_FAILURES = 10;
        /**
         * Service unavailable; radio power off
         */
        public static final int CODE_LOCAL_POWER_OFF = 11;
        /**
         * Service unavailable; low battery
         */
        public static final int CODE_LOCAL_LOW_BATTERY = 12;
        /**
         * Service unavailable; out of service (data service state)
         */
        public static final int CODE_LOCAL_NETWORK_NO_SERVICE = 13;
        /**
         * Service unavailable; no LTE coverage
         * (VoLTE is not supported even though IMS is registered)
         */
        public static final int CODE_LOCAL_NETWORK_NO_LTE_COVERAGE = 14;
        /**
         * Service unavailable; located in roaming area
         */
        public static final int CODE_LOCAL_NETWORK_ROAMING = 15;
        /**
         * Service unavailable; IP changed
         */
        public static final int CODE_LOCAL_NETWORK_IP_CHANGED = 16;
        /**
         * Service unavailable; for an unspecified reason
         */
        public static final int CODE_LOCAL_SERVICE_UNAVAILABLE = 17;
        /**
         * Service unavailable; IMS is not registered
         */
        public static final int CODE_LOCAL_NOT_REGISTERED = 18;
        /**
         * The current RAT was blocked because registration failed for all P-CSCFs.
         */
        public static final int CODE_RAT_BLOCK = 19;
        /**
         * Clears blocks for all RATs.
         */
        public static final int CODE_CLEAR_RAT_BLOCKS = 20;

        public static String toString(int reasonCode) {
            return switch (reasonCode) {
                case CODE_UNSPECIFIED -> "CODE_UNSPECIFIED";
                case CODE_PLMN_BLOCK -> "CODE_PLMN_BLOCK";
                case CODE_PLMN_BLOCK_WITH_TIMEOUT -> "CODE_PLMN_BLOCK_WITH_TIMEOUT";
                case CODE_REGISTRATION_ERROR -> "CODE_REGISTRATION_ERROR";
                case CODE_REGISTRATION_ERROR_WFC_REG_403 -> "CODE_REGISTRATION_ERROR_WFC_REG_403";
                case CODE_REGISTRATION_ERROR_WFC_REG_500 -> "CODE_REGISTRATION_ERROR_WFC_REG_500";
                case CODE_REGISTRATION_ERROR_WFC_NOT_SUPPORTED_COUNTRY ->
                        "CODE_REGISTRATION_ERROR_WFC_NOT_SUPPORTED_COUNTRY";
                case CODE_REGISTRATION_ERROR_WFC_SUB_403 -> "CODE_REGISTRATION_ERROR_WFC_SUB_403";
                case CODE_REGISTRATION_ERROR_WFC_NOTIFY_TERMINATED ->
                        "CODE_REGISTRATION_ERROR_WFC_NOTIFY_TERMINATED";
                case CODE_REGISTRATION_ERROR_WFC_OTHER_FAILURES ->
                        "CODE_REGISTRATION_ERROR_WFC_OTHER_FAILURES";
                case CODE_REGISTRATION_ERROR_USIM_AUTHENTICATION_FAILURES ->
                        "CODE_REGISTRATION_ERROR_USIM_AUTHENTICATION_FAILURES";
                case CODE_LOCAL_POWER_OFF -> "CODE_LOCAL_POWER_OFF";
                case CODE_LOCAL_LOW_BATTERY -> "CODE_LOCAL_LOW_BATTERY";
                case CODE_LOCAL_NETWORK_NO_SERVICE -> "CODE_LOCAL_NETWORK_NO_SERVICE";
                case CODE_LOCAL_NETWORK_NO_LTE_COVERAGE -> "CODE_LOCAL_NETWORK_NO_LTE_COVERAGE";
                case CODE_LOCAL_NETWORK_ROAMING -> "CODE_LOCAL_NETWORK_ROAMING";
                case CODE_LOCAL_NETWORK_IP_CHANGED -> "CODE_LOCAL_NETWORK_IP_CHANGED";
                case CODE_LOCAL_SERVICE_UNAVAILABLE -> "CODE_LOCAL_SERVICE_UNAVAILABLE";
                case CODE_LOCAL_NOT_REGISTERED -> "CODE_LOCAL_NOT_REGISTERED";
                case CODE_RAT_BLOCK -> "CODE_RAT_BLOCK";
                case CODE_CLEAR_RAT_BLOCKS -> "CODE_CLEAR_RAT_BLOCKS";
                default -> "Unknown";
            };
        }
    }

    /**
     * CapabilityReason
     */
    class CapabilityReason {

        /**
         * The capability was unable to be changed.
         */
        public static final int ERROR_GENERIC = -1;
        /**
         * The capability was able to be changed.
         */
        public static final int SUCCESS = 0;
    }
}
