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
import android.telephony.ims.ImsReasonInfo;
import android.util.Pair;

import com.android.internal.annotations.Keep;

import java.util.Arrays;
import java.util.Locale;
import java.util.Map;
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
    void notifyRegistered(int regType, NetworkType networkType, int featureTagBits,
            Set<String> featureTags);

    /**
     * Notify the application that the device is trying to connect to the IMS network.
     *
     * @param regType Type of the registration. See {@link RegistrationType}.
     * @param networkType The radio access technology. See {@link NetworkType}.
     * @param featureTagBits Type of bits an integer. See {@link FeatureTagMask}.
     * @param featureTags Type of {@code Set<String>}.
     */
    void notifyRegistering(int regType, NetworkType networkType, int featureTagBits,
            Set<String> featureTags);

    /**
     * Notify the application that the device is disconnected from the IMS network.
     *
     * @param regType Type of the registration. See {@link RegistrationType}.
     * @param networkType The radio access technology. See {@link NetworkType}.
     * @param reason associated with why registration was disconnected. See {@link ReasonCode}.
     * @param message associated with why registration was disconnected.
     */
    void notifyDeregistered(
            int regType, NetworkType networkType, ReasonCode reason, String message);

    /**
     * Notify the framework that the handover from the current radio technology to the other
     * technology has failed.
     *
     * @param regType Type of the registration. See {@link RegistrationType}.
     * @param networkType The technology that has failed to be changed to. See {@link NetworkType}.
     * @param reason The handover failure reason. See {@link ReasonCode}.
     * @param message The handover failure message.
     */
    void notifyTechnologyChangeFailed(
            int regType, NetworkType networkType, ReasonCode reason, String message);

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
    void notifyCapabilitiesUpdateFailed(int capabilities, NetworkType networkType, int reason);

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
     * Represents the registration state. This enum defines the possible states of registration,
     * including deregistered, registering, and registered.
     * Each state is associated with an integer value for easier identification and comparison.
     */
    enum RegistrationState {

        DEREGISTERED(0),
        REGISTERING(1),
        REGISTERED(2);

        private final int mValue;

        RegistrationState(int value) {
            mValue = value;
        }

        /**
         * Returns the integer value associated with this registration state.
         *
         * @return The integer value representing the state.
         */
        public int getValue() {
            return mValue;
        }

        /**
         * Returns the string representation of this registration state in lowercase.
         *
         * @return The lowercase string representation of the state.
         */
        @Override
        public String toString() {
            return name().toLowerCase(Locale.US);
        }
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
    enum NetworkType {
        NONE(-1),
        LTE(0),
        IWLAN(1),
        CROSS_SIM(2),
        NR(3),
        UTRAN(4);

        private final int mValue;

        NetworkType(int value) {
            mValue = value;
        }

        /**
         * Returns the integer value of the NetworkType.
         *
         * @return The integer value of the NetworkType.
         */
        public int getValue() {
            return mValue;
        }

        @Override
        public String toString() {
            return name();
        }

        /**
         * Returns the NetworkType enum constant corresponding to the given integer value.
         *
         * @param value The integer value to look up.
         * @return The NetworkType enum constant with the given value, or
         *         {@code NetworkType.NONE} if no such constant exists.
         */
        public static NetworkType of(int value) {
            return Arrays.stream(values())
                .filter(type -> type.mValue == value)
                .findFirst()
                .orElse(NONE);
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
        public static final int CALL_COMPOSER_BUSINESS_ONLY = 1 << 5;
        /** RcsFeature capability
        * {@link android.telephony.ims.feature.RcsFeature.RcsImsCapabilities}.
        */
        public static final int OPTIONS_UCE = 1 << 6;
        public static final int PRESENCE_UCE = 1 << 7;
        // Internal capability
        public static final int TEXT = 1 << 11;

        public static String toString(int capabilities) {
            StringBuilder sb = new StringBuilder("[");

            appendToken(sb, capabilities, VOICE, "voice");
            appendToken(sb, capabilities, VIDEO, "video");
            appendToken(sb, capabilities, UT, "ut");
            appendToken(sb, capabilities, SMS, "sms");
            appendToken(sb, capabilities, CALL_COMPOSER, "call_composer");
            appendToken(sb, capabilities, CALL_COMPOSER_BUSINESS_ONLY,
                    "call_composer_business_only");
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

    /**
     * Enum class representing the reasons for registration disconnection.
     * Each reason belongs to a specific category, indicated by its base value.
     */
    enum ReasonCode {

        /**
         * BASE : 0 (General Errors)
         */
        UNSPECIFIED(ReasonCode.BASE, 0),
        REGISTRATION_ERROR(ReasonCode.BASE, 1),
        POWER_OFF(ReasonCode.BASE, 2),
        LOW_BATTERY(ReasonCode.BASE, 3),
        NETWORK_NO_SERVICE(ReasonCode.BASE, 4),
        NETWORK_NO_LTE_COVERAGE(ReasonCode.BASE, 5),
        NETWORK_ROAMING(ReasonCode.BASE, 6),
        NETWORK_IP_CHANGED(ReasonCode.BASE, 7),
        SERVICE_UNAVAILABLE(ReasonCode.BASE, 8),
        NOT_REGISTERED(ReasonCode.BASE, 9),
        USIM_AUTHENTICATION_FAILURES(ReasonCode.BASE, 10),
        INTERNAL_ERROR(ReasonCode.BASE, 11),

        /**
         * BASE_MODEM : 2000 (Errors requiring special action from the modem.)
         */
        PLMN_BLOCK(ReasonCode.BASE_MODEM, 0),
        PLMN_BLOCK_WITH_TIMEOUT(ReasonCode.BASE_MODEM, 1),
        RAT_BLOCK(ReasonCode.BASE_MODEM, 2),
        CLEAR_RAT_BLOCKS(ReasonCode.BASE_MODEM, 3),

        /**
         * BASE_DATA : 3000 (Errors due to data failures.)
         * The reasons below are for mapping the ImsReasonInfo ExtraCode by the data failure causes.
         */
        DATA_NOT_MATCHED(ReasonCode.BASE_DATA, 0),
        DATA_UNSPECIFIED(ReasonCode.BASE_DATA, 1),
        DATA_LOCAL_NETWORK_NO_SERVICE(ReasonCode.BASE_DATA, 2),
        DATA_LOCAL_SERVICE_UNAVAILABLE(ReasonCode.BASE_DATA, 3),
        DATA_EPDG_TUNNEL_ESTABLISH_FAILURE(ReasonCode.BASE_DATA, 4),
        DATA_WIFI_LOST(ReasonCode.BASE_DATA, 5),
        DATA_RADIO_OFF(ReasonCode.BASE_DATA, 6),
        DATA_NO_VALID_SIM(ReasonCode.BASE_DATA, 7),
        DATA_RADIO_INTERNAL_ERROR(ReasonCode.BASE_DATA, 8),
        DATA_RADIO_LINK_LOST(ReasonCode.BASE_DATA, 9),
        DATA_RADIO_RELEASE_NORMAL(ReasonCode.BASE_DATA, 10),
        DATA_RADIO_RELEASE_ABNORMAL(ReasonCode.BASE_DATA, 11),
        DATA_NETWORK_DETACH(ReasonCode.BASE_DATA, 12),
        DATA_OEM_CAUSE_4(ReasonCode.BASE_DATA, 13),

        /**
         * BASE_RESP_4XX : 14000 (Errors due to registration response 4XX.)
         */
        REG_RESP_403(ReasonCode.BASE_RESP_4XX, 403),

        /**
         * BASE_RESP_OTHER : 17000 (Errors due to registration other response.)
         */
        REG_RESP_NETWORK_TIMEOUT(ReasonCode.BASE_RESP_OTHER, 0),

        /**
         * BASE_RESP_WFC_4XX : 24000 (Errors due to WFC registration response 4XX.)
         */
        WFC_REG_RESP_403(ReasonCode.BASE_RESP_WFC_4XX, 403),

        /**
         * BASE_RESP_WFC_5XX : 25000 (Errors due to WFC registration response 5XX)
         */
        WFC_REG_RESP_500(ReasonCode.BASE_RESP_WFC_5XX, 500),

        /**
         * BASE_RESP_WFC_OTHER : 27000 (Errors due to WFC registration other response.)
         */
        WFC_REG_RESP_403_NOT_SUPPORTED_COUNTRY(ReasonCode.BASE_RESP_WFC_OTHER, 0),
        WFC_REG_RESP_OTHER_FAILURES(ReasonCode.BASE_RESP_WFC_OTHER, 1),
        WFC_SUB_RESP_403(ReasonCode.BASE_RESP_WFC_OTHER, 2),
        WFC_SUB_NOTIFY_TERMINATED(ReasonCode.BASE_RESP_WFC_OTHER, 3);


        /**
         * General Errors.
         */
        private static final int BASE = 0;

        /**
         * Errors requiring special action from the modem.
         */
        private static final int BASE_MODEM = 2000;

        /**
         * Errors due to data failures.
         */
        private static final int BASE_DATA = 3000;

        /**
         * Errors due to registration common failures.
         */
        @Keep // Reserved for future use.
        private static final int BASE_COMMON_OTHER = 4000;

        /**
         * Errors due to registration response 3XX.
         */
        @Keep // Reserved for future use.
        private static final int BASE_RESP_3XX = 13000;

        /**
         * Errors due to registration response 4XX.
         */
        private static final int BASE_RESP_4XX = 14000;

        /**
         * Errors due to registration response 5XX.
         */
        @Keep // Reserved for future use.
        private static final int BASE_RESP_5XX = 15000;

        /**
         * Errors due to registration response 6XX.
         */
        @Keep // Reserved for future use.
        private static final int BASE_RESP_6XX = 16000;

        /**
         * Errors due to registration other response.
         */
        private static final int BASE_RESP_OTHER = 17000;

        /**
         * Errors due to WFC registration response 3XX.
         */
        @Keep // Reserved for future use.
        private static final int BASE_RESP_WFC_3XX = 23000;

        /**
         * Errors due to WFC registration response 4XX.
         */
        private static final int BASE_RESP_WFC_4XX = 24000;

        /**
         * Errors due to WFC registration response 5XX.
         */
        private static final int BASE_RESP_WFC_5XX = 25000;

        /**
         * Errors due to WFC registration response 6XX.
         */
        @Keep // Reserved for future use.
        private static final int BASE_RESP_WFC_6XX = 26000;

        /**
         * Errors due to WFC registration other response.
         */
        private static final int BASE_RESP_WFC_OTHER = 27000;

        private final int mValue;

        /**
         * Constructs a {@code ReasonCode} enum value with the specified base value and offset.
         *
         * @param base   The base value representing the category of the reason code.
         * @param offset The offset within the category.
         */
        ReasonCode(int base, int offset) {
            mValue = base + offset;
        }

        /**
         * Retrieves the integer value associated with this reason code.
         *
         * @return The integer value of the reason code.
         */
        public int getValue() {
            return mValue;
        }

        /**
         * Returns the name of this enum constant, as contained in the declaration.
         *
         * @return the name of this enum constant
         */
        @Override
        public String toString() {
            return name();
        }

        /**
         * Returns the enum constant of this type with the specified integer value.
         * If no matching constant is found, {@code UNSPECIFIED} is returned.
         *
         * @param value The integer value of the reason code.
         * @return The corresponding {@code ReasonCode} enum constant or
         *         {@code UNSPECIFIED} if not found
         */
        public static ReasonCode of(int value) {
            return Arrays.stream(values())
                .filter(cause -> cause.mValue == value)
                .findFirst()
                .orElse(UNSPECIFIED);
        }
    }

    /**
     *  Map it with the {@code ImsReasonInfo} information based on {@code ReasonCode}
     */
    class ReasonCodeMap {
        private static final Map<ReasonCode, Pair<Integer, Integer>> REASON_MAP = Map.ofEntries(
                Map.entry(ReasonCode.DATA_NOT_MATCHED, Pair.create(
                        ImsReasonInfo.CODE_REGISTRATION_ERROR, 65535)),
                Map.entry(ReasonCode.DATA_UNSPECIFIED, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE,
                        ImsReasonInfo.CODE_UNSPECIFIED)),
                Map.entry(ReasonCode.DATA_LOCAL_NETWORK_NO_SERVICE, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE,
                        ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE)),
                Map.entry(ReasonCode.DATA_LOCAL_SERVICE_UNAVAILABLE, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE,
                        ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE)),
                Map.entry(ReasonCode.DATA_EPDG_TUNNEL_ESTABLISH_FAILURE, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE,
                        ImsReasonInfo.CODE_EPDG_TUNNEL_ESTABLISH_FAILURE)),
                Map.entry(ReasonCode.DATA_WIFI_LOST, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE, ImsReasonInfo.CODE_WIFI_LOST)),
                Map.entry(ReasonCode.DATA_RADIO_OFF, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE, ImsReasonInfo.CODE_RADIO_OFF)),
                Map.entry(ReasonCode.DATA_NO_VALID_SIM, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE,
                        ImsReasonInfo.CODE_NO_VALID_SIM)),
                Map.entry(ReasonCode.DATA_RADIO_INTERNAL_ERROR, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE,
                        ImsReasonInfo.CODE_RADIO_INTERNAL_ERROR)),
                Map.entry(ReasonCode.DATA_RADIO_LINK_LOST, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE,
                        ImsReasonInfo.CODE_RADIO_LINK_LOST)),
                Map.entry(ReasonCode.DATA_RADIO_RELEASE_NORMAL, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE,
                        ImsReasonInfo.CODE_RADIO_RELEASE_NORMAL)),
                Map.entry(ReasonCode.DATA_RADIO_RELEASE_ABNORMAL, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE,
                        ImsReasonInfo.CODE_RADIO_RELEASE_ABNORMAL)),
                Map.entry(ReasonCode.DATA_NETWORK_DETACH, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE,
                        ImsReasonInfo.CODE_NETWORK_DETACH)),
                Map.entry(ReasonCode.DATA_OEM_CAUSE_4, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE,
                        ImsReasonInfo.CODE_OEM_CAUSE_4)),
                Map.entry(ReasonCode.REG_RESP_403, Pair.create(
                        ImsReasonInfo.CODE_REGISTRATION_ERROR, ImsReasonInfo.CODE_SIP_FORBIDDEN)),
                Map.entry(ReasonCode.WFC_REG_RESP_403, Pair.create(
                        ImsReasonInfo.CODE_REGISTRATION_ERROR, ImsReasonInfo.CODE_SIP_FORBIDDEN)),
                Map.entry(ReasonCode.WFC_REG_RESP_403_NOT_SUPPORTED_COUNTRY, Pair.create(
                        ImsReasonInfo.CODE_REGISTRATION_ERROR, ImsReasonInfo.CODE_SIP_FORBIDDEN)),
                Map.entry(ReasonCode.REG_RESP_NETWORK_TIMEOUT, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NOT_REGISTERED,
                                ImsReasonInfo.CODE_NETWORK_RESP_TIMEOUT)),
                Map.entry(ReasonCode.INTERNAL_ERROR, Pair.create(
                        ImsReasonInfo.CODE_LOCAL_NOT_REGISTERED,
                        ImsReasonInfo.CODE_RADIO_INTERNAL_ERROR))
        );

        /**
         * Return the map variable that matches {@code ReasonCode} and {@code ImsReasonInfo}.
         *
         * @return The map variable.
         */
        public static Map<ReasonCode, Pair<Integer, Integer>> getReasonMap() {
            return REASON_MAP;
        }

        /**
         * Returns the pair of {@code ImsReasonInfo} based on Reasoncode.
         * The pair is consist of code and extraCode of {@code ImsReasonInfo}
         *
         * @param key ReasonCode
         * @return The pair of {@code ImsReasonInfo}
         */
        public static Pair<Integer, Integer> getImsReasonPair(ReasonCode key) {
            return REASON_MAP.getOrDefault(key, Pair.create(ImsReasonInfo.CODE_REGISTRATION_ERROR,
                    ImsReasonInfo.CODE_UNSPECIFIED));
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
