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

import android.net.Uri;

import java.util.Set;

public interface IAosRegistrationListener {

    /**
     * Notify the application that the device is connected to the IMS network.
     *
     * @param networkType The radio access technology. See {@link NetworkType}.
     * @param featureTagBits Type of bits an integer. See {@link FeatureTagMask}
     * @param featureTags Type of {@code Set<String>}.
     */
    void notifyRegistered(int networkType, int featureTagBits, Set<String> featureTags);
    /**
     * Notify the application that the device is connected to the IMS network.
     *
     * @param networkType The radio access technology. See {@link NetworkType}.
     * @param featureTagBits Type of bits an integer. See {@link FeatureTagMask}
     * @param featureTags Type of {@code Set<String>}.
     */
    void notifyRegistering(int networkType, int featureTagBits, Set<String> featureTags);

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
     * @param networkType The technology that has failed to be changed to. See {@link NetworkType}.
     * @param causeCode The handover failure cause. See {@link android.telephony.DataFailCause}.
     * @param message The handover failure message.
     */
    void notifyTechnologyChangeFailed(int networkType, int causeCode, String message);

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
     * Regsitration State
     */
    class RegistrationState {
        public static final int DEREGISTERED = 0;
        public static final int REGISTERING = 1;
        public static final int REGISTERED = 2;
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
    }

    /**
     * Capability
     */
    class Capability {

        public static final int NONE = 0;
        public static final int VOICE = 1;
        public static final int VIDEO = 1 << 1;
        public static final int UT = 1 << 2;
        public static final int SMS = 1 << 3;
        public static final int CALL_COMPOSER = 1 << 4;
        public static final int OPTIONS_UCE = 1 << 5;
        public static final int PRESENCE_UCE = 1 << 6;
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
         * Service unavailable; radio power off
         */
        public static final int CODE_LOCAL_POWER_OFF = 10;
        /**
         * Service unavailable; low battery
         */
        public static final int CODE_LOCAL_LOW_BATTERY = 11;
        /**
         * Service unavailable; out of service (data service state)
         */
        public static final int CODE_LOCAL_NETWORK_NO_SERVICE = 12;
        /**
         * Service unavailable; no LTE coverage
         * (VoLTE is not supported even though IMS is registered)
         */
        public static final int CODE_LOCAL_NETWORK_NO_LTE_COVERAGE = 13;
        /**
         * Service unavailable; located in roaming area
         */
        public static final int CODE_LOCAL_NETWORK_ROAMING = 14;
        /**
         * Service unavailable; IP changed
         */
        public static final int CODE_LOCAL_NETWORK_IP_CHANGED = 15;
        /**
         * Service unavailable; for an unspecified reason
         */
        public static final int CODE_LOCAL_SERVICE_UNAVAILABLE = 16;
        /**
         * Service unavailable; IMS is not registered
         */
        public static final int CODE_LOCAL_NOT_REGISTERED = 17;
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
